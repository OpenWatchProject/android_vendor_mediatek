package com.mediatek.wfo.ril;

import vendor.mediatek.hardware.mtkradioex.V1_0.IMwiRadioIndication;
import android.os.AsyncResult;
import android.os.RemoteException;

import java.util.ArrayList;

public class MwiRadioIndication extends IMwiRadioIndication.Stub {

    MwiRadioIndication(MwiRIL ril, int phoneId) {
        mRil= ril;
        mPhoneId = phoneId;
        mRil.riljLogv("MwiRadioIndication, phone = " + mPhoneId);
    }

    // IMS RIL Instance
    private MwiRIL mRil;
    // Phone Id
    private int mPhoneId;


    @Override
    public void onWifiMonitoringThreshouldChanged(int type,
            ArrayList<Integer> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_WIFI_RSSI_MONITORING_CONFIG, indStgs);
        }

        if (mRil.mRssiThresholdChangedRegistrants != null) {
            mRil.mRssiThresholdChangedRegistrants.notifyRegistrants(
                new AsyncResult(null, convertArrayListToIntArray(indStgs), null));
        }
    }

    @Override
    public void onWifiPdnActivate(int type, ArrayList<Integer> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_ACTIVE_WIFI_PDN_COUNT, indStgs);
        }

        if (mRil.mWifiPdnActivatedRegistrants != null) {
            mRil.mWifiPdnActivatedRegistrants.notifyRegistrants(
                new AsyncResult(null, convertArrayListToIntArray(indStgs), null));
        }
    }

    @Override
    public void onWfcPdnError(int type, ArrayList<Integer> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_WIFI_PDN_ERROR, indStgs);
        }

        if (mRil.mWifiPdnErrorRegistrants != null) {
            mRil.mWifiPdnErrorRegistrants.notifyRegistrants(
                new AsyncResult(null, convertArrayListToIntArray(indStgs), null));
        }
    }

    @Override
    public void onPdnHandover(int type, ArrayList<Integer> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_MOBILE_WIFI_HANDOVER, indStgs);
        }

        if (mRil.mWifiPdnHandoverRegistrants != null) {
            mRil.mWifiPdnHandoverRegistrants.notifyRegistrants(
                new AsyncResult(null, convertArrayListToIntArray(indStgs), null));
        }
    }

    @Override
    public void onWifiRoveout(int type, ArrayList<String> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_MOBILE_WIFI_ROVEOUT, indStgs);
        }
        indStgs.add(Integer.toString(mRil.mPhoneId));
        String[] ret = new String[indStgs.size()];
        ret = indStgs.toArray(ret);

        if (mRil.mWifiPdnRoveOutRegistrants != null) {
            mRil.mWifiPdnRoveOutRegistrants.notifyRegistrants(
                new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void onLocationRequest(int type, ArrayList<String> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLog(MwiRILConstants.RIL_UNSOL_REQUEST_GEO_LOCATION);
        }
        indStgs.add(Integer.toString(mRil.mPhoneId));
        String[] ret = new String[indStgs.size()];
        ret = indStgs.toArray(ret);

        if (mRil.mRequestGeoLocationRegistrants != null) {
            mRil.mRequestGeoLocationRegistrants.notifyRegistrants(
                new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void onWfcPdnStateChanged(int type, ArrayList<Integer> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_WFC_PDN_STATE, indStgs);
        }

        if (mRil.mWfcPdnStateChangedRegistrants != null) {
            mRil.mWfcPdnStateChangedRegistrants.notifyRegistrants(
                new AsyncResult(null, convertArrayListToIntArray(indStgs), null));
        }
    }

    @Override
    public void onNattKeepAliveChanged(int type, ArrayList<String> indStgs) {
        mRil.processIndication(type);
        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLog(MwiRILConstants.RIL_UNSOL_NATT_KEEP_ALIVE_CHANGED);
        }
        indStgs.add(Integer.toString(mRil.mPhoneId));
        String[] ret = new String[indStgs.size()];
        ret = indStgs.toArray(ret);
        if (mRil.mNattKeepAliveChangedRegistrants != null) {
            mRil.mNattKeepAliveChangedRegistrants.notifyRegistrants(
                new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void onWifiPdnOOS(int type, ArrayList<String> indStgs) {
        mRil.processIndication(type);

        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_WIFI_PDN_OOS, indStgs);
        }
        indStgs.add(Integer.toString(mRil.mPhoneId));
        String[] ret = new String[indStgs.size()];
        ret = indStgs.toArray(ret);

        if (mRil.mWifiPdnOosRegistrants != null) {
            mRil.mWifiPdnOosRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void onWifiLock(int type, ArrayList<String> indStgs) {
        mRil.processIndication(type);

        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_WIFI_LOCK, indStgs);
        }
        indStgs.add(Integer.toString(mRil.mPhoneId));
        String[] ret = new String[indStgs.size()];
        ret = indStgs.toArray(ret);

        if (mRil.mWifiLockRegistrants != null) {
            mRil.mWifiLockRegistrants.notifyRegistrants(new AsyncResult(null, ret, null));
        }
    }

    @Override
    public void onWifiPingRequest(int type, ArrayList<Integer> indStgs) {

        mRil.processIndication(type);

        if (MwiRIL.MWI_RILA_LOGD) {
            mRil.unsljLogRet(MwiRILConstants.RIL_UNSOL_WIFI_PING_REQUEST, indStgs);
        }

        if (mRil.mWifiPingRequestRegistrants != null) {
            mRil.mWifiPingRequestRegistrants.notifyRegistrants(
                new AsyncResult(null, convertArrayListToIntArray(indStgs), null));
        }
    }

    private int[] convertArrayListToIntArray(java.util.ArrayList<Integer> input) {
        int[] ret = new int[input.size()];
        for (int i = 0; i < input.size(); i++) {
            ret[i] = input.get(i);
        }
        return ret;
    }

}
