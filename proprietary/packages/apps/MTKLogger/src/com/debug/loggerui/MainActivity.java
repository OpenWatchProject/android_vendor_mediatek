package com.debug.loggerui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.StatFs;
import android.os.SystemClock;
import android.text.InputType;
import android.text.method.NumberKeyListener;
import android.util.SparseArray;
import android.util.SparseIntArray;
import android.util.TypedValue;
import android.view.ActionMode;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager;
import com.debug.loggerui.framework.DebugLoggerUIServiceManager.ServiceNullException;
import com.debug.loggerui.framework.SecurityWarning;
import com.debug.loggerui.permission.PermissionUtils;
import com.debug.loggerui.settings.SettingsActivity;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.Timer;
import java.util.TimerTask;

/**
 * @author MTK81255
 *
 */
public class MainActivity extends Activity {
    private static final String TAG = Utils.TAG + "/MainActivity";

    private static final int ALPHA_FULL = 255;
    private static final int ALPHA_GRAY = 75;
    private static final int ALPHA_TRANSPARENT = 0;

    private static final int TIMER_PERIOD = 1000;
    private static final int START_STOP_TIMER = 25 * TIMER_PERIOD;
    private static final int SDCARD_RATIO_BAR_TIMER = 30 * TIMER_PERIOD;
    private static final int TAGLOG_PROGRESS_DIALOG_SHOW_TIMEOUT = 60 * TIMER_PERIOD;

    private static final int MSG_TIMER = 1;
    private static final int MSG_WAITING_DIALOG_TIMER = 2;
    private static final int MSG_MONITOR_SDCARD_BAR = 3;
//    private static final int DLG_WAITING_SERVICE = 10;
    private static final int TAGLOG_TIMER_CHECK = 11;

    /**
     * At boot time, since DebugLoggerUI service will not start up before receiving boot_completed
     * broadcast, to avoid status confusion, blocking UI until we receive such broadcast. Use this
     * message to monitor boot_completed broadcast coming status
     */
    private static final int MSG_WAITING_SERVICE_READY = 5;
    private static final int MESSAGE_DELAY_WAITING_SERVICE = 500; // ms
    private static final int WAITING_SERVICE_READY_TIMER = 60 * TIMER_PERIOD;

    /**
     * If log exist, clear log button will be enabled, else, disable it. Since
     * IO operation may need a long time, put such check operation into thread
     */
    private static final int MSG_CHECK_LOG_EXIST = 6;

    /**
     * Before enter clear log activity. We need to check current log folder
     * status. Do such things in main thread may hang UI, so need to put them
     * into thread
     */
//    private static final int DLG_CHECKING_LOG_FILES = 11;
    private static final int MSG_SHOW_CHECKING_LOG_DIALOG = 7;
    private static final int MSG_REMOVE_CHECKING_LOG_DIALOG = 8;
    private static final int MSG_WAITING_GET_MODEM_STATUS_READY = 9;
    private static final int WAITING_CHECKING_LOG_TIMER = 600 * TIMER_PERIOD;
    private static final int WAITING_GET_MDLOG_STATUS_TIMER = 20 * TIMER_PERIOD;

    private SharedPreferences mSharedPreferences;

    private MenuItem mSettingsMenuItem;

    static final SparseIntArray KEY_LOG_TOGGLE_BUTTON_ID_MAP = new SparseIntArray();
    static {
        KEY_LOG_TOGGLE_BUTTON_ID_MAP.put(Utils.LOG_TYPE_MOBILE,
                R.id.mobileLogStartStopToggleButton);
        KEY_LOG_TOGGLE_BUTTON_ID_MAP.put(Utils.LOG_TYPE_MODEM,
                R.id.modemLogStartStopToggleButton);
        KEY_LOG_TOGGLE_BUTTON_ID_MAP.put(Utils.LOG_TYPE_NETWORK,
                R.id.networkLogStartStopToggleButton);
        KEY_LOG_TOGGLE_BUTTON_ID_MAP.put(Utils.LOG_TYPE_CONNSYS,
                R.id.connsysLogStartStopToggleButton);
        KEY_LOG_TOGGLE_BUTTON_ID_MAP.put(Utils.LOG_TYPE_MET,
                R.id.metLogStartStopToggleButton);
    }
    static final SparseIntArray KEY_LOG_TEXT_VIEW_ID_MAP = new SparseIntArray();
    static {
        KEY_LOG_TEXT_VIEW_ID_MAP.put(Utils.LOG_TYPE_MOBILE, R.id.mobileLogTextView);
        KEY_LOG_TEXT_VIEW_ID_MAP.put(Utils.LOG_TYPE_MODEM, R.id.modemLogTextView);
        KEY_LOG_TEXT_VIEW_ID_MAP.put(Utils.LOG_TYPE_NETWORK, R.id.networkLogTextView);
        KEY_LOG_TEXT_VIEW_ID_MAP.put(Utils.LOG_TYPE_CONNSYS, R.id.connsysLogTextView);
        KEY_LOG_TEXT_VIEW_ID_MAP.put(Utils.LOG_TYPE_MET, R.id.metLogTextView);
    }
    private SparseArray<TypeLogLayout> mTypeLogLayout = new SparseArray<TypeLogLayout>();

    private TextView mTimeText;
    private TextView mSavePathText;
    private LinearColorBar mSdcardRatioBar;
    private TextView mStorageChartLabelText;
    private TextView mUsedStorageText;
    private TextView mFreeStorageText;
    private ImageButton mTagImageButton;
    private ToggleButton mStartStopToggleButton;
    private ImageButton mClearLogImageButton;

    private String mSDCardPathStr;
    private String mSavePathStr;
    private int mAvailableStorageSize = -1;

    private Timer mTimer;
    private long mTimeMillisecond = 0;
    float mNormalTimeTextSize = 1.0f;
    private boolean mIsUpdateTimerFirst = false;

    private ProgressDialog mWaitingDialog;
    private Timer mMonitorTimer;

    private boolean mAlreadyNotifyUserSDNotReady = false;
    RefreshStorageAsyncTask mRefreshStorageTask = null;
    private ProgressDialog mTagProgressDialog;

    private UpdateUITask mUpdateUITask = null;

    private Context mContext;
    private Activity mActivity;

    /**
     * Use this flag to make sure taglog dialog not been clicked twice.
     */
    private boolean mIsTaglogClicked = false;

    private boolean mIsAutoTest = false;


