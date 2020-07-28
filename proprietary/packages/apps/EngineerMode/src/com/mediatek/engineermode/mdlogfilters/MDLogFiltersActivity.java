/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.engineermode.mdlogfilters;



import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.os.Message;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;
import com.mediatek.engineermode.emsvr.AFMFunctionCallEx;
import com.mediatek.engineermode.emsvr.FunctionReturn;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Collections;

/**
 * Class for updating filter in modem log.
 *
 */
public class MDLogFiltersActivity extends Activity {
    private static final String TAG = "MDLogFilters";
    private static final String POST_FIX = ".bin";
    private static final String DEFAULT_FILTER = "_Default";
    private static final String BIN_RECORD_PATH = "filter_config";
    private static final String STORAGE_AUTHORITY = "com.android.externalstorage.documents";
    private static final String PRIMARY_STORAGE = "primary";
    private static final String ACTION_ASK_LOG_PATH = "com.debug.loggerui.ADB_CMD";
    private static final String LOG_CMD_PKG = "com.debug.loggerui";
    private static final String LOG_CMD_CLASS = "com.debug.loggerui.framework.LogReceiver";
    private static final String KEY_ASK_LOG_PATH = "cmd_name";
    private static final String VALUE_ASK_LOG_PATH = "get_mtklog_path";
    private static final String ACTION_GET_LOG_PATH = "com.debug.loggerui.result";
    private static final String KEY_GET_LOG_PATH = "result_value";
    private static final String FILTER_TAR_PATH = "/mdlog1_config/";
//    private static final int DIALOG_UPDATE_PATH = 1;
    private static final int DIALOG_QUERY_FILTERS = 2;

    private RadioGroup mRgFilterList;
    private Button mBtnMore;
    private Button mBtnCopy;
    private String mSrcBinName;
    private View mMoreFilterView;
    private ArrayList<FilterInfo> mFilterList = new ArrayList<FilterInfo>();
    private static final int MSG_FILE_COPY_DONE = 1;
    private static final int MSG_IO_EXCEPTION = 2;
    private static final int MSG_FILE_FORMAT_WRONG = 3;
    private static final int REQUEST_SELECT_BIN = 1;
    private static final byte[] VALID_BIN_BYTES = {(byte) 0xCD, (byte) 0xAB,
        (byte) 0x54, (byte) 0x24};
    private static final String[] KEY_WORD_LIST = {
        "md1_filter__Default",
        "md1_filter_FullLog",
        "md1_filter_meta",
        "md1_filter_SlimLog_DspAllOff",
        "md1_filter_PLS_PS_ONLY",
        "md1_filter_LowPowerMonitor"
    };
    private static final String[] GEN_FILTER_FILE_LIST = {
        "catcher_filter_1_ulwctg_n__Default.bin",
        "catcher_filter_1_ulwctg_n_FullLog.bin",
        "catcher_filter_1_ulwctg_n_meta.bin",
        "catcher_filter_1_ulwctg_n_SlimLog_DspAllOff.bin",
        "catcher_filter_1_ulwctg_n_PLS_PS_ONLY.bin",
        "catcher_filter_1_ulwctg_n_LowPowerMonitor.bin"
    };
    private static final String mDefaultFilterPath = "/vendor/etc/firmware/customfilter/";
    private static final String mGenFilterPath =
      Environment.getExternalStorageDirectory().getAbsolutePath() + "/mdlogfilters/";

    private int mPreFixIndex = -1;


    private Handler mHandler = new MyHandler();


    private String mSrcPath = null;
    private String mTarPath = null;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.md_log_filter);
        mRgFilterList = (RadioGroup) findViewById(R.id.rg_filter);
        mBtnMore = (Button) findViewById(R.id.btn_more);
        mBtnMore.setOnClickListener(mOnClickListener);
        mBtnCopy = (Button) findViewById(R.id.btn_copy);
        mBtnCopy.setOnClickListener(mOnClickListener);
        mTarPath = Environment.getExternalStorageDirectory().getAbsolutePath()
                + File.separator + "debuglogger" + FILTER_TAR_PATH;
        Elog.i(TAG, "mTarPath is " + mTarPath);
