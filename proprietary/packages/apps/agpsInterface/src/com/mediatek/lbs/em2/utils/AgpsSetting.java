package com.mediatek.lbs.em2.utils;

import android.util.Log;

public class AgpsSetting {
    public final static int AGPS_PROTOCOL_UP = 0;
    public final static int AGPS_PROTOCOL_CP = 1;

    public boolean agpsEnable;
    public int agpsProtocol; // refer to AGPS_PROTOCOL_*
    public boolean gpevt;
    public boolean e911GpsIconEnable;
    public boolean e911OpenGpsEnable;
    public boolean tc10IgnoreFwConfig;
    public boolean lppeHideWifiBtStatus;
    public boolean lppeNetworkLocationDisable;
    public boolean agpsNvramEnable;
    public boolean lbsLogEnable;
    public int lppeCrowdSourceConfident;
    public boolean ignoreSiForE911; // North America operator 'V' asks us to ignore SI triggered by GMS
    public boolean useTc10Config;
    public boolean defaultNlpEnable;
    public int emergencyExtSecs;

    public AgpsSetting() {
        agpsEnable = false;
        agpsProtocol = AGPS_PROTOCOL_UP;
        gpevt = false;
        e911GpsIconEnable = false;
        e911OpenGpsEnable = false;
        tc10IgnoreFwConfig = false;
        lppeHideWifiBtStatus = false;
        lppeNetworkLocationDisable = false;
        agpsNvramEnable = false;
        lbsLogEnable = false;
        lppeCrowdSourceConfident = 90;
        ignoreSiForE911 = false;
        useTc10Config = false;
        defaultNlpEnable = false;
        emergencyExtSecs = 0;
    }

    public boolean correctIfInvalid() {
        boolean ret = true;
        if (agpsProtocol < AGPS_PROTOCOL_UP || agpsProtocol > AGPS_PROTOCOL_CP) {
            loge("agpsProtocol=" + agpsProtocol);
            agpsProtocol = AGPS_PROTOCOL_UP;
            ret = false;
        }
        return ret;
    }

    public String toString() {
        String ret = "";
        ret += "agpsEnable=[" + agpsEnable + "] ";
        if(agpsProtocol == AGPS_PROTOCOL_UP) {
            ret += "agpsProtocol=[UP] ";
        } else if(agpsProtocol == AGPS_PROTOCOL_CP) {
            ret += "agpsProtocol=[CP] ";
        } else {
            ret += "agpsProtocol=[UKNOWN " + agpsProtocol + "] ";
        }
        ret += "gpevt=[" + gpevt + "] ";
        ret += "e911GpsIconEnable=[" + e911GpsIconEnable + "] ";
        ret += "e911OpenGpsEnable=[" + e911OpenGpsEnable + "] ";
        ret += "tc10IgnoreFwConfig=[" + tc10IgnoreFwConfig + "] ";
        ret += "lppeHideWifiBtStatus=[" + lppeHideWifiBtStatus + "] ";
        ret += "lppeNetworkLocationDisable=[" + lppeNetworkLocationDisable + "] ";
        ret += "agpsNvramEnable=[" + agpsNvramEnable + "] ";
        ret += "lbsLogEnable=[" + lbsLogEnable + "] ";
        ret += "lppeCrowdSourceConfident=[" + lppeCrowdSourceConfident + "] ";
        ret += "ignoreSiForE911=[" + ignoreSiForE911 + "] ";
        ret += "useTc10Config=[" + useTc10Config + "] ";
        ret += "defaultNlpEnable=[" + defaultNlpEnable + "] ";
        ret += "emergencyExtSecs=[" + emergencyExtSecs + "] ";
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
