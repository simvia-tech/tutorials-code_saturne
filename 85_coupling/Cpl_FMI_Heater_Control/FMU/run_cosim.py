#!/usr/bin/env python3
"""Drive the code_saturne FMU from Python: the controller side of the loop.

Two modes:

    python3 run_cosim.py open-loop    step on the heater flux, no controller,
                                      to see the plant response and tune the PID
    python3 run_cosim.py pid          closed loop: a PI controller holds the
                                      outlet temperature at a setpoint

Both need the paths below, either on the command line or by editing the
defaults. One fmi2DoStep is one code_saturne time step, so `dt` must be the
time step of the case (time_step_ref in setup.xml).
"""

import argparse
import csv
import os
import shutil
import sys
import time

from fmpy import extract, read_model_description
from fmpy.fmi2 import FMU2Slave

HERE = os.path.dirname(os.path.abspath(__file__))

DT = 0.005            # the case's time step: one doStep advances exactly one
T_IN = 300.0          # inlet temperature [K], for reference only


def parse_args():
    p = argparse.ArgumentParser(description=__doc__,
                                formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument("mode", choices=["open-loop", "pid"])
    p.add_argument("--fmu", default=os.path.join(HERE, "FMU_CS.fmu"))
    p.add_argument("--code-saturne", required=True,
                   help="path to the code_saturne launcher script")
    p.add_argument("--case", default=os.path.normpath(os.path.join(HERE, "..", "CASE")),
                   help="path to the CASE directory")
    p.add_argument("--run-id", default=None,
                   help="RESU/<run-id> for this run (default: fmi_<mode>)")
    p.add_argument("--t-end", type=float, default=15.0, help="physical end time [s]")
    p.add_argument("--out", default=None, help="CSV output (default: RESU/<run-id>/cosim.csv)")
    # controller: SIMC rules on the plant identified from the open-loop step
    # (K = 0.0207 K per W/m2, dead time L = 0.39 s, time constant tau = 0.47 s):
    # Kc = tau / (K (tau_c + L)) with tau_c = L, Ti = min(tau, 4 (tau_c + L))
    p.add_argument("--kp", type=float, default=30.0, help="proportional gain [W/m2 per K]")
    p.add_argument("--ki", type=float, default=60.0, help="integral gain [W/m2 per K per s]")
    p.add_argument("--q-max", type=float, default=4000.0, help="heater limit [W/m2]")
    return p.parse_args()


def scenario(mode, t):
    """Setpoint, heater flux (open loop only) and inlet velocity at time t."""
    if mode == "open-loop":
        q = 1000.0 if t >= 1.0 else 0.0          # step at t = 1 s
        return None, q, 0.5
    t_set = 315.0 if t < 5.0 else 320.0          # setpoint step at t = 5 s
    u_in = 0.5 if t < 10.0 else 0.7              # flow disturbance at t = 10 s
    return t_set, None, u_in


class PI:
    """Textbook PI with output clamping and anti-windup (integrator freezes
    while the heater is saturated in the direction of the error)."""

    def __init__(self, kp, ki, q_min, q_max):
        self.kp, self.ki, self.q_min, self.q_max = kp, ki, q_min, q_max
        self.integral = 0.0

    def update(self, error, dt):
        q_unsat = self.kp*error + self.ki*(self.integral + error*dt)
        if (q_unsat > self.q_max and error > 0) or (q_unsat < self.q_min and error < 0):
            pass                                  # saturated: do not wind up
        else:
            self.integral += error*dt
        q = self.kp*error + self.ki*self.integral
        return min(max(q, self.q_min), self.q_max)


def main():
    a = parse_args()
    run_id = a.run_id or f"fmi_{a.mode.replace('-', '_')}"
    out = a.out or os.path.join(a.case, "RESU", run_id, "cosim.csv")

    md = read_model_description(a.fmu)
    vr = {v.name: v.valueReference for v in md.modelVariables}
    unzipdir = extract(a.fmu)

    fmu = FMU2Slave(guid=md.guid, unzipDirectory=unzipdir,
                    modelIdentifier=md.coSimulation.modelIdentifier,
                    instanceName="code_saturne")
    fmu.instantiate()

    # where and how the FMU launches the solver
    fmu.setString([vr["code_saturne"], vr["casename"], vr["run_id"]],
                  [a.code_saturne, a.case, run_id])

    # Initial inputs are set *before* initialization mode. The FMU registers
    # every input with the solver when it enters that mode, and the 9.1 solver
    # registers a variable again each time it is told about it, so setting an
    # input during initialization would make the solver expect more values per
    # step than the FMU sends, and the first step would never return.
    t_set, q, u_in = scenario(a.mode, 0.0)
    q0 = 0.0 if a.mode == "open-loop" else 1000.0
    fmu.setupExperiment(startTime=0.0)
    fmu.setReal([vr["q_wall"], vr["u_in"]], [q0, u_in])
    fmu.enterInitializationMode()            # code_saturne is launched here
    fmu.exitInitializationMode()

    pi = PI(a.kp, a.ki, 0.0, a.q_max)
    n_steps = int(round(a.t_end/DT))
    rows = []
    t = 0.0
    q = q0
    wall0 = time.time()
    print(f"{a.mode}: {n_steps} steps of {DT} s, run id {run_id}")

    for n in range(n_steps):
        t_set, q_open, u_in = scenario(a.mode, t)
        t_out = fmu.getReal([vr["t_out"]])[0]

        if a.mode == "open-loop":
            q = q_open
        else:
            q = pi.update(t_set - t_out, DT)

        fmu.setReal([vr["q_wall"], vr["u_in"]], [q, u_in])
        fmu.doStep(currentCommunicationPoint=t, communicationStepSize=DT)
        t += DT

        rows.append((round(t, 6), t_out, q, u_in, t_set if t_set is not None else ""))
        if (n + 1) % 200 == 0:
            print(f"  t = {t:6.2f} s   t_out = {t_out:8.3f} K   q_wall = {q:8.1f} W/m2"
                  f"   u_in = {u_in:.2f} m/s")

    fmu.terminate()
    fmu.freeInstance()
    shutil.rmtree(unzipdir, ignore_errors=True)

    os.makedirs(os.path.dirname(out), exist_ok=True)
    with open(out, "w", newline="") as fh:
        w = csv.writer(fh)
        w.writerow(["t", "t_out", "q_wall", "u_in", "t_set"])
        w.writerows(rows)
    print(f"done in {time.time() - wall0:.0f} s, wrote {out}")


if __name__ == "__main__":
    sys.exit(main())
