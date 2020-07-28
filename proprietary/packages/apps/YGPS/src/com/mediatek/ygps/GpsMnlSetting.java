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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ygps;

import android.os.SystemProperties;
import android.util.Log;

import java.util.ArrayList;

/**
 * GPS MNL flag setting.
 *
 * @author mtk54046
 * @version 1.0
 */
public class GpsMnlSetting {

    private static final String TAG = "YGPS/MnlSetting";
    static final String PROP_VALUE_0 = "0";
    static final String PROP_VALUE_1 = "1";
    static final String PROP_VALUE_2 = "2";
    static final String PROP_VALUE_NONE_EPO = "0";
    static final String PROP_VALUE_BOTH_EPO = "1";
    static final String PROP_VALUE_ONLY_EPO = "2";
    static final String PROP_VALUE_ONLY_QEPO = "3";


    static final String KEY_DEBUG_DBG2SOCKET = "debug.dbg2socket";
    static final String KEY_DEBUG_NMEA2SOCKET = "debug.nmea2socket";
    static final String KEY_DEBUG_DBG2FILE = "debug.dbg2file";
    static final String KEY_DEBUG_DEBUG_NMEA = "debug.debug_nmea";
    static final String KEY_BEE_ENABLED = "BEE_enabled";
    static final String KEY_TEST_MODE = "test.mode";
    static final String KEY_GPSLOG_ENABLED = "gpslog_enabled";
    static final String KEY_GPS_EPO = "gps.epo";

    private static final String MNL_PROP_NAME = "persist.vendor.radio.mnl.prop";
    private static final String GPS_CHIP_PROP = "vendor.gps.gps.version";

    private static final String DEFAULT_MNL_PROP = "00010001";
    private static ArrayList<String> sKeyList = null;
    private static final String GPS_CLOCK_PROP = "vendor.gps.clock.type";
    private static final String LOG_HIDDEN_PROP = "ro.vendor.mtk_log_hide_gps";
    private static final String HIDDEN_PROP_1 = "1";
    private static final String HIDDEN_PROP_2 = "2";
    private static final String HIDDEN_PROP_VALUE = SystemProperties.get(LOG_HIDDEN_PROP);

    private static boolean sLogHidden = HIDDEN_PROP_1.equals(HIDDEN_PROP_VALUE)
            || HIDDEN_PROP_2.equals(HIDDEN_PROP_VALUE);

    /**
     * Get gps chip version.
     *
     * @param defaultValue
     *            Default value
     * @return GPS chip version
     */
    static String getChipVersion(String defaultValue) {
        String chipVersion = SystemProperties.get(GPS_CHIP_PROP);
        if (null == chipVersion || chipVersion.isEmpty()) {
            chipVersion = defaultValue;
        }
        return chipVersion;
    }

    /**
     * Get MNL system property.
     *
     * @param key
     *            The key of the property
     * @param defaultValue
     *            The default value of the property
     * @return The value of the property
     */
    static String getMnlProp(String key, String defaultValue) {
        String result = defaultValue;
        String prop = SystemProperties.get(MNL_PROP_NAME);
        if (null == sKeyList) {
            initList();
        }
        int index = sKeyList.indexOf(key);
        Log.i(TAG, "getMnlProp: " + prop);
        if (null == prop || prop.isEmpty()
                || -1 == index || index >= prop.length()) {
            result = defaultValue;
        } else {
            char c = prop.charAt(index);
            result = String.valueOf(c);
        }
        Log.i(TAG, "getMnlProp result: " + result);
        return result;
    }

    /**
     * Set MNL system property.
     * @param conn Mnld HIDL connection agent
     * @param key The key of the property
     * @param value The value of the property
     */
    static void setMnlProp(MnldConn conn, String key, String value) {
        Log.i(TAG, "setMnlProp: " + key + " " + value);
        String prop = SystemProperties.get(MNL_PROP_NAME);
        if (null == sKeyList) {
            initList();
        }
        int index = sKeyList.indexOf(key);
        if (index != -1) {
            if (null == prop || prop.isEmpty()) {
                prop = DEFAULT_MNL_PROP;
            }
            if (prop.length() > index) {
                char[] charArray = prop.toCharArray();
                charArray[index] = value.charAt(0);
                String newProp = String.valueOf(charArray);
                conn.setMnldProp(newProp);
                Log.i(TAG, "setMnlProp newProp: " + newProp);
            }
        }
    }

    /**
     * Get GPS clock type value.
     * @param defaultValue default value if the system property is not valid
     * @return GPS clock type value
     */
    static String getClockProp(String defaultValue) {
        String clockType = SystemProperties.get(GPS_CLOCK_PROP);
        if (null == clockType || clockType.isEmpty()) {
            clockType = defaultValue;
        }
        return clockType;
    }

    static boolean isLogHidden() {
        Log.i(TAG, "sLogHidden: " + sLogHidden);
        return sLogHidden;
    }

    private static void initList() {
        sKeyList = new ArrayList<String>();
        sKeyList.add(KEY_DEBUG_DBG2SOCKET);
        sKeyList.add(KEY_DEBUG_NMEA2SOCKET);
        sKeyList.add(KEY_DEBUG_DBG2FILE);
        sKeyList.add(KEY_DEBUG_DEBUG_NMEA);
        sKeyList.add(KEY_BEE_ENABLED);
        sKeyList.add(KEY_TEST_MODE);
        sKeyList.add(KEY_GPSLOG_ENABLED);
        sKeyList.add(KEY_GPS_EPO);
    }
}
