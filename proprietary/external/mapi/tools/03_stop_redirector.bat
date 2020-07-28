@set ADB_PATH=D:\Tools\AndroidSdkTool\adb.exe

@echo ========================
@echo      Terminate Redirector
@echo ========================
%ADB_PATH% shell "mdi_redirector_ctrl --stop-monitor"
%ADB_PATH% shell "mdi_redirector_ctrl --leave-testing-mode"

pause