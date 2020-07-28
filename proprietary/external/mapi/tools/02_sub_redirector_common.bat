@set ADB_PATH=D:\Tools\AndroidSdkTool\adb.exe
@set NC_PATH=D:\Tools\netcat-win32-1.12\nc64.exe
@set OUTPUT_FILE_NAME=adb_raw.bin

%ADB_PATH% shell "mdi_redirector_ctrl --set-payload-size size=1500 OID=1.3.4"
%ADB_PATH% shell "mdi_redirector_ctrl --subscribe-object OID=1"

pause