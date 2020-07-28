package com.mediatek.op18.incallui;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.BatteryManager;
import android.os.Handler;
import android.preference.PreferenceManager;
import android.telecom.VideoProfile;
import android.util.Log;
import android.widget.Toast;

//import com.android.ims.ImsManager;
import com.android.incallui.InCallPresenter.InCallState;
import com.android.incallui.call.CallList;
import com.android.incallui.call.DialerCall;

import com.mediatek.incallui.ext.DefaultInCallExt;
import com.mediatek.op18.dialer.R;
import com.mediatek.wfo.WifiOffloadManager;

/**
 * Plug in implementation for OP18 InCall interfaces.
 */
public class OP18InCallExt extends DefaultInCallExt {
    private static final String TAG = "OP18InCallExt";
    private Context mContext;
    private ToneGenerator mToneGenerator;
    private int mVideoState = VideoProfile.STATE_BIDIRECTIONAL;
    private SharedPreferences mPref;
    private final Handler mHandler = new Handler();
    private static final int WAITING_FOR_DOWNGRADE_RESPONSE = 6;
    private static final int TONE_RELATIVE_VOLUME = 80;
    private static final int BATTERY_LEVEL = 15;
    private static final String KEY_IS_FIRST_BATTERY_WARNING = "key_first_battery_warning";
    private static final float BATTERY_PERCENTAGE = 0.15f;

    /** Constructor.
     * @param context context
     */
    public OP18InCallExt(Context context) {
        mContext = context;
    }

    @Override
    public void showHandoverNotification(Handler handler, int stage, int ratType) {
        DialerCall call = CallList.getInstance().getActiveOrBackgroundCall();
        if (call != null && call.hasProperty(android.telecom.Call.Details.PROPERTY_WIFI)) {
            if (stage == WifiOffloadManager.HANDOVER_START && ratType ==
                    WifiOffloadManager.RAN_TYPE_MOBILE_3GPP) {
                handler.post(new Runnable() {
                    @Override
                    public void run() {
                        Toast.makeText(mContext, mContext.getResources().getString(
                                R.string.switch_to_Volte_from_Wifi), Toast.LENGTH_SHORT).show();
                    }
                });
                playTone();
            }
        }
    }

    private void playTone() {
        if (mToneGenerator == null) {
            try {
                mToneGenerator = new ToneGenerator(AudioManager.STREAM_VOICE_CALL,
                        TONE_RELATIVE_VOLUME);
            } catch (RuntimeException e) {
                Log.w(TAG, "[WFC]Exception caught while creating local tone generator: " + e);
                mToneGenerator = null;
                return;
            }
        }
        mToneGenerator.startTone(ToneGenerator.TONE_PROP_BEEP2, -1);
    }

