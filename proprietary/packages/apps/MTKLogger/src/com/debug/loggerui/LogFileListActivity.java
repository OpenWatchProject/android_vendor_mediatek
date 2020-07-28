package com.debug.loggerui;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.debug.loggerui.settings.OptionalActionBarSwitch;
import com.debug.loggerui.utils.Utils;

import java.io.File;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.HashSet;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class LogFileListActivity extends Activity implements OnItemClickListener {

    private static final String TAG = Utils.TAG + "/LogFileListActivity";

    private static final int FINISH_CLEAR_LOG = 1;

    /**
     * Before showing log file list. We need to check current log list status
     * and file size Do such things in main thread may hang UI, so need to put
     * them into thread
     */
    private static final int DLG_CHECKING_LOG_FILES = 1001;
    private static final int MSG_SHOW_CHECKING_LOG_DIALOG = 11;
    private static final int MSG_REMOVE_CHECKING_LOG_DIALOG = 12;

    private ListView mListView;

    private Dialog mClearLogConfirmDialog;

    private ProgressDialog mClearLogWaitingStopDialog;
    private List<LogFileItem> mLogItemList = new ArrayList<LogFileItem>();
    private LogFileAdapter mAdapter;
    private OptionalActionBarSwitch mActionBar;
    private int mNumSelected;
//    private String mRootPath;

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Utils.logi(TAG, "mHandler msg.what = " + msg.what);
            if (msg.what == FINISH_CLEAR_LOG) {
                if (mClearLogWaitingStopDialog != null) {
                    mClearLogWaitingStopDialog.cancel();
                }
                mAdapter.setLogItemViewList(mLogItemList);
                mNumSelected = 0;
                updateTitle(mNumSelected);
            } else if (msg.what == MSG_SHOW_CHECKING_LOG_DIALOG) {
                createProgressDialog(DLG_CHECKING_LOG_FILES);
            } else if (msg.what == MSG_REMOVE_CHECKING_LOG_DIALOG) {
                dismissProgressDialog();
                mAdapter.setLogItemViewList(mLogItemList);
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.log_files);

        findViews();
        setListeners();
        initViews();
    }

    @Override
    protected void onResume() {
        mHandler.sendEmptyMessage(MSG_SHOW_CHECKING_LOG_DIALOG);
        new Thread(new Runnable() {
            @Override
            public void run() {
                initLogItemList((HashSet<File>) getIntent().getExtras().get(
                        LogFolderListActivity.EXTRA_KEY_LOG_FOLDER_LIST));
                mHandler.sendEmptyMessage(MSG_REMOVE_CHECKING_LOG_DIALOG);
            }
        }).start();
        super.onResume();
    }

    @Override
    protected void onDestroy() {
        if (mClearLogConfirmDialog != null) {
            mClearLogConfirmDialog.dismiss();
        }
        dismissProgressDialog();
        super.onDestroy();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.loggerui_contact_menu, menu);
        return true;
    }

    @Override
    public boolean onMenuItemSelected(int featureId, MenuItem item) {
        switch (item.getItemId()) {
        case R.id.action_select_all:
            setAllFileSelected(true);
            break;
        case R.id.action_unselect_all:
            setAllFileSelected(false);
            break;
        case R.id.action_delete_selected:
            clearFileSelected();
            break;
        default:
            break;
        }
        return super.onMenuItemSelected(featureId, item);
    }

    /**
     * select all file list in the adapter.
     *
     * @param checked
     *            true for check
     */
    private void setAllFileSelected(boolean checked) {
        if (mListView != null) {
            for (LogFileItem logFileItem : mAdapter.getLogItemViewList()) {
                logFileItem.setChecked(checked);
            }
            mNumSelected = checked ? mAdapter.getLogItemViewList().size() : 0;
            mListView.invalidateViews();
        }
        updateTitle(mNumSelected);
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
        CheckBox checkBox = (CheckBox) view.findViewById(R.id.log_files_check_box);
        if (checkBox != null) {
            boolean isChecked = checkBox.isChecked();
            checkBox.setChecked(!isChecked);
            if (checkBox.isChecked()) {
                mNumSelected++;
                mAdapter.getLogItemViewList().get(position).setChecked(true);
            } else {
                mNumSelected--;
                mAdapter.getLogItemViewList().get(position).setChecked(false);
            }
            updateTitle(mNumSelected);
        }
    }

    private void findViews() {
        mListView = (ListView) findViewById(R.id.log_files_list_view);
    }

    private void initViews() {
        mAdapter = new LogFileAdapter(this);
        mListView.setAdapter(mAdapter);

        mActionBar = new OptionalActionBarSwitch(this, mNumSelected);
    }

    private void updateTitle(int num) {
        mActionBar.updateTitle(num);
    }

    private void setListeners() {
        if (mListView != null) {
            mListView.setOnItemClickListener(LogFileListActivity.this);
        }
    }

    private void initLogItemList(HashSet<File> logFolderList) {
        Utils.logd(TAG, "initLogItemList()");
        mLogItemList.clear();
        if (logFolderList.size() == 0) {
            return;
        }
        for (File logFolder : logFolderList) {
            long fileSize = Utils.getFileSize(logFolder.getPath());
            mLogItemList.add(new LogFileItem(logFolder, fileSize));
        }
        Collections.sort(mLogItemList, new Comparator<LogFileItem>() {
            @Override
            public int compare(LogFileItem logFileItem1, LogFileItem logFileItem2) {
                return logFileItem1.getFile().getName().compareTo(
                        logFileItem2.getFile().getName());
            }
        });
        Utils.logi(TAG, "initLogItemList() mLogItemList.size() = " + mLogItemList.size());
    }

    private ProgressDialog mProgressDialog;

    /**
     * @param id
     *            int
     */
    private void createProgressDialog(int id) {
        if (id == DLG_CHECKING_LOG_FILES) {
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

    private void clearFileSelected() {
        if (mNumSelected == 0) {
            Toast.makeText(LogFileListActivity.this, getString(R.string.clear_non_selected_item),
                    Toast.LENGTH_SHORT).show();
            return;
        }
        mClearLogConfirmDialog = new AlertDialog.Builder(LogFileListActivity.this)
                .setTitle(R.string.clear_dlg_title)
                .setMessage(R.string.message_deletelog)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        mClearLogWaitingStopDialog = ProgressDialog.show(
                                LogFileListActivity.this, getString(R.string.clear_dialog_title),
                                getString(R.string.clear_dialog_content), true, false);
                        new Thread() {
                            @Override
                            public void run() {
                                int i = mLogItemList.size() - 1;
                                for (; i >= 0; i--) {
                                    LogFileItem logFileItem = mLogItemList.get(i);
                                    Utils.logi(TAG, "Log File Item name : "
                                            + logFileItem.getFile().getName());
                                    if (logFileItem.isChecked()) {
                                        clearLogs(logFileItem.getFile());
                                        mLogItemList.remove(i);
                                    }
                                }
                                mHandler.sendEmptyMessage(FINISH_CLEAR_LOG);
                            }
                        }.start();
                    }
                })
                .setNegativeButton(android.R.string.cancel, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int whichButton) {
                        dialog.dismiss();
                    }
                }).create();
        mClearLogConfirmDialog.show();
    }

    private void clearLogs(File dir) {
        Utils.deleteFile(dir);
    }

    /**
     * @author MTK81255
     *
     */
    class LogFileItem {

        private File mFile;
        private long mFileSize;
        private boolean mIsChecked;

        public LogFileItem(File file, long fileSize) {
            mFile = file;
            mFileSize = fileSize;
        }

        public File getFile() {
            return mFile;
        }

        public long getFileSize() {
            return mFileSize;
        }

        public void setFileSize(long fileSize) {
            mFileSize = fileSize;
        }

        public boolean isChecked() {
            return mIsChecked;
        }

        public void setChecked(boolean isChecked) {
            mIsChecked = isChecked;
        }

    }

    /**
     * @author MTK81255
     *
     */
    class LogFileAdapter extends BaseAdapter {

        private LayoutInflater mInflater;
        private List<LogFileItem> mLogItemViewList = new ArrayList<LogFileItem>();

        public LogFileAdapter(Context context) {
            mInflater = LayoutInflater.from(context);
        }

        public void setLogItemViewList(List<LogFileItem> logItemList) {
            mLogItemViewList.clear();
            mLogItemViewList.addAll(logItemList);
            this.notifyDataSetChanged();
        }

        public List<LogFileItem> getLogItemViewList() {
            return mLogItemViewList;
        }

        @Override
        public int getCount() {
            return mLogItemViewList.size();
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
        public View getView(final int position, View convertView, ViewGroup parent) {
            View view = convertView;
            if (view == null) {
                view = mInflater.inflate(R.xml.log_files_item, parent, false);
            }

            TextView fileNameTextView = (TextView) view.findViewById(R.id.log_files_name);
            TextView fileSizeTextView = (TextView) view.findViewById(R.id.log_files_size);
            CheckBox checkBox = (CheckBox) view.findViewById(R.id.log_files_check_box);

            LogFileItem logFileItem = mLogItemViewList.get(position);
            fileNameTextView.setText(logFileItem.getFile().getName());
            double fileSize = logFileItem.getFileSize();
            if (fileSize < 1024) {
                fileSizeTextView.setText("Size " + logFileItem.getFileSize() + " B");
            } else if (fileSize / 1024 < 1024) {
                fileSizeTextView.setText("Size " + new DecimalFormat(".00").format(fileSize / 1024)
                        + " KB");
            } else {
                fileSizeTextView.setText("Size "
                        + new DecimalFormat(".00").format(fileSize / 1024 / 1024) + " MB");
            }
            checkBox.setChecked(logFileItem.isChecked());

            return view;
        }

    }

}
