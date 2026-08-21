#!/usr/bin/env python3
"""Build catalog/tutorials.json from catalog/tutorials.yaml.

- validates every field against the controlled vocabulary,
- checks that the manifest covers exactly the tutorials on disk (no missing/extra),
- emits tutorials.json (consumed by the GitHub Pages faceted catalog),
- prints a per-facet summary for review.
"""
import os, re, json, sys, glob

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)

VOCAB = {
    "topic": {"Foundations","Preprocessing","Turbulence","Thermal & Radiation",
              "Compressible","Turbomachinery","Multiphase (VOF)",
              "Atmospheric & Environmental","Particles & Dispersion","Reactive & Electric",
              "Mesh Methods (IBM / ALE / AMR)","Campaigns & Automation"},
    "module": {"base","turb","les","cfbl","vof","cogz","ctwr","atmo","gwf","elec","lagr","rayt","cdo","meshing"},
    "regime": {"laminar","RANS","LES","inviscid","compressible","multiphase","reactive","porous","n/a"},
    "physics": {"heat-transfer","buoyancy","radiation","combustion","cavitation","surface-tension",
                "phase-change","evaporation","coriolis","rotation","groundwater","species-transport",
                "shock-waves","electric-joule","humid-air","particle-transport","porous-media"},
    "cs_version": {"9.2","9.1","9.0.0"},
    "difficulty": {"beginner","intermediate","advanced"},
}

def load_yaml(path):
    try:
        import yaml
        with open(path) as f:
            return yaml.safe_load(f)["tutorials"]
    except ImportError:
        sys.exit("PyYAML not available; run with a python that has pyyaml, or `pip install pyyaml`.")

def main():
    entries = load_yaml(os.path.join(HERE, "tutorials.yaml"))
    errors = []

    # coverage: every case dir with a README must be in the manifest and vice-versa
    on_disk = set()
    for topic in os.listdir(ROOT):
        if not re.match(r"\d\d_", topic): continue
        for case in os.listdir(os.path.join(ROOT, topic)):
            if os.path.exists(os.path.join(ROOT, topic, case, "README.md")):
                on_disk.add(f"{topic}/{case}")
    in_manifest = {e["path"] for e in entries}
    for p in sorted(on_disk - in_manifest): errors.append(f"MISSING from manifest: {p}")
    for p in sorted(in_manifest - on_disk): errors.append(f"NOT ON DISK: {p}")

    # field validation
    for e in entries:
        for k in ("topic","module","regime","cs_version","difficulty"):
            if e.get(k) not in VOCAB[k]:
                errors.append(f"{e['path']}: bad {k}={e.get(k)!r}")
        for ph in e.get("physics",[]):
            if ph not in VOCAB["physics"]:
                errors.append(f"{e['path']}: bad physics {ph!r}")
        if not e.get("tags"): errors.append(f"{e['path']}: no tags")

    if errors:
        print("VALIDATION ERRORS:"); [print("  -",x) for x in errors]; sys.exit(1)

    # emit json (add a stable id = last path component)
    for e in entries: e["id"] = e["path"].split("/")[-1]
    with open(os.path.join(HERE, "tutorials.json"), "w") as f:
        json.dump({"tutorials": entries}, f, indent=2, ensure_ascii=False)

    # summary
    from collections import Counter
    print(f"OK: {len(entries)} tutorials, coverage complete, all fields valid.\n")
    for facet in ("topic","module","regime","difficulty"):
        c = Counter(e[facet] for e in entries)
        print(f"{facet}:")
        for k,n in sorted(c.items(), key=lambda x:-x[1]): print(f"   {n:2d}  {k}")
    phys = Counter(p for e in entries for p in e["physics"])
    print("physics (coupled phenomena):")
    for k,n in sorted(phys.items(), key=lambda x:-x[1]): print(f"   {n:2d}  {k}")
    print(f"   {sum(1 for e in entries if not e['physics']):2d}  (none / pure flow)")

if __name__ == "__main__":
    main()
