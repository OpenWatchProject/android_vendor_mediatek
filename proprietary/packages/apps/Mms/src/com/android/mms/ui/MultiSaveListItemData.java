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

package com.android.mms.ui;

import android.content.ContentResolver;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.media.ExifInterface;
import android.media.MediaMetadataRetriever;
import android.net.Uri;
import android.os.ParcelFileDescriptor;
import android.webkit.MimeTypeMap;

import com.android.mms.R;
import com.android.mms.util.MmsContentType;
import com.android.mms.util.MmsLog;
import com.google.android.mms.pdu.PduPart;

import com.mediatek.mms.model.FileAttachmentModel;
import com.mediatek.mms.util.DrmUtilsEx;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

/** M:
 * The data structure describing each item in the MultiSave List.
 */
public class MultiSaveListItemData {
    private static final String TAG = "Mms/MultiSaveListItemData";

    private PduPart mPduPart;

    private String mSize;

    private boolean mItemSelected = false;

    private String mName;

    private Bitmap mThumbnail;

    private long mMessageId;

    private String mFallbackName;

    private Uri mDataUri;

    private static Bitmap sThumbDefaultImage;

    private static Bitmap sThumbDefaultAudio;

    private static Bitmap sThumbDefaultVideo;

    private static Bitmap sThumbDefaultVCard;

    private static Bitmap sThumbDefaultVCalendar;

    public MultiSaveListItemData(Context context, PduPart part, long msgid) {
        mPduPart = part;
        mMessageId = msgid;
        mFallbackName = Long.toHexString(msgid);
        mDataUri = part.getDataUri();
        mName = getNameFromPart(part);
        mSize = getSizeFromPart(part, context.getContentResolver());
        final float density = context.getResources().getDisplayMetrics().density;
        mThumbnail = getThumbnailFromPart(part, context, getDesiredThumbnailWidth(density),
                getDesiredThumbnailHeight(density));
    }

    public Bitmap getThumbnail() {
        return mThumbnail;
    }

    public String getName() {
        return mName;
    }

    public String getSize() {
        return mSize;
    }

    public PduPart getPduPart() {
        return mPduPart;
    }

    public boolean isSelected() {
        return mItemSelected;
    }

    public void setSelectedState(boolean isSelected) {
        mItemSelected = isSelected;
    }

    @Override
    public String toString() {
        return "[MultiSaveListItemData from:" + getName() + " subject:" + getSize() + ",selected "
                + mItemSelected + "]";
    }

    private String getNameFromPart(PduPart part) {
        byte[] mylocation = part.getContentLocation();

        if (mylocation == null) {
            mylocation = part.getName();
        }

        if (mylocation == null) {
            mylocation = part.getFilename();
        }

        String fileName;
        if (mylocation == null) {
            // Use fallback name, which is based on Message ID
            fileName = mFallbackName;
        } else {
            fileName = new String(mylocation);
        }
        String extension;
        int index;
        if ((index = fileName.indexOf(".")) == -1) {
            String type = new String(part.getContentType());
            extension = MimeTypeMap.getSingleton().getExtensionFromMimeType(type);
            // MimeTypeMap does not recognize 'audio/amr'
            if (extension == null && type.equals("audio/amr")) {
                extension = "amr";
            }
            // a vcard file without extension, show the .vcf
            if (extension == null && type.equalsIgnoreCase("text/x-vcard")) {
                extension = "vcf";
            }
            // a vcalendar file without extension, show the .vcs
            if (extension == null && type.equalsIgnoreCase("text/x-vcalendar")) {
                extension = "vcs";
            }
        } else {
            extension = fileName.substring(index + 1, fileName.length());
            fileName = fileName.substring(0, index);
        }
        // Get rid of illegal characters in filename
        final String regex = "[:\\/?,. ]";
        fileName = fileName.replaceAll(regex, "_");
        fileName = fileName.replaceAll("<", "");
        fileName = fileName.replaceAll(">", "");
        MmsLog.ipi(TAG, "getNameFromPart, fileName is " + fileName + ", extension is " + extension);
        return fileName + "." + extension;
    }

    private String getSizeFromPart(PduPart part, ContentResolver cr) {
        long size = 0;

        // If the attachment is text or html
        String type = new String(part.getContentType());
        MmsLog.i(TAG, "getNameFromPart, type =  " + type);
        if (type.equals(MmsContentType.TEXT_PLAIN) ||
                               type.equals(MmsContentType.TEXT_HTML)) {
            if (part != null && part.getData() != null) {
                size = part.getData().length;
            }
            return  size < 1024 ? (size + "B") : (String.format("%.2f", (size / 1024.0)) + "KB");
        }

        // The attachment is other type
        ParcelFileDescriptor pfd = null;
        try {
            try {
                pfd = cr.openFileDescriptor(mDataUri, "r");
                if (pfd != null) {
                    size = pfd.getStatSize();
                }
            } finally {
                if (pfd != null) {
                    pfd.close();
                }
            }
        } catch (FileNotFoundException e) {
            MmsLog.e(TAG, "getSizeFromPart, " + e.getMessage(), e);
        } catch (IOException e) {
            MmsLog.e(TAG, "getSizeFromPart, " + e.getMessage(), e);
        }
        return size < 1024 ? (size + "B") : (String.format("%.2f", (size / 1024.0)) + "KB");
    }

