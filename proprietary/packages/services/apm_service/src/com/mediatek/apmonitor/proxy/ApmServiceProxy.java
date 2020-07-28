package com.mediatek.apmonitor.proxy;

import com.mediatek.apmonitor.IApmKpiMonitor;
import com.mediatek.apmonitor.proxy.IApmSessionCallback;

import android.content.Context;
import android.content.Intent;
import android.util.Log;

import java.util.ArrayList;
public class ApmServiceProxy {
    private static final String TAG = "APM-SessionJ";
    private ApmSession mApmSession = null;
    private IApmSessionCallback mSessionCallback = null;

    public ApmServiceProxy() {
        mApmSession = new ApmSession();
        mSessionCallback = new ApmSessionCallback();
        mApmSession.registControlCallback(mSessionCallback);
    }

    // APM HIDL Type
    public long getApmSessionParam() {
        return mApmSession.getApmSessionParam();
    }

    public boolean isApmSessionReady() {
        return mApmSession.isApmSessionReady();

    }

    /**
     * Submit KPI through APM client.
     *
     * @param msgId Defined in interface ApmMsgDefs of libapm_msg_defs.
     * @param len Length of the data.
     * @param pyalod The KPI data to be submitted.
     *
     * @return Returns true if submit the KPI successfully.
     */
    public boolean apmSubmitKpi(Short msgId, int len, byte[] payload) {
        return apmSubmitKpiST(msgId, IApmKpiMonitor.SIM_ID_DEFAULT, 0, len, payload);
    }

    /**
     * Submit KPI through APM client, "S" stands for SIM ID.
     *
     * @param msgId Defined in interface ApmMsgDefs of libapm_msg_defs.
     * @param simId The SIM ID submitted with the KPI, if the KPI is not related to the SIM,
     *              please use the apmSubmitKpiX() without "S".
     * @param len Length of the data.
     * @param payload The KPI data to be submitted.
     *
     * @return Returns true if submit the KPI successfully.
     */
    public boolean apmSubmitKpiS(Short msgId, int simId, int len, byte[] payload) {
        return apmSubmitKpiST(msgId, simId, 0, len, payload);
    }

    /**
     * Submit KPI through APM client, "T" means the epoch timestamp in milliseconds.
     *
     * @param msgId Defined in interface ApmMsgDefs of libapm_msg_defs.
     * @param timestampMs The timestamp binding with the KPI, epoch from 1970/01/01 00:00:00.
     *                    Caller can get current time by API java.lang.System.currentTimeMillis().
     * @param len Length of the data.
     * @param payload The KPI data to be submitted.
     *
     * @return Returns true if submit the KPI successfully.
     */
    public boolean apmSubmitKpiT(Short msgId, long timestampMs, int len, byte[] payload) {
        return apmSubmitKpiST(msgId, IApmKpiMonitor.SIM_ID_DEFAULT, timestampMs, len, payload);
    }

    /**
     * Submit KPI through APM client with SIM ID and epoch timestamp information.
     *
     * @param msgId Defined in interface ApmMsgDefs of libapm_msg_defs.
     * @param simId The SIM ID submitted with the KPI, if the KPI is not related to the SIM,
     *              please use the apmSubmitKpiX() without "S".
     * @param timestampMs The timestamp binding with the KPI, epoch from 1970/01/01 00:00:00.
     *                    Caller can get current time by API java.lang.System.currentTimeMillis().
     * @param len Length of the data.
     * @param payload The KPI data to be submitted.
     *
     * @return Returns true if submit the KPI successfully.
     */
    public boolean apmSubmitKpiST(Short msgId, int simId, long timestampMs, int len, byte[] payload) {
        byte id = (byte)simId;
        Log.d(TAG, "[" + mApmSession.getApmSessionId() + "]apmSubmitKpiST(" + msgId + ")");
        return mApmSession.apmSubmitKpiST(msgId, id, timestampMs, len, payload);
    }

    /**
     * Submission layer check this API to decide to submit KPI or not, however,
     * This API is being called inside apmSubmitKpiXXX(), as the result,
     * submission layer can use this API to avoid overhead if necessary.
     *
     * @param msgId Defined in interface ApmMsgDefs of libapm_msg_defs.
     *
     * @return Returns true to tell submission layer to submit the KPI.
     */
    public boolean apmShouldSubmitKpi(Short msgId) {
        return mApmSession.shouldSubmitKpi(msgId);
    }

    // Broadcast Type Interface
    /**
     * To retrieve a Intent, a content to have KPI data.
     * Assign source to indicate where the KPI come from. (Defined source ID in IApmKpiMonitor)
     * Update current timestamp to this KPI as default.
     * Developer can update the timestamp by API setApmKpiTimestampMs() later.
     *
     * @param source Source type of the KPI, defined in IApmKpiMonitor.KPI_SOURCE_XXX.
     *
     * @return Returns APM KPI Intent to be sent, including basic information.
     */
    public static Intent getApmKpiIntent(int source) {
        Intent apmIntent = new Intent(IApmKpiMonitor.ACTION_APM_KPI_EVENT);

        if (source <= IApmKpiMonitor.KPI_SOURCE_UNKNOWN) {
            return null;
        }
        apmIntent.putExtra(IApmKpiMonitor.EXTRA_KPI_SOURCE, source);
        setApmKpiTimestampMs(apmIntent, System.currentTimeMillis());

        return apmIntent;
    }

