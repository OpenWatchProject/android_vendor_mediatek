package com.mediatek.engineermode.channellock;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

public class ChannelLockReceiver extends BroadcastReceiver {
    private static final String TAG = "ChannelLockReceiver";
    public static final String ACTION_CHANNELLOCK_CONFIG_QUERY =
            "com.mediatek.channellock.ACTION_CONFIG_QUERY";
    public static final String ACTION_CHANNELLOCK_CONFIG_CHANGE =
            "com.mediatek.channellock.ACTION_CONFIG_CHANGE";
    public static final String ACTION_CHANNELLOCK_CONFIG_CHANGE_RESULT =
            "com.mediatek.channellock.ACTION_CONFIG_CHANGE_RESULT";
    public static final String ACTION_CHANNELLOCK_CMD_CONFIG_QUERY_RESPONSE =
            "com.mediatek.channellock.ACTION_CONFIG_QUERY_RESPONSE";


    public static final String EXTRAL_CHANNELLOCK_ENABLED = "Enabled";
    public static final String EXTRAL_CHANNELLOCK_RAT = "RAT";
    public static final String EXTRAL_CHANNELLOCK_ARFCN = "ARFCN";
    public static final String EXTRAL_CHANNELLOCK_CELLID = "CELL_ID";
    public static final String EXTRAL_CHANNELLOCK_GSM1900 = "GSM1900";
    public static final String EXTRAL_CHANNELLOCK_RESULT = "set.channellock.result";
    private static final String CMD_CONFIG_QUERY = "AT+EMMCHLCK?";

    private static final int MSG_CHANNEL_LOCK = 7;
    private static final int MSG_QUERY_CHANNEL_LOCK = 8;

    private int mChannelLockEnabled = 0;
    private int mChannelLockRat = 0;
    private int mChannelLockARFCN = 0;
    private int mChannelLockCellId = 0;
    private int mChannelLockGsm1900 = 0;

    private Context mContext;

    private String mEMMCHLCKcommand = new String();

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();
        Elog.d(TAG, " -->onReceive(), action=" + action);

