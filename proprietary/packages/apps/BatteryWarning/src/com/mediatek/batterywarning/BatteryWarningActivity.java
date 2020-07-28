package com.mediatek.batterywarning;

import android.app.Activity;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.media.AudioManager;
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.util.Log;
import android.view.View.OnClickListener;
import android.view.View;
import android.view.Window;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.ImageView;
import android.widget.TextView;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;

public class BatteryWarningActivity extends Activity {
    private static final String TAG = "BatteryWarningActivity";
    public static final String WARNING_NOTIFICATION_CHANNEL_ID = "battery_warning_notif_channel";
    private static final Uri WARNING_SOUND_URI = Uri
            .parse("file:///system/media/audio/ui/VideoRecord.ogg");
    private static final String SHARED_PREFERENCES_NAME = "battery_warning_settings";
    protected static final String KEY_TYPE = "type";
    private static boolean mWaterGas =false;
    private Ringtone mRingtone;
    private int mType;

    private static final int CHARGER_OVER_VOLTAGE_TYPE = 0;
    private static final int BATTERY_OVER_TEMPERATURE_TYPE = 1;
    private static final int CURRENT_OVER_PROTECTION_TYPE = 2;
    private static final int BATTERY_OVER_VOLTAGE_TYPE = 3;
    private static final int SAFETY_OVER_TIMEOUT_TYPE = 4;
    private static final int BATTERY_LOW_TEMPERATURE_TYPE = 5;
    private static final int TYPEC_DETECTION_WATER_GAS_TYPE = 6;

