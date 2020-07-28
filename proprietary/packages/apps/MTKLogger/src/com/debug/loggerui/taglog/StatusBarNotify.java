package com.debug.loggerui.taglog;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.content.Context;
import android.content.SharedPreferences;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.R;
import com.debug.loggerui.utils.Utils;

import java.util.Timer;
import java.util.TimerTask;

/**
 * @author MTK11515
 *
 */
public class StatusBarNotify {
    private static final String TAG = TagLogUtils.TAGLOG_TAG + "/StatusBarHandler";
    private Context mContext;
    private SharedPreferences mDefaultSharedPreferences;
    private int mZippedFileCount = 0;
    private int mZipFileTotalCount = 0;
    // Used to refresh compress progress shown in status bar
    private Timer mZipRateTimer;
    private static final int MONITOR_TIMER = 200;
    private int mLastZippedFilesCount = -1;
    private static NotificationManager sNotificationManager = null;
    private Notification.Builder mNotificationBuilder = null;
    private String mStatusBarTitle = "";
    private int mStatusBarId = 0;
    private Object mLock = new Object();

    /**
     * @param title String
     * @param id int
     */
    public StatusBarNotify(String title, int id) {
        Utils.logd(TAG, "StatusBarHandler start, title = " + title);
        mDefaultSharedPreferences = MyApplication.getInstance()
                .getDefaultSharedPreferences();
        mContext = MyApplication.getInstance();
        sNotificationManager = (NotificationManager) mContext
                .getSystemService(Context.NOTIFICATION_SERVICE);
        mStatusBarTitle = title.replace("TMP_", "");
        mStatusBarId = id;
    }

    /**
     * @param msgNum
     *            int
     * @param fileCount
     *            int
     */
    public void updateState(int msgNum, int fileCount) {
        switch (msgNum) {
        case TagLogUtils.MSG_ZIPPED_FILE_COUNT:
            mZippedFileCount = fileCount;
            if (mZippedFileCount >= mZipFileTotalCount) {
                hidewZipProgressStatusBar();
                mZippedFileCount = 0;
                mZipFileTotalCount = 0;
            }
            Utils.logd(TAG, "mZippedFileCount = " + mZippedFileCount);
            break;
        case TagLogUtils.MSG_ZIP_FILE_TOTAL_COUNT:
            if (mZipFileTotalCount == 0) {
                showZipProgressStatusBar();
            }
            mZipFileTotalCount = fileCount;
            Utils.logd(TAG, "mZipFileTotalCount = " + mZipFileTotalCount);
            break;
        default:
            break;
        }
    }

    private static final String NOTIFICATION_TAGLOG_CHANNEL_ID =
            "com.debug.loggerui.taglog.notification";
    private void showZipProgressStatusBar() {
        Utils.logd(TAG, "-->showZipProgressStatusBar()");
        if (!mDefaultSharedPreferences.getBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED,
                Utils.DEFAULT_NOTIFICATION_ENABLED_VALUE)) {
            Utils.logw(TAG, "Notification is disabled, does not show any notification.");
            return;
        }
        // Prepare notification in status bar
        sNotificationManager.cancel(mStatusBarId);
        try {
            mNotificationBuilder = new Notification.Builder(mContext);
            mNotificationBuilder
                    .setContentTitle(
                            mContext.getString(R.string.notification_title_taglog_compressing))
                    .setTicker(
                            mContext.getString(R.string.notification_title_taglog_compressing))
                    .setProgress(mZipFileTotalCount, mZippedFileCount, false)
                    .setSmallIcon(R.drawable.ic_notification_taglog_compressing)
                    .setContentText(mStatusBarTitle);

            NotificationChannel channnel = new NotificationChannel(NOTIFICATION_TAGLOG_CHANNEL_ID,
                    "TagLog", NotificationManager.IMPORTANCE_LOW);
            sNotificationManager.createNotificationChannel(channnel);
            mNotificationBuilder.setChannelId(NOTIFICATION_TAGLOG_CHANNEL_ID);
            sNotificationManager.notify(mStatusBarId, mNotificationBuilder.build());
        } catch (ClassCastException e) {
            Utils.loge(TAG, "ClassCastException mNotificationManager error!");
        } catch (ArrayIndexOutOfBoundsException e) {
            Utils.loge(TAG, "ArrayIndexOutOfBoundsException mNotificationManager error!");
        }

        if (mZipRateTimer != null) {
            mZipRateTimer.cancel();
        }
        mZipRateTimer = new Timer();
        mLastZippedFilesCount = -1;
        mZipRateTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                if (mZippedFileCount == mLastZippedFilesCount) {
                    return;
                }
                mLastZippedFilesCount = mZippedFileCount;
                synchronized (mLock) {
                    if (mNotificationBuilder != null) {
                        mNotificationBuilder.setProgress(
                                mZipFileTotalCount, mZippedFileCount, false);
                        try {
                            sNotificationManager.notify(mStatusBarId, mNotificationBuilder.build());
                        } catch (ClassCastException e) {
                            Utils.loge(TAG, "ClassCastException mNotificationManager error!");
                        } catch (ArrayIndexOutOfBoundsException e) {
                            Utils.loge(TAG,
                                    "ArrayIndexOutOfBoundsException mNotificationManager error!");
                        } catch (NullPointerException npe) {
                            Utils.loge(TAG, "NullPointerException mNotificationManager error!");
                        } catch (IllegalArgumentException iae) {
                            Utils.loge(TAG, "IllegalArgumentException mNotificationManager error!");
                        }
                    }
                }
            }
        }, 0, MONITOR_TIMER);
    }

    private void hidewZipProgressStatusBar() {
        Utils.logd(TAG, "-->hideTagLogFromStatusBar()");
        if (!mDefaultSharedPreferences.getBoolean(Utils.KEY_PREFERENCE_NOTIFICATION_ENABLED,
                Utils.DEFAULT_NOTIFICATION_ENABLED_VALUE)) {
            Utils.logw(TAG, "Notification is disabled, does not show any notification.");
            return;
        }
        if (mZipRateTimer != null) {
            mZipRateTimer.cancel();
        }
        synchronized (mLock) {
            if (mNotificationBuilder != null) {
                try {
                    mNotificationBuilder
                            .setContentTitle(
                                    mContext.getString(
                                            R.string.notification_title_taglog_compress_success))
                            .setTicker(
                                    mContext.getString(
                                            R.string.notification_title_taglog_compress_success))
                            .setSmallIcon(R.drawable.ic_notification_taglog_compress_success)
                            .setContentText(mStatusBarTitle)
                            .setAutoCancel(true).setProgress(0, 0, false);
                    sNotificationManager.notify(mStatusBarId, mNotificationBuilder.build());
                } catch (ClassCastException e) {
                    Utils.loge(TAG, "ClassCastException mNotificationManager error!");
                } catch (ArrayIndexOutOfBoundsException e) {
                    Utils.loge(TAG, "ArrayIndexOutOfBoundsException mNotificationManager error!");
                } catch (NullPointerException npe) {
                    Utils.loge(TAG, "NullPointerException mNotificationManager error!");
                } catch (IllegalArgumentException iae) {
                    Utils.loge(TAG, "IllegalArgumentException mNotificationManager error!");
                }
            }
        }
        Utils.logd(TAG, "Taglog compress finished, wait 3s");
        if (mZipRateTimer != null) {
            mZipRateTimer.cancel();
        }
        mZipRateTimer = new Timer();
        mZipRateTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                if (sNotificationManager != null) {
                    sNotificationManager.cancel(mStatusBarId);
                }
            }
        }, 3000);
    }
}
