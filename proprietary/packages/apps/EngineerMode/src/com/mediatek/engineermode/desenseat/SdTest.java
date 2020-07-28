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

package com.mediatek.engineermode.desenseat;

import android.annotation.SuppressLint;
import android.content.Context;
import android.os.Environment;
import android.os.StatFs;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;

import com.mediatek.engineermode.Elog;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.Random;
import java.util.Vector;

/**
 * Description: Test SD card by constantly reading and writing.
 *
 * @author mtk54043
 *
 */
public class SdTest {
    private static final String TAG = "DesenseAT/sdtest";
    private static final int FILECOUNT_MAX = 200;
    private static final int AVAILABLESPACE = 4193304;
    private static final int COUNT = 10;
    private static final int PRE_FILE_SIZE = 256;
    private static final String FODERNAME = "EM_SDLog";
    private static final String FILENAME = "EM_SDLOG_TESTFILE";
    static final int TYPE_PREPARE = 0;
    static final int TYPE_READ = 1;
    static final int TYPE_WRITE = 2;
    static final int STORAGE_INTERNAL = 0;
    static final int STORAGE_SDCARD = 1;
    private Context mContext;
    private int mFileCount = 0;
    private Vector<String> mFileList;
    private Random mRandom;
    private StorageManager mStorageManager;
    private int mSelectedIndex;
    private boolean mPrepared;

    /**
     * Constructor function.
     * @param context the context of the test
     */
    public SdTest(Context context) {
        mContext = context;
    }

