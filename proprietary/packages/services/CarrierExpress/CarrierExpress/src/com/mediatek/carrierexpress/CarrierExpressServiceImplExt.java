package com.mediatek.carrierexpress;

import android.content.Context;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;

import java.util.ArrayList;
import java.util.List;


/**
 * CarrierExpressServiceImplExt.
 */
public class CarrierExpressServiceImplExt {
    private final String TAG = "CarrierExpressServiceImplExt";
    private CustomOperator mCustomOpClass;
    private Context mContext;
    private CarrierExpressServiceImpl mSrv;

    /**
     * constructor of Custom Operator.
     * @param context from CarrierExpress Service.
     * @param String for operator id.
     */
    CarrierExpressServiceImplExt(Context context, CarrierExpressServiceImpl srv) {
        mContext = context;
        // This is new operator
        mCustomOpClass = null;
        mSrv = srv;
    }

    protected List<String> getOperatorSubIdList(String optr) {
        switch (optr) {
        case "OP07":
            return Op07Customize.getDoubleCustomizableIdList();
        case "OP03":
            return Op03Customize.getDoubleCustomizableIdList();
        default:
            List <String> lst = new ArrayList<String>();
            lst.add("0");
            return lst;
        }
    }

    protected String getOperatorSubId() {
        int id = 0;
        // need to check which operator (current or new) need to invoke
        if (mCustomOpClass != null) {
            id = mCustomOpClass.getDoubleCustomizableId();
            Log.d(TAG, "getCustomizableId " + id);
        }
        return "" + id;
    }

    /** fetch current operator class.
     * @param String optr
     * @return CustomOperator
     */

    protected void instantiateOperatorClass(String optr, String mccMnc) {
        if (mCustomOpClass !=  null &&
                mCustomOpClass.getOperatorId().equals(optr)) {
           Log.d(TAG, "instantiateOperatorClass already found: " + optr);
           return;
        }
        ICarrierExpressServiceCallback CarrierExpressCallback = new ICarrierExpressServiceCallback() {
            public void updateSubidValue(CustomOperator obj) {
                Log.d(TAG, "enter in updateSubidValue");
                String subID = "" + obj.getDoubleCustomizableId();
                if (!subID.equals("-1")) {
                    // Not Allow Switching when switch mode is 3
                    String simSwitchMode = SystemProperties.get("persist.vendor.mtk_usp_switch_mode");
                    if (simSwitchMode != null && simSwitchMode.equals("3")) {
                        Log.d(TAG, "sim switch not allowed mode is: " + simSwitchMode);
                        return;
                    }
                    mSrv.handleSwitchOperator(obj.getOperatorId(), subID);
                }
            }
        };
        switch (optr) {
            case "OP07":
                mCustomOpClass = new Op07Customize(mContext, optr, CarrierExpressCallback);
                Log.d(TAG, "instantiateOperatorClass instance created: " + optr);
                break;
            case "OP03":
                mCustomOpClass = new Op03Customize(mContext, optr, mccMnc, CarrierExpressCallback);
                Log.d(TAG, "instantiateOperatorClass instance created: " + optr);
                break;
            default:
                Log.d(TAG, "instantiateOperatorClass optr NOT FOUND");
                if(mCustomOpClass != null) {
                    mCustomOpClass.unregisterReceiver();
                    mCustomOpClass = null;
                }
        }
    }

    boolean isToPreventOperatorSwitching(String optr) {
        if ("OP06".equals(optr)) {
            TelephonyManager tm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
            String spn = tm.getSimOperatorName();
            // When spn is empty or voda then do switching for OP06 only
            // Vodafone_R21_TCD-SMTC-REQ-012350, Vodafone_R21_TCD-SMTC-REQ-012351,
            // Vodafone_R21_TCD-SMTC-REQ-012354, Vodafone_R21_TCD-SMTC-REQ-012355,
            // Vodafone_R21_TCD-SMTC-REQ-012356, Vodafone_R21_TCD-SMTC-REQ-012352,
            // Vodafone_R21_TCD-SMTC-REQ-012353
            if (spn != null && spn.length() > 0 && !spn.regionMatches(true,0,"voda",0,4)) {
                return true;
            }
        }
        return false;
    }

    // APIs to override here

    // sbp code override here if needed
    protected String getModemSbpCode(String sbpCode) {
        // For now not overriden
        return sbpCode;
    }

    // system property override here
    protected String getPropertyValue(String propName, String propValue) {
        if (mCustomOpClass != null) {
            return mCustomOpClass.getPropertyValue(propName, propValue);
        }
        return propValue;
    }

    /**
    * Interface ICarrierExpressServiceCallback used for callback from operator class.
    */
    interface ICarrierExpressServiceCallback {
        void updateSubidValue(CustomOperator obj);
    }
}