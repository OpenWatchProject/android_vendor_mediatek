@echo off

for /f %%x in ('wmic os get localdatetime ^| findstr /b [0-9]') do set format_time=%%x
set filename=%format_time:~0,15%
adb pull sdcard/mtklog %filename%/
adb pull sdcard/debuglogger %filename%/
adb pull data/aee_exp %filename%/
copy ca_log.log %filename%
copy dut_log.log %filename%
copy pf_log.log %filename%
move ca_log.log %filename%
move dut_log.log %filename%
move pf_log.log %filename%


:END
