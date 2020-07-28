/*
* Copyright (C) 2016 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.soundrecorder;

import android.Manifest;
import android.app.Activity;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.DialogInterface.OnMultiChoiceClickListener;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.PowerManager;
import android.os.PowerManager.WakeLock;
import android.util.AttributeSet;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.LayoutInflater.Factory;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.mediatek.soundrecorder.ext.ExtensionHelper;
import com.mediatek.soundrecorder.ext.IRecordingTimeCalculationExt;

import java.io.File;
import java.lang.reflect.Constructor;
import java.lang.reflect.InvocationTargetException;
import java.util.ArrayList;
import java.util.List;

/**
 * Sound Recorder Activity.
 */
public class SoundRecorder extends Activity implements SoundRecorderService.OnEventListener,
        SoundRecorderService.OnErrorListener, SoundRecorderService.OnStateChangedListener,
        Button.OnClickListener, SoundRecorderService.OnUpdateTimeViewListener {

    private static final String TAG = "SR/SoundRecorder";
    private static final String NULL_STRING = "";
    private static final int OPTIONMENU_SELECT_FORMAT = 0;
    private static final int OPTIONMENU_SELECT_MODE = 1;
    private static final int OPTIONMENU_SELECT_EFFECT = 2;
    private static final int DIALOG_SELECT_MODE = 0;
    private static final int DIALOG_SELECT_FORMAT = 1;
    public static final int DIALOG_SELECT_EFFECT = 2;
    private static final int TWO_BUTTON_WEIGHT_SUM = 2;
    private static final int THREE_BUTTON_WEIGHT_SUM = 3;
    private static final int REQURST_FILE_LIST = 1;
    private static final int TIME_BASE = 60;
    private static final long MAX_FILE_SIZE_NULL = -1L;
    private static final int TIME_NINE_MIN = 540;
    private static final int MMS_FILE_LIMIT = 190;
    private static final long ONE_SECOND = 1000;
    private static final int DONE = 100;
    public static final int TWO_LEVELS = 2;
    public static final int THREE_LEVELS = 3;
    private static final int PERMISSION_RECORD_AUDIO = 1;
    private static final int PERMISSION_READ_STORAGE = 2;
    private static final int PERMISSION_READ_STORAGE_LIST = 3;
    private static final int PERMISSION_WRITE_STORAGE_LIST = 4;
    private static final String INTENT_ACTION_MAIN = "android.intent.action.MAIN";
    private static final String EXTRA_MAX_BYTES =
            android.provider.MediaStore.Audio.Media.EXTRA_MAX_BYTES;
    private static final String AUDIO_NOT_LIMIT_TYPE = "audio/*";
    private static final String DIALOG_TAG_SELECT_MODE = "SelectMode";
    private static final String DIALOG_TAG_SELECT_FORMAT = "SelectFormat";
    private static final String DIALOG_TAG_SELECT_EFFECT = "SelectEffect";
    private static final String SOUND_RECORDER_DATA = "sound_recorder_data";
    private static final String PATH = "path";
    public static final String PLAY = "play";
    public static final String RECORD = "record";
    public static final String INIT = "init";
    public static final String DOWHAT = "dowhat";
    public static final String EMPTY = "";
    public static final String ERROR_CODE = "errorCode";

    private int mSelectedFormat = -1;
    private int mSelectedMode = -1;
    private boolean[] mSelectEffectArray = new boolean[3];
    private boolean[] mSelectEffectArrayTemp = new boolean[3];

    private String mRequestedType = AUDIO_NOT_LIMIT_TYPE;
    private String mTimerFormat = null;
    private String mFileName = "";
    private String mDoWhat = null;
    private String mDoWhatFilePath = null;
    private long mMaxFileSize = -1L;
    private boolean mRunFromLauncher = true;
    private boolean mHasFileSizeLimitation = false;
    private boolean mBackPressed = false;
    private boolean mOnSaveInstanceStateHasRun = false;
    private WakeLock mWakeLock = null;
    private boolean mIsStopService = false;
    // M: used for saving record file when SoundRecorder launch from other
    // application
    private boolean mSetResultAfterSave = true;
    // private WakeLock mWakeLock = null;
    private SharedPreferences mPrefs = null;
    private boolean mIsButtonDisabled = false;

    private Menu mMenu = null;
    private Button mAcceptButton;
    private Button mDiscardButton;
    private ImageButton mRecordButton;
    private ImageButton mPlayButton;
    private ImageButton mStopButton;
    private ImageButton mFileListButton;
    private ImageButton mPauseRecordingButton;
    // image view at the left of mStateTextView
    private ImageView mRecordingStateImageView;
    // image view at the left of mRecordingFileNameTextView
    private ImageView mPlayingStateImageView;
    private TextView mRemainingTimeTextView; // message below the state message
    private TextView mStateTextView; // state message with LED
    private TextView mTimerTextView;
    private TextView mRecordingFileNameTextView;
    private ProgressBar mStateProgressBar;
    private LinearLayout mExitButtons;
    private VUMeter mVUMeter;
    private LinearLayout mButtonParent;
    private OnScreenHint mStorageHint;
    private ImageView mFirstLine;
    private ImageView mSecondLine;
    private FrameLayout mFrameLayout;
    private boolean mIsRecordStarting = false;
    private boolean mDoRecordWhenResume = false;

    // M: add for long string in option menu
    private static final String LIST_MENUITEM_VIEW_NAME =
            "com.android.internal.view.menu.ListMenuItemView";
    private static final Class[] INFLATER_CONSTRUCTOR_SIGNATURE = new Class[] { Context.class,
            AttributeSet.class };
    private static Class sListMenuItemViewClass = null;
    private static Constructor sListMenuItemViewConstructor = null;

    private boolean mFileFromList = false;
    private boolean mResumeNeedRefresh = false;
    private boolean mSavingRecordFileFromMms = false;

    private SoundRecorderService mService = null;
    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName arg0, IBinder arg1) {
            PDebug.start("onServiceConnected");
            LogUtils.i(TAG, "<onServiceConnected> Service connected");
            mService = ((SoundRecorderService.SoundRecorderBinder) arg1).getService();
            initWhenHaveService();
            PDebug.end("onServiceConnected");
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            LogUtils.i(TAG, "<onServiceDisconnected> Service dis connected");
            mService = null;
        }
    };

    private DialogInterface.OnClickListener mSelectFormatListener =
            new DialogInterface.OnClickListener() {
        @Override
        public void onClick(DialogInterface dialog, int whichItemSelect) {
            LogUtils.i(TAG, "<mSelectFormatListener onClick>");
            setSelectedFormat(whichItemSelect);
            dialog.dismiss();
        }
    };

    private DialogInterface.OnClickListener mSelectModeListener =
            new DialogInterface.OnClickListener() {
        @Override
        public void onClick(DialogInterface dialog, int whichItemSelect) {
            LogUtils.i(TAG, "<mSelectModeListener onClick>");
            setSelectedMode(whichItemSelect);
            dialog.dismiss();
        }
    };

    private DialogInterface.OnClickListener mSelectEffectOkListener = new OnClickListener() {

        @Override
        public void onClick(DialogInterface arg0, int arg1) {
            mSelectEffectArray = mSelectEffectArrayTemp.clone();
            if (null != mService) {
                mService.setSelectEffectArray(mSelectEffectArray);
            }
        }
    };

    private DialogInterface.OnMultiChoiceClickListener
        mSelectEffectMultiChoiceClickListener = new OnMultiChoiceClickListener() {

        @Override
        public void onClick(DialogInterface arg0, int arg1, boolean arg2) {
            mSelectEffectArrayTemp[arg1] = arg2;
            if (null != mService) {
                mService.setSelectEffectArrayTmp(mSelectEffectArrayTemp);
            }
        }
    };

    private SoundRecorderService.OnUpdateButtonState mButtonUpdater =
            new SoundRecorderService.OnUpdateButtonState() {
                @Override
                public void updateButtonState(boolean enable) {
                    if (!enable) {
                        SoundRecorder.this.disableButton();
                    }
                }
            };

    @Override
    public void onEvent(int eventCode) {
        switch (eventCode) {
        case SoundRecorderService.EVENT_SAVE_SUCCESS:
            LogUtils.i(TAG, "<onEvent> EVENT_SAVE_SUCCESS");
            Uri uri = mService.getSaveFileUri();
            if (null != uri) {
                mHandler.sendEmptyMessage(SoundRecorderService.STATE_SAVE_SUCESS);
            }
            if (!mRunFromLauncher) {
                LogUtils.i(TAG, "<onEvent> mSetResultAfterSave = " + mSetResultAfterSave);
                if (mSetResultAfterSave) {
                    setResult(RESULT_OK, new Intent().setData(uri)
                                         .setFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION));
                    LogUtils.i(TAG, "<onEvent> finish");
                    LogUtils.i(TAG, "<onEvent> Activity = " + this.toString());
                    finish();
                } else {
                    mSetResultAfterSave = true;
                }
            }
            mService.reset();
            mHandler.sendEmptyMessage(mService.getCurrentState());
            long mEndSaveTime = System.currentTimeMillis();
            Log.i(TAG, "[Performance test][SoundRecorder] recording save end [" + mEndSaveTime
                    + "]");
            break;
        case SoundRecorderService.EVENT_DISCARD_SUCCESS:
            LogUtils.i(TAG, "<onEvent> EVENT_DISCARD_SUCCESS");
            if (mRunFromLauncher) {
                mService.reset();
                mHandler.sendEmptyMessage(mService.getCurrentState());
            } else {
                mService.reset();
                LogUtils.i(TAG, "<onEvent> finish");
                LogUtils.i(TAG, "<onEvent> Activity = " + this.toString());
                finish();
//              return;
            }
            break;
        case SoundRecorderService.EVENT_STORAGE_MOUNTED:
            LogUtils.i(TAG, "<onEvent> EVENT_STORAGE_MOUNTED");
            // remove error dialog after sd card mounted
            removeOldFragmentByTag(ErrorHandle.ERROR_DIALOG_TAG);
            break;
        default:
            LogUtils.i(TAG, "<onEvent> event out of range, event code = " + eventCode);
            break;
        }
    }

    @Override
    public void onStateChanged(int stateCode) {
        LogUtils.i(TAG, "<onStateChanged> change to " + stateCode);
        if (!mRunFromLauncher) {
            if (stateCode == SoundRecorderService.STATE_RECORDING) {
                acquireWakeLock();
            } else {
                releaseWakeLock();
            }
        }
        mHandler.removeMessages(stateCode);
        mHandler.sendEmptyMessage(stateCode);
    }

    @Override
    public void onError(int errorCode) {
        LogUtils.i(TAG, "<onError> errorCode = " + errorCode);
        // M: if OnSaveInstanceState has run, we do not show Dialogfragment now,
        // or else FragmentManager will throw IllegalStateException
        if (!mOnSaveInstanceStateHasRun) {
            Bundle bundle = new Bundle(1);
            bundle.putInt(ERROR_CODE, errorCode);
            Message msg = mHandler.obtainMessage(SoundRecorderService.STATE_ERROR);
            msg.setData(bundle);
            mHandler.sendMessage(msg);
        }
    }

    /**
     * Handle the button.
     *
     * @param button
     *            which button has been clicked
     */
    public void onClick(View button) {
        // avoid to response button click event when
        // activity is paused/stopped/destroy.
        if (isFinishing()) {
            return;
        }
        if (!button.isEnabled()) {
            return;
        }
        LogUtils.i(TAG, "<onClick> Activity = " + this.toString());
        switch (button.getId()) {
        case R.id.recordButton:
            LogUtils.i(TAG, "<onClick> recordButton");
            onClickRecordButton();
            break;
        case R.id.playButton:
            LogUtils.i(TAG, "<onClick> playButton");
            onClickPlayButton();
            break;
        case R.id.stopButton:
            LogUtils.i(TAG, "<onClick> stopButton");
            onClickStopButton();
            break;
        case R.id.acceptButton:
            LogUtils.i(TAG, "<onClick> acceptButton");
            onClickAcceptButton();
            break;
        case R.id.discardButton:
            LogUtils.i(TAG, "<onClick> discardButton");
            onClickDiscardButton();
            break;
        case R.id.fileListButton:
            onClickFileListButton();
            break;
        case R.id.pauseRecordingButton:
            LogUtils.i(TAG, "<onClick> pauseRecordingButton");
            onClickPauseRecordingButton();
            break;
        default:
            break;
        }
    }

    @Override
