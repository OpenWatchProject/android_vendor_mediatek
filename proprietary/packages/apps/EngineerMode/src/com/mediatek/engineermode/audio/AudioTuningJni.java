/*
 *  Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly
 * prohibited.
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
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY
 * ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY
 * THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK
 * SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO
 * RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN
 * FORUM.
 * RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
 * LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation
 * ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
package com.mediatek.engineermode.audio;

/**
 * Java JNI interface for Audio.
 *
 */
public class AudioTuningJni {

    static {
        System.loadLibrary("em_audio_jni");
    }

    /**
     * @param param1 category
     * @param param2 type
     * @return detail category type
     */
    public static native String getCategory(String param1, String param2);
    /**
     * @param param1 category
     * @param param2 configuration
     * @param param3 tag for detail information
     * @return detail parameters
     */
    public static native String getParams(String param1, String param2, String param3);

    /**
     * @param param1 category
     * @param param2 configuration
     * @param param3 tag for detail information
     * @return detail list
     */
    public static native String getChecklist(String param1, String param2, String param3);
    /**
     * @param param1 category
     * @param param2 configuration
     * @param param3 tag for detail information
     * @param param4 value to set
     * @return true for success, false for fail
     */
    public static native boolean setParams(String param1, String param2,
            String param3, String param4);
    /**
     * @param param category
     * @return true for success, false for fail
     */
    public static native boolean saveToWork(String param);
    /**
     * @return true for success, false for fail
     */
    public static native boolean registerXmlChangedCallback();
    /**
     * @param param value 1: enable 0:disable
     * @return true for success, false for fail
     */
    public static native boolean CustXmlEnableChanged(int value);

    /**
     * @param param ptr len
     * @return int
     */
    public static native int setAudioCommand(int ptr,int len);
    public static native int getAudioCommand(int ptr);
    public static native int setAudioData(int type,int len,byte[] ptr);
    public static native int getAudioData(int type,int len,byte[] ptr);
    public static native int setEmParameter(byte[] aptr,int len);
    public static native int getEmParameter(byte[] aptr,int len);

    /*
    Copy audio HAL dump
     */
    public static native void copyAudioHalDumpFilesToSdcard(OnCopyProgressChangeListener l);
    public static native void cancleCopyAudioHalDumpFile();
    public static native void delAudioHalDumpFiles(OnCopyProgressChangeListener l);

    /*
    Get Audio FeatureOption support
     */
    public static native boolean isFeatureSupported(String featureName);
}
