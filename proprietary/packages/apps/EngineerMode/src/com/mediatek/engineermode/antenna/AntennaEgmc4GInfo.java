package com.mediatek.engineermode.antenna;

import java.util.Arrays;

public class AntennaEgmc4GInfo {

    private boolean forceRx;
    private boolean cssFollowPcc;
    private boolean[] pCell2Rx = new boolean[2];
    private boolean[] pCell4Rx = new boolean[4];
    private boolean[] sCell2Rx = new boolean[2];
    private boolean[] sCell4Rx = new boolean[4];

    public static int booleanToInt(boolean[] data) {
        int intValue = 0;
        for(int i=0; i<data.length; i++) {
            if(data[i]) {
                intValue |= (1 << i);
            } else {
                intValue &= ~(1 << i);
            }
        }
       return intValue;
    }
 
      public static boolean[] intToBoolean(int data, int length) {
            String boolConfig = Integer.toBinaryString(data);
            boolean[] boolResults = new boolean[length];
            char[] results = boolConfig.toCharArray();
            for(int i = 0; i < length - results.length; i++) {
                boolResults[length - 1 - i] = false;
            }
            for(int i=0; i<results.length; i++) {
              boolResults[results.length - 1 - i] = results[i] == '1';
            }
            return boolResults;
        }

    public AntennaEgmc4GInfo(int forceRx, int cssFollowPcc, int pCell2Rx, int pCell4Rx, int sCell2Rx, int sCell4Rx){
        setForceRx(forceRx);
        setCssFollowPcc(cssFollowPcc);
        setPCell2Rx(pCell2Rx);
        setPCell4Rx(pCell4Rx);
        setSCell2Rx(sCell2Rx);
        setSCell4Rx(sCell4Rx);
    }
    public void updateAntennaEgmc4GInfo(int forceRx, int cssFollowPcc, int pCell2Rx, int pCell4Rx, int sCell2Rx, int sCell4Rx){
        setForceRx(forceRx);
        setCssFollowPcc(cssFollowPcc);
        setPCell2Rx(pCell2Rx);
        setPCell4Rx(pCell4Rx);
        setSCell2Rx(sCell2Rx);
        setSCell4Rx(sCell4Rx);
    }
    public AntennaEgmc4GInfo()
    {
        setForceRx(0);
        setCssFollowPcc(0);
        setPCell2Rx(0);
        setPCell4Rx(0);
        setSCell2Rx(0);
        setSCell4Rx(0);
    }

    public void CloseForceRxFor4G()
    {
        setForceRx(false);
        setCssFollowPcc(false);
        setPCell2Rx(0);
        setPCell4Rx(0);
        setSCell2Rx(0);
        setSCell4Rx(0);
    }

    public boolean getForceRx() {
        return this.forceRx;
    }
    public int getIntForceRx() {
        if(forceRx) return 1;
        return 0;
    }
    public void setForceRx(boolean forceRx) {
        this.forceRx = forceRx;
    }
    public void setForceRx(int forceRx) {
        this.forceRx = forceRx == 1 ? true : false;
    }
    public boolean getCssFollowPcc() {
        return this.cssFollowPcc;
    }
    public int getIntCssFollowPcc() {
        if(cssFollowPcc) return 1;
        return 0;
    }
    public void setCssFollowPcc(boolean cssFollowPcc) {
        this.cssFollowPcc = cssFollowPcc;
    }
    public void setCssFollowPcc(int cssFollowPcc) {
        this.cssFollowPcc = cssFollowPcc == 1 ? true : false;
    }
    public int getPCell2Rx() {
        return booleanToInt(this.pCell2Rx);
    }
    public boolean[] getBoolArrayPCell2Rx() {
        return this.pCell2Rx;
    }
    public void setPCell2Rx(int pCell2Rx) {
        this.pCell2Rx = intToBoolean(pCell2Rx, 2);
    }
    public void updatePCell2Rx(boolean value, int pos) {
        if(pos >= this.pCell2Rx.length) return;
        this.pCell2Rx[pos] = value;
    }
    public int getPCell4Rx() {
        return booleanToInt(this.pCell4Rx);
    }
    public boolean[] getBoolArrayPCell4Rx() {
        return this.pCell4Rx;
    }
    public void setPCell4Rx(int pCell4Rx) {
        this.pCell4Rx = intToBoolean(pCell4Rx, 4);
    }
    public void updatePCell4Rx(boolean value, int pos) {
        if(pos >= this.pCell4Rx.length) return;
        this.pCell4Rx[pos] = value;
    }
    public int getSCell2Rx() {
        return booleanToInt(this.sCell2Rx);
    }
    public boolean[] getBoolArraySCell2Rx() {
        return this.sCell2Rx;
    }
    public void setSCell2Rx(int sCell2Rx) {
        this.sCell2Rx = intToBoolean(sCell2Rx, 2);
    }
    public void updateSCell2Rx(boolean value, int pos) {
        if(pos >= this.sCell2Rx.length) return;
        this.sCell2Rx[pos] = value;
    }
    public int getSCell4Rx() {
        return booleanToInt(this.sCell4Rx);
    }
    public boolean[] getBoolArraySCell4Rx() {
        return this.sCell4Rx;
    }
    public void setSCell4Rx(int sCell4Rx) {
        this.sCell4Rx = intToBoolean(sCell4Rx, 4);
    }
    public void updateSCell4Rx(boolean value, int pos) {
        if(pos >= this.sCell4Rx.length) return;
        this.sCell4Rx[pos] = value;
    }
}
