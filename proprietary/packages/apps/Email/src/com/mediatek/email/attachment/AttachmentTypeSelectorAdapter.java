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

import android.content.Context;

import com.android.email.R;

import java.util.ArrayList;
import java.util.List;

/**
 * M: An adapter to store icons and strings for attachment type list.
 */
public class AttachmentTypeSelectorAdapter extends IconListAdapter {

    public static final int ADD_IMAGE               = 0;
    public static final int ADD_MUSIC               = 1;
    public static final int ADD_VIDEO               = 2;
    public static final int ADD_CONTACT             = 3;
    public static final int ADD_FILE                = 4;
    public static final int ADD_CALENDAR            = 5;
    public static final int ADD_ATTACHMENT_TYPE_NUM = 6;

    public AttachmentTypeSelectorAdapter(Context context) {
        super(context, getData(context));
    }

    public int buttonToCommand(int whichButton) {
        AttachmentListItem item = (AttachmentListItem) getItem(whichButton);
        return item.getCommand();
    }

    protected static List<IconListItem> getData(Context context) {
        List<IconListItem> data = new ArrayList<IconListItem>(ADD_ATTACHMENT_TYPE_NUM);

        addItem(data, context.getString(R.string.attach_image), ADD_IMAGE);

        addItem(data, context.getString(R.string.attach_sound), ADD_MUSIC);

        addItem(data, context.getString(R.string.attach_video), ADD_VIDEO);

        addItem(data, context.getString(R.string.attach_contact), ADD_CONTACT);

        // disable select calendar item if calendar importer is unavailable.
        if (AttachmentHelper.isCalenderImporterAvailable(context)) {
            addItem(data, context.getString(R.string.attach_calender), ADD_CALENDAR);
        }

        addItem(data, context.getString(R.string.attach_file), ADD_FILE);

        return data;

    }

    protected static void addItem(List<IconListItem> data, String title,  int command) {
        AttachmentListItem temp = new AttachmentListItem(title, command);
        data.add(temp);
    }

    public static class AttachmentListItem extends IconListAdapter.IconListItem {
        private int mCommand;

        public AttachmentListItem(String title, int command) {
            super(title);

            mCommand = command;
        }

        public int getCommand() {
            return mCommand;
        }
    }
}
