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

import com.android.mms.model.MediaModel;
import com.android.mms.model.SlideModel;
import com.android.mms.model.SlideshowModel;

import java.util.Iterator;

public class MmsSizeUtils {
    private static final int SMIL_TAG_SIZE_TEXT              = 120;
    public static final int SMIL_TAG_SIZE_PAGE              = 26;
    public static final int SMIL_TAG_SIZE_ATTACH            = 26;
    public static final int SMIL_TAG_SIZE_IMAGE             = 50;
    private static final int SMIL_TAG_SIZE_AUDIO             = 50;
    private static final int SMIL_TAG_SIZE_VIDEO             = 50;

    private static final int MMS_HEADER_SIZE                  = 128;
    private static final int MMS_CONTENT_TYPE_HEAER_LENGTH    = 128;
    private static final int SMIL_HEADER_SIZE                = 128;

    public static int getMediaPackagedSize(MediaModel media, int size) {
        int packagedSize = 0;
        if (true == media.isText()) {
            packagedSize = size == 0 ? 0 : (size + getSlideSmilSize());
        } else {
            packagedSize = size + getMediaSmilSize();
        }
        return packagedSize;
    }

    public static int getSlideModelPackagedSize(SlideModel slide) {
        int packagedSize = 0;
        for (Iterator<MediaModel> it = slide.iterator(); it.hasNext(); ) {
            MediaModel media = it.next();
            packagedSize += media.getMediaPackagedSize();
        }
        return packagedSize;
    }

    /// M: fix bug ALPS00500614, solute shake when delete text and appear301kb. @{
    public static int getSlideSmilSize() {
        return (SMIL_TAG_SIZE_ATTACH + SMIL_TAG_SIZE_PAGE + SMIL_TAG_SIZE_TEXT);
    }
    /// @}

    /* M: Code analyze 003, fix bug ALPS00261194,
     * let recorder auto stop when the messaging reach limit.
     */
    public static int getSlideshowInitSize() {
        return MMS_HEADER_SIZE + MMS_CONTENT_TYPE_HEAER_LENGTH
                + SMIL_HEADER_SIZE + SlideshowModel.SLIDESHOW_SLOP;
    }

    public static int getSlideshowReserveSize() {
        return MMS_HEADER_SIZE + MMS_CONTENT_TYPE_HEAER_LENGTH
                + SMIL_HEADER_SIZE + SlideshowModel.SLIDESHOW_SLOP;
    }

    public static int getMediaSmilSize() {
        return (SMIL_TAG_SIZE_ATTACH + SMIL_TAG_SIZE_PAGE + SMIL_TAG_SIZE_IMAGE);
    }

    public static int getSlideshowHeaderSize() {
        return MMS_HEADER_SIZE + MMS_CONTENT_TYPE_HEAER_LENGTH + SMIL_HEADER_SIZE;
    }
}
