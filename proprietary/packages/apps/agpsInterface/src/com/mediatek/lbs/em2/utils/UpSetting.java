package com.mediatek.lbs.em2.utils;

import android.util.Log;

public class UpSetting {
    public final static int CDMA_PREFERRED_WCDMA = 0;
    public final static int CDMA_PREFERRED_CDMA = 1;
    public final static int CDMA_PREFERRED_CDMA_FORCE = 2;

    public final static int PREF_METHOD_MSA = 0;
    public final static int PREF_METHOD_MSB = 1;
    public final static int PREF_METHOD_NO_PREF = 2;

    public boolean caEnable;
    public boolean niRequest;
    public boolean roaming;
    public int cdmaPreferred; // refer to CDMA_PREFERRED_*
    public int prefMethod; // refer to PREF_METHOD_*
    public int suplVersion;
    public int tlsVersion;
    public boolean suplLog;
    public boolean msaEnable;
    public boolean msbEnable;
    public boolean ecidEnable;
    public boolean otdoaEnable;
    public int qopHacc;
    public int qopVacc;
    public int qopLocAge;
    public int qopDelay;
    public boolean lppEnable;
    public boolean certFromSdcard;
    public boolean autoProfileEnable;
    public byte ut2;
    public byte ut3;
    public boolean apnEnable;
    public boolean syncToslp;
    public boolean udpEnable;
    public boolean autonomousEnable;
    public boolean afltEnable;
    public boolean imsiEnable;
    public byte suplVerMinor;
    public byte suplVerSerInd;
    public int shaVersion;
    public int preferred2g3gCellAge;
    public byte ut1;
    public boolean noSensitiveLog;
    public boolean tlsReuseEnable;
    public boolean imsiCacheEnable;
    public boolean suplRawDataEnable;
    public boolean tc10Enable;
    public boolean tc10UseApn;
    public boolean tc10UseFwDns;
    public boolean allowNiForGpsOff;
    public boolean forceOtdoaAssistReq;
    public boolean upLppeEnable;
    public int esuplApnMode;
    public int tcpKeepAlive;
    public boolean aospProfileEnable;
    public boolean bindNlpSettingToSupl;

    public boolean upLppeWlanEnable;
    public boolean upLppeSrnEnable;
    public boolean upLppeSensorEnable;
    public boolean upLppeDbhEnable;
    public int ipVersionPrefer;            //0=IPv6 prefer   1=IPv4 prefer
    public boolean upLppIn2g3gDisable; // For ATT SUPL server
    public boolean upRrlpIn4gDisable;  // For ATT SUPL server
    public boolean upSiDisable;          // For Sprint
    public boolean useNiSlp;
    public boolean aospPosModeEnable;
    public int privacyOverrideMode;

    public UpSetting() {
        caEnable = false;
        niRequest = false;
        roaming = false;
        cdmaPreferred = CDMA_PREFERRED_WCDMA;
        prefMethod = PREF_METHOD_MSB;
        suplVersion = 1;
        tlsVersion = 0;
        suplLog = false;
        msaEnable = false;
        msbEnable = false;
        ecidEnable = false;
        otdoaEnable = false;
        qopHacc = 0;
        qopVacc = 0;
        qopLocAge = 0;
        qopDelay = 0;
        lppEnable = false;
        certFromSdcard = false;
        autoProfileEnable = false;
        ut1 = 11;
        ut2 = 11;
        ut3 = 10;
        apnEnable = false;
        syncToslp = false;
        udpEnable = false;
        autonomousEnable = false;
        afltEnable = false;
        imsiEnable = false;
        suplVerMinor  = 0;
        suplVerSerInd = 0;
        shaVersion = 0;
        preferred2g3gCellAge = 0;
        noSensitiveLog = false;
        tlsReuseEnable = false;
        imsiCacheEnable = false;
        suplRawDataEnable = false;
        tc10Enable = false;
        tc10UseApn = false;
        tc10UseFwDns = false;
        allowNiForGpsOff = false;
        forceOtdoaAssistReq = false;
        upLppeEnable = false;

        esuplApnMode = 0;
        tcpKeepAlive = 0;
        aospProfileEnable = false;
        bindNlpSettingToSupl = false;

        upLppeWlanEnable = false;
        upLppeSrnEnable = false;
        upLppeSensorEnable = false;
        upLppeDbhEnable = false;
        ipVersionPrefer = 0;
        upLppIn2g3gDisable = false;
        upRrlpIn4gDisable = false;
        upSiDisable = false;
        useNiSlp = false;
        aospPosModeEnable = false;
        privacyOverrideMode = 0;
    }

    public boolean correctIfInvalid() {
        boolean ret = true;
        if (cdmaPreferred < CDMA_PREFERRED_WCDMA
                || cdmaPreferred > CDMA_PREFERRED_CDMA_FORCE) {
            loge("cdmaPreferred=" + cdmaPreferred);
            cdmaPreferred = CDMA_PREFERRED_WCDMA;
            ret = false;
        }
        if (prefMethod < PREF_METHOD_MSA || prefMethod > PREF_METHOD_NO_PREF) {
            loge("prefMethod=" + prefMethod);
            prefMethod = PREF_METHOD_MSB;
            ret = false;
        }
        if (suplVersion < 1 || suplVersion > 2) {
            loge("suplVersion=" + suplVersion);
            suplVersion = 1;
            ret = false;
        }
        if(tlsVersion < 0 || tlsVersion > 2) {
            loge("tlsVersion=" + tlsVersion);
            tlsVersion = 0;
            ret = false;
        }
        if(suplVerMinor < 0 || suplVerMinor > 255) {
            loge("suplVerMinor=" + suplVerMinor);
            suplVerMinor = 0;
            ret = false;
        }
        if(suplVerSerInd < 0 || suplVerSerInd > 255) {
            loge("suplVerSerInd=" + suplVerSerInd);
            suplVerSerInd = 0;
            ret = false;
        }
        if (shaVersion < 0 || shaVersion > 2) {
            loge("shaVersion=" + shaVersion);
            shaVersion = 0;
            ret = false;
        }
        return ret;
    }

