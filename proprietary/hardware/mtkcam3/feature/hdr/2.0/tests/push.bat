@echo off

adb remount
adb shell "rm -rf /data/input"
adb shell "mkdir -p /data/input"
adb push input /data/input