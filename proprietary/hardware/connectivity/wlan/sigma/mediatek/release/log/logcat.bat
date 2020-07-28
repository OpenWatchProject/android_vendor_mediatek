@echo off

for /f %%x in ('wmic os get localdatetime ^| findstr /b [0-9]') do set format_time=%%x
set filename=%format_time:~0,15%

echo redirecting logcat to log.txt ...
adb logcat -c
adb logcat -v threadtime -b all > log.txt



:END
