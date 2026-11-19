#!/bin/sh
# prepare.sh DIR [no-process]: the files of one run of test.bat, in DIR: the script,
# its expected output and check.sh. With no-process, without the script's part that
# starts a program and the line of output it gives (a row where no program can be
# started: firmware, DOS, the browser and node, an openkal image under okrun).
here=$(cd "$(dirname "$0")" && pwd)
mkdir -p "$1"
cp "$here/check.sh" "$1/"
if [ "${2:-}" = no-process ]; then
  sed '/^rem process:/,/^rem end of process/d' "$here/test.bat" > "$1/test.bat"
  grep -vx 'child=3' "$here/expected.txt" > "$1/expected.txt"
else
  cp "$here/test.bat" "$here/expected.txt" "$1/"
fi
