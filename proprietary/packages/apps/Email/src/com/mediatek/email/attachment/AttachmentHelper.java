/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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
 */

/*
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

package com.mediatek.email.attachment;

import com.android.email.R;
import com.android.emailcommon.utility.FeatureOption;
import android.app.Activity;
import android.content.ActivityNotFoundException;
import android.content.Context;
import android.content.Intent;
import android.net.Uri;
import android.widget.Toast;

import com.android.mail.utils.LogUtils;

/**
 * M: Use to add types of attachments.
 */
public class AttachmentHelper {
    private static final String TAG = "AttachmentHelper";
    // Choose types of attachment
    private static final String AUDIO_XOGG_MIME_TYPE = "application/x-ogg";
    private static final String AUDIO_OGG_MIME_TYPE = "application/ogg";
    private static final String AUDIO_UNSPECIFIED_MIME_TYPE = "audio/*";
    /*
     * This is a parameter defined by contact app,to differ what kind of contact
     * list be back.
     */
    public static final int CHOICE_CONTACT_REQUEST_TYPE = 3;
    // Set max number user would choose in contact list.
    public static final int MAX_CHOICE_NUMBER = 20;
    public static final String CHOICE_CONTACT_ACTION =
           "mediatek.intent.action.contacts.list.PICKMULTICONTACTS";
    public static final String CHOICE_CALENDAR_ACTION = "mediatek.intent.action.CALENDARCHOICE";
    public static final String CHOICE_FILEMANAGER_ACTION = "com.mediatek.filemanager.ADD_FILE";
    private static final String ATTACH_CONTACT_EXTRA_REQUEST_TYPE = "request_type";
    private static final String ATTACH_CONTACT_EXTRA_PICK_COUNT = "pick_count";
    private static final String CONTACT_MIME_TYPE = "vnd.android.cursor.dir/contact";
    private static final String ATTACH_CALENDAR_EXTRA_REQUEST_TYPE = "request_type";
    private static final String CALENDAR_MIME_TYPE = "text/x-vcalendar";

    // Calendarimporter uri, use it to check if it is enable.
    public static final String VCALENDAR_URI = "content://com.mediatek.calendarimporter/";

    // Add attachment request code.
    public static final int REQUEST_CODE_ATTACH_IMAGE = 102;
    public static final int REQUEST_CODE_ATTACH_VIDEO = 103;
    public static final int REQUEST_CODE_ATTACH_SOUND = 104;
    public static final int REQUEST_CODE_ATTACH_CONTACT = 105;
    public static final int REQUEST_CODE_ATTACH_FILE = 106;
    public static final int REQUEST_CODE_ATTACH_CALENDAR = 107;
    public static final String ITEXTRA_CONTACTS = "com.mediatek.contacts.list.pickcontactsresult";

    /** M: MTK Dependence @{ */
    public static final String EXTRA_DRM_LEVEL = "android.intent.extra.drm_level";
    public static final int LEVEL_SD = 2;
    /** @} */

    public static void selectAudio(Context context)
            throws ActivityNotFoundException {
        if (context instanceof Activity) {
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.addCategory(Intent.CATEGORY_DEFAULT);
            intent.addCategory(Intent.CATEGORY_OPENABLE);
            intent.setType(AUDIO_UNSPECIFIED_MIME_TYPE);
            // some app use EXTRA_MIME_TYPES to filter attached file, avoid lost some attachable files.
            String[] extendAudioMimeTypes = new String[] { AUDIO_UNSPECIFIED_MIME_TYPE,
                    AUDIO_OGG_MIME_TYPE, AUDIO_XOGG_MIME_TYPE };
            intent.putExtra(Intent.EXTRA_MIME_TYPES, extendAudioMimeTypes);
            /** M: MTK Dependence @{ */
            if (FeatureOption.MTK_DRM_APP) {
                intent.putExtra(EXTRA_DRM_LEVEL, LEVEL_SD);
            }
            /** @} */
            ((Activity) context).startActivityForResult(intent,
                    REQUEST_CODE_ATTACH_SOUND);
            LogUtils.i(TAG,
                    "Add attachment Music, send intent %s", intent.toString());
        }
    }

    public static void selectVideo(Context context)
            throws ActivityNotFoundException {
        /** M: MTK Dependence */
        selectMediaByType(context, REQUEST_CODE_ATTACH_VIDEO, "video/*");
    }

    public static void selectImage(Context context)
            throws ActivityNotFoundException {
        /** M: MTK Dependence */
        selectMediaByType(context, REQUEST_CODE_ATTACH_IMAGE, "image/*");
    }

    private static void selectMediaByType(Context context, int requestCode,
            String contentType) throws ActivityNotFoundException {
        if (context instanceof Activity) {
            Intent innerIntent = new Intent(Intent.ACTION_GET_CONTENT);
            innerIntent.setType(contentType);
            /** M: MTK Dependence @{ */
            if (FeatureOption.MTK_DRM_APP) {
                innerIntent.putExtra(EXTRA_DRM_LEVEL, LEVEL_SD);
            }
            /** @} */
            Intent wrapperIntent = Intent.createChooser(innerIntent, null);
            ((Activity) context).startActivityForResult(wrapperIntent,
                    requestCode);
            LogUtils.i(TAG, "Add attachment Image/Video, send intent %s",
                    innerIntent.toString());
        }
    }

