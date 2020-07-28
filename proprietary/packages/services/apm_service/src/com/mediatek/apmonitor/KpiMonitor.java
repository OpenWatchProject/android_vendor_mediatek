package com.mediatek.apmonitor;

import com.mediatek.apmonitor.proxy.ApmServiceProxy;


import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

//import com.mediatek.apmonitor.ApmNative;

/**
 * Basic implementation for a KPI monitor, provide utility functions.
 */
public class KpiMonitor extends IApmKpiMonitor {
    private static final String TAG = "APM-KpiMonitor";
    private Context mContext = null;
    private Handler mHandler = null;
    //private ApmNative mApmNative = null;
    private String mName = TAG;

    protected static final int MSG_OFFSET_BASE = 0;
    protected static final int MSG_OFFSET_NETWORK = 200;
    protected static final int MSG_OFFSET_WIFI = 400;

    // Common Message ID
    protected static final int MSG_ID_APM_KPI_BROADCAST = MSG_OFFSET_BASE + 1;
    protected static final int MSG_ID_APM_KPI_STATE_CHANGED = MSG_OFFSET_BASE + 2;
    protected static final int MSG_ID_APM_KPI_ON_QUERY = MSG_OFFSET_BASE + 3;

    // Const
    public static final int EVENT_KPI_ADDED = 0;
    public static final int EVENT_KPI_REMOVED = 1;

    private KpiMonitor() {}

    public KpiMonitor(Context context, Handler handler, String name) {
        mContext = context;
        mHandler = handler;
        if (name != null && !name.isEmpty()) {
            setMonitorName(name);
        }
    }

    /**
     * Override this function to handle Message.
     *
     * @parm msg The message to be handled.
     *
     * @return Returns true if the Message got handled, or false if not handle it.
     */
    public boolean handleMessage(Message msg) {
        return false;
    }

    /**
     * Utility to get Application Context.
     *
     * @return Returns Application Context object.
     */
    public Context getContext() {
        return mContext;
    }

    /**
     * Utility to get Handler, KpiMonitor send Message to this handler,
     * and process the Message on handleMessage() callback function.
     *
     * @return Returns Handler object.
     */
    public Handler getHandler() {
        return mHandler;
    }

    /**
     * Set the name of KPI monitor for debug purpose.
     *
     * @param name The name of the KPI monitor.
     */
    void setMonitorName(String name) {
        mName = name;
    }

    /**
     * Return the name of KPI monitor for debug purpose.
     *
     * @return Returns the name of KPI monitor.
     */
    public String getMonitorName() {
        return mName;
    }

    /**
     * To extract KPI source from broadcast IApmKpiMonitor.ACTION_APM_KPI_EVENT.
     *
     * @param intent The Intent object
     *
     * @return Returns the KPI source defined in IApmKpiMonitor.KPI_SOURCE_*.
     */
    public int getIntentKpiSource(Intent intent) {
        return ApmServiceProxy.getApmKpiSource(intent);
    }

    /**
     * To extract KPI SIM ID from broadcast IApmKpiMonitor.ACTION_APM_KPI_EVENT.
     *
     * @param intent The Intent object
     *
     * @return Returns the SIMD ID of the KPI.
     */
    public int getIntentKpiSimId(Intent intent) {
        return ApmServiceProxy.getApmKpiSimId(intent);
    }

    /**
     * To extract KPI timestamp from broadcast IApmKpiMonitor.ACTION_APM_KPI_EVENT.
     *
     * @param intent The Intent object
     *
     * @return Returns the timestamp of the KPI.
     */
    public long getIntentKpiTimestampMs(Intent intent) {
        return ApmServiceProxy.getApmKpiTimestampMs(intent);
    }

    /**
     * Submit KPI through APM client.
     *
     * @param msgId Defined in interface ApmMsgDefs of libapm_msg_defs.
     * @param len Length of the data.
     * @param payload The KPI data to be submitted.
     *
     * @return Returns true if submit the KPI successfully.
     */
    public boolean apmSubmitKpi(Short msgId, int len, byte[] payload) {
        return apmSubmitKpiST(msgId, SIM_ID_DEFAULT, 0, len, payload);
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
        return apmSubmitKpiST(msgId, SIM_ID_DEFAULT, timestampMs, len, payload);
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
        ApmService service = (ApmService)getContext();
        byte id = (byte)simId;
        return service.getApmClient().apmSubmitKpiST(msgId, id, timestampMs, len, payload);
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
        ApmService service = (ApmService)getContext();
        return service.getApmClient().shouldSubmitKpi(msgId);
    }
}
