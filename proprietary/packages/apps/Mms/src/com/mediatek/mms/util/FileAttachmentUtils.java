/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
package com.mediatek.mms.util;

import android.content.Context;
import android.net.Uri;
import android.text.TextUtils;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.mms.MmsConfig;
import com.android.mms.R;
import com.android.mms.data.WorkingMessage;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;
import com.android.mms.ui.MessageUtils;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.google.android.mms.MmsException;
import com.google.android.mms.pdu.PduBody;
import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.ext.IOpFileAttachmentUtilsExt;
import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.mms.model.FileModel;
import com.mediatek.mms.model.VCalendarModel;
import com.mediatek.mms.model.VCardModel;
import com.mediatek.opmsg.util.OpMessageUtils;

import java.util.ArrayList;

public class FileAttachmentUtils {
    private static final String TAG = "Mms/FileAttachmentUtils";

    public static final String VCARD = "BEGIN:VCARD";
    public static final String VCALENDAR = "BEGIN:VCALENDAR";
    public static final String VCARD_DESCRIPTION = ".vcf";
    public static final String VCALENDAR_DESCRIPTION = ".vcs";

    public IOpFileAttachmentUtilsExt mOpFileAttachmentUtilsExt = null;

    public FileAttachmentUtils() {
        mOpFileAttachmentUtilsExt = OpMessageUtils.getOpMessagePlugin()
                .getOpFileAttachmentUtilsExt();
    }

    public void setOrAppendFileAttachment(
            Context context, SlideshowModel slideshow, CharSequence text,
            int type, Uri uri, boolean append) throws MmsException {
        FileAttachmentModel fileAttach = null;
        if (type == WorkingMessage.VCARD) {
            MmsLog.ipi(TAG, "setOrAppendFileAttachment(): for vcard " + uri);
            fileAttach = new VCardModel(context, uri);
        } else if (type == WorkingMessage.VCALENDAR) {
            MmsLog.ipi(TAG, "setOrAppendFileAttachment(): for vcalendar " + uri);
            fileAttach = new VCalendarModel(context, uri);
        } else {
            throw new IllegalArgumentException(
                    "setOrAppendFileAttachment type=" + type + ", uri=" + uri);
        }
        if (fileAttach == null) {
            throw new IllegalStateException(
                    "setOrAppendFileAttachment failedto create FileAttachmentModel "
                    + type + " uri = " + uri);
        }

        /// M: fix bug ALPS01269885, add vcard must calculate text size
        int textSize = 0;
        if (slideshow.size() == 1 && text != null) {
            textSize = text.toString().getBytes().length + MmsSizeUtils.getSlideSmilSize();
        }

        slideshow.checkAttachmentSize(fileAttach.getAttachSize(), append, textSize);

        /// M: Modify ALPS00474719
        if (!mOpFileAttachmentUtilsExt.setOrAppendFileAttachment(append)) {
            SlideModel slide = slideshow.get(0);
            slide.removeImage();
            slide.removeVideo();
            slide.removeAudio();
            int size = slideshow.size();
            for (int i = size - 1; i >= 1; i--) {
                slideshow.remove(i);
            }
            MmsLog.d(TAG, "Replace vcard or vcalender or Not OP01");
        }

        // Add file attachments
        if (append) {
            slideshow.addFileAttachment(fileAttach);
        } else {
            // reset file attachment, so that this is the only one
            slideshow.removeAllAttachFiles();
            slideshow.addFileAttachment(fileAttach);
        }
    }