    boolean doApiTest(int storage) {
        mSelectedIndex = selectStorage(storage);
        if (mSelectedIndex < 0) {
            return false;
        }
        if (!checkSDCard()) {
            return false;
        }
        if (isSdWriteable()) {
            File testFile = new File(getSdPath() + File.separator + FILENAME);
            if (!testFile.exists()) {
                try {
                    testFile.createNewFile();
                    Elog.i(TAG, "CreateAndWriteFile :" + testFile.getPath());
                } catch (IOException e) {
                    e.printStackTrace();
                    return false;
                }
            }

            FileOutputStream outputStream = null;
            try {
                outputStream = new FileOutputStream(testFile);
                try {
                    for (int i = 0; i < COUNT; i++) {
                        outputStream.write(SDLOG_TEXT.getBytes());
                    }
                    outputStream.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                    return false;
                } finally {
                    try {
                        if (null != outputStream) {
                            outputStream.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                        return false;
                    }
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                return false;
            }
            FileInputStream inputStream = null;
            try {
                inputStream = new FileInputStream(testFile);
                byte[] buffer = new byte[PRE_FILE_SIZE];
                try {
                    int len = inputStream.read(buffer);
                    while (len != -1) {
                        len = inputStream.read(buffer);
                    }
                } catch (IOException e) {
                    e.printStackTrace();
                    return false;
                } finally {
                    try {
                        if (null != inputStream) {
                            inputStream.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                        return false;
                    }
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
                return false;
            }

            if (!testFile.delete()) {
                return false;
            }
        } else {
            return false;
        }

        return true;

    }

    void doTest(int type, int storage, TestItem item) {
        switch (type) {
        case TYPE_PREPARE:
            mSelectedIndex = selectStorage(storage);
            if (mSelectedIndex < 0) {
                return;
            }
            if (!checkSDCard()) {
                return;
            }
            mRandom = new Random();
            mFileList = new Vector<String>();
            mFileCount = 0;
            createFileForder();
            emptyForder(false);
            while (mFileCount < FILECOUNT_MAX && !item.isCancelled()) {
                createAndWriteFile();
            }
            mPrepared = true;
            break;
        case TYPE_READ:
            if (mPrepared) {
                readFile();
            }
            break;
        case TYPE_WRITE:
            if (mPrepared) {
                createAndWriteFile();
            }
            break;
        default:
            break;
        }
    }

    @SuppressLint("InlinedApi")
    private int selectStorage(int storage) {
        mStorageManager = (StorageManager) mContext.getSystemService(Context.STORAGE_SERVICE);
        StorageVolume[] volumes = mStorageManager.getVolumeList();
        for (int i = 0; i < volumes.length; i++) {
            boolean removable = volumes[i].isRemovable();
            if (storage == STORAGE_INTERNAL && !removable
                    || storage == STORAGE_SDCARD && removable) {
                Elog.d(TAG, "select " + volumes[i].getPath());
                return i;
            }
        }
        return -1;
    }

    private StorageVolume getMountedVolumeById(int index) {
        StorageVolume[] volumes = mStorageManager.getVolumeList();
        int mountedIndx = 0;
        for (int i = 0; i < volumes.length; i++) {
            String path = volumes[i].getPath();
            String state = mStorageManager.getVolumeState(path);
            if (state.equals(Environment.MEDIA_MOUNTED)) {
                if (mountedIndx == index) {
                    return volumes[i];
                }
                mountedIndx++;
            }
        }
        return null;
    }

    private boolean checkSDCard() {
        if (!isSdMounted()) {
            return false;
        } else if (!isSdWriteable()) {
            return false;
        } else if (getSdAvailableSpace() < AVAILABLESPACE) {
            return false;
        }
        return true;
    }

    private void emptyForder(boolean isDeleteForder) {
        File testForder = new File(getSdPath() + File.separator + FODERNAME);
        if (testForder.exists() && testForder.isDirectory()) {
            File[] fileList = testForder.listFiles();
            if (null != fileList) {
                for (File file : fileList) {
                    if (file.exists()) {
                        file.delete();
                        Elog.v(TAG, "Delete File :" + file.getPath());
                    }
                }
            }
            if (isDeleteForder) {
                testForder.delete();
            }
        }
    }

    private void createFileForder() {
        if (isSdMounted()) {
            File testForder = new File(getSdPath() + File.separator + FODERNAME);
            if (!testForder.exists()) {
                testForder.mkdir();
                Elog.i(TAG, "createFileForder: " + testForder.getPath());
            }
        }
    }

    private void createAndWriteFile() {
        if (getSdAvailableSpace() < AVAILABLESPACE) {
            emptyForder(false);
        }
        if (isSdWriteable()) {
            File testFile = new File(getSdPath() + File.separator
                    + FODERNAME + File.separator + FILENAME + mFileCount);
            if (!testFile.exists()) {
                try {
                    testFile.createNewFile();
                    Elog.i(TAG, "CreateAndWriteFile :" + testFile.getPath());
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }
            mFileList.add(FILENAME + mFileCount);
            mFileCount++;
            FileOutputStream outputStream = null;
            try {
                outputStream = new FileOutputStream(testFile);
                try {
                    for (int i = 0; i < COUNT; i++) {
                        outputStream.write(SDLOG_TEXT.getBytes());
                    }
                    outputStream.flush();
                } catch (IOException e) {
                    e.printStackTrace();
                } finally {
                    try {
                        if (null != outputStream) {
                            outputStream.close();
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                }
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
        }
    }

    private void readFile() {
        if (mFileList.size() > 0) {
            File readFile = new File(getSdPath() + File.separator + FODERNAME + File.separator
                    + mFileList.get(getRandom(mFileList.size())));
            Elog.i(TAG, "readFile: " + readFile.getPath());
            if (readFile.exists()) {
                FileInputStream inputStream = null;
                try {
                    inputStream = new FileInputStream(readFile);
                    byte[] buffer = new byte[PRE_FILE_SIZE];
                    try {
                        int len = inputStream.read(buffer);
                        while (len != -1) {
                            len = inputStream.read(buffer);
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        try {
                            if (null != inputStream) {
                                inputStream.close();
                            }
                        } catch (IOException e) {
                            e.printStackTrace();
                        }
                    }
                } catch (FileNotFoundException e) {
                    e.printStackTrace();
                }
            } else {
                Elog.w(TAG, "readFile doesn't exist!");
            }
        } else {
            createAndWriteFile();
        }
    }

    private int getRandom(int count) {
        if (count <= 0) {
            return 0;
        }

        return mRandom.nextInt(count);
    }

    private boolean isSdMounted() {
        // if (Environment.MEDIA_MOUNTED.equals(Environment
        // .getExternalStorageState())
        // || Environment.MEDIA_MOUNTED_READ_ONLY.equals(Environment
        // .getExternalStorageState())) {
        // return true;
        // } else {
        // return false;
        // }

        //return (Environment.MEDIA_MOUNTED.equals(Environment.getExternalStorageState())
        //|| Environment.MEDIA_MOUNTED_READ_ONLY.equals(Environment.getExternalStorageState()));
        String state;
        StorageVolume storage = getMountedVolumeById(mSelectedIndex);
        if (storage == null) {
            return false;
        }
        state = mStorageManager.getVolumeState(storage.getPath());
        return (Environment.MEDIA_MOUNTED.equals(state)
                        || Environment.MEDIA_MOUNTED_READ_ONLY.equals(state));
    }

    // private static boolean isSdReadable() {
    // return isSdMounted();
    // }

    private boolean isSdWriteable() {
        //return Environment.getExternalStorageState().equals(Environment.MEDIA_MOUNTED);
        StorageVolume volume = getMountedVolumeById(mSelectedIndex);
        if (volume == null) {
            return false;
        }
        String state = mStorageManager.getVolumeState(volume.getPath());
        return Environment.MEDIA_MOUNTED.equals(state);
    }

    private String getSdPath() {
        //return Environment.getExternalStorageDirectory().getPath();
        StorageVolume sv = getMountedVolumeById(mSelectedIndex);
        if (sv != null) {

            Elog.i(TAG, "sv.getPath() :" + sv.getPath());
            return sv.getPath();
        }
        return null;
    }

    private long getSdAvailableSpace() {
        if (isSdMounted()) {
            String sdcard = getSdPath();
            StatFs statFs = new StatFs(sdcard);
            @SuppressWarnings("deprecation")
            long availableSpace = (long) statFs.getBlockSize() * statFs.getAvailableBlocks();

            return availableSpace;
        } else {
            return -1;
        }
    }

    private static final String SDLOG_TEXT = "Copyright Statement:This software/firmware"
            + " and related documentation MediaTek Softwareare* protected under relevant "
            + "copyright laws. The information contained herein* is confidential and proprietary"
            + " to MediaTek Inc. and/or its licensors.* Without the prior written permission of "
            + "MediaTek inc. and/or its licensors,* any reproduction, modification, use or "
            + "disclosure of MediaTek Software,* and information contained herein, in whole "
            + "or in part, shall be strictly prohibited. MediaTek Inc. (C) 2010. All rights "
            + "reserved** BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES "
            + "AND AGREES* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS (MEDIATEK SOFTWARE)"
            + "* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON* "
            + "AN AS-IS BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,* EXPRESS"
            + " OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED OF* MERCHANTABILITY,"
            + " FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.* NEITHER DOES MEDIATEK PROVIDE"
            + " ANY WARRANTY WHATSOEVER WITH RESPECT TO THE* SOFTWARE OF ANY THIRD PARTY WHICH MAY "
            + "BE USED BY, INCORPORATED IN, OR* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER "
            + "AGREES TO LOOK ONLY TO SUCH* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. "
            + "RECEIVER EXPRESSLY ACKNOWLEDGES* THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN"
            + " FROM ANY THIRD PARTY ALL PROPER LICENSES* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK "
            + "SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEKSOFTWARE RELEASES MADE TO RECEIVER'S "
            + "SPECIFICATION OR TO CONFORM TO A PARTICULARSTANDARD OR OPEN FORUM. RECEIVER'S SOLE "
            + "AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE ANCUMULATIVE LIABILITY WITH RESPECT TO "
            + "THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,AT MEDIATEK'S OPTION, TO REVISE OR "
            + "REPLACE THE MEDIATEK SOFTWARE AT ISSUE,OR REFUND ANY LICENSE FEES OR SERVICE"
            + " CHARGE PAID BY RECEIVER TOMEDIATEK FOR SUCH MEDIATEK  AT ISSUE.The following"
            + " software/firmware and/or related documentation have been modified"
            + " by MediaTek Inc. All revisions are subject to any receiver'sapplicable license "
            + "agreements with MediaTek Inc.";
}
