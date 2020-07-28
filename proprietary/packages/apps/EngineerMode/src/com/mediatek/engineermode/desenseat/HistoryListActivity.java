package com.mediatek.engineermode.desenseat;

import android.app.Dialog;
import android.app.ListActivity;
import android.app.ProgressDialog;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.io.File;
import java.util.ArrayList;

/**
 * Activity to show summary of history data.
 *
 */
public class HistoryListActivity extends ListActivity {

    protected static final String TAG = "DesenseAT/History";
    protected static final String FILE_NAME_KEY = "filename";
    private static final int DIALOG_LOADING = 1;
    private static final int LOADING_DONE = 1;
    private ArrayList<String> mDataList = new ArrayList<String>();
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        // TODO Auto-generated method stub
        super.onCreate(savedInstanceState);
        setContentView(R.layout.desense_at_history_list);
    }

    @Override
    protected void onStart() {
        // TODO Auto-generated method stub
        super.onStart();
        updateDataList();
    }

    private void updateDataList() {
        mDataList.clear();
        showDialog(DIALOG_LOADING);
        new LoadingTask().execute();
    }

    /**
     * Background operation for attach.
     *
     */
    private class LoadingTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub

            String folderPath = Util.getTestFilePath(HistoryListActivity.this, Util.HISTORY_PATH);
            File parentFolder = new File(folderPath);
            if (!parentFolder.exists()) {
                mHandler.sendEmptyMessage(LOADING_DONE);
                return true;
            }
            File[] fileList = parentFolder.listFiles();
            for (File data: fileList) {
                mDataList.add(data.getName());
                Elog.d(TAG, "file name " + data.getName());
            }
            mHandler.sendEmptyMessage(LOADING_DONE);
            return true;
        }
    }

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case LOADING_DONE:
                ArrayAdapter<String> moduleAdapter =
                        new ArrayAdapter<String>(HistoryListActivity.this,
                        android.R.layout.simple_list_item_1, mDataList);
                setListAdapter(moduleAdapter);
                removeDialog(DIALOG_LOADING);
                break;
            default:
                break;
            }
        }
    };

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        case DIALOG_LOADING:
            ProgressDialog dlgLoading = new ProgressDialog(this);
            dlgLoading.setMessage(getString(R.string.desense_at_history_loading));
            dlgLoading.setCancelable(false);
            dlgLoading.setIndeterminate(true);
            return dlgLoading;
        default:
            return null;
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        // TODO Auto-generated method stub
        final Intent intent = new Intent();
        intent.setClass(this, HistoryDetailActivity.class);
        intent.putExtra(FILE_NAME_KEY, mDataList.get(position));
        startActivity(intent);
    }
}
