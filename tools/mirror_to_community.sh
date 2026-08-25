#!/usr/bin/env bash
#
# Mirror the content of this repository into a subdirectory of another one.
#
# This is a *content* mirror, not a git mirror: the destination gets one commit
# per synchronisation and none of our history. That is deliberate. The
# destination is shared with other contributors, so `git push --mirror` is out
# of the question, since it deletes every ref the source does not have.
#
# Usage:
#   tools/mirror_to_community.sh <source-worktree> <dest-clone> <subdir> [--dry-run]
#
# What is copied is the list of files git tracks, taken from the worktree so
# that LFS files carry their real contents. Anything git ignores (RESU/,
# validation/, ...) is therefore never copied, however dirty the worktree is.
# Run `git lfs pull` in the source first, or the meshes are still pointers and
# the script refuses to run.
#
# The destination does not use Git LFS, so the meshes are written there as
# ordinary blobs and the LFS declarations are stripped from the copied
# .gitattributes. They still compress well: 138 MB of ASCII gmsh packs down to
# about 23 MB.
#
set -euo pipefail

[ $# -ge 3 ] || { sed -n '3,18p' "$0" | sed 's/^# \{0,1\}//'; exit 1; }

SRC=$(cd "$1" && pwd)
DEST=$(cd "$2" && pwd)
SUBDIR=$3
DRY=${4:-}

# ---------------------------------------------------------------- safety
# A subdirectory that collapses to the repository root would make the wipe
# below destroy what the other contributors have put there.
case "$SUBDIR" in
  "" | "." | "/" | /* | */.. | ../* | *"/../"*)
    echo "refusing an unsafe destination subdirectory: '$SUBDIR'" >&2
    exit 1
    ;;
esac
[ -d "$SRC/.git" ] || { echo "source '$SRC' is not a git worktree" >&2; exit 1; }
[ -d "$DEST/.git" ] || { echo "destination '$DEST' is not a git clone" >&2; exit 1; }

# ---------------------------------------------------------------- LFS check
# Copying a pointer file would leave a broken LFS reference in the destination.
while IFS= read -r f; do
  [ -n "$f" ] || continue
  if head -c 42 "$SRC/$f" 2>/dev/null | grep -q '^version https://git-lfs'; then
    echo "'$f' is still an LFS pointer in the source." >&2
    echo "Run 'git lfs pull' in '$SRC' before mirroring." >&2
    exit 1
  fi
done < <(git -C "$SRC" lfs ls-files -n 2>/dev/null || true)

# ---------------------------------------------------------------- file list
# Tracked files only, minus what belongs to this repository alone: our CI, and
# the machinery that builds our own website and the mirroring script itself,
# all of which would arrive over there inert and only puzzle a reader.
#
# The governance files go too. GitHub only ever surfaces the ones at the root
# of a repository, so ours would be invisible inside the subdirectory, and
# shipping a code of conduct into someone else's project reads as imposing our
# rules on it. LICENSE stays: our content is GPL-3.0 in a project whose solver
# is GPL-2.0, so it has to say so where it lives.
list=$(mktemp)
trap 'rm -f "$list"' EXIT
git -C "$SRC" ls-files -z \
  | grep -zv '^\.github/' \
  | grep -zv '^catalog/' \
  | grep -zv '^tools/' \
  | grep -zv '^CODE_OF_CONDUCT\.md$' \
  | grep -zv '^CONTRIBUTING\.md$' \
  | tr '\0' '\n' > "$list"
count=$(grep -c . "$list")
echo "$count tracked files to mirror into '$SUBDIR/'"

if [ "$DRY" = "--dry-run" ]; then
  echo "dry run: nothing is written and nothing is committed"
  rsync -a --dry-run --itemize-changes --files-from="$list" \
        "$SRC/" "$DEST/$SUBDIR/" | head -40
  exit 0
fi

# ---------------------------------------------------------------- copy
# Wipe and rewrite: that is what makes it a mirror. Files we deleted here
# disappear there, and git only records what actually changed.
rm -rf "$DEST/$SUBDIR"
mkdir -p "$DEST/$SUBDIR"
rsync -a --files-from="$list" "$SRC/" "$DEST/$SUBDIR/"

# ---------------------------------------------------------------- no LFS there
# Our .gitattributes routes the meshes through Git LFS. Copied as is, it would
# apply to the destination subdirectory too and ask a repository that does not
# use LFS to filter them. Drop the filter and keep the `-text` flag, so the
# meshes are still stored verbatim instead of being line-ending normalised.
attr="$DEST/$SUBDIR/.gitattributes"
if [ -f "$attr" ]; then
  sed -i -e 's/filter=lfs diff=lfs merge=lfs //' \
         -e 's|^# Git LFS *:.*|# Maillages et formats lourds : stockés tels quels, sans filtre|' \
         "$attr"
  if grep -q 'filter=lfs' "$attr"; then
    echo "LFS declarations survived in $attr" >&2
    exit 1
  fi
fi

# ------------------------------------------------- our plumbing, not theirs
# The tail of .gitignore covers the website build and our logo sources. None
# of that is copied, so the rules would arrive there with nothing to match,
# and one of the filenames advertises artwork that has no business being
# listed in someone else's repository.
ign="$DEST/$SUBDIR/.gitignore"
if [ -f "$ign" ]; then
  sed -i '/^# Working artifacts, never shipped$/,$d' "$ign"
  sed -i -e '/catalog\//d' -e '/_site_src/d' -e '/SIMVIA/d' -e '/simvia\.png/d' "$ign"
  # the cut leaves the block's opening rule and blank lines dangling
  awk '{ l[NR] = $0 }
       END { n = NR
             while (n > 0 && (l[n] ~ /^[[:space:]]*$/ || l[n] ~ /^#[[:space:]]*-*[[:space:]]*$/))
               n--
             for (i = 1; i <= n; i++) print l[i] }' "$ign" > "$ign.tmp"
  mv "$ign.tmp" "$ign"
  if grep -qiE 'catalog/|_site_src|SIMVIA' "$ign"; then
    echo "our own plumbing survived in $ign" >&2
    exit 1
  fi
fi

# The README's website section explains how to build our own site from
# catalog/ and .github/, neither of which is copied, so over there it is a
# recipe for files that do not exist.
rme="$DEST/$SUBDIR/README.md"
if [ -f "$rme" ]; then
  awk '/^## The website$/            { skip = 1 }
       /^## / && !/^## The website$/ { skip = 0 }
       !skip                         { print }' "$rme" > "$rme.tmp"
  mv "$rme.tmp" "$rme"
  if grep -q 'catalog/' "$rme"; then
    echo "the website section survived in $rme" >&2
    exit 1
  fi
fi

# ---------------------------------------------------------------- commit
sha=$(git -C "$SRC" rev-parse --short HEAD)
url=$(git -C "$SRC" remote get-url github 2>/dev/null \
      || git -C "$SRC" remote get-url origin 2>/dev/null || echo "unknown")

git -C "$DEST" add -A -- "$SUBDIR"
if git -C "$DEST" diff --cached --quiet; then
  echo "nothing to mirror: '$SUBDIR' is already up to date"
  exit 0
fi

# A stray LFS configuration on the destination side would turn the meshes back
# into pointers, which is exactly what the destination does not want.
while IFS= read -r f; do
  [ -n "$f" ] || continue
  if git -C "$DEST" show ":$SUBDIR/$f" 2>/dev/null \
     | head -c 42 | grep -q '^version https://git-lfs'; then
    echo "'$SUBDIR/$f' was staged as an LFS pointer, not as its contents." >&2
    echo "The destination clone still has an LFS filter configured." >&2
    exit 1
  fi
done < <(git -C "$SRC" lfs ls-files -n 2>/dev/null || true)

git -C "$DEST" commit -q \
  -m "Sync $SUBDIR from tutorials-code_saturne@$sha" \
  -m "Content mirror of $url at $sha, by tools/mirror_to_community.sh."
git -C "$DEST" show --stat --oneline HEAD | head -15
