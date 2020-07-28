package com.mediatek.op08.deviceswitch;

/** Information about the device to switch the call to. */
public class DeviceSwitchInfo {
  public String virtualLine;
  public String realLine;
  public String deviceName;
  public String deviceId;
  public String[] msisdns;


  // TODO: Convert to autovalue. b/34502119
  public static DeviceSwitchInfo createEmptyDeviceSwitchInfo() {
    return new DeviceSwitchInfo(null, null,null, null, null);
  }

  public DeviceSwitchInfo(String virtual, String real, String name, String deviceId, 
          String[] msisdns) {
    this.virtualLine = virtual;
    this.realLine = real;
    this.deviceName = name;
    this.deviceId = deviceId;
    this.msisdns = msisdns;
  }

  public String getVirtualLine() {
      return virtualLine;
  }

  public String getRealLine() {
      return realLine;
  }

  public String getDeviceName() {
      return deviceName;
  }

  public String getDeviceId() {
      return deviceId;
  }

  public String[] getMsisdns() {
      return msisdns;
  }
}
