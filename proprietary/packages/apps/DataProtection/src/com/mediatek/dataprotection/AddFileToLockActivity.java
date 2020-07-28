package com.mediatek.dataprotection;

import java.io.File;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.drm.DrmManagerClient;
import android.graphics.Color;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.text.TextUtils;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.PopupMenu.OnMenuItemClickListener;
import android.widget.TextView;

import com.mediatek.dataprotection.DataProtectionService.FileOperationEventListener;
import com.mediatek.dataprotection.DataProtectionService.MountListener;
import com.mediatek.dataprotection.DataProtectionService.NotificationListener;
import com.mediatek.dataprotection.utils.DataBaseHelper;
import com.mediatek.dataprotection.utils.FeatureOptionsUtils;
import com.mediatek.dataprotection.utils.FileUtils;
import com.mediatek.dataprotection.utils.Log;
import com.mediatek.dataprotection.utils.UiUtils;

public class AddFileToLockActivity extends BaseActivity implements
        AdapterView.OnItemClickListener, MountListener {

    public static final String LOCKED_TITLE = "Locked";
    protected static final String TAG = "AddFileToLockActivity";
    public static final int MODE_VIEW_FILE = 1;
    public static final int MODE_SELECT_FILE = 2;

    private static final String SAFE_FOLDER_NAME = ".lockedfiles";
    private static final String SELECT_PATH_TAG = "select_card_path";
    private static final String LIST_DIALOG_TAG = "LIST_DIALOG";
    private static final String FILE_SCHEME = "file://";
    private static final int CODE_REQUEST_CONFIRM = 1;
    private static final int CODE_REQUEST_SET_PASSWORD = 2;
    protected static final int MSG_DO_MOUNTED = 1;
    protected static final int MSG_DO_EJECTED = 2;
    protected static final int MSG_DO_UNMOUNTED = 3;
    protected static final int MSG_DO_SDSWAP = 4;
    private static final String PATTERN_VERIFY = "select_pattern_verify";
    private static final int CODE_VIEW_DECRYPT_FAIL_HISTORY = 101;

    private ListView mFileList = null;
    private TextView mEmptyTextView = null;

    private String mCurrentFilePath = null;

    private ArrayList<String> mNavigationList = new ArrayList<String>();
    private int mCurrentState = MODE_VIEW_FILE;
    private Menu mMenu = null;
    private ActionMode mActionMode;
    private final ActionModeCallBack mActionModeCallBack = new ActionModeCallBack();
    private boolean mIsLoading = false;
    protected long mModifiedTime = -1;
    private View mMainView = null;
    private boolean mNeedRestore = false;
    private ListFileTask mListFileTask = null;

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName arg0, IBinder arg1) {
            mService = ((DataProtectionService.DataProtectionBinder) arg1)
                    .getService();
            initWhenHaveService();
        }

        private void initWhenHaveService() {
            MountPointManager.getInstance().init(AddFileToLockActivity.this);
            FileListAdapter adapter = new FileListAdapter(
                    AddFileToLockActivity.this, MountPointManager.getInstance()
                            .getMountPointFileInfo());
            if (null != mFileList) {
                mCurrentFilePath = MountPointManager.getInstance()
                        .getRootPath();
                mModifiedTime = new File(mCurrentFilePath).lastModified();
                mFileList.setAdapter(adapter);
                mFileList.setVisibility(View.VISIBLE);
                Log.d(TAG, "initWhenHaveService");
            }
            updateState(ViewDisplayState.BrowseFile);
            mService.registerMountListener(AddFileToLockActivity.this);
            mService.registerDataListener(mDataObserver);
            mService.registerNotificationListener(mNotificationListener);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mService = null;
        }
    };

    private ConfirmLockPatternFragment.PatternEventListener mPatternConfirmEventListener = new ConfirmLockPatternFragment.PatternEventListener() {

        @Override
        public void onPatternVerifySuccess(String pattern) {
            if (hasReadWritePermission()) {
                mHasPriviledge = true;
                mMainView.setVisibility(View.VISIBLE);
                if (mNeedRestore) {
                    mNeedRestore = false;
                    mActionMode = AddFileToLockActivity.this
                        .startActionMode(mActionModeCallBack);
                    mActionModeCallBack.updateActionMode();
                } else {
                    updateListProgressDialog(true);
                    AddFileToLockActivity.this.invalidateOptionsMenu();
                }

                showCancelTaskDialog();
            } else {
                requestReadWritePermission();
            }
        }

        @Override
        public void onCancel() {
        }

        @Override
        public void onPatternNotSet() {
            Log.d(TAG, "error if go here...");
        }
    };

    private DataProtectionService.EncryptDecryptListener mDataObserver = new DataProtectionService.EncryptDecryptListener() {

        private boolean isChildFile(File file) {
            if (mCurrentFilePath != null
                    && file.getParent().equalsIgnoreCase(mCurrentFilePath)) {
                return true;
            }
            return false;
        }

        @Override
        public void onStorageNotEnough(final List<FileInfo> files) {
            if (files == null || files.size() == 0) {
                Log.e(TAG, " onStorageNotEnough not go here..");
                return;
            }
            File firstFile = files.get(0).getFile();
            String filePath = files.get(0).getFileAbsolutePath();
            if (firstFile.getName().startsWith(".")
                    || firstFile.getName().endsWith(
                            "." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
                Log.s(TAG, " not show in this ui " + filePath);
                return;
            }
            if (filePath == null || filePath.isEmpty() || mIsLoading) {
                return;
            } else if (isChildFile(files.get(0).getFile())) {
                for (FileInfo file : files) {
                    file.setChecked(false);
                }
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        addFilesToUi(files);
                    }

                });
            }
        }

        @Override
        public void onDecryptFail(FileInfo file) {

        }

        @Override
        public void onEncryptFail(final FileInfo file) {
            String filePath = file.getFileAbsolutePath();
            if (!filePath.startsWith(mCurrentFilePath) || mIsLoading) {
                return;
            }

            Log.s(TAG, "onEncryptFail " + file.getFileAbsolutePath()
                    + " mCurrentFilePath: " + mCurrentFilePath);
            file.setChecked(false);
            mHandler.post(new Runnable() {
                @Override
                public void run() {
                    Log.d(TAG, "onEncryptFail " + file.getFileAbsolutePath());
                    if (isChildFile(file.getFile())) {
                       List<FileInfo> files = new ArrayList<FileInfo>(1);
                       files.add(file);
                       addFilesToUi(files);
                    } else {
                        Log.s(TAG, "not show, file " + file.getFileAbsolutePath()
                            + " is not in current folder");
                    }
                }

            });
        }

        @Override
        public void onEncryptCancel(final List<FileInfo> files) {
            // add files into here, if file is in current file path.
            if (files == null || files.size() == 0) {
                Log.e(TAG, " onEncryptCancel not go here..");
                return;
            }
            Log.d(TAG, "file size: " + files.size());
            String filePath = files.get(0).getFileAbsolutePath();
            String fileName = files.get(0).getFileName();
            if (filePath == null || TextUtils.isEmpty(filePath) || fileName == null || fileName.startsWith(".")
                    || fileName.endsWith("." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
                return;
            }
            if (filePath == null || filePath.isEmpty() || mIsLoading) {
                return;
            } else if (isChildFile(files.get(0).getFile())) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        addFilesToUi(files);
                    }

                });
            }
        }

        @Override
        public void onDecryptCancel(List<FileInfo> files) {
        }

        @Override
        public void onDeleteCancel(List<FileInfo> files) {
        }

        @Override
        public void onDeleteFail(FileInfo file) {
            // do nothing.
        }

    };

    private DataProtectionService.NotificationListener mNotificationListener = new NotificationListener() {

        @Override
        public void onCancel(long taskId, int resId) {
            AlertDialogFragment.showCancelTaskDialog(AddFileToLockActivity.this, mService, resId, TAG_CANCEL_DIALG, taskId);
            setTaskIdAndResId(taskId, resId);
        }

        @Override
        public void onViewDecryptFailHistory() {
            startDecryptFailActivity();
        }

    };

    private void doOnEncryptCancel(List<FileInfo> files) {
        Log.d(TAG, "AddFileToLockActivity... doOnEncryptCancel " + files.size());
        FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
        if (adapter != null) {
            adapter.addFileInfo(files);
        }
    }

    private void addFilesToUi(List<FileInfo> files) {
        Log.d(TAG, "AddFileToLockActivity... addFilesToUi " + files.size());
        FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
        if (adapter != null) {
            adapter.addFileInfo(files);
            adapter.reSort();
            updateAdapter();
        } else {
            Log.d(TAG, "error adapter is null");
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.add_file_lock_main);
        mMainView = findViewById(R.id.mainLayout);

        mFileList = (ListView) findViewById(R.id.list_main);
        mEmptyTextView = (TextView) findViewById(R.id.empty);

        MountPointManager.getInstance().init(AddFileToLockActivity.this);
        if (!isExpired() && savedInstanceState == null) {
            mHasPriviledge = getIntent().getBooleanExtra(
            DataProtectionService.KEY_INVOKE, false);
        } else {
            Log.d(TAG, "The password has been expired passed in...");
        }

        // bind service
        bindService(new Intent(this, DataProtectionService.class), mServiceConnection, BIND_AUTO_CREATE);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        mService.unRegisterMountListener(AddFileToLockActivity.this);
        mHasPriviledge = false;
        cancelLoadingFiles();
        unbindService(mServiceConnection);
        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        final Intent intent = getIntent();
        Log.d(TAG, "onResume.. HasPrivilege: " + mHasPriviledge);
        if (!mHasPriviledge) {
            // close option
            if (mActionMode != null) {
                mNeedRestore = true;
                mActionMode.finish();
                invalidateOptionsMenu();
            } else {
                updateListProgressDialog(false);
                invalidateOptionsMenu();
            }
            mMainView.setVisibility(View.GONE);
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mPatternConfirmEventListener,
                    PATTERN_VERIFY);
        } else {
            updateListProgressDialog(true);
        }

        MountPointManager.getInstance().init(AddFileToLockActivity.this);
        // reloadContent();
        if (mService != null) {
            mService.registerMountListener(this);
            mService.registerDataListener(mDataObserver);
            mService.registerNotificationListener(mNotificationListener);
        }
        mFileList.setOnItemClickListener(this);
        DataProtectionApplication.setActivityState(this, true);
        checkModifiedTimeBeforeLoading();
    }

    @Override
    protected void onPause() {
        super.onPause();
        mHasPriviledge = false;
        if (null != mService) {
            mService.unListenNotificationEvent(null);
            mService.unRegisterDataListener();
            mService.unRegistenerNotificationListener();
        }
        DataProtectionApplication.setActivityState(this, false);

    }

    @Override
    protected void onNewIntent(Intent intent) {
        intent.getBundleExtra("NOTIFY " + intent);
        Log.d(TAG,
                "onNewIntent....."
                        + intent.getExtras().getLong(
                                DataProtectionService.KEY_TASK_ID) + " "
                        + intent.getBundleExtra("NOTIFY"));
        UiUtils.showToast(this, intent.getAction());
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String permissions[],
            int[] grantResults) {
        if(permissions == null || permissions.length == 0 ||
                grantResults == null || grantResults.length == 0){
            Log.e(TAG, "onRequestPermissionsResult, Permission or grant res null");
            return;
        }
        switch (requestCode) {
            case REQUEST_READ_WRITE_STORAGE_PERMISSION_CODE:
                if (hasReadWritePermission()) {
                    mPatternConfirmEventListener.onPatternVerifySuccess(null);
                } else {
                    Intent intent = new Intent();
                    intent.putExtra(DataProtectionService.KEY_INVOKE, mHasPriviledge);
                    setResult(Activity.RESULT_CANCELED, intent);
                    finish();
                }
            break;
            default:
            //do nothing
        }
    }

    private void startDecryptFailActivity() {
        Intent intent = new Intent(this, DecryptFailHistoryActivity.class);
        intent.putExtra(DataProtectionService.KEY_INVOKE,
                mHasPriviledge);
        startActivityForResult(intent,
                CODE_VIEW_DECRYPT_FAIL_HISTORY);
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.d(TAG, "onConfigurationChanged...");
        FragmentManager mgr = getFragmentManager();
        Fragment fragment = mgr.findFragmentByTag(PATTERN_VERIFY);
        if (fragment != null) {
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mPatternConfirmEventListener,
                    PATTERN_VERIFY);
            Log.d(TAG,
                    "onConfigurationChanged... refresh pattern verify fragment");
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
            mHasPriviledge = data.getBooleanExtra(
                    DataProtectionService.KEY_INVOKE, false);
            if (!mHasPriviledge) {
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
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        FileListAdapter mAdapter = (FileListAdapter) mFileList.getAdapter();
        FileInfo selectedItemFileInfo = (FileInfo) mAdapter.getItem(position);
        if (mCurrentState == MODE_VIEW_FILE) {
            if (selectedItemFileInfo.getFile().isDirectory()) {
                mModifiedTime = selectedItemFileInfo.getLastModified();
                loadingFiles(selectedItemFileInfo.getFileAbsolutePath());
                //mIsLoading = true;
                mCurrentFilePath = selectedItemFileInfo.getFileAbsolutePath();
                if (mIsLoading) {
                    Log.d(TAG, "already loading...");
                    return;
                }
            } else {
                openFile(selectedItemFileInfo);
            }
        } else if (mCurrentState == MODE_SELECT_FILE) {
            if (selectedItemFileInfo.isDirectory()) {
                return;
            }
            boolean state = mAdapter.getItem(position).isChecked();
            mAdapter.setChecked(position, !state);
            mActionModeCallBack.updateActionMode();
            mAdapter.notifyDataSetChanged();
        }
    }

    private void updateListProgressDialog(boolean isNeedShow) {
        ProgressDialogFragment listDialogFragment = (ProgressDialogFragment) getFragmentManager()
                .findFragmentByTag(LIST_DIALOG_TAG);
        if (listDialogFragment != null && !isNeedShow) {
            FragmentManager fragmentManager = getFragmentManager();
            FragmentTransaction transaction = fragmentManager.beginTransaction();
            transaction.remove(listDialogFragment);
            transaction.commitAllowingStateLoss();
        } else if (isNeedShow) {
            //listDialogFragment.show();
            if (mListFileTask != null && mIsLoading) {
                mListFileTask.setListener(new ListListener());
            }
        }
    }

    private void updateActivityName() {
        String activityTitle = getString(R.string.app_name);
        MountPointManager mountManager = MountPointManager.getInstance();
        if (!TextUtils.isEmpty(mCurrentFilePath) && !mountManager.isRootPath(mCurrentFilePath)) {
            activityTitle = mountManager.getMountPointDescription(mCurrentFilePath);
        }
        setTitle(activityTitle);
    }

    private void openFile(FileInfo file) {
        boolean canOpen = true;
        String mimeType = file.getMimeType();

        if (file.isDrmFile()) {
            DrmManagerClient drmClient = new DrmManagerClient(this);
            mimeType = drmClient.getOriginalMimeType(file.getFileAbsolutePath());

            if (TextUtils.isEmpty(mimeType)) {
                canOpen = false;
                UiUtils.showToast(this,
                        getString(R.string.msg_unable_open_file));
            }
            drmClient.release();
        }

        if (canOpen) {
            Intent intent = new Intent(Intent.ACTION_VIEW);

            String path = file.getFileAbsolutePath();
            Uri uri = DataBaseHelper.convertPathToUri(getApplicationContext(), path);
            if (uri == null) {
                UiUtils.showToast(this,
                        getString(R.string.msg_unable_open_file));
                return ;
            }
            intent.setDataAndType(uri, mimeType);

            try {
                startActivity(intent);
            } catch (android.content.ActivityNotFoundException e) {
                UiUtils.showToast(this,
                        getString(R.string.msg_unable_open_file));
            }
        }
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.edit:
            changeToSelectFile();
            break;
        default:
            break;
        }
        return true;
    }

    private void changeToSelectFile() {
        mActionMode = startActionMode(mActionModeCallBack);
        mActionModeCallBack.updateActionMode();
        updateState(ViewDisplayState.SelectFile);
        updateAdapterMode();
    }

    private void updateAdapterMode() {
        FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
        if (adapter != null) {
            adapter.changeMode(mCurrentState);
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);

        MenuInflater inflater = getMenuInflater();
        menu.clear();
        if (null == mService) {
            return false;
        }
        inflater.inflate(R.menu.add_file_navigation_view_menu, menu);
        mMenu = menu;
        return mHasPriviledge;
    }

    @Override
    public void onBackPressed() {
        if (!mHasPriviledge) {
            ConfirmLockPatternFragment fragment = (ConfirmLockPatternFragment) getFragmentManager()
                    .findFragmentByTag(PATTERN_VERIFY);
            if (fragment != null) {
                FragmentTransaction transaction = getFragmentManager()
                        .beginTransaction();
                transaction.remove(fragment);
                transaction.commitAllowingStateLoss();
            }
            Intent intent = new Intent();
            intent.putExtra(DataProtectionService.KEY_INVOKE, mHasPriviledge);
            //setResult(Activity.RESULT_OK, intent);
            setResult(Activity.RESULT_CANCELED, intent);
            finish();
            return;
        }
        if (mListFileTask != null && mIsLoading) {
            return;
        }
        if (mCurrentFilePath != null) {
            if (mCurrentFilePath.equalsIgnoreCase(MountPointManager
                    .getInstance().getRootPath())) {
                Intent intent = new Intent();
                intent.putExtra(DataProtectionService.KEY_INVOKE, mHasPriviledge);
                setResult(Activity.RESULT_OK, intent);
                finish();
            } else if (MountPointManager.getInstance().isMountPoint(
                    mCurrentFilePath)) {
                Log.s(TAG, "onBackPressed current path: " + mCurrentFilePath);
                mCurrentFilePath = MountPointManager.getInstance()
                        .getRootPath();
                FileListAdapter adapter = new FileListAdapter(this,
                        MountPointManager.getInstance().getMountPointFileInfo());
                mFileList.setAdapter(adapter);
                updateAdapter();
            } else {
                File file = new File(mCurrentFilePath);
                if (file != null && file.exists()) {
                    String parentPath = file.getParent();
                    if (parentPath != null && !parentPath.isEmpty()) {
                        if (MountPointManager.getInstance().getRootPath()
                                .equalsIgnoreCase(parentPath)) {
                            mCurrentFilePath = parentPath;
                            FileListAdapter adapter = new FileListAdapter(this,
                                    MountPointManager.getInstance()
                                            .getMountPointFileInfo());
                            mFileList.setAdapter(adapter);
                            adapter.notifyDataSetChanged();
                        } else {
                            mCurrentFilePath = parentPath;
                            loadingFiles(parentPath);
                        }
                    }
                }
            }
        }

    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        // here must return, otherwise menu item will null(because of no inflate
        // menu
        // onCreateOptionsMenu() and this leads null pointer exception
        if (null == mService) {
            return true;
        }
        mMenu = menu;
        MenuItem editItem = mMenu.findItem(R.id.edit);
        if (editItem != null) {
            editItem.setVisible(false);
            // editItem.
        }
        MenuItem historyItem = mMenu.findItem(R.id.unlock_history);
        if (null != historyItem) {
            historyItem.setVisible(false);
        }
        updateOptionsMenu();
        return true;
    }

    private void reloadContent() {
        if (mService != null && mCurrentFilePath != null) {
            cancelLoadingFiles();
            if (MountPointManager.getInstance().isRootPath(mCurrentFilePath)) {
                FileListAdapter adapter = new FileListAdapter(
                        AddFileToLockActivity.this, MountPointManager
                                .getInstance().getMountPointFileInfo());
                mFileList.setAdapter(adapter);
                adapter.notifyDataSetChanged();
            } else if (!mIsLoading) {
                loadingFiles(mCurrentFilePath);
                // mService.listFiles(mCurrentFilePath, new ListListener());
                mIsLoading = true;
            }
        }
    }

    private void checkModifiedTimeBeforeLoading() {
        if (mService != null && mCurrentFilePath != null) {
            if (MountPointManager.getInstance().isRootPath(mCurrentFilePath)) {
                FileListAdapter adapter = new FileListAdapter(
                        AddFileToLockActivity.this, MountPointManager
                                .getInstance().getMountPointFileInfo());
                mFileList.setAdapter(adapter);
                adapter.notifyDataSetChanged();
            } else if (!mIsLoading && mModifiedTime != -1) {
                ArrayList<FileInfo> infos = new ArrayList<FileInfo>();
                ListFileTask task = new ListFileTask(getApplicationContext(),
                        infos, mCurrentFilePath, new ListListener(), mService,
                        mModifiedTime);
                task.execute();
                mListFileTask = task;
                mIsLoading = true;
            }
        }
    }

    private void updateOptionsMenu() {
        if (mMenu == null) {
            return;
        }
        MenuItem editItem = mMenu.findItem(R.id.edit);
        if (editItem != null) {
            editItem.setVisible(false);
            // editItem.
        }
        MenuItem historyItem = mMenu.findItem(R.id.unlock_history);
        if (null != historyItem) {
            historyItem.setVisible(false);
        }
        FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
        switch (mCurrentState) {
        case MODE_VIEW_FILE:
            if (adapter == null || adapter.getCount() == 0
                    || adapter.isAllItemDirectory()) {
                mMenu.findItem(R.id.edit).setVisible(false);
            } else {
                mMenu.findItem(R.id.edit).setVisible(true);
            }
            break;
        case MODE_SELECT_FILE:
            break;
        }
    }

    private class ListListener implements FileOperationEventListener {

        @Override
        public void onTaskPrepare() {
            if (!mHasPriviledge) {
                return;
            }
            ProgressDialogFragment listDialogFragment = (ProgressDialogFragment) getFragmentManager()
                    .findFragmentByTag(LIST_DIALOG_TAG);
            if (AddFileToLockActivity.this.isResumed()) {
                if (listDialogFragment == null) {
                    listDialogFragment = ProgressDialogFragment.newInstance(
                            ProgressDialog.STYLE_HORIZONTAL, -1,
                            R.string.loading, -1);

                    listDialogFragment.show(getFragmentManager(),
                            LIST_DIALOG_TAG);
                    getFragmentManager().executePendingTransactions();
                }
            }
        }

        @Override
        public void onTaskResult(int result) {
        }

        @Override
        public void onTaskResult(int result, List<FileInfo> list,
                long modifiedTime) {
            Log.s(TAG, "onTaskResult..." + result + " currentFilePath: "
                    + mCurrentFilePath + " lastModified: " + modifiedTime);
            ProgressDialogFragment listDialogFragment = (ProgressDialogFragment) getFragmentManager()
                    .findFragmentByTag(LIST_DIALOG_TAG);
            if (listDialogFragment != null) {
                listDialogFragment.dismissAllowingStateLoss();
            }
            if (result == FileOperationEventListener.ERROR_CODE_SUCCESS
                    && mFileList != null) {
                sort(list);
                FileListAdapter adapter = new FileListAdapter(
                        AddFileToLockActivity.this, list);
                mFileList.setAdapter(adapter);
                adapter.notifyDataSetChanged();
                updateAdapterMode();
                updateOptionsMenu();
                AddFileToLockActivity.this.mModifiedTime = modifiedTime != 0 ? modifiedTime
                        : mModifiedTime;
                if (list.size() == 0) {
                    mFileList.setVisibility(View.GONE);
                    mEmptyTextView.setVisibility(View.VISIBLE);
                } else {
                    mFileList.setVisibility(View.VISIBLE);
                    mEmptyTextView.setVisibility(View.GONE);
                }
            }
            updateActivityName();
            mIsLoading = false;
            mListFileTask = null;
        }

        @Override
        public void onTaskProgress(int now, int total) {
            if (mListFileTask == null || mListFileTask.isCancelled() || !mHasPriviledge) {
                return;
            }
            ProgressDialogFragment listDialogFragment = (ProgressDialogFragment) getFragmentManager()
                    .findFragmentByTag(LIST_DIALOG_TAG);
            if (AddFileToLockActivity.this.isResumed()) {
                if (listDialogFragment == null) {
                    listDialogFragment = ProgressDialogFragment.newInstance(
                            ProgressDialog.STYLE_HORIZONTAL, -1,
                            R.string.loading, -1);

                    listDialogFragment.show(getFragmentManager(),
                            LIST_DIALOG_TAG);
                    getFragmentManager().executePendingTransactions();
                }
                listDialogFragment.setProgress(now, total);
            }
        }

    }

    private void updateAdapter() {
        FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();

        if (null != adapter) {
            adapter.notifyDataSetChanged();
            Log.d(TAG, "updateAdapter " + adapter.getCount());

            if (adapter.getCount() == 0) {
                mFileList.setVisibility(View.GONE);
                mEmptyTextView.setVisibility(View.VISIBLE);
            } else {
                mFileList.setVisibility(View.VISIBLE);
                mEmptyTextView.setVisibility(View.GONE);
            }
        } else {
            Log.d(TAG, "updateAdapter " + adapter);
        }
        updateActivityName();
        invalidateOptionsMenu();
    }

    private void sort(List<FileInfo> files) {
        Collections
                .sort(files, FileInfoComparator
                        .getInstance(FileInfoComparator.SORT_BY_TYPE));
    }

    private void addToNavigationList(String path) {
        if (path == null || path.isEmpty()) {
            throw new IllegalArgumentException();
        }
        int itemNum = mNavigationList.size();
        if (itemNum > 0) {
            String lastPath = mNavigationList.get(itemNum - 1);
            if (lastPath.equalsIgnoreCase(path)) {
                return;
            }
        }
        if (itemNum >= 20) {
            mNavigationList.remove(0);
        }
        mNavigationList.add(path);
    }

    private String getLastOperationFolderPath() {
        String path = null;
        int itemNum = mNavigationList.size();
        if (itemNum > 0) {
            path = mNavigationList.remove(itemNum - 1);
        }
        return path;
    }

    protected class ActionModeCallBack implements ActionMode.Callback,
            OnMenuItemClickListener {

        private PopupMenu mSelectPopupMenu = null;
        private boolean mSelectedAll = true;
        private Button mTextSelect = null;
        private Menu mMenu = null;

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
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
            menuInflater.inflate(R.menu.add_file_edit_view_menu, menu);
            mMenu = menu;

            FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
            int selectedFileNum = adapter.getCheckedItemsCount();
            MenuItem editItem = menu.findItem(R.id.edit);
            if (editItem != null && selectedFileNum == 0) {
                editItem.setEnabled(false);
                Log.d(TAG, "onPrepareActionMode set enable false");
            } else if (editItem != null) {
                editItem.setEnabled(true);
                Log.d(TAG, "onPrepareActionMode set enable true");
            }

            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {

            FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
            int selectedFileNum = adapter.getCheckedItemsCount();
            MenuItem editItem = menu.findItem(R.id.edit);
            if (editItem != null && selectedFileNum == 0) {
                editItem.setEnabled(false);
                Log.d(TAG, "onPrepareActionMode set enable false");
            } else if (editItem != null) {
                editItem.setEnabled(true);
                Log.d(TAG, "onPrepareActionMode set enable true");
            }
            String selectString = getResources().getString(
                    R.string.selected_format);
            selectString = String.format(selectString, selectedFileNum);
            mTextSelect.setText(selectString);
            return true;
        }

        @Override
        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {
            switch (item.getItemId()) {
            case R.id.edit:
                onClickEdit();
                break;
            default:
                break;

            }
            return false;
        }

        private void onClickEdit() {
            if (MODE_SELECT_FILE == mCurrentState) {
                startEncrypt();
                mActionMode.finish();
                FileListAdapter adapter = (FileListAdapter) mFileList
                        .getAdapter();
                if (adapter != null && adapter.getCheckedItemsCount() == 0) {
                    mEmptyTextView.setVisibility(View.VISIBLE);
                }
                updateAdapter();
            }
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            if (!mNeedRestore && (mCurrentState == MODE_SELECT_FILE)) {
                updateState(ViewDisplayState.BrowseFile);
                // force invalidate menu
                invalidateOptionsMenu();
                updateAdapterMode();
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
                    AddFileToLockActivity.this, anchorView);
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
            FileListAdapter mAdapter = (FileListAdapter) mFileList.getAdapter();
            int selectedCount = mAdapter.getCheckedItemsCount();
            if (mAdapter.getCount() == 0) {
                menu.findItem(R.id.select).setEnabled(false);
            } else {
                menu.findItem(R.id.select).setEnabled(true);
            }
            if (mAdapter.getCount() != selectedCount) {
                menu.findItem(R.id.select).setTitle(R.string.select_all);
                mSelectedAll = true;
            } else {
                menu.findItem(R.id.select).setTitle(R.string.deselect_all);
                mSelectedAll = false;
            }
        }

        @Override
        public boolean onMenuItemClick(MenuItem menuItem) {
            FileListAdapter mAdapter = (FileListAdapter) mFileList.getAdapter();
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
            FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
            int selectedFileNum = adapter.getCheckedItemsCount();
            MenuItem editItem = mMenu.findItem(R.id.edit);
            if (editItem != null && selectedFileNum == 0) {
                editItem.setEnabled(false);
                Log.d(TAG, "disable menu item");
            } else if (editItem != null) {
                editItem.setEnabled(true);
                Log.d(TAG, "enable menu item");
            }
            String selectString = getResources().getString(
                    R.string.selected_format);
            selectString = String.format(selectString, selectedFileNum);
            mTextSelect.setText(selectString);

            mActionModeCallBack.updateSelectPopupMenu();
            if (mActionMode != null) {
                mActionMode.invalidate();
            }
        }
    } // action mode callback

    private void startEncrypt() {
        FileListAdapter adapter = (FileListAdapter) mFileList.getAdapter();
        List<FileInfo> selectedFile = adapter
                .getCheckedFileInfoItemsListAndRemove();
        if (null != mService) {
            UiUtils.showToast(getApplicationContext(),
                    getString(R.string.start_lock));
            mService.encrypt(selectedFile);
        }
        adapter.notifyDataSetChanged();
    }

    private void updateState(ViewDisplayState state) {
        mFileList.setVisibility(state.listViewVisible);
        mEmptyTextView.setVisibility(state.emptyViewVisible);
        mCurrentState = state.editMode;
        updateAdapterMode();
        updateOptionsMenu();
    }

    enum ViewDisplayState {

        BrowseFile(MODE_VIEW_FILE, View.GONE, View.VISIBLE, View.GONE), SelectFile(
                MODE_SELECT_FILE, View.GONE, View.VISIBLE, View.GONE);

        ViewDisplayState(int editState, int isHeaderVisible,
                int isListViewVisible, int isEmptyViewVisible) {
            this.editMode = editState;
            this.headerVisible = isHeaderVisible;
            this.listViewVisible = isListViewVisible;
            this.emptyViewVisible = isEmptyViewVisible;
        }

        final int editMode;
        final int headerVisible;
        final int listViewVisible;
        final int emptyViewVisible;
    }

    private Handler mHandler = new Handler();

    public void onMounted(String mountPoint) {
        Log.e(TAG, "onMounted " + mountPoint);
        String path = null;
        if (!TextUtils.isEmpty(mountPoint)) {
            if (mountPoint.startsWith(FILE_SCHEME)) {
                path = mountPoint.substring(FILE_SCHEME.length());
            }
        } else {
            Log.e(TAG, "mountPoint is empty " + mountPoint);
        }
        if (!TextUtils.isEmpty(path)) {
            doSdcardMounted(path);
        }
    }

    private void doSdcardMounted(String path) {
        if (MountPointManager.getInstance().isRootPath(mCurrentFilePath)) {
            reloadContent();
        }
    }

    public void onUnMounted(String unMountPoint) {
        Log.e(TAG, "onMounted " + unMountPoint);
        String path = null;
        if (!TextUtils.isEmpty(unMountPoint)) {
            if (unMountPoint.startsWith(FILE_SCHEME)) {
                path = unMountPoint.substring(FILE_SCHEME.length());
            }
        } else {
            Log.e(TAG, "mountPoint is empty " + unMountPoint);
        }
        if (!TextUtils.isEmpty(path)) {
            doSdcardUnMounted(path);
        }
    }

    private void doSdcardUnMounted(String path) {
        Log.d(TAG, "doSdcardUnMounted: " + path);
        if (!TextUtils.isEmpty(path) && !TextUtils.isEmpty(mCurrentFilePath)
                && mCurrentFilePath.startsWith(path)) {
            mCurrentFilePath = MountPointManager.getInstance().getRootPath();
            reloadContent();
        } else if (!TextUtils.isEmpty(mCurrentFilePath)
                && MountPointManager.getInstance().isRootPath(mCurrentFilePath)) {
            reloadContent();
        }
    }

    public void onEjected(String unMountPoint) {
        Log.e(TAG, "onMounted " + unMountPoint);
        String path = null;
        if (!TextUtils.isEmpty(unMountPoint)) {
            if (unMountPoint.startsWith(FILE_SCHEME)) {
                path = unMountPoint.substring(FILE_SCHEME.length());
            }
        } else {
            Log.e(TAG, "mountPoint is empty " + unMountPoint);
        }
        if (!TextUtils.isEmpty(path)) {
            doSdcardEjected();
        }
    }

    private void doSdcardEjected() {

    }

    private void loadingFiles(String path) {
        if (mService == null || !mHasPriviledge) {
            return;
        }
        ArrayList<FileInfo> infos = new ArrayList<FileInfo>();
        ListFileTask task = new ListFileTask(getApplicationContext(), infos,
                path, new ListListener(), mService);
        task.execute();
        mListFileTask = task;
        mIsLoading = true;
    }

    private void cancelLoadingFiles() {
        if (mListFileTask != null) {
            mListFileTask.cancel(true);
            Log.d(TAG, "cancelLoadingFiles ...");
        }
        updateListProgressDialog(false);
        mIsLoading = false;
    }

    public void onSdSwap() {
        Log.e(TAG, "onSdSwap... ");
        mCurrentFilePath = MountPointManager.getInstance().getRootPath();
        reloadContent();
    }
}
