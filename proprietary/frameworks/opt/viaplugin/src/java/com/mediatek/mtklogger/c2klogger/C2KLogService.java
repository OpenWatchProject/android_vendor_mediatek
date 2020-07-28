package com.mediatek.mtklogger.c2klogger;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.util.Log;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.sql.Date;
import java.text.DecimalFormat;
import java.util.Calendar;
import java.util.GregorianCalendar;

/**
 * @author MTK81255
 *
 */
public class C2KLogService {

    public static final String UPDATE_INFO_ACTION = "android.intent.action.UPDATE_INFO";

    private static final String ACTION_VIA_ETS_DEV_CHANGED =
            "via.cdma.action.ets.dev.changed.c2klogger";

    private SharedPreferences mSharedPreferences;
    public String mDevicePath = null; // the path of cbp device
    public String mFilterFilePath = null; // the path of ets config file which
                                          // the log service need it
    private String mMdlogPath = null;

    private EtsLog mEtsLog = null;

    private static final int MSG_SET_CHANNEL = 1;

    private boolean mIsOnDestroy = false;

    private boolean mIsSetChannelDone = true;
    private Handler mMessageHandler;

    private Service mService;

    private Thread mMonitModemThread;
    private HandlerThread mHandlerThread;

    /**
     * @param service Service
     */
    public C2KLogService(Service service) {
        mService = service;
    }

    private EtsLog.EtsLogCallback mCbEtsLog = new EtsLog.EtsLogCallback() {

        public void onProcess(EtsLog.LogStatus status, String info) {
            if (status == EtsLog.LogStatus.Error) {
                Log.e("saber", "error:" + info);
                mService.stopSelf();
            } else if (status == EtsLog.LogStatus.Logging) {
                Log.i("saber", "info:" + info);
            }
        }
    };

    /**
     * @param arg0 Intent
     * @return IBinder
     */
    public IBinder onBind(Intent arg0) {
        Log.i("saber", "============>  onBind");
        return null;
    }

    /**
     * @param i Intent
     * @return boolean
     */
    public boolean onUnbind(Intent i) {
        Log.i("saber", "============>  onUnbind");
        return mService.onUnbind(i);
    }

    /**
     * @param i Intent
     */
    public void onRebind(Intent i) {
        Log.i("saber", "============>  onRebind");
    }

    /**
     * C2KLogService create.
     */
    public void onCreate() {
        Log.i("saber", "============>  onCreate");
        mIsOnDestroy = false;
        mHandlerThread = new HandlerThread("C2KLoggerServiceThread");
        mHandlerThread.start();
        mMessageHandler = new MyHandler(mHandlerThread.getLooper());
        init();
        IntentFilter filter = new IntentFilter();
        filter.addAction(ACTION_VIA_ETS_DEV_CHANGED);
        mService.registerReceiver(mBcRecverCbp, filter);

        IntentFilter filterReboot = new IntentFilter();
        filterReboot.addAction(Intent.ACTION_SHUTDOWN);
        mService.registerReceiver(mRebootReceiver, filterReboot);
    }

    private void init() {
        Log.i("saber", "============>  init()");
        new C2KLogConfig(mService).checkConfig();
        // get settings
        mSharedPreferences = mService.getSharedPreferences(C2KLogUtils.CONFIG_FILE_NAME,
                Context.MODE_PRIVATE);
        // check the log dir
        mMdlogPath = mSharedPreferences.getString("ModemLogPath", "") + "/mtklog/"
                + C2KLogUtils.C2K_MODEM_LOG_PATH + "/";

        mDevicePath = mSharedPreferences.getString(C2KLogUtils.CONIFG_DEVICEPATH,
                C2KLogUtils.DEFAULT_CONIFG_DEVICEPATH);
        Log.i("saber", "device path is " + mDevicePath);

        String filterFileName = mSharedPreferences.getString(C2KLogUtils.CONIFG_FILTERFILE,
                C2KLogUtils.DEFAULT_CONIFG_FILTERFILE);
        mFilterFilePath = mSharedPreferences.getString(C2KLogUtils.CONIFG_PATH,
                C2KLogUtils.DEFAULT_CONIFG_PATH) + "/" + filterFileName;
        Log.i("saber", "bcfg path is " + mFilterFilePath);

        int perLogSize = mSharedPreferences.getInt(C2KLogUtils.CONIFG_PERLOGSIZE,
                C2KLogUtils.DEFAULT_CONIFG_PERLOGSIZE);
        mEtsLog = new EtsLog(perLogSize * 1024 * 1024, mCbEtsLog);
        long c2KLogMaxSize = mSharedPreferences.getInt(C2KLogUtils.CONIFG_TOTALLOGSIZE,
                C2KLogUtils.DEFAULT_CONIFG_TOTALLOGSIZE);

        C2KLogRecycle.getInstance().startRecycle(mMdlogPath, c2KLogMaxSize, mSharedPreferences);
    }