    private int getDesiredThumbnailWidth(float density) {
        return (int) (50 * density);
    }

    private int getDesiredThumbnailHeight(float density) {
        return (int) (50 * density);
    }

    private Bitmap getThumbnailFromPart(PduPart part, Context context, int width, int height) {
        final String type = new String(part.getContentType());
        if (MmsContentType.isImageType(type)) {
            Bitmap raw = null;
            if (!DrmUtilsEx.checkHasDrmContent(context, null, mDataUri)) {
                InputStream input = null;
                InputStream inputForDegree = null;
                try {
                    input = context.getContentResolver().openInputStream(mDataUri);
                    inputForDegree = context.getContentResolver().openInputStream(mDataUri);
                    int orientation = 0;
                    int degree = 0;
                    if (input != null) {
                        ExifInterface exif = new ExifInterface(inputForDegree);
                        if (exif != null) {
                            orientation = exif.getAttributeInt(ExifInterface.TAG_ORIENTATION, 0);
                            degree = UriImage.getExifRotation(orientation);
                        }
                        raw = BitmapFactory.decodeStream(input, null, null);
                        raw = UriImage.rotate(raw, degree);
                    }
                } catch (FileNotFoundException e) {
                    MmsLog.e(TAG, e.getMessage(), e);
                } catch (IOException e) {
                    MmsLog.e(TAG, e.getMessage());
                } catch (OutOfMemoryError ex) {
                    MessageUtils.writeHprofDataToFile();
                    throw ex;
                } catch (IllegalArgumentException e) {
                    MmsLog.e(TAG, e.getMessage(), e);
                } finally {
                    if (input != null) {
                        try {
                            input.close();
                        } catch (IOException e) {
                            MmsLog.e(TAG, e.getMessage(), e);
                        }
                    }
                    if (inputForDegree != null) {
                        try {
                            inputForDegree.close();
                        } catch (IOException e) {
                            MmsLog.e(TAG, e.getMessage(), e);
                        }
                    }
                }
            }
            Bitmap thumb;
            if (raw == null) {
                if (sThumbDefaultImage == null) {
                    sThumbDefaultImage = BitmapFactory.decodeResource(context.getResources(),
                            R.drawable.ic_multi_save_thumb_image);
                }
                thumb = sThumbDefaultImage;
            } else {
                thumb = Bitmap.createScaledBitmap(raw, width, height, true);
                if (thumb != raw) {
                    raw.recycle();
                }
            }
            return thumb;
        } else if (MmsContentType.isVideoType(type)) {
            MediaMetadataRetriever retriever = new MediaMetadataRetriever();
            Bitmap raw = null;
            try {
                try {
                    retriever.setDataSource(context, mDataUri);
                    raw = retriever.getFrameAtTime(-1);
                } finally {
                    retriever.release();
                }
            } catch (IllegalArgumentException e) {
                // corrupted video
            } catch (RuntimeException e) {
                // corrupted video
            }
            Bitmap thumb;
            if (raw == null) {
                if (sThumbDefaultVideo == null) {
                    sThumbDefaultVideo = BitmapFactory.decodeResource(context.getResources(),
                            R.drawable.ic_multi_save_thumb_video);
                }
                thumb = sThumbDefaultVideo;
            } else {
                thumb = Bitmap.createScaledBitmap(raw, width, height, true);
                if (thumb != raw) {
                    raw.recycle();
                }
            }
            return thumb;
        } else if (MmsContentType.isAudioType(type)
                || "application/ogg".equalsIgnoreCase(type)) {
            if (sThumbDefaultAudio == null) {
                sThumbDefaultAudio = BitmapFactory.decodeResource(context.getResources(),
                        R.drawable.ic_multi_save_thumb_audio);
            }
            return sThumbDefaultAudio;
        } else if (FileAttachmentModel.isVCard(part)) {
            if (sThumbDefaultVCard == null) {
                sThumbDefaultVCard = BitmapFactory.decodeResource(context.getResources(),
                        R.drawable.ic_vcard_attach);
            }
            return sThumbDefaultVCard;
        } else if (FileAttachmentModel.isVCalendar(part)) {
            if (sThumbDefaultVCalendar == null) {
                sThumbDefaultVCalendar = BitmapFactory.decodeResource(context.getResources(),
                        R.drawable.ic_vcalendar_attach);
            }
            return sThumbDefaultVCalendar;
        }
        return null;
    }
}
