/*
 * Copyright Statement:
 *
 *   This software/firmware and related documentation ("MediaTek Software") are
 *   protected under relevant copyright laws. The information contained herein is
 *   confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 *   the prior written permission of MediaTek inc. and/or its licensors, any
 *   reproduction, modification, use or disclosure of MediaTek Software, and
 *   information contained herein, in whole or in part, shall be strictly
 *   prohibited.
 *
 *   MediaTek Inc. (C) 2016. All rights reserved.
 *
 *   BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *   THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 *   RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 *   ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 *   WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 *   WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 *   NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 *   RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *   INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 *   TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 *   RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 *   OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 *   SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 *   RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 *   STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 *   ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 *   RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 *   MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 *   CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *   The following software/firmware and/or related documentation ("MediaTek
 *   Software") have been modified by MediaTek Inc. All revisions are subject to
 *   any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.common.setting;

import android.os.ConditionVariable;

import com.mediatek.camera.common.debug.LogHelper;
import com.mediatek.camera.common.debug.LogUtil;

import java.util.ArrayList;
import java.util.List;

/**
 * Manager to manage accessing settings in different threads.
 */

public class SettingAccessManager {
    private static final LogUtil.Tag TAG
            = new LogUtil.Tag(SettingAccessManager.class.getSimpleName());

    private List<Access> mAccessList = new ArrayList<>();
    private boolean mInControlling;
    private volatile int mActiveAccessCount = 0;
    private Object mAccessCountSyncObject = new Object();
    private ConditionVariable mConditionVariable = new ConditionVariable();

    /**
     * Get an access to operate settings.
     *
     * @param name The name to indicate this access.
     * @return An access object.
     */
    public Access getAccess(String name) {
        Access access = new Access(name);
        return access;
    }

    /**
     * Active the access to have permission to operate settings. If current settings
     * is in controlling, the method will return false, so it has no permission to
     * use settings.
     *
     * @param access The access object.
     * @return True means it has permission to use settings.
     */
    public synchronized boolean activeAccess(Access access) {
        return activeAccess(access, true);
    }

    /**
     * Active the access to have permission to operate settings. If current settings
     * is in controlling, the method will return false, so it has no permission to
     * use settings.
     *
     * @param access The access object.
     * @param loggable Decide to print log or not.
     * @return True means it has permission to use settings.
     */
    public synchronized boolean activeAccess(Access access, boolean loggable) {
        if (loggable) {
            LogHelper.d(TAG, "[activeAccess], access:" + access.mName
                    + ", mInControlling:" + mInControlling);
        }
        if (mInControlling) {
            return false;
        }
        access.validate();
        mAccessList.add(access);
        synchronized (mAccessCountSyncObject) {
            mActiveAccessCount++;
        }
        return true;
    }

    /**
     * Recycle the access, make it has no permission to use settings.
     *
     * @param access The access object.
     */
    public void recycleAccess(Access access) {
        recycleAccess(access, true);
    }

    /**
     * Recycle the access, make it has no permission to use settings.
     *
     * @param access The access object.
     * @param loggable Decide to print log or not.
     */
    public void recycleAccess(Access access, boolean loggable) {
        if (loggable) {
            LogHelper.d(TAG, "[recycleAccess], access:" + access.mName);
        }
        synchronized (mAccessCountSyncObject) {
            mActiveAccessCount--;
        }
        access.invalidate();
        if (mActiveAccessCount <= 0) {
            mConditionVariable.open();
        }
    }

    /**
     * Start controlling the settings, during the controlling all the operates is
     * forbidden except destroying settings.
     */
    public synchronized void startControl() {
        LogHelper.d(TAG, "[startControl]");
        mInControlling = true;
        for (Access access : mAccessList) {
            access.invalidate();
        }
        mConditionVariable.close();
        LogHelper.d(TAG, "[startControl], mActiveAccessCount:" + mActiveAccessCount);
        if (mActiveAccessCount > 0) {
            mConditionVariable.block();
        }
        mAccessList.clear();
        mActiveAccessCount = 0;
    }

    /**
     * Stop controlling.
     */
    public synchronized void stopControl() {
        LogHelper.d(TAG, "[stopControl]");
        mInControlling = false;
    }

    /**
     * Access class to operate settings.
     */
    public class Access {
        private String mName;
        private boolean mValid;

        /**
         * Access constructor.
         *
         * @param name The name of access.
         */
        public Access(String name) {
            mName = name;
        }

        /**
         * The access is valid or not.
         *
         * @return The valid of the access.
         */
        public boolean isValid() {
            return mValid;
        }

        private void validate() {
            mValid = true;
        }
        private void invalidate() {
            mValid = false;
        }
    }
}

