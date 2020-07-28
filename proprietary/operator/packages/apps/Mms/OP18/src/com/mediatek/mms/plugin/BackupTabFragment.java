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

package com.mediatek.op18.mms;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ListFragment;
import android.app.ProgressDialog;
import android.app.Service;
import android.content.Context;
import android.content.DialogInterface;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.inputmethod.InputMethodManager;
import android.view.LayoutInflater;
import android.view.View;
import android.view.WindowManager;
import android.view.WindowManager.BadTokenException;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.Toast;

import com.mediatek.op18.mms.BackupEngine.BackupResultType;
import com.mediatek.op18.mms.BackupService.BackupBinder;
import com.mediatek.op18.mms.BackupService.BackupProgress;
import com.mediatek.op18.mms.BackupService.OnBackupStatusListener;
import com.mediatek.op18.mms.modules.Composer;
import com.mediatek.op18.mms.modules.SmsBackupComposer;
import com.mediatek.op18.mms.modules.MmsBackupComposer;
import com.mediatek.op18.mms.ResultDialog.ResultEntity;
import com.mediatek.op18.mms.utils.Constants;
import com.mediatek.op18.mms.utils.Constants.State;
import com.mediatek.op18.mms.utils.MyLogger;
import com.mediatek.op18.mms.utils.ModuleType;
import com.mediatek.op18.mms.utils.SDCardUtils;
import com.mediatek.op18.mms.utils.Utils;
import com.mediatek.op18.mms.ExitActivity;
import com.mediatek.op18.mms.R;
//import com.mediatek.telephony.TelephonyManagerEx;



import java.io.File;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

public class BackupTabFragment extends ListFragment {
    private final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupTabFragment";
    protected BackupBinder mBackupService;
    private String mBackupFolderPath;
    InitMessageTask mInitMessageTask = null;
    private Handler mHandler;
    private OnBackupStatusListener mBackupListener;
    private ProgressDialog mProgressDialog;
    private AlertDialog mAlertFolderName = null;
    private Context mMainActivity;

    private static final int SD_CARD_FULL = 1;
    private static final int NAME_EXISTS = 2;
    private static final int CREATE_FOLDER_FAIL = 3;
    private static final int NO_MESSAGES = 4;
    private static final int MESSAGES_PRESENT = 5;
    private Toast mToast = null;

    private int countMsg = 0;

