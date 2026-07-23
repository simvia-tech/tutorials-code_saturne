# Contributing to the code_saturne tutorials

Thank you for considering a contribution. This repository is a collection of
small, self-contained and reproducible **code_saturne** tutorials. Each tutorial
pairs a pre-configured case with a README that explains the physics, the setup
and (where applicable) the validation against reference data.

Contributions are welcome: a brand-new tutorial, a fix to an existing case, a
clearer explanation, a better figure, or a correction to the catalog metadata.

## Guiding principles

Please keep these in mind before opening a pull request:

- **One feature per tutorial.** Each case should showcase a single code_saturne
  capability (a physical model, a boundary-condition type, a numerical option, a
  meshing feature). Keep the geometry and the setup as simple as the feature
  allows.
- **Reproducible.** Anyone who clones the repo and runs the case with the stated
  code_saturne version must obtain the results shown in the README. State the
  version you used.
- **Honest about results.** If the solution differs from a reference, say so and
  explain the offset (mesh resolution, a model constant, a known limitation).
  Do not hide or cosmetically fix discrepancies.
- **Self-contained.** No external download step, no helper scripts to ship, no
  invented mesh provenance. Cite the source of any reference data.
- **code_saturne only.** Do not reference other solvers in the prose.

## Prerequisites

- **code_saturne v9.1** (a few cases target a different version; state it in the
  README and in the catalog entry).
- **Git LFS** for meshes and figures: run `git lfs install` before cloning or
  committing.
- **Python 3** with the site dependencies if you want to preview the website:
  `pip install -r catalog/requirements.txt`.

## Repository layout

Tutorials are grouped by topic in numbered top-level directories. A tutorial is
one case directory inside a topic:

```
<NN_topic>/<Case_Name>/
├── CASE/
│   ├── DATA/setup.xml   # GUI-authored configuration (the source of truth)
│   ├── DATA/run.cfg     # run parameters for `code_saturne run`
│   └── SRC/*.cpp         # optional cs_user_*.cpp user routines
├── MESH/                # mesh files, tracked with Git LFS (omit if generated internally)
├── FIGURES/             # PNGs embedded in the README
└── README.md            # the tutorial itself
```

Current topics:

```
00_foundations            30_compressible           70_lagrangian
05_preprocessing          40_turbomachinery         80_combustion_electric
10_turbulence_rans        50_vof
20_thermal                60_atmospheric_environmental
```

Note: `RESU/` (run outputs) and any `validation/` scripts are intentionally
**git-ignored** and must not be committed.

## Adding a new tutorial

### 1. Choose a topic and a case name

Place the case under the most relevant `NN_topic/` directory. Name the case
directory with a short module-style prefix and a PascalCase description,
matching the closest existing sibling, for example:

```
Inc_*   incompressible/base flow      Comp_*  compressible
Th_*    thermal                       Vof_*   volume of fluid
Cogz_*  gas combustion                Elec_*  electric arc / Joule
Tbm_*   turbomachinery                Gwf_*   groundwater flow
Lag_*   Lagrangian particles          Pre_*   preprocessing / meshing
```

### 2. Build the case

- Configure the case from the GUI so that `CASE/DATA/setup.xml` is the source of
  truth: `code_saturne gui <Case_Name>/CASE/DATA/setup.xml &`.
- Keep any user routines in `CASE/SRC/*.cpp` minimal and mirror the idioms of the
  existing ones. They are compiled automatically by `code_saturne run`; this is
  not a standalone C++ project.
- Prefer the built-in Cartesian mesher when the geometry allows, so no mesh file
  needs to be shipped. Otherwise put the mesh under `MESH/` (tracked by LFS).
- Verify the case runs cleanly from a fresh clone:
  `cd <Case_Name>/CASE && code_saturne run`.

### 3. Write the README

Follow the canonical section order used by every shipped tutorial:

