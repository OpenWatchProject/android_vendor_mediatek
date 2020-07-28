package com.mediatek.engineermode.bypass;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbManager;
import android.os.FileUtils;
import android.os.IBinder;
import android.support.v4.app.NotificationCompat;
import com.mediatek.engineermode.EmUtils;

import com.mediatek.engineermode.Elog;

import java.io.File;
import java.io.IOException;

import android.support.v4.content.LocalBroadcastManager;

/**
 * List for Bypass Service.
 */
public class BypassService extends Service {

    private static final boolean DEBUG = true;
    private static final String TAG = "BypassService";
    private static final int ID_FORE_GROUND = 20170713;
    private Bypass mBypass;
    /**
     * List for Bypass.
     */
    private final class Bypass {
        private static final String ACTION_USB_BYPASS_SETFUNCTION =
            "com.via.bypass.action.setfunction";
        private static final String ACTION_USB_BYPASS_SETTETHERFUNCTION =
            "com.via.bypass.action.settetherfunction";
        private static final String VALUE_ENABLE_BYPASS =
            "com.via.bypass.enable_bypass";
        private static final String ACTION_USB_BYPASS_SETBYPASS =
            "com.via.bypass.action.setbypass";
        private static final String ACTION_USB_BYPASS_SETBYPASS_RESULT =
            "com.via.bypass.action.setbypass_result";
        private static final String VALUE_ISSET_BYPASS =
            "com.via.bypass.isset_bypass";
        private static final String ACTION_USB_BYPASS_GETBYPASS =
            "com.via.bypass.action.getbypass";
        private static final String ACTION_USB_BYPASS_GETBYPASS_RESULT =
            "com.via.bypass.action.getbypass_result";
        private static final String VALUE_BYPASS_CODE =
            "com.via.bypass.bypass_code";
        private static final String ACTION_VIA_ETS_DEV_CHANGED =
            "via.cdma.action.ets.dev.changed";
        private static final String ACTION_RADIO_AVAILABLE =
            "android.intent.action.RADIO_AVAILABLE";

        private static final String ACTION_VIA_SET_ETS_DEV =
            "via.cdma.action.set.ets.dev";
        private static final String EXTRAL_VIA_ETS_DEV =
            "via.cdma.extral.ets.dev";
        private UsbManager mUsbManager;
        /*Bypass function values*/
        private File[] mBypassFiles;
        private final int[] mBypassCodes = new int[]{1, 2, 4, 8, 16};
        private final String[] mBypassName = new String[]{"gps", "pcv", "atc", "ets", "data"};
        private int mBypassAll = 0;
        private int mBypassToSet;
        private boolean mEtsDevInUse = false;

        private final BroadcastReceiver mBypassReceiver = new BroadcastReceiver()
        {
            @Override
                public void onReceive(Context context, Intent intent) {
                    if (DEBUG) {
                        Elog.i(TAG, "onReceive=" + intent.getAction());
                    }
                    if (intent.getAction() != null) {
                        if (intent.getAction().equals(ACTION_USB_BYPASS_SETFUNCTION)) {
                            Boolean enablebypass =
                                intent.getBooleanExtra(VALUE_ENABLE_BYPASS, false);
                            if (enablebypass) {
                                mUsbManager.setCurrentFunction("via_bypass", false);
                            } else {
                                closeBypassFunction();
                            }
                        } else if (intent.getAction().equals(ACTION_USB_BYPASS_SETTETHERFUNCTION)) {
                            Elog.w(TAG, "intent - ACTION_USB_BYPASS_SETTETHERFUNCTION");
                        } else if (intent.getAction().equals(ACTION_USB_BYPASS_SETBYPASS)) {
                            int bypasscode = intent.getIntExtra(VALUE_BYPASS_CODE, -1);
                            if (bypasscode >= 0 && bypasscode <= mBypassAll) {
                                setBypassMode(bypasscode);
                            } else {
                                notifySetBypassResult(false, getCurrentBypassMode());
                            }
                        } else if (intent.getAction().equals(ACTION_USB_BYPASS_GETBYPASS)) {
                            Intent reintent = new Intent(ACTION_USB_BYPASS_GETBYPASS_RESULT);
                            reintent.putExtra(VALUE_BYPASS_CODE, getCurrentBypassMode());
                            LocalBroadcastManager.getInstance(BypassService.this)
                                                 .sendBroadcast(reintent);
                        } else if (intent.getAction().equals(ACTION_VIA_ETS_DEV_CHANGED)) {
                            boolean result = intent.getBooleanExtra("set.ets.dev.result", false);
                            int bypass;
                            if (result) {
                                //setBypass(mBypassToSet);
                                bypass = mBypassToSet;
                            } else {
                                //setBypass(currentBypass);
                                bypass = getCurrentBypassMode();
                            }
                            setBypass(bypass);
                        } else if (intent.getAction().equals(ACTION_RADIO_AVAILABLE)) {
                            if (mEtsDevInUse) {
                                Intent reintent = new Intent(ACTION_VIA_SET_ETS_DEV);
                                reintent.putExtra(EXTRAL_VIA_ETS_DEV, 1);
                                LocalBroadcastManager.getInstance(BypassService.this)
                                                     .sendBroadcast(reintent);
                            }
                        } else if (intent.getAction().equals(UsbManager.ACTION_USB_STATE)) {
                            boolean usbConnected = intent
                                .getBooleanExtra(UsbManager.USB_CONNECTED, false);
                            if (!usbConnected) {
                                updateBypassMode(0);
                            }
                        }
                    }
                }
        };

