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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.voicecommand.cfg;

/**
 * Voice Model Information.
 *
 */
public class VoiceModelInfo {

    public String mPackageName;
    public String mLocale;
    public String mFileName;
    public String mSubFolder;
    public String mFolder;
    public int mStageNo = 0;
    public int mVersion = 0;
    public boolean mDeleted = false;
    public boolean mNeedWriteOut = false;
    public boolean mDefault = false;
    public byte[] mData = null;

    /**
     * VoiceWakeupInfo constructor.
     */
    public VoiceModelInfo() {
        super();
    }

    public boolean isAvalible() {
        return mPackageName != null && mLocale != null && mFileName != null
                && mStageNo > 0 && mVersion > 0;
    }

    public String getResPath() {
        String path = (mFolder != null && mFolder.length() > 0 ? mFolder + "/" : "");
        path += (mSubFolder != null && mSubFolder.length() > 0 ? mSubFolder + "/" : "");
        path += (mFileName != null && mFileName.length() > 0 ? mFileName : "");
        return path;
    }

    public String getResFolder(int version) {
        String path = (mFolder != null && mFolder.length() > 0 ?
                mFolder + "_" + version + "/" : "");
        return path;
    }

    @Override
    public String toString() {
        return "VoiceModelInfo:" + "mPackageName=" + mPackageName + " mLocale="
                + mLocale + " mFileName=" + mFileName + " mSubFolder="
                + mSubFolder + " mFolder=" + mFolder + " mStageNo=" + mStageNo
                + " mVersion=" + mVersion + " mDeleted=" + mDeleted
                + " mNeedWriteOut=" + mNeedWriteOut + " mDefault=" + mDefault;
    }
}