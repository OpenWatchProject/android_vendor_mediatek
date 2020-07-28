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

import android.content.Context;
import android.widget.Toast;

//import com.mediatek.datatransfer.modules.AppBackupComposer;
//import com.mediatek.datatransfer.modules.BookmarkBackupComposer;
//import com.mediatek.datatransfer.modules.CalendarBackupComposer;
import com.mediatek.op18.mms.modules.Composer;
//import com.mediatek.datatransfer.modules.ContactBackupComposer;
import com.mediatek.op18.mms.modules.MessageBackupComposer;
//import com.mediatek.mms.plugin.modules.MmsBackupComposer;
//import com.mediatek.datatransfer.modules.MusicBackupComposer;
//import com.mediatek.datatransfer.modules.NoteBookBackupComposer;
//import com.mediatek.datatransfer.modules.PictureBackupComposer;
import com.mediatek.op18.mms.ProgressReporter;
import com.mediatek.op18.mms.utils.Constants;
import com.mediatek.op18.mms.utils.FileUtils;
import com.mediatek.op18.mms.utils.ModuleType;
import com.mediatek.op18.mms.utils.MyLogger;
import com.mediatek.op18.mms.utils.Utils;

import com.mediatek.op18.mms.R;

import java.io.File;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;

public class BackupEngine {
    private static final String CLASS_TAG = MyLogger.LOG_TAG + "/BackupEngine";

    public interface OnBackupDoneListner {
        public void onFinishBackup(BackupResultType result);
    }

    public enum BackupResultType {
        Success, Fail, Error, Cancel
    }

    private Context mContext;
    private ProgressReporter mProgressReporter;
    private List<Composer> mComposerList;
    private OnBackupDoneListner mBackupDoneListner;
    private boolean mIsRunning = false;
    private long mThreadIdentifier = -1;
    private boolean mIsPause = false;
    private boolean mIsCancel = false;
    private Object mLock = new Object();
    private String mBackupFolder;
    HashMap<Integer, ArrayList<String>> mParasMap = new HashMap<Integer, ArrayList<String>>();

    private static BackupEngine mSelfInstance;

    public static BackupEngine getInstance(final Context context, final ProgressReporter reporter) {
        if (mSelfInstance == null) {
            new BackupEngine(context, reporter);
        } else {
            mSelfInstance.updateInfo(context, reporter);
        }

        return mSelfInstance;
    }

    public BackupEngine(final Context context, final ProgressReporter reporter) {
        mContext = context;
        mProgressReporter = reporter;
        mComposerList = new ArrayList<Composer>();
        mSelfInstance = this;
    }

    ArrayList<Integer> mModuleList;

    public void setBackupModelList(ArrayList<Integer> moduleList) {
        reset();
        mModuleList = moduleList;
    }

    public void setBackupItemParam(int itemType, ArrayList<String> paraList) {
        mParasMap.put(itemType, paraList);
    }

    public boolean startBackup(final String folderName) {
        boolean startSuccess = true;
        mBackupFolder = folderName;
        MyLogger.logD(CLASS_TAG, "startBackup():" + folderName);

        Utils.isBackingUp = mIsRunning = true;
        if (setupComposer(mModuleList)) {
            mThreadIdentifier = System.currentTimeMillis();
            new BackupThread(mThreadIdentifier).start();
        } else {
            Utils.isBackingUp = mIsRunning = false;
            startSuccess = false;
        }
        return startSuccess;
    }

    public final boolean isRunning() {
        return mIsRunning;
    }

    private final void updateInfo(final Context context, final ProgressReporter reporter) {
        mContext = context;
        mProgressReporter = reporter;
    }

    public final void pause() {
        mIsPause = true;
    }

    public final boolean isPaused() {
        return mIsPause;
    }

    public final void continueBackup() {
        synchronized (mLock) {
            if (mIsPause) {
                mIsPause = false;
                mLock.notify();
            }
        }
    }

