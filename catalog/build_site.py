#!/usr/bin/env python3
"""Assemble a MkDocs Material site from the tutorial collection.

Reads catalog/tutorials.yaml (source of truth for order/titles/grouping),
copies each README + its FIGURES into docs/, and generates mkdocs.yml with a
nav grouped by section. Re-run whenever tutorials are added: nothing is
hand-maintained, no hardcoded counts.

Usage: build_site.py [OUT_DIR]   (default: ./_site_src)
"""
import os, sys, shutil, re, base64, json

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
OUT  = os.path.abspath(sys.argv[1]) if len(sys.argv) > 1 else os.path.join(ROOT, "_site_src")

TOPIC_ORDER = ["Foundations","Preprocessing","Turbulence","Thermal & Radiation","Compressible",
               "Turbomachinery","Multiphase (VOF)","Atmospheric & Environmental",
               "Particles & Dispersion","Reactive & Electric"]
GITHUB_URL = "https://github.com/simvia-tech/tutorials-code_saturne"

# GitHub mark (octicons), inlined so the per-tutorial "browse on GitHub" link
# needs no icon extension.
GH_MARK_SVG = ('<svg viewBox="0 0 16 16" aria-hidden="true"><path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 '
               '2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69'
               '-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 '
               '1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15'
               '-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 '
               '1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 '
               '3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.013 '
               '8.013 0 0016 8c0-4.42-3.58-8-8-8z"></path></svg>')

def gh_source_banner(path):
    """A 'browse this case on GitHub' link, injected under each tutorial's H1."""
    url = f"{GITHUB_URL}/tree/main/{path}"
    return (f'<a class="cs-ghsource" href="{url}" target="_blank" rel="noopener">'
            f'{GH_MARK_SVG} Browse this case on GitHub</a>')

def inject_gh_source(md, path):
    """Insert the GitHub source link right after the first H1 heading."""
    banner = gh_source_banner(path)
    out, inserted = [], False
    for ln in md.split("\n"):
        out.append(ln)
        if not inserted and ln.lstrip().startswith("# "):
            out.extend(["", banner, ""])
            inserted = True
    if not inserted:
        out = [banner, ""] + out
    return "\n".join(out)

FOOTER_PARTIAL = r"""<footer class="md-footer">
  <div class="md-footer-meta cs-footer">
    <div class="md-grid cs-footer__inner">
      <div class="cs-footer__mnt">Maintained by
        <a href="https://simvia.tech/fr" target="_blank" rel="noopener"><img src="{{ 'assets/simvia.svg' | url }}" alt="Simvia, groupe EDF"></a>
      </div>
      <nav class="cs-footer__links">
        <a href="https://code-saturne.org/doc/" target="_blank" rel="noopener">Documentation</a>
        <a href="https://code-saturne.org/forum/" target="_blank" rel="noopener">Forum</a>
        <a href="https://open-simulation-center.org/fr/downloads/code_saturne/code_saturne" target="_blank" rel="noopener">Downloads</a>
      </nav>
    </div>
  </div>
</footer>
"""

# main.html: adds Open Graph / Twitter card meta so shared links show the
# code_saturne social card. home.html extends this so the home gets them too.
MAIN_TEMPLATE = r"""{% extends "base.html" %}
{% block extrahead %}
  {% set og_image = config.site_url ~ 'assets/og-card.png' %}
  {% set og_title = config.site_name if (not page or page.is_homepage) else (page.title ~ ' - ' ~ config.site_name) %}
  {% set og_desc = page.meta.description if (page and page.meta.description) else config.site_description %}
  <meta property="og:type" content="website">
  <meta property="og:site_name" content="{{ config.site_name }}">
  <meta property="og:title" content="{{ og_title }}">
  <meta property="og:description" content="{{ og_desc }}">
  <meta property="og:url" content="{{ page.canonical_url }}">
  <meta property="og:image" content="{{ og_image }}">
  <meta property="og:image:width" content="1200">
  <meta property="og:image:height" content="630">
  <meta name="twitter:card" content="summary_large_image">
  <meta name="twitter:title" content="{{ og_title }}">
  <meta name="twitter:description" content="{{ og_desc }}">
  <meta name="twitter:image" content="{{ og_image }}">
{% endblock %}
"""

