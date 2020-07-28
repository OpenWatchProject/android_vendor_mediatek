@echo off
set num1=0
set num2=1
rem set terminator=168
set terminator=1000000

:loop
if %num1%==%terminator% goto close
goto open

:close
  rem echo %num1%
  pause
  exit /b

:open
  for /F "tokens=2" %%i in ('date /t') do set mydate=%%i
  set mytime=%time%
  echo Current time is %mydate%:%mytime%
  echo %num1%
  timeout /t 30
  adb.exe -s %1 wait-for-device
rem  adb.exe devices
  set /a random_sleep=%random% %%120+1
  timeout /t %random_sleep%
  adb.exe -s %1 wait-for-device
rem adb.exe -s %1 shell "/data/fod_ca 1 cmds"
  adb.exe -s %1 root
  adb.exe -s %1 wait-for-device
  adb.exe -s %1 shell "/data/local/tmp/fod_ca 1 cmds-reboot"
rem timeout /t 10
  set /a num1= %num1% + %num2%
rem adb.exe reboot
  goto loop