# Steady Conjugate Heat Transfer 

This tutorial solves a steady conjugate heat-transfer (CHT) problem with **code_saturne**. A laminar incompressible flow passes over three hollow solid cylinders aligned with the streamwise direction. Each cylinder is heated from its inner core, heat is conducted through the solid annulus, and the resulting thermal flux is transferred to the surrounding fluid through internally coupled fluid-solid interfaces.

The supplied setup is configured for **code_saturne 9.1**. The calculation uses a pseudo-steady local time-stepping procedure to converge the direct flow and temperature solution. We consider a **direct CHT problem** here.

Maintained by [Simvia](https://Simvia.tech/fr), part of the
[tutoriel-code_saturne](https://github.com/simvia-tech/tutorials-code_saturne) collection.

## Learning objectives

After completing this tutorial, the user should be able to:

1. define separate fluid and solid volume zones in a single code_saturne mesh;
2. activate the temperature equation in both fluid and solid regions;
3. configure internal fluid-solid coupling for the thermal scalar;
4. prescribe different thermal conductivities in fluid and solid zones;
5. impose a fixed temperature on the inner cores of hollow solids;
6. solve a steady laminar incompressible flow with heat transfer;
7. use a temperature-dependent density law in the fluid region;
8. converge the problem with the SIMPLEC pseudo-steady algorithm;
9. visualize temperature conduction through the solids and convection into the fluid;
10. assess the basic physical consistency of a conjugate heat-transfer solution.

## Prerequisites

| Requirement | Detail |
|---|---|
| code_saturne | **v9.1** |
| Background | Basic notions of convective heat transfer |
| Case | [Inc_Turbulent_Plate](../../10_turbulence_rans/Inc_Turbulent_Plate) |

If code_saturne is not yet installed, build it from the
[official homepage](https://code-saturne.org/), pull a
ready-to-use Singularity image from the
[Open Simulation Center](https://open-simulation-center.org/downloads/code_saturne/code_saturne),
or pull the
[Simvia Docker image](https://hub.docker.com/r/Simvia/code_saturne) before continuing.

## Case files

```text
Th_Conjugate_Heat_Transfer/
├── CASE/
│   └── DATA/
│       ├── run.cfg
│       └── setup.xml
├── MESH/
│   └── cht_3cylinders_code_saturne_split_farfield.msh
├── FIGURES/
│   ├── cht_mesh_boundary_conditions.png
│   └── cht_conjugate_temperature.png
└── README.md
```

## Physical model

### Fluid region

The fluid is modeled as a steady, laminar, incompressible flow with variable density and heat transfer. Turbulence, gravity, radiation, combustion, and compressibility models are disabled.

The steady mass-conservation equation is

$$
\nabla\cdot(\rho\mathbf{u})=0.
$$

The momentum equation may be written as

$$
\nabla\cdot(\rho\mathbf{u}\otimes\mathbf{u})
=-\nabla p
+\nabla\cdot\left[\mu\left(\nabla\mathbf{u}+\nabla\mathbf{u}^{T}\right)\right].
$$

The temperature field in the fluid is governed by the steady advection-diffusion equation

$$
\nabla\cdot(\rho c_p\mathbf{u}T)
=\nabla\cdot(k_f\nabla T),
$$

where $k_f$ is the fluid thermal conductivity.

The fluid density follows the temperature-dependent law used in `setup.xml`:

$$
\rho(T)=\rho_{\mathrm{ref}}\frac{T_{\mathrm{ref}}}{T},
$$

with

$$
\rho_{\mathrm{ref}}=2.10322\times10^{-4}\ \mathrm{kg\,m^{-3}},
\qquad
T_{\mathrm{ref}}=288.15\ \mathrm{K}.
$$

This density scaling is part of the benchmark definition and is chosen together with the viscosity, velocity, and cylinder diameter to obtain the target Reynolds number. It should not be interpreted as standard atmospheric air density.

### Solid regions

The three cylinders are defined as solid volume zones named `solid1`, `solid2`, and `solid3`. No momentum equation is solved in the solids. In the steady case, heat conduction is governed by

$$
\nabla\cdot(k_s\nabla T)=0,
$$

where $k_s$ is the solid thermal conductivity.

The inner surface of each hollow cylinder is maintained at

$$
T_{\mathrm{core}}=350\ \mathrm{K}.
$$

Heat therefore conducts radially through each solid annulus before being transferred to the surrounding fluid.

### Internal conjugate coupling

The thermal scalar `temperature` is activated for internal coupling, and the three solid volume zones are registered as coupled solid zones. code_saturne automatically constructs the internal fluid-solid interfaces from these volume definitions.

At each coupled interface, the solution must satisfy temperature continuity and conservation of normal heat flux:

$$
T_f=T_s,
$$

$$
k_f\frac{\partial T_f}{\partial n}
=k_s\frac{\partial T_s}{\partial n}.
$$

The interface temperature is therefore not prescribed. It is obtained as part of the coupled solution from the balance between conduction in the solid and convection-diffusion in the fluid.

## Flow parameters

The inlet and material properties used by the supplied `setup.xml` are summarized below.

### Fluid properties

| Parameter | Value | Unit |
|---|---:|---|
| Inlet velocity magnitude $U_\infty$ | 3.40297 | $\mathrm{m\,s^{-1}}$ |
| Flow direction | $(1,0,0)$ | - |
| Inlet temperature $T_\infty$ | 288.15 | K |
| Reference density $\rho_{\mathrm{ref}}$ | $2.10322\times10^{-4}$ | $\mathrm{kg\,m^{-3}}$ |
| Dynamic viscosity $\mu$ | $1.7893\times10^{-5}$ | $\mathrm{Pa\,s}$ |
| Specific heat $c_p$ | 1004.703 | $\mathrm{J\,kg^{-1}\,K^{-1}}$ |
| Thermal conductivity $k_f$ | 0.024968265 | $\mathrm{W\,m^{-1}\,K^{-1}}$ |
| Reference pressure | 101325 | Pa |

Using the outer cylinder diameter $D=1\ \mathrm{m}$, the inlet Reynolds number is

$$
Re_D=\frac{\rho_{\mathrm{ref}}U_\infty D}{\mu}\approx40.
$$

The corresponding Prandtl number is

$$
Pr=\frac{c_p\mu}{k_f}\approx0.72.
$$

The low Reynolds number supports the laminar-flow assumption used in this tutorial.

### Solid properties

All three solid zones use the same material properties:

| Parameter | Value | Unit |
|---|---:|---|
| Density $\rho_s$ | $2.10322\times10^{-4}$ | $\mathrm{kg\,m^{-3}}$ |
| Specific heat $c_{p,s}$ | 1004.703 | $\mathrm{J\,kg^{-1}\,K^{-1}}$ |
| Thermal conductivity $k_s$ | 0.1028 | $\mathrm{W\,m^{-1}\,K^{-1}}$ |
| Inner-core temperature | 350 | K |

The solid conductivity is approximately $4.12$ times the fluid conductivity:

$$
\frac{k_s}{k_f}\approx4.12.
$$

For the present steady problem, the thermal conductivity directly controls the conductive temperature gradient through the solid annuli.

## Geometry and boundary conditions

### Geometry

The domain contains three identical hollow cylinders arranged in-line along the $x$ direction.

| Quantity | Value |
|---|---:|
| Far-field circle center | $(5.5,0)$ m |
| Far-field radius | 30.5 m |
| Core 1 center | $(0.5,0)$ m |
| Core 2 center | $(5.5,0)$ m |
| Core 3 center | $(10.5,0)$ m |
| Outer core diameter | 1.0 m |
| Inner-core diameter | 0.5 m |
| Extruded thickness | 1.0 m |

The cylinders therefore have an outer radius of $0.5\ \mathrm{m}$ and an inner radius of $0.25\ \mathrm{m}$. The front and back planes are placed at $z=-0.5\ \mathrm{m}$ and $z=0.5\ \mathrm{m}$ and are treated as symmetry boundaries, making the calculation quasi-two-dimensional.

### Mesh

The mesh contains four volume zones:

- `fluid`: 33,700 cells;
- `solid1`: 4,534 cells;
- `solid2`: 4,534 cells;
- `solid3`: 4,534 cells.

The complete mesh therefore contains **47,302 volume cells** and **93,248 vertices**. The one-layer extrusion contains 45,750 hexahedra and 1,552 triangular prisms. The mesh is strongly refined around the cylinders and their fluid-solid interfaces, while cell sizes increase gradually toward the far-field boundary.

<p align="center">
  <img src="FIGURES/cht_mesh_boundary_conditions.png"
       alt="2D mesh and boundary conditions."
       width="900"/>
  <br>
  <em>Figure 1: Mesh and boundary conditions (z = 0m slice).</em>
</p>

The external circular boundary is split into an upstream inlet half and a downstream outlet half. The green interfaces shown in the figure correspond to the internally coupled outer surfaces of the three solid cylinders. The magenta inner circles are the heated cores.

### Boundary conditions

| Boundary | Type | Thermal condition | Flow condition |
|---|---|---|---|
| `inlet` | Inlet | $T=288.15\ \mathrm{K}$ | $\mathbf{u}=(3.40297,0,0)\ \mathrm{m\,s^{-1}}$ |
| `outlet` | Outlet | Zero normal temperature gradient | Standard pressure outlet |
| `core1` | Wall | $T=350\ \mathrm{K}$ | Solid inner boundary |
| `core2` | Wall | $T=350\ \mathrm{K}$ | Solid inner boundary |
| `core3` | Wall | $T=350\ \mathrm{K}$ | Solid inner boundary |
| `front` | Symmetry | Symmetry | Symmetry |
| `back` | Symmetry | Symmetry | Symmetry |
| Fluid-solid interfaces | Internal coupling | Temperature and heat-flux continuity | Automatically generated from the solid zones |

The outer cylinder surfaces do not require prescribed wall temperatures. Their interface temperatures are determined by the conjugate heat-transfer solution.

## Numerical setup

The principal numerical settings in `CASE/DATA/setup.xml` are:

| Setting | Value |
|---|---:|
| Flow regime | Laminar |
| Fluid-solid coupling | Internal coupling of `temperature` |
| Solid zones | `solid1`, `solid2`, `solid3` |
| Velocity-pressure algorithm | SIMPLEC |
| Time treatment | Pseudo-steady local time step |
| Number of iterations | 1500 |
| Reference pseudo-time step | 0.1 s |
| Target maximum Courant number | 1 |
| Target maximum Fourier number | 10 |
| Minimum local-step factor | 0.1 |
| Maximum local-step factor | 1000 |
| Gravity | $(0,0,0)$ |

The pseudo-time used by the steady local-time-step algorithm is a numerical convergence device. Intermediate iteration times should therefore **not** be interpreted as physical transient time.

### Initialization

The fluid region is initialized with

$$
\mathbf{u}=(3.40297,0,0)\ \mathrm{m\,s^{-1}},
\qquad
T=288.15\ \mathrm{K}.
$$

The three solid regions are initialized at

$$
T=350\ \mathrm{K}.
$$

Three monitoring probes are defined on the centerline:

| Probe | $x$ (m) | $y$ (m) | $z$ (m) |
|---|---:|---:|---:|
| 1 | 1.5 | 0 | 0 |
| 2 | 5.875 | 0 | 0 |
| 3 | 11.5 | 0 | 0 |

These probes can be used to monitor the convergence of temperature, velocity, and other fields in the wakes of the heated cylinders.

## Running the simulation

The commands below start from the tutorial directory:

```bash
cd Th_Conjugate_Heat_Transfer/
```

### Option A: Graphical interface

```bash
code_saturne gui CASE/DATA/setup.xml &
```

The GUI opens the pre-configured `setup.xml`. Review the setup if you wish,
then launch the run with the **gear (Run) button** in the toolbar. To run in
parallel, set the number of processes under
**Calculation management > Performance settings** before launching.

### Option B: Command line

The command-line launcher is run from inside the case directory:

```bash
cd CASE

# serial
code_saturne run

# parallel (e.g. 4 MPI ranks)
code_saturne run --n 4
```

Each run creates a time-stamped directory `CASE/RESU/<YYYYMMDD-HHMM>/` containing:

- `run_solver.log`: solver log and residual history,
- `monitoring/`: probe time series (`probes_*.csv`),
- `postprocessing/`: volume and boundary fields in EnSight Gold format.

## Results and verification

### Conjugate temperature field

<p align="center">
  <img src="FIGURES/cht_conjugate_temperature.png"
       alt="Conjugate temperature distribution."
       width="900"/>
  <br>
  <em>Figure 2: Conjugate temperature distribution.</em>
</p>

The temperature field shows the expected coupled conduction-convection behavior. Each core supplies heat to its surrounding solid annulus. The heat then crosses the internally coupled outer cylinder surface and is transported downstream by the fluid.

Several characteristic features are visible:

- the hottest region is located around each heated core;
- the temperature decreases through the solid toward each fluid-solid interface;
- a thermal wake develops downstream of every cylinder;
- the wakes are convected in the positive $x$ direction;
- the second and third cylinders interact with fluid that has already been thermally disturbed by upstream cylinders;
- the solution remains symmetric about the centerline, as expected for the geometry and imposed boundary conditions of the steady laminar problem.

### Verification checks

The present tutorial is intended primarily as a reproducible CHT setup rather than a high-accuracy validation campaign. The direct solution can nevertheless be checked using several physical and numerical consistency criteria:

1. The inlet conditions give $Re_D\approx40$, consistent with the laminar model.
2. The material properties give $Pr\approx0.72$ and $k_s/k_f\approx4.12$.
3. Temperature should vary smoothly across each internally coupled fluid-solid interface; an artificial temperature jump would indicate an incorrect coupling definition or post-processing interpretation.
4. The heat-flux direction must be from the $350\ \mathrm{K}$ cores toward the colder incoming fluid.
5. The converged temperature field should be symmetric about $y=0$.
6. The downstream thermal wakes should become progressively influenced by upstream heating as the flow crosses the three-cylinder array.
7. Residual histories and the three centerline probes should approach stationary values before the solution is considered converged.

No grid-convergence study or independent quantitative temperature/heat-flux reference is included in the present case. Verification is therefore based on the benchmark parameters, interface continuity, symmetry, convergence behavior, and the expected structure of the conjugate temperature field.

## Summary and limitations

This tutorial demonstrates a complete direct conjugate heat-transfer calculation in code_saturne using:

- one laminar fluid region and three solid regions in a single mesh;
- the temperature equation in both fluid and solids;
- internal thermal coupling across fluid-solid interfaces;
- temperature-dependent fluid density;
- distinct fluid and solid thermal conductivities;
- fixed-temperature inner cores;
- a pseudo-steady SIMPLEC solution procedure;
- a quasi-two-dimensional mesh with symmetry front and back planes.

The case provides a compact example of how conduction inside solids and convection in a surrounding fluid can be solved simultaneously without prescribing the unknown outer cylinder temperatures.

The following limitations should be kept in mind:

- the calculation is quasi-two-dimensional and excludes spanwise effects;
- the flow is laminar and fixed at approximately $Re_D=40$;
- gravity and thermal radiation are neglected;
- material properties are constant except for the prescribed fluid density law;
- the very low reference density is a benchmark scaling choice and is not representative of atmospheric air;
- the inner cores are maintained at a fixed temperature rather than heated by a volumetric source or imposed heat flux;
- only the direct steady solution is considered;
- no mesh-independence study is supplied;
- the current verification is mainly physical and qualitative rather than a quantitative validation against independent measurements.

The case is therefore best suited as an introductory internal-CHT tutorial and as a starting point for more detailed studies of interface heat flux, mesh sensitivity, alternative material properties, or higher-Reynolds-number thermal flows.

## References

1. code_saturne documentation, *Conjugate heat transfer — Internal Fluid-Thermal coupling*: <https://code-saturne.org/doc/code_saturne-9.0/advanced_conjugate_heat_transfer.html>.
2. code_saturne project documentation and user resources: <https://www.code-saturne.org/>.
3. S. V. Patankar, *Numerical Heat Transfer and Fluid Flow*, Hemisphere Publishing Corporation, 1980.
4. F. P. Incropera, D. P. DeWitt, T. L. Bergman, and A. S. Lavine, *Fundamentals of Heat and Mass Transfer*, Wiley.