    /**
     * @return int
     */
    public int onStartCommand() {
        Log.i("saber", "============>  onStartCommand");
        try {
            BufferedReader br = null;
            if (!new File(mFilterFilePath).exists()) {
                Log.i("saber", "open v5.bcfg");
                br = new BufferedReader(new InputStreamReader(mService.getAssets().open("v5.bcfg")),
                        1024 * 1024);
            } else {
                Log.i("saber", "open " + mFilterFilePath);
                br = new BufferedReader(new FileReader(mFilterFilePath), 1024 * 1024);
            }

            String logPath = mMdlogPath + "MDLog3_" + translateTime(System.currentTimeMillis());
            File logFile = new File(logPath);
            if (!logFile.exists()) {
                logFile.mkdirs();
            }
            mSharedPreferences.edit().putString(
                    C2KLogUtils.KEY_C2K_MODEM_LOGGING_PATH, logPath).apply();
            Log.i("saber", "logPath " + logPath);
            C2KLogUtils.writerToFileTree(mMdlogPath + C2KLogUtils.LOG_TREE_FILE, logPath);

            mEtsLog.start(mDevicePath, br, logPath + "/");
            br.close();

            mEtsLog.stopLogRecord(false); // let log recording start immediately
            setChannelForStart();
        } catch (IOException e) {
            Log.w("saber", e.getMessage());
        }
        return Service.START_REDELIVER_INTENT;
    }

    private void setChannelForStart() {
        // net.cdma.mdmstat = not ready/ ready / exception
        String isReadyStr = SystemProperties.get("net.cdma.mdmstat", "");
        Log.i("saber", "isReadyStr = " + isReadyStr);
        mIsModemReady = isReadyStr.equalsIgnoreCase("ready");
        Log.i("saber", "Modem state:" + mIsModemReady);
        if (mIsModemReady) {
            setEtsChannel(1);
            mEtsLog.stopReadThread(false);
            mEtsLog.startReadThread();
            mEtsLog.writeEtsCfgs(true);
            mMonitModemThread = new Thread(new Runnable() {
                @Override
                public void run() {
                    monitModemStatus();
                }
            });
            mMonitModemThread.start();

        } else if (isReadyStr.equalsIgnoreCase("exception")
                || isReadyStr.equalsIgnoreCase("not ready")) {
            Log.i("saber", "Waiting modem boot up!");
            if (mMessageHandler != null) {
                mMessageHandler.sendMessageDelayed(
                        mMessageHandler.obtainMessage(MSG_SET_CHANNEL), 2000);
            }
        } else {
            C2KLogService.this.onDestroy();
        }
    }

