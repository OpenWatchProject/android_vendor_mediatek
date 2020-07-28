package com.mediatek.engineermode.desenseat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.ModemCategory;


/**
 * Receiver for sending AT command.
 */
public class ATCSenderReceiver extends BroadcastReceiver {
    static final String ATC_SEND_ACTION = "com.mediatek.engineermode.desenseat.atc_send";
    static final String ATC_EXTRA_CMD = "atc_send.cmd";
    static final String ATC_EXTRA_MODEM_TYPE = "atc_send.modem";
    static final String ATC_EXTRA_MSG_ID = "atc_send.msgId";
    private static final int ATCMD_RESULT = 1;
    private static final String TAG = "DesenseAT/ATCSenderReceiver";

    private Handler mResponseHandler = new Handler() {
        public void handleMessage(final Message msg) {
            Elog.v(TAG, "responseHander receive a message: " + msg.what);

            AsyncResult asyncResult = (AsyncResult) msg.obj;
            switch (msg.what) {
                case ATCMD_RESULT: {
                    if (asyncResult.exception == null) {
                        BandTest.sAtcDone = true;
                        Elog.v(TAG, "succeed to execute at cmd");
                    } else {
                        Elog.i(TAG, "fail to execute at cmd");
                        BandTest.sAtcDone = false;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };


    @Override
    public void onReceive(Context context, Intent intent) {
        // TODO Auto-generated method stub
        if (ATC_SEND_ACTION.equals(intent.getAction())) {
            String param = intent.getStringExtra(ATC_EXTRA_CMD);
            boolean isCDMA = intent.getBooleanExtra(ATC_EXTRA_MODEM_TYPE, false);
            int msgId = intent.getIntExtra(ATC_EXTRA_MSG_ID, -1);
            Elog.d(TAG, "receive broadcast: ATC_SEND_ACTION and param is " + param +
                    " isCDMA: " + isCDMA);
            String[] cmd = new String[2];
            String[] cmdCdma = new String[3];
            cmdCdma[0] = param;
            cmdCdma[1] = "";
            cmdCdma[2] = "DESTRILD:C2K";
            String[] cmdCdmaArray = ModemCategory.getCdmaCmdArr(cmdCdma);
            cmd[0] = param;
            cmd[1] = "";

            EmUtils.invokeOemRilRequestStringsEm(isCDMA, isCDMA ? cmdCdmaArray : cmd,
                    mResponseHandler.obtainMessage(ATCMD_RESULT));
            Elog.d(TAG, "send cmd done");
        }
    }
}