    public static void selectContact(Context context)
            throws ActivityNotFoundException {
        if (context instanceof Activity) {
            Intent intent = new Intent(CHOICE_CONTACT_ACTION);
            intent.addCategory(Intent.CATEGORY_DEFAULT);
            intent.putExtra(ATTACH_CONTACT_EXTRA_REQUEST_TYPE,
                    CHOICE_CONTACT_REQUEST_TYPE);
            intent.putExtra(ATTACH_CONTACT_EXTRA_PICK_COUNT, MAX_CHOICE_NUMBER);
            intent.setType(CONTACT_MIME_TYPE);
            ((Activity) context).startActivityForResult(intent,
                    REQUEST_CODE_ATTACH_CONTACT);
            LogUtils.i(TAG, "Add attachment Contact, send intent %s",
                    intent.toString());
        }
    }

    public static void selectCalendar(Context context)
            throws ActivityNotFoundException {
        if (context instanceof Activity) {
            Intent intent = new Intent(CHOICE_CALENDAR_ACTION);
            intent.setType(CALENDAR_MIME_TYPE);
            intent.putExtra(ATTACH_CALENDAR_EXTRA_REQUEST_TYPE, 0);
            ((Activity) context).startActivityForResult(intent,
                    REQUEST_CODE_ATTACH_CALENDAR);
            LogUtils.i(TAG, "Add attachment Calendar, send intent %s",
                    intent.toString());
        }
    }

    public static void selectFile(Context context) {
        if (context instanceof Activity) {
            //Intent intent = new Intent(CHOICE_FILEMANAGER_ACTION);
            Intent intent = new Intent(Intent.ACTION_GET_CONTENT);
            intent.setType("*/*");
            intent.addCategory(Intent.CATEGORY_DEFAULT);
            /** M: MTK Dependence @{ */
            if (FeatureOption.MTK_DRM_APP) {
                intent.putExtra(EXTRA_DRM_LEVEL, LEVEL_SD);
            }
            /** @} */
            ((Activity) context).startActivityForResult(intent,
                    REQUEST_CODE_ATTACH_FILE);
            LogUtils.i(TAG, "Add attachment FileManager, send intent %s ",
                    intent.toString());
        }
    }

    /**
     * M: Check if the CalendarImporter is available or not.
     */
    public static boolean isCalenderImporterAvailable(Context context) {
        String type = context.getContentResolver().getType(
                Uri.parse(VCALENDAR_URI));
        return CALENDAR_MIME_TYPE.equalsIgnoreCase(type);
    }

    public static void addAttachment(int type, Context context) {
        switch (type) {
        case AttachmentTypeSelectorAdapter.ADD_IMAGE:
            try {
                selectImage(context);
            } catch (ActivityNotFoundException anf) {
                showError(context);
                LogUtils.e(TAG, anf,
                        " ActivityNotFoundException happend in attach Image");
            }
            break;
        case AttachmentTypeSelectorAdapter.ADD_MUSIC:
            try {
                selectAudio(context);
            } catch (ActivityNotFoundException anf) {
                showError(context);
                LogUtils.e(TAG, anf,
                        " ActivityNotFoundException happend in attach Video");
            }
            break;
        case AttachmentTypeSelectorAdapter.ADD_VIDEO:
            try {
                selectVideo(context);
            } catch (ActivityNotFoundException anf) {
                showError(context);
                LogUtils.e(TAG, anf,
                        " ActivityNotFoundException happend in attach Music");
            }
            break;
        case AttachmentTypeSelectorAdapter.ADD_CONTACT:
            try {
                selectContact(context);
            } catch (ActivityNotFoundException anf) {
                showError(context);
                LogUtils.e(TAG, anf,
                        " ActivityNotFoundException happend in attach Contact");
            }
            break;
        case AttachmentTypeSelectorAdapter.ADD_FILE:
            try {
                selectFile(context);
            } catch (ActivityNotFoundException anf) {
                showError(context);
                LogUtils.e(TAG, anf,
                        " ActivityNotFoundException happend in attach FileManager");
            }
            break;
        case AttachmentTypeSelectorAdapter.ADD_CALENDAR:
            try {
                selectCalendar(context);
            } catch (ActivityNotFoundException anf) {
                showError(context);
                LogUtils.e(TAG, anf,
                        " ActivityNotFoundException happend in attach Calendar");
            }
            break;
        default:
            LogUtils.i(TAG, "Can not handle attachment types of " + type);
        }
    }

    private static void showError(Context context) {
        Toast.makeText(context,
                context.getString(R.string.attach_error_occurred),
                Toast.LENGTH_SHORT).show();
    }
}