//        IntentFilter intentFilter = new IntentFilter();
//        intentFilter.addAction(ACTION_GET_LOG_PATH);
//        registerReceiver(mReceiver, intentFilter);
        new FileLoadTask().execute();
    }

    @Override
    protected void onResume() {
        // TODO Auto-generated method stub
        super.onResume();
//        askLogPath();
//        showDialog(DIALOG_UPDATE_PATH);
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
//        unregisterReceiver(mReceiver);
        super.onDestroy();
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        switch (id) {
/*        case DIALOG_UPDATE_PATH:
            ProgressDialog dlgUpdatePath = new ProgressDialog(this);
            dlgUpdatePath.setMessage(getString(R.string.md_log_filter_update_log_path));
            dlgUpdatePath.setCancelable(false);
            dlgUpdatePath.setIndeterminate(true);
            return dlgUpdatePath;*/
        case DIALOG_QUERY_FILTERS:
            ProgressDialog dlgQueryFilters = new ProgressDialog(this);
            dlgQueryFilters.setMessage(getString(R.string.md_log_filter_query_filters));
            dlgQueryFilters.setCancelable(false);
            dlgQueryFilters.setIndeterminate(true);
            return dlgQueryFilters;
        default:
            return null;
        }
    }

/*    private void askLogPath() {
        Intent intentDone = new Intent(ACTION_ASK_LOG_PATH);
        intentDone.setClassName(LOG_CMD_PKG, LOG_CMD_CLASS);
        intentDone.putExtra(KEY_ASK_LOG_PATH, VALUE_ASK_LOG_PATH);
        sendBroadcast(intentDone);
    }*/

