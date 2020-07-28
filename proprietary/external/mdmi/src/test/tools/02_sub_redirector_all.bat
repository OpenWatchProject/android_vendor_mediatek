@set ADB_PATH=D:\Tools\androidSdk\adb.exe
@set NC_PATH=D:\Tools\netcat-win32-1.12\nc64.exe
@set OUTPUT_FILE_NAME=adb_raw.bin

REM Subscribe all MDMI KPIs
%ADB_PATH% shell "mdmi_redirector_ctrl --subscribe libtype=0 OID=1"

pause