    /**
     * Flag for whether we are now calculating storage used ratio, if already
     * being doing so, blocking duplicated refresh request.
     */
    private boolean mWaitingRefreshStatusBar = false;

    private BroadcastReceiver mServiceReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Utils.ACTION_LOG_STATE_CHANGED)) {
                Utils.logi(TAG, "ACTION_LOG_STATE_CHANGED");
                stopWaitingDialog();
                String failReason = intent.getStringExtra(Utils.EXTRA_FAIL_REASON);
                if (failReason != null && !"".equals(failReason)) {
                    Utils.logd(TAG, "ACTION_LOG_STATE_CHANGED : failReason = " + failReason);
                    Toast.makeText(MainActivity.this, analyzeReason(failReason), Toast.LENGTH_SHORT)
                            .show();
                }
                updateUI();
            } else if (action.equals(Utils.EXTRA_RUNNING_STAGE_CHANGE_EVENT)) {
                Utils.logi(TAG, "EXTRA_RUNNING_STAGE_CHANGE_EVENT");
                int runningStage = intent.getIntExtra(Utils.EXTRA_RUNNING_STAGE_CHANGE_VALUE,
                                   Utils.RUNNING_STAGE_IDLE);
                changeWaitingDialog(runningStage);
            }
        }
    };

    /**
     * When storage status changed, need to update U to check whether need to
     * refresh current log path.
     */
    private BroadcastReceiver mStorageStatusReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Utils.logi(TAG, "Storage status changed, update UI now");
            try {
                Uri data = intent.getData();
                String affectedPath = null;
                if (data != null) {
                    affectedPath = data.getPath();
                }
                if (affectedPath != null && DebugLoggerUIServiceManager.getInstance().getService()
                        .isAffectCurrentLogType(affectedPath)) {
                    mMessageHandler.sendEmptyMessage(MSG_MONITOR_SDCARD_BAR);
                    updateUI();
                } else {
                    Utils.logi(TAG,
                            "The storage status changed not affect current log path, just igore!");
                }
            } catch (ServiceNullException e) {
                e.printStackTrace();
            }
        }
    };

    private String analyzeReason(String reason) {
        String rsReason = "";
        for (int i = 0; i < reason.length(); ) {
            int index = reason.indexOf(":", i);
            if (index == -1) {
                break;
            }
            int logType = -1;
            try {
                logType = Integer.parseInt(reason.substring(i, index));
            } catch (NumberFormatException e) {
                logType = -1;
            }
            if (!Utils.LOG_TYPE_SET.contains(logType)) {
                break;
            }
            i = index + 1;
            index = reason.indexOf(";", i);
            if (index == -1) {
                break;
            }
            String reasonType = reason.substring(i, index);

            // For error reason return start
            int reasonKey = 0;
            try {
                reasonKey = Utils.FAIL_REASON_DETAIL_MAP.get(reasonType);
            } catch (NullPointerException e) {
                Utils.logd(TAG, "NullPointerException;");
                reasonKey = -1;
                rsReason += getString(Utils.LOG_NAME_MAP.get(logType)) + " : " + reasonType + "\n";
                Utils.logd(TAG, "analyzeReason:after " + rsReason);
                return rsReason;
            }
            if (reasonKey == -1) {
                i = index + 1;
                break;
            }
            // For error reason return end
            rsReason +=
                    getString(Utils.LOG_NAME_MAP.get(logType)) + " : " + getString(reasonKey)
                            + "\n";
            i = index + 1;
        }
        return rsReason;
    }

    private void changeWaitingDialog(int currentRunningStage) {
        dismissTagProgressDialog();
        Utils.logi(TAG, "changeWaitingDialog() -> currentRunningStage is " + currentRunningStage);
        if (currentRunningStage == Utils.RUNNING_STAGE_IDLE) {
            stopWaitingDialog();
            if (mAvailableStorageSize <= Utils.DEFAULT_STORAGE_WATER_LEVEL
                    && mAvailableStorageSize > Utils.RESERVED_STORAGE_SIZE
                    && LogControllerUtils.isAnyControlledLogRunning()) {
                alertLowStorageDialog();
            }
        } else {
            String title = "";
            String message = "";
            int timeout = 0;
            if (currentRunningStage == Utils.RUNNING_STAGE_STARTING_LOG) {
                title = getString(R.string.waiting_dialog_title_start_log);
                message = getString(R.string.waiting_dialog_message_start_log);
                timeout = START_STOP_TIMER;
            } else if (currentRunningStage == Utils.RUNNING_STAGE_STOPPING_LOG) {
                title = getString(R.string.waiting_dialog_title_stop_log);
                message = getString(R.string.waiting_dialog_message_stop_log);
                timeout = START_STOP_TIMER;
            } else if (currentRunningStage == Utils.RUNNING_STAGE_RESTARTING_LOG) {
                title = getString(R.string.waiting_dialog_title_restart_log);
                message = getString(R.string.waiting_dialog_message_restart_log);
                timeout = START_STOP_TIMER * 2;
            } else if (currentRunningStage == Utils.RUNNING_STAGE_POLLING_LOG) {
                title = getString(R.string.waiting_dialog_title_poll_log);
                message = getString(R.string.waiting_dialog_message_poll_log);
            } else if (currentRunningStage == Utils.RUNNING_STAGE_FLUSHING_LOG) {
                title = getString(R.string.waiting_dialog_title_flush_log);
                message = getString(R.string.waiting_dialog_message_flush_log);
            }
            showWaitingDialog(title, message, timeout);
        }
    }

    private void showWaitingDialog(String title, String message, int timeout) {
        // Before show dialog, our process may be killed by system, then the
        // dialog.show() method
        // will pop up JE. Do such check before dialog show
        boolean isFinishingFlag = isFinishing();
        Utils.logv(TAG, "Before show dialog, isFinishingFlag=" + isFinishingFlag);
        if (!isFinishingFlag ) {
            if (mWaitingDialog == null) {
                mWaitingDialog = new ProgressDialog(MainActivity.this);
                mWaitingDialog.setIndeterminate(true);
                mWaitingDialog.setCancelable(false);
            }
            mWaitingDialog.setTitle(title);
            mWaitingDialog.setMessage(message);
            mWaitingDialog.show();
            Utils.logi(TAG, "showWaitingDialog() -> title = " + title
                    + ", message = " + message + ", timeout = " + timeout);
        }
        mMessageHandler.removeMessages(MSG_WAITING_DIALOG_TIMER);
        if (timeout > 0) {
            mMessageHandler.sendMessageDelayed(
                    mMessageHandler.obtainMessage(MSG_WAITING_DIALOG_TIMER, message), timeout);
        }
    }

    private Handler mMessageHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MSG_TIMER:
                mTimeMillisecond += 1;
                setTimeText();
                break;
            case TAGLOG_TIMER_CHECK:
                Utils.logw(TAG,
                        "mMessageHandler->handleMessage() Tag Progress Dialog shows timeout,"
                        + " maybe some error happended for manual TagLog!");
                dismissTagProgressDialog();
                break;
            case MSG_WAITING_DIALOG_TIMER:
                String message = "";
                if (msg.obj != null && msg.obj instanceof String) {
                    message = (String) msg.obj;
                }
                Utils.logd(TAG,
                        "mMessageHandler->handleMessage() "
                        + "msg.what == MSG_WAITING_DIALOG_TIMER, msg.obj = " + message);
                if (message.equals(getString(R.string.waiting_service_dialog_message))) {
                    mSharedPreferences.edit().putLong(
                            Utils.KEY_BEGIN_RECORDING_TIME, SystemClock.elapsedRealtime()).apply();
                    updateUI();
                } else {
                    stopWaitingDialog();
                }
                break;
            case MSG_MONITOR_SDCARD_BAR:
                Utils.logd(TAG,
                        "mMessageHandler->handleMessage() msg.what == MSG_MONITOR_SDCARD_BAR");
                refreshSdcardBar();
                break;
            case MSG_WAITING_SERVICE_READY:
                boolean isServiceReady = true;
                try {
                    isServiceReady =
                            (DebugLoggerUIServiceManager.getInstance().getService() != null);
                } catch (ServiceNullException e) {
                    isServiceReady = false;
                }
                if (!isServiceReady) {
                    showWaitingDialog("", getString(R.string.waiting_service_dialog_message),
                            WAITING_SERVICE_READY_TIMER);
                    mMessageHandler.removeMessages(MSG_WAITING_SERVICE_READY);
                    mMessageHandler.sendMessageDelayed(
                            mMessageHandler.obtainMessage(MSG_WAITING_SERVICE_READY),
                            MESSAGE_DELAY_WAITING_SERVICE);
                    break;
                } else {
                    Utils.logi(TAG, "Log service is ready, release UI blocking.");
                    stopWaitingDialog();
                    updateUI();
                }
                break;
            case MSG_CHECK_LOG_EXIST:
                Utils.logi(TAG, "Receive check existing log folder done message," + " result="
                        + msg.obj);
                boolean isLogExists = (Boolean) msg.obj;
                Utils.logv(TAG, "isLogExists=" + isLogExists);
                if (isLogExists) {
                    mClearLogImageButton.setEnabled(true);
                    mClearLogImageButton.setImageAlpha(ALPHA_FULL);
                } else {
                    mClearLogImageButton.setEnabled(false);
                    mClearLogImageButton.setImageAlpha(ALPHA_GRAY);
                }
                break;
            case MSG_SHOW_CHECKING_LOG_DIALOG:
                Utils.logv(TAG, "Show waiting checking log files dialog now.");
                showWaitingDialog("", getString(R.string.waiting_checking_log_dialog_message),
                        WAITING_CHECKING_LOG_TIMER);
                break;
            case MSG_REMOVE_CHECKING_LOG_DIALOG:
                Utils.logv(TAG, "Remove waiting checking log files dialog now.");
                stopWaitingDialog();
                break;
            case MSG_WAITING_GET_MODEM_STATUS_READY:
                showWaitingDialog("", getString(R.string.waiting_getstatus_dialog_message),
                                  WAITING_GET_MDLOG_STATUS_TIMER);
                break;
            default:
                Utils.logw(TAG, "Unknown message");
                break;
            }
        }
    };

    /*
     * (non-Javadoc)
     * @see android.app.Activity#onCreate(android.os.Bundle)
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Utils.logi(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        mContext = this;
        mActivity = this;
        findViews();
        initViews();
        setListeners();
        PermissionUtils.requestStoragePermissions();
    }

    @Override
    protected void onResume() {
        Utils.logi(TAG, "-->onResume");

        IntentFilter serviceIntent = new IntentFilter();
        serviceIntent.addAction(Utils.ACTION_LOG_STATE_CHANGED);
        serviceIntent.addAction(Utils.EXTRA_RUNNING_STAGE_CHANGE_EVENT);
        registerReceiver(mServiceReceiver, serviceIntent,
                "android.permission.DUMP", null);

        IntentFilter sdStatusIntentFilter = new IntentFilter();
        sdStatusIntentFilter.addAction(Intent.ACTION_MEDIA_BAD_REMOVAL);
        sdStatusIntentFilter.addAction(Intent.ACTION_MEDIA_EJECT);
        sdStatusIntentFilter.addAction(Intent.ACTION_MEDIA_REMOVED);
        sdStatusIntentFilter.addAction(Intent.ACTION_MEDIA_UNMOUNTED);
        sdStatusIntentFilter.addAction(Intent.ACTION_MEDIA_MOUNTED);
        sdStatusIntentFilter.addDataScheme("file");
        registerReceiver(mStorageStatusReceiver, sdStatusIntentFilter,
                "android.permission.DUMP", null);
        disabledAllButton();
        updateUI();

        monitorSdcardRatioBar();
        super.onResume();
    }

    @Override
    protected void onPause() {
        Utils.logi(TAG, "onPause");
        super.onPause();
        if (mMonitorTimer != null) {
            mMonitorTimer.cancel();
            mMonitorTimer = null;
        }
        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
        try {
            unregisterReceiver(mServiceReceiver);
            unregisterReceiver(mStorageStatusReceiver);
        } catch (IllegalArgumentException e) {
            Utils.logd(TAG, "unregisterReceiver failed!");
        }
        mMessageHandler.removeMessages(MSG_WAITING_SERVICE_READY);
        stopWaitingDialog();
        dismissTagProgressDialog();
        if (mRefreshStorageTask != null) {
            mRefreshStorageTask.cancel(true);
            mWaitingRefreshStatusBar = false;
        }
        if (mUpdateUITask != null) {
            mUpdateUITask.cancel(true);
        }
    }

    @Override
    protected void onStop() {
        Utils.logi(TAG, "onStop");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        Utils.logi(TAG, "onDestroy");
        dismissTagProgressDialog();
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        mSettingsMenuItem = menu.add(getString(R.string.menu_settings));
        mSettingsMenuItem.setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        mSettingsMenuItem.setIcon(R.drawable.ic_settings);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        if (!Utils.isDeviceOwner()) {
            Utils.logi(TAG, "In multi user case, only device owner can change log configuration");
            Toast.makeText(this, R.string.warning_no_permission_for_setting, Toast.LENGTH_SHORT)
                    .show();
            return true;
        }
        Utils.logi(TAG, "SettingsActivity open!");
        Intent intent = new Intent(this, SettingsActivity.class);
        intent.setClass(this, SettingsActivity.class);
        startActivity(intent);
        return true;
    }

    private void findViews() {
        mTypeLogLayout.put(Utils.LOG_TYPE_MOBILE, new TypeLogLayout(Utils.LOG_TYPE_MOBILE));
        mTypeLogLayout.put(Utils.LOG_TYPE_MODEM, new TypeLogLayout(Utils.LOG_TYPE_MODEM));
        mTypeLogLayout.put(Utils.LOG_TYPE_NETWORK, new TypeLogLayout(Utils.LOG_TYPE_NETWORK));
        mTypeLogLayout.put(Utils.LOG_TYPE_CONNSYS, new TypeLogLayout(Utils.LOG_TYPE_CONNSYS));
        mTypeLogLayout.put(Utils.LOG_TYPE_MET, new TypeLogLayout(Utils.LOG_TYPE_MET));

        mTimeText = (TextView) findViewById(R.id.timeTextView);
        mSavePathText = (TextView) findViewById(R.id.savePathTextView);

        mSdcardRatioBar = (LinearColorBar) findViewById(R.id.storage_color_bar);
        mStorageChartLabelText = (TextView) findViewById(R.id.storageChartLabel);
        mUsedStorageText = (TextView) findViewById(R.id.usedStorageText);
        mFreeStorageText = (TextView) findViewById(R.id.freeStorageText);
        mTagImageButton = (ImageButton) findViewById(R.id.tagImageButton);
        mStartStopToggleButton = (ToggleButton) findViewById(R.id.startStopToggleButton);
        mClearLogImageButton = (ImageButton) findViewById(R.id.clearLogImageButton);
    }

    private void initViews() {
        Utils.logv(TAG, "-->initViews()");
        mSharedPreferences = getSharedPreferences(Utils.CONFIG_FILE_NAME, Context.MODE_PRIVATE);
        initMetLog();
        float fontScale = getResources().getConfiguration().fontScale;
        Utils.logd(TAG, "fontScale = " + fontScale);
        mIsUpdateTimerFirst = true;
        mTimeText.setTextSize(TypedValue.COMPLEX_UNIT_PX, mTimeText.getTextSize() / fontScale);

        for (int i = 0; i < mTypeLogLayout.size(); i++) {
            int logType = mTypeLogLayout.keyAt(i);
            TextView logTextView = mTypeLogLayout.valueAt(i).getLogStatusTextView();
            logTextView.setText(
                    getString(R.string.log_stop, getString(Utils.LOG_NAME_MAP.get(logType))));
            logTextView.setTextSize(TypedValue.COMPLEX_UNIT_PX,
                    logTextView.getTextSize() / fontScale);
        }
        mSavePathText.setTextSize(TypedValue.COMPLEX_UNIT_PX, mSavePathText.getTextSize()
                / fontScale);
        mUsedStorageText.setTextSize(TypedValue.COMPLEX_UNIT_PX,
                mUsedStorageText.getTextSize() / fontScale);
        mFreeStorageText.setTextSize(TypedValue.COMPLEX_UNIT_PX,
                mFreeStorageText.getTextSize() / fontScale);
    }

    private void initMetLog() {
        if (!mSharedPreferences.getBoolean(Utils.MET_LOG_ENABLE, false)) {
            TypeLogLayout metLogLayout = mTypeLogLayout.get(Utils.LOG_TYPE_MET);
            if (metLogLayout != null) {
                metLogLayout.getLogStartStopToggleButton().setVisibility(View.GONE);
                metLogLayout.getLogStatusTextView().setVisibility(View.GONE);
                mTypeLogLayout.remove(Utils.LOG_TYPE_MET);
            }
        } else {
            TypeLogLayout metLogLayout = mTypeLogLayout.get(Utils.LOG_TYPE_MET);
            if (metLogLayout == null) {
                metLogLayout = new TypeLogLayout(Utils.LOG_TYPE_MET);
                mTypeLogLayout.put(Utils.LOG_TYPE_MET, metLogLayout);
            }
            metLogLayout.getLogStartStopToggleButton().setVisibility(View.VISIBLE);
            metLogLayout.getLogStatusTextView().setVisibility(View.VISIBLE);
        }
    }

    private void alertLowStorageDialog() {
        new AlertDialog.Builder(MainActivity.this)
                .setTitle(R.string.low_storage_warning_dialog_title)
                .setMessage(
                        getString(R.string.low_storage_warning_dialog_msg,
                                Utils.DEFAULT_STORAGE_WATER_LEVEL))
                .setPositiveButton(android.R.string.ok,
                        new android.content.DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int which) {
                                dialog.dismiss();
                            }
                        }).show();
    }

    private void setListeners() {
        for (int i = 0; i < mTypeLogLayout.size(); i++) {
            int logType = mTypeLogLayout.keyAt(i);
            mTypeLogLayout.valueAt(i).getLogStartStopToggleButton().
                setOnClickListener(new OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (v instanceof ToggleButton) {
                        ToggleButton button = (ToggleButton) v;
                        if (button.isChecked()) {
                            startLogs(logType, button);
                        } else {
                            stopLogs(logType, button);
                        }
                    }
                }
            });
        }
        mSavePathText.setOnClickListener(new OnClickListener() {

            @Override
            public void onClick(View arg0) {
                try {
                    ComponentName component = new ComponentName("com.android.documentsui",
                            "com.android.documentsui.files.FilesActivity");
                    Intent intent = new Intent();
                    intent.setComponent(component);
                    intent.setAction("android.intent.action.VIEW");
                    String uriStr = "content://com.android.externalstorage.documents/";
                    File logFolder = new File(mSavePathStr);
                    if (Utils.LOG_PATH_TYPE_SYSTEM_DATA.equals(
                            Utils.getCurrentLogPathType())) {
                        return;
                    } else if (logFolder.getParentFile() == null
                            || !logFolder.getParentFile().exists()) {
                        return;
                    } else if (logFolder.exists()) {
                        uriStr += "document/";
                        if (Utils.LOG_PATH_TYPE_DEVICE_STORAGE.equals(
                                Utils.getCurrentLogPathType())) {
                            uriStr += "primary:debuglogger";
                        } else if (Utils.LOG_PATH_TYPE_PORTABLE_STORAGE.equals(
                                Utils.getCurrentLogPathType())) {
                            uriStr += logFolder.getParentFile().getName() + ":debuglogger";
                        }
                    } else {
                        uriStr += "root/";
                        if (Utils.LOG_PATH_TYPE_DEVICE_STORAGE.equals(
                                Utils.getCurrentLogPathType())) {
                            uriStr += "primary";
                        } else if (Utils.LOG_PATH_TYPE_PORTABLE_STORAGE.equals(
                                Utils.getCurrentLogPathType())) {
                            uriStr += logFolder.getParentFile().getName();
                        }
                    }
                    Utils.logi(TAG, "uriStr = " + uriStr);
                    intent.setData(Uri.parse(uriStr));
                    startActivity(intent);
                } catch (ActivityNotFoundException anfe) {
                    Utils.logw(TAG, "Can not find com.android.documentsui, maybe not build in,"
                            + " just do noting!");
                }
            }
        });

        mStartStopToggleButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                if (v instanceof ToggleButton) {
                    ToggleButton button = (ToggleButton) v;
                    boolean isChecked = button.isChecked();
                    if (!Utils.isDeviceOwner()) {
                        Toast.makeText(mContext, R.string.info_not_device_owner,
                                Toast.LENGTH_LONG).show();
                        button.setChecked(!isChecked);
                        return;
                    }
                    if (isChecked) {
                        int enabledLogType = 0;
                        for (Integer logType : Utils.LOG_TYPE_SET) {
                            if (LogControllerUtils.getLogControllerInstance(logType)
                                    .isLogFeatureSupport()
                                && LogControllerUtils.getLogControllerInstance(logType)
                                    .isLogControlled()) {
                                enabledLogType |= logType;
                            }
                        }
                        if (enabledLogType == 0) {
                            button.setChecked(!isChecked);
                            Utils.logw(TAG, "No log type was enabled in settings page.");
                            Toast.makeText(MainActivity.this, getString(R.string.no_log_on),
                                    Toast.LENGTH_SHORT).show();
                            return;
                        }
                        startLogs(enabledLogType, button);
                    } else {
                        stopLogs(Utils.LOG_TYPE_ALL, button);
                    }
                }
            }
        });

        mClearLogImageButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                Utils.logi(TAG, "ClearLogs button is clicked!");
                if (!checkPath()) {
                    return;
                }
                Intent intent = new Intent(mContext, LogFolderListActivity.class);
                intent.setClass(mContext, LogFolderListActivity.class);
                startActivity(intent);
            }
        });

        mTagImageButton.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View arg0) {
                Utils.logi(TAG, "Taglog button is clicked!");
                tagLogs();
            }
        });
    }

    private void startLogs(int logType, ToggleButton button) {
        Utils.logi(TAG, "startLogs log:" + logType);
        if (SecurityWarning.isNeedAlert()) {
            SecurityWarning.DialogExcute dialogExcute =
                    new SecurityWarning.DialogExcute() {
                @Override
                public void okButtonClicked() {
                    startRecording(logType);
                }

                @Override
                public void cancelButtonClicked() {
                    Utils.logw(TAG, "Do not start logs after confime.");
                    button.setChecked(!button.isChecked());
                }

            };
            SecurityWarning.getInstance().getAlertHander()
            .obtainMessage(SecurityWarning.MSG_START_LOGS, dialogExcute)
            .sendToTarget();
        } else {
            startRecording(logType);
        }
    }

    private void stopLogs(int logType, ToggleButton button) {
        Utils.logi(TAG, "stopLogs log:" + logType);
        if (SecurityWarning.isNeedAlert()) {
            SecurityWarning.DialogExcute dialogExcute =
                    new SecurityWarning.DialogExcute() {
                @Override
                public void okButtonClicked() {
                    stopRecording(logType);
                }

                @Override
                public void cancelButtonClicked() {
                }
            };
            SecurityWarning.getInstance().getAlertHander()
            .obtainMessage(SecurityWarning.MSG_STOP_LOGS, dialogExcute)
            .sendToTarget();
        } else {
            // Need stop all logs even UI is disabled.
            // Maybe some type logs not start from UI.
            stopRecording(logType);
        }
    }

    private void startRecording(final int logType) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    DebugLoggerUIServiceManager.getInstance().getService()
                            .startRecording(logType, Utils.LOG_START_STOP_REASON_FROM_UI);
                } catch (ServiceNullException e) {
                    return;
                }
            }
        }).start();
        changeWaitingDialog(
                Utils.RUNNING_STAGE_STARTING_LOG);
    }

    private void stopRecording(final int logType) {
        new Thread(new Runnable() {
            @Override
            public void run() {
                try {
                    DebugLoggerUIServiceManager.getInstance().getService()
                            .stopRecording(logType, Utils.LOG_START_STOP_REASON_FROM_UI);
                } catch (ServiceNullException e) {
                    return;
                }
            }
        }).start();
        changeWaitingDialog(
                Utils.RUNNING_STAGE_STOPPING_LOG);
    }

    /**
     * @author MTK81255
     *
     */
    private class UpdateUITask extends AsyncTask<Void, Void, Void> {
        // Do the long-running work in here
        private int mMdlogStatus = Utils.RUNNING_STAGE_IDLE;
        @Override
        protected Void doInBackground(Void... params) {
            // Update for Save path
            if (isCancelled()) {
                return null;
            }
            mSDCardPathStr = Utils.getCurrentLogPath();
            if (isCancelled()) {
                return null;
            }
            mAvailableStorageSize = Utils.getAvailableStorageSize(mSDCardPathStr);
            Utils.logd(TAG, " mSDCardPathStr=" + mSDCardPathStr + ", mAvailableStorageSize = "
                    + mAvailableStorageSize);
            if (mSDCardPathStr == null || mSDCardPathStr.isEmpty()) {
                mSavePathStr = "Can not find storage!";
            } else {
                mSavePathStr = mSDCardPathStr + Utils.LOG_PATH_PARENT;
            }
            mIsAutoTest = Utils.isAutoTest();
            try {
                mMdlogStatus = DebugLoggerUIServiceManager.getInstance().getService()
                               .getCurrentRunningStage();
            } catch (ServiceNullException e) {
                Utils.logw(TAG, "get mdlog status fail.");
            }
            mMessageHandler.removeMessages(MSG_WAITING_GET_MODEM_STATUS_READY);
            return null;
        }

        // This is called when doInBackground() is finished
        @Override
        protected void onPostExecute(Void result) {
            updateUI();
        }

        private void updateUI() {
            Utils.logi(TAG, "-->updateUI(), Update UI Start");
            if (isCancelled()) {
                return;
            }
            changeWaitingDialog(mMdlogStatus);
            mSavePathText.setText(getString(R.string.log_path_str) + ": " + mSavePathStr);
            if (Utils.LOG_PATH_TYPE_SYSTEM_DATA.equals(Utils.getCurrentLogPathType())
                    || new File(mSavePathStr).getParentFile() == null
                    || !new File(mSavePathStr).getParentFile().exists()) {
                mSavePathText.setTextColor(android.graphics.Color.BLACK);
                mSavePathText.setEnabled(false);
            } else {
                mSavePathText.setTextColor(android.graphics.Color.BLUE);
                mSavePathText.setEnabled(true);
            }
            mStorageChartLabelText
                    .setText(Utils.LOG_PATH_TYPE_STRING_MAPS.get(Utils.getCurrentLogPathType()));
            // Do re-init for MET log if back from settings page.
            initMetLog();
            boolean isStart = false;
            // Update for logs status of UI
            for (int i = 0; i < mTypeLogLayout.size(); i++) {
                int logType = mTypeLogLayout.keyAt(i);
                TextView logTextView = mTypeLogLayout.valueAt(i).getLogStatusTextView();
                ToggleButton logToggleButton =
                        mTypeLogLayout.valueAt(i).getLogStartStopToggleButton();

                boolean isLogcontrolled = LogControllerUtils.getLogControllerInstance(logType)
                        .isLogControlled();
                logToggleButton.setEnabled(isLogcontrolled);
                logToggleButton.setAlpha(isLogcontrolled ? ALPHA_FULL : ALPHA_TRANSPARENT);

                boolean isLogStart = LogControllerUtils.getLogControllerInstance(logType)
                        .isLogRunning();
                if (Utils.LOG_TYPE_CONNSYS == logType) {
                    isLogStart = LogControllerUtils.isAnyConnsysLogRunning();
                }
                logToggleButton.setChecked(isLogStart);
                String textStr = getString(isLogcontrolled
                                ? (isLogStart ? R.string.log_start : R.string.log_stop)
                                : R.string.log_uncontrolled,
                        getString(Utils.LOG_NAME_MAP.get(logType)));
                logTextView.setText(textStr);
                if (isLogcontrolled && isLogStart) {
                    isStart = true;
                }
            }
            mStartStopToggleButton.setChecked(isStart);
            // Update for record time refresh
            if (isStart) {
                startTimer();
            } else {
                stopTimer();
            }
            // Update for tag log button
            if (isStart && Utils.isTaglogEnable()) {
                mTagImageButton.setEnabled(true);
                mTagImageButton.setImageAlpha(ALPHA_FULL);
            } else {
                mTagImageButton.setEnabled(false);
                mTagImageButton.setImageAlpha(ALPHA_GRAY);
            }

            // Disable clear log button until check log file done
            mClearLogImageButton.setEnabled(false);
            mClearLogImageButton.setImageAlpha(ALPHA_GRAY);

            // Update for auto test & not device Owner
            if (mIsAutoTest) {
                disabledAllButton();
            } else if (!Utils.isDeviceOwner()) {
                disabledAllButton();
                // If is not device owner,
                // need show toast to notify user why start/stop can not used.
                mStartStopToggleButton.setEnabled(true);
            } else {
                if (mSettingsMenuItem != null) {
                    mSettingsMenuItem.setEnabled(true);
                }
                mStartStopToggleButton.setEnabled(true);
                updateClearLogImageButtonStatus();
            }
        }
    }

    private void updateClearLogImageButtonStatus() {
        new Thread(new Runnable() {
            @Override
            public void run() {
                Utils.logd(TAG, "Start a new thread to check whether existing any log now.");
                mMessageHandler.obtainMessage(MSG_CHECK_LOG_EXIST,
                        LogControllerUtils.getCanBeDeletedFileList().size() > 0).sendToTarget();
            }
        }).start();
    }

    private void disabledAllButton() {
        if (mSettingsMenuItem != null) {
            mSettingsMenuItem.setEnabled(false);
        }
        mStartStopToggleButton.setEnabled(false);

        for (int i = 0; i < mTypeLogLayout.size(); i++) {
            mTypeLogLayout.valueAt(i).getLogStartStopToggleButton().setEnabled(false);
        }

        mSavePathText.setEnabled(false);
        mSavePathText.setTextColor(android.graphics.Color.BLACK);
        mTagImageButton.setEnabled(false);
        mTagImageButton.setImageAlpha(ALPHA_GRAY);
        mClearLogImageButton.setEnabled(false);
        mClearLogImageButton.setImageAlpha(ALPHA_GRAY);
    }

    private void startTimer() {
        long startTime =
                mSharedPreferences.getLong(Utils.KEY_BEGIN_RECORDING_TIME,
                        Utils.VALUE_BEGIN_RECORDING_TIME_DEFAULT);
        if (startTime == 0) {
            mTimeMillisecond = 0;
        } else {
            mTimeMillisecond = (SystemClock.elapsedRealtime() - startTime) / 1000;
        }
        Utils.logi(TAG, "timeString : " + calculateTimer());
        setTimeText();
        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
        mTimer = new Timer(true);
        mTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                mMessageHandler.sendEmptyMessage(MSG_TIMER);
            }
        }, TIMER_PERIOD, TIMER_PERIOD);
    }

    private void stopTimer() {
        long startTime =
                mSharedPreferences.getLong(Utils.KEY_BEGIN_RECORDING_TIME,
                        Utils.VALUE_BEGIN_RECORDING_TIME_DEFAULT);
        long endTime = mSharedPreferences.getLong(Utils.KEY_END_RECORDING_TIME,
                Utils.VALUE_BEGIN_RECORDING_TIME_DEFAULT);
        mTimeMillisecond = (endTime - startTime) / 1000;
        Utils.logi(TAG, "timeString : " + calculateTimer());
        setTimeText();
        if (mTimer != null) {
            mTimer.cancel();
            mTimer = null;
        }
    }

    private void setTimeText() {
        String timeString = calculateTimer();
        mTimeText.setText(timeString);
        if (mIsUpdateTimerFirst) {
            mNormalTimeTextSize = mTimeText.getTextSize();
            mIsUpdateTimerFirst = false;
        }
        int timeLength = timeString.length(); // 0:00:00-9999:59:59
        float rate = (17 - timeLength) / 10f;
        mTimeText.setTextSize(TypedValue.COMPLEX_UNIT_PX, mNormalTimeTextSize * rate);
    }

    private String calculateTimer() {
        if (mTimeMillisecond < 0) {
            Utils.logi(TAG, "System time adjust to past, just reset log time to 0,"
                    + " to avoid UI show incorrect");
            mTimeMillisecond = 0;
        }

        int hour = (int) mTimeMillisecond / 3600;
        if (hour > 9999) {
            Utils.loge(TAG, "There is something wrong with time record! The hour is " + hour);
            mTimeMillisecond = 0;
            Utils.loge(TAG, "There is something wrong with time record!");
        }
        int minute = (int) mTimeMillisecond / 60 % 60;
        long second = mTimeMillisecond % 60;
        String timerStr =
                "" + hour + ":" + (minute < 10 ? "0" : "") + minute + ":"
                        + (second < 10 ? "0" : "") + second;

        return timerStr;
    }

    private void monitorSdcardRatioBar() {
        if (mMonitorTimer != null) {
            return;
        }
        mMonitorTimer = new Timer(true);
        mMonitorTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                mMessageHandler.sendEmptyMessage(MSG_MONITOR_SDCARD_BAR);
            }
        }, 0, SDCARD_RATIO_BAR_TIMER);
    }

    private void refreshSdcardBar() {
        Utils.logd(TAG, "-->refreshSdcardBar()");
        if (mWaitingRefreshStatusBar) {
            Utils.logi(TAG, " Last refresh request not finished yet, just wait a moment.");
        } else {
            mRefreshStorageTask = new RefreshStorageAsyncTask();
            mRefreshStorageTask.execute();
        }
    }

    /**
     * Get storage information need to access SD card, to void blocking UI
     * thread, put it into background task.
     */
    class RefreshStorageAsyncTask extends AsyncTask<Void, Void, Void> {
        private boolean mIsPathOK = false;
        private long mUsedStorageSize;
        private long mFreeStorageSize;
        private long mMtkLogSize;

        @Override
        protected Void doInBackground(Void... params) {
            mWaitingRefreshStatusBar = true;
            if (isCancelled()) {
                return null;
            }
            mIsPathOK = checkPath();
            if (!mIsPathOK) {
                return null;
            }
            int retryNum = 1;
            while (retryNum <= 3) {
                try {
                    if (isCancelled()) {
                        return null;
                    }
                    StatFs statFs = new StatFs(mSDCardPathStr);
                    long blockSize = statFs.getBlockSizeLong() / 1024;
                    mFreeStorageSize = statFs.getAvailableBlocksLong() * blockSize;
                    mUsedStorageSize = statFs.getBlockCountLong() * blockSize - mFreeStorageSize;
                    Utils.logd(TAG, " mSDCardPathStr=" + mSDCardPathStr + ", free size="
                            + mFreeStorageSize + "KB, used size=" + mUsedStorageSize + "KB");
                    mMtkLogSize = Utils.getFileSize(mSavePathStr);
                    // For show GradientPaint, set the minimum size of
                    // mtkLogSize == 1024bit
                    if (mMtkLogSize <= 1024) {
                        mMtkLogSize = 1024;
                    }
                    return null;
                } catch (IllegalArgumentException e) {
                    Utils.loge(TAG, "Fail to get storage info from [" + mSDCardPathStr
                            + "] by StatFs, try again(index=" + retryNum + ").", e);
                    try {
                        Thread.sleep(200);
                    } catch (InterruptedException e1) {
                        e1.printStackTrace();
                    }
                    mFreeStorageSize = 0;
                    mUsedStorageSize = 0;
                    mMtkLogSize = 1024;
                }
                retryNum++;
            }
            Utils.loge(TAG, "Fail to get [" + mSDCardPathStr + "]storage info through StatFs");
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            Utils.logv(TAG, " -->RefreshStorageAsyncTask.refreshStatusBar()");
            if (isCancelled()) {
                return;
            }
            if (!mIsPathOK) {
                Utils.logd(TAG, "Selected log path is unavailable, reset storage info");
                Toast.makeText(MainActivity.this,
                        getString(R.string.log_path_not_exist, mSDCardPathStr), Toast.LENGTH_SHORT)
                        .show();
                mSdcardRatioBar.setGradientPaint(0, 0);
                mSdcardRatioBar.setRatios(0, 0, 0);
                mUsedStorageText.setText("0M " + getString(R.string.log_used_storage));
                mFreeStorageText.setText("0M " + getString(R.string.log_free_storage));
                mWaitingRefreshStatusBar = false;
                return;
            }
            mSdcardRatioBar.setShowingGreen(false);
            // Add a tiny extra in storage size to avoid divide 0 exception
            float ratio = 1.0f * mUsedStorageSize / (mFreeStorageSize + mUsedStorageSize + 1);
            mSdcardRatioBar.setGradientPaint(ratio - 1.0f * (mMtkLogSize / 1024)
                    / (mFreeStorageSize + mUsedStorageSize + 1), ratio);
            mSdcardRatioBar.setRatios(0, ratio, 1 - ratio);
            mUsedStorageText.setText((int) (mUsedStorageSize / 1024) + "M "
                    + getString(R.string.log_used_storage));
            mFreeStorageText.setText((int) (mFreeStorageSize / 1024) + "M "
                    + getString(R.string.log_free_storage));
            mWaitingRefreshStatusBar = false;
        }

        @Override
        protected void onCancelled(Void result) {
            Utils.logi(TAG, "RefreshStorageAsyncTask is cancelled.");
            mWaitingRefreshStatusBar = false;
            super.onCancelled(result);
        }
    }

    private synchronized void stopWaitingDialog() {
        mMessageHandler.removeMessages(MSG_WAITING_SERVICE_READY);
        mMessageHandler.removeMessages(MSG_WAITING_DIALOG_TIMER);
        if (mWaitingDialog != null) {
            try {
                mWaitingDialog.cancel();
                mWaitingDialog = null;
            } catch (IllegalArgumentException e) {
                mWaitingDialog = null;
                Utils.logd(TAG, "exception happened when cancel waitingdialog.");
            }
        }
    }

    private void tagLogs() {
        if (!mStartStopToggleButton.isChecked()) {
            return;
        }

        Builder builder = new AlertDialog.Builder(this);
        final EditText inputText = new EditText(this);
        inputText.setCustomSelectionActionModeCallback(new ActionMode.Callback() {
            @Override
            public boolean onCreateActionMode(ActionMode actionMode, Menu menu) {
                return false;
            }

            @Override
            public boolean onPrepareActionMode(ActionMode actionMode, Menu menu) {
                return false;
            }

            @Override
            public boolean onActionItemClicked(ActionMode actionMode, MenuItem menuItem) {
                return false;
            }

            @Override
            public void onDestroyActionMode(ActionMode actionMode) {
            }
        });
        inputText.setLongClickable(false);
        inputText.setTextIsSelectable(false);

        inputText.setKeyListener(new NumberKeyListener() {
            @Override
            public int getInputType() {
                return InputType.TYPE_TEXT_FLAG_CAP_WORDS;
            }

            @Override
            protected char[] getAcceptedChars() {
                char[] numberChars =
                        { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '0',
                                '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', ' ', 'A', 'B',
                                'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
                                'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
                return numberChars;
            }
        });

        mIsTaglogClicked = false;
        builder.setTitle(R.string.taglog_title)
                .setMessage(R.string.taglog_msg_input)
                .setView(inputText)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (mIsTaglogClicked) {
                            Utils.logd(TAG,
                                    "Dialog button is already clicked, do not click OK again");
                            return;
                        }
                        mIsTaglogClicked = true;
                        // inputText.setImeOptions(EditorInfo.IME_ACTION_DONE);
                        InputMethodManager inputManager =
                                (InputMethodManager) MainActivity.this
                                        .getSystemService(Context.INPUT_METHOD_SERVICE);
                        inputManager.hideSoftInputFromWindow(inputText.getWindowToken(), 0);

                        // User input tag log name
                        String mTag = inputText.getText().toString().trim();
                        Utils.logi(TAG, "Input tag: " + mTag);
                        new Thread(new Runnable() {
                            @Override
                            public void run() {
                                try {
                                    DebugLoggerUIServiceManager.getInstance().getService()
                                            .beginTagLog(mTag);
                                } catch (ServiceNullException e) {
                                    return;
                                }
                            }
                        }).start();
                        createTagProgressDialog();
                    }
                })
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (mIsTaglogClicked) {
                            Utils.logd(TAG,
                                    "Dialog button is already clicked, do not click Cancel again");
                            return;
                        }
                        mIsTaglogClicked = true;
                        Utils.logv(TAG, " Cancel Taglog operation manually.");
                    }
                });
        Dialog mDialog = builder.create();
        mDialog.setCancelable(false);
        mDialog.show();
    }

    private boolean checkPath() {
        boolean isExist = true;
        mSDCardPathStr = Utils.getCurrentLogPath();
        if (mSDCardPathStr != null && !mSDCardPathStr.isEmpty()) {
            // Update for Save path
            mAvailableStorageSize = Utils.getAvailableStorageSize(mSDCardPathStr);
            Utils.logd(TAG, " mSDCardPathStr=" + mSDCardPathStr + ", mAvailableStorageSize = "
                    + mAvailableStorageSize);
            mSavePathStr = mSDCardPathStr + Utils.LOG_PATH_PARENT;
        } else {
            Utils.loge(TAG, "mSDCardPathStr is null!");
            mSavePathStr = "Can not find storage!";
            return false;
        }
        if (!new File(mSDCardPathStr).exists()) {
            isExist = false;
        }
        Utils.logd(TAG, "-->checkPath(), path=" + mSDCardPathStr + ", exist?" + isExist);
        // For /data, should not judge its volume state
        return isExist;
    }

    private void createTagProgressDialog() {
        Utils.logd(TAG, "-->createTagProgressDialog()");
        boolean isFinishingFlag = isFinishing();
        Utils.logv(TAG, "Before show dialog, isFinishingFlag=" + isFinishingFlag);
        if (mTagProgressDialog == null && !isFinishingFlag) {
            mTagProgressDialog =
                    ProgressDialog.show(MainActivity.this, getString(R.string.taglog_title),
                            getString(R.string.taglog_msg_tag_log), true, false);
            mMessageHandler.sendMessageDelayed(mMessageHandler.obtainMessage(TAGLOG_TIMER_CHECK),
                    TAGLOG_PROGRESS_DIALOG_SHOW_TIMEOUT);
        }
        return;
    }

    private void dismissTagProgressDialog() {
        Utils.logi(TAG, "-->dismissTagProgressDialog()");
        if (null != mTagProgressDialog) {
            mMessageHandler.removeMessages(TAGLOG_TIMER_CHECK);
            mTagProgressDialog.dismiss();
            mTagProgressDialog = null;
        }
    }

    /**
     * Update UI.
     */
    private void updateUI() {
        boolean isServiceReady;
        try {
            isServiceReady = (DebugLoggerUIServiceManager.getInstance().getService() != null);
        } catch (ServiceNullException e) {
            isServiceReady = false;
        }
        if (!isServiceReady) {
            mMessageHandler.sendEmptyMessage(MSG_WAITING_SERVICE_READY);
            return;
        }
        mMessageHandler.sendMessageDelayed(
                mMessageHandler.obtainMessage(MSG_WAITING_GET_MODEM_STATUS_READY),
                                              2 * MESSAGE_DELAY_WAITING_SERVICE);
        mUpdateUITask = new UpdateUITask();
        mUpdateUITask.execute();
    }

    /**
     * @author mtk
     *
     */
    class TypeLogLayout {
        ToggleButton mLogStartStopToggleButton;
        TextView mLogStatusTextView;
        int mLogType = 0;
        public TypeLogLayout(int logType) {
            this.mLogType = logType;
            mLogStartStopToggleButton = (ToggleButton) mActivity.
                    findViewById(KEY_LOG_TOGGLE_BUTTON_ID_MAP.get(logType));
            mLogStatusTextView = (TextView) mActivity.
                    findViewById(KEY_LOG_TEXT_VIEW_ID_MAP.get(logType));
        }

        public ToggleButton getLogStartStopToggleButton() {
            return mLogStartStopToggleButton;
        }

        public TextView getLogStatusTextView() {
            return mLogStatusTextView;
        }

    }

}