    /**
     * To send KPI intent, must use this API because we limited the receiver by Android permission,
     * only the application have “com.mediatek.apmonitor.APM_KPI_EVENT_PERMISSION” permission
     * can received the broadcast.
     *
     * @param context android.content.Context to send the broadcast.
     * @param intent The APM KPI Intent to be sent to ApmService.
     *
     */
    public static void sendApmKpiIntent(Context context, Intent intent) {
        if (context != null && intent != null) {
            if (isApmKpiIntent(intent)) {
                Log.d(TAG, "sendApmKpiIntent(" + getApmKpiSource(intent) + ")");
                context.sendBroadcast(intent, IApmKpiMonitor.RECEIVE_APM_KPI_PERMISSION);
            }
        }
    }

    /**
     * Get the source type of the KPI intent, defined in IApmKpiMonitor.KPI_SOURCE_XXX.
     *
     * @param intent The APM KPI intent to be checked.
     *
     * @return Returns the source of the APM KPI Intent.
     *
     */
    public static int getApmKpiSource(Intent target) {
        if (target != null) {
            if (isApmKpiIntent(target)) {
                return target.getIntExtra(IApmKpiMonitor.EXTRA_KPI_SOURCE, IApmKpiMonitor.KPI_SOURCE_UNKNOWN);
            }
        }
        return IApmKpiMonitor.KPI_SOURCE_UNKNOWN;
    }

    /**
     * Optional, set SIM ID binding with the KPI if required.
     *
     * @param intent The APM KPI intent to be sent.
     * @param simId The SIM ID binding with the KPI.
     *
     * @return Returns the APM KPI Intent to be sent to ApmService.
     *
     */
    public static Intent setApmKpiSimId(Intent target, int simId) {
        if (target != null) {
            if (isApmKpiIntent(target)) {
                target.putExtra(IApmKpiMonitor.EXTRA_KPI_SIMID, simId);
            }
        }
        return target;
    }

    /**
     * Get the SIM ID of the KPI intent.
     *
     * @param intent The APM KPI intent to be sent.
     *
     * @return Returns the SIMD ID binding with the KPI, and
     * return IApmKpiMonitor.KPI_SIMID_UNKNOWN if EXTRA_KPI_SIMID not set.
     *
     */
    public static int getApmKpiSimId(Intent target) {
        if (target != null) {
            if (isApmKpiIntent(target)) {
                return target.getIntExtra(IApmKpiMonitor.EXTRA_KPI_SIMID, IApmKpiMonitor.KPI_SIMID_UNKNOWN);
            }
        }
        return IApmKpiMonitor.KPI_SIMID_UNKNOWN;
    }

    /**
     * Optional, Set the timestamp to the KPI, epoch milliseconds from 1970/01/01 00:00:00.
     *
     * @param intent The APM KPI intent to be sent.
     * @param timestamp The timestamp in milliseconds binding with the KPI.
     *
     * @return Returns the APM KPI Intent to be sent to ApmService.
     *
     */
    public static Intent setApmKpiTimestampMs(Intent target, long timestamp) {
        if (target != null) {
            if (isApmKpiIntent(target)) {
                target.putExtra(IApmKpiMonitor.EXTRA_KPI_TIMESTAMP, timestamp);
            }
        }
        return target;
    }

    /**
     * Get the timestamp of the KPI intent, epoch milliseconds from 1970/01/01 00:00:00.
     *
     * @param intent The APM KPI intent to be sent.
     *
     * @return Returns timestamp in milliseconds binding with the KPI.
     *
     */
    public static long getApmKpiTimestampMs(Intent target) {
        if (target != null) {
            if (isApmKpiIntent(target)) {
                target.getLongExtra(IApmKpiMonitor.EXTRA_KPI_TIMESTAMP, IApmKpiMonitor.KPI_INVALID_TIMESTAMP);
            }
        }
        return IApmKpiMonitor.KPI_INVALID_TIMESTAMP;
    }

    class ApmSessionCallback implements IApmSessionCallback {
        @Override
        public void onQueryKpi(Short msgId) {
        }
        @Override
        public void onKpiSubscriptionStateChanged(ArrayList<Short> subscribedMessageList) {
        }
    }
    private static boolean isApmKpiIntent(Intent intent) {
        if (intent == null) {
            return false;
        }
        String action = intent.getAction();
        return IApmKpiMonitor.ACTION_APM_KPI_EVENT.equals(action);
    }
}