HOME_TEMPLATE = r"""{% extends "main.html" %}
{% block content %}
{% raw %}
<div class="cs-hero">
  <h1>code<span class="cs-us">_</span>saturne tutorials</h1>
  <p>A growing collection of step-by-step CFD tutorials for
  <a href="https://code-saturne.org/">code_saturne</a>. This site is still under
  construction: the aim is for each tutorial to showcase a specific code_saturne
  capability, as a small reproducible case with a physics write-up. Some are
  validated against reference data, others are demonstrations. Search and filter
  below, or browse by section on the left.</p>

  <div class="cs-search"><div class="cs-search__box">
    <svg class="cs-sic" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="7"/><path d="m21 21-4.3-4.3"/></svg>
    <span id="cs-chips" style="display:contents"></span>
    <input id="cs-q" type="text" placeholder="Search a tutorial, physics or keyword…" autocomplete="off">
    <span class="cs-addf"><button id="cs-addbtn" type="button">&#65291; Filter</button><div class="cs-pop" id="cs-pop" hidden></div></span>
  </div></div>

  <div class="cs-clone">
    <span>Get the collection:</span>
    <code>git clone https://github.com/simvia-tech/tutorials-code_saturne.git</code>
  </div>

  <div class="cs-hero-mnt">Maintained by
    <a href="https://simvia.tech/fr" target="_blank" rel="noopener"><img src="assets/simvia.svg" alt="Simvia, groupe EDF"></a>
  </div>

  <div class="cs-results" id="cs-results" hidden></div>
</div>

<script>
const TUTORIALS=__DATA__;
const TC={"Foundations":"#4a6fa5","Preprocessing":"#7a8699","Turbulence":"#2f8f8a","Thermal & Radiation":"#c1663a","Compressible":"#6b5b95","Turbomachinery":"#3f5bb0","Multiphase (VOF)":"#2b7fb8","Atmospheric & Environmental":"#4f8a4f","Particles & Dispersion":"#8a6d3b","Reactive & Electric":"#b5473f"};
const FACETS=[["module","Module"],["regime","Regime"],["physics","Physics"],["difficulty","Difficulty"],["cs_version","Version"]];
const A={module:new Set(),regime:new Set(),physics:new Set(),difficulty:new Set(),cs_version:new Set()};
let Q="";
const uniq=k=>{const s=new Set();TUTORIALS.forEach(t=>(Array.isArray(t[k])?t[k]:[t[k]]).forEach(v=>v&&s.add(v)));return[...s].sort();};
function match(t){for(const[k]of FACETS){const s=A[k];if(!s.size)continue;const v=t[k];const a=Array.isArray(v)?v:[v];if(!a.some(x=>s.has(x)))return false;}
  if(Q){const h=(t.title+" "+t.tags.join(" ")+" "+t.physics.join(" ")+" "+t.module+" "+t.topic+" "+t.regime).toLowerCase();if(!Q.toLowerCase().split(/\s+/).every(w=>h.includes(w)))return false;}return true;}
const anyA=()=>Q||FACETS.some(([k])=>A[k].size);
function chips(){const c=document.getElementById("cs-chips");c.innerHTML="";for(const[k,l]of FACETS)for(const v of A[k]){const e=document.createElement("span");e.className="cs-fchip";e.innerHTML=`<span class="k">${l}</span><b>${v}</b><button aria-label="x">&times;</button>`;e.querySelector("button").onclick=ev=>{ev.stopPropagation();A[k].delete(v);draw();};c.appendChild(e);}}
function pop(){const p=document.getElementById("cs-pop");p.innerHTML="";for(const[k,l]of FACETS){const g=document.createElement("div");g.className="cs-pg";g.innerHTML=`<h4>${l}</h4>`;const vv=document.createElement("div");vv.className="cs-pv";for(const v of uniq(k)){const b=document.createElement("button");b.type="button";b.className="cs-vchip";b.setAttribute("aria-pressed",A[k].has(v));b.textContent=v;b.onclick=()=>{A[k].has(v)?A[k].delete(v):A[k].add(v);draw();};vv.appendChild(b);}g.appendChild(vv);p.appendChild(g);}}
function draw(){chips();pop();const box=document.getElementById("cs-results");
  if(!anyA()){box.hidden=true;box.innerHTML="";document.getElementById("cs-addbtn").parentNode.querySelector;return;}
  const list=TUTORIALS.filter(match);box.hidden=false;
  box.innerHTML=`<div class="cs-rcount">${list.length} tutorial${list.length!==1?"s":""}</div><div class="cs-grid"></div>`;
  const g=box.querySelector(".cs-grid");
  if(!list.length){g.innerHTML='<p class="cs-none">No tutorial matches. Remove a filter or clear the search.</p>';return;}
  for(const t of list){const a=document.createElement("a");a.className="cs-card";a.href=t.path+"/";
    const ph=t.physics.map(p=>`<span class="ph">${p}</span>`).join("");
    a.innerHTML=`<div class="cr"><span class="tp"><span class="d" style="background:${TC[t.topic]}"></span>${t.topic}</span><span class="pill ${t.difficulty}">${t.difficulty}</span></div><h3>${t.title}</h3><div class="mt"><span class="bd">${t.module}</span><span>${t.regime}</span></div>${ph?`<div class="phs">${ph}</div>`:""}`;
    g.appendChild(a);}
}
document.getElementById("cs-q").addEventListener("input",e=>{Q=e.target.value.trim();draw();});
const P=document.getElementById("cs-pop"),AB=document.getElementById("cs-addbtn");
AB.onclick=e=>{e.stopPropagation();P.hidden=!P.hidden;};
document.addEventListener("click",e=>{if(!P.hidden&&!P.contains(e.target)&&e.target!==AB)P.hidden=true;});
draw();
</script>
{% endraw %}
{% endblock %}
"""

