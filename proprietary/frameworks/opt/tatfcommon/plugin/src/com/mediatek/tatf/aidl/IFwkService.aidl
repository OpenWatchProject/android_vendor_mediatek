package com.mediatek.tatf.aidl;

import android.os.Bundle;

interface IFwkService {
  boolean dataSwitch(boolean open, int subId);
  boolean openUrl(String url);
  boolean moCsCall(int phoneId, String phoneNumber, int duration);
  int getPreferredNetworkType(int phoneId);
  boolean setPreferredNetworkType(int phoneId, int networkType);
  Bundle getAvailableNetworks(int phoneId);
  int getServiceState(int phoneId);
  int getCapabilityPhoneId();
  boolean plugInSimCard(int phoneId);
  boolean plugOutSimCard(int phoneId);
  boolean enableFdn(boolean enable, int subId);
  boolean getIccFdnEnabled(int subId);
  // TK only API.
  int moCall(int phoneId, String phoneNumber, int type);
  String sendATCommand(int subId, String cmd);
  Bundle sendCommand(String cmd, in Bundle param);
  boolean getPolCapability(int phoneId);
  boolean getPolList(int phoneId);
  boolean setPolEntry(int phoneId, int priority, String mccmnc, int Act);
}