    public String toString() {
        String ret = "";
        ret += "caEnable=[" + caEnable + "] ";
        ret += "niRequest=[" + niRequest + "] ";
        ret += "roaming=[" + roaming + "] ";
        if (cdmaPreferred == CDMA_PREFERRED_WCDMA) {
            ret += "cdmaPreferred=[WCDMA] ";
        } else if (cdmaPreferred == CDMA_PREFERRED_CDMA) {
            ret += "cdmaPreferred=[CDMA] ";
        } else if (cdmaPreferred == CDMA_PREFERRED_CDMA_FORCE) {
            ret += "cdmaPreferred=[CDMA_FORCE] ";
        } else {
            ret += "cdmaPreferred=[UNKNOWN " + cdmaPreferred + "] ";
        }
        if (prefMethod == PREF_METHOD_MSA) {
            ret += "prefMethod=[MSA] ";
        } else if (prefMethod == PREF_METHOD_MSB) {
            ret += "prefMethod=[MSB] ";
        } else if (prefMethod == PREF_METHOD_NO_PREF) {
            ret += "prefMethod=[NO_PREF] ";
        } else {
            ret += "prefMethod=[UNKNOWN " + prefMethod + "] ";
        }
        ret += "suplVersion=[" + suplVersion + "] ";
        ret += "tlsVersion=[" + tlsVersion + "] ";
        ret += "suplLog=[" + suplLog + "] ";
        ret += "msaEnable=[" + msaEnable + "] ";
        ret += "msbEnable=[" + msbEnable + "] ";
        ret += "ecidEnable=[" + ecidEnable + "] ";
        ret += "otdoaEnable=[" + otdoaEnable + "] ";
        ret += "qopHacc=[" + qopHacc + "] ";
        ret += "qopVacc=[" + qopVacc + "] ";
        ret += "qopLocAge=[" + qopLocAge + "] ";
        ret += "qopDelay=[" + qopDelay + "] ";
        ret += "lppEnable=[" + lppEnable + "] ";
        ret += "certFromSdcard=[" + certFromSdcard + "] ";
        ret += "autoProfileEnable=[" + autoProfileEnable + "] ";
        ret += "ut1=[" + ut1 + "] ";
        ret += "ut2=[" + ut2 + "] ";
        ret += "ut3=[" + ut3 + "] ";
        ret += "apnEnable=[" + apnEnable + "] ";
        ret += "syncToslp=[" + syncToslp + "] ";
        ret += "udpEnable=[" + udpEnable + "] ";
        ret += "autonomousEnable=[" + autonomousEnable + "] ";
        ret += "afltEnable=[" + afltEnable + "] ";
        ret += "imsiEnable=[" + imsiEnable + "] ";
        ret += "suplVerMinor=[" + suplVerMinor + "] ";
        ret += "suplVerSerInd=[" + suplVerSerInd + "] ";
        ret += "shaVersion=[" + shaVersion + "] ";
        ret += "preferred2g3gCellAge=[" + preferred2g3gCellAge + "] ";
        ret += "noSensitiveLog=[" + noSensitiveLog + "] ";
        ret += "tlsReuseEnable=[" + tlsReuseEnable + "] ";
        ret += "imsiCacheEnable=[" + imsiCacheEnable + "] ";
        ret += "suplRawDataEnable=[" + suplRawDataEnable + "] ";
        ret += "tc10Enable=[" + tc10Enable + "] ";
        ret += "tc10UseApn=[" + tc10UseApn + "] ";
        ret += "tc10UseFwDns=[" + tc10UseFwDns + "] ";
        ret += "allowNiForGpsOff=[" + allowNiForGpsOff + "] ";
        ret += "forceOtdoaAssistReq=[" + forceOtdoaAssistReq + "] ";
        ret += "upLppeEnable=[" + upLppeEnable + "] ";
        ret += "esuplApnMode=[" + esuplApnMode + "] ";
        ret += "tcpKeepAlive=[" + tcpKeepAlive + "] ";
        ret += "aospProfileEnable=[" + aospProfileEnable + "] ";
        ret += "bindNlpSettingToSupl=[" + bindNlpSettingToSupl + "] ";
        ret += "upLppeWlanEnable=[" + upLppeWlanEnable + "] ";
        ret += "upLppeSrnEnable=[" + upLppeSrnEnable + "] ";
        ret += "upLppeSensorEnable=[" + upLppeSensorEnable + "] ";
        ret += "upLppeDbhEnable=[" + upLppeDbhEnable + "] ";
        ret += "ipVersionPrefer=[" + ipVersionPrefer + "] ";
        ret += "upLppIn2g3gDisable=[" + upLppIn2g3gDisable + "] ";
        ret += "upRrlpIn4gDisable=[" + upRrlpIn4gDisable + "] ";
        ret += "upSiDisable=[" + upSiDisable + "] ";
        ret += "useNiSlp=[" + useNiSlp + "] ";
        ret += "aospPosModeEnable=[" + aospPosModeEnable + "] ";
        ret += "privacyOverrideMode=[" + privacyOverrideMode + "] ";
        return ret;
    }

    protected static void log(Object msg) {
        //System.out.println(msg);
        Log.d("LocationEM [agps]:", msg.toString());
    }

    protected static void loge(Object msg) {
        //System.out.println("ERR: " + message);
        Log.d("LocationEM [agps] ERR:", msg.toString());
    }
}
