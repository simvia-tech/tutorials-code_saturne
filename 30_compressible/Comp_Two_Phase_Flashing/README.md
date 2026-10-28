# Depressurization and Flashing of Hot Water (Relaxation Time Scale)

A tutorial on the **mass transfer closure** of the **compressible homogeneous two-phase model** of **code_saturne**: hot pressurized water is suddenly put in contact with low-pressure steam, the rarefaction wave brings the liquid below its saturation pressure, and the liquid **flashes**. The same case is run with four relaxation time scales $\tau$, from the frozen mixture to the instantaneous equilibrium, with the user routine that provides $\tau$ to the code.

This is the second tutorial on this model. Its setup is the one of [Comp_Two_Phase_Shock_Tube](../Comp_Two_Phase_Shock_Tube), which should be read first.

Maintained by [Simvia](https://Simvia.tech/fr), part of the
[tutoriel-code_saturne](https://github.com/simvia-tech/tutorials-code_saturne) collection.

## Learning objectives

After completing this tutorial you will be able to:

1. Write the user routine `cs_user_hgn_thermo_relax_time` that provides the **relaxation time scale**, and drive it from a **notebook variable**.
2. Recognize the three regimes of the model on a flashing flow: **frozen mixture**, **metastable liquid** with a finite $\tau$, **instantaneous equilibrium**.
3. Initialize a **mixture at given pressure, temperature and void fraction**, with a spread interface.
4. Verify the **exponential integration** of the relaxation step against its exact solution.

## Prerequisites

| Requirement | Detail |
|---|---|
| code_saturne | **v9.1** |
| Case | [Comp_Two_Phase_Shock_Tube](../Comp_Two_Phase_Shock_Tube) (model activation, EOS, limitations of v9.1) |
| Background | Saturation curve, metastable states, flashing |

If code_saturne is not yet installed, build it from the
[official homepage](https://code-saturne.org/), pull a
ready-to-use Singularity image from the
[Open Simulation Center](https://open-simulation-center.org/downloads/code_saturne/code_saturne),
or pull the
[Simvia Docker image](https://hub.docker.com/r/Simvia/code_saturne) before continuing.

## Case files

```
Comp_Two_Phase_Flashing/
├── CASE/
│   ├── DATA/
│   │   └── setup.xml                      # mesh, time stepping, notebook, probes, 1D profile
│   └── SRC/
│       ├── cs_user_parameters.cpp         # model activation, EOS, stiffened gas parameters
│       ├── cs_user_initialization.cpp     # left and right mixtures, smoothed interface
│       ├── cs_user_hgn.cpp                # relaxation time scale (notebook variable tau)
│       └── cs_user_extra_operations.cpp   # return-to-equilibrium step (notebook switch)
├── FIGURES/
└── README.md
```

> No MESH directory. All the physical parameters of the case (states, interface width, relaxation switch and time scale) are **notebook variables** of `setup.xml`: the runs only differ by their command line.

## Physical model

The model and its equations of state are those of the [two-phase shock tube](../Comp_Two_Phase_Shock_Tube#physical-model). The steam fractions $(\alpha, y, z)$ relax towards their equilibrium values with the time scale $\tau$. After the convection step, code_saturne integrates the relaxation exactly on each cell, at constant $\rho$, $u$ and $E$:

$$
\phi^{n+1} = e^{-\Delta t/\tau}\,\phi^{n} + \left(1 - e^{-\Delta t/\tau}\right)\phi_{eq},
\qquad \phi \in \{\alpha, y, z\}.
$$

The time scale is the closure of the model. It is returned cell by cell by `cs_user_hgn_thermo_relax_time`, which receives the equilibrium fractions, the internal energy and the specific volume, so that a closure law may depend on the local state. Here a **uniform value** is imposed from the notebook variable `tau`; `tau = 0` keeps the code default, $10^{-30}$ s, which is an instantaneous return to equilibrium.

### The depressurization tube

Each side is a mixture at a common pressure and temperature with a given steam volume fraction; mass fraction, specific volume and energy follow from the phasic laws ($y = (\alpha/\tau_1) / (\alpha/\tau_1 + (1-\alpha)/\tau_2)$, $\tau = y\tau_1 + (1-y)\tau_2$, $e = y e_1 + (1-y) e_2$, $z = y e_1/e$).

| State | $p$ [bar] | $T$ [K] | $T_{sat}(p)$ [K] | $\alpha$ | $y$ | $\rho$ [kg/m³] | $c$ [m/s] |
|---|---|---|---|---|---|---|---|
| Left ($x < 0.5$ m), liquid-rich | $70$ | $555$ | $557.3$ | $0.01$ | $4.55 \times 10^{-4}$ | $729.9$ | $925$ |
| Right ($x > 0.5$ m), steam-rich | $10$ | $430$ | $418.7$ | $0.99$ | $0.393$ | $15.4$ | $699$ |

When the interface is released, a rarefaction travels into the liquid at about $925$ m/s and brings its pressure down towards $10$ bar, far below the saturation pressure of $555$ K water ($68$ bar): the liquid becomes strongly superheated and must vaporize. How fast it does so is what $\tau$ controls.

## Geometry and boundary conditions

Thin box of $3 \times 0.01 \times 1$ m, $3000 \times 1 \times 1$ cells, **symmetry planes** on the six faces (closed domain, see the shock tube tutorial). The interface at $x = 0.5$ m is **spread over 2 cm** by a hyperbolic tangent on $(p, T, \alpha)$, about twenty cells.

> **A sharp interface between (nearly) pure phases cannot be computed.** With a sharp jump, the run stops within a few time steps with a negative pressure, even with a ten times smaller time step: the fractions, transported with the mass flux, and the density, updated by the pressure equation, become inconsistent in the interface cell, and the stiffened gas of the liquid ($\Pi = 3.7 \times 10^{8}$ Pa) turns this inconsistency into a negative pressure. Five cells of spreading are not enough; twenty are.

## Numerical setup

| Setting | Value |
|---|---|
| Model | as in the shock tube tutorial (`CS_COMPRESSIBLE = 2`, two-phase EOS, 1st-order upwind) |
| Time step | fixed, $2.5 \times 10^{-7}$ s, $2000$ iterations, $t_{end} = 5 \times 10^{-4}$ s, CFL $\approx 0.23$ |
| States and interface | notebook: `p_left`, `t_left`, `alpha_left`, `p_right`, `t_right`, `alpha_right`, `xw` |
| Return to equilibrium | notebook `relax` ($0$/$1$) and `tau` (s), applied from `cs_user_extra_operations` |
| Outputs | 1D profile at $t_{end}$, probes at $x = 0.3$ and $0.5$ m every time step |

## Running the simulation

```bash
cd Comp_Two_Phase_Flashing/CASE

code_saturne run --id frozen                                       # frozen mixture
code_saturne run --notebook-args relax=1 tau=1e-3 --id tau1e-3     # finite time scales
code_saturne run --notebook-args relax=1 tau=1e-4 --id tau1e-4
code_saturne run --notebook-args relax=1 --id equilibrium          # instantaneous equilibrium

# verification of the relaxation step on a uniform superheated mixture
code_saturne run --notebook-args relax=1 p_left=3e6 p_right=3e6 t_left=555 t_right=555 alpha_left=0.01 alpha_right=0.01 xw=0 --id zero_d_eq
code_saturne run --notebook-args relax=1 tau=1e-4 p_left=3e6 p_right=3e6 t_left=555 t_right=555 alpha_left=0.01 alpha_right=0.01 xw=0 --id zero_d_tau
```

Notebook values are passed as space-separated `key=value` pairs; they can also be edited in the GUI (**Physical properties > Notebook**). A run takes about $25$ s in serial and writes `profiles/tube.csv` and `monitoring/probes_<variable>.csv`.

## Results and verification

### Verification of the relaxation step

The last two runs start from a uniform mixture at $30$ bar and $555$ K, a liquid superheated by $62$ K. Nothing moves, so the fractions must follow $y(t) = y_{eq} + (y_0 - y_{eq})\,e^{-t/\tau}$, with $y_{eq}$ given by the instantaneous run.

<p align="center">
  <img src="FIGURES/flashing_zero_d.png"
       alt="Steam mass fraction versus time on a uniform superheated mixture: code_saturne with tau = 1e-4 s against the exact exponential relaxation."
       width="640"/>
  <br>
  <em>Figure 1: Mass fraction at the probe, code_saturne with $\tau = 10^{-4}$ s (dots) against the exact exponential law (dashed).</em>
</p>

The computed mass fraction follows the exact law to $10^{-8}$ of the jump $y_0 - y_{eq}$: the update is exact and the value of $\tau$ returned by the routine is the one used.

### The four regimes of the depressurization

<p align="center">
  <img src="FIGURES/flashing_profiles.png"
       alt="Pressure, temperature, steam fractions, density and velocity at t = 5e-4 s for the four relaxation time scales."
       width="900"/>
  <br>
  <em>Figure 2: Profiles at $t = 5 \times 10^{-4}$ s for the four relaxation time scales.</em>
</p>

<p align="center">
  <img src="FIGURES/flashing_history.png"
       alt="Pressure and steam mass fraction versus time at the probe located in the liquid 0.2 m from the interface."
       width="900"/>
  <br>
  <em>Figure 3: Pressure and steam mass fraction at the probe in the liquid, $x = 0.3$ m.</em>
</p>

| Run | $p$ at $x = 0.3$ m [MPa] | $\alpha$ at $x = 0.3$ m | mean superheat of two-phase cells [K] |
|---|---|---|---|
| Frozen mixture | $1.07$ | $0.0100$ | $64$ |
| $\tau = 10^{-3}$ s | $1.64$ | $0.0124$ | $48$ |
| $\tau = 10^{-4}$ s | $4.01$ | $0.0213$ | $20$ |
| Instantaneous equilibrium | $6.79$ | $0.0096$ | $< 10^{-3}$ |

- **Frozen mixture.** The rarefaction reaches the probe after $0.2$ ms and drops the liquid to the steam pressure in a few tens of microseconds, with a numerical undershoot to $0.3$ MPa. The liquid is then superheated by about $130$ K and nothing happens to it.
- **$\tau = 10^{-3}$ s.** The liquid is still depressurized by the fast wave, but the slow vaporization raises the pressure back to $1.6$ MPa: undershoot then recovery, the typical behavior of a relaxation model with delayed nucleation.
- **$\tau = 10^{-4}$ s.** Vaporization acts during the depressurization itself: the pressure falls along a ramp instead of a step, the superheat is divided by three, the steam fraction in the liquid doubles.
- **Instantaneous equilibrium.** The liquid column stays at the saturation pressure of $555$ K water, $6.794$ MPa, with $T = T_{sat}(p)$ to $10^{-3}$ K on every two-phase cell. The expansion is confined to an **evaporation front** advancing into the liquid at about $100$ m/s, far slower than the frozen rarefaction, which expels the steam at up to $130$ m/s.

Mass and total energy are conserved by the four runs ($4 \times 10^{-7}$ and $1.2 \times 10^{-6}$ relative). The thermodynamics is only representative of water around $80$ bar, and the spread interface fixes the initial thickness of the evaporation front.

## Summary

The relaxation time scale of the model is provided by `cs_user_hgn_thermo_relax_time`, driven here by a notebook variable. On a uniform superheated mixture the relaxation follows its exact exponential law. On the depressurization of hot water, the frozen mixture, two finite time scales and the instantaneous equilibrium give four clearly ordered solutions, from a fully metastable liquid behind a $925$ m/s rarefaction to a slow evaporation front at the saturation pressure. A sharp interface between pure phases is out of reach of the model in v9.1 and must be spread over about twenty cells.

## References

- P. Helluy, O. Hurisse, E. Le Coupanec. Verification of a two-phase flow code based on an homogeneous model, [International Journal on Finite Volumes, 13, 2016](https://hal.science/hal-01396200).
- Z. Bilicki, J. Kestin. Physical aspects of the relaxation model in two-phase flow, [Proceedings of the Royal Society A, 428:379-397, 1990](https://doi.org/10.1098/rspa.1990.0040).
- [code_saturne documentation](https://code-saturne.org/doc/)

## Authors

[Simvia](https://Simvia.tech/fr) - Questions, remarks and requests are welcome.
