package com.mediatek.engineermode.mcfconfig;

import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Comparator;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Intent;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.RemoteException;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;

import vendor.mediatek.hardware.engineermode.V1_0.IEmd;
import vendor.mediatek.hardware.engineermode.V1_0.IEmCallback ;


public class McfFileSelectActivity extends Activity
    implements OnItemClickListener {
    protected static final String TAG = "McfConfig/McfFileSelectActivity";
    public static final String SAVED_PATH_KEY = "saved_path";
    private static final int DIALOG_LOAD_FILES = 0;
    public static final int OTA_FILE_CODE = 0;
    public static final int OPOTA_FILE_CODE = 1;
    public static final int GEN_OPOTA_FILE_CODE = 2;
    public static final String[] OP_OTA_SUFFIX = new String[]{"mcfopota", "bin"};
    public static final String[] OTA_SUFFIX = new String[]{"mcfota", "bin"};
    private McfFileSelectActivity instance = this;
    private ArrayList<FileInfo> mFileList = new ArrayList<FileInfo>();
    private FileInfoAdapter adapter;
    private String root;
    private int mcfFileType;
    private String currentPath;
    private ListView lvContent;
    private Button btnCancel;
    protected Bundle mSavedInstanceState = null;

    /**
     * Start activity by intent
     * */
    public static void actionStart(Activity mActivity, String root, int REQUEST) {
        Intent intent = new Intent(mActivity, McfFileSelectActivity.class);
        intent.putExtra("root", root);
        intent.putExtra("file_type", REQUEST);
        (mActivity).startActivityForResult(intent, REQUEST);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        root = getIntent().getStringExtra("root");
        mcfFileType = getIntent().getIntExtra("file_type", OTA_FILE_CODE);
        currentPath = root;
        Elog.d(TAG, "root path: " + root + ", file_type:" + mcfFileType);
        new FileLoadTask().execute(root);
        initLayout();
    }

    public void initLayout() {
        setContentView(R.layout.select_file_main);

        btnCancel = (Button) findViewById(R.id.select_cancel);

        // set up a list view
        lvContent = (ListView) findViewById(R.id.list_view);
        if (lvContent != null) {
            lvContent.setEmptyView(findViewById(R.id.empty_view));
            lvContent.setOnItemClickListener(this);
            lvContent.setFastScrollEnabled(true);
            lvContent.setVerticalScrollBarEnabled(true);
        }
        if(mSavedInstanceState != null){
            String savePath = mSavedInstanceState.getString(SAVED_PATH_KEY);
            if (savePath != null) {
                currentPath = savePath;
                Elog.d(TAG, "onCreate, mCurrentPath updated to = "+currentPath);
            }
        }

        initEvent();
    }

    private void initEvent() {
        btnCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Elog.d(TAG, "click 'Cancel' to quit directly ");
                finish();
            }
        });

    }

    /**
     * update UI
     * */
    private void refreshView() {
        if (adapter == null) {
            adapter = new FileInfoAdapter(instance, mFileList);
            lvContent.setAdapter(adapter);
            lvContent.setOnItemClickListener(this);
        } else {
            adapter.notifyDataSetChanged();
        }
    }


    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        switch (id) {
        case DIALOG_LOAD_FILES:
            ProgressDialog dlgQueryFilters = new ProgressDialog(this);
            dlgQueryFilters.setMessage(getString(R.string.md_log_filter_query_filters));
            dlgQueryFilters.setCancelable(false);
            dlgQueryFilters.setIndeterminate(true);
            return dlgQueryFilters;
        default:
            return null;
        }
    }

    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position, long id) {

        FileInfo selecteItemFileInfo = (FileInfo) parent.getItemAtPosition(position);
        Intent intent = new Intent();
        Uri uri = selecteItemFileInfo.getUri();
        Elog.d(TAG, "onItemClick RESULT_OK, uri : " + uri);
        intent.setData(uri);
        setResult(RESULT_OK, intent);
        finish();
    }

    @Override
    public void onBackPressed() {
        if (!currentPath.equals(root)) {
            File file = new File(currentPath);
            currentPath = file.getParentFile().getAbsolutePath();
            refreshView();
        } else {
            super.onBackPressed();
        }
    }

    private IEmCallback mEmCallback = new IEmCallback.Stub() {
        @Override
        public boolean callbackToClient(String dataStr) throws RemoteException {
            Elog.d(TAG, "callbackToClient data = " + dataStr);
            setFilePathListFromServer(dataStr);
            return true;
        }
    };

    public boolean checkFileValid(FileInfo file) {
        String fileExtension = FileUtils.getFileExtension(file.getFileName());
        switch(mcfFileType) {
        case OTA_FILE_CODE:
            if(Arrays.asList(OTA_SUFFIX).contains(fileExtension))
                return true;
            return false;
        case OPOTA_FILE_CODE:
        case GEN_OPOTA_FILE_CODE:
            if(Arrays.asList(OP_OTA_SUFFIX).contains(fileExtension))
                return true;
            return false;
        default:
            return true;
        }
    }


    public void setFilePathListFromServer(String mReturnString) {
        if(mReturnString != null && !mReturnString.trim().equals("")) {
            Elog.i(TAG, "add fileInfo:" +
                mReturnString);
            String[] fileArray = mReturnString.split(";");
            for(int i=0; i<fileArray.length; i++) {
                FileInfo fileInfo = null;
                String[] file = fileArray[i].split(":");
                if(file != null) {
                    if(file.length == 2 && file[0] != null) {
                        try {
                            fileInfo = new FileInfo(file[0], Long.parseLong(file[1]));
                        }catch ( NumberFormatException e) {
                            fileInfo = new FileInfo(file[0], -1);
                            Elog.e(TAG, "getFileSize occurs exception:" +
                                e.getMessage());
                        }
                    } else if(file.length == 1){
                        fileInfo = new FileInfo(file[0], -1);
                    }
                    if (fileInfo != null && checkFileValid(fileInfo)) mFileList.add(fileInfo);
                }
            }
        } else {
            Elog.d(TAG, "[setFilePathListFromServer] return empty");
        }
    }

    /**
     * Class for loading file list.
     *
     */
    class FileLoadTask extends AsyncTask<String, Void, Void> {


        @Override
         protected void onPreExecute() {
             super.onPreExecute();
             showDialog(DIALOG_LOAD_FILES);
         }

        @Override
        protected Void doInBackground(String... params) {
            try {
                EmUtils.getEmHidlService().getFilePathListWithCallBack(params[0], mEmCallback);
                Collections.sort(mFileList, new Comparator<FileInfo>() {

                    @Override
                    public int compare(FileInfo lhs, FileInfo rhs) {
                        return lhs.getFileName().compareTo(rhs.getFileName());
                    }

                });
            } catch (RemoteException e) {
                e.printStackTrace();
            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            removeDialog(DIALOG_LOAD_FILES);
            refreshView();
        }
    }

}

