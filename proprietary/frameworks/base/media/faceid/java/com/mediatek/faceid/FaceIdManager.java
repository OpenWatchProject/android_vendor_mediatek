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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.faceid;

import android.util.Log;

import java.util.HashSet;
import java.util.Iterator;

public class FaceIdManager {
    public static final int CMD_START_REGISTER_FACE = 2;
    public static final int CMD_STOP_REGISTER_FACE = 3;
    public static final int CMD_DELETE_REGISTER_FACE = 4;
    public static final int CMD_START_COMPARE_FACE = 5;
    public static final int CMD_STOP_COMPARE_FACE = 6;


    public static final int ERROR_BASE = 91000;
    public static final int SUCCESS = ERROR_BASE + 0;
    public static final int ERROR_NOT_FOUND = ERROR_BASE + 1;
    public static final int ERROR_BAD_QUALITY  = ERROR_BASE + 2;
    public static final int ERROR_TOO_SMALL  = ERROR_BASE + 3;
    public static final int ERROR_TOO_LARGE  = ERROR_BASE + 4;
    public static final int ERROR_LEFT  = ERROR_BASE + 5;
    public static final int ERROR_TOP  = ERROR_BASE + 6;
    public static final int ERROR_RIGHT  = ERROR_BASE + 7;
    public static final int ERROR_BOTTOM  = ERROR_BASE + 8;
    public static final int ERROR_BLUR  = ERROR_BASE + 9;
    public static final int ERROR_INCOMPLETE  = ERROR_BASE + 10;
    public static final int ERROR_EYE_OCCLUSION   = ERROR_BASE + 11;
    public static final int ERROR_MOUTH_OCCLUSION  = ERROR_BASE + 12;
    public static final int ERROR_ROTATED_LEFT  = ERROR_BASE + 13;
    public static final int ERROR_RISE  = ERROR_BASE + 14;
    public static final int ERROR_ROTATED_RIGHT  = ERROR_BASE + 15;
    public static final int ERROR_DOWN  = ERROR_BASE + 16;
    public static final int ERROR_MULTI  = ERROR_BASE + 17;
    public static final int ERROR_DARKLIGHT  = ERROR_BASE + 18;
    public static final int ERROR_HIGHLIGHT  = ERROR_BASE + 19;
    public static final int ERROR_HALF_SHADOW  = ERROR_BASE + 20;
    public static final int ERROR_EYE_CLOSED = ERROR_BASE + 21;
    public static final int ERROR_EYE_CLOSED_UNKNOW  = ERROR_BASE + 22;
    public static final int ERROR_GOOD_FOR_ENROLL  = ERROR_BASE + 23;
    public static final int FAILURE  = ERROR_BASE + 24;

    //public static final int ERROR_ = ;

    public static final int COMPARE_FAIL = 1;

    public enum DETECT_ERROR_CODE {
        DETECT_BASE,
        DETECT_OK,
        DETECT_NOT_FOUND,
        DETECT_BAD_QUALITY,
        DETECT_TOO_SMALL,
        DETECT_TOO_LARGE,
        DETECT_LEFT,
        DETECT_TOP,
        DETECT_RIGHT,
        DETECT_BOTTOM,
        DETECT_BLUR,
        DETECT_INCOMPLETE,
        DETECT_EYE_OCCLUSION,
        DETECT_MOUTH_OCCULSION,
        DETECT_FACE_ROTATED_LEFT,
        DETECT_FACE_RISE,
        DETECT_FACE_ROTATED_RIGHT,
        DETECT_FACE_DOWN,
        DETECT_FACE_MULTI,
        DETECT_FACE_DARKLIGHT,
        DETECT_FACE_HIGHLIGHT,
        DETECT_FACE_HALF_SHADOW,
        DETECT_FACE_EYE_CLOSED,
        DETECT_FACE_EYE_CLOSED_UNKOWN,
        DETECT_FACE_GOOD_FOR_ENROLL,
        DETECT_FACE_FAILURE
    }

    private static final String TAG = "FaceIdManager";
    private static FaceIdManager sClient = null;
    private static final Object sClientLock = new Object();

    private static HashSet<InfoListener> mListeners = new HashSet<InfoListener>();

    private FaceIdManager() {
    }

    public static FaceIdManager getInstance() {
        synchronized(sClientLock) {
            if (sClient == null) {
                sClient = new FaceIdManager();
                System.loadLibrary("fr_jni");
                Log.d(TAG, "create new client(setup)");
            }
        }
        return sClient;
    }

    public static void onInfo(int cmd_id, int errCode) {
        Log.e(TAG, "onInfo " + cmd_id + " code: " + errCode);
        if (mListeners.size() > 0) {
            Iterator<InfoListener> iterator = mListeners.iterator();
            while (iterator.hasNext()) {
                iterator.next().onInfo(cmd_id, errCode);
            }
        }
    }

    public int init(int uid, InfoListener callback) {
        if (callback != null) {
            mListeners.add(callback);
        }
        return initNative(uid, callback);
    }

    public int setPowerMode(int mode) {
        return setPowerModeNative(mode);
    }

    public int detectAndSaveFeature(String uname) {
        return detectAndSaveFeatureNative(uname);
    }

    public int startCompareFeature(String uname) {
        return startCompareFeatureNative(uname);
    }

    public int stopCompareFeature() {
        return stopCompareFeatureNative();
    }

    public int registerCallback(int uid, InfoListener callback) {
        if (callback != null) {
            mListeners.add(callback);
        }
        return registerCallbackNative(uid, callback);
    }

    public void unregisterCallback(int uid, InfoListener callback) {
        if (callback != null) {
            mListeners.remove(callback);
        }
    }

    public int deleteFeature(String uname) {
        return deleteFeatureNative(uname);
    }

    public int release() {
        return releaseNative();
    }

    public interface InfoListener {
        public void onInfo(int cmd, int errorCode);
    }

    private static native int initNative(int uid, Object callback);
    private static native int registerCallbackNative(int uid, Object callback);
    private static native int setPowerModeNative(int mode);
    private static native int detectAndSaveFeatureNative(String uname);
    private static native int startCompareFeatureNative(String uname);
    private static native int stopCompareFeatureNative();
    private static native int deleteFeatureNative(String uname);
    private static native int releaseNative();

}
