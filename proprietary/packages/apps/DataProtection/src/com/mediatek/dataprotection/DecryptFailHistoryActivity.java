package com.mediatek.dataprotection;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.dataprotection.AlertDialogFragment.DynamicAlertDialogFragment;
import com.mediatek.dataprotection.AlertDialogFragment.DynamicAlertDialogFragmentBuilder;
import com.mediatek.dataprotection.AlertDialogFragment.PatternInputDialogFragment;
import com.mediatek.dataprotection.AlertDialogFragment.PatternInputDialogFragmentBuilder;
import com.mediatek.dataprotection.AlertDialogFragment.PatternInputDialogFragment.PasswordInputListener;
import com.mediatek.dataprotection.DataProtectionService.NotificationListener;
import com.mediatek.dataprotection.utils.FileUtils;
import com.mediatek.dataprotection.utils.UiUtils;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.ComponentName;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.graphics.Color;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.os.Handler;
import android.os.IBinder;
import android.os.SystemClock;
import android.text.TextUtils;
import android.view.ActionMode;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.BaseAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.PopupMenu;
import android.widget.TextView;
import android.widget.PopupMenu.OnMenuItemClickListener;
import com.mediatek.dataprotection.utils.Log;

public class DecryptFailHistoryActivity extends BaseActivity implements
        AdapterView.OnItemClickListener {

    private static final String TAG = "DecryptFailHistoryActivity";
    private static final String KEY_PATTERN_REQUEST_DIALOG = "KEY_PATTERN_REQUEST";
    private static final String TAG_PATTERN_VERIFY = "decrypt_fail_pattern_verify";
    protected static final String TAG_CANCEL_DIALOG = "cancel_dialog";
    private static final String KEY_ALERT_DIALOG = "key_alert_user";
    private ListView mListView = null;
    private DecryptFailAdapter mAdapter = null;
    private View mMainView = null;
    private TextView mEmptyView = null;
    private TextView mHeaderText = null;
    private boolean mHasPriviledge = false;
    private DataProtectionService mService = null;
    private boolean mNeedRestorePatternInput = false;
    private ArrayList<FileInfo> mDecryptFailHistory = new ArrayList<FileInfo>();
    private boolean mNeedRestore = false;
    private ActionMode mActionMode;
    private String mDateFormat = null;
    private ActionModeCallBack mActionModeCallBack = new ActionModeCallBack();
    private CountDownTimer mCountdownTimer;
    private long mRemainTime = 0;
    private boolean mNeedRestoreAlertDialog = false;
    private ServiceConnection mServiceConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = ((DataProtectionService.DataProtectionBinder) service)
                    .getService();
            updateAdapter();
            mService.registerDataListener(mDataObserver);
            mService.registerNotificationListener(mNotificationListener);
            long deadline = mService.getTryDecryptLockOutDeadline();
            if (deadline != 0 && mActionModeCallBack != null) {
                mActionModeCallBack.handleAttemptLockout(deadline);
            }
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
                    mActionMode = startActionMode(mActionModeCallBack);
                    mActionModeCallBack.updateActionMode();
                    if (mNeedRestoreAlertDialog) {
                        updateAlertDialog(true);
                        mNeedRestoreAlertDialog = false;
                    }
                } else {
                    invalidateOptionsMenu();
                }
                if (mNeedRestorePatternInput) {
                    restorePatternInputFragment();
                    mNeedRestorePatternInput = false;
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

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        setContentView(R.layout.decrypt_fail_history);
        mListView = (ListView) findViewById(R.id.list_view);
        mAdapter = new DecryptFailAdapter();

        mMainView = findViewById(R.id.mainLayout);
        mListView.setOnItemClickListener(this);
        mListView.setAdapter(mAdapter);
        mEmptyView = (TextView) findViewById(android.R.id.empty);
        mHeaderText = (TextView) findViewById(R.id.unlocking_fail_header);
        //setTitle(R.string.decrypt_fail_history);
        mDateFormat = getString(R.string.date_format);
        if (!isExpired() && savedInstanceState == null) {
            mHasPriviledge = getIntent().getBooleanExtra(
            DataProtectionService.KEY_INVOKE, false);
        } else {
            Log.d(TAG, "The password has been expired passed in...");
        }
        // bind service
        bindService(new Intent(this, DataProtectionService.class),
                mServiceConnection, BIND_AUTO_CREATE);
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, "onDestroy");
        unbindService(mServiceConnection);
        super.onDestroy();
    }

    @Override
    public void onResume() {
        super.onResume();
        Log.d(TAG, "onResume " + mHasPriviledge);
        if (!mHasPriviledge) {
            mMainView.setVisibility(View.GONE);
            if (mActionMode != null) {
                mNeedRestore = true;
                updateAlertDialog(false);
                mActionMode.finish();
                invalidateOptionsMenu();
            } else {
                invalidateOptionsMenu();
            }
            invalidateOptionsMenu();
            removePatternInputFragment();
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mPatternConfirmEventListener,
                    TAG_PATTERN_VERIFY);
        }
        if (mService != null) {
            mService.registerNotificationListener(mNotificationListener);
            mService.registerDataListener(mDataObserver);
            long deadline = mService.getTryDecryptLockOutDeadline();
            if (deadline != 0 && mActionModeCallBack != null) {
                mActionModeCallBack.handleAttemptLockout(deadline);
            }
        }
        DataProtectionApplication.setActivityState(this, true);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mHasPriviledge = false;
        DataProtectionApplication.setActivityState(this, false);
        mService.unRegisterDataListener();
        mService.unRegistenerNotificationListener();
        if (mCountdownTimer != null) {
            mCountdownTimer.cancel();
            mCountdownTimer = null;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        Log.d(TAG, " onCreateOptionsMenu MENU ");

        MenuInflater inflater = getMenuInflater();
        menu.clear();
        if (null == mService) {
            return false;
        }
        inflater.inflate(R.menu.decrypt_fail_history_menu, menu);
        return mHasPriviledge;
    }

    @Override
    public void onConfigurationChanged(Configuration newConfig) {
        super.onConfigurationChanged(newConfig);
        Log.d(TAG, "onConfigurationChanged...");
        FragmentManager mgr = getFragmentManager();
        Fragment fragment = mgr.findFragmentByTag(TAG_PATTERN_VERIFY);
        if (fragment != null) {
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mPatternConfirmEventListener,
                    TAG_PATTERN_VERIFY);
            Log.d(TAG,
                    "onConfigurationChanged... refresh pattern verify fragment");
        } else if ((fragment = mgr.findFragmentByTag(KEY_PATTERN_REQUEST_DIALOG)) != null) {
            showPatternRequestDialog(true);
        }
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

    private void removePatternInputFragment() {
        PatternInputDialogFragment fragment = (PatternInputDialogFragment) getFragmentManager()
                .findFragmentByTag(KEY_PATTERN_REQUEST_DIALOG);
        if (fragment != null) {
            FragmentTransaction transaction = getFragmentManager()
                    .beginTransaction();
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
            mNeedRestorePatternInput = true;
        }
    }

    private void restorePatternInputFragment() {
        showPatternRequestDialog(false);
        mNeedRestorePatternInput = false;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.edit:
            mActionMode = startActionMode(mActionModeCallBack);
            mActionModeCallBack.updateActionMode();
            break;
        default:
            break;
        }
        return true;
    }

    @Override
    public void onBackPressed() {

        ConfirmLockPatternFragment fragment = (ConfirmLockPatternFragment) getFragmentManager()
                .findFragmentByTag(TAG_PATTERN_VERIFY);
        if (fragment != null) {
            FragmentTransaction transaction = getFragmentManager()
                    .beginTransaction();
            transaction.remove(fragment);
            transaction.commitAllowingStateLoss();
        }
        Intent intent = new Intent();
        intent.putExtra(DataProtectionService.KEY_INVOKE, mHasPriviledge);
        if (mHasPriviledge) {
            setResult(RESULT_OK, intent);
        } else {
            setResult(Activity.RESULT_CANCELED, intent);
        }
        super.onBackPressed();
    }

    private void updateAlertDialog(boolean isShow) {
        AlertDialogFragment listDialogFragment = (AlertDialogFragment) getFragmentManager()
                .findFragmentByTag(KEY_ALERT_DIALOG);
        if (!isShow && listDialogFragment != null) {
            FragmentTransaction transaction = getFragmentManager().beginTransaction();
            transaction.remove(listDialogFragment);
            transaction.commitAllowingStateLoss();
            mNeedRestoreAlertDialog = true;
        }

        if (isShow && listDialogFragment == null && mRemainTime > 0) {
            mActionModeCallBack.showDynamicAlertDialog();
        }
    }

    private void updateAdapter() {
        if (mService != null) {
            Log.d(TAG, "updateAdapter: count = " + mDecryptFailHistory.size());
            // mDecryptFailHistory = mService.getDecryptFailHistory();
            ArrayList<String> history = mService.getDecryptFailHistory();
            mDecryptFailHistory.clear();
            for (int i = 0; i < history.size(); i++) {
                String mergerString = history.get(i);
                long modifyDate = 0;
                long size = 0;
                String path = null;
                try {
                     modifyDate = Long.parseLong(mergerString.substring(0, mergerString.indexOf(File.separator)));
                     String mergeString = mergerString.substring(mergerString.indexOf(File.separator) + 1);
                     size = Long.parseLong(mergeString.substring(0, mergeString.indexOf(File.separator)));
                     path = mergeString.substring(mergeString.indexOf(File.separator));
                     //path = mergeString.substring()
                } catch (NumberFormatException e) {
                    modifyDate = System.currentTimeMillis();
                    size = 0;
                }
                mDecryptFailHistory.add(new FileInfo(path, modifyDate, size));
            }
            mEmptyView
                    .setVisibility(mDecryptFailHistory.isEmpty() ? View.VISIBLE
                            : View.GONE);
            mHeaderText.setText(String.format(getString(R.string.unlocking_fail_header_text), mAdapter.getCount()));
            mAdapter.notifyDataSetChanged();
            // mService.listenUnlockUpdate(mListerner);
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
            long id) {
        Log.d(TAG, "onItemClick: " + position);
        FileInfo sel = (FileInfo) mAdapter.getItem(position);
        if (sel != null) {
            if (mActionMode != null) {
                boolean check = sel.isChecked();
                sel.setChecked(!check);
                mActionModeCallBack.updateActionMode();
                mAdapter.notifyDataSetChanged();
            }
        }
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
            menuInflater.inflate(R.menu.decrypt_fail_history_edit_menu, menu);
            mMenu = menu;
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {

            int selectedFileNum = mAdapter.getCheckedItemsCount();
            MenuItem editItem = menu.findItem(R.id.decrypt_try);
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
            case R.id.decrypt_try:
                onClickEdit();
                break;
            default:
                break;

            }
            return false;
        }

        private void onClickEdit() {
            int tryTime = mService != null ? mService.queryUserAttemptTimes() : 0;
            Log.d(TAG, "onClickEdit tryTime: " + tryTime);
            if (tryTime >= DataProtectionService.TIMES_USER_ATTEMPT_LIMIT && mCountdownTimer == null) {
                mRemainTime = 30;
                //showAlertUserDialog();
                showDynamicAlertDialog();
                long deadline = mService.setTryDecryptLockOutDeadline();
                handleAttemptLockout(deadline);
                //mService.startAttemptTimeCounter();
            } else if (tryTime < DataProtectionService.TIMES_USER_ATTEMPT_LIMIT) {
                 showPatternRequestDialog(false);
                 mService.updateUserAttemptTimes(tryTime + 1);
            } else if (tryTime >= DataProtectionService.TIMES_USER_ATTEMPT_LIMIT && mCountdownTimer != null) {
                showDynamicAlertDialog();
            }
        }

        protected void handleAttemptLockout(long deadline) {
            long elapsedRealtime = SystemClock.elapsedRealtime();
            mCountdownTimer = new CountDownTimer(
                    deadline - elapsedRealtime,
                    1000) {

                @Override
                public void onTick(long millisUntilFinished) {
                    mRemainTime = millisUntilFinished / 1000;
                    DynamicAlertDialogFragment listDialogFragment = (DynamicAlertDialogFragment) getFragmentManager()
                            .findFragmentByTag(KEY_ALERT_DIALOG);
                    if (listDialogFragment != null && mRemainTime > 0) {
                        //showAlertUserDialog();
                        String message = String.format(getString(R.string.dialog_input_too_many_header), mRemainTime);
                        listDialogFragment.setContentText(message);
                    }
                    Log.d(TAG, "currentTime " + millisUntilFinished + " remain " + mRemainTime);
                }

                @Override
                public void onFinish() {
                     if (mService != null) {
                         mService.updateUserAttemptTimes(0);
                     }
                     mRemainTime = 0;
                     mCountdownTimer = null;
                     updateAlertDialog(false);
                }
            } .start();
        }

        private void showDynamicAlertDialog() {
            DynamicAlertDialogFragment listDialogFragment = (DynamicAlertDialogFragment) getFragmentManager()
                    .findFragmentByTag(KEY_ALERT_DIALOG);
            final DialogInterface.OnClickListener cListener = new DialogInterface.OnClickListener() {

                @Override
                public void onClick(DialogInterface dialog, int which) {
                    Log.d(TAG, "onClick button clicked " + which);
                    dialog.dismiss();
                }
            };
            if (DecryptFailHistoryActivity.this.isResumed()) {
                if (listDialogFragment == null) {
                    DynamicAlertDialogFragmentBuilder builder = new DynamicAlertDialogFragmentBuilder();
                    String message = String.format(getString(R.string.dialog_input_too_many_header), mRemainTime);
                    builder.setDefaultMessage(message);
                    builder.setCancelable(true);
                    builder.setCancelTitle(R.string.btn_ok);
                    Log.d(TAG, "message: " + message + " time " + mRemainTime);
                    listDialogFragment = (DynamicAlertDialogFragment) builder.setLayout(R.layout.dynamic_alert_view).create();

                    listDialogFragment
                            .show(getFragmentManager(), KEY_ALERT_DIALOG);
                    boolean ret = getFragmentManager().executePendingTransactions();
                }
            }
        }

        @Override
        public void onDestroyActionMode(ActionMode mode) {
            // switchToNavigationView();
            if (mNeedRestore) {
                // changeToViewNormalFile();
            } else {
                mAdapter.setAllItemChecked(false);
                mAdapter.notifyDataSetChanged();
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
                    DecryptFailHistoryActivity.this, anchorView);
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
            int selectedFileNum = mAdapter.getCheckedItemsCount();
            MenuItem editItem = mMenu.findItem(R.id.decrypt_try);
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

    private class DecryptFailAdapter extends BaseAdapter {

        private static final int THEME_COLOR_DEFAULT = 0x7F33b5e5;
        private static final int DEFAULT_PRIMARY_TEXT_COLOR = Color.WHITE;

        @Override
        public int getCount() {
            if (mDecryptFailHistory != null) {
                return mDecryptFailHistory.size();
            }
            return 0;
        }

        public List<FileInfo> getCheckedFileInfoItemsListAndRemove() {
            List<FileInfo> fileInfoCheckedList = new ArrayList<FileInfo>();
            for (int i = 0; i < mDecryptFailHistory.size(); i++) {

                if (mDecryptFailHistory.get(i).isChecked()) {
                    fileInfoCheckedList.add(mDecryptFailHistory.get(i));
                    mDecryptFailHistory.get(i).setChecked(false);
                }
            }
            mDecryptFailHistory.removeAll(fileInfoCheckedList);
            return fileInfoCheckedList;
        }

        public void setAllItemChecked(boolean checked) {
            for (FileInfo info : mDecryptFailHistory) {
                if (!info.isDirectory()) {
                    info.setChecked(checked);
                } else {
                    info.setChecked(false);
                }
            }
            notifyDataSetChanged();
        }

        public int getCheckedItemsCount() {
            int count = 0;
            for (FileInfo fileInfo : mDecryptFailHistory) {
                if (fileInfo.isChecked()) {
                    count++;
                }
            }
            return count;
        }

        @Override
        public Object getItem(int arg0) {
            if (mDecryptFailHistory != null) {
                return mDecryptFailHistory.get(arg0);
            }
            return null;
        }

        @Override
        public long getItemId(int arg0) {
            if (mDecryptFailHistory != null) {
                FileInfo file = mDecryptFailHistory.get(arg0);
                if (file != null) {
                    return file.hashCode();
                }
            }
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                final LayoutInflater inflater = LayoutInflater
                        .from(DecryptFailHistoryActivity.this);
                convertView = inflater.inflate(
                        R.layout.decrypt_fail_history_list_item, null);
            }
            TextView title = (TextView) convertView
                    .findViewById(R.id.list_item_title);
            TextView date = (TextView) convertView
                    .findViewById(R.id.list_item_date);
            TextView size = (TextView) convertView.findViewById(R.id.list_item_size);
            FileInfo currentItem = mDecryptFailHistory.get(position);
            title.setText(getFileName(currentItem.getFileAbsolutePath()));
            size.setText(FileUtils.sizeToString(currentItem.getFileSize()));
            date.setText(FileUtils.longToDate(mDateFormat,
                    currentItem.getLastModified()));
            if (currentItem.isChecked()) {
                convertView.setBackgroundColor(THEME_COLOR_DEFAULT);
            } else {
                convertView.setBackgroundColor(Color.TRANSPARENT);
            }
            return convertView;
        }
    }

    private String getFileName(String path) {
        String fileName = (path == null ? "" : path.toString());
        int idx = fileName.lastIndexOf(File.separator);
        if (idx >= 0) {
            fileName = fileName.substring(idx + 1);
        }
        if (fileName.startsWith(".")) {
            fileName = fileName.substring(1);
        }
        if (fileName.endsWith("." + FileInfo.ENCRYPT_FILE_EXTENSION)) {
            fileName = fileName.substring(0, fileName.indexOf("."
                + FileInfo.ENCRYPT_FILE_EXTENSION));
        }
        return fileName;
    }

    private void tryToDecrypt(String key) {
        List<FileInfo> selectedFiles = null;
        if (mAdapter != null) {
            selectedFiles = mAdapter.getCheckedFileInfoItemsListAndRemove();
        }
        if (null != selectedFiles && null != mService) {
            UiUtils.showToast(getApplicationContext(),
                    getString(R.string.start_unlock));
            mService.tryToDecryptFiles(key, selectedFiles);
        }
        if (mAdapter != null) {
            mAdapter.notifyDataSetChanged();
        }
        if (mActionMode != null) {
            mActionMode.finish();
        }
    }

    private DataProtectionService.EncryptDecryptListener mDataObserver =
        new DataProtectionService.EncryptDecryptListener() {

        @Override
        public void onStorageNotEnough(List<FileInfo> file) {

        }

        @Override
        public void onEncryptCancel(List<FileInfo> files) {

        }

        @Override
        public void onDecryptCancel(List<FileInfo> files) {

        }

        @Override
        public void onDeleteCancel(List<FileInfo> files) {

        }

        @Override
        public void onDecryptFail(final FileInfo file) {
            if (file != null && !TextUtils.isEmpty(file.getFileAbsolutePath())) {
                Handler handler = new Handler(getMainLooper());
                handler.post(new Runnable() {

                    @Override
                    public void run() {
                        if (DecryptFailHistoryActivity.this.isResumed()) {
                            updateAdapter();
                        }
                    }

                });
            }
        }

        @Override
        public void onEncryptFail(FileInfo file) {

        }

        @Override
        public void onDeleteFail(FileInfo file) {

        }

    };
    private DataProtectionService.NotificationListener mNotificationListener
        = new NotificationListener() {

        @Override
        public void onCancel(long taskId, int resId) {
            AlertDialogFragment.showCancelTaskDialog(DecryptFailHistoryActivity.this, mService,
                resId, TAG_CANCEL_DIALG, taskId);
            setTaskIdAndResId(taskId, resId);
        }

        @Override
        public void onViewDecryptFailHistory() {
        }

    };
    private void showPatternRequestDialog(boolean alwaysCreate) {
        PatternInputDialogFragment listDialogFragment =
            (PatternInputDialogFragment) getFragmentManager()
                .findFragmentByTag(KEY_PATTERN_REQUEST_DIALOG);
        if (alwaysCreate && listDialogFragment != null) {
            FragmentTransaction transaction = getFragmentManager()
                    .beginTransaction();
            transaction.remove(listDialogFragment);
            transaction.commitAllowingStateLoss();
            listDialogFragment = null;
        }
        if (DecryptFailHistoryActivity.this.isResumed()) {
            if (listDialogFragment == null) {
                PatternInputDialogFragmentBuilder builder = new PatternInputDialogFragmentBuilder();
                builder.setCancelable(true);
                builder.setCancelTitle(R.string.btn_cancel);
                // builder.set
                // builder.setUnlockFileName(mDecryptFailFileName);
                // builder.setUnlockFileName(failName);
                listDialogFragment = builder.create();
                listDialogFragment
                        .setOnPatternInput(new PasswordInputListener() {

                            @Override
                            public void onPatternDetect(String password) {
                                if (password != null && mService != null) {
                                    Log.d(TAG,
                                            "onPatternDetect user has input key");
                                    tryToDecrypt(password);
                                    if (mActionMode != null) {
                                        mActionMode.finish();
                                    }
                                    updateAdapter();
                                    mNeedRestorePatternInput = false;
                                }
                            }

                        });

                listDialogFragment.show(getFragmentManager(),
                        KEY_PATTERN_REQUEST_DIALOG);
                boolean ret = getFragmentManager().executePendingTransactions();
            }
        }
    }
}