/*    private BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Elog.d(TAG, "onReceive: " + intent.getAction());
            if (intent.getAction().equals(ACTION_GET_LOG_PATH)) {

                String path = intent.getStringExtra(KEY_GET_LOG_PATH);
                Elog.i(TAG, "path: " + path);
                if (path != null) {
                    mTarPath = path + FILTER_TAR_PATH;

                }
                removeDialog(DIALOG_UPDATE_PATH);
            }
        }
    };*/

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // TODO Auto-generated method stub
        if (requestCode == REQUEST_SELECT_BIN) {
            if (resultCode == Activity.RESULT_OK) {
                //remove old user added filter
                if (mMoreFilterView != null) {
                    mRgFilterList.removeView(mMoreFilterView);
                }
                Uri uri = data.getData();
                Elog.i(TAG, "uri:" + uri);
                //get selected file path
                String path = getPathFromUri(uri);
                if (path == null) {
                    return;
                }
                //Show new user added filter on UI
                String strFilterName = extractFilterName(path, false);
                if (strFilterName == null) {
                    Toast.makeText(this, R.string.md_log_filter_wrong_filter_name,
                            Toast.LENGTH_SHORT).show();
                    return;
                }
                FilterInfo filterInfo = new FilterInfo(strFilterName, path);
                RadioButton radio = new RadioButton(MDLogFiltersActivity.this);
                radio.setText(filterInfo.getFilterName());
                radio.setTag(filterInfo);
                mRgFilterList.addView(radio);
                Elog.i(TAG, "mRgFilterList addView:" + filterInfo.getFilterName());
                mRgFilterList.check(radio.getId());
                mMoreFilterView = radio;
            }
        }
    }

    private String getPathFromUri(Uri uri) {

        if (uri == null) {
            return null;
        }
        if (!STORAGE_AUTHORITY.equals(uri.getAuthority())) {
            Elog.e(TAG, "not support:" + uri.getAuthority());
            Toast.makeText(this, R.string.md_log_filter_wrong_filter_path,
                    Toast.LENGTH_SHORT).show();
            return null;
        }
        String strLastPathSegment = uri.getLastPathSegment();
        Elog.i(TAG, "strLastPathSegment:" + strLastPathSegment);
        if (strLastPathSegment != null) {
            String[] pathArray = strLastPathSegment.split(":");
            if ((pathArray != null) && (pathArray.length >= 2)) {

                if (PRIMARY_STORAGE.equalsIgnoreCase(pathArray[0])) {
                    return Environment.getExternalStorageDirectory().getAbsolutePath()
                            + "/" + pathArray[1];
                } else {
                    String strPath = getExternalSDPath(pathArray[0]);
                    if (strPath != null) {
                        return strPath + "/" + pathArray[1];
                    }

                }
            }

        }

        return null;

    }

    private String getExternalSDPath(String name) {
        if ((name != null) && (name.length() != 0)) {
            StorageManager storageManager =
                    (StorageManager) getSystemService(Context.STORAGE_SERVICE);
            StorageVolume[] volumes = storageManager.getVolumeList();
            for (StorageVolume volume : volumes) {
                String volumePathStr = volume.getPath();
                if (volumePathStr.contains(name) &&
                        Environment.MEDIA_MOUNTED.equalsIgnoreCase(volume.getState())) {
                    Elog.i(TAG, "volumePathStr:" + volumePathStr);
                    return volumePathStr;
                }
            }
        }

        return null;
    }

    private OnClickListener mOnClickListener = new OnClickListener() {
        @Override
        public void onClick(View v) {
            if (v.equals(mBtnMore)) {
                Intent it = new Intent(Intent.ACTION_GET_CONTENT);
                it.setType("*/*");
                startActivityForResult(it, REQUEST_SELECT_BIN);
            } else if (v.equals(mBtnCopy)) {
                //get selected bin file path
                int btnID = mRgFilterList.getCheckedRadioButtonId();
                RadioButton rb = (RadioButton) findViewById(btnID);
                if (rb != null) {
                    FilterInfo fileFilter = (FilterInfo) rb.getTag();
                    mSrcBinName = fileFilter.getBinName();
                } else {
                    Toast.makeText(MDLogFiltersActivity.this,
                            R.string.md_log_filter_no_filter_selected,
                            Toast.LENGTH_SHORT).show();
                    return;
                }

                mBtnCopy.setEnabled(false);
                new Thread() {
                    public void run() {
                        Message message = new Message();
                        //Check bin file valid
                        if (!checkBinValid(mSrcBinName)) {
                            message.what = MSG_FILE_FORMAT_WRONG;
                            mHandler.sendMessage(message);
                            return;
                        }

                        //copy filter
                        if (recordFilterPath(mSrcBinName, mTarPath)) {
                            message.what = MSG_FILE_COPY_DONE;
                        } else {
                            message.what = MSG_IO_EXCEPTION;
                        }

                        mHandler.sendMessage(message);

                    }
                }.start();

            }
            }


    };


    private boolean recordFilterPath(String srcFile, String tarDir) {
        Elog.i(TAG, "recordFilterPath " + srcFile + " " + tarDir);
        File tarFolder = new File(tarDir);
        if (!tarFolder.exists()) {
            if (!tarFolder.mkdirs()) {
                return false;
            }
        }

        FileOutputStream osRecord = null;
        try {
            File recordFile = new File(tarDir + BIN_RECORD_PATH);
            Elog.i(TAG, "recordFile is:" + recordFile.getAbsolutePath());
            osRecord = new FileOutputStream(recordFile);
            File source = new File(srcFile);
            if (source.exists()) {
                osRecord.write(srcFile.getBytes());
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            try {
                if (osRecord != null) {
                    osRecord.close();
                }
            } catch (IOException e) {
                e.printStackTrace();
            }
        }

        return true;
    }


    /**
     * Class for handling message.
     *
     */
    private class MyHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            // TODO Auto-generated method stub
            super.handleMessage(msg);
            if (msg.what == MSG_FILE_COPY_DONE) {
                mBtnCopy.setEnabled(true);

                Toast.makeText(MDLogFiltersActivity.this, R.string.md_log_filter_copy_success,
                        Toast.LENGTH_SHORT).show();
                Elog.i(TAG, "Update filter success");
            } else if (msg.what == MSG_IO_EXCEPTION) {
                mBtnCopy.setEnabled(true);
                Toast.makeText(MDLogFiltersActivity.this, R.string.md_log_filter_copy_fail,
                        Toast.LENGTH_SHORT).show();
                Elog.i(TAG, "Update filter fail for IO error");
            } else if (msg.what == MSG_FILE_FORMAT_WRONG) {
                mBtnCopy.setEnabled(true);
                Toast.makeText(MDLogFiltersActivity.this,
                        R.string.md_log_filter_wrong_filter_format,
                        Toast.LENGTH_SHORT).show();
                Elog.i(TAG, "Update filter fail for wrong filter format");
            }

        }
    }

    private void extractFilterFiles() {
        if (FeatureSupport.isSupported(FeatureSupport.FK_SINGLE_BIN_MODEM_SUPPORT)) {
            File path = new File(mGenFilterPath);
            if (!path.isDirectory()) {
                path.mkdirs();
            }
            Elog.i(TAG, "extractFilterFiles");
            for (int k = 0; k < KEY_WORD_LIST.length; k++) {
                AFMFunctionCallEx functionCall = new AFMFunctionCallEx();
                if (functionCall.startCallFunctionStringReturn(
                        AFMFunctionCallEx.FUNCTION_EM_MD_LOG_FILTER_GEN)) {
                    functionCall.writeParamNo(2);
                    functionCall.writeParamString(KEY_WORD_LIST[k]);
                    functionCall.writeParamString(mGenFilterPath + GEN_FILTER_FILE_LIST[k]);
                    FunctionReturn funcRet = null;

                    do {
                        funcRet = functionCall.getNextResult();
                    } while (funcRet.mReturnCode == AFMFunctionCallEx.RESULT_CONTINUE);
                }

            }
            mSrcPath = mGenFilterPath;
        } else {
            mSrcPath = mDefaultFilterPath;
        }
    }
    /**
     * Class for loading filter files.
     *
     */
    private class FileLoadTask extends AsyncTask<Void, Void, Void> {

        @Override
        protected void onPreExecute() {
            // TODO Auto-generated method stub
            super.onPreExecute();
            showDialog(DIALOG_QUERY_FILTERS);
        }

        @Override
        protected Void doInBackground(Void... params) {
            extractFilterFiles();
            Elog.i(TAG, "default filter path:" + mSrcPath);
            File srcFolder = new File(mSrcPath);
            if ((srcFolder != null) && srcFolder.exists() && srcFolder.isDirectory()) {
                //List filter from pre-install filter folder
                File[] fileList = srcFolder.listFiles();
                ArrayList<String> tempFileList = new ArrayList<String>();
                if (fileList != null) {
                    for (File file: fileList) {
                        String filePath = file.getAbsolutePath();
                        Elog.i(TAG, "filePath:" + filePath);
                        String filterName = extractFilterName(filePath, true);
                        Elog.i(TAG, "filterName:" + filterName);
                        if (filterName == null) {
                            continue;
                        }
                        //Make _default the first filter
                        if (DEFAULT_FILTER.equals(filterName)) {
                            FilterInfo fileInfo = new FilterInfo(filterName, filePath);
                            mFilterList.add(fileInfo);
                            Elog.i(TAG, "add fileInfo:" + filterName + " " + filePath);
                        } else {
                            tempFileList.add(filePath);
                        }


                    }
                    //Sort filter by Alphabetical order
                    Collections.sort(tempFileList);
                    for (String filePath:tempFileList) {
                        //Get filter name
                        String filterName = extractFilterName(filePath, true);
                        if (filterName != null) {
                            filterName = filterName.replaceAll("_", " ");
                            FilterInfo fileInfo = new FilterInfo(filterName, filePath);
                            mFilterList.add(fileInfo);
                            Elog.i(TAG, "add fileInfo:" + filterName + " " + filePath);
                        }


                    }
                }

            }
            return null;
        }

        @Override
        protected void onPostExecute(Void result) {
            removeDialog(DIALOG_QUERY_FILTERS);
            showFilterList();
        }
    }

    private void showFilterList() {
        //Show filter as radio button
        for (FilterInfo filterInfo: mFilterList) {
            RadioButton radio = new RadioButton(MDLogFiltersActivity.this);
            radio.setText(filterInfo.getFilterName());
            radio.setTag(filterInfo);
            mRgFilterList.addView(radio);
            Elog.i(TAG, "mRgFilterList addView:" + filterInfo.getFilterName());
        }
        mRgFilterList.clearCheck();
    }

    private boolean checkBinValid(String filePath) {
        Elog.i(TAG, "checkBinValid:" + filePath);
        InputStream is = null;
        try {
            File file = new File(filePath);
            if (file.exists()) {
                is = new FileInputStream(file);
                byte[] buffer = new byte[VALID_BIN_BYTES.length];
                int byteRead = is.read(buffer);
                if (byteRead < VALID_BIN_BYTES.length) {
                    return false;
                } else {
                    for (int k = 0; k < VALID_BIN_BYTES.length; k++) {
                        if (VALID_BIN_BYTES[k] != buffer[k]) {
                            return false;
                        }
                    }
                }
                return true;
            }
        } catch (IOException e) {
            e.printStackTrace();
            return false;
        } finally {
            if (is != null) {
                try {
                    is.close();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
        }
        return false;
    }

    private String extractFilterName(String name, boolean checkRule) {
        if (name == null) {
            return null;
        }
        Elog.i(TAG, "extractFilterName:" + name);
        int indexName = name.lastIndexOf("/");
        if (indexName != -1) {
            name = name.substring(indexName + 1);
        } else {
            return null;
        }
        if (!checkRule) {
            return name.trim();
        }
        if (mPreFixIndex == -1) {
            for (int k = 0; k < 5; k++) {
                mPreFixIndex = name.indexOf("_", mPreFixIndex + 1);
                if (mPreFixIndex == -1) {
                    return null;
                }
            }
        }

        int postFixIndex = name.indexOf(POST_FIX);
        if ((mPreFixIndex != -1) && (postFixIndex != -1) && (mPreFixIndex < postFixIndex)) {
            String filterName = name.substring(mPreFixIndex + 1, postFixIndex);
            return filterName.trim();
        } else {
            return null;
        }
    }

    /**
     * Class for filter information.
     *
     */
    private class FilterInfo {
        private String mFilterName;
        private String mBinName;
        FilterInfo(String filterName, String binName) {
            mFilterName = filterName;
            mBinName = binName;
        }
        public String getFilterName() {
            return mFilterName;
        }
        public String getBinName() {
            return mBinName;
        }


    }
}