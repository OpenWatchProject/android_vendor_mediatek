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
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.camera.tests.v3.observer;

import com.mediatek.camera.common.debug.LogUtil;
import com.mediatek.camera.common.device.CameraDeviceManagerFactory;
import com.mediatek.camera.common.mode.CameraApiHelper;
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observer used to check AE lock state is right.
 */
public class AeLockObserver extends AbstractLogObserver {
    /**
     * Index stands for check ae unlock.
     */
    public static final int INDEX_UNLOCK = 0;
    /**
     * Index stands for check ae lock.
     */
    public static final int INDEX_LOCK = 1;

    private static final LogUtil.Tag TAG = Utils.getTestTag(AeLockObserver.class
            .getSimpleName());
    private static final String LOG_TAG_EXPOSURE_API1 = "CamAp_ExposureParamete";
    private static final String LOG_TAG_EXPOSURE_API2 = "CamAp_ExposureCaptureR";
    private static final String LOG_TAG_AE_MGR = "ae_mgr";
    private int mExpectedIndex = 0;
    private CameraDeviceManagerFactory.CameraApi mCameraApi = null;
    private boolean mIsOnlyCheckNativeLog = false;

    private static final String[] LOG_TAG_LIST_API1 = new String[]{
            LOG_TAG_EXPOSURE_API1,
            LOG_TAG_AE_MGR
    };

    private static final String[] LOG_KEY_LIST_API1_LOCK = new String[]{
            "[configParameters] setAutoExposureLock true",
            "m_bAPAELock: 0 -> 1"
    };

    private static final String[] LOG_KEY_LIST_API1_UNLOCK = new String[]{
            "[configParameters] setAutoExposureLock false",
            "m_bAPAELock: 1 -> 0"
    };

    private static final String[] LOG_TAG_LIST_API2 = new String[]{
            LOG_TAG_EXPOSURE_API2,
            LOG_TAG_AE_MGR
    };

    private static final String[] LOG_KEY_LIST_API2_LOCK = new String[]{
            "[addBaselineCaptureKeysToRequest] mAeLock true",
            "m_bAPAELock: 0 -> 1"

    };

    private static final String[] LOG_KEY_LIST_API2_UNLOCK = new String[]{
            "[addBaselineCaptureKeysToRequest] mAeLock false",
            "m_bAPAELock: 1 -> 0"
    };

    /**
     * Init api level and whether only native log  need to be checked.
     *
     * @param api                     The camera api level.
     * @param isOnlyCheckNativeLogLog True if only check native log,false otherwise.
     */
    public void initEnv(CameraDeviceManagerFactory.CameraApi api, boolean isOnlyCheckNativeLogLog) {
        mCameraApi = api;
        mIsOnlyCheckNativeLog = isOnlyCheckNativeLogLog;
    }


    @Override
    protected String[] getObservedTagList(int index) {
        if (mCameraApi == null) {
            mCameraApi = CameraApiHelper.getCameraApiType(null);
        }
        switch (mCameraApi) {
            case API1:
                if (mIsOnlyCheckNativeLog) {
                    return new String[]{LOG_TAG_LIST_API1[1]};
                } else {
                    return LOG_TAG_LIST_API1;
                }
            case API2:
                if (mIsOnlyCheckNativeLog) {
                    return new String[]{LOG_TAG_LIST_API2[1]};
                } else {
                    return LOG_TAG_LIST_API2;
                }
            default:
                return null;
        }
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        if (mCameraApi == null) {
            mCameraApi = CameraApiHelper.getCameraApiType(null);
        }
        switch (mCameraApi) {
            case API1:
                if (INDEX_UNLOCK == index) {
                    if (mIsOnlyCheckNativeLog) {
                        return new String[]{LOG_KEY_LIST_API1_UNLOCK[1]};
                    } else {
                        return LOG_KEY_LIST_API1_UNLOCK;
                    }
                } else if (INDEX_LOCK == index) {
                    if (mIsOnlyCheckNativeLog) {
                        return new String[]{LOG_KEY_LIST_API1_LOCK[1]};
                    } else {
                        return LOG_KEY_LIST_API1_LOCK;
                    }
                }
            case API2:
                if (INDEX_UNLOCK == index) {
                    if (mIsOnlyCheckNativeLog) {
                        return new String[]{LOG_KEY_LIST_API2_UNLOCK[1]};
                    } else {
                        return LOG_KEY_LIST_API2_UNLOCK;
                    }
                } else if (INDEX_LOCK == index) {
                    if (mIsOnlyCheckNativeLog) {
                        return new String[]{LOG_KEY_LIST_API2_LOCK[1]};
                    } else {
                        return LOG_KEY_LIST_API2_LOCK;
                    }
                }
            default:
                return null;
        }
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mExpectedIndex == getObservedTagList(index).length;
    }

    @Override
    public int getObserveCount() {
        return 2;
    }


    @Override
    protected void onLogComing(int index, String line) {
        if (mExpectedIndex >= getObservedTagList(index).length) {
            return;
        }
        if (line.contains(getObservedTagList(index)[mExpectedIndex])
                && line.contains(getObservedKeyList(index)[mExpectedIndex])) {
            mExpectedIndex++;
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        int length = getObservedTagList(index).length;
        Utils.assertRightNow(mExpectedIndex == length, "[onObserveEnd] mExpectedIndex " +
                mExpectedIndex + ",length " + length + ",mIsOnlyCheckNativeLog = " +
                mIsOnlyCheckNativeLog);
    }

    @Override
    protected void onObserveBegin(int index) {
        mExpectedIndex = 0;
    }

    @Override
    public String getDescription(int index) {
        StringBuilder sb = new StringBuilder("Observe this group tag has printed out as order, ");
        for (int i = 0; i < getObservedTagList(index).length; i++) {
            sb.append("[TAG-" + i + "] ");
            sb.append(getObservedTagList(index)[i]);
            sb.append(",[KEY-" + i + "] ");
            sb.append(getObservedKeyList(index)[i]);
            sb.append("\n");
        }
        return sb.toString();
    }

}
