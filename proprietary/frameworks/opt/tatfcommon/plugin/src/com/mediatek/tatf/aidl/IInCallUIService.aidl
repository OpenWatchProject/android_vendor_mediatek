package com.mediatek.tatf.aidl;

interface IInCallUIService {
  boolean startCall(String number, String subId);
  boolean endCall(String number, String subId);
  String getCallState(String subId);
}
