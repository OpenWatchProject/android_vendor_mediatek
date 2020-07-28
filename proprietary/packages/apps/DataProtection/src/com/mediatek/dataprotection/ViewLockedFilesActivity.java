package com.mediatek.dataprotection;


import android.app.Activity;
import android.app.ExpandableListActivity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.LoaderManager.LoaderCallbacks;
import android.content.ComponentName;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.Loader;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.content.ServiceConnection;
import android.drm.DrmManagerClient;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.Point;
import android.graphics.drawable.Drawable;
import android.media.MediaFile;
import android.Manifest;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.text.TextUtils;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseExpandableListAdapter;
import android.widget.Button;
import android.widget.ExpandableListView;
import android.widget.ImageView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.AbsListView.RecyclerListener;
import android.widget.PopupMenu.OnMenuItemClickListener;

import com.mediatek.dataprotection.DataProtectionApplication.ThumbnailCache;
import com.mediatek.dataprotection.DataProtectionService.DecryptFailListener;
import com.mediatek.dataprotection.DataProtectionService.NotificationListener;
import com.mediatek.dataprotection.utils.CtaDrmUtils;
import com.mediatek.dataprotection.utils.DataBaseHelper;
import com.mediatek.dataprotection.utils.FeatureOptionsUtils;
import com.mediatek.dataprotection.utils.FileUtils;
import com.mediatek.dataprotection.utils.DataProtectionLockPatternUtils;
import com.mediatek.dataprotection.utils.PermissionUtils;
import com.mediatek.dataprotection.utils.Log;
import com.mediatek.dataprotection.utils.UiUtils;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashSet;
import java.util.List;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;