NAV_PARTIAL = r"""{% set cs_colors = {
  "Foundations":"#4a6fa5","Preprocessing":"#7a8699","Turbulence":"#2f8f8a",
  "Thermal & Radiation":"#c1663a","Compressible":"#6b5b95","Turbomachinery":"#3f5bb0",
  "Multiphase (VOF)":"#2b7fb8","Atmospheric & Environmental":"#4f8a4f",
  "Particles & Dispersion":"#8a6d3b","Reactive & Electric":"#b5473f"} %}
<nav class="cs-tree" aria-label="{{ lang.t('nav') }}">
  <div class="cs-navlbl">Tutorials</div>
  {% for item in nav %}
    {% if item.children %}
      <details class="cs-grp"{% if item.active %} open{% endif %}>
        <summary><span class="cs-dot" style="background:{{ cs_colors.get(item.title, '#8b98ac') }}"></span><span class="cs-name">{{ item.title }}</span><span class="cs-chev">&#9656;</span></summary>
        <ul>
          {% for c in item.children %}
            <li><a class="cs-link{% if c.active %} on{% endif %}" href="{{ c.url | url }}">{{ c.title }}</a></li>
          {% endfor %}
        </ul>
      </details>
    {% else %}
      <a class="cs-toplink{% if item.active %} on{% endif %}" href="{{ item.url | url }}">{{ item.title }}</a>
    {% endif %}
  {% endfor %}
</nav>
"""

