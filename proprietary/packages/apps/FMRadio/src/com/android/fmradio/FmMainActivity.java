/*
 * Copyright (C) 2014 The Android Open Source Project
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

package com.android.fmradio;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.List;

import android.Manifest;
import android.app.Activity;
import android.app.ActivityManager;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.content.res.Resources;
import android.database.Cursor;
import android.media.AudioDeviceInfo;
import android.media.AudioManager;
import android.media.AudioSystem;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewConfiguration;
import android.view.animation.Animation;
import android.view.animation.Animation.AnimationListener;
import android.view.animation.AnimationUtils;
import android.widget.ImageButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.Toolbar;

import com.android.fmradio.FmStation.Station;
import com.android.fmradio.dialogs.FmFavoriteEditDialog;
import com.android.fmradio.views.FmScroller;
import com.android.fmradio.views.FmScroller.EventListener;
import com.android.fmradio.views.FmSnackBar;

//import static android.Manifest.permission.READ_MEDIA_AUDIO;
/**
 * This class interact with user, provide FM basic function.
 */
public class FmMainActivity extends Activity implements FmFavoriteEditDialog.EditFavoriteListener {
    // Logging
    private static final String TAG = "FmMainActivity";

    // Request code
    private static final int REQUEST_CODE_FAVORITE = 1;

    public static final int REQUEST_CODE_RECORDING = 2;

    private static final int PERMISSION_REQUEST_POWER_ON = 100;

    private static final int PERMISSION_REQUEST_CODE_RECORDING = 101;

    private static final int PERMISSION_REQUEST_CODE_SAVED_RECORDING = 102;

    // Extra for result of request REQUEST_CODE_RECORDING
    public static final String EXTRA_RESULT_STRING = "result_string";

    // FM
    private static final String FM = "FM";

    // UI views
    private TextView mTextStationName = null;

    private TextView mTextStationValue = null;

    // RDS text view
    private TextView mTextRds = null;

    private TextView mActionBarTitle = null;

    private TextView mNoEarPhoneTxt = null;

    private ImageButton mButtonDecrease = null;

    private ImageButton mButtonPrevStation = null;

    private ImageButton mButtonNextStation = null;

    private ImageButton mButtonIncrease = null;

    private ImageButton mButtonAddToFavorite = null;

    private ImageButton mButtonPlay = null;

    private ImageView mNoHeadsetImgView = null;

    private View mNoHeadsetImgViewWrap = null;

    private float mMiddleShadowSize;

    private LinearLayout mMainLayout = null;

    private RelativeLayout mNoHeadsetLayout = null;

    private LinearLayout mNoEarphoneTextLayout = null;

    private LinearLayout mBtnPlayInnerContainer = null;

    private LinearLayout mBtnPlayContainer = null;

    // Menu items
    private MenuItem mMenuItemStationlList = null;

    private MenuItem mMenuItemHeadset = null;

    private MenuItem mMenuItemStartRecord = null;

    private MenuItem mMenuItemRecordList = null;

    private MenuItem mMenuItemRdStereo = null;

    private MenuItem mMenuItemRdMono = null;

    // State variables
    private boolean mIsServiceStarted = false;

    private boolean mIsServiceBinded = false;

    private boolean mIsServiceConnected = false;

    private boolean mIsOnStopCalled = false;

    private boolean mIsTune = false;

    private boolean mIsDisablePowerMenu = false;

    private boolean mIsActivityForeground = true;

    private int mCurrentStation = FmUtils.DEFAULT_STATION;

    // Instance variables
    private FmService mService = null;

    private Context mContext = null;

    private Toast mToast = null;

    private FragmentManager mFragmentManager = null;

    private AudioManager mAudioManager = null;

    private FmScroller mScroller;

    private FmScroller.EventListener mEventListener;

     // Service listener
    private FmListener mFmRadioListener = new FmListener() {
        @Override
        public void onCallBack(Bundle bundle) {
            int flag = bundle.getInt(FmListener.CALLBACK_FLAG);
            if (flag == FmListener.MSGID_FM_EXIT) {
                mHandler.removeCallbacksAndMessages(null);
            }

            // remove tag message first, avoid too many same messages in queue.
            Message msg = mHandler.obtainMessage(flag);
            msg.setData(bundle);
            mHandler.removeMessages(flag);
            mHandler.sendMessage(msg);
        }
    };

    // Button click listeners on UI
    private final View.OnClickListener mButtonClickListener = new View.OnClickListener() {
        @Override
        public void onClick(View v) {
            Log.d(TAG, "onClick: button_id = " + v.getId());
            switch (v.getId()) {

                case R.id.button_add_to_favorite:
                    updateFavoriteStation();
                    break;

                case R.id.button_decrease:
                    tuneStation(FmUtils.computeDecreaseStation(mCurrentStation));
                    break;

                case R.id.button_increase:
                    tuneStation(FmUtils.computeIncreaseStation(mCurrentStation));
                    break;

                case R.id.button_prevstation:
                    seekStation(mCurrentStation, false); // false: previous station
                    break;

                case R.id.button_nextstation:
                    seekStation(mCurrentStation, true); // true: previous station
                    break;

                case R.id.play_button:
                    if (mService != null) {
                        if (mService.getPowerStatus() == FmService.POWER_UP) {
                            powerDownFm();
                        } else {
                            powerUpFm();
                        }
                    }
                    break;
                default:
                    Log.d(TAG, "mButtonClickListener.onClick, invalid view id");
                    break;
            }
        }
    };

