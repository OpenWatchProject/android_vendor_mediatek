package com.debug.loggerui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.util.ArrayMap;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.AdapterView.OnItemLongClickListener;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.debug.loggerui.controller.LogControllerUtils;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.util.HashSet;
import java.util.Set;

/**
 * @author MTK81255
 *
 */
public class LogFolderListActivity extends Activity
        implements OnItemClickListener, OnItemLongClickListener {

    private static final String TAG = Utils.TAG + "/LogFolderListActivity";

    private static final int CANCEL_MENU_ID = 11;
    private static final int CLEAR_ALL_MENU_ID = 12;

    public static final String EXTRA_KEY_LOG_FOLDER_LIST = "key_log_folder_list";
    private ListView mListView;

    private static final int FINISH_CLEAR_LOG = 1;
    /**
     * Before enter show log folder list. We need to check current log folder status, whether they
     * are empty. Do such things in main thread may hang UI, so need to put them into thread
     */
    private static final int DLG_CHECKING_LOG_FILES = 1001;
    private static final int MSG_SHOW_CHECKING_LOG_DIALOG = 11;
    private static final int MSG_REMOVE_CHECKING_LOG_DIALOG = 12;

    private ArrayMap<String, HashSet<File>> mLogFolderMap = new ArrayMap<String, HashSet<File>>();
    private LogFolderAdapter mAdapter;
    private Dialog mClearLogConfirmDialog;
    private static final int DLG_WAITING_DELETE = 1;

    private boolean mIsLongClick = false;
    /**
     * Since more than one thread may change clear log count value, add synchronized to it.
     */
    private boolean mIsClearing = false;
    public static final String IS_NEED_CLEAR_MD_BOOTUP_LOG_KEY = "is_need_clear_md_bootup_log_key";
    private SharedPreferences mSharedPreferences;

    private Handler mClearLogProgressHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Utils.logi(TAG, "mClearLogProgressHandler msg.what = " + msg.what);
            if (msg.what == FINISH_CLEAR_LOG) {
                finish();
            } else if (msg.what == MSG_SHOW_CHECKING_LOG_DIALOG) {
                createProgressDialog(DLG_CHECKING_LOG_FILES);
            } else if (msg.what == MSG_REMOVE_CHECKING_LOG_DIALOG) {
                dismissProgressDialog();
                mAdapter.setLogFolderShowList(mLogFolderMap);
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        Utils.logi(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.log_folder);
        mSharedPreferences = MyApplication.getInstance().getSharedPreferences();
        findViews();
        setListeners();
        initViews();
    }

    @Override
    protected void onDestroy() {
        Utils.logd(TAG, "onDestroy()");
        if (mClearLogConfirmDialog != null && mClearLogConfirmDialog.isShowing()) {
            mClearLogConfirmDialog.dismiss();
        }
        dismissProgressDialog();
        super.onDestroy();
    }

    @Override
    protected void onResume() {
        Utils.logi(TAG, "onResume()");
        mClearLogProgressHandler.sendEmptyMessage(MSG_SHOW_CHECKING_LOG_DIALOG);
        new Thread(new Runnable() {
            @Override
            public void run() {
                initLogItemList();
                mClearLogProgressHandler.sendEmptyMessage(MSG_REMOVE_CHECKING_LOG_DIALOG);
            }
        }).start();
        super.onResume();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(Menu.NONE, CANCEL_MENU_ID, 1, getString(R.string.cancel_menu))
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        menu.add(Menu.NONE, CLEAR_ALL_MENU_ID, 2, getString(R.string.clear_all_menu))
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_ALWAYS);
        return true;
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        switch (item.getItemId()) {
        case CANCEL_MENU_ID:
            finish();
            break;
        case CLEAR_ALL_MENU_ID:
            clearAllLogs();
            break;
        default:
            break;
        }
        return super.onMenuItemSelected(featureId, item);
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> adapterview, View view, int i, long l) {
        mIsLongClick = true;
        return false;
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        if (mIsLongClick) {
            mIsLongClick = false;
            return;
        }
        Intent intent = new Intent(LogFolderListActivity.this, LogFileListActivity.class);
        intent.setClass(LogFolderListActivity.this, LogFileListActivity.class);
        intent.putExtra(EXTRA_KEY_LOG_FOLDER_LIST,
                mAdapter.getLogFolderShowList().valueAt(position));
        startActivity(intent);
    }

    private void findViews() {
        mListView = (ListView) findViewById(R.id.log_folder_list_view);
    }

    private void initViews() {
        mAdapter = new LogFolderAdapter(this);
        mListView.setAdapter(mAdapter);
    }

    private ProgressDialog mProgressDialog;

    /**
     * @param id
     *            int
     */
    private void createProgressDialog(int id) {
        if (id == DLG_WAITING_DELETE) {
            mProgressDialog = ProgressDialog.show(LogFolderListActivity.this,
                    getString(R.string.clear_dialog_title),
                    getString(R.string.clear_dialog_content), true, false);
        } else if (id == DLG_CHECKING_LOG_FILES) {
            mProgressDialog = ProgressDialog.show(this, null,
                    getString(R.string.waiting_checking_log_dialog_message), true, false);
        }
    }

    private void dismissProgressDialog() {
        if (null != mProgressDialog) {
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
    }

    private void setListeners() {
        if (mListView != null) {
            mListView.setOnItemClickListener(LogFolderListActivity.this);
            mListView.setOnItemLongClickListener(LogFolderListActivity.this);
        }
    }

    private void clearAllLogs() {
        boolean[] clearMDBootupLogchecked =
                { mSharedPreferences.getBoolean(IS_NEED_CLEAR_MD_BOOTUP_LOG_KEY, false) };
        android.app.AlertDialog.Builder builder =
                new AlertDialog.Builder(LogFolderListActivity.this);
        String[] items = { getString(R.string.clear_modem_boot_up_log) };
        builder.setTitle(R.string.clear_all_dlg_title);
        builder.setMultiChoiceItems(items, new boolean[] {
                mSharedPreferences.getBoolean(IS_NEED_CLEAR_MD_BOOTUP_LOG_KEY, false) },
                new DialogInterface.OnMultiChoiceClickListener() {
                    @Override
                    public void onClick(DialogInterface dialogInterface, int i, boolean b) {
                        clearMDBootupLogchecked[i] = b;
                        mSharedPreferences.edit().putBoolean(
                                IS_NEED_CLEAR_MD_BOOTUP_LOG_KEY, b).apply();
                    }
                });
        builder.setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int whichButton) {
                if (mIsClearing) {
                    return;
                }
                mIsClearing = true;
                createProgressDialog(DLG_WAITING_DELETE);
                new Thread(new Runnable() {
                    @Override
                    public void run() {
                        LogControllerUtils.clearAllLogs();
                        Utils.logi(TAG, "Detelect log folder clear done");
                        mClearLogProgressHandler.sendEmptyMessage(FINISH_CLEAR_LOG);
                    }
                }).start();
            }
        }).setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int whichButton) {
                dialog.dismiss();
            }
        });
        mClearLogConfirmDialog = builder.create();
        mClearLogConfirmDialog.show();
    }

    private void initLogItemList() {
        mLogFolderMap.clear();
        Set<File> canBeDeletedFileList = LogControllerUtils.getCanBeDeletedFileList();
        Utils.logi(TAG, "initLogItemList()"
                + " canBeDeletedFileList.size() = " + canBeDeletedFileList.size());
        if (canBeDeletedFileList.size() == 0) {
            return;
        }

        for (File canBeDeletedFile : canBeDeletedFileList) {
            String logParentName = canBeDeletedFile.getParentFile().getName();
            if (mLogFolderMap.containsKey(logParentName)) {
                mLogFolderMap.get(logParentName).add(canBeDeletedFile);
            } else {
                HashSet<File> logFolderList = new HashSet<File>();
                logFolderList.add(canBeDeletedFile);
                mLogFolderMap.put(logParentName, logFolderList);
            }
        }
    }

    /**
     * @author MTK81255
     *
     */
    class LogFolderAdapter extends BaseAdapter {

        private LayoutInflater mInflater;
        private ArrayMap<String, HashSet<File>> mLogFolderShowMap =
                new ArrayMap<String, HashSet<File>>();

        public LogFolderAdapter(Context context) {
            mInflater = LayoutInflater.from(context);
        }

        public void setLogFolderShowList(ArrayMap<String, HashSet<File>> logFolderShowMap) {
            mLogFolderShowMap.clear();
            mLogFolderShowMap.putAll(logFolderShowMap);
            this.notifyDataSetChanged();
        }

        public ArrayMap<String, HashSet<File>> getLogFolderShowList() {
            return mLogFolderShowMap;
        }

        @Override
        public int getCount() {
            return mLogFolderShowMap.size();
        }

        @Override
        public Object getItem(int i) {
            return null;
        }

        @Override
        public long getItemId(int i) {
            return i;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(R.xml.log_folder_item, parent, false);
            }

            TextView textView = (TextView) view.findViewById(R.id.log_folder_name);
            textView.setText(mLogFolderShowMap.keyAt(position));

            return view;
        }

    }

}