    private boolean mIsModemReady = false;
    private void monitModemStatus() {
        Log.i("saber", "----->monitModemStatus() is start!");
        while (mIsModemReady) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
            String isReadyStr = SystemProperties.get("net.cdma.mdmstat", "");
            if (isReadyStr.equalsIgnoreCase("not ready")) {
                Log.i("saber", "isReadyStr = " + isReadyStr);
                Log.w("saber", "Modem is not ready now!");
                try {
                    Thread.sleep(5000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
                if (mIsOnDestroy) {
                    break;
                }
                destroy();
                mIsOnDestroy = false;
                init();
                this.onStartCommand();
                mIsModemReady = false;
            }
        }
        Log.i("saber", "<-----monitModemStatus() is stop!");
    }

    /**
     * C2KLogService destroy.
     */
    public void onDestroy() {
        Log.i("saber", "============>  onDestroy");
        mIsOnDestroy = true;
        try {
            mService.unregisterReceiver(mBcRecverCbp);
            mService.unregisterReceiver(mRebootReceiver);
        } catch (IllegalArgumentException e) {
            Log.e("saber", "unregisterReceiver exception");
        }
        if (mEtsLog == null) {
            return;
        }
        mEtsLog.writeEtsCfgs(false);
        destroy();
        int i = 0;
        while (!mEtsLog.isWriteConfigDone() && i++ < 20) {
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
    }

    private void destroy() {
        Log.i("saber", "============>  destroy");
        mIsOnDestroy = true;
        mIsModemReady = false;
        if (mHandlerThread != null) {
            mHandlerThread.interrupt();
            mHandlerThread = null;
            mMessageHandler = null;
        }
        if (mMonitModemThread != null) {
            mMonitModemThread.interrupt();
            mMonitModemThread = null;
        }
        if (mSharedPreferences == null
                || mSharedPreferences.getString(
                        C2KLogUtils.KEY_C2K_MODEM_LOGGING_PATH, "").length() == 0) {
            Log.i("saber", "The service has been stopped!");
            return;
        }

        if (mEtsLog.mStopLogrecord) {
            return;
        }

        mEtsLog.stop();
        C2KLogRecycle.getInstance().stopRecycle();
        mSharedPreferences.edit().putString(C2KLogUtils.KEY_C2K_MODEM_LOGGING_PATH, "").apply();
        setEtsChannel(0);
        Log.i("saber", "<============  Destroy Done!");
    }

    /**
     * @param channel int
     */
    private void setEtsChannel(int channel) {
        Log.i("saber", "setEtsChannel channel = " + channel);
        Intent intent = new Intent(C2KLogUtils.ACTION_VIA_SET_ETS_DEV);
        intent.putExtra(C2KLogUtils.EXTRAL_VIA_ETS_DEV, channel);
        C2KLogUtils.sendBroadCast(mService, intent);
        if (channel == 0) {
            return;
        }

        mIsSetChannelDone = false;
        int i = 0;
        while (!mIsSetChannelDone) {
            if (i >= 60) {
                Log.w("saber", "set channel timeout" + channel);
                break;
            }
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                Log.w("saber", "sleep failed?");
                break;
            }
            i++;
        }
        mIsSetChannelDone = true;
        Log.i("saber", "set channel done, time is " + i);
    }

    /**
     * transfer long time to time string.
     * @param time
     *            long type
     * @return ex: 2012_1221_2359
     */
    private String translateTime(long time) {
        GregorianCalendar calendar = new GregorianCalendar();
        DecimalFormat df = new DecimalFormat();
        String pattern = "00";
        df.applyPattern(pattern);
        calendar.setTime(new Date(time));

        int year = calendar.get(Calendar.YEAR);
        int month = calendar.get(Calendar.MONTH) + 1;
        int day = calendar.get(Calendar.DAY_OF_MONTH);
        int hour = calendar.get(Calendar.HOUR_OF_DAY);
        int minu = calendar.get(Calendar.MINUTE);
        int second = calendar.get(Calendar.SECOND);
        return "" + year + "_" + df.format(month) + df.format(day) + "_" + df.format(hour)
                + df.format(minu) + df.format(second);
    }

    private final BroadcastReceiver mBcRecverCbp = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            Log.i("saber", "I am bc receiver of cbp in log service of saber");
            if (intent.getAction().equals(ACTION_VIA_ETS_DEV_CHANGED)) {
                boolean resultOk = intent.getBooleanExtra(
                        C2KLogUtils.EXTRAL_VIA_ETS_DEV_RESULT, false);
                Log.i("saber", "The result for set SDIO channel is " + resultOk
                        + ", mIsSetChannelDone = " + mIsSetChannelDone);
                if (mIsSetChannelDone) {
                    // It is not in open channel, ignore the result
                    return;
                } else {
                    mIsSetChannelDone = true;
                }
            }
        }
    };

    private final BroadcastReceiver mRebootReceiver = new BroadcastReceiver() {

        private boolean mIsStopDone = false;
        @Override
        public void onReceive(final Context context, final Intent intent) {
            Log.i("saber", "I am bc receiver of mRebootReceiver");
            if (intent.getAction().equals(Intent.ACTION_SHUTDOWN)) {
                Log.i("saber", "-->System is normal SHUTDOWN now, save buffer to file.");
                mIsStopDone = false;
                new Thread(new Runnable() {
                    public void run() {
                        destroy();
                        mIsStopDone = true;
                    }
                }).start();

                int i = 0;
                while (!mIsStopDone && i++ < 60) {
                    try {
                        Thread.sleep(100);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }
                }
                Log.i("saber", "<--System is normal SHUTDOWN now, save buffer to file!");
            }
        }
    };

    /**
     * @author MTK81255
     *
     */
    private class MyHandler extends Handler {

        public MyHandler(Looper looper) {
            super(looper);
        }

        public void handleMessage(Message msg) {
            int what = msg.what;
            Log.d("saber", "mMessageHandler, what = " + what);
            if (what == MSG_SET_CHANNEL) {
                Log.i("saber", "Re-setChannelForStart");
                setChannelForStart();
            }
        };
    };

}