    /**
     * Main thread handler to update UI
     */
    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG,
                    "mHandler.handleMessage, what = " + msg.what + ",hashcode:"
                            + mHandler.hashCode());
            Bundle bundle;
            switch (msg.what) {

                case FmListener.MSGID_POWERUP_FINISHED:
                    bundle = msg.getData();
                    boolean isPowerup = (mService.getPowerStatus() == FmService.POWER_UP);
                    int station = bundle.getInt(FmListener.KEY_TUNE_TO_STATION);
                    mCurrentStation = station;
                    refreshStationUI(station);
                    if (isPowerup) {
                        refreshImageButton(true);
                        refreshPopupMenuItem(true);
                        refreshActionMenuItem(true);
                    } else {
                        showToast(getString(R.string.not_available));
                    }
                    // if not powerup success, refresh power to enable.
                    refreshPlayButton(true);
                    break;

                case FmListener.MSGID_SWITCH_ANTENNA:
                    bundle = msg.getData();
                    boolean hasAntenna = bundle.getBoolean(FmListener.KEY_IS_SWITCH_ANTENNA);
                    // if receive headset plug out, need set headset mode on ui
                    if (hasAntenna && isAntennaAvailable()) {
                        if (mIsActivityForeground) {
                            cancelNoHeadsetAnimation();
                            cancelMainAnimation();
                            playMainAnimation();
                        } else {
                            changeToMainLayout();
                        }
                    } else {
                        if (null != mMenuItemHeadset) {
                        mMenuItemHeadset.setIcon(R.drawable.btn_fm_headset_selector);
                            mMenuItemHeadset.setTitle(R.string.optmenu_earphone);
                        }
                        if (mIsActivityForeground) {
                            cancelMainAnimation();
                            /// M: modify to avoid  @{
                            if (!isAntennaAvailable()) {
                                playNoHeadsetAnimation();
                            }
                            /// @}
                        } else {
                            cancelNoHeadsetAnimation();
                            changeToNoHeadsetLayout();
                        }
                    }
                    break;

                case FmListener.MSGID_POWERDOWN_FINISHED:
                    Log.d(TAG, "powerdown finished");
                    bundle = msg.getData();
                    refreshImageButton(false);
                    refreshActionMenuItem(false);
                    refreshPopupMenuItem(false);
                    refreshPlayButton(true);
                    if (mService != null) {
                        mService.removeNotification();
                    }
                    break;

                case FmListener.MSGID_TUNE_FINISHED:
                    bundle = msg.getData();
                    boolean isTune = bundle.getBoolean(FmListener.KEY_IS_TUNE);
                    boolean isPowerUp = (mService.getPowerStatus() == FmService.POWER_UP);

                    // tune finished, should make power enable
                    mIsDisablePowerMenu = false;
                    float frequency = bundle.getFloat(FmListener.KEY_TUNE_TO_STATION);
                    mCurrentStation = FmUtils.computeStation(frequency);
                    // After tune to station finished, refresh favorite button and
                    // other button status.
                    refreshStationUI(mCurrentStation);
                    // tune fail,should resume button status
                    if (!isTune) {
                        Log.d(TAG, "mHandler.tune: " + isTune);
                        refreshActionMenuItem(isPowerUp);
                        refreshImageButton(isPowerUp);
                        refreshPopupMenuItem(isPowerUp);
                        refreshPlayButton(true);
                        return;
                    }
                    refreshImageButton(true);
                    refreshActionMenuItem(true);
                    refreshPopupMenuItem(true);
                    refreshPlayButton(true);
                    break;

                case FmListener.MSGID_FM_EXIT:
                    finish();
                    break;

                case FmListener.LISTEN_RDSSTATION_CHANGED:
                    bundle = msg.getData();
                    int rdsStation = bundle.getInt(FmListener.KEY_RDS_STATION);
                    refreshStationUI(rdsStation);
                    break;

                case FmListener.LISTEN_PS_CHANGED:
                    String stationName = FmStation.getStationName(mContext, mCurrentStation);
                    mTextStationName.setText(stationName);
                    if (null != mScroller) {
                    mScroller.notifyAdatperChange();
                        mScroller.updateHeaderTextAndButton();
                    }
                    break;

                case FmListener.LISTEN_RT_CHANGED:
                    bundle = msg.getData();
                    String rtString = bundle.getString(FmListener.KEY_RT_INFO);
                    mTextRds.setText(rtString);
                    if (null != mScroller) {
                        mScroller.updateHeaderTextAndButton();
                    }
                    break;

                case FmListener.LISTEN_SPEAKER_MODE_CHANGED:
                    bundle = msg.getData();
                    boolean isSpeakerUsed = bundle.getBoolean(FmListener.KEY_IS_SPEAKER_MODE);
                    if (null != mMenuItemHeadset) {
                        mMenuItemHeadset.setIcon(isSpeakerUsed ? R.drawable.btn_fm_speaker_selector
                                : R.drawable.btn_fm_headset_selector);
                        mMenuItemHeadset.setTitle(isSpeakerUsed ? R.string.optmenu_speaker
                                : R.string.optmenu_earphone);
                    }
                    break;

                case FmListener.LISTEN_RECORDSTATE_CHANGED:
                    if (mService != null) {
                        mService.updatePlayingNotification();
                    }
                    break;

                case FmListener.MSGID_BT_STATE_CHANGED:
                    Log.d(TAG, "BT_State_Changed");
                    updateMenuStatus();
                    break;

                default:
                    break;
            }
        }
    };

    // When call bind service, it will call service connect. register call back
    // listener and initial device
    private final ServiceConnection mServiceConnection = new ServiceConnection() {

        /**
         * called by system when bind service
         *
         * @param className component name
         * @param service service binder
         */
        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.d(TAG, "onServiceConnected");
            mIsServiceConnected = true;
            if (mIsOnStopCalled && FmMainActivity.this.mIsServiceBinded) {
                unbindService(mServiceConnection);
                mIsServiceBinded = false;
            }
            mService = ((FmService.ServiceBinder) service).getService();
            if (null == mService) {
                Log.e(TAG, "onServiceConnected, mService is null");
                finish();
                return;
            }

            mService.registerFmRadioListener(mFmRadioListener);
            mService.setFmMainActivityForeground(mIsActivityForeground);
            if (FmRecorder.STATE_RECORDING != mService.getRecorderState()) {
                mService.removeNotification();
            }
            if (!mService.isServiceInited()) {
                if (FmUtils.isFirstTimePlayFm(mContext) && FmUtils.is50KhzSupportOn()) {
                    mCurrentStation = FmUtils.DEFAULT_STATION_50KHZ;
                    Log.d(TAG, "update default station for 50Hz support");
                }
                mService.initService(mCurrentStation);
                if (mService.isAntennaAvailable() && mNoHeadsetLayout.getVisibility()
                        == View.VISIBLE && mMainLayout.getVisibility() == View.GONE) {
                    // this case happens at: this activity are not registered to FmService,
                    // which onServiceConnected() is callback after headset plug in event
                    if (mIsActivityForeground) {
                        cancelNoHeadsetAnimation();
                        playMainAnimation();
                    } else {
                        changeToMainLayout();
                    }
                }
                Log.d(TAG, "Powering up FM");
                powerUpFm();
            } else {
                if (mService.isDeviceOpen()) {
                    // tune to station during changing language,we need to tune
                    // again when service bind success
                    if (mIsTune) {
                        tuneStation(mCurrentStation);
                        mIsTune = false;
                    }
                    updateCurrentStation();
                    updateMenuStatus();
                } else {
                    // Normal case will not come here
                    // Need to exit FM for this case
                    exitService();
                    finish();
                }
            }
        }

        /**
         * When unbind service will call this method
         *
         * @param className The component name
         */
        @Override
        public void onServiceDisconnected(ComponentName className) {
        Log.d(TAG, "onServiceDisconnected");
        }
    };

    private class NoHeadsetAlpaOutListener implements AnimationListener {

        @Override
        public void onAnimationEnd(Animation animation) {
            if (!isAntennaAvailable()) {
                return;
            }
            changeToMainLayout();
            cancelMainAnimation();
            Animation anim = AnimationUtils.loadAnimation(mContext,
                    R.anim.main_alpha_in);
            mMainLayout.startAnimation(anim);
            anim = AnimationUtils.loadAnimation(mContext, R.anim.floatbtn_alpha_in);

            mBtnPlayContainer.startAnimation(anim);
        }

        @Override
        public void onAnimationRepeat(Animation animation) {
        }

        @Override
        public void onAnimationStart(Animation animation) {
            mNoHeadsetImgViewWrap.setElevation(0);
        }
    }

    private class NoHeadsetAlpaInListener implements AnimationListener {

        @Override
        public void onAnimationEnd(Animation animation) {
//            if (isAntennaAvailable()) {
//                return;
//            }
            changeToNoHeadsetLayout();
            cancelNoHeadsetAnimation();
            Animation anim = AnimationUtils.loadAnimation(mContext,
                    R.anim.noeaphone_alpha_in);
            mNoHeadsetLayout.startAnimation(anim);
        }

        @Override
        public void onAnimationRepeat(Animation animation) {
        }

        @Override
        public void onAnimationStart(Animation animation) {
            mNoHeadsetImgViewWrap.setElevation(mMiddleShadowSize);
        }

    }

    /**
     * Update the favorite UI state
     */
    private void updateFavoriteStation() {
        // Judge the current output and switch between the devices.
        if (FmStation.isFavoriteStation(mContext, mCurrentStation)) {
            FmStation.removeFromFavorite(mContext, mCurrentStation);
            mButtonAddToFavorite.setImageResource(R.drawable.btn_fm_favorite_off_selector);
            // Notify scroller
            if (mScroller != null) {
                mScroller.onRemoveFavorite();
            }
            mTextStationName.setText(FmStation.getStationName(mContext, mCurrentStation));
            if (null != mScroller) {
                mScroller.updateHeaderTextAndButton();
            }
        } else {
            // Add the station to favorite
            if (FmStation.isStationExist(mContext, mCurrentStation)) {
                FmStation.addToFavorite(mContext, mCurrentStation);
            } else {
                ContentValues values = new ContentValues(2);
                values.put(Station.FREQUENCY, mCurrentStation);
                values.put(Station.IS_FAVORITE, true);
                FmStation.insertStationToDb(mContext, values);
            }
            mButtonAddToFavorite.setImageResource(R.drawable.btn_fm_favorite_on_selector);
            // Notify scroller
            mScroller.onAddFavorite();
        }
    }

    /**
     * Called when the activity is first created, initial variables
     *
     * @param savedInstanceState The saved bundle in onSaveInstanceState
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate");
        super.onCreate(savedInstanceState);
        // Bind the activity to FM audio stream.
        setVolumeControlStream(AudioManager.STREAM_MUSIC);
        setContentView(R.layout.main);
        try {
            ViewConfiguration config = ViewConfiguration.get(this);
            Field menuKeyField = ViewConfiguration.class.getDeclaredField("sHasPermanentMenuKey");
            if (menuKeyField != null) {
                menuKeyField.setAccessible(true);
                menuKeyField.setBoolean(config, false);
            }
        } catch (NoSuchFieldException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
        }

        mFragmentManager = getFragmentManager();
        mContext = getApplicationContext();

        initUiComponent();
        registerButtonClickListener();
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);

        mScroller = (FmScroller) findViewById(R.id.multiscroller);
        mScroller.initialize();
        mEventListener = new EventListener() {
            @Override
            public void onRename(int frequency) {
                showRenameDialog(frequency);
            }

            @Override
            public void onRemoveFavorite(int frequency) {
            Log.d(TAG, "onRemoveFavorite, frequency = " + frequency);
                // TODO it's on UI thread, change to sub thread
                if (FmStation.isFavoriteStation(mContext, frequency)) {
                    FmStation.removeFromFavorite(mContext, frequency);
                    if (mCurrentStation == frequency) {
                        mTextStationName.setText(FmStation.getStationName(mContext, frequency));
                        if (null != mScroller) {
                            mScroller.updateHeaderTextAndButton();
                        }
                        mButtonAddToFavorite.setImageResource(R.drawable.
                                btn_fm_favorite_off_selector);
                    }
                    // Notify scroller
                    mScroller.onRemoveFavorite();
                }
            }

            @Override
            public void onPlay(int frequency) {
                Log.d(TAG, "onPlay, frequency = " + frequency);
                if (null == mService) {
                    Log.d(TAG, "onPlay, mService is null");
                    return;
                }
                if (frequency != 0 && (mService.getPowerStatus() == FmService.POWER_UP)) {
                    tuneStation(frequency);
                }
            }
        };
        mScroller.registerListener(mEventListener);
    }

    @Override
    public void editFavorite(int stationFreq, String name) {
        Log.d(TAG, "editFavorite, stationFreq = " + stationFreq);
        FmStation.updateStationToDb(mContext, stationFreq, name);
        if (mCurrentStation == stationFreq) {
            String stationName = FmStation.getStationName(mContext, mCurrentStation);
            mTextStationName.setText(stationName);
            if (mScroller != null) {
                mScroller.updateHeaderTextAndButton();
            }
        }
        if (mScroller != null) {
            mScroller.notifyAdatperChange();
        }
        String title = getString(R.string.toast_station_renamed);
        FmSnackBar.make(FmMainActivity.this, title, null, null,
                FmSnackBar.DEFAULT_DURATION).show();
    }

    /**
     * Display the rename dialog
     *
     * @param frequency The display frequency
     */
    public void showRenameDialog(int frequency) {
        if (mService != null) {
            String name = FmStation.getStationName(mContext, frequency);
            FmFavoriteEditDialog newFragment = FmFavoriteEditDialog.newInstance(name, frequency);
            //newFragment.show(mFragmentManager, "TAG_EDIT_FAVORITE");
            FragmentTransaction transaction = mFragmentManager.beginTransaction();
            transaction.add(newFragment, "TAG_EDIT_FAVORITE");
            transaction.commit();
            mFragmentManager.executePendingTransactions();
        }
    }

    /**
     * Go to station list activity
     */
    private void enterStationList() {
            Log.d(TAG, "enterStationList");
        if (mService != null) {
            // AMS change the design for background start
            // activity. need check app is background in app code
            if (mService.isActivityForeground()) {
                Intent intent = new Intent();
                intent.setClass(FmMainActivity.this, FmFavoriteActivity.class);
                startActivityForResult(intent, REQUEST_CODE_FAVORITE);
            }
        }
    }

    /**
     * Refresh the favorite button with the given station, if the station
     * is favorite station, show favorite icon, else show non-favorite icon.
     *
     * @param station The station frequency
     */
    private void refreshStationUI(int station) {
            Log.d(TAG, "refreshStationUI, station = " + station);
        if (FmUtils.isFirstTimePlayFm(mContext)) {
            Log.d(TAG, "refreshStationUI, set station value null when it is first time ");
            return;
        }
        // TODO it's on UI thread, change to sub thread
        // Change the station frequency displayed.
        mTextStationValue.setText(FmUtils.formatStation(station));
        // Show or hide the favorite icon
        if (FmStation.isFavoriteStation(mContext, station)) {
            mButtonAddToFavorite.setImageResource(R.drawable.btn_fm_favorite_on_selector);
        } else {
            mButtonAddToFavorite.setImageResource(R.drawable.btn_fm_favorite_off_selector);
        }

        String stationName = "";
        String radioText = "";
        ContentResolver resolver = mContext.getContentResolver();
        Cursor cursor = null;
        try {
            cursor = resolver.query(
                    Station.CONTENT_URI,
                    FmStation.COLUMNS,
                    Station.FREQUENCY + "=?",
                    new String[] { String.valueOf(mCurrentStation) },
                    null);
            if (cursor != null && cursor.moveToFirst()) {
                // If the station name is not exist, show program service(PS) instead
                stationName = cursor.getString(cursor.getColumnIndex(Station.STATION_NAME));
                if (TextUtils.isEmpty(stationName)) {
                    stationName = cursor.getString(cursor.getColumnIndex(Station.PROGRAM_SERVICE));
                }
                radioText = cursor.getString(cursor.getColumnIndex(Station.RADIO_TEXT));

            } else {
                Log.d(TAG, "showPlayingNotification, cursor is null");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
        mTextStationName.setText(stationName);
        mTextRds.setText(radioText);
        if (mScroller != null) {
           mScroller.updateHeaderTextAndButton();
        }
    }

    /**
     * Start and bind service, reduction variable values if configuration changed
     */
    @Override
    public void onStart() {
            Log.d(TAG, "onStart");
        super.onStart();
        // check layout onstart
        if (isAntennaAvailable()) {
            changeToMainLayout();
        } else {
            changeToNoHeadsetLayout();
        }

        // Should start FM service first.
        try {
            if (null == startService(new Intent(FmMainActivity.this, FmService.class))) {
                Log.e(TAG, "onStart, cannot start FM service");
                return;
            }
        } catch (IllegalStateException e) {
            Log.d(TAG, "unable to start service due to illegal state");
            return;

        }

        if (!mIsServiceStarted || mService == null) {
        mIsServiceStarted = true;
            mIsServiceConnected = false;
        mIsServiceBinded = bindService(new Intent(FmMainActivity.this, FmService.class),
                mServiceConnection, Context.BIND_AUTO_CREATE);
        }

        if (!mIsServiceBinded && mService == null) {
            Log.e(TAG, "onStart, cannot bind FM service");
            finish();
            return;
        }
    }

    /**
     * Refresh UI, when stop search, dismiss search dialog,
     * pop up recording dialog if FM stopped when recording in
     * background
     */
    @Override
    public void onResume() {
            Log.d(TAG, "onResume");
        super.onResume();
        mIsActivityForeground = true;
        mIsOnStopCalled = false;
        mScroller.onResume();
        if (null == mService) {
            Log.d(TAG, "onResume, mService is null");
            return;
        }
        mService.setFmMainActivityForeground(mIsActivityForeground);
        if (FmRecorder.STATE_RECORDING != mService.getRecorderState()) {
            mService.removeNotification();
        }
        updateMenuStatus();
    }

    /**
     * When activity is paused call this method, indicate activity
     * enter background if press exit, power down FM
     */
    @Override
    public void onPause() {
        Log.d(TAG, "onPause");
        mIsActivityForeground = false;
        if (null != mService) {
            mService.setFmMainActivityForeground(mIsActivityForeground);
        }
        mScroller.onPause();
        super.onPause();
    }

    /**
     * Called when activity enter stopped state,
     * unbind service, if exit pressed, stop service
     */
    @Override
    public void onStop() {
            Log.d(TAG, "onStop");
        if (null != mService) {
            mService.setNotificationClsName(FmMainActivity.class.getName());
            mService.updatePlayingNotification();
        }
        if (mIsServiceBinded && mIsServiceConnected) {
            mIsServiceConnected = false;
            unbindService(mServiceConnection);
            mIsServiceBinded = false;
        }
        mIsOnStopCalled = true;
        super.onStop();
    }

    /**
     * W activity destroy, unregister broadcast receiver and remove handler message
     */
    @Override
    public void onDestroy() {
            Log.d(TAG, "onDestroy");
        // need to call this function because if doesn't do this,after
        // configuration change will have many instance and recording time
        // or playing time will not refresh
        // Remove all the handle message
        mHandler.removeCallbacksAndMessages(null);
        if (mService != null) {
            mService.unregisterFmRadioListener(mFmRadioListener);
        }
        if (mIsServiceBinded) {
            unbindService(mServiceConnection);
            mIsServiceBinded = false;
        }
        mFmRadioListener = null;
        mScroller.closeAdapterCursor();
        mScroller.unregisterListener(mEventListener);
        super.onDestroy();
    }

    /**
     * Create options menu
     *
     * @param menu The option menu
     * @return true or false indicate need to handle other menu item
     */
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
            Log.d(TAG, "onCreateOptionsMenu");
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.fm_action_bar, menu);
        mMenuItemStationlList = menu.findItem(R.id.fm_station_list);
        mMenuItemHeadset = menu.findItem(R.id.fm_headset);
        mMenuItemStartRecord = menu.findItem(R.id.fm_start_record);
        mMenuItemRecordList = menu.findItem(R.id.fm_record_list);
        mMenuItemRdStereo = menu.findItem(R.id.FMR_Stereomono_stereo);
        mMenuItemRdMono = menu.findItem(R.id.FMR_Stereomono_mono);
        if (null != mMenuItemHeadset) {
            mMenuItemHeadset.setTitle(R.string.optmenu_earphone);
        }
        return true;
    }

    /**
     * Prepare options menu
     *
     * @param menu The option menu
     * @return true or false indicate need to handle other menu item
     */
    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
            Log.d(TAG, "onPrepareOptionsMenu, mService = " + mService);
        if (null == mService) {
            return true;
        }
        int powerStatus = mService.getPowerStatus();
        boolean isPowerUp = (powerStatus == FmService.POWER_UP);
        boolean isPowerdown = (powerStatus == FmService.POWER_DOWN);
        boolean isSeeking = mService.isSeeking();
        boolean isSpeakerUsed = mService.isSpeakerUsed();
        // if fm power down by other app, should enable power menu, make it to
        // powerup.
        refreshActionMenuItem(isSeeking ? false : isPowerUp);
        refreshPopupMenuItem(isSeeking ? false : isPowerUp);
        refreshPlayButton(isSeeking ? false
                : (isPowerUp || (isPowerdown && !mIsDisablePowerMenu)));
        mMenuItemHeadset.setIcon(isSpeakerUsed ? R.drawable.btn_fm_speaker_selector
                : R.drawable.btn_fm_headset_selector);
        mMenuItemHeadset.setTitle(isSpeakerUsed ? R.string.optmenu_speaker
                : R.string.optmenu_earphone);
        return true;
    }

    /**
     * Handle event when option item selected
     *
     * @param item The clicked item
     * @return true or false indicate need to handle other menu item or not
     */
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
            Log.d(TAG, "onOptionsItemSelected, item = " + item.getItemId());
        switch (item.getItemId()) {
            case android.R.id.home:
                onBackPressed();
                break;

            case R.id.fm_station_list:
                refreshImageButton(false);
                refreshActionMenuItem(false);
                refreshPopupMenuItem(false);
                refreshPlayButton(false);
                // Show favorite activity.
                enterStationList();
                break;

            case R.id.earphone_menu:
                setSpeakerPhoneOn(false);
                mMenuItemHeadset.setIcon(R.drawable.btn_fm_headset_selector);
                mMenuItemHeadset.setTitle(R.string.optmenu_earphone);
                invalidateOptionsMenu();
                break;

            case R.id.speaker_menu:
                setSpeakerPhoneOn(true);
                mMenuItemHeadset.setIcon(R.drawable.btn_fm_speaker_selector);
                mMenuItemHeadset.setTitle(R.string.optmenu_speaker);
                invalidateOptionsMenu();
                break;

            case R.id.fm_start_record:
                startRecording();
                break;

            case R.id.fm_record_list:
                openSavedRecordings();
                break;

            case R.id.FMR_Stereomono_stereo:
                setStereoMono(false);
                break;

            case R.id.FMR_Stereomono_mono:
                setStereoMono(true);
                break;
            default:
                Log.e(TAG, "onOptionsItemSelected, invalid options menu item.");
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    /**
     * Check whether antenna is available
     *
     * @return true or false indicate antenna available or not
     */
    private boolean isAntennaAvailable() {
        AudioDeviceInfo[] deviceList = mAudioManager.getDevices(AudioManager.GET_DEVICES_OUTPUTS);

        for (AudioDeviceInfo devInfo : deviceList) {
            int type = devInfo.getType();
            if (type == AudioDeviceInfo.TYPE_WIRED_HEADSET
                    || type == AudioDeviceInfo.TYPE_WIRED_HEADPHONES) {
                return true;
            }
        }

        return false;
    }

    /**
     * When on activity result, tune to station which is from station list
     *
     * @param requestCode The request code
     * @param resultCode The result code
     * @param data The intent from station list
     */
    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
            Log.d(TAG, "onActivityResult, resultCode = " + resultCode
                  + "requestCode = " + requestCode);
        if (RESULT_OK == resultCode) {
            if (REQUEST_CODE_RECORDING == requestCode) {
                final Uri playUri = data.getData();
                boolean isSaved = playUri != null;
                String title = data.getStringExtra(EXTRA_RESULT_STRING);
                String action = null;
                FmSnackBar.OnActionTriggerListener listener = null;

                if (isSaved) {
                    action = FmMainActivity.this.getString(R.string.toast_listen);
                    listener = new FmSnackBar.OnActionTriggerListener() {
                        @Override
                        public void onActionTriggered() {
                            Intent playMusicIntent = new Intent(Intent.ACTION_VIEW);
                            try {
                                playMusicIntent.setClassName("com.android.music",
                                        "com.android.music.AudioPreview");
                                playMusicIntent.setDataAndType(playUri, "audio/3gpp");
                                startActivity(playMusicIntent);
                            } catch (ActivityNotFoundException e1) {
                                try {
                                    playMusicIntent = new Intent(Intent.ACTION_VIEW);
                                    playMusicIntent.setDataAndType(playUri, "audio/3gpp");
                                    startActivity(playMusicIntent);
                                } catch (ActivityNotFoundException e2) {
                                    // No activity respond
                                    Log.d(TAG,"onActivityResult, no activity "
                                            + "respond play record file intent");
                                }
                            }
                        }
                    };
                }
                FmSnackBar.make(FmMainActivity.this, title, action, listener,
                        FmSnackBar.DEFAULT_DURATION).show();
            } else if (REQUEST_CODE_FAVORITE == requestCode) {
                int iStation =
                        data.getIntExtra(FmFavoriteActivity.ACTIVITY_RESULT, mCurrentStation);
                // Tune to this station.
                mCurrentStation = iStation;
                if (null == mService) {
                    Log.d(TAG, "onActivityResult, mService is null");
                    mIsTune = true;
                    return;
                }
                // if tune from station list, we should disable power menu,
                // especially for power down state
                mIsDisablePowerMenu = true;
                Log.d(TAG, "onActivityForReult:" + mIsDisablePowerMenu);
                tuneStation(iStation);
            } else {
                Log.e(TAG, "onActivityResult, invalid requestcode.");
                return;
            }
        }

        // TODO it's on UI thread, change to sub thread
        if (FmStation.isFavoriteStation(mContext, mCurrentStation)) {
            mButtonAddToFavorite.setImageResource(R.drawable.btn_fm_favorite_on_selector);
        } else {
            mButtonAddToFavorite.setImageResource(R.drawable.btn_fm_favorite_off_selector);
        }
        mTextStationName.setText(FmStation.getStationName(mContext, mCurrentStation));
        if (mScroller != null) {
            mScroller.updateHeaderTextAndButton();
        }
    }

    /**
     * Power up FM
     */
    private void powerUpFm() {
        Log.d(TAG, "powerUpFm");

        refreshImageButton(false);
        refreshActionMenuItem(false);
        refreshPopupMenuItem(false);
        refreshPlayButton(false);

        if (!isAntennaAvailable()) {
            Log.d(TAG, "powerUpFm: headset not present");
            return;
        }
        int recordAudioPermission = checkSelfPermission(Manifest.permission.RECORD_AUDIO);
        List<String> mPermissionStrings = new ArrayList<String>();
        boolean mRequest = false;

        if (recordAudioPermission != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.RECORD_AUDIO);
            mRequest = true;
        }
        if (mRequest == true) {
            String[] mPermissionList = new String[mPermissionStrings.size()];
            mPermissionList = mPermissionStrings.toArray(mPermissionList);
            requestPermissions(mPermissionList, PERMISSION_REQUEST_POWER_ON);
            return;
        }
        mService.setRecordingPermission(true);
        mService.powerUpAsync(FmUtils.computeFrequency(mCurrentStation));
    }

    /**
     * Power down FM
     */
    private void powerDownFm() {
        Log.d(TAG, "powerDownFm");
        refreshImageButton(false);
        refreshActionMenuItem(false);
        refreshPopupMenuItem(false);
        refreshPlayButton(false);
        mService.powerDownAsync();
    }

    private void setSpeakerPhoneOn(boolean isSpeaker) {
        if (isSpeaker) {
            mService.setSpeakerPhoneOn(true);
        } else {
            mService.setSpeakerPhoneOn(false);
        }
    }

    /**
     * Tune a station
     *
     * @param station The tune station
     */
    private void tuneStation(final int station) {
        Log.d(TAG, "tuneStation, station = " + station);
        if (mService != null) {
            refreshImageButton(false);
            refreshActionMenuItem(false);
            refreshPopupMenuItem(false);
            refreshPlayButton(false);
            mService.tuneStationAsync(FmUtils.computeFrequency(station));
        }
    }

    /**
     * Seek station according current frequency and direction
     *
     * @param station The seek start station
     * @param direction The seek direction
     */
    private void seekStation(final int station, boolean direction) {
        Log.d(TAG, "seekStation, station = " + station + ", direction = " + direction);
        if (mService != null) {
            // If the seek AsyncTask has been executed and not canceled, cancel it
            // before start new.
            refreshImageButton(false);
            refreshActionMenuItem(false);
            refreshPopupMenuItem(false);
            refreshPlayButton(false);
            mService.seekStationAsync(FmUtils.computeFrequency(station), direction);
        }
    }

    private void refreshImageButton(boolean enabled) {
        mButtonDecrease.setEnabled(enabled);
        mButtonPrevStation.setEnabled(enabled);
        mButtonNextStation.setEnabled(enabled);
        mButtonIncrease.setEnabled(enabled);
        mButtonAddToFavorite.setEnabled(enabled);
    }

    // Refresh action menu except power menu
    private void refreshActionMenuItem(boolean enabled) {
        // action menu
        if (null != mMenuItemStationlList) {
            // if power down by other app, should disable station list, over
            // menu
            mMenuItemStationlList.setEnabled(enabled);
           // If BT headset is in use, need to disable speaker/earphone switching menu.
            mMenuItemHeadset.setEnabled(enabled && !((mService.isBluetoothHeadsetInUse() ||
            mService.isA2DPInUse()) && mService.isRender() &&
            AudioSystem.getForceUse(AudioSystem.FOR_MEDIA) != AudioSystem.FORCE_NO_BT_A2DP));
        }
    }

    // Refresh play/stop float button
    private void refreshPlayButton(boolean enabled) {
        // action menu
        boolean isPowerUp = (mService != null && (mService.getPowerStatus() == FmService.POWER_UP));
        mButtonPlay.setEnabled(enabled);
        mButtonPlay.setImageResource((isPowerUp
                ? R.drawable.btn_fm_stop_selector
                : R.drawable.btn_fm_start_selector));
        Resources r = getResources();
        mBtnPlayInnerContainer.setBackground(r.getDrawable(R.drawable.fb_red));
        mScroller.refreshPlayIndicator(mCurrentStation, isPowerUp);
    }

    private void refreshPopupMenuItem(boolean enabled) {
        if (null != mMenuItemStationlList) {
            mMenuItemStartRecord.setEnabled(enabled);
        }
    }

    /**
     * Called when back pressed
     */
    @Override
    public void onBackPressed() {
        Log.d(TAG, "onBackPressed");
        ActivityManager activityManager =
        (ActivityManager) getSystemService(Context.ACTIVITY_SERVICE);
        // check if activity is pinned
        if (!activityManager.isInLockTaskMode()) {
            // exit fm, disable all button
            if ((null != mService) && (mService.getPowerStatus() == FmService.POWER_DOWN)) {
                refreshImageButton(false);
                refreshActionMenuItem(false);
                refreshPopupMenuItem(false);
                refreshPlayButton(false);
                exitService();
            }
        }
        super.onBackPressed();
    }

    private void showToast(CharSequence text) {
        if (null == mToast) {
            mToast = Toast.makeText(mContext, text, Toast.LENGTH_SHORT);
        }
        mToast.setText(text);
        mToast.show();
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
    }

    /**
     * Exit FM service
     */
    private void exitService() {
            Log.d(TAG, "exitService");
        if (mIsServiceBinded) {
            unbindService(mServiceConnection);
            mIsServiceBinded = false;
        }

        if (mIsServiceStarted) {
            stopService(new Intent(FmMainActivity.this, FmService.class));
            mIsServiceStarted = false;
        }
    }

    /**
     * Update current station according service station
     */
    private void updateCurrentStation() {
        // get the frequency from service, set frequency in activity, UI,
        // database
        // same as the frequency in service
        int freq = mService.getFrequency();
        if (FmUtils.isValidStation(freq)) {
            if (mCurrentStation != freq) {
                mCurrentStation = freq;
                FmStation.setCurrentStation(mContext, mCurrentStation);
                refreshStationUI(mCurrentStation);
            }
        }
    }

    /**
     * Update menu status, and animation
     */
    private void updateMenuStatus() {
            Log.d(TAG, "updateMenuStatus");
        int powerStatus = mService.getPowerStatus();
        boolean isPowerUp = (powerStatus == FmService.POWER_UP);
        boolean isDuringPowerup = (powerStatus == FmService.DURING_POWER_UP);
        boolean isSeeking = mService.isSeeking();
        boolean isPowerdown = (powerStatus == FmService.POWER_DOWN);
        boolean isSpeakerUsed = mService.isSpeakerUsed();
        boolean fmStatus = (isSeeking || isDuringPowerup);
        // when seeking, all button should disabled,
        // else should update as origin status
        refreshImageButton(fmStatus ? false : isPowerUp);
        refreshPopupMenuItem(fmStatus ? false : isPowerUp);
        refreshActionMenuItem(fmStatus ? false : isPowerUp);
        // if fm power down by other app, should enable power button
        // to powerup.
        Log.d(TAG, "updateMenuStatus.mIsDisablePowerMenu: " + mIsDisablePowerMenu);
        refreshPlayButton(fmStatus ? false
                : (isPowerUp || (isPowerdown && !mIsDisablePowerMenu)));
        if (null != mMenuItemHeadset) {
            mMenuItemHeadset.setIcon(isSpeakerUsed ? R.drawable.btn_fm_speaker_selector
                    : R.drawable.btn_fm_headset_selector);
            mMenuItemHeadset.setTitle(isSpeakerUsed ? R.string.optmenu_speaker
                    : R.string.optmenu_earphone);
        }

    }

    private void initUiComponent() {
            Log.d(TAG, "initUiComponent");
        mTextRds = (TextView) findViewById(R.id.station_rds);
        mTextStationValue = (TextView) findViewById(R.id.station_value);
        mButtonAddToFavorite = (ImageButton) findViewById(R.id.button_add_to_favorite);
        mTextStationName = (TextView) findViewById(R.id.station_name);
        mButtonDecrease = (ImageButton) findViewById(R.id.button_decrease);
        mButtonIncrease = (ImageButton) findViewById(R.id.button_increase);
        mButtonPrevStation = (ImageButton) findViewById(R.id.button_prevstation);
        mButtonNextStation = (ImageButton) findViewById(R.id.button_nextstation);

        // put favorite button here since it might be used very early in
        // changing recording mode
        mCurrentStation = FmStation.getCurrentStation(mContext);
        refreshStationUI(mCurrentStation);

        // l new
        mMainLayout = (LinearLayout) findViewById(R.id.main_view);
        mNoHeadsetLayout = (RelativeLayout) findViewById(R.id.no_headset);
        mNoEarphoneTextLayout = (LinearLayout) findViewById(R.id.no_bottom);
        mBtnPlayContainer = (LinearLayout) findViewById(R.id.play_button_container);
        mBtnPlayInnerContainer = (LinearLayout) findViewById(R.id.play_button_inner_container);
        mButtonPlay = (ImageButton) findViewById(R.id.play_button);
        mNoEarPhoneTxt = (TextView) findViewById(R.id.no_eaphone_text);
        mNoHeadsetImgView = (ImageView) findViewById(R.id.no_headset_img);
        mNoHeadsetImgViewWrap = findViewById(R.id.no_middle);
        mMiddleShadowSize = getResources().getDimension(R.dimen.fm_middle_shadow);
        // main ui layout params
        final Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setActionBar(toolbar);
        getActionBar().setTitle("");
    }

    private void registerButtonClickListener() {
        mButtonAddToFavorite.setOnClickListener(mButtonClickListener);
        mButtonDecrease.setOnClickListener(mButtonClickListener);
        mButtonIncrease.setOnClickListener(mButtonClickListener);
        mButtonPrevStation.setOnClickListener(mButtonClickListener);
        mButtonNextStation.setOnClickListener(mButtonClickListener);
        mButtonPlay.setOnClickListener(mButtonClickListener);
    }

    /**
     * play main animation
     */
    private void playMainAnimation() {
        if (null == mService) {
            Log.e(TAG, "playMainAnimation, mService is null");
            return;
        }
        //if (mMainLayout.isShown()) {
        //    Log.w(TAG, "playMainAnimation, main layout has already shown");
        //    return;
        //}
        Animation animation = AnimationUtils.loadAnimation(mContext,
                R.anim.noeaphone_alpha_out);
        mNoEarPhoneTxt.startAnimation(animation);
        mNoHeadsetImgView.startAnimation(animation);

        animation = AnimationUtils.loadAnimation(mContext,
                R.anim.noeaphone_translate_out);
        animation.setAnimationListener(new NoHeadsetAlpaOutListener());
        mNoEarphoneTextLayout.startAnimation(animation);
    }

    /**
     * clear main layout animation
     */
    private void cancelMainAnimation() {
        mNoEarPhoneTxt.clearAnimation();
        mNoHeadsetImgView.clearAnimation();
        mNoEarphoneTextLayout.clearAnimation();
    }

    /**
     * play change to no headset layout animation
     */
    private void playNoHeadsetAnimation() {
        if (null == mService) {
            Log.e(TAG, "playNoHeadsetAnimation, mService is null");
            return;
        }
        if (mNoHeadsetLayout.isShown()) {
            Log.w(TAG,"playNoHeadsetAnimation, no headset layout has already shown");
            return;
        }
        Animation animation = AnimationUtils.loadAnimation(mContext, R.anim.main_alpha_out);
        mMainLayout.startAnimation(animation);
        animation.setAnimationListener(new NoHeadsetAlpaInListener());
        mBtnPlayContainer.startAnimation(animation);
    }

    /**
     * clear no headset layout animation
     */
    private void cancelNoHeadsetAnimation() {
        mMainLayout.clearAnimation();
        mBtnPlayContainer.clearAnimation();
    }

    /**
     * change to main layout
     */
    private void changeToMainLayout() {
        mNoEarphoneTextLayout.setVisibility(View.GONE);
        mNoHeadsetImgView.setVisibility(View.GONE);
        mNoHeadsetImgViewWrap.setVisibility(View.GONE);
        mNoHeadsetLayout.setVisibility(View.GONE);
        // change to main layout
        mMainLayout.setVisibility(View.VISIBLE);
        mBtnPlayContainer.setVisibility(View.VISIBLE);
    }

    /**
     * change to no headset layout
     */
    private void changeToNoHeadsetLayout() {
        mMainLayout.setVisibility(View.GONE);
        mBtnPlayContainer.setVisibility(View.GONE);
        mNoEarphoneTextLayout.setVisibility(View.VISIBLE);
        mNoHeadsetImgView.setVisibility(View.VISIBLE);
        mNoHeadsetImgViewWrap.setVisibility(View.VISIBLE);
        mNoHeadsetLayout.setVisibility(View.VISIBLE);
        mNoHeadsetImgViewWrap.setElevation(mMiddleShadowSize);
    }

    /**
     * start recording
     */
    private void startRecording() {
        int readExtStorage = checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
        int writeExtStorage = checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        //int readMediaAudio = checkSelfPermission(android.Manifest.permission.READ_MEDIA_AUDIO);
        List<String> mPermissionStrings = new ArrayList<String>();
        boolean mRequest = false;

       if (readExtStorage != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.READ_EXTERNAL_STORAGE);
            mRequest = true;
        }
        if (writeExtStorage != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            mRequest = true;
        }
     /* if (readMediaAudio != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(android.Manifest.permission.READ_MEDIA_AUDIO);
            mRequest = true;
        }
        */
        if (mRequest == true) {
            String[] mPermissionList = new String[mPermissionStrings.size()];
            mPermissionList = mPermissionStrings.toArray(mPermissionList);
            requestPermissions(mPermissionList, PERMISSION_REQUEST_CODE_RECORDING);
            return;
        }
        Intent recordIntent = new Intent(this, FmRecordActivity.class);
        recordIntent.putExtra(FmStation.CURRENT_STATION, mCurrentStation);
        startActivityForResult(recordIntent, REQUEST_CODE_RECORDING);
    }

    private void openSavedRecordings() {
        int readExtStorage = checkSelfPermission(Manifest.permission.READ_EXTERNAL_STORAGE);
        int writeExtStorage = checkSelfPermission(Manifest.permission.WRITE_EXTERNAL_STORAGE);
        //int readMediaAudio = checkSelfPermission(android.Manifest.permission.READ_MEDIA_AUDIO);
        List<String> mPermissionStrings = new ArrayList<String>();
        boolean mRequest = false;

       if (readExtStorage != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.READ_EXTERNAL_STORAGE);
            mRequest = true;
        }
        if (writeExtStorage != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(Manifest.permission.WRITE_EXTERNAL_STORAGE);
            mRequest = true;
        }
      /*if (readMediaAudio != PackageManager.PERMISSION_GRANTED) {
            mPermissionStrings.add(android.Manifest.permission.READ_MEDIA_AUDIO);
            mRequest = true;
        }
        */
        if (mRequest == true) {
            String[] mPermissionList = new String[mPermissionStrings.size()];
            mPermissionList = mPermissionStrings.toArray(mPermissionList);
            requestPermissions(mPermissionList, PERMISSION_REQUEST_CODE_SAVED_RECORDING);
            return;
        }
        Intent playMusicIntent = new Intent(Intent.ACTION_VIEW);
                int playlistId = FmRecorder.getPlaylistId(mContext);
                Bundle extras = new Bundle();
                extras.putInt("playlist", playlistId);
                try {
                    playMusicIntent.putExtras(extras);
                    playMusicIntent.setClassName("com.google.android.music",
                            "com.google.android.music.ui.TrackContainerActivity");
                    playMusicIntent.setType("vnd.android.cursor.dir/playlist");
                    startActivity(playMusicIntent);
                } catch (ActivityNotFoundException e1) {
                    try {
                        playMusicIntent = new Intent();
                        Bundle extraData = new Bundle();
                        extraData.putString("playlist", String.valueOf(playlistId));
                        playMusicIntent.putExtras(extraData);
                        playMusicIntent.setClassName("com.android.music",
                            "com.android.music.PlaylistBrowserActivity");
                        startActivity(playMusicIntent);
                    } catch (ActivityNotFoundException e2) {
                        try {
                        playMusicIntent = new Intent(Intent.ACTION_VIEW);
                            Bundle aExtraData = new Bundle();
                            aExtraData.putString("playlist", String.valueOf(playlistId));
                            playMusicIntent.putExtras(aExtraData);
                        playMusicIntent.setType("vnd.android.cursor.dir/playlist");
                        startActivity(playMusicIntent);
                        } catch (ActivityNotFoundException e3) {
                            Log.d(TAG, "onOptionsItemSelected, No activity " +
                                "respond playlist view intent");
                        }
                    }
                }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
            int[] grantResults) {
        boolean granted = true;
        boolean mShowPermission = true;
        if (permissions.length <= 0 || grantResults.length <= 0) {
            Log.d(TAG, "permission length not sufficient");
            showToast(getString(R.string.missing_required_permission));
            return;
        }
        if (requestCode == PERMISSION_REQUEST_POWER_ON) {
            granted = (grantResults[0] == PackageManager.PERMISSION_GRANTED);
            if (!granted) {
                mShowPermission = shouldShowRequestPermissionRationale(permissions[0]);
            }
            Log.i(TAG, "<onRequestPermissionsResult> Power on fm granted" + granted);
            if (granted == true) {
                if (mService != null) {
                    mService.setRecordingPermission(true);
                    powerUpFm();
                }
            } else if (!mShowPermission) {
                showToast(getString(R.string.missing_required_permission));
            }
        } else if (requestCode == PERMISSION_REQUEST_CODE_RECORDING) {
            for (int counter = 0; counter < permissions.length; counter++) {
                boolean permissionGranted = false;
                permissionGranted = (grantResults[counter] ==
                                         PackageManager.PERMISSION_GRANTED);
                granted = granted && permissionGranted;
                if (!permissionGranted) {
                    mShowPermission = mShowPermission && shouldShowRequestPermissionRationale(
                                      permissions[counter]);
                }
            }
            Log.i(TAG, "<onRequestPermissionsResult> Record audio granted" + granted);
            if (granted == true) {
                startRecording();
            } else if (!mShowPermission) {
                showToast(getString(R.string.missing_required_permission));
            }
        } else if (requestCode == PERMISSION_REQUEST_CODE_SAVED_RECORDING) {
            for (int counter = 0; counter < permissions.length; counter++) {
                boolean permissionGranted = false;
                permissionGranted = (grantResults[counter] ==
                                             PackageManager.PERMISSION_GRANTED);
                granted = granted && permissionGranted;
                if (!permissionGranted) {
                    mShowPermission = mShowPermission && shouldShowRequestPermissionRationale(
                                      permissions[counter]);
                }
            }
            Log.i(TAG, "<onRequestPermissionsResult> Read/Write permission granted" + granted);
            if (granted == true) {
                openSavedRecordings();
            } else if (!mShowPermission) {
                showToast(getString(R.string.missing_required_permission));
            }
        }
    }

    /**
     * set stereo or mono
     * @param isMono mono or not
     * @return whether success
     */
    public void setStereoMono(boolean isMono) {
        mService.setStereoMono(isMono);
    }

    /**
     * get stereo or mono
     * @return true is stereo, false mono
     */
    public boolean getStereoMono() {
        if (null != mService) {
            return mService.getStereoMono();
        }
        return false;
    }
}
