#!/bin/sh
# check.sh OUTPUT: compares a run of test.bat with expected.txt, ignoring
# carriage returns, colour escapes and empty lines; and case, with
# CHECK_FOLD_CASE=1 (a system that keeps names in upper case: DOS without long names).
here=$(cd "$(dirname "$0")" && pwd)
esc=$(printf '\033')
fold() { if [ "${CHECK_FOLD_CASE:-0}" = 1 ]; then tr '[:upper:]' '[:lower:]'; else cat; fi; }
norm() { tr -d '\r' < "$1" | sed "s/${esc}\[[0-9;]*m//g" | grep -v '^[[:space:]]*$' | fold; }
norm "$here/expected.txt" > "$1.expected"
norm "$1" > "$1.norm"
if diff -u "$1.expected" "$1.norm"; then
  echo "test.bat: output as expected"
else
  echo "::error::test.bat: output differs"
  exit 1
fi
