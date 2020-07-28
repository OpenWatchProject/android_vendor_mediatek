/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2010 The Android Open Source Project
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

package com.mediatek.omadrm;

/**
 * An entity class that is used to pass information to an online DRM server. An instance of this
 * class is passed to the {@link DrmManagerClient#acquireDrmInfo acquireDrmInfo()} method to get an
 * instance of a {@link DrmInfo}. It define the Key-Value use in DrmInfoRequest's put method
 *
 */
public class OmaDrmInfoRequest {
    // Define keys
    public static final String KEY_ACTION = "action";
    public static final String KEY_DATA = "data";
    public static final String KEY_FILEDESCRIPTOR = "FileDescriptorKey";
    public static final String KEY_DATA_1 = "data_1";
    public static final String KEY_DATA_1_EXTRA = "data_1_extra";
    public static final String KEY_DATA_2 = "data_2";
    public static final String KEY_DATA_2_EXTRA = "data_2_extra";

    /*Add for CTA feature */
    public static final String KEY_CTA5_TOKEN = "CTA5Token";
    public static final String KEY_CTA5_FILEPATH = "CTA5FilePath";

    // Define actions
    // 1. install download drm file to device, include message(FL, CD, ext = '.dm'),
    // content(SD, ext = dcf), rights(xml/wbxml, ext = '.dr/.drc')
    public static final String ACTION_INSTALL_DRM_TO_DEVICE = "installDrmToDevice";

    // 2. secure timer(NTP or DRM time) check and update
    /**
     * Check whether the secure time in drmserver is valid, if not, need sync with NTP server
     */
    public static final String ACTION_CHECK_SECURE_TIME = "checkSecureTime";

    /**
     * When sync valid secure time from NTP server, we need save it to drmserver. Use offset
     * between NTP time with device time to store secure timer, so we need update offset when
     * device time change by user.
     */
    public static final String ACTION_SAVE_SECURE_TIME = "saveSecureTime";
    /**
     * Update offset to make sure secure timer valid, it is always trigger'ed when user change
     * device time.
     */
    public static final String ACTION_UPDATE_OFFSET = "updateOffset";

    // 3. device id, check whether native has get a valid device id, if not, we need get device id
    // from java(DrmProvider) and save to native. device id generate order(16Byte):
    // IMEI, MacAddress, random number
    public static final String ACTION_LOAD_DEVICE_ID = "loadDeviceId";
    public static final String ACTION_SAVE_DEVICE_ID = "saveDeviceId";

    public static final String ACTION_MARK_AS_CONSUME_IN_APP_CLIENT = "markAsConsumeInAppClient";

    /*Add for CTA feature */
    public static final String ACTION_CTA5_CHECKTOKEN = "CTA5Checktoken";
    public static final String ACTION_CTA5_CLEARTOKEN = "CTA5Cleartoken";

    /**
     * Check rights status by fd, some apps may copy drm data to app private path, so only can use
     * fd to check rights status. such as: MMS share SD will copy one to it's provider path.
     */
    public static final String ACTION_CHECK_RIGHTS_STATUS_BY_FD = "checkRightsStatusByFd";

    public static class DrmRequestResult {
        public static final String RESULT_SUCCESS = "success";
        public static final String RESULT_FAILURE = "failure";
    }
}