public class ViewLockedFilesActivity extends ExpandableListActivity implements
        DecryptFailListener {

    protected static final String TAG = "ViewLockedFilesActivity";
    protected static final String TAG_CANCEL_DIALG = "cacel_dialog_main";
    private static final int REQUEST_READ_WRITE_STORAGE_PERMISSION_CODE = 1;
    private static final int REQUEST_READ_WRITE_STORAGE_PERMISSION_FIRST_CODE = 2;

    private List<String> FILE_TYPES = new ArrayList<String>();
    private List<ArrayList<FileInfo>> mAllFiles = null;

    private ExpandableListView lv = null;
    private LockedFilesAdapter mAdapter = null;
    private TextView mLockedTitle = null;
    private ImageView mLockedTitleSpinner = null;
    private View mLoadingView;

    private DataProtectionService mService = null;
    private String mDateFormat = null;
    private ActionMode mActionMode;
    private final ActionModeCallBack mActionModeCallBack = new ActionModeCallBack();
    private LoaderCallbacks<LockedFileResult> mLoaderCallBack = null;
    private final int mLoaderId = 10;
    private boolean mNeedShowCancel = false;
    private int mCancelTitle = 0;
    private boolean mNeedShowPatternRequest = false;
    private boolean mHasExit = true;
    private View mMainView = null;
    private ThreadPoolExecutor mExecutor = new ThreadPoolExecutor(1, 1, 10,
            TimeUnit.SECONDS, new LinkedBlockingQueue<Runnable>());
    private boolean mHasSetKey = false;
    private boolean mNeedRestore = false;
    private Handler mHandler = new Handler();
    private Bundle mDataFromNewIntent = null;

    private long mCancelTaskId;
    private int mCancelResId;
    private boolean mNeedRestoreCancelDialog = false;
    private boolean mIsInChangePattern = false;
    private boolean mNeedRestoreConfirmForChangeFragment = false;
    private boolean mNeedRestoreChangePatternFragment = false;
    private String mCacheOldPattern;
    private Bundle mCachedChoosePatternState = null;
    private boolean mHasOpenedFileToShow = false;
    private boolean mIsLoading = false;

    private void initWhenHaveService() {
        ViewLockedFilesActivity.this.invalidateOptionsMenu();
        if (PermissionUtils.hasStorageReadPermission(ViewLockedFilesActivity.this)) {
            if (!mIsLoading) {
                getLoaderManager().initLoader(mLoaderId, null, mLoaderCallBack);
                mIsLoading = true;
            }
        }
        mService.registerDataListener(mDataObserver);
        mService.registerNotificationListener(mNotificationListener);
    }

    private DataProtectionService.NotificationListener mNotificationListener = new NotificationListener() {

        @Override
        public void onCancel(long taskId, int resId) {
            AlertDialogFragment.showCancelTaskDialog(
                    ViewLockedFilesActivity.this, mService, resId,
                    TAG_CANCEL_DIALG, taskId);

            // record task id, res id
            mCancelTaskId = taskId;
            mCancelResId = resId;
            Log.d(TAG, "onCancel - set task id: " + taskId + ", res id: " + resId);
        }

        @Override
        public void onViewDecryptFailHistory() {
            startDecryptFailActivity();
        }

    };

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName arg0, IBinder arg1) {
            mService = ((DataProtectionService.DataProtectionBinder) arg1)
                    .getService();
            initWhenHaveService();
            mService.registerDecryptFailListener(ViewLockedFilesActivity.this);
            mService.registerNotificationListener(mNotificationListener);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mService = null;
        }
    };

    private ChooseLockPatternFragment.PatternLockChooseEventListener mPatternChooseListener = new ChooseLockPatternFragment.PatternLockChooseEventListener() {

        @Override
        public void onPatternSetSuccess(String pattern) {
            if (!PermissionUtils.hasStorageReadPermission(ViewLockedFilesActivity.this) ||
                !PermissionUtils.hasStorageWritePermission(ViewLockedFilesActivity.this)) {
                     PermissionUtils.requestReadWriteStoragePermission(ViewLockedFilesActivity.this,
                         REQUEST_READ_WRITE_STORAGE_PERMISSION_FIRST_CODE);
            } else {
                mConfirmPassword = true;
                mMainView.setVisibility(View.VISIBLE);
                invalidateOptionsMenu();
            }
            if (!mHasSetKey && pattern != null) {
                DrmManagerClient client = new DrmManagerClient(
                    ViewLockedFilesActivity.this);
                CtaDrmUtils.setKey(client, pattern.getBytes());
                client.release();
                mHasSetKey = true;
            }
        }

        @Override
        public void onCancel() {
            mIsInChangePattern = false;
            mMainView.setVisibility(View.VISIBLE);
            invalidateOptionsMenu();
        }

        @Override
        public void onPatternChangeSuccess(String oldPattern, String newPattern) {
            mMainView.setVisibility(View.VISIBLE);
            mIsInChangePattern = false;
            invalidateOptionsMenu();
            if (mService != null) {
                mService.changePassword(oldPattern, newPattern);
            }
        }
    };
    private ConfirmLockPatternFragment.PatternEventListener mPatternConfirmEventListener = new ConfirmLockPatternFragment.PatternEventListener() {

        @Override
        public void onPatternVerifySuccess(String pattern) {
            if (!PermissionUtils.hasStorageReadPermission(ViewLockedFilesActivity.this) ||
                !PermissionUtils.hasStorageWritePermission(ViewLockedFilesActivity.this)) {
                     PermissionUtils.requestReadWriteStoragePermission(ViewLockedFilesActivity.this,
                         REQUEST_READ_WRITE_STORAGE_PERMISSION_CODE);
            } else {

                if (!mHasSetKey && pattern != null) {
                    DrmManagerClient client = new DrmManagerClient(
                        ViewLockedFilesActivity.this);
                    CtaDrmUtils.setKey(client, pattern.getBytes());
                    client.release();
                    mHasSetKey = true;
                }
                mConfirmPassword = true;
                if (mNeedRestore) {
                    mNeedRestore = false;
                    mActionMode = ViewLockedFilesActivity.this
                        .startActionMode(mActionModeCallBack);
                    mActionModeCallBack.updateActionMode();
                } else {
                    ViewLockedFilesActivity.this.invalidateOptionsMenu();
                }

                if (mNeedRestoreConfirmForChangeFragment) {
                    mMainView.setVisibility(View.GONE);
                    mIsInChangePattern = true;
                    invalidateOptionsMenu();
                    ConfirmLockPatternFragment.show(getFragmentManager(),
                        R.id.pattern_fragment, mInputForChangePatternListener,
                        TAG_PATTERN_VERIFY_FOR_CHANGE,
                        getString(R.string.lockpattern_confirm_pattern));
                } else if (mNeedRestoreChangePatternFragment) {
                    mMainView.setVisibility(View.GONE);
                    mIsInChangePattern = true;
                    invalidateOptionsMenu();
                    ChooseLockPatternFragment.show(getFragmentManager(),
                        mPatternChooseListener, R.id.pattern_fragment,
                        ChooseLockPatternFragment.TYPE_CHANGE_PATTERN, mCacheOldPattern,
                        TAG_PATTERN_CHANGE, mCachedChoosePatternState);
                } else {
                    mMainView.setVisibility(View.VISIBLE);
                    mIsInChangePattern = false;
                }
                mNeedRestoreChangePatternFragment = false;
                mNeedRestoreConfirmForChangeFragment = false;

                if (mNeedRestoreCancelDialog) {
                    Log.d(TAG, "onPatternVerifySuccess - show cancel dialog");
                    AlertDialogFragment.showCancelTaskDialog(
                        ViewLockedFilesActivity.this, mService, mCancelResId,
                        TAG_CANCEL_DIALG, mCancelTaskId);
                    mNeedRestoreCancelDialog = false;
                }
            }
        }

        @Override
        public void onCancel() {
            mNeedShowCancel = false;
            mNeedShowPatternRequest = false;
        }

        @Override
        public void onPatternNotSet() {
            ChooseLockPatternFragment.show(getFragmentManager(),
                    mPatternChooseListener, R.id.pattern_fragment,
                    ChooseLockPatternFragment.TYPE_SET_PATTERN, null,
                    TAG_PATTERN_SET, null);
        }

    };

    DataProtectionService.EncryptDecryptListener mDataObserver = new DataProtectionService.EncryptDecryptListener() {

        @Override
        public void onStorageNotEnough(List<FileInfo> file) {
            Handler handler = new Handler(getMainLooper());
            handler.post(new Runnable() {
                @Override
                public void run() {
                    if (PermissionUtils.hasStorageReadPermission(ViewLockedFilesActivity.this)) {
                        if (!mIsLoading) {
                            getLoaderManager().initLoader(mLoaderId, null, mLoaderCallBack);
                            mIsLoading = true;
                        }
                    }
                }

            });
        }

        @Override
        public void onDecryptFail(final FileInfo file) {
        }

        @Override
        public void onEncryptFail(FileInfo file) {
        }

        @Override
        public void onEncryptCancel(List<FileInfo> files) {
        }

        @Override
        public void onDecryptCancel(final List<FileInfo> files) {
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    if (PermissionUtils.hasStorageReadPermission(ViewLockedFilesActivity.this)) {
                        if (!mIsLoading) {
                            getLoaderManager().destroyLoader(mLoaderId);
                            getLoaderManager().restartLoader(mLoaderId, null, mLoaderCallBack);
                            mIsLoading = true;
                        }
                    }
                }

            });
        }

        @Override
        public void onDeleteCancel(final List<FileInfo> files) {
            Log.d(TAG, "onDeleteCancel " + files.size());
        }

        @Override
        public void onDeleteFail(FileInfo file) {

        }
    };

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        setContentView(R.layout.main_activity_locked_files);

        getLoaderManager();
        lv = getExpandableListView();
        mLockedTitle = (TextView) findViewById(R.id.locked_title);
        mLockedTitle.setText(String.format(getString(R.string.locked_header), 0));
        mLockedTitleSpinner = (ImageView) findViewById(R.id.locked_title_spinner);
        mMainView = findViewById(R.id.mainLayout);

        FILE_TYPES.add(getString(R.string.picture_title));
        FILE_TYPES.add(getString(R.string.audio_title));
        FILE_TYPES.add(getString(R.string.video_title));
        FILE_TYPES.add(getString(R.string.other_title));

        mAdapter = (LockedFilesAdapter) getLastNonConfigurationInstance();
        if (mAdapter == null) {

            mAdapter = new LockedFilesAdapter();
        }
        setListAdapter(mAdapter);
        lv.setOnChildClickListener(this);
        lv.setRecyclerListener(mRecycleListener);
        if (mAllFiles == null) {
            mAllFiles = new ArrayList<ArrayList<FileInfo>>();
            for (int i = 0; i < 4; i++) {
                mAllFiles.add(new ArrayList<FileInfo>());
            }
        }

        mLoadingView = findViewById(R.id.loading);
        TextView loadingTextView = (TextView) mLoadingView
                .findViewById(R.id.loading_text);
        loadingTextView
                .setText(com.mediatek.internal.R.string.contact_widget_loading);

        MountPointManager.getInstance().init(this);
        mDateFormat = getString(R.string.date_format);
        mLoaderCallBack = new LoaderCallbacks<LockedFileResult>() {

            @Override
            public Loader<LockedFileResult> onCreateLoader(int id, Bundle args) {
                return new LockedFileLoader(getApplicationContext(), mService);
            }

            @Override
            public void onLoadFinished(Loader<LockedFileResult> loader,
                    LockedFileResult data) {
                if (data != null) {

                    swapResult(data);
                    updateHeader();
                } else {
                    Log.d(TAG, "loading result is null");
                }
                mIsLoading = false;
            }

            @Override
            public void onLoaderReset(Loader<LockedFileResult> loader) {
                mIsLoading = false;
            }

        };
        mHasExit = false;
        // bind service
        Intent serviceIntent = new Intent(this, DataProtectionService.class);
        startService(serviceIntent);
        bindService(serviceIntent, mServiceConnection, BIND_AUTO_CREATE);
        initIntent();

        if (DataProtectionLockPatternUtils.hasPattern(ViewLockedFilesActivity.this) &&
            !PermissionUtils.hasStorageReadWritePermission(ViewLockedFilesActivity.this) &&
            !PermissionUtils.showReadOrWriteRational(ViewLockedFilesActivity.this)) {
                String toastStr = getString(R.string.denied_required_permission);
                Toast.makeText(getApplicationContext(), toastStr, Toast.LENGTH_LONG).show();
                finish();
        }

        Log.d(TAG, "onCreate: " + getIntent().getAction());
    }

    private void initIntent() {
        Intent newIntent = getIntent();
        Bundle extra = newIntent != null ? newIntent.getExtras() : null;
        String action = (extra != null ? extra.getString("ACTION") : null);
        mConfirmPassword = DataProtectionApplication.getPriviledge(this);
        Log.d(TAG, "initIntent " + mConfirmPassword);
        if (mConfirmPassword) {
            DataProtectionApplication.setPriviledge(this, false);
        }
        if (needShowCancelDialog()) {
            mCancelTitle = extra != null ? extra.getInt("TITLE") : 0;
            Log.d(TAG, "mNeedShowCancel " + action);
            mNeedShowCancel = mCancelTitle > 0 ? true : false;
            mDataFromNewIntent = extra;
            DataProtectionApplication.setNeedShowCancel(this, false);
        } else if (needShowDecryptFailActivity()) {
            startDecryptFailActivity();
        }
    }

    private RecyclerListener mRecycleListener = new RecyclerListener() {
        @Override
        public void onMovedToScrapHeap(View view) {
            final ImageView iconThumb = (ImageView) view
                    .findViewById(R.id.icon_thumb);
            if (iconThumb != null) {
                final ThumbnailAsyncTask oldTask = (ThumbnailAsyncTask) iconThumb
                        .getTag();
                if (oldTask != null) {
                    oldTask.cancel(true);
                    iconThumb.setTag(null);
                }
            }
        }
    };

    @Override
    public void onDestroy() {
        mExecutor.shutdown();
        unbindService(mServiceConnection);
        super.onDestroy();
    }

    @Override
    public void onBackPressed() {
        if (!mConfirmPassword) {
            finish();
        } else {
            if (mConfirmPassword) {
                FragmentManager mgr = getFragmentManager();
                Fragment frag = mgr
                        .findFragmentByTag(TAG_PATTERN_VERIFY_FOR_CHANGE);
                if (frag != null) {
                    FragmentTransaction transaction = mgr.beginTransaction();
                    transaction.remove(frag);
                    transaction.commitAllowingStateLoss();
                    // in confirm saved pattern state, click back key will cacel
                    // confirm saved pattern, so set mIsInChangePattern to false;
                    mIsInChangePattern = false;
                    updateUi();
                    return;
                }
                frag = mgr.findFragmentByTag(TAG_PATTERN_CHANGE);
                if (frag != null) {
                    return;
                }
            }
            super.onBackPressed();
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[],
            int[] grantResults) {
        Log.d(TAG, "onRequestPermissionsResult with request code: " + requestCode);
        if(permissions == null || permissions.length == 0 ||
                grantResults == null || grantResults.length == 0){
            Log.e(TAG, "onRequestPermissionsResult, Permission or grant res null");
            return;
        }

        boolean isGranted = true;
        String toastStr = getString(R.string.denied_required_permission);
        switch (requestCode) {
            case REQUEST_READ_WRITE_STORAGE_PERMISSION_CODE:
                for (int i=0; i < grantResults.length; i++) {
                    if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                        isGranted = false;
                        break;
                    }
                }

                if (isGranted) {
                    if (!mIsLoading) {
                        getLoaderManager().initLoader(mLoaderId, null, mLoaderCallBack);
                        mIsLoading = true;
                    }
                    mPatternConfirmEventListener.onPatternVerifySuccess(null);
                } else {
                    Toast.makeText(getApplicationContext(), toastStr, Toast.LENGTH_LONG).show();
                    finish();
                }
            break;

            case REQUEST_READ_WRITE_STORAGE_PERMISSION_FIRST_CODE:
                for (int i=0; i < grantResults.length; i++) {
                    if (grantResults[i] != PackageManager.PERMISSION_GRANTED) {
                        isGranted = false;
                        break;
                    }
                }

                if (isGranted) {
                    if (!mIsLoading) {
                        getLoaderManager().initLoader(mLoaderId, null, mLoaderCallBack);
                        mIsLoading = true;
                    }
                    mPatternChooseListener.onPatternSetSuccess(null);
                } else {
                    Toast.makeText(getApplicationContext(), toastStr, Toast.LENGTH_LONG).show();
                    finish();
                }
            break;

            default:
            super.onRequestPermissionsResult(requestCode, permissions,
                   grantResults);
        }
    }

    private void updateUi() {
        if (mConfirmPassword) {
            mMainView.setVisibility(View.VISIBLE);
            invalidateOptionsMenu();
        }
    }

    private void closeFragment() {
        FragmentManager mgr = getFragmentManager();
        Fragment fragment = mgr.findFragmentByTag(TAG_PATTERN_VERIFY);
        FragmentTransaction transaction = mgr.beginTransaction();
        if (fragment != null) {
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
        } else if (((fragment = mgr.findFragmentByTag(TAG_PATTERN_CHANGE)) != null)) {
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
        } else if (((fragment = mgr.findFragmentByTag(TAG_PATTERN_SET)) != null)) {
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
        } else if ((((fragment = mgr
                .findFragmentByTag(TAG_PATTERN_VERIFY_FOR_CHANGE)) != null))) {
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
        }
    }

    private void swapResult(LockedFileResult data) {
        // When finish loading, disable loading view and show locked title,
        // then enable all option menu.
        mLockedTitle.setVisibility(View.VISIBLE);
        mLockedTitleSpinner.setVisibility(View.VISIBLE);
        mLoadingView.setVisibility(View.GONE);
        invalidateOptionsMenu();

        if (data != null) {
            if (data.pictures != null) {
                for (FileInfo fileInfo : data.pictures) {
                    if (mAdapter.mSelectedList.contains(fileInfo.getPath())) {
                        fileInfo.setChecked(true);
                    }
                }
                mAllFiles.set(0, (ArrayList<FileInfo>) data.pictures);
            } else {
                mAllFiles.set(0, new ArrayList<FileInfo>());
            }
            Log.d(TAG, "pictures: " + mAllFiles.get(0).size());
            if (data.audios != null) {
                for (FileInfo fileInfo : data.audios) {
                    if (mAdapter.mSelectedList.contains(fileInfo.getPath())) {
                        fileInfo.setChecked(true);
                    }
                }
                mAllFiles.set(1, (ArrayList<FileInfo>) data.audios);
            } else {
                mAllFiles.set(1, new ArrayList<FileInfo>());
            }
            Log.d(TAG, "audios: " + mAllFiles.get(1).size());
            if (data.videos != null) {
                for (FileInfo fileInfo : data.videos) {
                    if (mAdapter.mSelectedList.contains(fileInfo.getPath())) {
                        fileInfo.setChecked(true);
                    }
                }
                mAllFiles.set(2, (ArrayList<FileInfo>) data.videos);
            } else {
                mAllFiles.set(2, new ArrayList<FileInfo>());
            }
            Log.d(TAG, "videos: " + mAllFiles.get(2).size());
            if (data.others != null) {
                for (FileInfo fileInfo : data.others) {
                    if (mAdapter.mSelectedList.contains(fileInfo.getPath())) {
                        fileInfo.setChecked(true);
                    }
                }
                mAllFiles.set(3, (ArrayList<FileInfo>) data.others);
            } else {
                mAllFiles.set(3, new ArrayList<FileInfo>());
            }
            Log.d(TAG, "others: " + mAllFiles.get(3).size());
        }
        sort();
        mAdapter.notifyDataSetChanged();
    }

    private boolean mConfirmPassword = false;
    private String mDecryptFailFileName = null;

    private final static String KEY_CANCEL_DIALOG = "CANCEL_OPERATE";
    private final static String KEY_PATTERN_REQUEST_DIALOG = "INPUT_PATTERN_OPERATE";

    private final static int CODE_REQUEST_CONFIRM = 100;
    private static final int CODE_REQUEST_SET_PASSWORD = 200;
    private static final int CODE_REQUEST_ADD_FILE = 300;
    private static final int CODE_VIEW_UNLOCK_HISTORY = 400;
    private static final int CODE_REQUEST_CHANGE_PASSWORD = 500;
    private static final int CODE_VIEW_DECRYPT_FAIL_HISTORY = 400;
    private static final String TAG_PATTERN_VERIFY = "verify_pattern";
    private static final String TAG_PATTERN_SET = "set_pattern";
    private static final String TAG_PATTERN_CHANGE = "change_pattern";
    private static final String TAG_PATTERN_VERIFY_FOR_CHANGE = "verify_pattern_for_change";
    private static final String ACTION_VIEW = "com.mediatek.dataprotection.ACTION_VIEW_LOCKED_FILE";

    @Override
    public void onResume() {
        super.onResume();
        if (DataProtectionLockPatternUtils.hasPattern(ViewLockedFilesActivity.this) &&
            !PermissionUtils.hasStorageReadWritePermission(ViewLockedFilesActivity.this) &&
            !PermissionUtils.showReadOrWriteRational(ViewLockedFilesActivity.this)) {
                String toastStr = getString(R.string.denied_required_permission);
                Toast.makeText(getApplicationContext(), toastStr, Toast.LENGTH_LONG).show();
                finish();
        }

        mHasOpenedFileToShow = false;
        final Intent intent = getIntent();
        Log.d(TAG, "onResume password 1 " + mConfirmPassword
                + " cache Intent: " + mDataFromNewIntent);
        mConfirmPassword = mDataFromNewIntent != null ? mDataFromNewIntent
                .getBoolean(DataProtectionService.KEY_INVOKE, false)
                : mConfirmPassword;
        Log.d(TAG, "onResume password 2 " + mConfirmPassword);

        if (mNeedShowCancel && mDataFromNewIntent != null) {
            // record task id, res id
            mCancelTaskId = mDataFromNewIntent.getLong(DataProtectionService.KEY_TASK_ID);
            mCancelResId = mDataFromNewIntent.getInt("TITLE");
            if (mCancelResId > 0 && mCancelTaskId > 0) {
                AlertDialogFragment.showCancelTaskDialog(
                        ViewLockedFilesActivity.this, mService,
                        mCancelResId, TAG_CANCEL_DIALG, mCancelTaskId);
            }
        }
        mNeedShowCancel = false;
        mDataFromNewIntent = null;

        if (!mConfirmPassword) {
            mMainView.setVisibility(View.INVISIBLE);
            if (mActionMode != null) {
                mNeedRestore = true;
                mActionMode.finish();
                invalidateOptionsMenu();
            } else {
                invalidateOptionsMenu();
            }
            // if cancel dialog exists and no permission, need cancel dialog
            Log.d(TAG, "cancel dialog exists and no permission, need cancel dialog");
            mNeedRestoreCancelDialog = AlertDialogFragment.dismissCancelTaskDialog(
                    ViewLockedFilesActivity.this, mService, mCancelResId,
                    TAG_CANCEL_DIALG, mCancelTaskId);
            mNeedRestoreChangePatternFragment = removeChangePatternFragment();
            mNeedRestoreConfirmForChangeFragment = removePatternVerifyForChangePatternFragment();
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mPatternConfirmEventListener,
                    TAG_PATTERN_VERIFY);
        } else {
            removeFragmentByTag(TAG_PATTERN_VERIFY);
            mPatternConfirmEventListener.onPatternVerifySuccess(null);
        }
        if (null == mService) {
            if (null == startService(new Intent(ViewLockedFilesActivity.this,
                    DataProtectionService.class))) {
                finish();
                return;
            }

            if (!bindService(new Intent(ViewLockedFilesActivity.this,
                    DataProtectionService.class), mServiceConnection,
                    BIND_AUTO_CREATE)) {
                finish();
                return;
            }
        } else {
            mService.registerDecryptFailListener(this);
            mService.registerNotificationListener(mNotificationListener);
        }
        if (mService != null) {
            mService.registerDataListener(mDataObserver);
        }
        DataProtectionApplication.setActivityState(this, true);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.d(TAG, "onConfigurationChanged mIsInChangePattern: " + mIsInChangePattern);

        FragmentManager mgr = getFragmentManager();
        Fragment fragment = mgr.findFragmentByTag(TAG_PATTERN_VERIFY);
        if (fragment != null) {
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mPatternConfirmEventListener,
                    TAG_PATTERN_VERIFY);
        } else if (((fragment = mgr.findFragmentByTag(TAG_PATTERN_CHANGE)) != null)) {
            String pattern = (fragment.getArguments() != null) ? fragment
                    .getArguments().getString("DATA") : null;
            if (!TextUtils.isEmpty(pattern)) {
                ChooseLockPatternFragment.show(getFragmentManager(),
                        mPatternChooseListener, R.id.pattern_fragment,
                        ChooseLockPatternFragment.TYPE_CHANGE_PATTERN, pattern,
                        TAG_PATTERN_CHANGE,
                        ((ChooseLockPatternFragment) fragment).getCurrentState());
            } else {
                FragmentTransaction transaction = mgr.beginTransaction();
                transaction.remove(fragment);
                transaction.commitAllowingStateLoss();
            }
        } else if (((fragment = mgr.findFragmentByTag(TAG_PATTERN_SET)) != null)) {
            ChooseLockPatternFragment.show(getFragmentManager(),
                    mPatternChooseListener, R.id.pattern_fragment,
                    ChooseLockPatternFragment.TYPE_SET_PATTERN, null,
                    TAG_PATTERN_SET, ((ChooseLockPatternFragment) fragment).getCurrentState());
        } else if ((((fragment = mgr.findFragmentByTag(TAG_PATTERN_VERIFY_FOR_CHANGE)) != null))) {
           // ConfirmLockPatternFragment header message is different because of different pattern
           // verify request. For resetinng the user pattern, show "Confirm saved pattern".
            if (mIsInChangePattern) {
                ConfirmLockPatternFragment.show(getFragmentManager(),
                        R.id.pattern_fragment, mInputForChangePatternListener,
                            TAG_PATTERN_VERIFY_FOR_CHANGE,
                                getString(R.string.lockpattern_confirm_pattern));
            } else {
                Log.e(TAG, "onConfigurationChanged error, should not go to here");
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        Log.d(TAG, "onActivityResult: " + requestCode + " " + resultCode + " "
                + data);
        if (data == null) {
            return;
        }
        switch (resultCode) {
        case Activity.RESULT_OK:
            mConfirmPassword = data.getBooleanExtra(
                    DataProtectionService.KEY_INVOKE, false);
            if (!mConfirmPassword) {
                finish();
            } else {
                invalidateOptionsMenu();
            }
            break;
        case Activity.RESULT_CANCELED:
            finish();
            break;
        default:
            break;
        }

    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        Log.d(TAG, "onCreateOptionsMenu menu " + mConfirmPassword);
        super.onCreateOptionsMenu(menu);

        MenuInflater inflater = getMenuInflater();
        menu.clear();
        if (null == mService) {
            return false;
        }
        inflater.inflate(R.menu.main_navigation_view_menu, menu);
        return mConfirmPassword && !mIsInChangePattern;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        Log.d(TAG, "onPrepareOptions menu " + mConfirmPassword);
        super.onPrepareOptionsMenu(menu);
        MenuItem addLockFile = menu.findItem(R.id.add_lock_file);
        MenuItem edit = menu.findItem(R.id.edit);
        MenuItem changePattern = menu.findItem(R.id.change_pattern);
        MenuItem unlockHistory = menu.findItem(R.id.unlock_history);
        MenuItem decryptFailHistory = menu.findItem(R.id.decrypt_fail_history);
        // Only when loading finish need show these option menus
        boolean isLoadingFinish = (mConfirmPassword && (mLoadingView
                .getVisibility() == View.GONE));
        addLockFile.setEnabled(isLoadingFinish);
        edit.setEnabled(isLoadingFinish);
        changePattern.setEnabled(isLoadingFinish);
        unlockHistory.setEnabled(isLoadingFinish);
        decryptFailHistory.setEnabled(isLoadingFinish);
        return true;
    }

    @Override
    public boolean onChildClick(ExpandableListView parent, View v,
            int groupPosition, int childPosition, long id) {
        Log.d(TAG, "onChildClick...");
        super.onChildClick(parent, v, groupPosition, childPosition, id);
        if (null != mAllFiles) {
            FileInfo selFileInfo = mAllFiles.get(groupPosition).get(
                    childPosition);
            if (!selFileInfo.isDirectory() && mActionMode != null) {
                Log.s(TAG, "filePath: " + selFileInfo.getFileAbsolutePath()
                        + " selFileInfo: " + selFileInfo.isChecked());
                if (selFileInfo.isChecked()) {
                    selFileInfo.setChecked(false);
                    mAdapter.mSelectedList.remove(selFileInfo.getPath());
                } else {
                    selFileInfo.setChecked(true);
                    mAdapter.mSelectedList.add(selFileInfo.getPath());
                }
                // mAdapter.notifyDataSetChanged();
                mAdapter.notifyDataSetInvalidated();
                invalidateOptionsMenu();
            } else if (!selFileInfo.isDirectory() && mActionMode == null) {
                //Music keep AOSP design and would be phase out, When clicking the encrypted
                // audio(groupPosition == 1) should notify the user decrypt the auto first
                // before playing it.
                if (selFileInfo != null && (groupPosition == 3 || groupPosition == 1)) {
                    UiUtils.showToast(this,
                            getString(R.string.unlock_file_first));
                    return true;
                }
                if (selFileInfo.getMimeType() != null) {
                    if (mService.isChangePattern()) {
                        UiUtils.showToast(getApplicationContext(),
                                getString(R.string.forbid_view_file));
                    } else {
                        openFile(selFileInfo);
                    }
                } else {
                    UiUtils.showToast(this,
                            getString(R.string.unlock_file_first));
                }
            }
        }
        if (mActionMode != null && mActionModeCallBack != null) {
            mActionModeCallBack.updateActionModeMenu();
        }
        return true;
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause ... ");
        Fragment fragment = getFragmentManager().findFragmentByTag(TAG_PATTERN_CHANGE);
        if (fragment != null) {
            mCachedChoosePatternState = ((ChooseLockPatternFragment) fragment).getCurrentState();
        } else if ((fragment = getFragmentManager().findFragmentByTag(TAG_PATTERN_SET)) != null) {
            mCachedChoosePatternState = ((ChooseLockPatternFragment) fragment).getCurrentState();
        } else {
            mCachedChoosePatternState = null;
        }

        mConfirmPassword = false;
        if (mService != null) {
            mService.unListenNotificationEvent(null);
            mService.unRegisterDataListener();
            mService.unRegistenerNotificationListener();
        }
        mHasExit = true;
        DataProtectionApplication.setActivityState(this, false);
        super.onPause();
    }

    @Override
    public void onNewIntent(Intent newIntent) {
        Log.d(TAG,
                "onNewIntent with intent: " + newIntent + " "
                        + newIntent.getAction());
        setIntent(newIntent);

        Bundle extra = newIntent.getExtras();
        String action = (extra != null ? extra.getString("ACTION") : null);
        mConfirmPassword = extra != null ? extra
                .getBoolean(DataProtectionService.KEY_INVOKE)
                : mConfirmPassword;
        Log.d(TAG, "mNeedShowCancel outer " + action + mConfirmPassword);
        if (action != null && action.equalsIgnoreCase("cancel")) {
            mNeedShowCancel = true;
            mCancelTitle = extra.getInt("TITLE");
            Log.d(TAG, "mNeedShowCancel " + action);
            mDataFromNewIntent = extra;
        } else if (action != null
                && action.equalsIgnoreCase("view_decrypt_fail_files")) {
            startDecryptFailActivity();
        }

    }

    private boolean needShowDecryptFailActivity() {
        return DataProtectionApplication.isNeedShowDecryptFail(this);
    }

    private boolean needShowCancelDialog() {
        return DataProtectionApplication.isNeedShowCancel(this);
    }

    private void startDecryptFailActivity() {
        Intent intent = new Intent(this, DecryptFailHistoryActivity.class);
        intent.putExtra(DataProtectionService.KEY_INVOKE, mConfirmPassword);
        Log.d(TAG, "startDecryptFail " + mConfirmPassword);
        startActivityForResult(intent, CODE_VIEW_DECRYPT_FAIL_HISTORY);
        DataProtectionApplication.setNeedShowDecryptFail(this, false);
    }

    private void removeFragmentByTag(String tag) {
        FragmentManager fragmentMgr = getFragmentManager();
        Fragment fragment = fragmentMgr.findFragmentByTag(tag);
        if (fragment != null) {
            FragmentTransaction transaction = fragmentMgr.beginTransaction();
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
        }
    }

    private boolean removePatternVerifyForChangePatternFragment() {
        boolean res = false;
        FragmentManager fragmentMgr = getFragmentManager();
        Fragment fragment = fragmentMgr.findFragmentByTag(TAG_PATTERN_VERIFY_FOR_CHANGE);
        if (fragment != null) {
            FragmentTransaction transaction = fragmentMgr.beginTransaction();
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
            res = true;
        }
        return res;
    }

    private boolean removeChangePatternFragment() {
        boolean res = false;
        FragmentManager fragmentMgr = getFragmentManager();
        Fragment fragment = fragmentMgr.findFragmentByTag(TAG_PATTERN_CHANGE);
        if (fragment != null) {
            FragmentTransaction transaction = fragmentMgr.beginTransaction();
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
            mCachedChoosePatternState = ((ChooseLockPatternFragment) fragment).getCurrentState();
            mCacheOldPattern = ((ChooseLockPatternFragment) fragment).getOldPattern();
            res = mCacheOldPattern != null ? true : false;
        }
        return res;
    }

    final ConfirmLockPatternFragment.PatternEventListener mInputForChangePatternListener = new ConfirmLockPatternFragment.PatternEventListener() {

        @Override
        public void onPatternVerifySuccess(String pattern) {
            ChooseLockPatternFragment.show(getFragmentManager(),
                    mPatternChooseListener, R.id.pattern_fragment,
                    ChooseLockPatternFragment.TYPE_CHANGE_PATTERN, pattern,
                    TAG_PATTERN_CHANGE, null);
        }

        @Override
        public void onCancel() {
            mMainView.setVisibility(View.VISIBLE);
            mIsInChangePattern = false;
            invalidateOptionsMenu();
        }

        @Override
        public void onPatternNotSet() {
        }

    };

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.add_lock_file:
            Intent sIntent = new Intent(this, AddFileToLockActivity.class);
            sIntent.putExtra(DataProtectionService.KEY_INVOKE, mConfirmPassword);
            sIntent.putExtra(BaseActivity.EXPIRED, android.os.Process.myPid());
            startActivityForResult(sIntent, CODE_REQUEST_ADD_FILE);
            break;
        case R.id.unlock_history:
            Intent hsIntent = new Intent(this, UnlockHistoryActivity.class);
            hsIntent.putExtra(DataProtectionService.KEY_INVOKE,
                    mConfirmPassword);
            hsIntent.putExtra(BaseActivity.EXPIRED, android.os.Process.myPid());
            startActivityForResult(hsIntent, CODE_VIEW_UNLOCK_HISTORY);
            break;
        case R.id.change_pattern:
            mMainView.setVisibility(View.GONE);
            mIsInChangePattern = true;
            invalidateOptionsMenu();
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mInputForChangePatternListener,
                    TAG_PATTERN_VERIFY_FOR_CHANGE, getString(R.string.lockpattern_confirm_pattern));
            break;
        case R.id.edit:
            mActionMode = startActionMode(mActionModeCallBack);
            mActionModeCallBack.updateActionMode();
            break;
        case R.id.decrypt_fail_history:
            Intent decryptFailIntent = new Intent(this,
                    DecryptFailHistoryActivity.class);
            decryptFailIntent.putExtra(DataProtectionService.KEY_INVOKE,
                    mConfirmPassword);
            decryptFailIntent.putExtra(BaseActivity.EXPIRED, android.os.Process.myPid());
            startActivityForResult(decryptFailIntent,
                    CODE_VIEW_DECRYPT_FAIL_HISTORY);
            break;
        default:
            break;
        }
        return true;
    }

    protected class ActionModeCallBack implements ActionMode.Callback,
            OnMenuItemClickListener {

        private PopupMenu mSelectPopupMenu = null;
        private boolean mSelectedAll = true;
        private Button mTextSelect = null;
        private Menu mEditMenu = null;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            Log.d(TAG, "onCreateActionMode: " + mNeedRestore);
            if (!mConfirmPassword) {
                return false;
            }
            LayoutInflater layoutInflater = (LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View customView = layoutInflater.inflate(R.layout.actionbar_select,
                    null);
            mode.setCustomView(customView);
            mTextSelect = (Button) customView.findViewById(R.id.text_select);
            mTextSelect.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    if (mSelectPopupMenu == null) {
                        mSelectPopupMenu = createSelectPopupMenu(mTextSelect);
                    }
                    updateSelectPopupMenu();
                    mSelectPopupMenu.show();
                }
            });
            MenuInflater menuInflater = mode.getMenuInflater();
            menuInflater.inflate(R.menu.edit_lock_file_menu, menu);
            mEditMenu = menu;
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            Log.d(TAG, "onPrepareActionMode: " + mNeedRestore);
            if (!mConfirmPassword) {
                return false;
            }
            mEditMenu = menu;
            if (mAdapter.getCheckedItemsCount() == 0) {
                menu.findItem(R.id.edit).setEnabled(false);
                menu.findItem(R.id.delete).setEnabled(false);
            } else {
                menu.findItem(R.id.edit).setEnabled(true);
                menu.findItem(R.id.delete).setEnabled(true);
            }
            int selectedFileNum = mAdapter.getCheckedItemsCount();
            String selectString = getResources().getString(
                    R.string.selected_format);
            selectString = String.format(selectString, selectedFileNum);
            Log.d(TAG, "onPrepareActionMode: " + selectString + " "
                    + selectedFileNum);
            mTextSelect.setText(selectString);
            return mConfirmPassword;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
            case R.id.edit:
                onClickEdit();
                break;
            case R.id.delete:
                if (null != mService) {
                    UiUtils.showToast(getApplicationContext(),
                            getString(R.string.start_delete));
                    mService.deleteLockedFiles(mAdapter
                            .getCheckedFileInfoItemsListAndRemove());
                    updateHeader();
                    mAdapter.notifyDataSetChanged();
                    if (mActionMode != null) {
                        mActionMode.finish();
                    }
                }
                break;
            default:
                break;

            }
            return false;
        }

        private void onClickEdit() {
            List<FileInfo> selFiles = null;
            if (mAdapter != null) {
                selFiles = mAdapter.getCheckedFileInfoItemsListAndRemove();
            }
            if (null != selFiles && null != mService) {
                UiUtils.showToast(getApplicationContext(),
                        getString(R.string.start_unlock));
                mService.decrypt(selFiles, null);
            }
            updateHeader();
            mAdapter.notifyDataSetChanged();
            if (mActionMode != null) {
                mActionMode.finish();
            }
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            Log.d(TAG, "onDestroyActionMode: " + mNeedRestore);
            if (null != mAdapter && !mNeedRestore) {
                mAdapter.selectOrDeselectAllItem(false);
                mAdapter.notifyDataSetInvalidated();
                invalidateOptionsMenu();
            }
            if (mActionMode != null) {
                mActionMode = null;
            }
            if (mSelectPopupMenu != null) {
                mSelectPopupMenu.dismiss();
                mSelectPopupMenu = null;
            }
        }

        private PopupMenu createSelectPopupMenu(View anchorView) {
            final PopupMenu popupMenu = new PopupMenu(
                    ViewLockedFilesActivity.this, anchorView);
            popupMenu.inflate(R.menu.select_popup_menu);
            popupMenu.setOnMenuItemClickListener(this);
            return popupMenu;
        }

        private void updateSelectPopupMenu() {
            if (mSelectPopupMenu == null) {
                mSelectPopupMenu = createSelectPopupMenu(mTextSelect);
                return;
            }
            final Menu menu = mSelectPopupMenu.getMenu();
            int selectedCount = mAdapter.getCheckedItemsCount();
            if (mAdapter.getCount() == 0) {
                menu.findItem(R.id.select).setEnabled(false);
            } else {
                menu.findItem(R.id.select).setEnabled(true);
            }
            if (mAdapter.getCount() != selectedCount || mAdapter.getCount() == 0) {
                menu.findItem(R.id.select).setTitle(R.string.select_all);
                mSelectedAll = true;
            } else {
                menu.findItem(R.id.select).setTitle(R.string.deselect_all);
                mSelectedAll = false;
            }
        }

        @Override
        public boolean onMenuItemClick(MenuItem menuItem) {
            switch (menuItem.getItemId()) {
            case R.id.select:
                if (mSelectedAll) {
                    mAdapter.setAllItemChecked(true);
                } else {
                    mAdapter.setAllItemChecked(false);
                }
                updateActionMode();
                invalidateOptionsMenu();
                break;
            default:
                return false;
            }
            return true;
        }

        public void updateActionMode() {
            if (mActionMode == null) {
                Log.d(TAG, "ActionMode has not be created.");
                return;
            }
            if (mActionMode != null && mEditMenu != null) {
                onPrepareActionMode(mActionMode,mEditMenu);
            }
            String format = getString(R.string.selected_format);
            int selected = mAdapter.getCheckedItemsCount();
            String selectString = String.format(format, selected);
            mTextSelect.setText(selectString);

            mActionModeCallBack.updateSelectPopupMenu();
            if (mActionMode != null) {
                mActionMode.invalidate();
            }
        }

        public void updateActionModeMenu() {
            if (mEditMenu == null) {
                return;
            }
            if (mAdapter.getCheckedItemsCount() == 0) {
                mEditMenu.findItem(R.id.edit).setEnabled(false);
                mEditMenu.findItem(R.id.delete).setEnabled(false);
            } else {
                mEditMenu.findItem(R.id.edit).setEnabled(true);
                mEditMenu.findItem(R.id.delete).setEnabled(true);
            }
            updateActionMode();
        }

    } // action mode callback

    private void sort() {
        for (int i = 0; i < mAllFiles.size(); i++) {
            ArrayList<FileInfo> files = mAllFiles.get(i);
            if (files != null && files.size() > 1) {
                Collections.sort(files, FileInfoComparator
                        .getInstance(FileInfoComparator.SORT_BY_TIME));
            }
        }
    }

    private class LockedFilesAdapter extends BaseExpandableListAdapter {

        private static final String TAG = "LockedFilesAdapter";
        private HashSet<String> mSelectedList = new HashSet<String>();

        @Override
        public int getGroupCount() {
            return FILE_TYPES.size();
        }

        public void setAllItemChecked(boolean checked) {
            for (int i = 0; i < mAllFiles.size(); i++) {
                List<FileInfo> files = mAllFiles.get(i);
                for (FileInfo file : files) {
                    file.setChecked(checked);
                    if (checked) {
                        mSelectedList.add(file.getPath());
                    }
                }
            }
            if (!checked) {
                mSelectedList.clear();
            }
            mAdapter.notifyDataSetChanged();
        }

        public int getCount() {
            int total = 0;
            for (int i = 0; i < mAllFiles.size(); i++) {
                List<FileInfo> files = mAllFiles.get(i);
                for (FileInfo file : files) {
                    total += 1;
                }
            }
            return total;
        }

        public int getCheckedItemsCount() {
            int total = 0;
            for (int i = 0; i < mAllFiles.size(); i++) {
                List<FileInfo> files = mAllFiles.get(i);
                for (FileInfo file : files) {
                    if (file.isChecked()) {
                        total += 1;
                    }
                }
            }
            return total;
        }

        @Override
        public int getChildrenCount(int groupPosition) {
            Log.d(TAG, "mAllFiles size: " + mAllFiles.size());
            if (mAllFiles.get(groupPosition).size() == 0) {
                return 1;
            }
            return mAllFiles.get(groupPosition).size();
        }

        @Override
        public Object getGroup(int groupPosition) {
            return mAllFiles.get(groupPosition);
        }

        @Override
        public Object getChild(int groupPosition, int childPosition) {
            if (null != mAllFiles.get(groupPosition)) {
                return mAllFiles.get(groupPosition).get(childPosition);
            }
            return null;
        }

        @Override
        public long getGroupId(int groupPosition) {
            return FILE_TYPES.get(groupPosition).hashCode();
        }

        @Override
        public long getChildId(int groupPosition, int childPosition) {
            if (groupPosition == 0) {
                return childPosition;
            }
            return 0;
        }

        @Override
        public boolean hasStableIds() {
            return false;
        }

        /**
         * This method gets the list of the checked items
         *
         * @return the list of the checked items
         */
        public List<FileInfo> getCheckedFileInfoItemsList() {
            List<FileInfo> fileInfoCheckedList = new ArrayList<FileInfo>();
            for (int i = 0; i < mAllFiles.size(); i++) {
                ArrayList<FileInfo> iFiles = mAllFiles.get(i);
                for (int j = 0; j < iFiles.size(); j++) {
                    if (iFiles.get(j).isChecked()) {
                        fileInfoCheckedList.add(iFiles.get(j));
                    }
                }
            }
            return fileInfoCheckedList;
        }

        /**
         * This method gets the list of the checked items
         *
         * @return the list of the checked items
         */
        public List<FileInfo> getCheckedFileInfoItemsListAndRemove() {
            List<FileInfo> fileInfoCheckedList = new ArrayList<FileInfo>();
            for (int i = 0; i < mAllFiles.size(); i++) {
                ArrayList<FileInfo> iFiles = mAllFiles.get(i);
                ArrayList<FileInfo> toRemoveFiles = new ArrayList<FileInfo>();
                for (int j = 0; j < iFiles.size(); j++) {
                    if (iFiles.get(j).isChecked()) {
                        fileInfoCheckedList.add(iFiles.get(j));
                        toRemoveFiles.add(iFiles.get(j));
                        if (mSelectedList.contains(iFiles.get(j).getFileAbsolutePath())) {
                            mSelectedList.remove(iFiles.get(j).getPath());
                        }
                    }
                }
                iFiles.removeAll(toRemoveFiles);
            }
            return fileInfoCheckedList;
        }

        public void selectOrDeselectAllItem(boolean checked) {
            for (int i = 0; i < mAllFiles.size(); i++) {
                ArrayList<FileInfo> iFiles = mAllFiles.get(i);
                for (int j = 0; j < iFiles.size(); j++) {
                    iFiles.get(j).setChecked(checked);
                    if (checked) {
                        mSelectedList.add(iFiles.get(j).getPath());
                    }
                }
            }
            if (!checked) {
                mSelectedList.clear();
            }
        }

        @Override
        public View getGroupView(int groupPosition, boolean isExpanded,
                View convertView, ViewGroup parent) {
            Log.d(TAG, "getGroupView");
            LayoutInflater mInflater = LayoutInflater
                    .from(ViewLockedFilesActivity.this);
            View view = convertView;
            ViewHolder viewHolder = null;
            if (view == null) {
                view = mInflater.inflate(R.layout.locked_file_group, null);
                viewHolder = new ViewHolder();
                viewHolder.line1 = (TextView) view
                        .findViewById(R.id.list_item_title);
                view.setTag(viewHolder);
            } else {
                viewHolder = (ViewHolder) view.getTag();
            }

            String title = String.format(FILE_TYPES.get(groupPosition),
                    mAllFiles.get(groupPosition).size());
            viewHolder.line1.setText(title);
            return view;
        }

        private String getGroupSubTitle(int grpId) {
            String subTitle = "";
            switch (grpId) {
            case 0:
                subTitle = getString(R.string.picture_sub_title);
                break;
            case 1:
                subTitle = getString(R.string.audio_sub_title);
                break;
            case 2:
                subTitle = getString(R.string.video_sub_title);
                break;
            case 3:
                subTitle = getString(R.string.other_sub_title);
                break;
            default:
                break;
            }
            return subTitle;
        }

        class ViewHolder {
            TextView line1;
            TextView line2;
            ImageView play_indicator;
            ImageView icon;
        }

        class LockedFilesViewHolder {
            ImageView icon;
            TextView modify;
            TextView fileSize;
            TextView title;
        }

        @Override
        public View getChildView(int groupPosition, int childPosition,
                boolean isLastChild, View convertView, ViewGroup parent) {
            Context context = parent.getContext();
            if (convertView == null) {
                LayoutInflater inflater = LayoutInflater.from(context);
                convertView = inflater.inflate(R.layout.locked_file_list_item, null);
            }
            View iconTotal = convertView.findViewById(R.id.list_item_icon);
            ImageView iconThumb = (ImageView) convertView.findViewById(R.id.icon_thumb);
            ImageView iconDefault = (ImageView) convertView.findViewById(R.id.icon_default);
            TextView date = (TextView) convertView.findViewById(R.id.list_item_date);
            TextView size = (TextView) convertView.findViewById(R.id.list_item_size);
            TextView title = (TextView) convertView.findViewById(R.id.list_item_title);

            // show no lock file text
            if (mAllFiles.get(groupPosition).size() == 0) {
                title.setText(getGroupSubTitle(groupPosition));
                title.setTextAppearance(context, android.R.attr.textAppearanceMediumInverse);
                title.setTextColor(R.color.grey_text);
                iconTotal.setVisibility(View.GONE);
                size.setVisibility(View.GONE);
                date.setVisibility(View.GONE);
                convertView.setBackgroundColor(0);
            } else {
                iconTotal.setVisibility(View.VISIBLE);
                size.setVisibility(View.VISIBLE);
                date.setVisibility(View.VISIBLE);

                FileInfo current = mAllFiles.get(groupPosition).get(childPosition);
                title.setText(current.getLockedFileShowName());
                //title.setTextColor(R.color.textColorMiddle);
                title.setTextColor(0xff101010);
                final ThumbnailCache cache = DataProtectionApplication.getThumbnailsCache(context);
                Bitmap cacheIcon = null;
                Drawable defaultIcon = null;
                switch (groupPosition) {
                    case 0:
                        // Image
                        cacheIcon = cache.get(current.getFileAbsolutePath());
                        if (cacheIcon == null) {
                            defaultIcon = getResources().getDrawable(R.drawable.ic_dataprotection_picture);
                            getThumbnail(current.getFileAbsolutePath(), false, iconThumb);
                        }
                        break;
                    case 1:
                        defaultIcon = getResources().getDrawable(R.drawable.ic_dataprotection_audio);
                        break;
                    case 2:
                        // video
                        cacheIcon = cache.get(current.getFileAbsolutePath());
                        if (cacheIcon == null) {
                            defaultIcon = getResources().getDrawable(R.drawable.ic_dataprotection_video);
                            getThumbnail(current.getFileAbsolutePath(), true, iconThumb);
                        }
                        break;
                    default:
                        // other
                        defaultIcon = getResources().getDrawable(R.drawable.ic_dataprotection_document);
                        break;
                }
                if (cacheIcon != null) {
                    iconDefault.setImageDrawable(null);
                    iconThumb.setImageBitmap(cacheIcon);
                } else {
                    iconThumb.setImageDrawable(null);
                    iconDefault.setImageDrawable(defaultIcon);
                }
                size.setText(FileUtils.sizeToString(current.getFileSize()));
                date.setText(FileUtils.longToDate(mDateFormat, current.getLastModified()));
                if (mActionMode != null && current.isChecked()) {
                    convertView.setBackgroundColor(FileListAdapter.THEME_COLOR_DEFAULT);
                } else {
                    convertView.setBackgroundColor(0);
                }
            }
            return convertView;
        }

        @Override
        public boolean isChildSelectable(int groupPosition, int childPosition) {
            if (mAllFiles.get(groupPosition).size() == 0) {
                return false;
            }
            return true;
        }

        private void getThumbnail(String path, boolean isVideo,
                ImageView iconThumb) {
            final ThumbnailAsyncTask oldTask = (ThumbnailAsyncTask) iconThumb
                    .getTag();
            if (oldTask != null) {
                oldTask.cancel(true);
                iconThumb.setTag(null);
            }

            ThumbnailAsyncTask thumbTask = new ThumbnailAsyncTask(path, isVideo, null, iconThumb);
            iconThumb.setTag(thumbTask);
            thumbTask.executeOnExecutor(mExecutor, path);
        }
    }

    private static class ThumbnailAsyncTask extends AsyncTask<String, Void, Bitmap> {
        private final String mPath;
        private final boolean mIsVideo;
        private final ImageView mIconDefault;
        private final ImageView mIconThumb;

        public ThumbnailAsyncTask(String path, boolean isVideo, ImageView iconDefault, ImageView iconThumb) {
            mPath = path;
            mIsVideo = isVideo;
            mIconDefault = iconDefault;
            mIconThumb = iconThumb;
        }

        @Override
        protected Bitmap doInBackground(String... params) {
            if (isCancelled() || mPath == null) {
                return null;
            }
            long start  = System.currentTimeMillis();
            final Context context = mIconThumb.getContext();
            Bitmap result = null;
            DrmManagerClient client = DataProtectionApplication.getCtaClient(context);
            if (mIsVideo) {
                result = CtaDrmUtils.getVideoThumbnail(client, mPath, 96);
            } else {
                result = CtaDrmUtils.getImageThumbnail(client, mPath, null);
            }

            if (result != null) {
                final ThumbnailCache cache = DataProtectionApplication
                        .getThumbnailsCache(context);
                cache.put(mPath, result);
            }
            long end  = System.currentTimeMillis();
            Log.s(TAG, "Loading thumbnail for " + mPath + " with video "
                + mIsVideo + " cost " + (end - start) + "ms");
            return result;
        }

        @Override
        protected void onPostExecute(Bitmap result) {
            if (result != null && mIconThumb.getTag() == this) {
                mIconThumb.setTag(null);
                mIconThumb.setImageBitmap(result);
            }
        }
    }

    private void updateHeader() {
        String format = getString(R.string.locked_header);
        int total = 0;
        for (int i = 0; i < mAllFiles.size(); i++) {
            total += mAllFiles.get(i).size();
        }
        mLockedTitle.setText(String.format(format, total));
    }

    private void openFile(FileInfo file) {
        if (mHasOpenedFileToShow) {
            Log.d(TAG, "has openFile, no respond to avoid open app to show with twice");
            return;
        }
        boolean canOpen = true;
        String mimeType = file.getMimeType();

        if (file.isDrmFile()) {
            DrmManagerClient client = new DrmManagerClient(this);
            mimeType = client
                    .getOriginalMimeType(file.getFileAbsolutePath());

            if (TextUtils.isEmpty(mimeType)) {
                canOpen = false;
                UiUtils.showToast(this,
                        getString(R.string.msg_unable_open_file));
            }
            client.release();
        }

        if (canOpen) {
            Intent intent = null;
            String path = file.getFileAbsolutePath();
            Uri uri = DataBaseHelper.convertPathToUri(getApplicationContext(), path);
            if (uri == null) {
                UiUtils.showToast(this,
                        getString(R.string.msg_unable_open_file));
                mHasOpenedFileToShow = false;
                return ;
            }

            DrmManagerClient client = new DrmManagerClient(this);
            String tokenKey = "file://" + path;
            String token = CtaDrmUtils.getToken(client, tokenKey);
            if (CtaDrmUtils.CTA5_ERRORTOKEN.equals(token)) {
                Log.e(TAG, "error token, setkey");
                CtaDrmUtils.setKey(client,
                                   DataProtectionLockPatternUtils.getPattern(this).getBytes());
                token = CtaDrmUtils.getToken(client, tokenKey);
                client.release();
            }

            if (mimeType != null && !TextUtils.isEmpty(mimeType)) {
                if (FileUtils.isAudio(mimeType)) {
                    intent = buildOpenCtaIntent(ACTION_VIEW, token, tokenKey,
                            "com.android.music", "com.android.music.AudioPreviewStarter");
                } else if (FileUtils.isImage(mimeType)) {
                    intent = buildOpenCtaIntent(ACTION_VIEW, token, tokenKey,
                            "com.android.gallery3d", "com.android.gallery3d.app.GalleryActivity");
                } else if (FileUtils.isVideo(mimeType)) {
                    intent = buildOpenCtaIntent(ACTION_VIEW, token, tokenKey,
                            "com.android.gallery3d", "com.android.gallery3d.app.MovieActivity");
                }
            }
            try {
                if (intent != null && uri != null) {
                    intent.setDataAndType(uri, mimeType);
                    startActivity(intent);
                    mHasOpenedFileToShow = true;
                }
            } catch (android.content.ActivityNotFoundException e) {
                UiUtils.showToast(this,
                        getString(R.string.msg_unable_open_file));
                mHasOpenedFileToShow = false;
            }
            Log.s(TAG, "ViewLockedFilesActivity open file: intent = " + intent + "token = " + token
                    + ", mimeType = " + mimeType + ", tokenKey = " + tokenKey
                    + ", uri = " + uri);
        }
    }

    private Intent buildOpenCtaIntent(String action, String token, String tokenKey,
            String packageName, String className) {
        Intent intent = new Intent(action);
        intent.setClassName(packageName, className);
        intent.putExtra("TOKEN", token);
        intent.putExtra("TOKEN_KEY", tokenKey);
        return intent;
    }

    @Override
    public void onDecryptFail(final String failFile) {

    }

}
