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
package com.mediatek.contacts.widget;

import android.content.Context;
import android.graphics.drawable.Drawable;
import android.os.storage.StorageManager;
import android.os.storage.StorageVolume;
import android.util.AttributeSet;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.TextView;

import com.android.contacts.R;

import com.mediatek.contacts.util.Log;

public class ImportExportItem extends LinearLayout {
    private static final String TAG = "ImportExportItem";

    private TextView mAccountUserName;
    private ImageView mIcon;
    private RadioButton mRadioButton;

    public ImportExportItem(Context context) {
        super(context);
    }

    public ImportExportItem(Context context, AttributeSet attrs) {
        super(context, attrs);
    }

    @Override
    public void setActivated(boolean activated) {
        super.setActivated(activated);
        if (mRadioButton != null) {
            mRadioButton.setChecked(activated);
        } else {
            Log.w(TAG, "[setActivated]radio-button cannot be activated because it is null");
        }
    }

    public void bindView(Drawable icon, String text, String path) {
        Log.d(TAG, "[bindView]text: " + Log.anonymize(text) + ",path = " + Log.anonymize(path));
        mAccountUserName = (TextView) findViewById(R.id.accountUserName);
        mIcon = (ImageView) findViewById(R.id.icon);
        mRadioButton = (RadioButton) findViewById(R.id.radioButton);

        if (icon != null && path == null) {
            mIcon.setImageDrawable(icon);
        } else if (path != null) {
            if (!isExternalStoragePath(path)) {
                mIcon.setImageResource(R.drawable.mtk_contact_phone_storage);
            } else {
                mIcon.setImageResource(R.drawable.mtk_contact_sd_card_icon);
            }
        } else {
            mIcon.setImageResource(R.drawable.unknown_source);
        }

        mAccountUserName.setText(text);
    }

    private boolean isExternalStoragePath(String path) {
        if (path == null || path.isEmpty()) {
            Log.d(TAG, "[isExternalStoragePath]empty path: " + Log.anonymize(path));
            return false;
        }
        final StorageManager storageManager = (StorageManager) getContext().
            getSystemService(Context.STORAGE_SERVICE);
        if (storageManager == null) {
            Log.d(TAG, "[isExternalStoragePath]failed to get StorageManager");
            return false;
        }
        StorageVolume[] storageVolumeList = storageManager.getVolumeList();
        if (storageVolumeList != null) {
            for (StorageVolume volume : storageVolumeList) {
                String p = volume.getPath();
                boolean isExternal = volume.isRemovable();
                Log.d(TAG, "[isExternalStoragePath]isExternal=" + isExternal
                        + ", path=" + Log.anonymize(p));
                if (isExternal && path.equals(p)) {
                    return true;
                }
            }
        }
        return false;
    }
}