//    public void onCreate(Bundle icycle) {
    protected void onCreate(Bundle savedInstanceState) {
        PDebug.start("onCreate()");
        PDebug.start("onCreate -- super onCreate");
        super.onCreate(savedInstanceState);
        PDebug.end("onCreate -- super onCreate");
        LogUtils.i(TAG, "<onCreate> start, Activity = " + this.toString());
        /**
         * M: process the string of menu item too long
         */
        PDebug.start("onCreate -- addOptionsMenuInflaterFactory");
        addOptionsMenuInflaterFactory();
        PDebug.end("onCreate -- addOptionsMenuInflaterFactory");

        // init
        PDebug.start("onCreate -- setContentView");
        setContentView(R.layout.main);
        PDebug.end("onCreate -- setContentView");
        PDebug.start("onCreate -- initFromIntent");
        if (!initFromIntent()) {
            setResult(RESULT_CANCELED);
            finish();
            return;
        }
        PDebug.end("onCreate -- initFromIntent");
        if (!mRunFromLauncher) {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            LogUtils.e(TAG, "<onCreate> PowerManager == " + pm);
            if (pm != null) {
                mWakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK, TAG);
            }
        }
        PDebug.start("onCreate -- initParams");
        //Initial the record parameters shared preferences when firstly use.
        RecordParamsSetting.initRecordParamsSharedPreference(SoundRecorder.this);
        PDebug.end("onCreate -- initParams");
        mResumeNeedRefresh = true;
        LogUtils.i(TAG, "<onCreate> end");
        PDebug.end("onCreate()");
    }

    @Override
    protected void onResume() {
        PDebug.start("onResume()");
        super.onResume();
        LogUtils.i(TAG, "<onResume> start mRunFromLauncher = " + mRunFromLauncher + ", Activity = "
                + this.toString());
        mOnSaveInstanceStateHasRun = false;

        if (mService == null) {
            disableButton();
            // start service
            LogUtils.i(TAG, "<onResume> start service");
            PDebug.start("onResume()-startService");
            if (null == startService(new Intent(SoundRecorder.this, SoundRecorderService.class))) {
                LogUtils.e(TAG, "<onResume> fail to start service");
                finish();
                return;
            }
            PDebug.end("onResume()-startService");

            // bind service
            LogUtils.i(TAG, "<onResume> bind service");
            PDebug.start("onResume()-bindService");
            if (!bindService(new Intent(SoundRecorder.this, SoundRecorderService.class),
                    mServiceConnection, BIND_AUTO_CREATE)) {
                LogUtils.e(TAG, "<onResume> fail to bind service");
                finish();
                return;
            }
            PDebug.end("onResume()-bindService");

            // M: reset ui to initial state, or else the UI may be abnormal before service not bind
            PDebug.start("onResume()-resetUi");
            if (mResumeNeedRefresh) {
                resetUi();
            }
            PDebug.end("onResume()-resetUi");
        } else {
            // M: when switch SoundRecorder and RecordingFileList quickly, it's
            // possible that onStop was not been called,
            // but onResume is called, in this case, mService has not been
            // unbind, so mService != null
            // but we still should do some initial operation, such as play
            // recording file which select from RecordingFileList
            initWhenHaveService();
        }
        if (mDoRecordWhenResume) {
            onClickRecordButton();
            mDoRecordWhenResume = false;
        }
        PDebug.end("onResume()");
        LogUtils.i(TAG, "<onResume> end");
    }

    @Override
    public void onBackPressed() {
        LogUtils.i(TAG, "<onBackPressed> start, Activity = " + this.toString());
        mBackPressed = true;
        if (!mRunFromLauncher) {
            if (mService != null) {
                mService.doStop(mButtonUpdater);
                if (mService.isCurrentFileWaitToSave()) {
                    LogUtils.i(TAG, "<onBackPressed> mService.saveRecord()");
                    mService.doSaveRecord(mButtonUpdater);
                } else {
                    // M: if not call saveRecord, we finish activity by ourself
                    finish();
                }
            } else {
                // M: if not call saveRecord, we finish activity by ourself
                finish();
            }
        } else {
            // M: if run from launcher, we do not run other operation when back
            // key pressed
            if (null != mService) {
                mService.storeRecordParamsSettings();
            }
            super.onBackPressed();
        }
        LogUtils.i(TAG, "<onBackPressed> end");
    }

    @Override
    protected void onPause() {
        LogUtils.i(TAG, "<onPause> start, Activity =" + this.toString());
        if (!mBackPressed && mService != null && !mRunFromLauncher) {
            if (mService.getCurrentState() == SoundRecorderService.STATE_RECORDING) {
                mService.doStop(mButtonUpdater);
            }
            if (mService.isCurrentFileWaitToSave()) {
                LogUtils.i(TAG, "<onPause> mService.saveRecord()");
                mService.saveRecordAsync();
            }
        }
        mBackPressed = false;
        LogUtils.i(TAG, "<onPause> end");
        super.onPause();
    }

    @Override
    protected void onStop() {
        LogUtils.i(TAG, "<onStop> start, Activity = " + this.toString());
        if (mRunFromLauncher && mService != null) {

            boolean stopService = (mService.getCurrentState() == SoundRecorderService.STATE_IDLE)
                    && !mService.isCurrentFileWaitToSave();

            // M: if another instance of soundrecorder has been resume,
            // the listener of service has changed to another instance, so we
            // cannot call setAllListenerSelf
            boolean isListener = mService.isListener(SoundRecorder.this);
            LogUtils.i(TAG, "<onStop> isListener = " + isListener);
            if (isListener) {
                // set listener of service as default,
                // so when error occurs, service can show error info in toast
                mService.setAllListenerSelf();
            }

            LogUtils.i(TAG, "<onStop> unbind service");
            unbindService(mServiceConnection);

            mIsStopService = stopService && isListener;
            mService = null;
        }
        hideStorageHint();
        LogUtils.i(TAG, "<onStop> end");
        super.onStop();
    }

    @Override
    protected void onDestroy() {
        LogUtils.i(TAG, "<onDestroy> start, Activity = " + this.toString());
        if (mRunFromLauncher && mIsStopService) {
            LogUtils.i(TAG, "<onDestroy> stop service");
            stopService(new Intent(SoundRecorder.this, SoundRecorderService.class));
        }
        if (!mRunFromLauncher) {
            releaseWakeLock();
        }
        LogUtils.i(TAG, "<onDestroy> end");
        super.onDestroy();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        LogUtils.i(TAG, "<onSaveInstanceState> start");
        mOnSaveInstanceStateHasRun = true;
        if (null != mService) {
            mService.storeRecordParamsSettings();
        }
        LogUtils.i(TAG, "<onSaveInstanceState> end");
        super.onSaveInstanceState(outState);
    }

    @Override
    protected void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        LogUtils.i(TAG, "<onRestoreInstanceState> start");
        restoreDialogFragment();
        restoreRecordParamsSettings();
        LogUtils.i(TAG, "<onRestoreInstanceState> end");
    }

    @Override
    /**
     * M: add option menu to select record mode and format
     */
    public boolean onCreateOptionsMenu(Menu menu) {
        LogUtils.i(TAG, "<onCreateOptionsMenu> begin");
        super.onCreateOptionsMenu(menu);
        MenuInflater menuInflater = new MenuInflater(getApplicationContext());
        menuInflater.inflate(R.menu.option_menu, menu);
        mMenu = menu;
        LogUtils.i(TAG, "<onCreateOptionsMenu> end");
        return true;
    }

    @Override
    /**
     * M: add option menu to select record mode and format
     */
    public boolean onPrepareOptionsMenu(Menu menu) {
        LogUtils.i(TAG, "<onPrepareOptionsMenu> start");
        super.onPrepareOptionsMenu(menu);
        mMenu = menu;
        updateOptionsMenu(true);
        LogUtils.i(TAG, "<onPrepareOptionsMenu> end");
        return true;
    }

    @Override
    /**
     * M: add option menu to select record mode and format,
     * when select one item, show corresponding dialog
     */
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (R.id.record_format == id) {
            showDialogFragment(DIALOG_SELECT_FORMAT, null);
        } else if (R.id.record_mode == id) {
            showDialogFragment(DIALOG_SELECT_MODE, null);
        } else if (R.id.record_effect == id) {
            mSelectEffectArrayTemp = mSelectEffectArray.clone();
            showDialogFragment(DIALOG_SELECT_EFFECT, null);
        }
        return true;
    }

    @Override
    /**
     * M: add process of view focus, and save configuration change state when
     * SoundRecorder is started by other application
     */
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        LogUtils.i(TAG, "<onConfigurationChanged> start");
        View viewFocus = this.getCurrentFocus();
        int viewId = -1;
        if (null != viewFocus) {
            viewId = viewFocus.getId();
        }
        setContentView(R.layout.main);

        if (null != mService) {
            initResourceRefs();
            if (!mService.isCurrentFileWaitToSave()) {
                mExitButtons.setVisibility(View.INVISIBLE);
                mFirstLine.setVisibility(View.INVISIBLE);
            }
            mHandler.sendEmptyMessage(mService.getCurrentState());
            mService.storeRecordParamsSettings();
        } else {
            resetUi();
            disableButton();
        }

        if (viewId >= 0) {
            View view = findViewById(viewId);
            if (null != view) {
                view.setFocusable(true);
                view.requestFocus();
            }
        }
        LogUtils.i(TAG, "<onConfigurationChanged> end");
    }

    @Override
    /**
     * M: do record or play operation after press record
     * or press one record item in RecordingFileList
     */
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        LogUtils.i(TAG, "<onActivityResult> start");
        if (RESULT_OK != resultCode) {
            LogUtils.i(TAG, "<onActivityResult> resultCode != RESULT_OK, return");
            return;
        }
        if ((null != mService) && (null != mFileListButton)) {
            mFileListButton.setEnabled(true);
        }
        Intent intent = data;
        Bundle bundle = intent.getExtras();
        if (null == bundle) {
            LogUtils.i(TAG, "<onActivityResult> bundle == null, return");
            return;
        }
        mDoWhat = bundle.getString(DOWHAT);
        if (null != mDoWhat) {
            if (mDoWhat.equals(PLAY)) {
                if ((null != intent.getExtras()) && (null != intent.getExtras().getString(PATH))) {
                    mDoWhatFilePath = intent.getExtras().getString(PATH);
                    mFileFromList = true;
                }
            }
        }
        // later, after mService connected, play/record
        LogUtils.i(TAG, "<onActivityResult> end");
    }

    private void initWhenHaveService() {
        LogUtils.i(TAG, "<initWhenHaveService> start");
        mService.setErrorListener(SoundRecorder.this);
        mService.setEventListener(SoundRecorder.this);
        mService.setStateChangedListener(SoundRecorder.this);
        mService.setShowNotification(mRunFromLauncher);
        //Add for update time view through implements the listener defined by SoundRecorderService.
        mService.setUpdateTimeViewListener(SoundRecorder.this);
        initResourceRefs();
        // M: if run from other application, we will stop recording and auto
        // save the recording file
        // and reset SoundRecorder to innitial state
        if (!mRunFromLauncher) {
              mService.doStop(mButtonUpdater);
            if (mService.isCurrentFileWaitToSave()) {
                // M: set mSetResultAfterSave = false ,and set
                // mSetResultAfterSave = true in onEvent
                mSetResultAfterSave = false;
                LogUtils.i(TAG, "<initWhenHaveService> save record when run from other ap");
                mService.doSaveRecord(mButtonUpdater);
                mSavingRecordFileFromMms = true;
            } else {
                mService.reset();
            }
//            return true;
//        } else {
//            return super.onKeyDown(keyCode, event);
        }
        restoreRecordParamsSettings();
        mHandler.sendEmptyMessage(mService.getCurrentState());
        // do action that need to bo in onActivityResult
        if (RECORD.equals(mDoWhat)) {
            onClickRecordButton();
        } else if (PLAY.equals(mDoWhat)) {
            mService.playFile(mDoWhatFilePath);
        }
        mDoWhat = null;
        mDoWhatFilePath = null;
        LogUtils.i(TAG, "<initWhenHaveService> end");
    }

    /**
     * Whenever the UI is re-created (due f.ex. to orientation change) we have
     * to reinitialize references to the views.
     */
    private void initResourceRefs() {
        LogUtils.i(TAG, "<initResourceRefs> start");
        initResourceRefsWhenNoService();
        /**
         * M: set related property according to if SoundRecorder is started by
         * launcher @{
         */
        if (mRunFromLauncher) {
            mPlayButton.setOnClickListener(this);
            mFileListButton.setOnClickListener(this);
            mPauseRecordingButton.setOnClickListener(this);
        } else {
            mPlayButton.setVisibility(View.GONE);
            mFileListButton.setVisibility(View.GONE);
            mPauseRecordingButton.setVisibility(View.GONE);
        }
        /** @} */
        mRecordButton.setOnClickListener(this);
        mStopButton.setOnClickListener(this);
        mAcceptButton.setOnClickListener(this);
        mDiscardButton.setOnClickListener(this);
        setTitle(getResources().getString(R.string.app_name));
        mVUMeter.setRecorder(mService.getRecorder());
        if (mResumeNeedRefresh) {
            setTimerTextView(true);
            mResumeNeedRefresh = false;
        }
        LogUtils.i(TAG, "<initResourceRefs> end");
    }

    /**
     * init state when onCreate.
     *
     * @return whether success when init
     */
    private boolean initFromIntent() {
        LogUtils.i(TAG, "<initFromIntent> start");
        Intent intent = getIntent();
        if (null != intent) {
            LogUtils.i(TAG, "<initFromIntent> Intent is " + intent.toString());
            /**
             * M: check if SoundRecorder is start by launcher or start by
             * SoundRecorderService
             */
            String action = intent.getAction();
            if (action == null) {
                LogUtils.i(TAG, "<initFromIntent> the action is null");
                mRunFromLauncher = true;
            } else {
                mRunFromLauncher = (action.equals(INTENT_ACTION_MAIN))
                        || (action.equals("com.android.soundrecorder.SoundRecorder"));
            }
            String typeString = intent.getType();
            if (null != typeString) {
                if (RecordParamsSetting.isAvailableRequestType(typeString)) {
                    mRequestedType = typeString;
                } else {
                    LogUtils.i(TAG, "<initFromIntent> return false");
                    return false;
                }
            }
            mMaxFileSize = intent.getLongExtra(EXTRA_MAX_BYTES, MAX_FILE_SIZE_NULL);
            /** M: if mMaxFileSize != -1, set mHasFileSizeLimitation as true. */
            mHasFileSizeLimitation = (mMaxFileSize != MAX_FILE_SIZE_NULL);

            /* Pass extras to recording time calculation Extension */
            IRecordingTimeCalculationExt ext =
                    ExtensionHelper.getRecordingTimeCalculationExt(getApplicationContext());
            ext.setExtras(intent.getExtras());
        }
        LogUtils.i(TAG, "<initFromIntent> end");
        return true;
    }

    /**
     * show dialog use DialogFragment.
     *
     * @param id
     *            the flag of dialog
     * @param args
     *            the parameters of create dialog
     *
     *            M: use DialogFragment to show dialog, for showDialog() is
     *            deprecated in current version
     */
    private void showDialogFragment(int id, Bundle args) {
        LogUtils.i(TAG, "<showDialogFragment> start");
        DialogFragment newFragment = null;
        FragmentManager fragmentManager = getFragmentManager();
        switch (id) {
        case DIALOG_SELECT_FORMAT:
            removeOldFragmentByTag(DIALOG_TAG_SELECT_FORMAT);
            newFragment = SelectDialogFragment.newInstance(RecordParamsSetting
                    .getFormatStringArray(SoundRecorder.this), null,
                    R.string.select_voice_quality, true, mSelectedFormat, null);
            ((SelectDialogFragment) newFragment).setOnClickListener(mSelectFormatListener);
            newFragment.show(fragmentManager, DIALOG_TAG_SELECT_FORMAT);
            LogUtils.i(TAG, "<showDialogFragment> show select format dialog");
            break;
        case DIALOG_SELECT_MODE:
            removeOldFragmentByTag(DIALOG_TAG_SELECT_MODE);
            newFragment = SelectDialogFragment.newInstance(RecordParamsSetting
                    .getModeStringIDArray(), null, R.string.select_recording_mode, true,
                    mSelectedMode, null);
            ((SelectDialogFragment) newFragment).setOnClickListener(mSelectModeListener);
            newFragment.show(fragmentManager, DIALOG_TAG_SELECT_MODE);
            LogUtils.i(TAG, "<showDialogFragment> show select mode dialog");
            break;
        case DIALOG_SELECT_EFFECT:
            removeOldFragmentByTag(DIALOG_TAG_SELECT_EFFECT);
            newFragment = SelectDialogFragment.newInstance(RecordParamsSetting
                    .getEffectStringIDArray(), null, R.string.select_recording_effect, false, 0,
                    mSelectEffectArray);
            ((SelectDialogFragment) newFragment).setOnClickListener(mSelectEffectOkListener);
            ((SelectDialogFragment) newFragment)
                    .setOnMultiChoiceListener(mSelectEffectMultiChoiceClickListener);
            newFragment.show(fragmentManager, DIALOG_TAG_SELECT_EFFECT);
            break;
        default:
            break;
        }
        fragmentManager.executePendingTransactions();
        LogUtils.i(TAG, "<showDialogFragment> end");
    }

    /**
     * remove old DialogFragment.
     *
     * @param tag the tag of DialogFragment to be removed
     */
    private void removeOldFragmentByTag(String tag) {
        LogUtils.i(TAG, "<removeOldFragmentByTag> start");
        FragmentManager fragmentManager = getFragmentManager();
        DialogFragment oldFragment = (DialogFragment) fragmentManager.findFragmentByTag(tag);
        LogUtils.i(TAG, "<removeOldFragmentByTag> oldFragment = " + oldFragment);
        if (null != oldFragment) {
            oldFragment.dismissAllowingStateLoss();
            LogUtils.i(TAG, "<removeOldFragmentByTag> remove oldFragment success");
        }
        LogUtils.i(TAG, "<removeOldFragmentByTag> end");
    }

    /**
     * set current record mode when user select an item in SelectDialogFragment.
     *
     * @param which mode to be set
     */
    public void setSelectedMode(int which) {
        mSelectedMode = RecordParamsSetting.getSelectMode(which);
        if (null != mService) {
            mService.setSelectedMode(mSelectedMode);
        }
        LogUtils.i(TAG, "<setSelectedMode> mSelectedMode = " + mSelectedMode);
    }

    /**
     * set current record format when user select an item in
     * SelectDialogFragment.
     *
     * @param which which format has selected
     */
    public void setSelectedFormat(int which) {
        mSelectedFormat = RecordParamsSetting.getSelectFormat(which);
        if (null != mService) {
            mService.setSelectedFormat(mSelectedFormat);
        }
        LogUtils.i(TAG, "<setSelectedFormat> mSelectedFormat = " + mSelectedFormat);
    }

    /**
     * M: reset the UI to initial state when mService is not available, only
     * used in onResume.
     */
    private void resetUi() {
        initResourceRefsWhenNoService();
        disableButton();
        setTitle(getResources().getString(R.string.app_name));
        mButtonParent.setWeightSum(TWO_BUTTON_WEIGHT_SUM);
        mRecordButton.setEnabled(true);
        mRecordButton.setFocusable(true);
        mRecordButton.setSoundEffectsEnabled(true);
        mPlayButton.setVisibility(View.GONE);
        mStopButton.setEnabled(false);
        mStopButton.setFocusable(false);
        mStopButton.setVisibility(View.GONE);
        if (mRunFromLauncher) {
            mFileListButton.setVisibility(View.VISIBLE);
            mFileListButton.setEnabled(true);
            mFileListButton.setFocusable(true);
        } else {
            mFileListButton.setVisibility(View.GONE);
            mStopButton.setVisibility(View.VISIBLE);
            mStopButton.setEnabled(false);
            mStopButton.setFocusable(false);
        }
        mPauseRecordingButton.setVisibility(View.GONE);
        mPauseRecordingButton.setSoundEffectsEnabled(false);
        mRemainingTimeTextView.setVisibility(View.INVISIBLE);
        mRecordingStateImageView.setVisibility(View.INVISIBLE);
        mStateTextView.setVisibility(View.INVISIBLE);
        mPlayingStateImageView.setVisibility(View.GONE);
        mRecordingFileNameTextView.setVisibility(View.INVISIBLE);
        mFirstLine.setVisibility(View.GONE);
        mExitButtons.setVisibility(View.INVISIBLE);
        mVUMeter.setVisibility(View.VISIBLE);
        mVUMeter.mCurrentAngle = 0;
        mStateProgressBar.setVisibility(View.INVISIBLE);
        /**M: Avoid fresh the timer view if current is recording state.@{**/
        if (mService == null ||
                (mService.getCurrentState() != SoundRecorderService.STATE_RECORDING)) {
            setTimerTextView(true);
        }
        /**@}**/
    }

    private void initResourceRefsWhenNoService() {
        mRecordButton = (ImageButton) findViewById(R.id.recordButton);
        mStopButton = (ImageButton) findViewById(R.id.stopButton);
        mPlayButton = (ImageButton) findViewById(R.id.playButton);
        mFileListButton = (ImageButton) findViewById(R.id.fileListButton);
        mPauseRecordingButton = (ImageButton) findViewById(R.id.pauseRecordingButton);
        mButtonParent = (LinearLayout) findViewById(R.id.buttonParent);
        mRecordingStateImageView = (ImageView) findViewById(R.id.stateLED);
        mRemainingTimeTextView = (TextView) findViewById(R.id.stateMessage1);
        mStateTextView = (TextView) findViewById(R.id.stateMessage2);
        mStateProgressBar = (ProgressBar) findViewById(R.id.stateProgressBar);
        mTimerTextView = (TextView) findViewById(R.id.timerView);
        mPlayingStateImageView = (ImageView) findViewById(R.id.currState);
        mRecordingFileNameTextView = (TextView) findViewById(R.id.recordingFileName);
        mExitButtons = (LinearLayout) findViewById(R.id.exitButtons);
        mVUMeter = (VUMeter) findViewById(R.id.uvMeter);
        mTimerFormat = getResources().getString(R.string.timer_format);
        mAcceptButton = (Button) findViewById(R.id.acceptButton);
        mDiscardButton = (Button) findViewById(R.id.discardButton);
        mFirstLine = (ImageView) findViewById(R.id.firstLine);
        mSecondLine = (ImageView) findViewById(R.id.secondLine);
        mFrameLayout = (FrameLayout) findViewById(R.id.frameLayout);
    }

    /**
     * M: Update UI on idle state.
     */
    private void updateUiOnIdleState() {
        LogUtils.i(TAG, "<updateUiOnIdleState> start");
        boolean isCurrentFileWaitToSave = mService.isCurrentFileWaitToSave();
        int time = 0;
        if (mFileFromList) {
            mFileFromList = false;
        } else {
            time = (int) mService.getCurrentFileDurationInSecond();
        }
        if (mIsRecordStarting) {
            time = 0;
        }
        String timerString = String.format(mTimerFormat, time / TIME_BASE, time % TIME_BASE);
        setTimerViewTextSize(time);
        LogUtils.i(TAG, "<updateUiOnIdleState> time = " + timerString);
        mTimerTextView.setText(timerString);

        String currentFilePath = mService.getCurrentFilePath();
        mRecordingStateImageView.setVisibility(View.INVISIBLE);
        mStateTextView.setVisibility(View.INVISIBLE);
        mStateProgressBar.setProgress(0);
        mStateProgressBar.setVisibility(View.INVISIBLE);
        mButtonParent.setWeightSum(TWO_BUTTON_WEIGHT_SUM);
        mRecordButton.setVisibility(View.VISIBLE);
        mRecordButton.setEnabled(true);
        mRecordButton.setFocusable(true);
        mRecordButton.setSoundEffectsEnabled(true);
        mRecordButton.requestFocus();
        mPauseRecordingButton.setVisibility(View.GONE);
        mPauseRecordingButton.setSoundEffectsEnabled(false);
        mStopButton.setEnabled(false);
        mStopButton.setVisibility(View.GONE);
        if (null == currentFilePath) {
            mFrameLayout.setBackgroundColor(getResources().getColor(R.color.frameLayoutGrayColor));
            mPlayButton.setVisibility(View.GONE);
            if (mRunFromLauncher) {
                mFileListButton.setVisibility(View.VISIBLE);
                mFileListButton.setEnabled(true);
                mFileListButton.setFocusable(true);
            } else {
                mFileListButton.setVisibility(View.GONE);
                mStopButton.setVisibility(View.VISIBLE);
                mStopButton.setEnabled(false);
                mStopButton.setFocusable(false);
            }
            mRemainingTimeTextView.setVisibility(View.INVISIBLE);
            mPlayingStateImageView.setVisibility(View.GONE);
            mRecordingFileNameTextView.setVisibility(View.INVISIBLE);
            mExitButtons.setVisibility(View.INVISIBLE);
            mFirstLine.setVisibility(View.INVISIBLE);
            mSecondLine.setVisibility(View.VISIBLE);
            mVUMeter.setVisibility(View.VISIBLE);
            mVUMeter.mCurrentAngle = 0;
        } else {
            if (mRunFromLauncher) {
                mButtonParent.setWeightSum(THREE_BUTTON_WEIGHT_SUM);
                mPlayButton.setVisibility(View.VISIBLE);
                mPlayButton.setEnabled(true);
                mPlayButton.setFocusable(true);
                mPlayButton.setImageResource(R.drawable.play);
            }
            mRemainingTimeTextView.setVisibility(View.INVISIBLE);
            mPlayingStateImageView.setImageResource(R.drawable.stop);
            mPlayingStateImageView.setVisibility(View.VISIBLE);
            mRecordingFileNameTextView.setVisibility(View.VISIBLE);
            mExitButtons.setVisibility(View.INVISIBLE);
            mVUMeter.setVisibility(View.INVISIBLE);
            mFirstLine.setVisibility(View.INVISIBLE);
            mFileListButton.setVisibility(View.VISIBLE);
            if (isCurrentFileWaitToSave && mService.getCurrentState() ==
                    SoundRecorderService.STATE_IDLE) {
                mFrameLayout.setBackgroundColor(getResources().getColor(R.color.blackColor));
                mSecondLine.setVisibility(View.INVISIBLE);
                mFirstLine.setVisibility(View.VISIBLE);
                mExitButtons.setVisibility(View.VISIBLE);
                mAcceptButton.setEnabled(true);
                mDiscardButton.setEnabled(true);
                mStopButton.setVisibility(View.VISIBLE);
                mStopButton.setEnabled(false);
                mStopButton.setFocusable(false);
                mFileListButton.setVisibility(View.GONE);
            } else {
                mFrameLayout.setBackgroundColor(getResources().getColor(
                        R.color.frameLayoutGrayColor));
                mSecondLine.setVisibility(View.VISIBLE);
                mFileListButton.setEnabled(true);
            }
        }
        mIsButtonDisabled = false;
        LogUtils.i(TAG, "<updateUiOnIdleState> end");
    }

    /**
     * M: Update UI on success state.
     */
    private void updateUiOnSaveSuccessState() {
        LogUtils.i(TAG, "<updateUiOnSaveSuccessState> start");
        updateUiOnIdleState();
        LogUtils.i(TAG, "<updateUiOnSaveSuccessState> end");
    }

    /**
     * M: Update UI on pause playing state.
     */
    private void updateUiOnPausePlayingState() {
        LogUtils.i(TAG, "<updateUiOnPausePlayingState> start");
        mButtonParent.setWeightSum(THREE_BUTTON_WEIGHT_SUM);
        mRecordButton.setEnabled(true);
        mRecordButton.setFocusable(true);
        mRecordButton.setSoundEffectsEnabled(true);
        mPlayButton.setVisibility(View.VISIBLE);
        mPlayButton.setEnabled(true);
        mPlayButton.setFocusable(true);
        mPlayButton.setImageResource(R.drawable.play);
        mStopButton.setVisibility(View.VISIBLE);
        mFileListButton.setVisibility(View.GONE);
        mPauseRecordingButton.setVisibility(View.GONE);
        mPauseRecordingButton.setSoundEffectsEnabled(false);
        mStopButton.setEnabled(true);
        mStopButton.setFocusable(true);
        mRemainingTimeTextView.setVisibility(View.INVISIBLE);
        mRecordingStateImageView.setVisibility(View.INVISIBLE);
        mStateTextView.setVisibility(View.INVISIBLE);
        mPlayingStateImageView.setImageResource(R.drawable.pause);
        mPlayingStateImageView.setVisibility(View.VISIBLE);
        mRecordingFileNameTextView.setVisibility(View.VISIBLE);
        boolean isCurrentFileWaitToSave = false;
        if (null != mService) {
            isCurrentFileWaitToSave = mService.isCurrentFileWaitToSave();
        }
        if (!isCurrentFileWaitToSave) {
            mExitButtons.setVisibility(View.INVISIBLE);
            mFirstLine.setVisibility(View.INVISIBLE);
            mSecondLine.setVisibility(View.VISIBLE);
        } else {
            mFrameLayout.setBackgroundColor(getResources().getColor(R.color.blackColor));
            mExitButtons.setVisibility(View.VISIBLE);
            mAcceptButton.setEnabled(true);
            mDiscardButton.setEnabled(true);
            mFirstLine.setVisibility(View.VISIBLE);
            mSecondLine.setVisibility(View.INVISIBLE);
        }
        setTimerTextView(false);
        mStateProgressBar.setVisibility(View.VISIBLE);
        mStateProgressBar.setProgress((int) (100
                * mService.getCurrentProgressInMillSecond()
                / mService.getCurrentFileDurationInMillSecond()));
        mVUMeter.setVisibility(View.INVISIBLE);
        mIsButtonDisabled = false;
        LogUtils.i(TAG, "<updateUiOnPausePlayingState> end");
    }

    /**
     * M: Update UI on recording state.
     */
    private void updateUiOnRecordingState() {
        LogUtils.i(TAG, "<updateUiOnRecordingState> start");
        mFrameLayout.setBackgroundColor(getResources().getColor(R.color.frameLayoutGrayColor));
        Resources res = getResources();
        mButtonParent.setWeightSum(TWO_BUTTON_WEIGHT_SUM);
        mRecordButton.setVisibility(View.GONE);
        mPlayButton.setVisibility(View.GONE);
        mStopButton.setVisibility(View.VISIBLE);
        mStopButton.setEnabled(true);
        mStopButton.setFocusable(true);
        if (mRunFromLauncher) {
            mFileListButton.setVisibility(View.GONE);
            mPauseRecordingButton.setVisibility(View.VISIBLE);
            mPauseRecordingButton.setEnabled(true);
            mPauseRecordingButton.setFocusable(true);
            mPauseRecordingButton.setSoundEffectsEnabled(false);
        } else {
            mRecordButton.setVisibility(View.VISIBLE);
            mRecordButton.setEnabled(false);
            mRecordButton.setFocusable(false);
            mRecordButton.setSoundEffectsEnabled(true);
            mFileListButton.setVisibility(View.GONE);
            mPauseRecordingButton.setVisibility(View.GONE);
        }
        mRecordingStateImageView.setVisibility(View.VISIBLE);
        mRecordingStateImageView.setImageResource(R.drawable.recording_led);
        mStateTextView.setVisibility(View.VISIBLE);
        mStateTextView.setText(res.getString(R.string.recording));
        mRemainingTimeTextView.setText(EMPTY);
        mRemainingTimeTextView.setVisibility(View.VISIBLE);
        mPlayingStateImageView.setVisibility(View.GONE);
        mRecordingFileNameTextView.setVisibility(View.VISIBLE);
        mExitButtons.setVisibility(View.INVISIBLE);
        mFirstLine.setVisibility(View.INVISIBLE);
        mSecondLine.setVisibility(View.VISIBLE);
        mVUMeter.setVisibility(View.VISIBLE);
        mStateProgressBar.setVisibility(View.INVISIBLE);
        int remainingTime = (int) mService.getRemainingTime();
        boolean isUpdateRemainingTimerView =
                mHasFileSizeLimitation ? (remainingTime < MMS_FILE_LIMIT)
                : (remainingTime < TIME_NINE_MIN);
        if ((remainingTime > 0) && isUpdateRemainingTimerView) {
            updateRemainingTimerView(remainingTime);
        }
        setTimerTextView(false);
        mIsButtonDisabled = false;
        LogUtils.i(TAG, "<updateUiOnRecordingState> end");
    }

    /**
     * M: Update UI on pause Recording state.
     */
    private void updateUiOnPauseRecordingState() {
        LogUtils.i(TAG, "<updateUiOnPauseRecordingState> start");
        Resources res = getResources();
        mButtonParent.setWeightSum(TWO_BUTTON_WEIGHT_SUM);
        mRecordButton.setVisibility(View.VISIBLE);
        mRecordButton.setEnabled(true);
        mRecordButton.setFocusable(true);
        mRecordButton.setSoundEffectsEnabled(false);
        mPlayButton.setVisibility(View.GONE);
        mFileListButton.setVisibility(View.GONE);
        mPauseRecordingButton.setVisibility(View.GONE);
        mStopButton.setVisibility(View.VISIBLE);
        mStopButton.setEnabled(true);
        mStopButton.setFocusable(true);
        mRecordingStateImageView.setVisibility(View.VISIBLE);
        mRecordingStateImageView.setImageResource(R.drawable.idle_led);
        mStateTextView.setVisibility(View.VISIBLE);
        mStateTextView.setText(res.getString(R.string.recording_paused));
        mRemainingTimeTextView.setVisibility(View.INVISIBLE);
        mPlayingStateImageView.setVisibility(View.GONE);
        mRecordingFileNameTextView.setVisibility(View.VISIBLE);
        mExitButtons.setVisibility(View.INVISIBLE);
        mFirstLine.setVisibility(View.INVISIBLE);
        mSecondLine.setVisibility(View.VISIBLE);
        mVUMeter.setVisibility(View.VISIBLE);
        mVUMeter.mCurrentAngle = 0;
        mStateProgressBar.setVisibility(View.INVISIBLE);
        setTimerTextView(false);
        mIsButtonDisabled = false;
        LogUtils.i(TAG, "<updateUiOnPauseRecordingState> end");
    }

    /**
     * M: Update UI on playing state.
     */
    private void updateUiOnPlayingState() {
        LogUtils.i(TAG, "<updateUiOnPlayingState> start");
        mButtonParent.setWeightSum(THREE_BUTTON_WEIGHT_SUM);
        mRecordButton.setEnabled(true);
        mRecordButton.setFocusable(true);
        mRecordButton.setSoundEffectsEnabled(true);
        mPlayButton.setVisibility(View.VISIBLE);
        mPlayButton.setEnabled(true);
        mPlayButton.setFocusable(true);
        mPlayButton.setImageResource(R.drawable.pause);
        mFileListButton.setVisibility(View.GONE);
        mPauseRecordingButton.setVisibility(View.GONE);
        mPauseRecordingButton.setSoundEffectsEnabled(false);
        mStopButton.setVisibility(View.VISIBLE);
        mStopButton.setEnabled(true);
        mStopButton.setFocusable(true);
        mRecordingStateImageView.setVisibility(View.INVISIBLE);
        mStateTextView.setVisibility(View.INVISIBLE);
        mRemainingTimeTextView.setVisibility(View.INVISIBLE);
        mPlayingStateImageView.setVisibility(View.VISIBLE);
        mPlayingStateImageView.setImageResource(R.drawable.play);
        mRecordingFileNameTextView.setVisibility(View.VISIBLE);
        if (!mService.isCurrentFileWaitToSave()) {
            mExitButtons.setVisibility(View.INVISIBLE);
            mFirstLine.setVisibility(View.INVISIBLE);
            mSecondLine.setVisibility(View.VISIBLE);
        } else {
            mFrameLayout.setBackgroundColor(getResources().getColor(R.color.blackColor));
            mExitButtons.setVisibility(View.VISIBLE);
            mAcceptButton.setEnabled(true);
            mDiscardButton.setEnabled(true);
            mFirstLine.setVisibility(View.VISIBLE);
            mSecondLine.setVisibility(View.INVISIBLE);
        }
        mVUMeter.setVisibility(View.INVISIBLE);
        mStateProgressBar.setVisibility(View.VISIBLE);
        setTimerTextView(true);
        mIsButtonDisabled = false;
        LogUtils.i(TAG, "<updateUiOnPlayingState> end");
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
            int[] grantResults) {
        boolean mShowPermission = true;
        if (permissions.length == 0) {
            return;
        }
        if (requestCode == PERMISSION_RECORD_AUDIO) {
            boolean granted = true;
            for (int counter = 0; counter < permissions.length; counter++) {
                granted = granted && (grantResults[counter] == PackageManager.PERMISSION_GRANTED);
                LogUtils.i(TAG, "<onRequestPermissionsResult> " + grantResults[counter]);
                if (grantResults[counter] != PackageManager.PERMISSION_GRANTED) {
                    mShowPermission = mShowPermission &&
                                      shouldShowRequestPermissionRationale(permissions[counter]);
                }
                LogUtils.i(TAG, "<onRequestPermissionsResult1>" + granted + mShowPermission);
            }
            if (granted == true) {
                if (mRunFromLauncher) {
                    onClickRecordButton();
                } else {
                    mDoRecordWhenResume = true;
                }
            } else {
                if (mShowPermission == false) {
                    //SoundRecorderUtils.getToast(SoundRecorder.this,
                    //        com.mediatek.internal.R.string.denied_required_permission);
                    return;
                }
            }
        } else if (requestCode == PERMISSION_READ_STORAGE_LIST) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                onClickFileListButton();
            } else {
                if (!shouldShowRequestPermissionRationale(
                        Manifest.permission.READ_EXTERNAL_STORAGE)) {
                        //SoundRecorderUtils.getToast(
                        //    SoundRecorder.this,
                        //    com.mediatek.internal.R.string.denied_required_permission);
                        return;
                }
            }
        } else if (requestCode == PERMISSION_WRITE_STORAGE_LIST) {
            if (grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                onClickAcceptButton();
            } else {
                if (!shouldShowRequestPermissionRationale(
                        Manifest.permission.WRITE_EXTERNAL_STORAGE)) {
                        //SoundRecorderUtils.getToast(
                            //SoundRecorder.this,
                            //com.mediatek.internal.R.string.denied_required_permission);
                        return;
                }
            }
        }
    }

    /**
     * process after click record button.
     */
    void onClickRecordButton() {
        if (OptionsUtil.isRunningInEmulator()) {
            LogUtils.d(TAG, "for special action for emulator load, do nothing...");
            return;
        }
        int recordAudioPermission = checkSelfPermission(Manifest.permission.RECORD_AUDIO);
        int readExtStorage = checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
        int writeExtStorage = checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        List<String> mPermissionStrings = new ArrayList<String>();
        boolean mRequest = false;
        LogUtils.d(TAG, "<onClickRecordButton1> " + recordAudioPermission + readExtStorage);
        if (writeExtStorage != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            mRequest = true;
        }
        if (readExtStorage != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.READ_EXTERNAL_STORAGE);
            mRequest = true;
        }
        if (recordAudioPermission != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.RECORD_AUDIO);
            mRequest = true;
        }
        if (mRequest == true) {
            String[] mPermissionList = new String[mPermissionStrings.size()];
            mPermissionList = mPermissionStrings.toArray(mPermissionList);
            requestPermissions(mPermissionList, PERMISSION_RECORD_AUDIO);
            return;
        }
        if (null != mService) {
            mIsRecordStarting = true;
            mService.startRecordingAsync(
                    RecordParamsSetting.getRecordParams(mRequestedType, mSelectedFormat,
                    mSelectedMode, mSelectEffectArray, SoundRecorder.this),
                    (int) mMaxFileSize, mButtonUpdater);
        }
        long mEndRecordingTime = System.currentTimeMillis();
        Log.i(TAG, "[Performance test][SoundRecorder] recording end [" + mEndRecordingTime + "]");
    }

    /**
     * process after click play button.
     */
    void onClickPlayButton() {
        if (!isFinishing() && null != mService) {
            mService.doPlayRecord(mButtonUpdater);
        }
    }

    /**
     * process after click stop button.
     */
    void onClickStopButton() {
        if (null == mService) {
            long mEndStopTime = System.currentTimeMillis();
            Log.i(TAG, "[Performance test][SoundRecorder] recording stop end [" + mEndStopTime
                    + "]");
            return;
        }
        mService.doStop(mButtonUpdater);
        long mEndStopTime = System.currentTimeMillis();
        Log.i(TAG, "[Performance test][SoundRecorder] recording stop end [" + mEndStopTime + "]");
    }

    /**
     * process after click accept button.
     */
    void onClickAcceptButton() {
        if (null == mService) {
            return;
        }
        if (checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            LogUtils.i(TAG, "<onClickAcceptButton> Need storage permission");
            requestPermissions(new String[]{Manifest.permission.WRITE_EXTERNAL_STORAGE},
                    PERMISSION_WRITE_STORAGE_LIST);
            return;
        }
        mService.doSaveRecord(mButtonUpdater);
    }

    /**
     * process after click discard button.
     */
    void onClickDiscardButton() {
        if (mService != null) {
            mService.doDiscardRecord(mButtonUpdater);
        }
        mVUMeter.mCurrentAngle = 0;
        mVUMeter.invalidate();
    }

    /**
     * process after click file list button.
     */
    void onClickFileListButton() {
        if (checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE)
                != PackageManager.PERMISSION_GRANTED) {
            LogUtils.i(TAG, "<onClickRecordButton> Need storage permission");
            requestPermissions(new String[]{Manifest.permission.READ_EXTERNAL_STORAGE},
                    PERMISSION_READ_STORAGE_LIST);
            return;
        }
        disableButton();
        if (null != mService) {
            LogUtils.i(TAG, "<onClickFileListButton> mService.resetAsync()");
            mService.resetAsync();
        }
        Intent mIntent = new Intent();
        mIntent.setClass(this, RecordingFileList.class);
        startActivityForResult(mIntent, REQURST_FILE_LIST);
    }

    /**
     * process after click pause recording button.
     */
    void onClickPauseRecordingButton() {
        if (null != mService) {
            mService.doPause(mButtonUpdater);
        }
    }

    /**
     * Update the big MM:SS timer. If we are in play back, also update the
     * progress bar.
     *
     * @param time time to be updated
     */
    @Override
    public void updateTimerView(int time) {
        LogUtils.i(TAG, "<updateTimerView> start time = " + time);
        int state = mService.getCurrentState();
        // update progress bar
        if (SoundRecorderService.STATE_PLAYING == state) {
            long fileDuration = mService.getCurrentFileDurationInMillSecond();
            LogUtils.i(TAG, "<updateTimerView> fileDuration = " + fileDuration);
            if (fileDuration > ONE_SECOND) {
                long progress = mService.getCurrentProgressInMillSecond();
                LogUtils.i(TAG, "<updateTimerView> progress = " + (fileDuration - progress));
                if (fileDuration - progress < SoundRecorderService.WAIT_TIME) {
                    mStateProgressBar.setProgress(DONE);
                } else {
                    mStateProgressBar.setProgress((int) (100 * progress / fileDuration));
                }
            } else {
                mStateProgressBar.setProgress(DONE);
            }
        }
        // update timer
        setTimerTextView(time);
        // update remaining time
        if (SoundRecorderService.STATE_RECORDING == mService.getCurrentState()) {
            int remainingTime = (int) mService.getRemainingTime();
            if (mService.isStorageLower()) {
                showStorageHint(getString(R.string.storage_low));
            } else {
                hideStorageHint();
            }
            boolean isUpdateRemainingTimerView =
                    mHasFileSizeLimitation ? (remainingTime < MMS_FILE_LIMIT)
                    : (remainingTime < TIME_NINE_MIN);
            if ((remainingTime > 0) && isUpdateRemainingTimerView) {
                updateRemainingTimerView(remainingTime);
            }
        }
        LogUtils.i(TAG, "<updateTimerView> end");
    }

    /**
     * set text size bigger if time >= 100.
     *
     * @param time time in minutes should be shown
     */
    private void setTimerViewTextSize(int time) {
        /** M: set text size bigger if >= 100 @{ */
        final int textSizeChangeBoundary = 100;
        float scale = getResources().getConfiguration().fontScale;
        float textSize;
    if ((time / TIME_BASE) >= textSizeChangeBoundary) {
            textSize = getResources().getDimension(R.dimen.timerView_TextSize_Small);
        } else {
            textSize = getResources().getDimension(R.dimen.timerView_TextSize_Big);
        }
    if (scale > 1) {
        textSize = textSize / scale;
    }
            mTimerTextView.setTextSize(textSize);
        /** @} */
    }

    private void restoreDialogFragment() {
        FragmentManager fragmentManager = getFragmentManager();
        Fragment fragment = fragmentManager.findFragmentByTag(DIALOG_TAG_SELECT_FORMAT);
        if (null != fragment) {
            ((SelectDialogFragment) fragment).setOnClickListener(mSelectFormatListener);
        }

        fragment = fragmentManager.findFragmentByTag(DIALOG_TAG_SELECT_MODE);
        if (null != fragment) {
            ((SelectDialogFragment) fragment).setOnClickListener(mSelectModeListener);
        }

        fragment = fragmentManager.findFragmentByTag(DIALOG_TAG_SELECT_EFFECT);
        if (null != fragment) {
            ((SelectDialogFragment) fragment)
            .setOnMultiChoiceListener(mSelectEffectMultiChoiceClickListener);
            ((SelectDialogFragment) fragment).setOnClickListener(mSelectEffectOkListener);
        }
    }

    private void restoreRecordParamsSettings() {
        LogUtils.i(TAG, "<restoreRecordParamsSettings> ");
        if (mSelectedFormat != -1) {
            if (null != mService) {
                mService.setSelectedFormat(mSelectedFormat);
                mService.setSelectedMode(mSelectedMode);
                mService.setSelectEffectArray(mSelectEffectArray);
                mService.setSelectEffectArrayTmp(mSelectEffectArrayTemp);
            }
            LogUtils.i(TAG, "<restoreRecordParamsSettings> selectedFormat return ");
            return;
        }
        if (null == mPrefs) {
            mPrefs = getSharedPreferences(SOUND_RECORDER_DATA, 0);
        }
        int defaultRecordingLevel =
                RecordParamsSetting.getDefaultRecordingLevel(RecordParamsSetting.FORMAT_HIGH);
        mSelectedFormat = mPrefs.getInt(SoundRecorderService.SELECTED_RECORDING_FORMAT,
                  defaultRecordingLevel);
        if (mSelectedFormat < 0 || mSelectedFormat > RecordParamsSetting.getQualityLevelNumber()) {
              mSelectedFormat = defaultRecordingLevel;
        }
        mSelectedMode = mPrefs
                .getInt(SoundRecorderService.SELECTED_RECORDING_MODE,
                        RecordParamsSetting.MODE_NORMAL);
        if (mSelectedMode < 0) {
            mSelectedMode = RecordParamsSetting.MODE_NORMAL;
        }
        mSelectEffectArray[RecordParamsSetting.EFFECT_AEC] = mPrefs.getBoolean(
                SoundRecorderService.SELECTED_RECORDING_EFFECT_AEC, false);
        mSelectEffectArray[RecordParamsSetting.EFFECT_AGC] = mPrefs.getBoolean(
                SoundRecorderService.SELECTED_RECORDING_EFFECT_AGC, false);
        mSelectEffectArray[RecordParamsSetting.EFFECT_NS] = mPrefs.getBoolean(
                SoundRecorderService.SELECTED_RECORDING_EFFECT_NS, false);
        mSelectEffectArrayTemp[RecordParamsSetting.EFFECT_AEC] = mPrefs.getBoolean(
                SoundRecorderService.SELECTED_RECORDING_EFFECT_AEC_TMP, false);
        mSelectEffectArrayTemp[RecordParamsSetting.EFFECT_AGC] = mPrefs.getBoolean(
                SoundRecorderService.SELECTED_RECORDING_EFFECT_AGC_TMP, false);
        mSelectEffectArrayTemp[RecordParamsSetting.EFFECT_NS] = mPrefs.getBoolean(
                SoundRecorderService.SELECTED_RECORDING_EFFECT_NS_TMP, false);
        if (null != mService) {
            mService.setSelectedFormat(mSelectedFormat);
            mService.setSelectedMode(mSelectedMode);
            mService.setSelectEffectArray(mSelectEffectArray);
            mService.setSelectEffectArrayTmp(mSelectEffectArrayTemp);
        }
        LogUtils.i(TAG, "mSelectedFormat is:"
        + mSelectedFormat + "; mSelectedMode is:" + mSelectedMode);
    }

    private void updateRemainingTimerView(int second) {
        String timeString = "";
        if (second < 0) {
            mRemainingTimeTextView.setText(NULL_STRING);
        } else if (second < TIME_BASE) {
            timeString = String.format(getString(R.string.sec_available), second);
        } else {
            if (second % TIME_BASE == 0) {
                timeString = String.format(getString(R.string.min_available), second / TIME_BASE);
            } else {
                timeString = String.format(getString(R.string.time_available), second / TIME_BASE,
                        second % TIME_BASE);
            }
        }
        LogUtils.i(TAG, "<updateRemainingTimerView> mRemainingTimeTextView.setText: "
                + timeString);
        mRemainingTimeTextView.setText(timeString);
        mRemainingTimeTextView.setVisibility(View.VISIBLE);
    }

    /**
     * M: release wake lock.
     */
    private void releaseWakeLock() {
        // if mWakeLock is not release, release it
        if ((null != mWakeLock) && mWakeLock.isHeld()) {
            mWakeLock.release();
            LogUtils.i(TAG, "<releaseWakeLock>");
        }
    }

    /**
     * M: acquire wake lock.
     */
    private void acquireWakeLock() {
        if ((null != mWakeLock) && !mWakeLock.isHeld()) {
            mWakeLock.acquire();
            LogUtils.i(TAG, "<acquireWakeLock>");
        }
    }

    /**
     * M: add for long string in option menu.
     */
    protected void addOptionsMenuInflaterFactory() {
        final LayoutInflater infl = getLayoutInflater();
        infl.setFactory(new Factory() {
            public View onCreateView(final String name, final Context context,
                    final AttributeSet attrs) {
                // not create list menu item view
                if (!name.equalsIgnoreCase(LIST_MENUITEM_VIEW_NAME)) {
                    return null;
                }

                // get class and constructor
                if (null == sListMenuItemViewClass) {
                    try {
                        sListMenuItemViewClass = getClassLoader().loadClass(name);
                    } catch (ClassNotFoundException e) {
                        return null;
                    }
                }
                if (null == sListMenuItemViewClass) {
                    return null;
                }
                if (null == sListMenuItemViewConstructor) {
                    try {
                        sListMenuItemViewConstructor = sListMenuItemViewClass
                                .getConstructor(INFLATER_CONSTRUCTOR_SIGNATURE);
                    } catch (SecurityException e) {
                        return null;
                    } catch (NoSuchMethodException e) {
                        return null;
                    }
                }
                if (null == sListMenuItemViewConstructor) {
                    return null;
                }

                // create list menu item view
                View view = null;
                try {
                    Object[] args = new Object[] { context, attrs };
                    view = (View) (sListMenuItemViewConstructor.newInstance(args));
                } catch (IllegalArgumentException e) {
                    return null;
                } catch (InstantiationException e) {
                    return null;
                } catch (IllegalAccessException e) {
                    return null;
                } catch (InvocationTargetException e) {
                    return null;
                }
                if (null == view) {
                    return null;
                }

                final View viewTemp = view;
                new Handler().post(new Runnable() {
                    public void run() {
                        TextView textView = (TextView) viewTemp.findViewById(android.R.id.title);
                        LogUtils.e(TAG, "<create ListMenuItemView> setSingleLine");
                        // multi line if item string too long
                        textView.setSingleLine(false);
                    }
                });
                LogUtils.e(TAG, "<create ListMenuItemView> return view = " + view.toString());
                return view;
            }
        });
    }

    private void updateOptionsMenu(boolean isShow) {
        LogUtils.i(TAG, "<updateOptionsMenu>");
        if (null == mMenu) {
            LogUtils.i(TAG, "<updateOptionsMenu> mMenu == null, return");
            return;
        }

        boolean allowSelectFormatAndMode = mRunFromLauncher && isShow;
        if (null != mService) {
            allowSelectFormatAndMode = mRunFromLauncher && isShow
                    && (SoundRecorderService.STATE_IDLE == mService.getCurrentState());
        }

        if (RecordParamsSetting.canSelectFormat()) {
            MenuItem item1 = mMenu.getItem(OPTIONMENU_SELECT_FORMAT);
            if (null != item1) {
                item1.setVisible(allowSelectFormatAndMode);
            }
        }
    }

    private void showStorageHint(String message) {
        if (null == mStorageHint) {
            mStorageHint = OnScreenHint.makeText(this, message);
        } else {
            mStorageHint.setText(message);
        }
        mStorageHint.show();
    }

    private void hideStorageHint() {
        if (null != mStorageHint) {
            mStorageHint.cancel();
            mStorageHint = null;
        }
    }

    /**
     * M: for reduce repeat code.
     *
     * @param initial true to set the time as 0, otherwise set as current progress
     */
    public void setTimerTextView(boolean initial) {
        int time = 0;
        if (!initial) {
            if (null != mService) {
                time = (int) mService.getCurrentProgressInSecond();
            }
        }
        setTimerTextView(time);
    }

    private void setTimerTextView(int time) {
        LogUtils.i(TAG, "<setTimerTextView> start with time = " + time);
        String timerString = String.format(mTimerFormat, time / TIME_BASE, time % TIME_BASE);
        setTimerViewTextSize(time);
        mTimerTextView.setText(timerString);
        LogUtils.i(TAG, "<setTimerTextView> end");
    }

    /**
     * Shows/hides the appropriate child views for the new state. M: use
     * different function in different state to update UI
     */
    private Handler mHandler = new Handler() {

        @Override
        public void handleMessage(Message msg) {
            LogUtils.i(TAG, "<handleMessage> start with msg.what-" + msg.what);
            if (null == mService || SoundRecorder.this.isFinishing()) {
                return;
            }
            if (mSavingRecordFileFromMms && msg.what != SoundRecorderService.STATE_SAVE_SUCESS) {
                return;
            } else if (mSavingRecordFileFromMms && msg.what ==
                    SoundRecorderService.STATE_SAVE_SUCESS) {
                mSavingRecordFileFromMms = false;
            }
            updateOptionsMenu(true);
            String filePath = mService.getCurrentFilePath();
            LogUtils.i(TAG, "<handleMessage> mService.getCurrentFilePath() = " + filePath);
            mFileName = NULL_STRING;
            if (null != filePath) {
                mFileName = filePath.substring(filePath.lastIndexOf(File.separator) + 1, filePath
                        .length());
                mFileName = (mFileName.endsWith(Recorder.SAMPLE_SUFFIX)) ? mFileName.substring(0,
                        mFileName.lastIndexOf(Recorder.SAMPLE_SUFFIX)) : mFileName;
            }
            LogUtils.i(TAG, "<updateUi> mRecordingFileNameTextView.setText : " + mFileName);
            mRecordingFileNameTextView.setText(mFileName);
            mAcceptButton.setText(R.string.accept);
            if (mRunFromLauncher) {
                mAcceptButton.setText(R.string.save_record);
            }
            hideStorageHint();
            switch (msg.what) {
                case SoundRecorderService.STATE_IDLE:
                    updateUiOnIdleState();
                    break;
                case SoundRecorderService.STATE_PAUSE_PLAYING:
                    updateUiOnPausePlayingState();
                    break;
                case SoundRecorderService.STATE_RECORDING:
                    mIsRecordStarting = false;
                    updateUiOnRecordingState();
                    break;
                case SoundRecorderService.STATE_PAUSE_RECORDING:
                    updateUiOnPauseRecordingState();
                    break;
                case SoundRecorderService.STATE_PLAYING:
                    updateUiOnPlayingState();
                    break;
                case SoundRecorderService.STATE_ERROR:
                    Bundle bundle = msg.getData();
                    int errorCode = bundle.getInt(ERROR_CODE);
                    ErrorHandle.showErrorInfo(SoundRecorder.this, errorCode);
                    if (mService != null && mIsButtonDisabled) {
                        updateUiAccordingState(mService.getCurrentState());
                    }
                    break;
                case SoundRecorderService.STATE_SAVE_SUCESS:
                    updateUiOnSaveSuccessState();
                    SoundRecorderUtils.getToast(SoundRecorder.this,
                            R.string.tell_save_record_success);
                    break;
                default:
                    break;
            }
            mVUMeter.invalidate();
            LogUtils.i(TAG, "<handleMessage> end");
        }
    };

    private void updateUiAccordingState(int code) {
        LogUtils.d(TAG, "updateUiAccordingState start : " + code);
        switch (code) {
        case SoundRecorderService.STATE_IDLE:
            updateUiOnIdleState();
            break;
        case SoundRecorderService.STATE_PAUSE_PLAYING:
            updateUiOnPausePlayingState();
            break;
        case SoundRecorderService.STATE_RECORDING:
            updateUiOnRecordingState();
            break;
        case SoundRecorderService.STATE_PAUSE_RECORDING:
            updateUiOnPauseRecordingState();
            break;
        case SoundRecorderService.STATE_PLAYING:
            updateUiOnPlayingState();
            break;
        default:
            break;
        }
        LogUtils.d(TAG, "updateUiAccordingState end : " + code);
    }

    /**
     * disable all buttons.
     */
    private void disableButton() {
        LogUtils.i(TAG, "<disableButton>");
        closeOptionsMenu();
        updateOptionsMenu(false);
        if (mRecordButton == null) {
            return;
        }
        mRecordButton.setEnabled(false);
        mPauseRecordingButton.setEnabled(false);
        mStopButton.setEnabled(false);
        mPlayButton.setEnabled(false);
        mFileListButton.setEnabled(false);
        mDiscardButton.setEnabled(false);
        mAcceptButton.setEnabled(false);
        mIsButtonDisabled = true;
    }
}
