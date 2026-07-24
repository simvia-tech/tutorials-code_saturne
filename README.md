# code_saturne tutorials

A collection of step-by-step CFD tutorials for
[code_saturne](https://code-saturne.org/), maintained by
[Simvia](https://simvia.tech/fr).

**Website:** https://simvia-tech.github.io/tutorials-code_saturne/

> This collection is a work in progress. The aim is for each tutorial to
> showcase a specific code_saturne capability, as a small, self-contained and
> reproducible case with a physics write-up. Some cases are validated against
> reference data, others are demonstrations.

## What you get

Each tutorial is a self-contained code_saturne case plus a README that explains
the physics, the setup and (where applicable) the validation. Tutorials are
grouped by topic in numbered directories:

```
<NN_topic>/<Case_Name>/
├── CASE/
│   ├── DATA/setup.xml   # GUI-authored configuration
│   └── SRC/*.cpp        # optional cs_user_*.cpp user routines
├── MESH/                # mesh files (Git LFS), when not generated internally
├── FIGURES/             # figures used in the README
└── README.md            # the tutorial itself
```

Topics: foundations, preprocessing, turbulence (RANS/LES), thermal & radiation,
compressible, turbomachinery, multiphase (VOF), atmospheric & environmental,
particles & dispersion, reactive & electric.

## Requirements

- **code_saturne v9.1** (a few cases note a different version in their README).
- **Git LFS** for the mesh files: `git lfs install` before cloning.

If code_saturne is not installed, build it from the
[official site](https://code-saturne.org/) or download a ready-to-use image from
the [Open Simulation Center](https://open-simulation-center.org/fr/downloads/code_saturne/code_saturne).

## Running a case

```bash
cd <NN_topic>/<Case_Name>/CASE
code_saturne run                 # serial
code_saturne run --n 8      # parallel
```

Each run writes a timestamped `CASE/RESU/<id>/` with `run_solver.log` and the
post-processing output.

## The website

The website is built with [MkDocs Material](https://squidfunk.github.io/mkdocs-material/)
directly from the tutorial READMEs, and deployed to GitHub Pages by
`.github/workflows/pages.yml`. The navigation and pages are generated from the
folder structure and `catalog/tutorials.yaml`, so adding a tutorial requires no
manual page edits.

To preview locally:

```bash
pip install -r catalog/requirements.txt
python catalog/build_site.py _site_src
mkdocs serve -f _site_src/mkdocs.yml
```

## License

Distributed under the GNU General Public License v3.0 (see [LICENSE](LICENSE)).

Maintained by [Simvia](https://simvia.tech/fr).
