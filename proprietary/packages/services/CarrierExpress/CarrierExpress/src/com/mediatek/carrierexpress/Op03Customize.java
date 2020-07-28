package com.mediatek.carrierexpress;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.mediatek.common.carrierexpress.CarrierExpressManager;

import java.util.ArrayList;
import java.util.List;

/**
 * Op03 operator customize class.
 */
public class Op03Customize extends CustomOperator {

    private static List <String> sOpSubIdList;
    private String mMccMnc;
    private Context mContext;
    static {
        sOpSubIdList = new ArrayList<String>();
        sOpSubIdList.add("" + CarrierExpressManager.OPERATOR_OP03_SUBID_DEFAULT);
        sOpSubIdList.add("" + CarrierExpressManager.OPERATOR_OP03_SUBID_1);
        sOpSubIdList.add("" + CarrierExpressManager.OPERATOR_OP03_SUBID_2);
        sOpSubIdList.add("" + CarrierExpressManager.OPERATOR_OP03_SUBID_3);
        sOpSubIdList.add("" + CarrierExpressManager.OPERATOR_OP03_SUBID_4);
        sOpSubIdList.add("" + CarrierExpressManager.OPERATOR_OP03_SUBID_5);
    }
    private final String TAG = "Op03Customize";

    /**
     * constructor of Op03Customize.
     * @param context from CarrierExpress Service.
     * @param String for operator id.
     */
    Op03Customize(Context context, String opId,
            String mccMnc, CarrierExpressServiceImplExt.ICarrierExpressServiceCallback callback) {
        super(context, opId, callback);
        mMccMnc = mccMnc;
        mContext = context;
    }

    /**
     * returns subid for OP03 operator.
     * @return int for sub id for operator
     */
    public int getDoubleCustomizableId() {
        String CurOptr = SystemProperties.get("persist.vendor.operator.optr");
        int CurSubId = SystemProperties.getInt("persist.vendor.operator.subid", 0);
        TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        String spn = tm.getSimOperatorName();
        Log.d(TAG, "sim operator SPN: " + spn);
        if ("Orange F".equals(spn)) {
            if ("20801".equals(mMccMnc)) {
                return CarrierExpressManager.OPERATOR_OP03_SUBID_1;
            }
        } else if ("Orange".equals(spn)) {
            if ("26003".equals(mMccMnc)) {
                return CarrierExpressManager.OPERATOR_OP03_SUBID_2;
            } else if ("22610".equals(mMccMnc)) {
                return CarrierExpressManager.OPERATOR_OP03_SUBID_3;
            } else if ("23101".equals(mMccMnc)) {
                return CarrierExpressManager.OPERATOR_OP03_SUBID_4;
            } else if ("21403".equals(mMccMnc)) {
                return CarrierExpressManager.OPERATOR_OP03_SUBID_5;
            }
        }
        // To skip any loop/cycle in switching
        if ("OP03".equals(CurOptr)) {
            return CurSubId;
        }
        return CarrierExpressManager.OPERATOR_OP03_SUBID_DEFAULT;
    }

    /**
     * returns subid list for OP03 operator.
     * @return List for sub id for operator
     */
    protected static List<String> getDoubleCustomizableIdList() {
        return sOpSubIdList;
    }
}