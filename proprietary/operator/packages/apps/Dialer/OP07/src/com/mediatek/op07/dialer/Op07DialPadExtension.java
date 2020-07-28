package com.mediatek.op07.dialer;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.graphics.Color;
import android.net.ConnectivityManager;
import android.net.NetworkInfo;
import android.os.Handler;
import android.provider.Settings;
import android.telecom.TelecomManager;
import android.telecom.VideoProfile;

import android.util.Log;
import android.view.Menu;
import android.view.View;

import com.android.dialer.util.CallUtil;
import com.android.ims.ImsManager;
import com.mediatek.dialer.ext.DefaultDialPadExtension;

public class Op07DialPadExtension extends DefaultDialPadExtension {
    private static final String TAG = "Op07DialPadExtension";

    private Context mContext;
    public Op07DialPadExtension(Context context) {
        mContext = context;
    }

         /**
     * Show dialog to remind user to turn ON video.
     * @param appContext
     * @param intent
     * @return false
     */
    @Override
    public boolean checkVideoSetting(final Context appContext, Intent intent) {
        Log.d(TAG, "checkVideoSetting");
        int intentVideoState = intent.getIntExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                VideoProfile.STATE_AUDIO_ONLY);
        if (!VideoProfile.isVideo(intentVideoState)) {
            Log.d(TAG, "checkVideoSetting: Not video state");
            return false;
        }

        final boolean enableVideoSetting = ImsManager.isVtEnabledByUser(mContext);

        if (enableVideoSetting) {
           Log.d(TAG, "checkVideoSetting: Video Call setting is enabled");
           if (!CallUtil.isVideoEnabled(mContext)) {
                Log.d(TAG, "checkVideoSetting: SIM is non VoLTE");
                intent.putExtra(TelecomManager.EXTRA_START_CALL_WITH_VIDEO_STATE,
                            VideoProfile.STATE_AUDIO_ONLY);
           }
           return false;
        }
        Log.d(TAG, "checkVideoSetting: Video state");
         Handler mainHandler = new Handler(mContext.getMainLooper());
        mainHandler.post(new Runnable() {
            @Override
            public void run() {
                AlertDialog.Builder builder = new AlertDialog.Builder(appContext);
                DialogInterface.OnClickListener MobileSettingsClickListener =
                    new Dialog.OnClickListener() {
                         @Override
                          public void onClick(DialogInterface dialog, int which) {
                              try {
                                    Intent mobileSettingIntent = new Intent();
                                    mobileSettingIntent.setFlags(
                                    Intent.FLAG_ACTIVITY_NEW_TASK);
                                    mobileSettingIntent.setClassName("com.android.settings",
                                    "com.android.settings.Settings$DataUsageSummaryActivity");
                                    mContext.startActivity(mobileSettingIntent);
                              } catch (ActivityNotFoundException e) {
                                    Log.e(TAG, e.toString());
                              } catch (Exception e) {
                                    Log.e(TAG, e.toString());
                              }
                          }
                };
                DialogInterface.OnClickListener CallSettingsClickListener =
                    new Dialog.OnClickListener() {
                         @Override
                    public void onClick(DialogInterface dialog, int which) {
                        try {
                            Intent callSettingIntent =
                            new Intent(TelecomManager.ACTION_SHOW_CALL_SETTINGS);
                            callSettingIntent.setFlags(
                            Intent.FLAG_ACTIVITY_NEW_TASK);
                            mContext.startActivity(callSettingIntent);
                        } catch (ActivityNotFoundException e) {
                            Log.e(TAG, e.toString());
                        } catch (Exception e) {
                            Log.e(TAG, e.toString());
                        }
                    }
                };
                DialogInterface.OnClickListener WifiSettingsClickListener =
                    new Dialog.OnClickListener() {
                         @Override
                    public void onClick(DialogInterface dialog, int which) {
                              try {
                                    Intent wifiSettingIntent =
                                    new Intent(Settings.ACTION_WIFI_SETTINGS);
                                    wifiSettingIntent.setFlags(
                                    Intent.FLAG_ACTIVITY_NEW_TASK);
                                    mContext.startActivity(wifiSettingIntent);
                              } catch (ActivityNotFoundException e) {
                                    Log.e(TAG, e.toString());
                              } catch (Exception e) {
                                    Log.e(TAG, e.toString());
                              }
                          }
                };
                builder.setTitle(mContext.getString(R.string.video_alert));
                builder.setCancelable(false);
                if (!isInternetConnected()) {
                    Log.d(TAG, "Only Mobile data/wifi off");
                    builder.setMessage(mContext.getString(R.string.video_message_data));
                    builder.setNegativeButton(mContext.getString(R.string.mobile_data_settings),
                                                            MobileSettingsClickListener);
                    builder.setPositiveButton(mContext.getString(R.string.wifi_settings),
                                                            WifiSettingsClickListener);
                    builder.setNeutralButton(mContext.getString(R.string.cancel), null);
                } else {
                    Log.d(TAG, "Only video setting off");
                    builder.setMessage(mContext.getString(R.string.video_message_settings));
                    builder.setPositiveButton(mContext.getString(R.string.call_settings),
                                                            CallSettingsClickListener);
                    builder.setNegativeButton(mContext.getString(R.string.cancel), null);
                    }
                builder.show();
            }
        });
        return true;
    }

    private boolean isInternetConnected() {
    ConnectivityManager connectivityMgr = (ConnectivityManager) mContext
            .getSystemService(Context.CONNECTIVITY_SERVICE);
    NetworkInfo wifi = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_WIFI);
    NetworkInfo mobile = connectivityMgr.getNetworkInfo(ConnectivityManager.TYPE_MOBILE);
    // Check if wifi or mobile network is available or not. If any of them is
    // available or connected then it will return true, otherwise false;
    if (wifi != null) {
        if (wifi.isConnected()) {
            Log.d(TAG, "isInternetConnected: Wifi is connected");
            return true;
        }
    }
    if (mobile != null) {
        if (mobile.isConnected()) {
            Log.d(TAG, "isInternetConnected: Mobile data is connected");
            return true;
        }
    }
    Log.d(TAG, "isInternetConnected: Internet not Connected");
    return false;
  }
}