        public Bypass() {

            mUsbManager = getSystemService(UsbManager.class);

            mBypassFiles = new File[mBypassName.length];
            for (int i = 0; i < mBypassName.length; i++) {
                final String path = "/sys/class/usb_rawbulk/" + mBypassName[i] + "/enable" ;
                //if (DEBUG) Elog.d(TAG, "bypass mode file path="+path);
                mBypassFiles[i] = new File(path);
                mBypassAll += mBypassCodes[i];
            }
            //register bypass receiver
            IntentFilter intent = new IntentFilter(ACTION_USB_BYPASS_SETFUNCTION);

            intent.addAction(ACTION_USB_BYPASS_SETTETHERFUNCTION);
            intent.addAction(ACTION_USB_BYPASS_SETBYPASS);
            intent.addAction(ACTION_USB_BYPASS_GETBYPASS);
            intent.addAction(ACTION_VIA_ETS_DEV_CHANGED);
            intent.addAction(ACTION_RADIO_AVAILABLE);
            intent.addAction(UsbManager.ACTION_USB_STATE);

            LocalBroadcastManager.getInstance(BypassService.this)
                                 .registerReceiver(mBypassReceiver, intent);
        }
        private int getCurrentBypassMode() {
            int bypassmode = 0;
            try {
                for (int i = 0; i < mBypassCodes.length; i++) {
                    String code;

                    code = FileUtils.readTextFile(mBypassFiles[i], 0, null);
                    if (DEBUG) {
                        Elog.d(TAG, "'" + mBypassFiles[i].getAbsolutePath() + "' value is " + code);
                    }
                    if (code != null && code.trim().equals("1")) {
                        bypassmode |= mBypassCodes[i];
                    }
                }
                if (DEBUG) {
                    Elog.d(TAG, "getCurrentBypassMode()=" + bypassmode);
                }
            } catch (IOException e) {
                Elog.e(TAG, "failed to read bypass mode code!");
            }
            return bypassmode;
        }

