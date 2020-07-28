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
import com.mediatek.camera.tests.v3.util.LogHelper;
import com.mediatek.camera.tests.v3.util.Utils;

import java.util.Locale;

/**
 * Class to observer AIS parameters.
 */

public class AisObserver extends AbstractLogObserver {
    private static final LogUtil.Tag TAG = Utils.getTestTag(
            AisObserver.class.getSimpleName());

    private static final String LOG_TAG_FOR_API1 = "MtkCam/ParamsManager";
    private static final String KEY_STRING_FOR_API1 = "mfb=";
    private static final String LOG_TAG_FOR_API2 = "MtkCam/MetadataConverter";
    private static final String KEY_STRING_FOR_API2 = "com.mediatek.mfnrfeature.mfbmode";
    private boolean mPass = false;

    private static final String[] EFFECT_NAME = {
            "ais",
            "auto",
            "off",
    };

    /**
     * AIS enum value for API2.
     */
    enum ModeEnum {
        OFF(0),
        MFLL(1),
        AIS(2),
        AUTO(255);

        private int mValue = 0;

        ModeEnum(int value) {
            this.mValue = value;
        }

        /**
         * Get enum value which is in integer.
         *
         * @return The enum value.
         */
        public int getValue() {
            return this.mValue;
        }

        /**
         * Get enum name which is in string.
         *
         * @return The enum name.
         */
        public String getName() {
            return this.toString();
        }
    }

    @Override
    protected String[] getObservedTagList(int index) {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            return new String[]{LOG_TAG_FOR_API1};
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            return new String[]{LOG_TAG_FOR_API2};
        }
        return null;
    }

    @Override
    protected String[] getObservedKeyList(int index) {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            return new String[]{KEY_STRING_FOR_API1};
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            return new String[]{KEY_STRING_FOR_API2};
        }
        return null;
    }

    @Override
    public boolean isSupported(int index) {
        if (index == 0) {
            return Utils.isFeatureSupported("com.mediatek.camera.at.anti-shake");
        } else {
            return true;
        }
    }

    @Override
    public int getObserveCount() {
        return 2;
    }

    @Override
    public String getDescription(int index) {
        return "Check AIS: " + EFFECT_NAME[index];
    }

    @Override
    protected void onObserveEnd(int index) {
        Utils.assertRightNow(mPass);
    }

    @Override
    protected void onObserveBegin(int index) {
        mPass = false;
    }

    @Override
    protected boolean isAlreadyFindTarget(int index) {
        return mPass;
    }

    @Override
    protected void applyAdbCommandBeforeStarted() {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            setAdbCommand("debug.camera.log.AppStreamMgr", "2");
        }
    }

    @Override
    protected void clearAdbCommandAfterStopped() {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            setAdbCommand("debug.camera.log.AppStreamMgr", "0");
        }
    }

    @Override
    protected void onLogComing(int index, String line) {
        if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API1) {
            int beginIndex = line.indexOf(KEY_STRING_FOR_API1) + KEY_STRING_FOR_API1.length();
            int endIndex = line.indexOf(";", beginIndex);
            String subString = line.substring(beginIndex, endIndex);
            LogHelper.d(TAG, "[onLogComing], mfb=" + subString + ", index = " + index);
            mPass = EFFECT_NAME[index].equals(subString);
        } else if (CameraApiHelper.getCameraApiType(null)
                == CameraDeviceManagerFactory.CameraApi.API2) {
            int beginIndex = line.indexOf("=> ") + "=> ".length();
            String subString = line.substring(beginIndex, line.length() - 1);
            String effectName = convertEnumToString(Integer.parseInt(subString));
            LogHelper.d(TAG, "[onLogComing], com.mediatek.mfnrfeature.mfbmode="
                    + subString + "(" + effectName + ")");
            if (index == 1) {
                mPass = "auto".equals(effectName);
            } else {
                mPass = EFFECT_NAME[index].equals(effectName);
            }
        }
    }

    private String convertEnumToString(int enumIndex) {
        String name = null;
        ModeEnum[] modes = ModeEnum.values();
        for (ModeEnum mode : modes) {
            if (mode.getValue() == enumIndex) {
                name = mode.getName().replace('_', '-').toLowerCase(Locale.ENGLISH);
                break;
            }
        }
        return name;
    }
}
