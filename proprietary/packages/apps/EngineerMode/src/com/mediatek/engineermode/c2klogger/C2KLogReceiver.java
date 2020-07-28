package com.mediatek.engineermode.c2klogger;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;

/**
 * @author MTK81255
 */
public class C2KLogReceiver extends BroadcastReceiver {
    public static final String ACTION_VIA_SET_ETS_DEV_C2KLOGGER =
            "via.cdma.action.set.ets.dev.c2klogger";
    public static final String ACTION_VIA_ETS_DEV_CHANGED_C2KLOGGER =
            "via.cdma.action.ets.dev.changed.c2klogger";
    public static final String ACTION_VIA_SET_ETS_DEV_BYPASS =
            "via.cdma.action.set.ets.dev";
    public static final String ACTION_VIA_ETS_DEV_CHANGED_BYPASS =
            "via.cdma.action.ets.dev.changed";
    public static final String EXTRAL_VIA_ETS_DEV = "via.cdma.extral.ets.dev";
    public static final String EXTRAL_VIA_ETS_DEV_RESULT =
            "set.ets.dev.result";
    private final static int MSG_SET_ETS_DEV_RESULT = 1;
    private Context mContext;
    private String mActionFrom;
    private Handler mMessageHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            int what = msg.what;
            Elog.d("saber", "mMessageHandler, what = " + what);
            if (what == MSG_SET_ETS_DEV_RESULT) {
                String resultAction = ACTION_VIA_SET_ETS_DEV_BYPASS.equals(mActionFrom) ?
                        ACTION_VIA_ETS_DEV_CHANGED_BYPASS : ACTION_VIA_ETS_DEV_CHANGED_C2KLOGGER;
                Intent intent = new Intent();
                intent.setAction(resultAction);
                AsyncResult ar = (AsyncResult) msg.obj;
                boolean isOK = (ar.exception == null);
                intent.putExtra(EXTRAL_VIA_ETS_DEV_RESULT, isOK);
                if (mContext != null) {
                    Elog.d("saber", "sendBroadcast : Action =  "
                            + resultAction
                            + " Extra C2KLogUtils.EXTRAL_VIA_ETS_DEV_RESULT = " + isOK);
                    mContext.sendBroadcast(intent, "android.permission.DUMP");
                }
            }
        }
    };

    @Override
    public void onReceive(Context context, Intent intent) {
        mContext = context;
        String action = intent.getAction();
        Elog.d("saber", " -->onReceive(), action=" + action);
        mActionFrom = action;
        if (ACTION_VIA_SET_ETS_DEV_C2KLOGGER.equals(action)
                || ACTION_VIA_SET_ETS_DEV_BYPASS.equals(action)) {
            int channel = intent.getIntExtra(EXTRAL_VIA_ETS_DEV, 0);
            Elog.d("saber", "channel = " + channel);

            String[] cmd = new String[3];
            cmd[0] = "AT+VTCHSW=1," + (channel == 1 ? 3 : 0);
            cmd[1] = "";
            cmd[2] = "DESTRILD:C2K";
            Elog.d("saber", "Set AT command " + cmd[0] + " to " + cmd[2]);
            EmUtils.invokeOemRilRequestStringsEm(true, cmd, mMessageHandler.obtainMessage
                    (MSG_SET_ETS_DEV_RESULT));
        }
    }
}