HEADER_PARTIAL = r"""<header class="md-header" data-md-component="header">
  <nav class="md-header__inner md-grid" aria-label="{{ lang.t('header') }}">
    <a href="{{ config.extra.homepage | d(nav.homepage.url, true) | url }}" title="{{ config.site_name | e }}" class="md-header__button md-logo" aria-label="{{ config.site_name }}" data-md-component="logo">
      {% include "partials/logo.html" %}
    </a>
    <label class="md-header__button md-icon" for="__drawer">
      {% set icon = config.theme.icon.menu or "material/menu" %}
      {% include ".icons/" ~ icon ~ ".svg" %}
    </label>
    <div class="md-header__title" data-md-component="header-title">
      <div class="md-header__ellipsis"><div class="md-header__topic" data-md-component="header-topic">
        <span class="md-ellipsis">
          {% if page.meta and page.meta.title %}{{ page.meta.title }}{% else %}{{ page.title }}{% endif %}
        </span>
      </div></div>
    </div>
    <nav class="cs-hnav">
      <a href="https://code-saturne.org/doc/" target="_blank" rel="noopener">Documentation</a>
      <a href="https://code-saturne.org/forum/" target="_blank" rel="noopener">Forum</a>
      <a href="https://open-simulation-center.org/fr/downloads/code_saturne/code_saturne" target="_blank" rel="noopener">Downloads</a>
    </nav>
    {% if "material/search" in config.plugins %}
      {% set search = config.plugins["material/search"] | attr("config") %}
      {% if search.enabled %}
        <label class="md-header__button md-icon" for="__search">
          {% set icon = config.theme.icon.search or "material/magnify" %}
          {% include ".icons/" ~ icon ~ ".svg" %}
        </label>
        {% include "partials/search.html" %}
      {% endif %}
    {% endif %}
    {% if config.repo_url %}
      <div class="md-header__source">{% include "partials/source.html" %}</div>
    {% endif %}
    <span class="cs-mnt">
      <a href="https://simvia.tech/fr" target="_blank" rel="noopener"><img src="{{ 'assets/simvia.svg' | url }}" alt="Simvia, groupe EDF"></a>
    </span>
  </nav>
</header>
"""

def load():
    import yaml
    return yaml.safe_load(open(os.path.join(HERE,"tutorials.yaml")))["tutorials"]

def disp_title(s):                      # drop parenthetical suffix for display
    return re.sub(r"\s*\([^)]*\)","",s).strip()

def yaml_str(s):
    return '"' + s.replace('"','\\"') + '"'

