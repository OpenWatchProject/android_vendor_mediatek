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
 * BY OPENING THIS Rat, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
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

package com.mediatek.engineermode.npt;

import android.content.Context;
import android.net.Uri;
import android.os.Environment;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.rfdesense.SingleMediaScanner;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.text.SimpleDateFormat;
import java.util.Date;

public class NoiseProfilingFileSave {
    private static final String TAG = "NPT/FileSave";
    private static final String mNptPath = "/npt/";
    private static final String mNptSavePath = "/npt/result";
    public static final String mNptConfigLoadPath = "/npt/configuration";
    private static final String STORAGE_AUTHORITY = "com.android.externalstorage.documents";
    private static final String PRIMARY_STORAGE = "primary";
    private static String mInputFileName = "/npt_input.csv";
    private static String NptFiletName;
    private static SimpleDateFormat mCurrectTime = null;

    public static String getCurrectTime() {
        mCurrectTime = new SimpleDateFormat("yyyy-MM-dd-HH-mm-ss-SS");
        String mTimes = mCurrectTime.format(new Date());
        return mTimes;
    }

    public static void saveRatTestResult(Context context, String NptdesenseContent,
                                         boolean append) {
        try {
            saveToSDCard(context, mNptSavePath, NptFiletName, NptdesenseContent, append);
        } catch (Exception e) {
            e.printStackTrace();
        }

    }

    public static void setNptFiletName(String name) {
        NptFiletName = name;
    }

    public static String getNptInputFileName() {
        return mInputFileName;
    }

    public static void setNptInputFileName(String name) {
        mInputFileName = name;
    }

    public static void initNptDirPATH() {
        String path0 = Environment.getExternalStorageDirectory().getPath() + mNptPath;
        File filedDir0 = new File(path0);
        if (!filedDir0.exists())
            filedDir0.mkdir();

        String path1 = Environment.getExternalStorageDirectory().getPath() + mNptSavePath;
        File filedDir1 = new File(path1);
        if (!filedDir1.exists())
            filedDir1.mkdir();

        String path2 = Environment.getExternalStorageDirectory().getPath() + mNptConfigLoadPath;
        File filedDir2 = new File(path2);
        if (!filedDir2.exists())
            filedDir2.mkdir();
    }


    private static void saveToSDCard(Context context, String dirName, String fileName, String
            content, boolean append)
            throws IOException {
        String path = Environment.getExternalStorageDirectory().getPath() + dirName;
        File file = new File(path, fileName);
        FileOutputStream fos = new FileOutputStream(file, append);
        fos.write(content.getBytes());
        fos.close();
        new SingleMediaScanner(context, file);
    }

    public static String getPathFromUri(Uri uri) {
        if (uri == null) {
            return null;
        }
        if (!STORAGE_AUTHORITY.equals(uri.getAuthority())) {
            Elog.e(TAG, "not support:" + uri.getAuthority());
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
                }
            }
        }
        return null;
    }
}
