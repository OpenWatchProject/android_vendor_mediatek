package com.debug.loggerui.framework;

import android.app.AlertDialog;
import android.content.DialogInterface;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.view.WindowManager;

import com.debug.loggerui.MyApplication;
import com.debug.loggerui.R;
import com.debug.loggerui.utils.Utils;

/**
 * SecurityWarning.
 *
 */
public class SecurityWarning {
    private static final String TAG = Utils.TAG + "/SecurityWarning";

    private static SecurityWarning sInstance;
    private AlertHandler mAlertHandler;
    public static final int MSG_BOOT_COMPLETE = 1;
    public static final int MSG_START_LOGS = 2;
    public static final int MSG_STOP_LOGS = 3;

    private SecurityWarning() {
        mAlertHandler = new AlertHandler(Looper.getMainLooper());
    }

    /**
     * @return SecurityWarning
     */
    public static SecurityWarning getInstance() {
        if (sInstance == null) {
            synchronized (SecurityWarning.class) {
                if (sInstance == null) {
                    sInstance = new SecurityWarning();
                }
            }
        }
        return sInstance;
    }

    /**
     * @return boolean
     */
    public static boolean isNeedAlert() {
        return Utils.isCustomerUserLoad()
                && !Utils.LOG_PATH_TYPE_SYSTEM_DATA.equals(Utils.getCurrentLogPathType());
    }

    public Handler getAlertHander() {
        return mAlertHandler;
    }

    /**
     * AlertHandler.
     *
     */
    class AlertHandler extends Handler {
        public AlertHandler(Looper lopper) {
            super(lopper);
        }

        @Override
        public void handleMessage(Message msg) {
            Utils.logi(TAG, "handleMessage msg.what = " + msg.what);
            DialogExcute dialogExcute = null;
            if (msg.obj != null && msg.obj instanceof DialogExcute) {
                dialogExcute = (DialogExcute) msg.obj;
            } else {
                Utils.logw(TAG,
                        "The msg.obj is null or not instanceof DialogExcute, can not do anything!");
                return;
            }
            switch (msg.what) {
            case MSG_BOOT_COMPLETE:
                alertWarningDialog(dialogExcute, MyApplication.getInstance().getApplicationContext()
                        .getString(R.string.message_start_log_confirm), true);
                return;
            case MSG_START_LOGS:
                alertWarningDialog(dialogExcute, MyApplication.getInstance().getApplicationContext()
                        .getString(R.string.message_start_log_confirm), true);
                return;
            case MSG_STOP_LOGS:
                alertWarningDialog(dialogExcute, MyApplication.getInstance().getApplicationContext()
                        .getString(R.string.message_stop_log_confirm), false);
                return;
            default:
                return;
            }
        }

        private void alertWarningDialog(DialogExcute dialogExcute, String message,
                boolean isNeedCancelButton) {
            AlertDialog.Builder build =
                    new AlertDialog.Builder(MyApplication.getInstance().getApplicationContext())
                            .setTitle(R.string.security_warning_dlg_title)
                            .setIcon(android.R.drawable.ic_dialog_alert)
                            .setMessage(message).setPositiveButton(
                                    android.R.string.ok, new DialogInterface.OnClickListener() {
                                        @Override
                                        public void onClick(DialogInterface dialog,
                                                int whichButton) {
                                            dialogExcute.excute(whichButton);
                                        }
                                    });
            if (isNeedCancelButton) {
                build.setNegativeButton(android.R.string.cancel,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                Utils.logw(TAG, "Do not start logs after confime.");
                                dialogExcute.excute(whichButton);
                            }
                        });
            }
            AlertDialog alertDialog = build.create();
            alertDialog.getWindow().setType(WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY);
            alertDialog.setCancelable(false);
            alertDialog.show();
            return;
        }
    }

    /**
     * DialogExcute.
     *
     */
    public abstract static class DialogExcute {
        /**
         * @param whichButton
         *            int
         */
        public void excute(int whichButton) {
            switch (whichButton) {
            case DialogInterface.BUTTON_POSITIVE:
                okButtonClicked();
                return;
            case DialogInterface.BUTTON_NEGATIVE:
                cancelButtonClicked();
                return;
            default:
                return;
            }
        }

        /**
         * Deal with OK button clicked.
         */
        public abstract void okButtonClicked();

        /**
         * Deal with cancel button clicked.
         */
        public abstract void cancelButtonClicked();
    }
}
