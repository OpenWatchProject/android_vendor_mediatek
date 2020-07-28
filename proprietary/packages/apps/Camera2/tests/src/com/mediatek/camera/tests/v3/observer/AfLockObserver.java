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
import com.mediatek.camera.tests.v3.util.Utils;

/**
 * Observer used to check focus lock state is right.
 */
public class AfLockObserver extends AbstractLogObserver {

    private static final LogUtil.Tag TAG = Utils.getTestTag(AfLockObserver.class
            .getSimpleName());
    private static final String LOG_TAG_AF_MGR = "af_mgr_v3";
    private static final int TIME_OUT_SHORT_SHORT = 3000; // 3s
    private long mLockTime = 0;
    private boolean mAfLock = false;

    private static final String[] LOG_TAG_LIST = new String[]{
            LOG_TAG_AF_MGR
    };

    private static final String[] LOG_KEY_LIST = new String[]{
            "cmd-getAFState"
    };

    @Override
    protected String[] getObservedTagList(int index) {
        return LOG_TAG_LIST;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        return LOG_KEY_LIST;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        if (mLockTime != 0) {
            boolean isCheckDone = System.currentTimeMillis() - mLockTime > TIME_OUT_SHORT_SHORT;
            return isCheckDone;
        } else {
            return false;
        }
    }

    @Override
    public int getObserveCount() {
        return 1;
    }


    @Override
    protected void onLogComing(int index, String line) {
        if (line.contains(getObservedTagList(index)[0])
                && line.contains(getObservedKeyList(index)[0])) {
            if (line.contains("->4") || line.contains("->5")) {
                mLockTime = System.currentTimeMillis();
                mAfLock = true;
            }
            if (line.contains("4->") || line.contains("5->")) {
                mAfLock = false;
            }
        }
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mAfLock, "[onObserveEnd] observer af lock fail");
    }

    @Override
    protected void onObserveBegin(int index) {
        mLockTime = 0;
        mAfLock = false;
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
