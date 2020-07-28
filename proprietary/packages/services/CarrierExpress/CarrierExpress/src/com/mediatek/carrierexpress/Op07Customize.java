package com.mediatek.carrierexpress;

import android.content.Context;
import android.os.SystemProperties;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;

/**
 * Op07 operator customize class.
 */
public class Op07Customize extends CustomOperator {

    private static List <String> sOpSubIdList;
    static {
        sOpSubIdList = new ArrayList<String>();
        sOpSubIdList.add("0");
        sOpSubIdList.add("1");
    }
    private final String TAG = "Op07Customize";

    /**
     * constructor of Op07Customize.
     * @param context from CarrierExpress Service.
     * @param String for operator id.
     */
    Op07Customize(Context mContext, String opId, CarrierExpressServiceImplExt.ICarrierExpressServiceCallback callback) {
        super(mContext, opId, callback);
    }

    /**
     * returns subid for OP07 operator.
     * @return int for sub id for operator
     */
    public int getDoubleCustomizableId() {
        // 1. According to GSMA TS.32, the ICCID support is optional
        // 2. Follow GDPR, we do not get the ICCID as it is sensitive data
        //String iccId = getIccId();
        String CurOptr = SystemProperties.get("persist.vendor.operator.optr");
        int CurSubId = SystemProperties.getInt("persist.vendor.operator.subid", 0);
        //if (iccId == null) {
            Log.d(TAG, "CurOptr: " + CurOptr + " CurSubId: " + CurSubId);
            if ("OP07".equals(CurOptr)) {
                return CurSubId;
            } else {
                return 0; //AT&T
            }
        /*} else if (iccId.length() > 8 && iccId.charAt(8) != '0') {
            return 1;
        } else {
            return 0; //AT&T
        }*/
    }

    /**
     * returns subid list for OP07 operator.
     * @return List for sub id for operator
     */
    protected static List<String> getDoubleCustomizableIdList() {
        return sOpSubIdList;
    }

    /**
     * callback api for latched network change.
     * @param
     * @return
     */
    @Override
    protected void onParameterChange(CarrierExpressServiceImplExt.ICarrierExpressServiceCallback callback) {
        String currOpp = getCurrentOperatorName();
        callback.updateSubidValue(this);
        Log.d(TAG, "BroadcastReceiver(), SIM state change, current operator=" + currOpp);
    }
}