#!/usr/bin/env bash
W="$1"; A=/data/data/com.termux/files
adb wait-for-device
adb install -r "$W/termux.apk" >/dev/null
adb shell am start -n com.termux/com.termux.app.TermuxActivity >/dev/null
for _ in $(seq 1 60); do adb shell run-as com.termux ls "$A/usr/bin/sh" >/dev/null 2>&1 && break; sleep 5; done
for f in mcpp.tar.gz inside.sh; do adb push "$W/$f" /data/local/tmp/ >/dev/null; done
adb logcat -c
adb shell "run-as com.termux sh -c 'mkdir -p $A/home && cp /data/local/tmp/mcpp.tar.gz /data/local/tmp/inside.sh $A/home/ && cd $A/home && PATH=$A/usr/bin:/system/bin sh ./inside.sh $A/home/mcpp.tar.gz $A/home'" 2>&1 | tail -300
echo "===== logcat (avc / seccomp / signals) ====="
adb logcat -d | grep -iE "avc:|seccomp|SIGSYS|audit|Fatal signal|libc *:|mcpp" | tail -120
exit 0