    static final int[] sWarningTitle = new int[] {
            R.string.title_charger_over_voltage,
            R.string.title_battery_over_temperature,
            R.string.title_over_current_protection,
            R.string.title_battery_over_voltage,
            R.string.title_safety_timer_timeout,
            R.string.title_battery_low_temperature,
            R.string.title_typeC_detection_water_gas
            };
    private static final int[] sWarningMsg = new int[] {
            R.string.msg_charger_over_voltage,
            R.string.msg_battery_over_temperature,
            R.string.msg_over_current_protection,
            R.string.msg_battery_over_voltage,
            R.string.msg_safety_timer_timeout,
            R.string.msg_battery_low_temperature,
            R.string.msg_typeC_detection_water_gas };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Intent.ACTION_POWER_DISCONNECTED.equals(action)) {
                if (mType == CHARGER_OVER_VOLTAGE_TYPE
                        || mType == SAFETY_OVER_TIMEOUT_TYPE || mType == BATTERY_LOW_TEMPERATURE_TYPE) {
                    Log.d(TAG, "receive ACTION_POWER_DISCONNECTED broadcast, finish");
                    finish();
                }
            }
        }
    };


    public static void initWarningNotificationChannel(Context context) {
       NotificationManager notificationManager =
            (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
        // BatteryWarning Notification Channel
        CharSequence name = "Battery Warning";
        NotificationChannel mChannelBatWarn = new NotificationChannel(
                            WARNING_NOTIFICATION_CHANNEL_ID,
                            name,
                            NotificationManager.IMPORTANCE_DEFAULT);
        notificationManager.createNotificationChannel(mChannelBatWarn);
        Log.d(TAG, "initWarningNotificationChannel  " + mChannelBatWarn);
    }

    public static void deleteWarningNotificationChannel(Context context, String channelId) {
        Log.d(TAG, "deleteWarningNotificationChannel  " + channelId);
        NotificationManager notificationManager = (NotificationManager) context.
                                getSystemService(Context.NOTIFICATION_SERVICE);
        notificationManager.deleteNotificationChannel(channelId);
    }

    public void showBatteryWarningNotification(Context context) {
        String title="";
        String content="";
        title = getString(R.string.notification_title);
        content= getString(R.string.notification_wd_text);
        NotificationManager notificationManager = (NotificationManager) context.
                                getSystemService(Context.NOTIFICATION_SERVICE);
        Notification notif = new Notification.Builder(context, WARNING_NOTIFICATION_CHANNEL_ID)
            .setContentTitle(title)
            .setContentText(content)
            .setSmallIcon(android.R.drawable.ic_dialog_info)
            .setSound(null)
            .setOngoing(true)
            .build();
        notificationManager.notify(0, notif);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        requestWindowFeature(Window.FEATURE_CUSTOM_TITLE);
        Context context;
        context=BatteryWarningActivity.this;//this.getApplicationContext();
        setContentView(R.layout.battery_warning);
        getWindow().setFeatureInt(Window.FEATURE_CUSTOM_TITLE,
                R.layout.custom_title_1);

        Intent intent = getIntent();
        mType = intent.getIntExtra(KEY_TYPE, -1);
        TextView textView = (TextView) findViewById(R.id.left_text);
        Log.d(TAG, "onCreate, mType is " + mType);

        if (mType >= CHARGER_OVER_VOLTAGE_TYPE  && mType <= TYPEC_DETECTION_WATER_GAS_TYPE) {
            textView.setText(getString(sWarningTitle[mType]));
            if(mType==TYPEC_DETECTION_WATER_GAS_TYPE)
            {
                initWarningNotificationChannel(context);
                showBatteryWarningNotification(context);
                mWaterGas =true;
            } else {
                deleteWarningNotificationChannel(context,WARNING_NOTIFICATION_CHANNEL_ID);
                mWaterGas = false;
            }
            showWarningDialog(mType);
            registerReceiver(mReceiver, new IntentFilter(
                    Intent.ACTION_POWER_DISCONNECTED));
        } else {
              if(mWaterGas) {
                deleteWarningNotificationChannel(context,WARNING_NOTIFICATION_CHANNEL_ID);
                mWaterGas = false;
              }
              finish();
        }
    }

    protected void onDestroy() {
        super.onDestroy();
        Log.d(TAG, "onDestroy, stopRingtone");
        stopRingtone();
        if (mType >= CHARGER_OVER_VOLTAGE_TYPE  && mType <= TYPEC_DETECTION_WATER_GAS_TYPE) {
            unregisterReceiver(mReceiver);
        }
    }

    private void showWarningDialog(int type) {
        TextView textView = (TextView) findViewById(R.id.text);
        textView.setText(getString(sWarningMsg[mType]));

        LinearLayout layout = (LinearLayout)findViewById(R.id.inner_content);
        if(type!= TYPEC_DETECTION_WATER_GAS_TYPE) {
           ImageView iv = new ImageView(BatteryWarningActivity.this);
           iv.setImageDrawable(getResources().getDrawable(R.drawable.battery_low_battery));
           iv.setPadding(4, 4, 4, 4);
           layout.addView(iv);
        }
        Button button = (Button)findViewById(R.id.add);
        button.setText(getString(R.string.btn_cancel_msg));
        button.setOnClickListener(mDismissContentListener);
        button = (Button)findViewById(R.id.remove);
        button.setText(getString(R.string.btn_ok_msg));
        button.setOnClickListener(mSnoozeContentListener);

        playAlertSound(WARNING_SOUND_URI);
    }

    private OnClickListener mDismissContentListener = new OnClickListener() {
        public void onClick(View v) {
            stopRingtone();
            SharedPreferences.Editor editor = getSharedPreferences().edit();
            editor.putBoolean(Integer.toString(mType), false);
            editor.apply();
            Log.d(TAG, "set type " + mType + " false");
            finish();
        }
    };

    private OnClickListener mSnoozeContentListener = new OnClickListener() {
        public void onClick(View v) {
            stopRingtone();
            finish();
        }
    };

    /**
     *
     * @param context
     *            The Context that had been passed to
     *            {@link #warningMessageDialog(Context, Uri)}
     * @param defaultUri
     */
    private void playAlertSound(Uri defaultUri) {

        if (defaultUri != null) {
            mRingtone = RingtoneManager.getRingtone(this, defaultUri);
            if (mRingtone != null) {
                mRingtone.setStreamType(AudioManager.STREAM_SYSTEM);
                mRingtone.play();
            }
        }
    }

    private void stopRingtone() {
        if (mRingtone != null) {
            mRingtone.stop();
            mRingtone = null;
        }
    }

    private SharedPreferences getSharedPreferences() {
        return getSharedPreferences(SHARED_PREFERENCES_NAME, Context.MODE_PRIVATE);
    }
}