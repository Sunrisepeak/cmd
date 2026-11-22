#!/bin/sh
# collect.sh TARGET: a built target's files for a release, in dist/:
#   dist/bare/<target>-<file>  the programs as built (the form of the project's releases)
#   dist/pack/                 mcpp's package of the target (both programs, README, LICENSE)
# The web page is the package of wasm32-emscripten (the web job). A DJGPP program
# is static by construction; mcpp packs it as such only when told (mcpp#1007).
set -eu
t=$1
mkdir -p dist/bare dist/pack
if [ "$t" = openkal ]; then
  cp target/dist/cmd-*-openkal-bundle-all.tar.gz dist/pack/
  exit 0
fi
find "target/$t" -path '*/bin/*' -type f | while read -r f; do
  cp "$f" "dist/bare/$t-$(basename "$f")"
done
case $t in
  wasm32-emscripten) ;;
  i586-dos-djgpp) mode='--mode static' ;;
  *) mode= ;;
esac
if [ "$t" != wasm32-emscripten ]; then
  # shellcheck disable=SC2086
  mcpp pack --release --target "$t" --all-programs $mode -o dist/pack/ ||
    echo "::warning::$t: mcpp pack failed; the release has its programs as built only"
fi
ls -l dist/bare dist/pack
