package com.mediatek.dataprotection;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import com.mediatek.dataprotection.AlertDialogFragment.AlertDialogFragmentBuilder;
import com.mediatek.dataprotection.DataProtectionService.NotificationListener;
import com.mediatek.dataprotection.utils.FileUtils;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.FragmentTransaction;
import android.content.ComponentName;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.ServiceConnection;
import android.content.res.Configuration;
import android.os.Bundle;
import android.os.IBinder;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;
import com.mediatek.dataprotection.utils.Log;

public class UnlockHistoryActivity extends BaseActivity {

    private static final int CODE_REQUEST_CONFIRM = 300;
    private static final String TAG = "UnlockHistoryActivity";
    private static final String TAG_PATTERN_VERIFY = "verify_pattern_unlock_history";
    private static final String SHOW_DETAIL = "show_detail_unlock_history";
    private static final int CODE_VIEW_DECRYPT_FAIL_HISTORY = 100;
    private ListView mListView = null;
    private List<String> mUnlockHistoryList = new ArrayList<String>();
    private UnlockedFilesAdapter mAdapter = null;
    private boolean mHasPriviledge = false;
    private DataProtectionService mService;
    private View mMainView = null;
    private View mEmptyView = null;
    private MenuItem mMenuItemClear = null;

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            mService = ((DataProtectionService.DataProtectionBinder) service).getService();
            updateAdapter();
            mService.registerNotificationListener(mNotificationListener);
        }

        @Override
        public void onServiceDisconnected(ComponentName arg0) {
            mService = null;
        }
    };

    // pattern logic <<<
    private DataProtectionService.NotificationListener mNotificationListener = new NotificationListener() {

        @Override
        public void onCancel(long taskId, int resId) {
            AlertDialogFragment.showCancelTaskDialog(UnlockHistoryActivity.this, mService, resId, TAG_CANCEL_DIALG, taskId);
            setTaskIdAndResId(taskId, resId);
        }

        @Override
        public void onViewDecryptFailHistory() {
            startDecryptFailActivity();
        }

    };
    private ConfirmLockPatternFragment.PatternEventListener mPatternConfirmEventListener =
            new ConfirmLockPatternFragment.PatternEventListener() {

        @Override
        public void onPatternVerifySuccess(String pattern) {
            if (hasReadWritePermission()) {
                mHasPriviledge = true;
                mMainView.setVisibility(View.VISIBLE);
                invalidateOptionsMenu();

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
    // pattern logic <<<

    private OnItemClickListener mItemListener = new OnItemClickListener() {
        @Override
        public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
            String history = mUnlockHistoryList.get(mAdapter.getCount() - 1 - position);
            showDetailDialog(history.substring(history.indexOf("/")));
            Log.s(TAG, "show in dialog with detail path " + history);
        }
    };

    private OnUnlockHistoryUpdate mListerner = new OnUnlockHistoryUpdate() {
        @Override
        public void onUpdated() {
            runOnUiThread(new Runnable() {
                public void run() {
                    updateAdapter();
                }
            });
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate savedInstanceState = " + savedInstanceState);
        setContentView(R.layout.unlock_history);
        mListView = (ListView) findViewById(R.id.list_view);
        mAdapter = new UnlockedFilesAdapter();

        mMainView = findViewById(R.id.mainLayout);
        mListView.setOnItemClickListener(mItemListener);
        mListView.setAdapter(mAdapter);
        mEmptyView = findViewById(android.R.id.empty);
        setTitle(R.string.unlock_history_title);
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
        unbindService(mServiceConnection);
        super.onDestroy();
    }

    @Override
    public void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();
        if (!mHasPriviledge) {
            mMainView.setVisibility(View.GONE);
            invalidateOptionsMenu();
            ConfirmLockPatternFragment.show(getFragmentManager(),
                    R.id.pattern_fragment, mPatternConfirmEventListener, TAG_PATTERN_VERIFY);
        } else {
            updateAdapter();
        }
        if (mService != null) {
            mService.registerNotificationListener(mNotificationListener);
        }

        DataProtectionApplication.setActivityState(this, true);
    }

    @Override
    protected void onPause() {
        super.onPause();
        mHasPriviledge = false;
        DataProtectionApplication.setActivityState(this, false);
        // Dismiss dialog when activity and stop listen unlock notify when go to background
        AlertDialogFragment detailDialog = (AlertDialogFragment) getFragmentManager().findFragmentByTag(SHOW_DETAIL);
        if (detailDialog != null) {
            detailDialog.dismiss();
            Log.d(TAG, "onPause dismiss detailDialog");
        }
        if (mService != null) {
            mService.listenUnlockUpdate(null);
            mService.unListenNotificationEvent(null);
            mService.unRegistenerNotificationListener();
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
                Log.d("EYE", "onRequestPermission Result");
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
            Log.d(TAG, "onConfigurationChanged... refresh pattern verify fragment");
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        super.onCreateOptionsMenu(menu);
        Log.d(TAG, " onCreateOptionsMenu MENU ");

        MenuInflater inflater = getMenuInflater();
        menu.clear();
        inflater.inflate(R.menu.unlock_history_menu, menu);
        mMenuItemClear = menu.findItem(R.id.clear);
        return mHasPriviledge;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        Log.d(TAG, "onPrepareOptionsMenu mService = " + mService);
        if (null == mService) {
            return true;
        }
        if (null != mMenuItemClear && null != mUnlockHistoryList) {
            mMenuItemClear.setVisible(mUnlockHistoryList.size() > 0);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
        case R.id.clear:
            if (mService != null) {
                mService.clearUnlockHistory();
                updateAdapter();
            }
            break;
        default:
            break;
        }
        return true;
    }


    @Override
    public void onBackPressed() {
/*
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
            intent.putExtra(LockPatternActivity.KEY_INVOKE, mHasPriviledge);
            setResult(Activity.RESULT_OK, intent);
            finish();
            return;
        }*/
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

    private void startDecryptFailActivity() {
        Intent intent = new Intent(this, DecryptFailHistoryActivity.class);
        intent.putExtra(DataProtectionService.KEY_INVOKE,
                mHasPriviledge);
        startActivityForResult(intent,
                CODE_VIEW_DECRYPT_FAIL_HISTORY);
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

    private void showCancelDialog() {
        AlertDialogFragment listDialogFragment = (AlertDialogFragment) getFragmentManager()
                .findFragmentByTag("KEY");
        final DialogInterface.OnClickListener cListener = new DialogInterface.OnClickListener() {

            @Override
            public void onClick(DialogInterface dialog, int which) {
                dialog.dismiss();
            }
        };

        if (UnlockHistoryActivity.this.isResumed()) {
            if (listDialogFragment == null) {

                AlertDialogFragmentBuilder builder = new AlertDialogFragmentBuilder();
                listDialogFragment = builder.setDoneTitle(R.string.btn_ok)
                        .setCancelTitle(R.string.btn_cancel)
                        .setMessage(R.string.cancel_locking).create();
                listDialogFragment.setOnDoneListener(cListener);

                listDialogFragment.show(getFragmentManager(), "KEY");
                boolean ret = getFragmentManager().executePendingTransactions();

            }
        }
    }

    private void updateAdapter() {
        if (mService != null) {
            Log.d(TAG, "updateAdapter: count = " + mUnlockHistoryList.size());
            mUnlockHistoryList = mService.getUnlockHistory();
            mEmptyView.setVisibility(mUnlockHistoryList.isEmpty() ? View.VISIBLE : View.GONE);
            mAdapter.notifyDataSetChanged();
            mService.listenUnlockUpdate(mListerner);
            invalidateOptionsMenu();
        }
    }

    private class UnlockedFilesAdapter extends BaseAdapter {

        @Override
        public int getCount() {
            if (mUnlockHistoryList != null) {
                return mUnlockHistoryList.size();
            }
            return 0;
        }

        @Override
        public Object getItem(int arg0) {
            if (mUnlockHistoryList != null) {
                return mUnlockHistoryList.get(arg0);
            }
            return null;
        }

        @Override
        public long getItemId(int arg0) {
            if (mUnlockHistoryList != null) {
                String file = mUnlockHistoryList.get(arg0);
                if (file != null) {
                    return file.hashCode();
                }
            }
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) {
                final LayoutInflater inflater = LayoutInflater.from(UnlockHistoryActivity.this);
                convertView = inflater.inflate(R.layout.unlock_history_list_item, null);
            }
            TextView title = (TextView) convertView.findViewById(R.id.list_item_title);
            TextView date = (TextView) convertView.findViewById(R.id.list_item_date);
            TextView path = (TextView) convertView.findViewById(R.id.list_item_path);
            String history = mUnlockHistoryList.get(getCount() - 1 - position);

            long dateModify = 0;
            try {
                dateModify = Long.parseLong(history.substring(0, history.indexOf(File.separator)));
            } catch (NumberFormatException e) {
                dateModify = System.currentTimeMillis();
                Log.e(TAG, "getView with NumberFormatException", e);
            }
            String filePath = history.substring(history.indexOf(File.separator));
            title.setText(getFileName(filePath));
            date.setText(FileUtils.formatTimeStampStringExtend(getApplicationContext(), dateModify));
            path.setText(filePath);
            return convertView;
        }
    }

    private void showDetailDialog(String path) {
        FragmentManager fm = getFragmentManager();
        AlertDialogFragment detailDialog = (AlertDialogFragment) fm.findFragmentByTag(SHOW_DETAIL);
        if (UnlockHistoryActivity.this.isResumed()) {
            if (detailDialog == null) {
                AlertDialogFragmentBuilder builder = new AlertDialogFragmentBuilder();
                detailDialog = builder
                        .setTitle(getFileName(path))
                        .setDoneTitle(R.string.btn_ok)
                        .setMessage(path)
                        .create();
                detailDialog.show(fm, SHOW_DETAIL);
                fm.executePendingTransactions();
            }
        }
    }

    private String getFileName(String path) {
        String fileName = (path == null ? "" : path.toString());
        int idx = fileName.lastIndexOf(File.separator);
        if (idx >= 0) {
            fileName = fileName.substring(idx + 1);
        }
        return fileName;
    }

    public interface OnUnlockHistoryUpdate {
        void onUpdated();
    }
}
