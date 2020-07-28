@set ADB_PATH=D:\Tools\androidSdk\adb.exe
@set NC_PATH=D:\Tools\netcat-win32-1.12\nc64.exe
@set OUTPUT_FILE_NAME=output.mdmi

@ping 127.0.0.1 -n 10 -w 1000 > nul
@echo ========================
@echo      Execute MDMI Redirector
@echo ========================
%ADB_PATH% shell "mdmi_redirector_ctrl --start"
@ping 127.0.0.1 -n 10 -w 1000 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=0"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=1"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=2"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=3"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=4"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=5"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=6"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=7"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=8"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=9"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --create-session libtype=10"

@ping 127.0.0.1 -n 5 -w 1000 > nul
%ADB_PATH% forward tcp:10567 tcp:10567

@echo =================================
@echo    Record ADB data to output.mdmi
@echo =================================
@ping 127.0.0.1 -n 3 -w 1000 > nul
%NC_PATH% localhost 10567 > %OUTPUT_FILE_NAME%

pause