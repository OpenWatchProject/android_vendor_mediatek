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
 * Copyright (C) 2008 Esmertec AG.
 * Copyright (C) 2008 The Android Open Source Project
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
package com.mediatek.mms.model;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.text.TextUtils;
import android.text.format.DateFormat;
import android.util.Log;
import android.webkit.MimeTypeMap;

import com.android.mms.util.MmsContentType;
import com.google.android.mms.MmsException;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.Date;

/// M:
public class VCalendarModel extends FileAttachmentModel {
    private static final String TAG = "VCalendarModel";

    public VCalendarModel(Context context, Uri uri) throws MmsException {
        Log.d(TAG, "New VCalendarModel (1) ");
        mContext = context;
        mContentType = MmsContentType.TEXT_VCALENDAR;
        mUri = uri;
        initFromUri();
        initAttachmentSize();
        checkContentRestriction();
        initPlugin();
    }

    public VCalendarModel(Context context,
            String contentType, String src, Uri uri) throws MmsException {
        super(context, contentType, src, uri);
        Log.d(TAG, "New VCalendarModel (2) ");
    }

    private void initFromUri() throws MmsException {
        try {
            final String scheme = mUri.getScheme();
            if (scheme.equals("file")) {
                initFromFile();
            }

            final String timestamp =
                DateFormat.format("yyyyMMdd_hhmmss",
                        new Date(System.currentTimeMillis())).toString();
            mFileName = timestamp + ".vcs";
        } catch (IllegalArgumentException e) {
            Log.e(TAG, "IllegalArgumentException caught while opening or reading stream", e);
            throw new MmsException("Type of vcard is unknown.");
        }
    }

    private void initFromFile() {
        mFileName = mUri.getPath();
        if (TextUtils.isEmpty(mContentType)) {
            // parse content type via file extension
            int index;
            if ((index = mFileName.lastIndexOf(".")) != -1) {
                final String extension = mFileName.substring(index + 1, mFileName.length());
                mContentType = MimeTypeMap.getSingleton()
                        .getMimeTypeFromExtension(extension.toLowerCase());
            }
            if (TextUtils.isEmpty(mContentType)) {
                mContentType = UNKNOWN_TYPE;
            }
        }
    }

    private void initAttachmentSize() throws MmsException {
        final ContentResolver cr = mContext.getContentResolver();
        // if uri is cotent://, it must be one from Calendar, size of which can be query out
        try {
            Cursor c = null;
            try {
                c = cr.query(mUri, null, null, null, null);
                if (c != null && c.moveToFirst()) {
                    mSize = c.getInt(1);
                }
            } finally {
                if (c != null) {
                    c.close();
                }
            }
        } catch (Exception e) {
            throw new MmsException("VCalendar#initAttachmentSize() " + e.getMessage());
        }
        if (mSize > 0) {
            return;
        }
        InputStream input = null;
        try {
            try {
                input = cr.openInputStream(mUri);
                if (input instanceof FileInputStream) {
                    // avoid reading the whole stream to get its length
                    FileInputStream f = (FileInputStream) input;
                    mSize = (int) f.getChannel().size();
                } else {
                    if (input == null) {
                        Log.e(TAG, "initAttachmentSize, input == null");
                        throw new MmsException("VCalendar#initAttachmentSize() NPE");
                    }
                    while (-1 != input.read()) {
                        mSize++;
                    }
                }
            } finally {
                if (input != null) {
                    input.close();
                }
            }
        } catch (FileNotFoundException e) {
            Log.e(TAG, "initAttachmentSize, file is not found??");
            throw new MmsException("VCalendar#initAttachmentSize() " + e.getMessage());
        } catch (IOException e) {
            Log.e(TAG, "initAttachmentSize, other exceptions");
            throw new MmsException("VCalendar#initAttachmentSize() " + e.getMessage());
        }
    }
}
