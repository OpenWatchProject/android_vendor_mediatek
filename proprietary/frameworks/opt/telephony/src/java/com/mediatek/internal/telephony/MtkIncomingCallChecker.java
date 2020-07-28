package com.mediatek.internal.telephony;

import com.android.internal.telephony.BlockChecker;
import com.android.internal.telephony.CallerInfo;
import android.content.Context;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.PersistableBundle;
import android.provider.BlockedNumberContract;
import android.telecom.TelecomManager;
import android.telephony.CarrierConfigManager;
import android.telephony.Rlog;
import mediatek.telephony.MtkCarrierConfigManager;

public class MtkIncomingCallChecker {

    private static final String PROP_LOG_TAG = "MTCallChecker";

    // create a token for querying contact
    private static final int TOKEN_MT_CHECKER = 1 << 8; // 256

    // async block checker component to avoid occupy main thread.
    AsyncBlockCheckTask blockChecker = null;
    OnCheckCompleteListener mCallback = null;
    String mName;
    Object obj = null;

    public MtkIncomingCallChecker(String name, Object obj) {
        mName = name;
        this.obj = obj;
    }

    /** Function used to check incoming call number.
     *  It will return false if it fails to start.
     *
     * @param context current context
     * @param subId subscription id to check the incoming call.
     * @param number the number to be checekd.
     * @return true if the check starts
     */
    public boolean startIncomingCallNumberCheck(Context context, int subId, String number,
            OnCheckCompleteListener callback) {

        if (context == null) {
            proprietaryLogE("cannot do checkIncomingCallNumber (context=null, subId=" + subId
                    + ", number=" + number + "), call will enter");
            return false;
        }
        if (callback == null) {
            proprietaryLogE("checkIncomingCallNumber callback null, call will enter");
            return false;
        }
        if (number == null || number.isEmpty()) {
            proprietaryLog("checkIncomingCallNumber skipped (number=" + number + ")");
            // if no number can be found, there's no need to continue the incoming call
            // number check in framework.
            return false;
        }

        // here the number should exist. following flow is for TelecomManager.PRESENTATION_ALLOWED
        mCallback = callback;

        if (blockChecker != null) {
            proprietaryLog("block checker not null (" +
                    blockChecker.toString() + "). will create a new one.");
        }
        blockChecker = new AsyncBlockCheckTask(context);

        if (isMtkEnhancedCallBlockingEnabled(context, subId)) {
            MtkCallerInfoAsyncQuery.startQuery(TOKEN_MT_CHECKER, context, number,
                    new MtkCallerInfoAsyncQuery.OnQueryCompleteListener() {
                        @Override
                        public void onQueryComplete(int token, Object cookie, CallerInfo info) {
                            boolean contactExists = info == null ? false : info.contactExists;
                            blockChecker.execute(number,
                                    String.valueOf(TelecomManager.PRESENTATION_ALLOWED),
                                    String.valueOf(contactExists));
                        }
                    }, "ContactQuery", subId);
            return true; // return true since the check starts
        }

        return false;
    }

    void onBlockCheckComplete(Boolean isBlocked) {
        proprietaryLog("query result, isBlocked=" + isBlocked);
        if (mCallback != null) {
            mCallback.onCheckComplete(isBlocked, this.obj);
        }
    }

    static boolean isMtkEnhancedCallBlockingEnabled(Context context, int subId) {
        if (context == null) {
            proprietaryLog("isMtkEnhancedCallBlockingEnabled fail, return false (context null)");
            return false;
        }
        CarrierConfigManager configManager = (CarrierConfigManager) context.getSystemService(
                Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle carrierConfig = configManager.getConfigForSubId(subId);
        if (carrierConfig == null) {
            carrierConfig = configManager.getDefaultConfig();
        }
        return carrierConfig.getBoolean(
                MtkCarrierConfigManager.MTK_KEY_SUPPORT_ENHANCED_CALL_BLOCKING_BOOL);
    }

    static void proprietaryLog(String s) {
        Rlog.d(PROP_LOG_TAG, s);
    }

    static void proprietaryLogE(String s) {
        Rlog.e(PROP_LOG_TAG, s);
    }

    static void proprietaryLogI(String s) {
        Rlog.i(PROP_LOG_TAG, s);
    }

    public interface OnCheckCompleteListener {
        void onCheckComplete(boolean result, Object obj);
    }

    // asynchronous block number check task
    class AsyncBlockCheckTask extends AsyncTask<String, Void, Boolean> {

        private Context mContext;

        public AsyncBlockCheckTask(Context context) {
            mContext = context;
        }

        @Override
        protected Boolean doInBackground(String... params) {
            String number = "";

            Bundle extras = new Bundle();
            if (params.length > 0) {
                number = params[0];
            }
            if (params.length > 1) {
                extras.putInt(BlockedNumberContract.EXTRA_CALL_PRESENTATION,
                        Integer.valueOf(params[1]));
            }
            if (params.length > 2) {
                extras.putBoolean(BlockedNumberContract.EXTRA_CONTACT_EXIST,
                        Boolean.valueOf(params[2]));
            }
            return BlockChecker.isBlocked(mContext, number, extras);
        }

        @Override
        protected void onPostExecute(Boolean isBlocked) {
            onBlockCheckComplete(isBlocked.booleanValue());
        }

    }
}