    private final Handler toastHandler = new Handler() {
           public void handleMessage(Message msg) {
                 if (msg.arg1 == SD_CARD_FULL)
                    Toast.makeText(getActivity(),
                     R.string.error_sd_card_full, Toast.LENGTH_SHORT).show();
                 else if (msg.arg1 == NAME_EXISTS)
                    Toast.makeText(getActivity(),
                     R.string.error_name_exists, Toast.LENGTH_SHORT).show();
                 else if (msg.arg1 == CREATE_FOLDER_FAIL)
                    Toast.makeText(getActivity(),
                     R.string.create_folder_fail, Toast.LENGTH_SHORT).show();
                 else if (msg.arg1 == NO_MESSAGES) {
                    countMsg = 0;
                 }
                 else if (msg.arg1 == MESSAGES_PRESENT) {
                    countMsg = msg.arg2;
                 }

           }
       };


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        MyLogger.logI(CLASS_TAG, "onCreate");
        bindService();
        createProgressDlg();
        initHandler();
        MyLogger.logI(CLASS_TAG, "Anuj onCreate");
    }


    private final void initHandler() {
        mHandler = new Handler() {
            @Override
            public void handleMessage(final Message msg) {
                /*switch (msg.what) {
                case MessageID.PRESS_BACK:
                    if (mBackupService != null && mBackupService.getState() != State.INIT
                            && mBackupService.getState() != State.FINISH) {
                        mBackupService.pauseBackup();
                        AbstractBackupActivity.this.showDialog(DialogID.DLG_CANCEL_CONFIRM);
                    }
                    break;
                    case MessageID.TOAST_NO_ITEM:
                        Toast.makeText(
                                AbstractBackupActivity.this,
                                R.string.no_item_selected,
                                Toast.LENGTH_SHORT)
                            .show();
                        break;
                default:
                    break;
                }*/
            }
        };
    }

    private ProgressDialog createProgressDlg() {
        if (mProgressDialog == null) {
            mProgressDialog = new ProgressDialog(getActivity());
            mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
            mProgressDialog.setMessage(getString(R.string.backuping));
            //mProgressDialog.setCancelable(true);
            mProgressDialog.setCanceledOnTouchOutside(false);
            //mProgressDialog.setCancelMessage(mHandler.obtainMessage(MessageID.PRESS_BACK));
        }
        return mProgressDialog;
    }


    private void bindService() {
        MyLogger.logI(CLASS_TAG, "Anuj in bindService");
        boolean b = getActivity().getApplicationContext().bindService
        (new Intent(getActivity(), BackupService.class),
                mServiceCon, Service.BIND_AUTO_CREATE);
        if (b)
            MyLogger.logI(CLASS_TAG, "Anuj bindService is true");
        else
            MyLogger.logI(CLASS_TAG, "Anuj bindService is false");
    }

    private ServiceConnection mServiceCon = new ServiceConnection() {
        @Override
        public void onServiceConnected(final ComponentName name, final IBinder service) {
            MyLogger.logI(CLASS_TAG, "Anuj onServiceConnected");
            mBackupService = (BackupBinder) service;
            if (mBackupService != null) {
                if (mBackupListener != null) {
                    mBackupService.setOnBackupChangedListner(mBackupListener);
                }
            }
            // checkBackupState();
            afterServiceConnected();
            MyLogger.logI(CLASS_TAG, "onServiceConnected");
        }

        @Override
        public void onServiceDisconnected(final ComponentName name) {
            mBackupService = null;
            MyLogger.logI(CLASS_TAG, "onServiceDisconnected");
        }
    };

    private void afterServiceConnected() {
        mBackupListener = new PersonalDataBackupStatusListener();
        setOnBackupStatusListener(mBackupListener);
        checkBackupState();
    }

    private void checkBackupState() {
        if (mBackupService != null) {
            int state = mBackupService.getState();
            switch (state) {
                case State.RUNNING:
                case State.PAUSE:
                    BackupProgress p = mBackupService.getCurBackupProgress();
                    MyLogger.logE(CLASS_TAG, "checkBackupState: Max = " + p.mMax + " curprogress = "
                            + p.mCurNum);
                    if (state == State.RUNNING) {
                        mProgressDialog.show();
                    }
                    if (p.mCurNum < p.mMax) {
                        String msg = getProgressDlgMessage(p.mType);
                        if (mProgressDialog != null) {
                            mProgressDialog.setMessage(msg);
                        }
                    }
                    if (mProgressDialog != null) {
                        mProgressDialog.setMax(p.mMax);
                        mProgressDialog.setProgress(p.mCurNum);
                    }
                    break;
                case State.FINISH:
                    showBackupResult(mBackupService.getBackupResultType(),
                            mBackupService.getBackupResult());
                    MyLogger.logD(CLASS_TAG, "state = finish");
                    break;
                default:
                    //super.checkBackupState();
                    break;
            }
        }
    }


    private void setOnBackupStatusListener(OnBackupStatusListener listener) {
        mBackupListener = listener;
        if (mBackupListener != null && mBackupService != null) {
            mBackupService.setOnBackupChangedListner(mBackupListener);
        }
    }


    @Override
    public void onStart() {
        super.onStart();
        MyLogger.logI(CLASS_TAG, "onStart");
        try {
            if (SDCardUtils.getExternalStoragePath(getActivity()) == null) {
                MyLogger.logD(CLASS_TAG, "onStart NO SDCARD! cancel mInitDataTask ");
                getActivity().finish();
                return;
            }
        } catch (java.lang.SecurityException e) {
            MyLogger.logW(CLASS_TAG, "Permission not satisified");
            /*Toast.makeText(
                    getActivity(),
                    R.string.permission_not_satisfied_exit,
                    Toast.LENGTH_SHORT).show();
            Utils.exitLockTaskModeIfNeeded(getActivity());
            getActivity().finish();*/
        }

        // Seems more suitable at listItemClick
        // When permission are stopped from recents
        //mInitMessageTask = new InitMessageTask();
        //mInitMessageTask.execute();
    }

    @Override
    public void onStop() {
        super.onStop();
        MyLogger.logD(CLASS_TAG, "onStop()");
        //Need check why (show toast when switch to restore when SD full)
        //SDCardUtils.broadcastSDRemovedOrFullIfNecessary(getActivity());
    }


    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        MyLogger.logI(CLASS_TAG, "onAttach");
        mMainActivity = activity;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        // will do some init operation
        MyLogger.logD(CLASS_TAG, "onActivityCreated()");
        initAdapter();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        MyLogger.logI(CLASS_TAG, "onDestroy");
        if (mProgressDialog != null && mProgressDialog.isShowing() && getActivity().isFinishing()) {
            mProgressDialog.dismiss();
        }

        //todo
        //unRegisteSDCardListener();

        if (mBackupService != null && mBackupService.getState() == State.RUNNING
         && getActivity().isFinishing()) {
            stopService();
            Toast.makeText(getActivity().getApplicationContext(),
             R.string.backup_stopped, Toast.LENGTH_SHORT).show();
            MyLogger.logI(CLASS_TAG, "Anuj inside show toast");
        }

        if (mBackupService != null && mBackupService.getState() == State.INIT) {
            stopService();
        }

        if (getActivity().isFinishing()) {
            unBindService();
            mHandler = null;
            if (mBackupService != null) {
                mBackupService.setOnBackupChangedListner(null);
            }
            ExitActivity.exitApplication(getActivity());
        }

    }


    private void unBindService() {
        MyLogger.logI(CLASS_TAG, "Service Unbind");
        if (mBackupService != null) {
            mBackupService.setOnBackupChangedListner(null);
        }
        getActivity().getApplicationContext().unbindService(mServiceCon);
    }

    public void onPause() {
        super.onPause();
        MyLogger.logI(CLASS_TAG, "onPasue");
    }

    @Override
    public void onResume() {
        super.onResume();
        MyLogger.logI(CLASS_TAG, "onResume");
    }

    @Override
    public void onDetach() {
        super.onDetach();
        MyLogger.logI(CLASS_TAG, "onDetach");
    }

    public void initAdapter() {
        String[] content = new String[]{getString(R.string.backup_messages)};

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this.getActivity(),
                                                android.R.layout.simple_list_item_1, content);
        MyLogger.logD(CLASS_TAG, "initAdapter() --> setListAdapter");
        setListAdapter(adapter);
    }



    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        String item = (String) l.getAdapter().getItem(position);
        MyLogger.logI(CLASS_TAG, "onLIstItemClick: item is " + item);
        if (SDCardUtils.getExternalStoragePath(getActivity()) == null) {
            Utils.exitLockTaskModeIfNeeded(getActivity());
            getActivity().finish();
            return;
        }

        if (item.equals(getString(R.string.backup_messages))) {
            /*startService();
            ArrayList<String> params = new ArrayList<String>();
            params.add(Constants.ModulePath.NAME_SMS);
            params.add(Constants.ModulePath.NAME_MMS);
            mBackupService.setBackupItemParam(ModuleType.TYPE_MESSAGE, params);*/
            // Backup Messages
                /*if (mBackupService == null || mBackupService.getState() != State.INIT) {
                    MyLogger.logE(CLASS_TAG,
                                    "Can not to start. BackupService not " +
                                    "ready or BackupService is ruuning");
                    return;
                }*/
                mInitMessageTask = new InitMessageTask();
                mInitMessageTask.execute();
            }
          else {
            // for extends
            return;
        }
    }

    private void showToastSDRemovedOrFull() {
        Toast.makeText(getActivity(), R.string.toast_sd_removed_or_full, Toast.LENGTH_SHORT).show();
    }

    private static void doKeepDialog(Dialog dialog) {
        WindowManager.LayoutParams lp = new WindowManager.LayoutParams();
        lp.copyFrom(dialog.getWindow().getAttributes());
        lp.width = WindowManager.LayoutParams.WRAP_CONTENT;
        lp.height = WindowManager.LayoutParams.WRAP_CONTENT;
        dialog.getWindow().setAttributes(lp);
    }


    public void startBackup() {
        MyLogger.logD(CLASS_TAG, "startBackup");
        /*LayoutInflater factory = LayoutInflater.from(getActivity());
        final View view = factory.inflate(R.layout.dialog_edit_folder_name, null);
        EditText editor = (EditText) view.findViewById(R.id.edit_folder_name);
        if (editor != null) {
            SimpleDateFormat dateFormat = new SimpleDateFormat(
                    "yyyyMMddHHmmss");
            String dateString = dateFormat.format(
                    new Date(System.currentTimeMillis()));
            editor.setText(dateString);
        }*/
        if (mAlertFolderName != null) {
            mAlertFolderName.dismiss();
        }
        mAlertFolderName = createFolderEditorDialog();
        mAlertFolderName.setCanceledOnTouchOutside(false);
        mAlertFolderName.show();
        doKeepDialog(mAlertFolderName);
    }

    private AlertDialog createFolderEditorDialog() {

        LayoutInflater factory = LayoutInflater.from(getActivity());
        final View view = factory.inflate(R.layout.dialog_edit_folder_name, null);
        EditText editor = (EditText) view.findViewById(R.id.edit_folder_name);
        if (editor != null) {
            SimpleDateFormat dateFormat = new SimpleDateFormat(
                    "yyyyMMddHHmmss");
            String dateString = dateFormat.format(
                    new Date(System.currentTimeMillis()));
            editor.setText(dateString);
            editor.setSelection(editor.getText().length());
        }
        final AlertDialog dialog = new AlertDialog.Builder(getActivity())
                .setTitle(R.string.edit_folder_name).setView(view)
                .setPositiveButton(android.R.string.ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int whichButton) {
                        AlertDialog d = (AlertDialog) dialog;
                        EditText editor = (EditText) d.findViewById(R.id.edit_folder_name);
                        if (editor != null) {
                            final CharSequence folderName = editor.getText();
                            new Thread(new Runnable() {
                                public void run() {
                                    processClickStart(folderName);
                                }

                            }).start();
                            hideKeyboard(editor);
                            editor.setText("");
                        } else {
                            MyLogger.logE(CLASS_TAG, " can not get folder name");
                        }
                    }

                }).setNegativeButton(android.R.string.cancel, null).create();

        editor.addTextChangedListener(new TextWatcher() {

            @Override
            public void onTextChanged(CharSequence s, int start, int before,
                    int count) {
            }

            @Override
            public void beforeTextChanged(CharSequence s, int start, int count,
                    int after) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                if (s.toString().length() <= 0
                        || s.toString().matches(".*[/\\\\:;&%#*?\"<>|].*")
                        || s.toString().matches(" *\\.+ *")
                        || s.toString().trim().length() == 0) { // characters
                    // not allowed
                    dialog.getButton(Dialog.BUTTON_POSITIVE).setEnabled(false);
                } else {
                    dialog.getButton(Dialog.BUTTON_POSITIVE).setEnabled(true);
                }
            }
        });
        //mDialogs.add(dialog);
        return dialog;
    }

    private void hideKeyboard(EditText editor) {
        // TODO Auto-generated method stub
        InputMethodManager imm = ((InputMethodManager) getActivity().getSystemService(
                getActivity().INPUT_METHOD_SERVICE));
        imm.hideSoftInputFromWindow(editor.getWindowToken(), 0);
    }

    private void processClickStart(CharSequence folderName) {

        String path = SDCardUtils
                .getPersonalDataBackupPath(getActivity());
        if (path == null) {
            return;
        }
        /** M: Bug Fix for CR ALPS01694645 @{ */
        String pathSD = SDCardUtils
                .getStoragePath(getActivity());
        if (SDCardUtils.getAvailableSize(pathSD) <= SDCardUtils.MINIMUM_SIZE) {
            // no space
            MyLogger.logD(CLASS_TAG, "SDCard is full");
            Message msg = toastHandler.obtainMessage();
            msg.arg1 = SD_CARD_FULL;
            toastHandler.sendMessage(msg);
            //mUiHandler.obtainMessage(DialogID.DLG_SDCARD_FULL).sendToTarget();
            return;
        }
        /** @} */
        StringBuilder builder = new StringBuilder(path);
        builder.append(File.separator);
        builder.append(folderName);
        builder.append(File.separator);
        mBackupFolderPath = builder.toString();
        MyLogger.logD(CLASS_TAG, "[processClickStart] mBackupFolderPath is " + mBackupFolderPath);
        File folder = new File(mBackupFolderPath);
        File[] files = null;
        if (folder.exists()) {
            files = folder.listFiles();
        }

        if (files != null && files.length > 0) {
            MyLogger.logD(CLASS_TAG, "[processClickStart] DLG_BACKUP_CONFIRM_OVERWRITE Here! ");
            Message msg = toastHandler.obtainMessage();
            msg.arg1 = NAME_EXISTS;
            toastHandler.sendMessage(msg);
            //mUiHandler.obtainMessage(DialogID.DLG_BACKUP_CONFIRM_OVERWRITE).sendToTarget();
            return;
        } else {
            //Message msg = toastHandler.obtainMessage();
            //msg.arg1 = 3;
            //toastHandler.sendMessage(msg);
            startPersonalDataBackup(mBackupFolderPath);
        }
    }

    private void startPersonalDataBackup(String folderName) {
        if (folderName == null || folderName.trim().equals("")) {
            return;
        }

        /*if (!hasSelectedItem()) {
            return;
        }*/

        startService();
        if (mBackupService != null) {
            ArrayList<Integer> list = new ArrayList<Integer>();
            list.add(ModuleType.TYPE_MESSAGE);
            mBackupService.setBackupModelList(list);
            //int defaultMax = 0;
            //String defaultModule = null;
            if (list.contains(ModuleType.TYPE_MESSAGE)) {
                ArrayList<String> params = new ArrayList<String>();
                //if (mMessageCheckTypes[0]) {
                    params.add(Constants.ModulePath.NAME_SMS);
                //}
                //if (mMessageCheckTypes[1]) {
                    params.add(Constants.ModulePath.NAME_MMS);
                //}
                mBackupService.setBackupItemParam(ModuleType.TYPE_MESSAGE, params);
            }
            boolean ret = mBackupService.startBackup(folderName);
            if (!ret) {
                String path = SDCardUtils.getStoragePath(getActivity());
                if (path == null) {
                    // no sdcard
                    MyLogger.logD(CLASS_TAG, "SDCard is removed");
                    ret = true;
                } else if (SDCardUtils.getAvailableSize(path) <= SDCardUtils.MINIMUM_SIZE) {
                    // no space
                    MyLogger.logD(CLASS_TAG, "SDCard is full");
                    ret = true;
                    Message msg = toastHandler.obtainMessage();
                    msg.arg1 = SD_CARD_FULL;
                    toastHandler.sendMessage(msg);
                    //mUiHandler.obtainMessage(DialogID.DLG_SDCARD_FULL).sendToTarget();
                } else {
                    MyLogger.logE(CLASS_TAG, "Unknown error");

                    Message msg = toastHandler.obtainMessage();
                    msg.arg1 = CREATE_FOLDER_FAIL;
                    toastHandler.sendMessage(msg);

                    //Bundle b = new Bundle();
                    //b.putString("name", folderName.substring(folderName.lastIndexOf('/') + 1));
                    //mUiHandler.obtainMessage(DialogID.DLG_CREATE_FOLDER_FAILED, b).sendToTarget();
                }
                stopService();
            }
        } else {
            stopService();
            MyLogger.logE(CLASS_TAG, "startPersonalDataBackup: error! service is null");
        }
    }

    private void startService() {
        getActivity().startService(new Intent(getActivity(), BackupService.class));
    }

    private void stopService() {
        if (mBackupService != null) {
            mBackupService.reset();
        }
        getActivity().stopService(new Intent(getActivity(), BackupService.class));
    }


    private class InitMessageTask extends AsyncTask<Void, Void, Long> {
        private static final String TASK_TAG = "InitMessageTask";

        @Override
        protected void onPreExecute() {
            super.onPreExecute();
            //MyLogger.logD(CLASS_TAG, TASK_TAG + "---onPreExecute");
            //showLoadingContent(true);
            //setTitle(R.string.backup_personal_data);
            //setButtonsEnable(false);
        }

        @Override
        protected void onPostExecute(Long arg0) {
            //showLoadingContent(false);
            //setButtonsEnable(true);
            //updateData(mBackupDataList);
            //setOnBackupStatusListener(mBackupListener);
            //MyLogger.logD(
                    //CLASS_TAG,
                    //"---onPostExecute----getTitle " + PersonalDataBackupActivity.this.getTitle());
            super.onPostExecute(arg0);

            if (countMsg == 0) {
                if (mToast != null) {
                    mToast.cancel();
                }
                mToast = Toast.makeText(getActivity(),
                    R.string.no_messages_to_backup, Toast.LENGTH_SHORT);
                mToast.show();
                return;
            }
            String path =
                    SDCardUtils.getStoragePath(getActivity());
            MyLogger.logI(CLASS_TAG, "Storage path is " + path);
            if (path != null) {
                if (Utils.getWorkingInfo() < 0) {
                    startBackup();
                } else {
                    ///TODO:
                    //showDialog(DialogID.DLG_RUNNING);
                }
            } else {
                MyLogger.logE(CLASS_TAG, "SD removed or is full, do nothing");
                showToastSDRemovedOrFull();
            }


        }

        @Override
        protected Long doInBackground(Void... arg0) {
            try {
                    Composer composer;
                    int countSMS = 0;
                    int countMMS = 0;
                    int count = 0;
                    composer = new SmsBackupComposer(mMainActivity);
                    if (composer.init()) {
                        countSMS = composer.getCount();
                        composer.onEnd();
                    }
                    if (countSMS != 0) {
                        //mMessageEnable.add(getString(R.string.message_sms));
                    }
                    composer = new MmsBackupComposer(mMainActivity);
                    if (composer.init()) {
                        countMMS = composer.getCount();
                        composer.onEnd();
                    }
                    count = countSMS + countMMS;
                    MyLogger.logD(
                            CLASS_TAG,
                            "countSMS = " + countSMS + "countMMS" + countMMS);
                    if (countMMS != 0) {
                        //mMessageEnable.add(getString(R.string.message_mms));
                    }
                    if (count == 0) {
                        Message msg = toastHandler.obtainMessage();
                        msg.arg1 = NO_MESSAGES;
                        toastHandler.sendMessage(msg);
                    }
                    else {
                        Message msg = toastHandler.obtainMessage();
                        msg.arg1 = MESSAGES_PRESENT;
                        msg.arg2 = count;
                        toastHandler.sendMessage(msg);
                    }

                } catch (java.lang.SecurityException e) {
                    MyLogger.logW(CLASS_TAG, "Permission not satisified in InitMessageTask");
                        /*mHandler.post(new Runnable() {
                            @Override
                            public void run() {
                                Toast.makeText(
                                        getActivity(),
                                        R.string.permission_not_satisfied_exit,
                                        Toast.LENGTH_SHORT).show();
                                //Utils.exitLockTaskModeIfNeeded(PersonalDataBackupActivity.this);
                                getActivity().finish();
                            }
                        });*/
                }

            return null;
        }
    }

    protected boolean errChecked() {
        boolean result = errCheckedUtil();
        if (result) {
                //NotifyManager.getInstance(getActivity())
                  //                      .clearNotification();
        }
        return result;
    }

    protected boolean errCheckedUtil() {
        boolean ret = false;

        boolean isSDCardMissing = SDCardUtils.isSdCardMissing(getActivity());
        String path = SDCardUtils.getStoragePath(getActivity());

        if (isSDCardMissing) {
            MyLogger.logI(CLASS_TAG, "SDCard is removed");
            //stopService(new Intent(AbstractBackupActivity.this, BackupService.class));
            getActivity().stopService(new Intent(getActivity(), BackupService.class));
            //Utils.exitLockTaskModeIfNeeded(AbstractBackupActivity.this);
            Utils.exitLockTaskModeIfNeeded(getActivity());
            //todo
            //AbstractBackupActivity.this.finish();
        } else if (SDCardUtils.getAvailableSize(path) <= SDCardUtils.MINIMUM_SIZE) {
            ret = true;
            MyLogger.logI(CLASS_TAG, "errChecked(): SDCard is full");
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        //todo
                        //AbstractBackupActivity.this.showDialog(DialogID.DLG_SDCARD_FULL);
                    }
                });
            }
        } else {
            MyLogger.logE(CLASS_TAG, "Unkown error, don't pause.");
        }
        return ret;
    }

    private void showBackupResult(final BackupResultType result,
            final ArrayList<ResultEntity> list) {
        MyLogger.logI(CLASS_TAG, "showBackupResult");
        if (mProgressDialog != null && mProgressDialog.isShowing()) {
            mProgressDialog.dismiss();
        }

        /*if (mCancelDlg != null && mCancelDlg.isShowing()) {
            mCancelDlg.dismiss();
        }*/

        if (result != BackupResultType.Cancel) {
            Bundle args = new Bundle();
            args.putParcelableArrayList(Constants.RESULT_KEY, list);
            try {
                //showDialog(DialogID.DLG_RESULT, args);
                MyLogger.logI(CLASS_TAG, "Result backup number of modules = " + list.size());
                ResultEntity item = list.get(0);
                int resId = R.string.backup_success;
                switch (item.getResult()) {
                case ResultEntity.NO_CONTENT:
                    //resId = R.string.result_no_content;
                    //break;

                case ResultEntity.FAIL:
                    resId = R.string.backup_fail;
                    break;

                default:
                    resId = R.string.backup_success;
                    break;
                }

                Toast.makeText(getActivity(), resId, Toast.LENGTH_SHORT).show();
                stopService();
            } catch (BadTokenException e) {
                MyLogger.logE(CLASS_TAG, "BadTokenException");
            }

        } else {
            stopService();
        }
    }

    private void showProgress(int defaltMax, String defaltModule) {
        if (mProgressDialog == null) {
            mProgressDialog = createProgressDlg();
        }
        MyLogger.logI(CLASS_TAG, "no need to set max");
        if (defaltMax != 0 && defaltModule != null) {
            mProgressDialog.setMax(defaltMax);
            mProgressDialog.setMessage(defaltModule);
        }
        if (!getActivity().isFinishing()) {
            try {
                mProgressDialog.show();
            } catch (BadTokenException e) {
                MyLogger.logE(CLASS_TAG, " BadTokenException :" + e.toString());
            }
        }
    }

    protected String getProgressDlgMessage(final int type) {
        StringBuilder builder = new StringBuilder(getString(R.string.backuping));
        builder.append("(");
        builder.append(ModuleType.getModuleStringFromType(getActivity(), type));
        builder.append(")");
        return builder.toString();
    }


    public class PersonalDataBackupStatusListener implements OnBackupStatusListener {
        @Override
        public void onBackupEnd(final BackupResultType resultCode,
                final ArrayList<ResultEntity> resultRecord,
                final ArrayList<ResultEntity> appResultRecord) {

            if (resultCode != BackupResultType.Cancel) {
                RecordXmlInfo backupInfo = new RecordXmlInfo();
                backupInfo.setRestore(false);
                backupInfo.setDevice(Utils.getPhoneSearialNumber());
                backupInfo.setTime(String.valueOf(System.currentTimeMillis()));
                RecordXmlComposer xmlCompopser = new RecordXmlComposer();
                xmlCompopser.startCompose();
                xmlCompopser.addOneRecord(backupInfo);
                xmlCompopser.endCompose();
                if (mBackupFolderPath != null && !mBackupFolderPath.isEmpty()) {
                    Utils.writeToFile(xmlCompopser.getXmlInfo(), mBackupFolderPath + File.separator
                            + Constants.RECORD_XML);
                }
            } else {
                MyLogger.logE(CLASS_TAG, "ResultCode is cancel, not write record.xml");
            }

            final BackupResultType iResultCode = resultCode;
            final ArrayList<ResultEntity> iResultRecord = resultRecord;
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        showBackupResult(iResultCode, iResultRecord);
                    }
                });
            }
        }

        @Override
        public void onComposerChanged(final Composer composer) {
            if (composer == null) {
                MyLogger.logE(CLASS_TAG, "onComposerChanged: error[composer is null]");
                return;
            } else {
                MyLogger.logI(CLASS_TAG, "onComposerChanged: type = " + composer.getModuleType()
                        + "Max = " + composer.getCount());
            }
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        String msg = getProgressDlgMessage(composer.getModuleType());
                        MyLogger.logD(CLASS_TAG, "mProgressDialog : " + mProgressDialog);
                        if (mProgressDialog != null
                                && !mProgressDialog.isShowing()) {
                            if (mBackupService != null
                                    && mBackupService.getState() != State.PAUSE) {
                                showProgress(0, null);
                            }
                        }
                        if (mProgressDialog != null) {
                            mProgressDialog.setMessage(msg);
                            mProgressDialog.setMax(composer.getCount());
                            mProgressDialog.setProgress(0);
                        }
                    }
                });
            }
        }

        @Override
        public void onBackupErr(final IOException e) {
            if (errChecked()) {
                if (mBackupService != null &&
                    mBackupService.getState() != State.INIT &&
                    mBackupService.getState() != State.FINISH) {
                    mBackupService.pauseBackup();
                }
            }
        }

        @Override
        public void onProgressChanged(final Composer composer, final int progress) {
            if (mHandler != null) {
                mHandler.post(new Runnable() {
                    @Override
                    public void run() {
                        if (mProgressDialog != null) {
                            mProgressDialog.setProgress(progress);
                        }
                    }
                });
            }
        }


    }


}



