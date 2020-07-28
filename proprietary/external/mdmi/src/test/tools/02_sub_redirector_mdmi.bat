@set ADB_PATH=D:\Tools\androidSdk\adb.exe
@set NC_PATH=D:\Tools\netcat-win32-1.12\nc64.exe
@set OUTPUT_FILE_NAME=adb_raw.bin

REM Subscribe [1.2.2.2.2] bandIndicatorValue
REM Subscribe [1.2.2.2.3] dlBandwidthValue
REM Subscribe [1.2.2.2.4] ulBandwidthValue
REM Subscribe [1.2.2.2.5] dlFrequencyValue
REM Subscribe [1.2.2.2.6] ulFrequencyValue
REM Subscribe [1.2.2.2.7] trackingAreaCode
REM Subscribe [1.2.2.2.8] cellIdValue
%ADB_PATH% shell "mdmi_redirector_ctrl --subscribe libtype=0 OID=1.2.2.2.2 OID=1.2.2.2.3 OID=1.2.2.2.4 OID=1.2.2.2.5 OID=1.2.2.2.6 OID=1.2.2.2.7 OID=1.2.2.2.8"
@ping 127.0.0.1 -n 5 -w 500 > nul

REM Subscribe [1.2.2.3.2] HandoverEvent
REM Subscribe [1.2.2.3.4] newCellEvent
REM Subscribe [1.2.2.3.5] rrcStateChangeEvent
REM Subscribe [1.2.2.3.6] rrcTimerExpiryEvent
%ADB_PATH% shell "mdmi_redirector_ctrl --subscribe libtype=0 OID=1.2.2.3.2 OID=1.2.2.3.4 OID=1.2.2.3.5 OID=1.2.2.3.6"
@ping 127.0.0.1 -n 5 -w 500 > nul

REM Subscribe [1.2.7.2.1] servingCellMeasurement
REM Subscribe [1.2.7.2.2] neighborCellMeasurements
REM Subscribe [1.2.7.2.5] dlPhysLayerThroughputValue
REM Subscribe [1.2.7.2.6] ulPhysLayerThroughputValue
REM Subscribe [1.2.7.2.13] mimoPDSCHthroughput0Value
REM Subscribe [1.2.7.2.14] mimoPDSCHthroughput1Value
REM Subscribe [1.2.7.2.15] srsTxPowerMeasurement
%ADB_PATH% shell "mdmi_redirector_ctrl --subscribe libtype=0 OID=1.2.7.2.1 OID=1.2.7.2.2 OID=1.2.7.2.5 OID=1.2.7.2.6 OID=1.2.7.2.13 OID=1.2.7.2.14 OID=1.2.7.2.15"
@ping 127.0.0.1 -n 5 -w 500 > nul

pause