/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.providers.downloads.ui;

import android.app.DownloadManager;
import android.content.ActivityNotFoundException;
import android.content.BroadcastReceiver;
import android.content.ContentResolver;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.drm.DrmManagerClient;
//import android.drm.DrmStore;
//Anuj
import com.mediatek.omadrm.OmaDrmUtils;
import com.mediatek.omadrm.OmaDrmStore;
import android.net.Uri;
import android.provider.Downloads;
import android.util.Log;
import android.widget.Toast;


import java.io.File;

/**
 * This receiver clicks to notifications that
 * downloads for the OMA DL are in progress/complete.  Clicking on an
 * in-progress or failed download will open the download manager.  Clicking on
 * a complete, successful download will open the file.
 */
public class OMADLOpenDownloadReceiver extends BroadcastReceiver {
    private static final String LOG_OMA_DL = "DownloadManager/OMA";

    public void onReceive(Context context, Intent intent) {
        /// M: Add to fix OMADownloadActivity coverd by download list. @{
        String intentAction = intent.getAction();
        if (!DownloadManager.ACTION_NOTIFICATION_CLICKED.equals(intentAction)) {
            return;
        }
        /// @}
        long ids[] = intent.getLongArrayExtra(
                DownloadManager.EXTRA_NOTIFICATION_CLICK_DOWNLOAD_IDS);

        if (ids == null || ids.length != 1) {
            openDownloadsPage(context);
            return;
        }

        ContentResolver cr = context.getContentResolver();
        Uri data = intent.getData();
        Cursor cursor = null;
        try {
            cursor = cr.query(data, new String[] {
                    Downloads.Impl._ID, Downloads.Impl._DATA, Downloads.Impl.COLUMN_MIME_TYPE,
                    Downloads.Impl.COLUMN_STATUS
            }, null, null, null);
            if (cursor == null) {
                return;
            }
            if (cursor.moveToFirst()) {
                String filename = cursor.getString(1);
                String mimetype = cursor.getString(2);
                String action = intent.getAction();
                if (Downloads.Impl.ACTION_NOTIFICATION_CLICKED.equals(action)) {
                    int status = cursor.getInt(3);
                    if (Downloads.Impl.isStatusCompleted(status)
                            && Downloads.Impl.isStatusSuccess(status)) {
                        Intent launchIntent = new Intent(Intent.ACTION_VIEW);
                        Uri path = Uri.parse(filename);
                        // If there is no scheme, then it must be a file
                        if (path.getScheme() == null) {
                            path = Uri.fromFile(new File(filename));
                        }

                        // Add to support MTK DRM
                        //Anuj
                        if (OmaDrmUtils.isOmaDrmEnabled()
                                && (mimetype.equalsIgnoreCase(
                                       OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_MESSAGE) ||
                                    mimetype.equalsIgnoreCase(
                                       OmaDrmStore.DrmObjectMimeType.MIME_TYPE_DRM_CONTENT))) {
                            DrmManagerClient drmClient = new DrmManagerClient(context);
                            String oriMimeType = drmClient.getOriginalMimeType(filename);
                            if (oriMimeType != null) {
                                mimetype = oriMimeType;
                                Log.d("@M_" + LOG_OMA_DL, "Open DRM file:" + path + " MimeType is"
                                        + mimetype);
                            }
                            drmClient.close();
                        }

                        launchIntent.setDataAndType(path, mimetype);
                        launchIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        try {
                            context.startActivity(launchIntent);
                        } catch (ActivityNotFoundException ex) {
                            Toast.makeText(context, R.string.download_no_application_title,
                                    Toast.LENGTH_LONG).show();
                        }
                    } else {
                        // Open the downloads page
                        Intent pageView = new Intent(DownloadManager.ACTION_VIEW_DOWNLOADS);
                        pageView.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                        context.startActivity(pageView);
                    }
                }
            } else {
                Log.w("@M_" + LOG_OMA_DL, "OMAReceiver:cursor.moveToFirst() failed:");
            }
        } finally {
            if (cursor != null) {
                cursor.close();
            }
        }
    }

    private void openDownloadsPage(Context context) {
        Intent pageView = new Intent(DownloadManager.ACTION_VIEW_DOWNLOADS);
        pageView.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(pageView);
    }

}
