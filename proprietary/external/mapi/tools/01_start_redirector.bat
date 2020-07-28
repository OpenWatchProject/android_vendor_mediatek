@set ADB_PATH=D:\Tools\AndroidSdkTool\adb.exe
@set NC_PATH=D:\Tools\netcat-win32-1.12\nc64.exe
@set OUTPUT_FILE_NAME=adb_raw.bin

%ADB_PATH% root
%ADB_PATH% shell setenforce 0

@ping 127.0.0.1 -n 10 -w 1000 > nul
@echo ========================
@echo      Execute Redirector
@echo ========================
%ADB_PATH% shell "mdi_redirector_ctrl --leave-testing-mode"
@ping 127.0.0.1 -n 10 -w 1000 > nul
%ADB_PATH% shell "mdi_redirector_ctrl --enter-testing-mode"
@ping 127.0.0.1 -n 5 -w 1000 > nul
%ADB_PATH% shell "mdi_redirector_ctrl --start-monitor"
%ADB_PATH% forward tcp:10567 tcp:10567

@echo =================================
@echo    Record ADB data to adb_raw.bin
@echo =================================
@ping 127.0.0.1 -n 3 -w 1000 > nul
%NC_PATH% localhost 10567 > %OUTPUT_FILE_NAME%

pause