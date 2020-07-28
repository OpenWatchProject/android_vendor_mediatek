package com.mediatek.keyguard.Telephony;


import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.database.ContentObserver;
import android.os.Handler;
import android.os.SystemProperties;
import android.provider.Settings.System;
import android.view.LayoutInflater;
import android.view.WindowManager;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.IccCardConstants.State;
import com.android.keyguard.KeyguardUpdateMonitor;
import com.android.keyguard.KeyguardUpdateMonitorCallback;
import com.android.keyguard.KeyguardUtils;
import com.android.keyguard.R;

import android.util.Log;
import java.util.LinkedList;
import java.util.Queue;

public class KeyguardDialogManager {

    private static final String TAG = "KeyguardDialogManager";
    private static final boolean DEBUG = true;

    private static KeyguardDialogManager sInstance;

    private final Context mContext;
    private KeyguardUpdateMonitor mUpdateMonitor;

    /// M: Manage the dialog sequence.
    private DialogSequenceManager mDialogSequenceManager;

    /**
     * The type of dialog sequence for keyguard and STK dialogs
     * @hide
     */
    private static final String DIALOG_SEQUENCE_SETTINGS = "dialog_sequence_settings";

    /**
     * The dialog sequence default value
     * @hide
     */
    private static final int DIALOG_SEQUENCE_DEFAULT = 0;

    /**
     * The KEYGUARD dialog is showing
     * @hide
     */
    private static final int DIALOG_SEQUENCE_KEYGUARD = 1;

    private KeyguardDialogManager(Context context) {
        mContext = context;

        mDialogSequenceManager = new DialogSequenceManager();

        mUpdateMonitor = KeyguardUpdateMonitor.getInstance(context);
    }

