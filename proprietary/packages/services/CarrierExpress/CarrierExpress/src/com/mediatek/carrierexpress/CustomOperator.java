package com.mediatek.carrierexpress;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.util.Log;
import com.android.internal.telephony.TelephonyIntents;

/**
 * Custom Operator.
 */
abstract class CustomOperator {
    private final String TAG = "CustomOperator";
    private TelephonyManager mTelephonyManager = null;
    private CarrierExpressServiceImplExt.ICarrierExpressServiceCallback mCarrierExpressCallback = null;
    private String mLastOperator = null;
    private final static String TELEPHONY_SERVICE_STATE = "state";
    private String mOpId;
    private final Context mContext;

    /**
     * constructor of Custom Operator.
     * @param context from CarrierExpress Service.
     * @param String for operator id.
     */
    CustomOperator(Context context, String opId, CarrierExpressServiceImplExt.ICarrierExpressServiceCallback callBack) {
        mOpId = opId;
        mCarrierExpressCallback = callBack;
        mContext = context;
        mTelephonyManager =
                    (TelephonyManager) mContext.getSystemService(mContext.TELEPHONY_SERVICE);
        IntentFilter filter = new IntentFilter();
        filter.addAction(TelephonyIntents.ACTION_SERVICE_STATE_CHANGED);
        mContext.registerReceiver(mIntentReceiver, filter);
    }

    @Override
    protected void finalize() {
        Log.d(TAG, "CustomOperator finalize");
    }

    public void unregisterReceiver() {
        Log.d(TAG, "unregisterReceiver()");
        if (null != mIntentReceiver) {
            mContext.unregisterReceiver(mIntentReceiver);
            mIntentReceiver = null;
        }
    }

    /**
     * Returns the current latched operator name.
     * Return null if device had not latched to any network
     * @param
     * @return String operator name
     */
    String getCurrentOperatorName() {
        String operatorName = mTelephonyManager.getNetworkOperatorName();
        return operatorName;
    }

    /**
     * Returns the service provider name of sim.
     * Return null if device does not contain any valid sim or sim is not ready
     * @param
     * @return String operator name
     */
    String getSpn() {
        String operatorName = mTelephonyManager.getSimOperatorName();
        return operatorName;
    }

    /**
     * Returns the ICCID of sim.
     * Return null if device does not contain any valid sim or sim is not ready
     * @param
     * @return String iccid
     */
    /**
     * 1. According to GSMA TS.32, the ICCID support is optional
     * 2. Follow GDPR, we do not get the ICCID as it is sensitive data
    String getIccId() {
        String iccId = mTelephonyManager.getSimSerialNumber();
        return iccId;
    }*/

    /**
     * Returns the Group Id level 1 of sim.
     * Return null if device does not contain any valid sim or sim is not ready
     * @param
     * @return String groupId
     */
    String getGid1() {
        String groupId = mTelephonyManager.getGroupIdLevel1();
        return groupId;
    }

    /**
     * It returns operator id used by CarrierExpress service.
     * @return String mOpId
     */
    public String getOperatorId() {
       return mOpId;
   }

    /**
     * This needs to be overridden by the specific operator class if
     * system property (feature option) noed to be overridden.
     * @param string name of the system property value of the system property
     * @return String new value of the systme propery (feature option)
     */
    protected String getPropertyValue(String key, String value) {
       return value;
    }
    /**
     * Returns the Sub id to configure active pack setting.
     * needs to be implemented by specific opearor class
     * @param
     * @return int subid.
     */
    abstract int getDoubleCustomizableId();

    protected BroadcastReceiver mIntentReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            int state = intent.getExtras().getInt("TELEPHONY_SERVICE_STATE");
            if (state == ServiceState.STATE_IN_SERVICE) {
                String optr = getCurrentOperatorName();
                if (!optr.equals("")) {
                    if (mLastOperator == null || !mLastOperator.equals(optr)) {
                        mLastOperator = optr;
                        onParameterChange(mCarrierExpressCallback);
                    }
                }
            }
        }
    };

    /**
     * This needs to be overrided by the specific customer class if they
     * need callback on latched network change.
     * @param
     * @return
     */
    protected void onParameterChange(CarrierExpressServiceImplExt.ICarrierExpressServiceCallback mCarrierExpressCallback) {

    }
}