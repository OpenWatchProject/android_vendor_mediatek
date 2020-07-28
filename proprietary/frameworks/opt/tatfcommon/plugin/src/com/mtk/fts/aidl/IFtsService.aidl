package com.mtk.fts.aidl;

interface IFtsService {
    boolean mtCall(String number, int type, int timeout, String certification);
    String getOperatorServiceNumber(String iccid, int type);
    boolean hangupCall(String number, int type);
}