        if (ACTION_CHANNELLOCK_CONFIG_CHANGE.equals(action)) {
            mChannelLockEnabled = intent.getIntExtra(
                    EXTRAL_CHANNELLOCK_ENABLED, -1);
            mChannelLockRat = intent.getIntExtra(EXTRAL_CHANNELLOCK_RAT, -1);
            mChannelLockARFCN = intent
                    .getIntExtra(EXTRAL_CHANNELLOCK_ARFCN, -1);
            mChannelLockCellId = intent.getIntExtra(EXTRAL_CHANNELLOCK_CELLID,
                    -1);
            mChannelLockGsm1900 = intent.getIntExtra(
                    EXTRAL_CHANNELLOCK_GSM1900, -1);
            Elog.d(TAG, "mChannelLockEnabled = " + mChannelLockEnabled
                    + ",mChannelLockRat = "
                    + mChannelLockRat + ",mChannelLockARFCN = "
                    + mChannelLockARFCN
                    + ",mChannelLockCellId = " + mChannelLockCellId
                    + ",mChannelLockGsm1900 = " + mChannelLockGsm1900);

            // AT+EMMCHLCK=<mode>[,<act>,<band_indicator>, <arfcn>[,<cell_id>]]
            // Mandatory parameters: mode
            // Optional parameters: act[0|2|7], band_indicator[0|1], arfcn[int]
            // cell_id[0->511] (only utran and lte)
            mEMMCHLCKcommand = "AT+EMMCHLCK=";
            if (mChannelLockEnabled == 1) {
                // Start building the AT+EMMCHLCK= command
                // e.g. AT+EMMCHLCK=1,
                mEMMCHLCKcommand = mEMMCHLCKcommand + "1,";
                // Second parameter is the RAT: 0 GSM, 2, UTRAN, 7 LTE
                // e.g. AT+EMMCHLCK=1,7,
                if (mChannelLockRat == 0) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "0,";
                } else if (mChannelLockRat == 2) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "2,";
                } else if (mChannelLockRat == 7) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "7,";
                } else {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + ",";
                    Elog.e(TAG, " Error mChannelLockRat = " + mChannelLockRat
                            + ". Valid values 0, 2 and 7.");
                }
                // Third parameter band_indicator for GSM1900, 0 or 1
                // e.g. AT+EMMCHLCK=1,7,0,
                if (mChannelLockGsm1900 == 1) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "1,";
                } else if (mChannelLockGsm1900 == 0) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + "0,";
                } else {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + ",";
                    Elog.e(TAG, " Error mChannelLockGsm1900 = "
                            + mChannelLockGsm1900
                            + ". Valid values 0 and 1.");
                }

                // Fourth parameter arfcn, valid value 0 - 65535
                // e.g. AT+EMMCHLCK=1,7,0,10608
                if (mChannelLockARFCN != -1) {
                    mEMMCHLCKcommand = mEMMCHLCKcommand
                            + String.valueOf(mChannelLockARFCN)
                            + ",";
                } else {
                    mEMMCHLCKcommand = mEMMCHLCKcommand + ",";
                    Elog.e(TAG, " Error mChannelLockARFCN = "
                            + mChannelLockARFCN
                            + ". Valid values 0 to 65535.");
                }
                // cell_id is mandatory if RAT is UTRAN or LTE
                // e.g. AT+EMMCHLCK=1,7,0,10608,105
                if (mChannelLockRat == 2 || mChannelLockRat == 7) {
                    // Check that cell_id is defined
                    if (mChannelLockCellId != -1) {
                        mEMMCHLCKcommand = mEMMCHLCKcommand
                                + String.valueOf(mChannelLockCellId);
                    } else {
                        mEMMCHLCKcommand = mEMMCHLCKcommand + ",";
                        Elog.e(TAG, " Error mChannelLockCellId = " + mChannelLockCellId
                                + ". Valid values 0 to 65535.");
                    }
                }
                // If RAT is GSM, do not add parameter

            } else if (mChannelLockEnabled == 0) {
                mEMMCHLCKcommand = mEMMCHLCKcommand + "0";
            } else {
                mEMMCHLCKcommand = mEMMCHLCKcommand + ",";
                Elog.e(TAG, " Error mChannelLockEnabled = "
                        + mChannelLockEnabled);
            }
            sendATCommand(new String[] { mEMMCHLCKcommand, "" },
                    MSG_CHANNEL_LOCK);
        } else if (ACTION_CHANNELLOCK_CONFIG_QUERY.equals(action)) {
            sendATCommand(new String[] { CMD_CONFIG_QUERY, "+EMMCHLCK:" }, MSG_QUERY_CHANNEL_LOCK);
        }

    }

    private void sendATCommand(String[] atCommand, int msg) {
        EmUtils.invokeOemRilRequestStringsEm(atCommand, mATCmdHander.obtainMessage(msg));
    }

    private final Handler mATCmdHander = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Elog.d(TAG, "handleMessage: " + msg.what);
            AsyncResult ar = (AsyncResult) msg.obj;
            switch (msg.what) {
            case MSG_CHANNEL_LOCK:
                Intent intent = new Intent();
                intent.setAction(ACTION_CHANNELLOCK_CONFIG_CHANGE_RESULT);
                boolean isOK = false;
                if (ar.exception != null) {
                    Elog.e(TAG, ar.exception.getMessage());
                    isOK = false;
                    Elog.d(TAG, "send MSG_CHANNEL_LOCK failed");
                } else {
                    isOK = true;
                    Elog.d(TAG, "send MSG_CHANNEL_LOCK succeed");
                }
                intent.putExtra(EXTRAL_CHANNELLOCK_RESULT, isOK);
                if (mContext != null) {
                    Elog.d(TAG, "sendBroadcast result = " + isOK);
                  //  mContext.sendBroadcast(intent);
                }
                break;
            case MSG_QUERY_CHANNEL_LOCK:
                if (ar.exception != null) {
                    Elog.e(TAG, ar.exception.getMessage());
                    Elog.d(TAG, "send AT+EMMCHLCK? failed");
                } else {
                    Elog.d(TAG, "send AT+EMMCHLCK? succeed");
                    final String[] result = (String[]) ar.result;
                    String[] splited = null;
                    String strTemp = "";
                    Elog.d(TAG, result[0]);


                    try {
                        strTemp = result[0].substring("+EMMCHLCK:".length()).replaceAll(" ","");
                        splited = strTemp.split(",");
                    } catch (Exception e) {
                        Elog.e(TAG, "get the chekced label failed:" + e.getMessage());
                    }
                    for (int i = 0; splited != null && i < splited.length; i++) {
                        Elog.d(TAG, "splited[" + i + "] = " + splited[i]);
                    }

                    Intent intent_query = new Intent(ACTION_CHANNELLOCK_CMD_CONFIG_QUERY_RESPONSE);
                    intent_query.putExtra("Enabled", splited[0]);
                    if(splited.length == 5){
                        intent_query.putExtra("RAT", splited[1]);
                        intent_query.putExtra("GSM1900", splited[2]);
                        intent_query.putExtra("ARFCN",splited[3]);
                        intent_query.putExtra("CELL_ID", splited[4]);
                    }
                 //   mContext.sendBroadcast(intent_query);
                }
                break;
            default:
                break;
            }
        }
    };
}
