package com.mediatek.lbs.em2.utils;

import android.util.Log;

public class SuplProfile {
    public String name;
    public String addr;
    public int port;
    public boolean tls;
    public String mccMnc;
    public String appId;
    public String providerId;
    public String defaultApn;
    public String optionalApn;
    public String optionalApn2;
    public String addressType;

    public SuplProfile() {
        name = "";
        addr = "";
        port = 0;
        tls = false;
        mccMnc = "";
        appId = "";
        providerId = "";
        defaultApn = "";
        optionalApn = "";
        optionalApn2 = "";
        addressType = "";
    }

    public boolean correctIfInvalid() {
        return true;
    }

    public String toString() {
        String ret = "";
        ret += "name=[" + name + "] addr=[" + addr + "] port=[" + port
                + "] tls=[" + tls + "] ";
        if (!mccMnc.equals("")) {
            ret += "mccMnc=[" + mccMnc + "] ";
        }
        if (!appId.equals("")) {
            ret += "appId=[" + appId + "] ";
        }
        if (!providerId.equals("")) {
            ret += "providerId=[" + providerId + "] ";
        }
        if (!defaultApn.equals("")) {
            ret += "defaultApn=[" + defaultApn + "] ";
        }
        if (!optionalApn.equals("")) {
            ret += "optionalApn=[" + optionalApn + "] ";
        }
        if (!optionalApn2.equals("")) {
            ret += "optionalApn2=[" + optionalApn2 + "] ";
        }
        if (!addressType.equals("")) {
            ret += "addressType=[" + addressType + "] ";
        }
        return ret;
    }

    protected static void log(Object msg) {
        //System.out.println(message);
        Log.d("LocationEM [agps]:", msg.toString());
    }

    protected static void loge(Object msg) {
        //System.out.println("ERR: " + message);
        Log.d("LocationEM [agps] ERR:", msg.toString());
    }
}
