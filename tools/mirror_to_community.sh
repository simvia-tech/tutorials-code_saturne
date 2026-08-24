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
list=$(mktemp)
trap 'rm -f "$list"' EXIT
git -C "$SRC" ls-files -z \
  | grep -zv '^\.github/' \
  | grep -zv '^catalog/' \
  | grep -zv '^tools/' \
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

# ---------------------------------------------------------------- commit
sha=$(git -C "$SRC" rev-parse --short HEAD)
url=$(git -C "$SRC" remote get-url github 2>/dev/null \
      || git -C "$SRC" remote get-url origin 2>/dev/null || echo "unknown")

git -C "$DEST" add -A -- "$SUBDIR"
if git -C "$DEST" diff --cached --quiet; then
  echo "nothing to mirror: '$SUBDIR' is already up to date"
  exit 0
fi

git -C "$DEST" commit -q \
  -m "Sync $SUBDIR from tutorials-code_saturne@$sha" \
  -m "Content mirror of $url at $sha, by tools/mirror_to_community.sh."
git -C "$DEST" show --stat --oneline HEAD | head -15