    public static KeyguardDialogManager getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new KeyguardDialogManager(context);
        }
        return sInstance;
    }

     /**
     * M: interface is a call back for the user who need to popup Dialog.
     */
    public static interface DialogShowCallBack {
        public void show();
    }

    /**
     * M: request show dialog
     * @param callback the user need to implement the callback.
     */
    public void requestShowDialog(DialogShowCallBack callback) {
        //if (!KeyguardViewMediator.isKeyguardInActivity) {
            mDialogSequenceManager.requestShowDialog(callback);
        //} else {
        //    Log.d(TAG, "Ignore showing dialog in KeyguardMock");
        //}
    }

    /**
     * M: when the user close dialog, should report the status.
     */
    public void reportDialogClose() {
        mDialogSequenceManager.reportDialogClose();
    }

    /**
     * M: interface for showing dialog sequencely manager.
     *
     */
    public static interface SequenceDialog {
        /**
         * the client  needed to show a dialog should call this
         * @param callback the client should implement the callback.
         */
        public void requestShowDialog(DialogShowCallBack callback);
        /**
         * If the client close the dialog, should call this to report.
         */
        public void reportDialogClose();
    }

    /**
     * M: Manage the dialog sequence.
     * It implment the main logical of the sequence process.
     */
    private class DialogSequenceManager implements SequenceDialog {
        /// M: log tag for this class
        private static final String CLASS_TAG = "DialogSequenceManager";
        /// M: debug switch for the log.
        private static final boolean CLASS_DEBUG = true;
        /// M: The queue to save the call backs.
        private Queue<DialogShowCallBack> mDialogShowCallbackQueue;
        /// M: Whether the inner dialog is showing
        private boolean mInnerDialogShowing = false;
        /// M: If keyguard set the dialog sequence value, and inner dialog is showing.
        private boolean mLocked = false;

        public DialogSequenceManager() {
            if (CLASS_DEBUG) {
                Log.d(TAG, CLASS_TAG + " DialogSequenceManager()");
            }
            mDialogShowCallbackQueue = new LinkedList<DialogShowCallBack>();

            mContext.getContentResolver().registerContentObserver
                    (System.getUriFor(DIALOG_SEQUENCE_SETTINGS),
                    false, mDialogSequenceObserver);
         }

        public void requestShowDialog(DialogShowCallBack callback) {
            if (CLASS_DEBUG) {
                Log.d(TAG, CLASS_TAG + " --requestShowDialog()");
            }
            mDialogShowCallbackQueue.add(callback);
            handleShowDialog();
        }

        public void handleShowDialog() {
            if (CLASS_DEBUG) {
                Log.d(TAG, CLASS_TAG + " --handleShowDialog()--enableShow() = " + enableShow());
            }
            if (enableShow()) {
                if (getLocked()) {
                    DialogShowCallBack dialogCallBack = mDialogShowCallbackQueue.poll();
                    if (CLASS_DEBUG) {
                        Log.d(TAG, CLASS_TAG + " --handleShowDialog()--dialogCallBack = "
                            + dialogCallBack);
                    }
                    if (dialogCallBack != null) {
                        dialogCallBack.show();
                        setInnerDialogShowing(true);
                    }
                } else {
                    if (CLASS_DEBUG) {
                        Log.d(TAG, CLASS_TAG + " --handleShowDialog()--System.putInt( "
                                + DIALOG_SEQUENCE_SETTINGS + " value = "
                                + DIALOG_SEQUENCE_KEYGUARD);
                    }
                    System.putInt(mContext.getContentResolver(), DIALOG_SEQUENCE_SETTINGS,
                            DIALOG_SEQUENCE_KEYGUARD);
                }
            }
        }

        public void reportDialogClose() {
            if (CLASS_DEBUG) {
                Log.d(TAG, CLASS_TAG + " --reportDialogClose()--"
                        + "mDialogShowCallbackQueue.isEmpty() = "
                        + mDialogShowCallbackQueue.isEmpty());
            }
            setInnerDialogShowing(false);

            if (mDialogShowCallbackQueue.isEmpty()) {
                if (CLASS_DEBUG) {
                    Log.d(TAG, CLASS_TAG + " --reportDialogClose()--System.putInt( "
                            + DIALOG_SEQUENCE_SETTINGS + " value = "
                            + DIALOG_SEQUENCE_DEFAULT
                            + " --setLocked(false)--");
                }
                System.putInt(mContext.getContentResolver(), DIALOG_SEQUENCE_SETTINGS,
                        DIALOG_SEQUENCE_DEFAULT);
                setLocked(false);
            } else {
                handleShowDialog();
            }
        }

        /**
         * M : Combine the conditions to deceide whether enable showing or not
         */
        private boolean enableShow() {
            if (CLASS_DEBUG) {
                Log.d(TAG, CLASS_TAG + " --enableShow()-- !mDialogShowCallbackQueue.isEmpty() = "
                            + !mDialogShowCallbackQueue.isEmpty()
                        + " !getInnerDialogShowing() = " + !getInnerDialogShowing()
                        + " !isOtherModuleShowing() = " + !isOtherModuleShowing()
                        + " isDeviceProvisioned() = " + mUpdateMonitor.isDeviceProvisioned()
                        /*+ " !isOOBEShowing() = " + !isOOBEShowing()*/);
            }

            return !mDialogShowCallbackQueue.isEmpty() && !getInnerDialogShowing()
                    && !isOtherModuleShowing()
                    && mUpdateMonitor.isDeviceProvisioned()
                    && !KeyguardUtils.isSystemEncrypted();
        }

        /**
         * M : Query the dialog sequence settings to decide whether
         *   other module's dialog is showing or not.
         */
        private boolean isOtherModuleShowing() {
            int value = queryDialogSequenceSeetings();
            if (CLASS_DEBUG) {
                Log.d(TAG, CLASS_TAG + " --isOtherModuleShowing()--"
                    + DIALOG_SEQUENCE_SETTINGS + " = " + value);
            }
            if (value == DIALOG_SEQUENCE_DEFAULT
                || value == DIALOG_SEQUENCE_KEYGUARD) {
                return false;
            }
            return true;
        }

        private void setInnerDialogShowing(boolean show) {
            mInnerDialogShowing = show;
        }

        private boolean getInnerDialogShowing() {
            return mInnerDialogShowing;
        }

        private void setLocked(boolean locked) {
            mLocked = locked;
        }

        private boolean getLocked() {
            return mLocked;
        }

        /**
         * M : Query dialog sequence settings value
         */
        private int queryDialogSequenceSeetings() {
            int value = System.getInt(mContext.getContentResolver(),
                    DIALOG_SEQUENCE_SETTINGS,
                    DIALOG_SEQUENCE_DEFAULT);
            return value;
        }

        /// M: dialog sequence observer for dialog sequence settings
        private ContentObserver mDialogSequenceObserver = new ContentObserver(new Handler()) {
            @Override
            public void onChange(boolean selfChange) {
                int value = queryDialogSequenceSeetings();
                if (CLASS_DEBUG) {
                    Log.d(TAG, CLASS_TAG + " DialogSequenceObserver--onChange()--"
                            + DIALOG_SEQUENCE_SETTINGS + " = " + value);
                }
                if (value == DIALOG_SEQUENCE_DEFAULT) {
                    setLocked(false);
                    handleShowDialog();
                } else if (value == DIALOG_SEQUENCE_KEYGUARD) {
                    setLocked(true);
                    handleShowDialog();
                }
            }
        };
    }

}