    public final void cancel() {
        if (mComposerList != null && mComposerList.size() > 0) {
            for (Composer composer : mComposerList) {
                composer.setCancel(true);
            }
            mIsCancel = true;
            continueBackup();
        }
    }

    public final void setOnBackupDoneListner(final OnBackupDoneListner listner) {
        mBackupDoneListner = listner;
    }

    private void addComposer(final Composer composer) {
        if (composer != null) {
            int type = composer.getModuleType();
            ArrayList<String> params = mParasMap.get(type);
            if (params != null) {
                MyLogger.logD(CLASS_TAG, "Params size is " + params);
                composer.setParams(params);
            } else {
                MyLogger.logD(CLASS_TAG, "Params is null");
            }
            composer.setReporter(mProgressReporter);
            composer.setParentFolderPath(mBackupFolder);
            mComposerList.add(composer);
        }
    }

    private void reset() {
        if (mComposerList != null) {
            mComposerList.clear();
        }

        if (mParasMap != null) {
            mParasMap.clear();
        }

        mIsPause = false;
        mIsCancel = false;
    }

    private boolean setupComposer(final ArrayList<Integer> list) {
        MyLogger.logD(CLASS_TAG, "setupComposer begin...");

        boolean result = true;
        File path = new File(mBackupFolder);
        if (!path.exists()) {
            result = path.mkdirs();
        }
        MyLogger.logD(CLASS_TAG, "makedir end...");
        if (result) {
            MyLogger.logD(CLASS_TAG, "create folder " + mBackupFolder + " success");

            for (int type : list) {
                switch (type) {
                case ModuleType.TYPE_CONTACT:
                    //addComposer(new ContactBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_CALENDAR:
                    //addComposer(new CalendarBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_SMS:
                    //addComposer(new SmsBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_MMS:
                    //todo
                    //addComposer(new MmsBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_MESSAGE:
                    //todo
                    MyLogger.logD(CLASS_TAG, "Anuj ModuleType.TYPE_MESSAGE adding composer");
                    addComposer(new MessageBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_APP:
                    //addComposer(new AppBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_PICTURE:
                    //addComposer(new PictureBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_MUSIC:
                    //addComposer(new MusicBackupComposer(mContext));
                    break;

                case ModuleType.TYPE_NOTEBOOK:
                    //addComposer(new NoteBookBackupComposer(mContext));
                    break;

//                case ModuleType.TYPE_BOOKMARK:
//                    addComposer(new BookmarkBackupComposer(mContext));
//                    break;

                default:
                    result = false;
                    break;
                }
            }

            MyLogger.logD(CLASS_TAG, "setupComposer finish");
        } else {
            MyLogger.logE(CLASS_TAG, "setupComposer failed");
            result = false;
        }
        return result;
    }

    private class BackupThread extends Thread {
        private final long mId;
        public BackupThread(long id) {
            super();
            mId = id;
        }

        @Override
        public void run() {
            try {
                BackupResultType result = BackupResultType.Fail;

                MyLogger.logD(CLASS_TAG, "BackupThread begin...");
                for (Composer composer : mComposerList) {
                    MyLogger.logD(
                            CLASS_TAG,
                            "BackupThread->composer:" + composer.getModuleType() + " start...");
                    if (!composer.isCancel() && mId == mThreadIdentifier) {
                        composer.init();
                        composer.onStart();
                        MyLogger.logD(
                            CLASS_TAG,
                            "BackupThread->composer:" + composer.getModuleType() + " init finish");
                        while (!composer.isAfterLast() &&
                               !composer.isCancel() &&
                               mId == mThreadIdentifier) {
                            if (mIsPause) {
                                synchronized (mLock) {
                                    try {
                                        MyLogger.logD(CLASS_TAG, "BackupThread wait...");
                                        while (mIsPause) {
                                            mLock.wait();
                                        }
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                                }
                            }
                            if (!composer.isCancel()) {
                                composer.composeOneEntity();
                                MyLogger.logD(CLASS_TAG, "BackupThread->composer:"
                                        + composer.getModuleType() + " compose one entiry");
                            }
                        }
                    }

                    try {
                        sleep(Constants.TIME_SLEEP_WHEN_COMPOSE_ONE);
                    } catch (InterruptedException e) {
                        e.printStackTrace();
                    }

                    composer.onEnd();

                    // send scan file request (Refactoring)
                    sendScanFileRequests(composer.getModuleType(), mContext);

                    // generateModleXmlInfo(composer);

                    MyLogger.logD(
                            CLASS_TAG,
                            "BackupThread-> composer:" + composer.getModuleType() + " finish");
                }

                Utils.isBackingUp = mIsRunning = false;
                if (mIsCancel) {
                    result = BackupResultType.Cancel;
                    if (!mModuleList.contains(ModuleType.TYPE_APP)) {
                        try {
                            deleteFolder(new File(mBackupFolder));
                        } catch (NullPointerException e) {
                            e.fillInStackTrace();
                        }
                    }
                } else {
                    result = BackupResultType.Success;
                }
                MyLogger.logD(CLASS_TAG, "BackupThread run finish, result:" + result);

                if (mBackupDoneListner != null) {
                    if (mIsPause) {
                        synchronized (mLock) {
                            try {
                                MyLogger.logD(CLASS_TAG, "BackupThread wait before end...");
                                while (mIsPause) {
                                    mLock.wait();
                                }
                                if (mIsCancel) {
                                    result = BackupResultType.Cancel;
                                    if (!mModuleList.contains(ModuleType.TYPE_APP)) {
                                        try {
                                            if (new File(mBackupFolder) != null
                                                    && new File(mBackupFolder).exists()) {
                                                deleteFolder(new File(mBackupFolder));
                                            }
                                        } catch (NullPointerException e) {
                                            e.fillInStackTrace();
                                        }
                                    }
                                }
                            } catch (InterruptedException e) {
                                e.printStackTrace();
                            }
                        }
                    }
                    mBackupDoneListner.onFinishBackup(result);
                }
            } catch (java.lang.SecurityException e) {
                e.printStackTrace();
                Toast.makeText(
                        mContext,
                        R.string.permission_not_satisfied_exit,
                        Toast.LENGTH_SHORT).show();
                android.os.Process.killProcess(android.os.Process.myPid());
            }
        }

        private void sendScanFileRequests(int moduleType, Context mContext) {
            String backupFolderName = ModuleType.sModuleTpyeFolderInfo.get(moduleType);
            MyLogger.logD(CLASS_TAG, "Anuj sendScanFileRequests :"
             + backupFolderName + "module = " + moduleType);
            if (moduleType == ModuleType.TYPE_APP) {
                FileUtils.scanPathforMediaStore(mBackupFolder, mContext);
            } else if (backupFolderName != null) {
                String path = mBackupFolder + File.separator + backupFolderName;
                FileUtils.scanPathforMediaStore(path, mContext);
            }

            if (moduleType == ModuleType.TYPE_MESSAGE) {
                String path = mBackupFolder + ModuleType.sModuleTpyeFolderInfo.
                get(ModuleType.TYPE_SMS);
                MyLogger.logD(CLASS_TAG, "Anuj sendScanFileRequests path :" + path);
                FileUtils.scanPathforMediaStore(path, mContext);

                path = mBackupFolder + ModuleType.sModuleTpyeFolderInfo.get(ModuleType.TYPE_MMS);
                MyLogger.logD(CLASS_TAG, "Anuj sendScanFileRequests path :" + path);
                FileUtils.scanPathforMediaStore(path, mContext);
            }
        }
    }

    private void deleteFolder(File file) {
        if (file.exists()) {
            if (file.isFile()) {
                file.delete();
            } else if (file.isDirectory()) {
                File files[] = file.listFiles();
                for (int i = 0; i < files.length; ++i) {
                    this.deleteFolder(files[i]);
                }
            }

            file.delete();
        }
    }

}
