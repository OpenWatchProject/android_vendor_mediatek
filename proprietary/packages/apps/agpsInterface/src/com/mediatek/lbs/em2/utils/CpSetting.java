package com.mediatek.lbs.em2.utils;

import android.util.Log;

public class CpSetting {
    public final static int MOLR_POS_METHOD_LOC_EST = 0;
    public final static int MOLR_POS_METHOD_ASSIST_DATA = 1;

    public int molrPosMethod;
    public boolean externalAddrEnable;
    public String externalAddr;
    public boolean mlcNumberEnable;
    public String mlcNumber;
    public boolean cpAutoReset;
    public boolean epcMolrLppPayloadEnable;
    public byte[] epcMolrLppPayload;
    public boolean rejectNon911NilrEnable;
    public boolean cp2gDisable;
    public boolean cp3gDisable;
    public boolean cp4gDisable;
    public boolean cpLppeEnable;
    public boolean cpLppeSupport;
    public boolean cpLppeWlanEnable;
    public boolean cpLppeSrnEnable;
    public boolean cpLppeSensorEnable;
    public boolean cpLppeDbhEnable;

    public CpSetting() {
        molrPosMethod = MOLR_POS_METHOD_LOC_EST;
        externalAddrEnable = false;
        externalAddr = "";
        mlcNumberEnable = false;
        mlcNumber = "";
        cpAutoReset = false;
        epcMolrLppPayloadEnable = false;
        epcMolrLppPayload = new byte[0];
        rejectNon911NilrEnable = false;
        cp2gDisable = false;
        cp3gDisable = false;
        cp4gDisable = false;
        cpLppeEnable = false;
        cpLppeSupport = false;
        cpLppeWlanEnable = false;
        cpLppeSrnEnable = false;
        cpLppeSensorEnable = false;
        cpLppeDbhEnable = false;
    }

    public boolean correctIfInvalid() {
        boolean ret = true;
        if (molrPosMethod < MOLR_POS_METHOD_LOC_EST
                || molrPosMethod > MOLR_POS_METHOD_ASSIST_DATA) {
            molrPosMethod = MOLR_POS_METHOD_LOC_EST;
            ret = false;
        }
        return ret;
    }

    public String toString() {
        String ret = "";
        if(molrPosMethod == MOLR_POS_METHOD_LOC_EST) {
            ret += "molrPosMethod=[LOC_EST] ";
        } else if(molrPosMethod == MOLR_POS_METHOD_ASSIST_DATA) {
            ret += "molrPosMethod=[ASSIST_DATA] ";
        } else {
            ret += "molrPosMethod=[UNKNOWN " + molrPosMethod + "] ";
        }
        ret += "externalAddrEnable=[" + externalAddrEnable + "] ";
        ret += "externalAddr=[" + externalAddr + "] ";
        ret += "mlcNumberEnable=[" + mlcNumberEnable + "] ";
        ret += "mlcNumber=[" + mlcNumber + "] ";
        ret += "cpAutoReset=[" + cpAutoReset + "] ";
        ret += "rejectNon911NilrEnable=[" + rejectNon911NilrEnable + "] ";
        ret += "cp2gDisable=[" + cp2gDisable + "] ";
        ret += "cp3gDisable=[" + cp3gDisable + "] ";
        ret += "cp4gDisable=[" + cp4gDisable + "] ";
        ret += "cpLppeEnable=[" + cpLppeEnable + "] ";
        ret += "cpLppeSupport=[" + cpLppeSupport + "] ";
        ret += "cpLppeWlanEnable=[" + cpLppeWlanEnable + "] ";
        ret += "cpLppeSrnEnable=[" + cpLppeWlanEnable + "] ";
        ret += "cpLppeSensorEnable=[" + cpLppeWlanEnable + "] ";
        ret += "cpLppeDbhEnable=[" + cpLppeWlanEnable + "] ";
        ret += "epcMolrLppPayloadEnable=[" + epcMolrLppPayloadEnable + "] ";
        ret += "epcMolrLppPayload.len=[" + epcMolrLppPayload.length + "][";
        for (int i = 0; i < epcMolrLppPayload.length; i++) {
            ret += String.format("%02x", epcMolrLppPayload[i]);
        }
        ret += "]";
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