    public ArrayList<FileAttachmentModel> getAttachFiles(
            Context context, PduBody pb, ArrayList<SlideModel> slides) throws MmsException {
        /// M: Code analyze 005, new feature(ALPS00104088), Add vCard support @{
        ArrayList<FileAttachmentModel> attachFiles = new ArrayList<FileAttachmentModel>();
        for (int i = 0; i < pb.getPartsNum(); i++) {
            /// M: Code analyze 015, fix bug ALPS00245377,
            /// Need check the content_id content_name content_filename not only location @{
            PduPart part = pb.getPart(i);
            byte[] cl = part.getContentLocation();
            byte[] name = part.getName();
            byte[] ci = part.getContentId();
            byte[] fn = part.getFilename();
            byte[] data = part.getData();
            String filename = null;
            if (cl != null) {
                filename = new String(cl);
            } else if (name != null) {
                filename = new String(name);
            } else if (ci != null) {
                filename = new String(ci);
            } else if (fn != null) {
                filename = new String(fn);
            } else {
                continue;
            }
            /// @}

            final String type = new String(part.getContentType());
            /// M: Code analyze 016, fix bug ALPS00107176,
            /// incompatible MMSCs change vCard MmsContentType etc, use TimeStamp as FileName @{
            if (FileAttachmentModel.isVCard(part)) {
                FileAttachmentModel fam = new VCardModel(context, MmsContentType.TEXT_VCARD,
                        filename, part.getDataUri());
                attachFiles.add(fam);
            /// M: Code analyze 017, new feature(ALPS00249336), add vCalendar support @{
            } else if (FileAttachmentModel.isVCalendar(part)) {
                FileAttachmentModel fam = new VCalendarModel(context,
                        MmsContentType.TEXT_VCALENDAR,
                        filename, part.getDataUri());
                attachFiles.add(fam);
            } else if (data != null && ((new String(data)).startsWith(VCARD)
                      || (new String(data)).startsWith(VCALENDAR))
                      && !FileAttachmentModel.isTextType(part)) {
                String dataContent = new String(data);
                String filenameString = "";
                if (dataContent.startsWith(VCARD)) {
                    if (TextUtils.isEmpty(filename)) {
                        filenameString = System.currentTimeMillis() + ".vcf";
                    } else if (!filename.toLowerCase().endsWith(VCARD_DESCRIPTION)) {
                        filenameString = filename + VCARD_DESCRIPTION;
                    } else {
                        filenameString = filename;
                    }
                    FileAttachmentModel fam = new VCardModel(context,
                            MmsContentType.TEXT_VCARD, filenameString,
                            part.getDataUri());
                    attachFiles.add(fam);
                } else if (dataContent.startsWith(VCALENDAR)) {
                    if (TextUtils.isEmpty(filename)) {
                        filenameString = System.currentTimeMillis() + ".vcs";
                    } else if (!filename.toLowerCase().endsWith(VCALENDAR_DESCRIPTION)) {
                        filenameString = filename + VCALENDAR_DESCRIPTION;
                    } else {
                        filenameString = filename;
                    }
                    FileAttachmentModel fam = new VCalendarModel(context,
                            MmsContentType.TEXT_VCALENDAR, filenameString,
                            part.getDataUri());
                    attachFiles.add(fam);
                }
            } else {
                mOpFileAttachmentUtilsExt.getAttachFiles(context, part, filename, slides,
                        attachFiles, new FileModel());
            }
        }

        return attachFiles;
    }

    public View createFileAttachmentView(Context context, View view, SlideshowModel slideshow) {

        ArrayList<FileAttachmentModel> attachFiles = slideshow.getAttachFiles();

        if (attachFiles == null || attachFiles.size() < 1) {
            Log.e(TAG, "createFileAttachmentView, oops no attach files found.");
            return null;
        }

        FileAttachmentModel attach = attachFiles.get(0);
        MmsLog.ipi(TAG, "createFileAttachmentView, attach " + attach.toString());

        view.setVisibility(View.VISIBLE);
        ImageView thumb = (ImageView) view.findViewById(R.id.file_attachment_thumbnail);
        TextView name = (TextView) view.findViewById(R.id.file_attachment_name_info);
        final TextView size = (TextView) view.findViewById(R.id.file_attachment_size_info);
        String nameText = null;
        int thumbResId = -1;

        int attachSize = getAllAttachSize(attachFiles);
        if (!mOpFileAttachmentUtilsExt.createFileAttachmentView(context, attachFiles, attach,
                name, thumb, size,
                MessageUtils.getHumanReadableSize(attachSize), (TextView) view
                        .findViewById(R.id.file_attachment_name_info2), (ImageView) view
                        .findViewById(R.id.file_attachment_thumbnail2), context.getResources()
                        .getDrawable(R.drawable.ipmsg_chat_contact_vcard), context.getResources()
                        .getDrawable(R.drawable.ipmsg_chat_contact_calendar),
                attach.mOpFileAttachmentModelExt,
                MessageUtils.getHumanReadableSize(slideshow.getCurrentSlideshowSize()) + "/"
                        + MmsConfig.getUserSetMmsSizeLimit(false) + "K")) {
            if (attach.isVCard()) {
                nameText = context.getString(R.string.file_attachment_vcard_name, attach.getSrc());
                thumbResId = R.drawable.ic_vcard_attach;

            } else if (attach.isVCalendar()) {
                nameText = context.getString(R.string.file_attachment_vcalendar_name,
                        attach.getSrc());
                thumbResId = R.drawable.ic_vcalendar_attach;
            }
            name.setText(nameText);
            thumb.setImageResource(thumbResId);
            size.setText(MessageUtils.getHumanReadableSize(slideshow.getCurrentSlideshowSize())
                    + "/" + MmsConfig.getUserSetMmsSizeLimit(false) + "K");
        }

        return view;
    }

    public static int getAllAttachSize(ArrayList files) {
        if (files == null) {
            return 0;
        }

        int attachSize = 0;
        for (int i = 0; i < files.size(); i++) {
            attachSize += ((FileAttachmentModel) files.get(i)).getAttachSize();
        }
        return attachSize;
    }

}