    /** Register receiver for battery status.
     */
    public void registerReceiver() {
        Log.i(TAG, "registerReceiver");
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_BATTERY_CHANGED);
        filter.addAction(ErrorDialogActivity.DOWNGRADE_TO_AUDIO);
        mContext.registerReceiver(mReceiver, filter);
        mPref = PreferenceManager.getDefaultSharedPreferences(mContext);
    }

    /** Un-register receiver for battery status.
     */
    public void unregisterReceiver() {
        Log.i(TAG, "unregisterReceiver");
        if (mReceiver != null) {
            mContext.unregisterReceiver(mReceiver);
        }
        SharedPreferences.Editor editor = mPref.edit();
        editor.putBoolean(KEY_IS_FIRST_BATTERY_WARNING, true);
        editor.commit();
        dismissDialog();
        Log.i(TAG, "[WFC]showDialog write KEY_IS_FIRST_BATTERY_WARNING as true ");
    }

    /**
     * Called on Incall presenter setup to register battery indications receiver.
     *
     * @param statusbarNotifier statusbarNotifier
     * @param state state
     * @param callList callList
     */
    @Override
    public void onInCallPresenterSetUp(Object statusbarNotifier, Object state, Object callList) {
        registerReceiver();
    }

    /**
     * Called on Incall presenter tearDown to unregister battery indications receiver.
     */
    @Override
    public void onInCallPresenterTearDown() {
        unregisterReceiver();
    }

    /**
     * Called to check if battery dialog needs to be shown or not.
     *
     * @param newState newState of call
     * @param oldState oldState of call
     */
    @Override
    public void maybeShowBatteryDialog(Object newState, Object oldState) {
        CallList callList = CallList.getInstance();
        DialerCall call = callList.getActiveOrBackgroundCall();
        if (call != null) {
            InCallState nState = (InCallState) newState;
            InCallState oState = (InCallState) oldState;
            int nVideoState = call.getVideoState();
            int oVideoState = mVideoState;
            mVideoState = nVideoState;
            Log.d(TAG, "nState" + nState + "oState" + oState);
            Log.d(TAG, "nVideoState" + nVideoState + "oVideoState" + oVideoState);
            if ((nState == InCallState.INCALL && oState != nState) ||
                    nVideoState != oVideoState) {
                Log.d(TAG, "maybeShowBatteryDialog isVideoCall" + call.isVideoCall());
                float batteryLevel = batteryLevel(mContext);
                if (call.isVideoCall() && batteryLevel <= BATTERY_LEVEL
                        && !isCharging(mContext)) {
                    Log.d(TAG, "maybeShowBatteryDialog showDialog");
                    showDialog();
                }
            }
        }
    }

    /** Show the battery status dialog.
     */
    public void showDialog() {
        int timer = 100;
        Runnable runnable = new Runnable() {
            @Override
            public void run() {
                Log.i(TAG, "[WFC]showDialog pref.getBoolean(KEY_IS_FIRST_BATTERY_WARNING"
                    + mPref.getBoolean(KEY_IS_FIRST_BATTERY_WARNING, true));
                if (mPref.getBoolean(KEY_IS_FIRST_BATTERY_WARNING, true)) {
                    final Intent errorIntent = new Intent(mContext, ErrorDialogActivity.class);
                    errorIntent.putExtra(ErrorDialogActivity.SHOW_VIDEO_CALL, true);
                    errorIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    mContext.startActivity(errorIntent);
                    SharedPreferences.Editor editor = mPref.edit();
                    editor.putBoolean(KEY_IS_FIRST_BATTERY_WARNING, false);
                    editor.commit();
                    Log.i(TAG, "showDialog write KEY_IS_FIRST_BATTERY_WARNING as false");
                }
            }
        };
        mHandler.postDelayed(runnable, timer);
    }

    /** Interface to dismiss the battery dialog.
     */
    @Override
    public void maybeDismissBatteryDialog() {
        Log.d(TAG, "maybeDismissBatteryDialog");
        dismissDialog();
    }

    /** Interface to get the current battery level.
     */
    private float batteryLevel(Context context) {
        Intent intent  = context.registerReceiver(null,
                new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
        int    level   = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
        int    scale   = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 100);
        int    percent = (level * 100) / scale;
        Log.d(TAG, "batteryLevel percent" + percent);
        return percent;
    }

    /** Interface to check if phone is charging or not.
     */
    private boolean isCharging(Context context) {
        IntentFilter ifilter = new IntentFilter(Intent.ACTION_BATTERY_CHANGED);
        Intent batteryStatus = context.registerReceiver(null, ifilter);
        int chargePlug = batteryStatus.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
        boolean usbCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_USB;
        boolean acCharge = chargePlug == BatteryManager.BATTERY_PLUGGED_AC;
        Log.d(TAG, "isCharging usbCharge" + usbCharge + "acCharge" + acCharge);
        return usbCharge || acCharge;
    }

    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.v(TAG, action);
            CallList callList = CallList.getInstance();
            if (intent.getAction().equals(Intent.ACTION_BATTERY_CHANGED)) {
                int level = intent.getIntExtra(BatteryManager.EXTRA_LEVEL, 0);
                int scale = intent.getIntExtra(BatteryManager.EXTRA_SCALE, 0);
                float batteryPct = level / (float) scale;
                int plugged = intent.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
                boolean isPlugged = plugged == BatteryManager.BATTERY_PLUGGED_AC
                    || plugged == BatteryManager.BATTERY_PLUGGED_USB;
                DialerCall call = callList.getActiveOrBackgroundCall();
                Log.d(TAG, "ACTION_BATTERY_CHANGED batteryPct" + batteryPct +
                    "isPlugged" + isPlugged + "call" + call);
                if (call != null && call.isVideoCall()
                    && batteryPct <= BATTERY_PERCENTAGE && !isPlugged) {
                    Log.i(TAG, "batteryPct less than 15%");
                    showDialog();
                } else if (call != null && call.isVideoCall() && isPlugged) {
                    Log.i(TAG, "isPlugged " + isPlugged);
                    dismissDialog();
                }
            } else if (intent.getAction().equals(ErrorDialogActivity.DOWNGRADE_TO_AUDIO)) {
                Log.d(TAG, "DOWNGRADE_TO_AUDIO request");
                DialerCall call = callList.getActiveOrBackgroundCall();
                if (call != null && call.getVideoCall() != null) {
                    call.getVideoTech().downgradeToAudio();
                }
            }
        }
    };

    /** Interface to dismiss the battery dialog.
     */
    public void dismissDialog() {
        Log.i(TAG, "dismissDialog");
        Intent broadcastIntent = new Intent(ErrorDialogActivity.DISMISS_VIDEO_CALL);
        mContext.sendBroadcast(broadcastIntent);
    }
}
