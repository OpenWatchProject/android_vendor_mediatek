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
package com.mediatek.digits.iam;

import android.util.Log;
import java.util.ArrayList;
import java.util.HashMap;

public class UserProfile  {
    private static final String TAG = "UserProfile";

    // tmobileId is an unique string for each user
    private HashMap<String, String> mUserDataMap;

    public static final String USER_TMOBILEID_KEY = "tmobileid";

    public static final String USER_LASTNAME_KEY = "TMO_ID_profile_lastName";
    public static final String USER_FIRSTNAME_KEY = "TMO_ID_profile_firstName";
    public static final String USER_ISTMO_KEY = "TMO_ID_profile_isTMO";
    public static final String USER_IMPU1_KEY = "TMO_ID_profile_impu1";
    public static final String USER_IMPU2_KEY = "TMO_ID_profile_impu2";
    public static final String USER_ORIG_TMOBILEID_KEY = "TMO_ID_profile_orig_tmobileid";
    public static final String USER_IMPI1_KEY = "TMO_ID_profile_impi1";
    public static final String USER_EMAIL_KEY = "TMO_ID_profile_email";
    public static final String USER_DEFAULT_TMOACCOUNT_KEY = "associated_lines_defaultTMOAccount";

    public static final String USER_AUTHCODE_KEY = "authcode";
    public static final String USER_REFRESHTOKEN_KEY = "refreshtoken";

    private ArrayList<Line> myLineList = new ArrayList<Line>();
    private AccessToken mAccessToken;

    public UserProfile() {
        mUserDataMap = new HashMap<String, String>();
        Log.d(TAG, "create new UserProfile");
    }

    public void setTmobileId(String tmobileId) {
        mUserDataMap.put(USER_TMOBILEID_KEY, tmobileId);
    }

    public String getTmobileId() {
        return mUserDataMap.get(USER_TMOBILEID_KEY);
    }

    public void saveData(String key, String value) {
        mUserDataMap.put(key, value);
    }

    public String getData(String key) {
        return mUserDataMap.get(key);
    }

    public void addLine(Line line) {
        myLineList.add(line);
    }

    public void setAuthCode(String authCode) {
        mUserDataMap.put(USER_AUTHCODE_KEY, authCode);
    }

    public String getAuthCode() {
        return mUserDataMap.get(USER_AUTHCODE_KEY);
    }

    public void setRefreshToken(String refreshToken) {
        mUserDataMap.put(USER_REFRESHTOKEN_KEY, refreshToken);
    }

    public String getRefreshToken() {
        return mUserDataMap.get(USER_REFRESHTOKEN_KEY);
    }

    public void setAccessToken(AccessToken accessToken) {
        mAccessToken = accessToken;
    }

    public AccessToken getAccessToken() {
        return mAccessToken;
    }

}