        private void setBypass(int bypassmode) {
            Elog.d(TAG, "setBypass bypass = " + bypassmode);
            int bypassResult = getCurrentBypassMode();
            if (bypassmode == bypassResult) {
                Elog.d(TAG, "setBypass bypass == oldbypass!!");
                notifySetBypassResult(true, bypassResult);
                return;
            }

            for (int i = 0; i < mBypassCodes.length; i++) {
                if ((bypassmode & mBypassCodes[i]) != 0) {
                    if (DEBUG) {
                        Elog.d(TAG, "Write '" + mBypassFiles[i].getAbsolutePath() + "1");
                    }
                    try {
                        EmUtils.getEmHidlService().setBypassEn(Integer.toString(i));
                    } catch (Exception e) {
                        Elog.e(TAG, "set property failed ...");
                        e.printStackTrace();
                    }
                    bypassResult |= mBypassCodes[i];
                } else {
                    if (DEBUG) {
                        Elog.d(TAG, "Write '" + mBypassFiles[i].getAbsolutePath() + "0");
                    }
                    try {
                        EmUtils.getEmHidlService().setBypassDis(Integer.toString(i));
                    } catch (Exception e) {
                        Elog.e(TAG, "set property failed ...");
                        e.printStackTrace();
                    }
                    if ((bypassResult & mBypassCodes[i]) != 0) {
                        bypassResult ^= mBypassCodes[i];
                    }
                }
                if (DEBUG) {
                    Elog.d(TAG, "Write '" + mBypassFiles[i].getAbsolutePath()
                            + "' successsfully!");
                }
            }
            notifySetBypassResult(true, bypassResult);
            Elog.d(TAG, "setBypass success bypassResult = " + bypassResult);
        }

        void updateBypassMode(int bypassmode) {
            Elog.d(TAG, "updateBypassMode");
            //Open/Close ets port for pc
            if (!setEtsDev(bypassmode)) {
                //if needn't Open/Close ets port for pc set bypass code now
                setBypass(bypassmode);
            } else {
                Elog.d(TAG, "updateBypassMode mBypassToSet = " + mBypassToSet);
                mBypassToSet = bypassmode;
            }
        }

        private boolean setEtsDev(int bypass) {
            int oldBypass = getCurrentBypassMode();
            Elog.d(TAG, "setEtsDev bypass = " + bypass + " oldBypass = " + oldBypass);
            if ((bypass & mBypassCodes[3]) != 0 && (oldBypass & mBypassCodes[3]) == 0) {
                Elog.d(TAG, "setEtsDev mEtsDevInUse = true");
                Intent reintent = new Intent(ACTION_VIA_SET_ETS_DEV);
                reintent.putExtra(EXTRAL_VIA_ETS_DEV, 1);
                LocalBroadcastManager.getInstance(BypassService.this).sendBroadcast(reintent);
                mEtsDevInUse = true;
                return true;
            } else if ((bypass & mBypassCodes[3]) == 0 &&
                    (oldBypass & mBypassCodes[3]) != 0) {
                Elog.d(TAG, "setEtsDev mEtsDevInUse = false");
                Intent reintent = new Intent(ACTION_VIA_SET_ETS_DEV);
                reintent.putExtra(EXTRAL_VIA_ETS_DEV, 0);
                LocalBroadcastManager.getInstance(BypassService.this).sendBroadcast(reintent);
                mEtsDevInUse = false ;
                return true ;
            } else {
                return false ;
            }
        }

        /*Set bypass mode*/
        private void setBypassMode(int bypassmode) {
            if (DEBUG) {
                Elog.d(TAG, "setBypassMode()=" + bypassmode);
            }
            updateBypassMode(bypassmode);
        }
        private void notifySetBypassResult(Boolean isset, int bypassCode) {
            Intent intent = new Intent(ACTION_USB_BYPASS_SETBYPASS_RESULT);
            intent.putExtra(VALUE_ISSET_BYPASS, isset);
            intent.putExtra(VALUE_BYPASS_CODE, bypassCode);
            LocalBroadcastManager.getInstance(BypassService.this).sendBroadcast(intent);
        }

        private void closeBypassFunction() {
            mUsbManager.setCurrentFunction(null, false);
        }
    }
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();
        Elog.w(TAG, "onCreate");
        mBypass = new Bypass();
        try {
            EmUtils.getEmHidlService().setBypassService("1");
        } catch (Exception e) {
            Elog.e(TAG, "set property failed ...");
            e.printStackTrace();
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Elog.w(TAG, "onStartCommand");
        /*startForeground(ID_FORE_GROUND, new NotificationCompat.Builder(this)
                .setContentTitle(TAG)
                .setSmallIcon(android.R.drawable.ic_dialog_alert)
                .build());*/
        return START_STICKY;
    }

    @Override
    public void onDestroy() {
        stopService(new Intent(this, BypassService.class));
        super.onDestroy();
        Elog.w(TAG, "onDestroy");
    }

}