def main():
    tuts = load()
    docs = os.path.join(OUT,"docs")
    if os.path.exists(OUT): shutil.rmtree(OUT)
    os.makedirs(docs)
    os.makedirs(os.path.join(docs,"assets"))
    os.makedirs(os.path.join(docs,"stylesheets"))

    # logos (source of truth: catalog/assets/)
    for f in ("code-saturne.svg","simvia.svg","icone-code-saturne.svg","og-card.png"):
        src=os.path.join(HERE,"assets",f)
        if os.path.exists(src): shutil.copy(src, os.path.join(docs,"assets",f))

    # per-tutorial pages (docs/<topic_dir>/<Case>/index.md + FIGURES)
    nav_by_topic = {t:[] for t in TOPIC_ORDER}
    for t in tuts:
        path = t["path"]                       # e.g. 10_turbulence_rans/Inc_Turbulent_Plate
        topic_dir, case = path.split("/",1)
        page_dir = os.path.join(docs, topic_dir, case)
        os.makedirs(page_dir, exist_ok=True)
        rm = os.path.join(ROOT, path, "README.md")
        md = open(rm, encoding="utf-8").read() if os.path.exists(rm) else f"# {t['title']}\n"
        md = inject_gh_source(md, path)
        open(os.path.join(page_dir,"index.md"),"w",encoding="utf-8").write(md)
        fig = os.path.join(ROOT, path, "FIGURES")
        if os.path.isdir(fig):
            shutil.copytree(fig, os.path.join(page_dir,"FIGURES"), dirs_exist_ok=True)
        nav_by_topic[t["topic"]].append((disp_title(t["title"]), f"{topic_dir}/{case}/index.md"))

    # home is a MkDocs page too (same header + nav templates as the tutorial pages,
    # so the chrome is identical by construction); its content is the centered hero.
    open(os.path.join(docs,"index.md"),"w",encoding="utf-8").write(
        "---\ntemplate: home.html\ntitle: Home\nhide:\n  - toc\n---\n\n# code_saturne tutorials\n")
    ovr = os.path.join(OUT,"overrides"); os.makedirs(ovr, exist_ok=True)
    data_js = json.dumps([{ "path":t["path"], "title":disp_title(t["title"]), "topic":t["topic"],
        "module":t["module"], "regime":t["regime"], "physics":t.get("physics",[]) or [],
        "difficulty":t["difficulty"], "cs_version":str(t["cs_version"]), "tags":t.get("tags",[]) or [] }
        for t in tuts], ensure_ascii=False)
    open(os.path.join(ovr,"home.html"),"w",encoding="utf-8").write(HOME_TEMPLATE.replace("__DATA__", data_js))

    # extra css: code_saturne palette + logo sizing
    css = """:root{
  --md-primary-fg-color:#355bb2; --md-primary-fg-color--light:#5b7cc4; --md-primary-fg-color--dark:#26417e;
  --md-accent-fg-color:#f57726; --cs-soft:#eaf0fb;
}
/* white top bar, matching the home */
.md-header{background:#fff;color:var(--md-default-fg-color);box-shadow:0 0 .2rem rgba(18,35,59,.08),0 .2rem .4rem rgba(18,35,59,.06)}
.md-header__button.md-logo img{height:1.55rem;width:auto}
.md-header__title{color:var(--md-default-fg-color);font-weight:600}
.md-header [data-md-component=search] .md-search__input{background:#eef3f9}
.cs-hnav{display:flex;gap:.1rem;align-items:center;margin-left:auto}
.cs-hnav a{font-size:.72rem;color:var(--md-default-fg-color--light);padding:.4rem .6rem;border-radius:.35rem;white-space:nowrap}
.cs-hnav a:hover{color:var(--md-primary-fg-color);background:var(--cs-soft)}
.cs-mnt{display:flex;align-items:center;gap:.4rem;color:var(--md-default-fg-color--light);margin:0 .3rem 0 .5rem}
.cs-mnt img{height:1.9rem;display:block}
@media (max-width:76.1875em){.cs-hnav{display:none}.cs-mnt{display:none}}
/* headings in serif, like the home */
.md-typeset h1,.md-typeset h2,.md-typeset h3{font-family:Charter,"Iowan Old Style",Georgia,serif}
/* "browse this case on GitHub" link under each tutorial title */
.md-typeset a.cs-ghsource{display:inline-flex;align-items:center;gap:.45rem;margin:-.3rem 0 1.4rem;padding:.4rem .8rem;font-size:.78rem;font-weight:600;color:var(--md-default-fg-color--light);border:1px solid var(--md-default-fg-color--lightest);border-radius:.6rem;text-decoration:none;transition:color .15s,border-color .15s,background .15s}
.md-typeset a.cs-ghsource:hover{color:var(--md-primary-fg-color);border-color:var(--md-primary-fg-color);background:var(--cs-soft)}
.md-typeset a.cs-ghsource svg{width:1rem;height:1rem;fill:currentColor;flex:none}
.cs-home{color:var(--md-default-fg-color--light)}
/* home hero (centered) */
.cs-hero{max-width:52rem;margin:0 auto;padding:9vh 1rem 6rem;text-align:center;display:flex;flex-direction:column;align-items:center}
.cs-hero h1{font-family:Charter,"Iowan Old Style",Georgia,serif;font-size:2.6rem;font-weight:600;letter-spacing:-.02em;margin:0 0 .7rem}
/* Charter's underscore glyph is very wide; shrink it so "code_saturne" reads tight */
.cs-hero h1 .cs-us{display:inline-block;transform:scaleX(.5);margin:0 -.13em}
.cs-hero p{color:var(--md-default-fg-color--light);max-width:42rem;margin:0 0 1.7rem;font-size:.95rem}
.cs-herosearch{display:flex;align-items:center;gap:.7rem;width:min(38rem,92vw);border:1.5px solid var(--md-default-fg-color--lightest);border-radius:.85rem;padding:.95rem 1.15rem;color:var(--md-default-fg-color--light);cursor:text;box-shadow:0 .4rem 1.4rem rgba(53,91,178,.10);transition:border-color .15s}
.cs-herosearch:hover{border-color:var(--md-primary-fg-color)}
.cs-hero-mnt{margin-top:1.6rem;display:flex;align-items:center;gap:.5rem;color:var(--md-default-fg-color--light);font-size:.8rem}
.cs-hero-mnt img{height:2rem;display:block}
.cs-clone{margin-top:1.6rem;text-align:center;color:var(--md-default-fg-color--light);font-size:.8rem}
.cs-clone>span{display:block}
.cs-clone code{display:inline-block;margin-top:.45rem;background:#eef3f9;border:1px solid #e3e9f1;border-radius:.4rem;padding:.4rem .7rem;font-family:var(--md-code-font,monospace);font-size:.78rem;color:var(--md-primary-fg-color)}
.cs-clone-note{margin-top:.6rem;font-size:.68rem;color:var(--md-default-fg-color--lighter)}
.cs-clone-note code{margin:0;padding:.05rem .3rem;font-size:.68rem}
/* home faceted search + result cards */
.cs-search{width:min(40rem,92vw);margin:.3rem auto 0}
.cs-search__box{display:flex;align-items:center;flex-wrap:wrap;gap:.4rem;border:1.5px solid var(--md-default-fg-color--lightest);border-radius:.85rem;padding:.7rem .8rem;box-shadow:0 .4rem 1.4rem rgba(53,91,178,.10)}
.cs-search__box:focus-within{border-color:var(--md-primary-fg-color)}
.cs-sic{color:var(--md-default-fg-color--light);flex:none}
.cs-search input{flex:1;min-width:8rem;border:0;background:none;outline:none;font-size:.95rem;color:var(--md-default-fg-color)}
.cs-fchip{display:inline-flex;align-items:center;gap:.3rem;background:var(--cs-soft);color:var(--md-primary-fg-color);border-radius:.4rem;padding:.15rem .35rem .15rem .5rem;font-size:.72rem}
.cs-fchip .k{opacity:.6;font-size:.62rem;text-transform:uppercase}.cs-fchip b{font-weight:600}
.cs-fchip button{border:0;background:none;color:inherit;cursor:pointer;font-size:.9rem;opacity:.6}.cs-fchip button:hover{opacity:1}
.cs-addf{position:relative;flex:none}
.cs-addf>button{border:1px dashed var(--md-default-fg-color--lightest);background:none;color:var(--md-default-fg-color--light);cursor:pointer;border-radius:.5rem;padding:.3rem .6rem;font-size:.72rem}
.cs-addf>button:hover{border-color:var(--md-primary-fg-color);color:var(--md-primary-fg-color)}
.cs-pop{position:absolute;right:0;top:calc(100% + .4rem);width:18rem;max-height:60vh;overflow:auto;background:#fff;border:1px solid var(--md-default-fg-color--lightest);border-radius:.7rem;box-shadow:0 .6rem 2rem rgba(18,35,59,.18);padding:.7rem;z-index:10;text-align:left}
.cs-pop[hidden]{display:none}.cs-pg{margin-bottom:.6rem}.cs-pg h4{font-size:.6rem;text-transform:uppercase;letter-spacing:.05em;color:var(--md-default-fg-color--lighter);margin:0 0 .35rem;font-weight:700}
.cs-pv{display:flex;flex-wrap:wrap;gap:.3rem}
.cs-vchip{border:1px solid var(--md-default-fg-color--lightest);background:#fff;color:var(--md-default-fg-color--light);border-radius:1rem;padding:.15rem .5rem;font-size:.7rem;cursor:pointer}
.cs-vchip:hover{border-color:var(--md-primary-fg-color)}.cs-vchip[aria-pressed=true]{background:var(--md-primary-fg-color);border-color:var(--md-primary-fg-color);color:#fff}
.cs-results{width:min(60rem,94vw);margin:2rem auto 0;text-align:left}
.cs-rcount{font-size:.72rem;color:var(--md-default-fg-color--light);margin-bottom:.7rem}
.cs-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(15rem,1fr));gap:.7rem}
.cs-card{display:flex;flex-direction:column;gap:.4rem;border:1px solid var(--md-default-fg-color--lightest);border-radius:.6rem;padding:.8rem .85rem;background:#fff;box-shadow:0 1px 2px rgba(18,35,59,.05);transition:border-color .12s,transform .12s}
.cs-card:hover{border-color:var(--md-primary-fg-color);transform:translateY(-2px)}
.cs-card .cr{display:flex;justify-content:space-between;align-items:center;gap:.4rem}
.cs-card .tp{display:inline-flex;align-items:center;gap:.35rem;font-size:.66rem;color:var(--md-default-fg-color--light);font-weight:600}
.cs-card .tp .d{width:.5rem;height:.5rem;border-radius:50%}
.cs-card h3{font-family:Charter,"Iowan Old Style",Georgia,serif;font-size:1rem;margin:0;line-height:1.2;color:var(--md-default-fg-color)}
.cs-card .mt{display:flex;flex-wrap:wrap;gap:.3rem .7rem;font-size:.68rem;color:var(--md-default-fg-color--light);align-items:center}
.cs-card .bd{font-family:var(--md-code-font,monospace);font-size:.62rem;background:#eef3f9;border:1px solid #e3e9f1;padding:0 .3rem;border-radius:.25rem;color:var(--md-primary-fg-color)}
.cs-card .phs{display:flex;flex-wrap:wrap;gap:.25rem}.cs-card .ph{font-size:.62rem;color:var(--md-primary-fg-color);background:var(--cs-soft);padding:.05rem .4rem;border-radius:.3rem}
.cs-card .pill{font-size:.58rem;font-weight:700;text-transform:uppercase;letter-spacing:.03em;padding:.1rem .4rem;border-radius:1rem;white-space:nowrap}
.cs-card .pill.beginner{color:#2f7d4f;background:#e6f0e9}.cs-card .pill.intermediate{color:#a9781f;background:#f6eddb}.cs-card .pill.advanced{color:#b8531f;background:#f7e8dd}
.cs-none{color:var(--md-default-fg-color--light);padding:1.5rem 0}
/* left nav = same tree as the home */
.cs-navlbl{font-size:.6rem;text-transform:uppercase;letter-spacing:.08em;color:var(--md-default-fg-color--lighter);font-weight:700;padding:0 .6rem;margin:.2rem 0 .6rem}
.cs-grp{margin:0 0 .1rem}
.cs-grp>summary{list-style:none;display:flex;align-items:center;gap:.5rem;cursor:pointer;padding:.42rem .6rem;border-radius:.35rem;font-size:.76rem;font-weight:600;color:var(--md-default-fg-color)}
.cs-grp>summary::-webkit-details-marker{display:none}
.cs-grp>summary:hover{background:var(--cs-soft)}
.cs-chev{margin-left:auto;color:var(--md-default-fg-color--lighter);transition:transform .15s}
.cs-grp[open]>summary .cs-chev{transform:rotate(90deg)}
.cs-dot{width:.55rem;height:.55rem;border-radius:50%;flex:none}
.cs-tree ul{list-style:none;margin:.1rem 0 .5rem;padding:0 0 0 1.55rem}
.cs-link{display:block;padding:.32rem .6rem;border-radius:.3rem;font-size:.74rem;color:var(--md-default-fg-color--light);line-height:1.3}
.cs-link:hover{background:#fff;color:var(--md-primary-fg-color)}
.cs-link.on{color:var(--md-primary-fg-color);font-weight:700;background:var(--cs-soft)}
.cs-toplink{display:block;padding:.42rem .6rem;border-radius:.35rem;font-size:.76rem;font-weight:600;color:var(--md-default-fg-color)}
.cs-toplink:hover{background:var(--cs-soft)}
.cs-toplink.on{color:var(--md-primary-fg-color);background:var(--cs-soft)}
/* light footer with Simvia (no black bar, no prev/next) */
.md-footer{background:transparent}
.md-footer-meta.cs-footer{background:#f2f5fa;color:var(--md-default-fg-color--light);border-top:1px solid #e3e9f1}
.cs-footer__inner{display:flex;align-items:center;justify-content:space-between;gap:1rem;flex-wrap:wrap;padding:.85rem .2rem}
.cs-footer__mnt{display:flex;align-items:center;gap:.5rem;font-size:.72rem}
.cs-footer__mnt img{height:1.7rem;display:block}
.cs-footer__links{display:flex;gap:1.1rem;font-size:.72rem}
.cs-footer__links a{color:var(--md-default-fg-color--light)}
.cs-footer__links a:hover{color:var(--md-primary-fg-color)}
"""
    open(os.path.join(docs,"stylesheets","extra.css"),"w").write(css)

    # nav yaml: Home + topic sections (external links live in the custom header)
    nav = ["  - Home: index.md"]
    for topic in TOPIC_ORDER:
        items = nav_by_topic.get(topic,[])
        if not items: continue
        nav.append(f"  - {yaml_str(topic)}:")
        for title, page in items:
            nav.append(f"      - {yaml_str(title)}: {page}")

    # custom header partial: same top bar as the home (logo + links + GitHub + Simvia)
    os.makedirs(os.path.join(ovr,"partials"), exist_ok=True)
    open(os.path.join(ovr,"main.html"),"w",encoding="utf-8").write(MAIN_TEMPLATE)
    open(os.path.join(ovr,"partials","header.html"),"w",encoding="utf-8").write(HEADER_PARTIAL)
    open(os.path.join(ovr,"partials","nav.html"),"w",encoding="utf-8").write(NAV_PARTIAL)
    open(os.path.join(ovr,"partials","footer.html"),"w",encoding="utf-8").write(FOOTER_PARTIAL)

    cfg = f"""site_name: code_saturne tutorials
site_url: https://simvia-tech.github.io/tutorials-code_saturne/
site_description: Verified step-by-step CFD tutorials for code_saturne, maintained by Simvia.
repo_url: {GITHUB_URL}
repo_name: tutorials
edit_uri: ""

plugins:
  - search

theme:
  name: material
  custom_dir: overrides
  logo: assets/code-saturne.svg
  favicon: assets/icone-code-saturne.svg
  icon:
    repo: fontawesome/brands/github
  palette:
    scheme: default
    primary: custom
    accent: custom
  font:
    text: Source Sans 3
    code: Source Code Pro
  features:
    - navigation.indexes
    - navigation.top
    - content.code.copy
    - toc.follow
    - search.suggest
    - search.highlight

extra_css:
  - stylesheets/extra.css

extra:
  social:
    - icon: fontawesome/solid/globe
      link: https://code-saturne.org/
      name: code_saturne website
    - icon: fontawesome/solid/book
      link: https://code-saturne.org/doc/
      name: Documentation
    - icon: fontawesome/solid/comments
      link: https://code-saturne.org/forum/
      name: Forum

markdown_extensions:
  - admonition
  - attr_list
  - md_in_html
  - tables
  - footnotes
  - toc:
      permalink: true
  - pymdownx.superfences
  - pymdownx.details
  - pymdownx.highlight
  - pymdownx.arithmatex:
      generic: true

extra_javascript:
  - https://unpkg.com/mathjax@3/es5/tex-mml-chtml.js

nav:
{chr(10).join(nav)}
"""
    open(os.path.join(OUT,"mkdocs.yml"),"w").write(cfg)
    print(f"Built site source in {OUT}")
    print(f"  {len(tuts)} tutorials, {sum(1 for t in TOPIC_ORDER if nav_by_topic[t])} sections")

if __name__ == "__main__":
    main()
