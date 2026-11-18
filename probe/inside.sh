#!/bin/sh
set -u
TARBALL="$1"; HOME="$2"; export HOME
export TMPDIR="$HOME/tmp"; mkdir -p "$TMPDIR" "$HOME/dist"
tar -xzf "$TARBALL" -C "$HOME/dist"
MCPP=$(find "$HOME/dist" -path '*/bin/mcpp' -type f | head -1)
"$MCPP" --version
cd "$HOME" && rm -rf hello && "$MCPP" new hello >/dev/null
cd hello
printf 'import std;\nint main() { std::println("hello from termux"); }\n' > src/main.cpp
echo "===== build -v"
MCPP_VERBOSE=1 "$MCPP" build -v 2>&1 | grep -v "Downloading\|extract\|Source " | tail -80
echo "===== compiler-cap outside a generation"
"$MCPP" __compiler-cap --sem "$TMPDIR/sem" -- /system/bin/sh -c 'echo ran-under-cap'; echo "rc=$?"
echo "===== state"
find "$HOME/hello" -path '*compiler-runs*' | head -60
for f in $(find "$HOME/hello" -path '*compiler-runs*' -type f | head -40); do echo "--- $f"; head -c 400 "$f"; echo; done
find "$HOME/hello/target" -maxdepth 3 | head -40
echo "===== ninja direct"
D=$(find "$HOME/hello/target" -name build.ninja | head -1); NINJA=$(find "$HOME/dist" -name ninja -type f | head -1)
[ -n "$D" ] && [ -n "$NINJA" ] && (cd "$(dirname "$D")" && "$NINJA" -v -d explain 2>&1 | tail -30)
