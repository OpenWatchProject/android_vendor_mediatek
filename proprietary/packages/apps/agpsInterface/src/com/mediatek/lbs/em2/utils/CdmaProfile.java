package com.mediatek.lbs.em2.utils;

import android.util.Log;

public class CdmaProfile {
    public final static int PDE_IP_TYPE_IPV4 = 0;
    public final static int PDE_IP_TYPE_IPV6 = 1;

    public String name;
    public boolean mcpEnable;
    public String mcpAddr;
    public int mcpPort;
    public boolean pdeAddrValid;
    public int pdeIpType; // refer to PDE_IP_TYPE_IPV4 or PDE_IP_TYPE_IPV6
    public String pdeAddr;
    public int pdePort;
    public boolean pdeUrlValid;
    public String pdeUrlAddr;

    public CdmaProfile() {
        name = "";
        mcpEnable = false;
        mcpAddr = "";
        mcpPort = 0;
        pdeAddrValid = false;
        pdeIpType = PDE_IP_TYPE_IPV4;
        pdeAddr = "";
        pdePort = 0;
        pdeUrlValid = false;
        pdeUrlAddr = "";
    }

    public boolean correctIfInvalid() {
        boolean ret = true;
        if (pdeIpType < PDE_IP_TYPE_IPV4 || pdeIpType > PDE_IP_TYPE_IPV6) {
            loge("pdeIpType=" + pdeIpType);
            pdeIpType = PDE_IP_TYPE_IPV4;
            ret = false;
        }
        return ret;
    }

    public String toString() {
        String ret = "";
        ret += "name=[" + name + "] ";
        ret += "mcpEnable=[" + mcpEnable + "] ";
        ret += "mcpAddr=[" + mcpAddr + "] ";
        ret += "mcpPort=[" + mcpPort + "] ";
        ret += "pdeAddrValid=[" + pdeAddrValid + "] ";
        if (pdeIpType == PDE_IP_TYPE_IPV4) {
            ret += "pdeIpType=[IPv4] ";
        } else if (pdeIpType == PDE_IP_TYPE_IPV6) {
            ret += "pdeIpType=[IPv6] ";
        } else {
            ret += "pdeIpType=[UNKNOWN " + pdeIpType + "] ";
        }
        ret += "pdeAddr=[" + pdeAddr + "] ";
        ret += "pdePort=[" + pdePort + "] ";
        ret += "pdeUrlValid=[" + pdeUrlValid + "] ";
        ret += "pdeUrlAddr=[" + pdeUrlAddr + "] ";
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
