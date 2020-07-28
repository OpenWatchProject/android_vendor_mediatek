package com.mediatek.op18.incallui;

import android.app.Notification;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.graphics.Bitmap;
import android.graphics.drawable.Icon;
import android.telecom.Call.Details;
import android.util.Log;

import com.android.incallui.StatusBarNotifier;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;
//import com.android.ims.ImsManager;

import com.mediatek.incallui.ext.DefaultStatusBarExt;
import com.mediatek.op18.dialer.R;

/**
 * Plug in implementation for OP18 StatusBar interfaces.
 */
public class OP18StatusBarExt extends DefaultStatusBarExt {
    private static final String TAG = "OP18StatusBarExt";
    private Context mContext;
    private StatusBarNotifier mStatusBarNotifier;
    private static final int ONHOLD = 8;
    private static final int MIN_PKT_LOST = 0;
    private static final int REC_MAX_PKT_LOST = 100;
    private static final int REC_MED1_PKT_LOST = 33;
    private static final int REC_MED2_PKT_LOST = 66;
    private int mReceivePktLost;
    private static final String ACTION_IMS_RTP_INFO = "com.android.ims.IMS_RTP_INFO";
    private static final String EXTRA_RTP_RECV_PKT_LOST = "android:rtpRecvPktLost";

    /** Constructor.
     * @param context context
     */
    public OP18StatusBarExt(Context context) {
        mContext = context;
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_IMS_RTP_INFO);
        mContext.registerReceiver(mReceiver, filter);
    }

    @Override
    public void getStatusBarNotifier(Object statusBarNotifier) {
        Log.d(TAG, "getStatusBarNotifier");
        mStatusBarNotifier = (StatusBarNotifier) statusBarNotifier;
    }

    @Override
    public boolean needUpdateNotification() {
        Log.d(TAG, "needUpdateNotification");
        return true;
    }

    @Override
    public void customizeNotification(Notification.Builder builder, Bitmap largeIcon) {
        final DialerCall call = CallList.getInstance().getActiveOrBackgroundCall();
        Log.d(TAG, "customizeNotification call " + call);
        if (call != null && call.hasProperty(Details.PROPERTY_WIFI)) {
            if (call.getState() == ONHOLD) {
                Icon icon = Icon.createWithResource(mContext, R.drawable.mtk_fab_wfc_pause_icon);
                builder.setSmallIcon(icon);
                builder.setContentText(mContext.getResources()
                        .getString(R.string.notification_on_hold));
                builder.setLargeIcon(largeIcon);
            } else if ((mReceivePktLost <= REC_MED1_PKT_LOST) &&
                    (mReceivePktLost >= MIN_PKT_LOST)) {
                Icon icon = Icon.createWithResource(mContext,
                        R.drawable.mtk_fab_ic_wfc_good_quality);
                builder.setSmallIcon(icon);
                builder.setContentText(mContext.getResources()
                        .getString(R.string.ongoing_call_good_quality));
                builder.setLargeIcon(largeIcon);
            } else if ((mReceivePktLost <= REC_MED2_PKT_LOST) &&
                    (mReceivePktLost > REC_MED1_PKT_LOST)) {
                Icon icon = Icon.createWithResource(mContext,
                        R.drawable.mtk_fab_ic_wfc_fair_quality);
                builder.setSmallIcon(icon);
                builder.setContentText(mContext.getResources()
                        .getString(R.string.ongoing_call_fair_quality));
                builder.setLargeIcon(largeIcon);
            } else if ((mReceivePktLost > REC_MED2_PKT_LOST) &&
                    (mReceivePktLost <= REC_MAX_PKT_LOST)) {
                Icon icon = Icon.createWithResource(mContext,
                        R.drawable.mtk_fab_ic_wfc_poor_quality);
                builder.setSmallIcon(icon);
                builder.setContentText(mContext.getResources()
                        .getString(R.string.ongoing_call_poor_quality));
                builder.setLargeIcon(largeIcon);
            }
        }
        Log.d(TAG, "customizeNotification");
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Log.d(TAG, "action = " + intent.getAction());
            if (ACTION_IMS_RTP_INFO.equals(intent.getAction())) {
                mReceivePktLost = intent.getIntExtra(EXTRA_RTP_RECV_PKT_LOST, 0);
                Log.d(TAG, "mReceivePktLost = " + mReceivePktLost);
                if (mStatusBarNotifier != null) {
                    mStatusBarNotifier.updateNotification();
                }
            }
        }
    };
}

