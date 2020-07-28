@set ADB_PATH=D:\Tools\androidSdk\adb.exe

@echo ========================
@echo      Terminate Redirector
@echo ========================
%ADB_PATH% shell "mdmi_redirector_ctrl --unsubscribe libtype=0 OID=1"
@ping 127.0.0.1 -n 10 -w 1000 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=0"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=1"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=2"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=3"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=4"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=5"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=6"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=7"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=8"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=9"
@ping 127.0.0.1 -n 1 -w 100 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --close-session libtype=10"
@ping 127.0.0.1 -n 5 -w 1000 > nul
%ADB_PATH% shell "mdmi_redirector_ctrl --stop"

.\DiffTool.exe .\output.mdmi .\output.txt > debug.txt

pause