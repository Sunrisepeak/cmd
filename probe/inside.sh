#!/bin/sh
set -u
echo "===== boot_id"; cat /proc/sys/kernel/random/boot_id; echo "rc=$?"
echo "===== ns/pid"; readlink /proc/self/ns/pid; echo "rc=$?"
echo "===== stat"; head -c 120 /proc/self/stat; echo; echo "rc=$?"
echo "===== setpgid"; (set -m 2>/dev/null; echo ok)
ls -l /proc/sys/kernel/random/ 2>&1 | head