1. Title
2. Learning objectives
3. Prerequisites
4. Case files
5. Physical model (with a Flow parameters subsection when relevant)
6. Geometry and boundary conditions
7. Numerical setup
8. Running the simulation (GUI and command line)
9. Results and verification (figures pulled from `FIGURES/`)
10. Summary
11. References
12. Authors

Guidelines for the prose:

- Open the results with an annotated geometry + mesh view taken from the real run
  mesh.
- Use absolute physical values and cite the source field in `setup.xml`.
- Be explicit about the reference data source and any offset from it.

### 4. Register the tutorial in the catalog

Every case with a README **must** have an entry in `catalog/tutorials.yaml`
(the site is generated from it, and the validator refuses missing or extra
entries). Add a block like:

```yaml
- path: 20_thermal/Th_My_New_Case
  title: My New Case (short parenthetical clarifier optional)
  topic: Thermal & Radiation
  module: base
  regime: laminar
  physics: [buoyancy]
  cs_version: "9.1"
  difficulty: beginner
  tags: [natural-convection, my-keyword]
```

Use only the controlled vocabulary documented at the top of `tutorials.yaml`:

- **topic**: one of the ten display topics (must match a topic directory).
- **module**: `base turb les cfbl vof cogz ctwr atmo gwf elec lagr rayt cdo meshing`
- **regime**: `laminar RANS LES inviscid compressible multiphase reactive porous n/a`
- **physics** (may be empty `[]`): `heat-transfer buoyancy radiation combustion
  cavitation surface-tension phase-change evaporation coriolis rotation
  groundwater species-transport shock-waves electric-joule humid-air
  particle-transport porous-media`
- **cs_version**: `"9.1"` or `"9.0.0"`
- **difficulty**: `beginner intermediate advanced`
- **tags**: free-form keywords (at least one).

The parenthetical part of a `title` is stripped for display on the site, so use
it for a clarifier, not for essential words.

### 5. Validate and preview locally

```bash
# validate the catalog: controlled vocab + full coverage (no missing/extra cases)
python catalog/build_catalog.py

# build and preview the website (served from the READMEs and the catalog)
python catalog/build_site.py _site_src
mkdocs serve -f _site_src/mkdocs.yml
```

`build_catalog.py` must exit `OK`, and the site must build. The CI runs
`mkdocs build --strict`, so make sure your README has no broken links or
references to missing figures.

## Meshes and Git LFS

- Everything under `MESH/` and all mesh formats (`.msh`, `.med`, `.cgns`, `.unv`,
  `.hdf`, ...) are tracked with **Git LFS**. Install it before committing.
- Keep meshes as small as the tutorial allows; prefer the built-in mesher when
  possible to avoid shipping a file at all.
- CGNS files in ADF format must be converted to HDF5 with `cgnsconvert`.
- CGNS group names that contain spaces must be **quoted** in code_saturne
  selection criteria.

## Commit and pull-request workflow

1. Fork the repository and create a topic branch from `main`.
2. Use clear, conventional commit messages: `feat: ...`, `fix: ...`,
   `docs: ...`, `refactor: ...`.
3. Do not commit `RESU/` outputs or local validation scripts (they are
   git-ignored).
4. Open a pull request against `main` with a short description of the tutorial or
   the fix, and confirm the checklist below.

### Pull-request checklist

- [ ] The case runs cleanly from a fresh clone with the stated code_saturne version.
- [ ] `README.md` follows the canonical section order and embeds its figures.
- [ ] Results are honest, with any discrepancy explained.
- [ ] A catalog entry was added/updated in `catalog/tutorials.yaml`.
- [ ] `python catalog/build_catalog.py` exits `OK`.
- [ ] The site builds (`mkdocs build --strict`).
- [ ] Meshes (if any) are tracked with Git LFS; no `RESU/` or scripts committed.

## License

By contributing, you agree that your contribution is distributed under the
**GNU General Public License v2.0** (see [LICENSE](LICENSE)), the same license
as code_saturne.

## Contact

For questions or feedback, reach out to us:

Email: maxime.gueguin@simvia.tech or florian.hermet@simvia.tech.
