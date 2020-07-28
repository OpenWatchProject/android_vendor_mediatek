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

import java.io.UnsupportedEncodingException;
import java.security.GeneralSecurityException;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.HashMap;
import javax.crypto.Cipher;
import javax.crypto.KeyGenerator;
import javax.crypto.SecretKey;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.SecretKeySpec;

import android.content.Context;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Settings;
import android.telephony.TelephonyManager;
import android.util.Base64;
import android.util.Log;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import com.mediatek.digits.DigitsConfig;
import com.mediatek.digits.DigitsSharedPreference;
import com.mediatek.digits.DigitsUtil;
import com.mediatek.digits.utils.Utils;

public class IamManager extends Handler {

    private static final String TAG = "IamManager";
    private static final boolean DEBUG = true;

    private static final Object mRequestIdLock = new Object();
    private static int mRequestIdNumber = 0;

    private Handler mHandler;
    private Context mContext;
    private IamServerApi mServerApi;
    private IamListener mListener;
    // current login user
    private UserProfile mUser;

    private static String mDeviceId = null;

    public static final int CMD_INIT_TMOID_CREATION = 0;
    public static final int CMD_INIT_MSISDN_AUTH = 1;
    public static final int CMD_INIT_MSISDN_AUTH_AND_UPDATE_TMOID = 2;
    public static final int CMD_GET_MSISDN_AUTH_LIST = 3;
    public static final int CMD_GET_CONSUMER_PROFILE = 4;
    public static final int CMD_ISSUE_ACCESS_TOKEN = 5;
    public static final int CMD_REFRESH_ACCESS_TOKEN = 6;
    public static final int CMD_CANCEL_TRANSACTION = 7;
    public static final int CMD_USER_LOGOUT = 8;

    public static final String EVENT_KEY_TMOBILE_ID = "tmobileId";
    public static final String EVENT_KEY_CONSUMER_PROFILE = "consumer_profile";
    public static final String EVENT_KEY_ACCESS_TOKEN = "access_token";
    public static final String EVENT_KEY_TOKEN_TYPE = "token_type";
    public static final String EVENT_KEY_MSISDN = "msisdn";
    public static final String EVENT_KEY_MSISDNLIST = "msisdnlist";

    /*
     * Not define
     */
    public static final int EVENT_INIT_TMOID_CREATION_RESULT = 0;
    /*
     * 1. key: EVENT_KEY_MSISDNLIST, value: HashMap<String(transactionId), MsisdnAuth> instance
     */
    public static final int EVENT_INIT_MSISDN_AUTH_RESULT = 1;
    /*
     * Not define
     */
    public static final int EVENT_INIT_MSISDN_AUTH_AND_UPDATE_TMOID_RESULT = 2;
    /*
     * 1. key: EVENT_KEY_MSISDNLIST, value: HashMap<String(transactionId), MsisdnAuth> instance
     */
    public static final int EVENT_GET_MSISDN_AUTH_LIST_RESULT = 3;
    /*
     * 1. key: EVENT_KEY_TMOBILE_ID, value: String
     * 2. key: EVENT_KEY_CONSUMER_PROFILE,
     *    value: HashMap<String(tmobileId), UserProfile> instance
     */
    public static final int EVENT_GET_CONSUMER_PROFILE_RESULT = 4;
    /*
     * 1. key: EVENT_KEY_TMOBILE_ID, value:String
     * 2. key: EVENT_GET_ACCESS_TOKEN_RESULT,
     *    value: HashMap<String(tmobileId), AccessToken> instance
     */
    public static final int EVENT_GET_ACCESS_TOKEN_RESULT = 5;
    /*
     * 1. key: EVENT_KEY_TMOBILE_ID, value:String
     * 2. key: EVENT_REFRESH_ACCESS_TOKEN_RESULT,
     *    value: HashMap<String(tmobileId), AccessToken> instance
     */
    public static final int EVENT_REFRESH_ACCESS_TOKEN_RESULT = 6;
    /*
     * 1. key: EVENT_KEY_MSISDNLIST, value: HashMap<String(transactionId), MsisdnAuth> instance
     */
    public static final int EVENT_CANCEL_TRANSACTION_RESULT = 7;
    /*
     * Not define
     */
    public static final int EVENT_USER_LOGOUT_RESULT = 8;

    public static final int EVENT_RESULT_SUCCEED = 0;
    public static final int EVENT_RESULT_FAIL = 1;

    /** Interface for callback to notify IAM event */
    public interface IamListener {
        /**
         * callback when any events happened.
         */
        void onIamEvent(int event, int result, Bundle bundle);
    }

    public IamManager(Context context, Looper looper, IamListener listener) {
        super(looper);

        mHandler = this;
        mContext = context;
        mListener = listener;
        mDeviceId = DigitsUtil.getInstance(mContext).getDeviceId();
        mServerApi = new IamServerApi(mContext, DigitsUtil.getInstance(mContext).getDeviceName(), mDeviceId);

        if (mUser == null) {
            log("Create new user and sync access token in shared preference");
            mUser = new UserProfile();

            if (getRefreshTokenFromSP(mContext) != null) {
                mUser.setRefreshToken(getRefreshTokenFromSP(mContext));
            }
        }

    }

    @Override
    public void handleMessage(Message msg) {
        UserProfile userProfile;
        JSONObject respJsonObj;
        String authCode;
        String msisdn;
        String[] strData;

        if (DEBUG) {
            Log.d(TAG, "handleMessage(): event = " + msg.what + ", requestId = " + msg.arg1 +
                  ", mListener=" + mListener);
        }
        switch (msg.what) {
            case CMD_INIT_TMOID_CREATION:
                msisdn = (String) msg.obj;
                doInitiateTmoIdCreation(msisdn);
                break;
            case CMD_INIT_MSISDN_AUTH:
                msisdn = (String) msg.obj;
                doInitiateMsisdnAuth(msisdn);
                break;
            case CMD_INIT_MSISDN_AUTH_AND_UPDATE_TMOID:
                strData = (String[]) msg.obj;
                doInitiateMsisdnAuthAndUpdateTmoId(strData[0], strData[1], strData[2]);
                break;
            case CMD_GET_MSISDN_AUTH_LIST:
                doGetMsisdnAuthList();
                break;
            case CMD_GET_CONSUMER_PROFILE:
                doGetConsumerProfile();
                break;
            case CMD_ISSUE_ACCESS_TOKEN:
                authCode = (String) msg.obj;
                doIssueAccessToken(authCode);
                break;
            case CMD_REFRESH_ACCESS_TOKEN:
                doRefreshAccessToken();
                break;
            case CMD_CANCEL_TRANSACTION:
                strData = (String[]) msg.obj;
                doCancelTransaction(strData[0], strData[1]);
                break;
            case CMD_USER_LOGOUT:
                doUserLogout();
                break;
            default:
                Log.d(TAG, "Unknown Event: " + msg.what);
                break;
        }
    }

    /**
     * Get TMOID login url for setting. It's an synchronous API.
     *
     */
    public String getTmoidLoginURL() {
        int requestId = assignNewRequestId("getTmoidLoginURL");
        String IAMRedirectURL;
        String IAMOAuthClientIDNSDS;
        String IAMOAuthClientIDNSDSScope;
        String EntitlementServerTokenURI;

        DigitsConfig digitsConfig = DigitsConfig.getInstance();

        if (digitsConfig != null) {
            IAMRedirectURL = digitsConfig.getIAMRedirectURL();
            IAMOAuthClientIDNSDS = digitsConfig.getIAMOAuthClientIDNSDS();
            IAMOAuthClientIDNSDSScope = digitsConfig.getIAMOAuthClientIDNSDSScope();
            EntitlementServerTokenURI = digitsConfig.getEntitlementServerTokenURI();

            String loginURL = IAMRedirectURL + "?client_id=" + IAMOAuthClientIDNSDS +
                              "&response_type=code&scope=" + IAMOAuthClientIDNSDSScope +
                              "&redirect_uri=" + EntitlementServerTokenURI +
                              "&access_type=OFFLINE";
            log("getTmoidLoginURL loginURL: " + loginURL);
            return loginURL;
        } else {
            log("Cannot find DigitsConfig instance, return empty string!");
            return "";
        }
    }

    /**
     * Get MSISDN Auth url for setting. It's an synchronous API.
     *
     */
    public String getMsisdnAuthURL() {
        int requestId = assignNewRequestId("getMsisdnAuthURL");

        return null;
    }

    /**
     * Get TMO ID Signup URL for setting. It's an synchronous API.
     *
     */
    public String getTmoidSignupURL() {
        int requestId = assignNewRequestId("getTmoidSignupURL");

        String IAMRedirectSignUpURL;
        DigitsConfig digitsConfig = DigitsConfig.getInstance();

        if (digitsConfig != null) {
            IAMRedirectSignUpURL = digitsConfig.getIAMRedirectSignUpURL();
            log("getTmoidSignupURL IAMRedirectSignUpURL: " + IAMRedirectSignUpURL);

            return IAMRedirectSignUpURL;
        } else {
            log("Cannot find DigitsConfig instance, return empty string!");
            return "";
        }
    }

    /**
     * Get TMOID logout url for setting. It's an synchronous API.
     *
     */
    public String getTmoidLogoutURL() {
        int requestId = assignNewRequestId("getTmoidLogoutURL");
        String signOutURL = "https://account.t-mobile.com/oauth2/v1/signout";

        return signOutURL;
    }

    /**
     * Logout for current user. It's an asynchronous API.
     *
     */
    public void logout() {
        int requestId = assignNewRequestId("logout");

        Message message = obtainMessage(CMD_USER_LOGOUT);
        message.sendToTarget();
    }

    /**
     * Set auth code after login through websheet from setting. It's an synchronous API.
     *
     */
    public void setAuthCode(String authCode) {
        int requestId = assignNewRequestId("setAuthCode");

        Message message = obtainMessage(CMD_ISSUE_ACCESS_TOKEN, (Object)authCode);
        message.sendToTarget();
    }

    /**
     * Refresh access token. It's an asynchronous API.
     *
     */
    public void refreshAccessToken() {
        int requestId = assignNewRequestId("refreshAccessToken");

        Message message = obtainMessage(CMD_REFRESH_ACCESS_TOKEN);
        message.sendToTarget();
    }

    /**
     * Get Current User. It's an synchronous API.
     *
     */
    public UserProfile getCurrentUser() {
        int requestId = assignNewRequestId("getCurrentUser");

        return mUser;
    }

    /**
     * Call after log in. It's an asynchronous API.
     *
     */
    public AccessToken getAccessToken() {
        int requestId = assignNewRequestId("getAccessToken");

        return mUser.getAccessToken();
    }

    /**
     * This method is utilized to request a new dummy TMOID creation for the supplied device.
     * It's an asynchronous API.
     *
     */
    public void initTmoidCreation(String msisdn) {
        int requestId = assignNewRequestId("initTmoidCreation");

        Message message = obtainMessage(CMD_INIT_TMOID_CREATION, (Object)msisdn);
        message.sendToTarget();
    }

    /**
     * Auth new line after user already login with real TMOID. It's an asynchronous API.
     *
     */
    public void initMsisdnAuth(String msisdn) {
        int requestId = assignNewRequestId("initMsisdnAuth");

        Message message = obtainMessage(CMD_INIT_MSISDN_AUTH, (Object)msisdn);
        message.sendToTarget();
    }

    /**
     * Auth second line after login, if user already login with temp TMOID. It's an asynchronous API.
     *
     */
    public void initMsisdnAuthAndUpdateTmoId(String msisdn, String email, String password) {
        int requestId = assignNewRequestId("initMsisdnAuthAndUpdateTmoId");
        String[] data = {msisdn, email, password};

        Message message = obtainMessage(CMD_INIT_MSISDN_AUTH_AND_UPDATE_TMOID, (Object)data);
        message.sendToTarget();
    }

    /**
     * Cancel transaction. It's an asynchronous API.
     *
     */
    public void cancelTransaction (String transactionId, String msisdn) {
        int requestId = assignNewRequestId("cancelTransaction");
        String[] data = {transactionId, msisdn};

        Message message = obtainMessage(CMD_CANCEL_TRANSACTION, (Object)data);
        message.sendToTarget();
    }

    /**
     * Retrieve the list of MSISDN authorization requests. It's an asynchronous API.
     *
     */
    public void getMsisdnAuthList() {
        int requestId = assignNewRequestId("getMsisdnAuthList");

        Message message = obtainMessage(CMD_GET_MSISDN_AUTH_LIST);
        message.sendToTarget();
    }

    /**
     * Query TMOID profile. It's an asynchronous API.
     *
     */
    public void getConsumerProfile() {
        int requestId = assignNewRequestId("getConsumerProfile");

        Message message = obtainMessage(CMD_GET_CONSUMER_PROFILE);
        message.sendToTarget();
    }

    /**
     * Clear cached data
     */
    public void clearCachedData() {
        int requestId = assignNewRequestId("clearCachedData");

        mUser.setRefreshToken(null);
        saveRefreshTokenToSP(mContext, null);
    }

    /**
     * Do initiate TMOID creation.
     *
     */
    private void doInitiateTmoIdCreation(String msisdn) {

        JSONObject reqJsonObj = new JSONObject();
        String formattedCurrentTime, tokenField, deviceId;

        if (msisdn == null || msisdn.length() == 0) {
            mListener.onIamEvent(EVENT_INIT_TMOID_CREATION_RESULT,
                                 EVENT_RESULT_FAIL, null);
            log("doInitiateTmoIdCreation, msisdn is null, should check again !!");
            return;
        }

        DigitsConfig digitsConfig = DigitsConfig.getInstance();
        if (digitsConfig == null) {
            mListener.onIamEvent(EVENT_INIT_TMOID_CREATION_RESULT,
                                 EVENT_RESULT_FAIL, null);
            log("doInitiateTmoIdCreation, digitsConfig is null, should check again !!");
            return;
        }

        if (mDeviceId == null) {
            mDeviceId = DigitsUtil.getInstance(mContext).getDeviceId();
            if (mDeviceId == null) {
                mListener.onIamEvent(EVENT_INIT_TMOID_CREATION_RESULT,
                                     EVENT_RESULT_FAIL, null);
                log("doInitiateTmoIdCreation, deviceId is null, should check again !!");
                return;
            }
        }

        // get current time
        Calendar c = Calendar.getInstance();
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'");
        formattedCurrentTime = sdf.format(c.getTime());
        log("doInitiateTmoIdCreation, formatted current time:" + formattedCurrentTime);

        tokenField = "client_id1=" + digitsConfig.getIAMOAuthClientIDNSDS() +
                     ", scope1=" + digitsConfig.getIAMOAuthClientIDNSDSScope() +
                     ", uri1=" + digitsConfig.getEntitlementServerRedirectURI() +
                     ", msisdn=" + msisdn +
                     ", device-name=" + DigitsUtil.getInstance(mContext).getDeviceName() +
                     ", device-id=" + Utils.base64Encode(mDeviceId) +
                     ", Timestamp=" + formattedCurrentTime;
        log("doInitiateTmoIdCreation, tokenField:" + tokenField);

        try {
            reqJsonObj.put("token", doBlowfish(getBlowfishKey(
                                                   formattedCurrentTime, mContext), tokenField));
            reqJsonObj.put("deviceId", MD5(Utils.base64Encode(mDeviceId)));
            reqJsonObj.put("timeStamp", formattedCurrentTime);

        } catch (JSONException e) {
            mListener.onIamEvent(EVENT_INIT_TMOID_CREATION_RESULT,
                                 EVENT_RESULT_FAIL, null);
            Log.e(TAG, "doInitiateTmoIdCreation, reqJsonObj put Exception:", e);
        }

        // TODO: Later should send to network

        // TODO: Later should trigger to do token exchange
    }

    /**
     * Do initiate MSISDN Authorization request.
     *
     */
    private void doInitiateMsisdnAuth(String msisdn) {

        JSONObject reqJsonObj = new JSONObject();
        String formattedCurrentTime, tokenField, deviceId;

        if (msisdn == null || msisdn.length() == 0) {
            mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                 EVENT_RESULT_FAIL, null);
            log("doInitiateMsisdnAuth, msisdn is null, should check again !!");
            return;
        }

        if (mUser.getAccessToken().getTokenValue() == null ||
                mUser.getAccessToken().getTokenValue().length() == 0) {
            mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                 EVENT_RESULT_FAIL, null);
            log("doInitiateMsisdnAuth, access token never been set, cannot do initiate MSISDN Auth");
            return;
        }

        if (mDeviceId == null) {
            mDeviceId = DigitsUtil.getInstance(mContext).getDeviceId();
            if (mDeviceId == null) {
                mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                     EVENT_RESULT_FAIL, null);
                log("doInitiateMsisdnAuth, deviceId is null, should check again !!");
                return;
            }
        }

        // get current time
        Calendar c = Calendar.getInstance();
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss'Z'");
        formattedCurrentTime = sdf.format(c.getTime());
        log("doInitiateMsisdnAuth, formatted current time:" + formattedCurrentTime);

        tokenField = "msisdn=" + msisdn + ", device-name=" + DigitsUtil.getInstance(mContext).getDeviceName() +
                     ", device-id=" + Utils.base64Encode(mDeviceId) +
                     ", Timestamp=" + formattedCurrentTime;
        log("doInitiateMsisdnAuth, tokenField:" + tokenField);

        try {
            reqJsonObj.put("token", doBlowfish(getBlowfishKey(
                                                   formattedCurrentTime, mContext), tokenField));
            reqJsonObj.put("deviceId", MD5(Utils.base64Encode(mDeviceId)));
            reqJsonObj.put("timeStamp", formattedCurrentTime);

        } catch (JSONException e) {
            mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                 EVENT_RESULT_FAIL, null);
            Log.e(TAG, "doInitiateMsisdnAuth, reqJsonObj put Exception:", e);
        }

        mServerApi.createRequest()
        .initiateMsisdnAuth(mUser, msisdn, reqJsonObj.toString())
        .execute(new IamServerApi.Callback() {
            @Override
            public void callback(IamServerApi.IamResponse iamResponse) {

                JSONObject respJsonObj;

                try {
                    iamResponse.respToString("doInitiateMsisdnAuth");

                    if (iamResponse.isHttpSuccess()) {
                        byte[] data = iamResponse.getResponse().body().bytes();
                        if (data == null) {
                            mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                            log("doInitiateMsisdnAuth, empty byte array");
                            return;
                        }
                        String responseString = Utils.gzipDecompress(data);
                        log("doInitiateMsisdnAuth, sendHttpRequest response=" + responseString);

                        respJsonObj = new JSONObject(responseString);
                        log("doInitiateMsisdnAuth, respJsonObject:" + respJsonObj);

                        if (respJsonObj.getString("statusCode").equals("success")) {
                            HashMap<String, MsisdnAuth> msisdnAuthMap = new HashMap<String, MsisdnAuth>();
                            MsisdnAuth msisdnAuth = new MsisdnAuth(
                                    msisdn,
                                    respJsonObj.getString("statusMessage"),
                                    respJsonObj.getString("transactionId"));
                            msisdnAuthMap.put(respJsonObj.getString("transactionId"), msisdnAuth);

                            Bundle bundle = new Bundle();
                            bundle.putSerializable(EVENT_KEY_MSISDNLIST, msisdnAuthMap);
                            mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                                 EVENT_RESULT_SUCCEED, bundle);
                        } else {
                            log("doInitiateMsisdnAuth, fail with statusCode " +
                                respJsonObj.getString("statusCode"));
                            mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                        }

                    } else {
                        Log.e(TAG, "doInitiateMsisdnAuth, bad HTTP response:" +
                              iamResponse.getResponse() + ", bad HTTP response body:" +
                              iamResponse.getResponse().body().string());
                        Bundle bundle = new Bundle();
                        bundle.putString(EVENT_KEY_MSISDN, msisdn);
                        mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                             EVENT_RESULT_FAIL, bundle);
                    }
                    iamResponse.getResponse().body().close();

                } catch (Exception e) {
                    mListener.onIamEvent(EVENT_INIT_MSISDN_AUTH_RESULT,
                                         EVENT_RESULT_FAIL, null);
                    Log.e(TAG, "doInitiateMsisdnAuth, sendHttpRequest Exception:", e);
                }
            }
        });

    }

    /**
     * Do initiate MSISDN Authorization request and update dummy TMOID.
     * On 2nd line request devcie needs to force a real TMO ID creation.
     *
     */
    private void doInitiateMsisdnAuthAndUpdateTmoId(String msisdn, String emailId, String password) {

      // Need implement after here
    }

    /**
     * Do to get MSISDN Authorication list
     *
     */
    private void doGetMsisdnAuthList() {

        if (mUser == null) {
            mListener.onIamEvent(EVENT_GET_MSISDN_AUTH_LIST_RESULT, EVENT_RESULT_FAIL, null);
            log("doGetMsisdnAuthList, User never been created, cannot get msisdn auth list");
            return;
        }

        if (mUser.getAccessToken() == null ||
                mUser.getAccessToken().getTokenValue() == null ||
                mUser.getAccessToken().getTokenValue().length() == 0) {
            mListener.onIamEvent(EVENT_GET_MSISDN_AUTH_LIST_RESULT, EVENT_RESULT_FAIL, null);
            log("doGetMsisdnAuthList, Access token never been set, cannot get msisdn auth list");
            return;
        }

        mServerApi.createRequest()
        .getMsisdnAuthList(mUser)
        .execute(new IamServerApi.Callback() {
            @Override
            public void callback(IamServerApi.IamResponse iamResponse) {

                JSONObject respJsonObj;
                JSONArray msisdnJsonArray;
                String msisdn, status, transactionId;

                try {
                    iamResponse.respToString("doGetMsisdnAuthList");

                    if (iamResponse.isHttpSuccess()) {
                        byte[] data = iamResponse.getResponse().body().bytes();
                        if (data == null) {
                            mListener.onIamEvent(EVENT_GET_MSISDN_AUTH_LIST_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                            log("doGetMsisdnAuthList, empty byte array");
                            return;
                        }
                        String responseString = Utils.gzipDecompress(data);
                        log("doGetMsisdnAuthList, sendHttpRequest response=" + responseString);

                        respJsonObj = new JSONObject(responseString);
                        log("doGetMsisdnAuthList, respJsonObject:" + respJsonObj);

                        if (respJsonObj.getString("statusCode").equals("success")) {

                            HashMap<String, MsisdnAuth> msisdnAuthMap =
                                new HashMap<String, MsisdnAuth>();

                            msisdnJsonArray = respJsonObj.getJSONArray("msisdns");
                            for (int i = 0 ; i < msisdnJsonArray.length(); i++) {
                                JSONObject obj = msisdnJsonArray.getJSONObject(i);
                                msisdn = obj.getString("msisdn");
                                status = obj.getString("status");
                                transactionId = obj.getString("transactionId");

                                MsisdnAuth msisdnAuth = new MsisdnAuth(msisdn, status,
                                                                       transactionId);
                                msisdnAuthMap.put(transactionId, msisdnAuth);
                            }

                            Bundle bundle = new Bundle();
                            bundle.putSerializable(EVENT_KEY_MSISDNLIST, msisdnAuthMap);
                            mListener.onIamEvent(EVENT_GET_MSISDN_AUTH_LIST_RESULT,
                                                 EVENT_RESULT_SUCCEED, bundle);
                        } else {
                            log("doGetMsisdnAuthList, fail with statusCode " +
                                respJsonObj.getString("statusCode"));
                            mListener.onIamEvent(EVENT_GET_MSISDN_AUTH_LIST_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                        }

                    } else {
                        Log.e(TAG, "doGetMsisdnAuthList, bad HTTP response:" +
                              iamResponse.getResponse() + ", bad HTTP response body:" +
                              iamResponse.getResponse().body().string());

                        mListener.onIamEvent(EVENT_GET_MSISDN_AUTH_LIST_RESULT,
                                             EVENT_RESULT_FAIL, null);
                    }
                    iamResponse.getResponse().body().close();

                } catch (Exception e) {
                    mListener.onIamEvent(EVENT_GET_MSISDN_AUTH_LIST_RESULT,
                                         EVENT_RESULT_FAIL, null);
                    Log.e(TAG, "doGetMsisdnAuthList, sendHttpRequest Exception:", e);
                }
            }
        });
    }

    /**
     * Do to exchange an authorization code from IAM for an access token.
     *
     */
    private void doIssueAccessToken (String authCode) {

        if (authCode == null || authCode.length() == 0) {
            mListener.onIamEvent(EVENT_GET_ACCESS_TOKEN_RESULT, EVENT_RESULT_FAIL, null);
            log("doIssueAccessToken, authCode is null, should check again !!");
            return;
        }

        if (mUser != null) {
            log("doIssueAccessToken, Create new user and ignore original");
        }
        mUser = new UserProfile();
        mUser.setAuthCode(authCode);

        mServerApi.createRequest()
        .issueAccessToken(mUser.getAuthCode())
        .execute(new IamServerApi.Callback() {
            @Override
            public void callback(IamServerApi.IamResponse iamResponse) {

                JSONObject respJsonObj;

                try {
                    iamResponse.respToString("doIssueAccessToken");

                    if (iamResponse.isHttpSuccess()) {
                        byte[] data = iamResponse.getResponse().body().bytes();
                        if (data == null) {
                            mListener.onIamEvent(EVENT_GET_ACCESS_TOKEN_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                            log("doIssueAccessToken, empty byte array");
                            return;
                        }
                        String responseString = Utils.gzipDecompress(data);
                        log("doIssueAccessToken, sendHttpRequest response=" + responseString);

                        respJsonObj = new JSONObject(responseString);
                        log("doIssueAccessToken, respJsonObject:" + respJsonObj);

                        AccessToken accessToken = saveAccessToken(respJsonObj);

                        HashMap<String, AccessToken> accessTokenMap = new HashMap<String, AccessToken>();
                        accessTokenMap.put(mUser.getTmobileId(), accessToken);

                        Bundle bundle = new Bundle();
                        bundle.putString(EVENT_KEY_TMOBILE_ID, mUser.getTmobileId());
                        bundle.putSerializable(EVENT_KEY_ACCESS_TOKEN, accessTokenMap);

                        mListener.onIamEvent(EVENT_GET_ACCESS_TOKEN_RESULT,
                                             EVENT_RESULT_SUCCEED, bundle);

                    } else {
                        Log.e(TAG, "doIssueAccessToken, bad HTTP response:" +
                              iamResponse.getResponse() + ", bad HTTP response body:" +
                              iamResponse.getResponse().body().string());

                        mListener.onIamEvent(EVENT_GET_ACCESS_TOKEN_RESULT,
                                             EVENT_RESULT_FAIL, null);
                    }
                    iamResponse.getResponse().body().close();

                } catch (Exception e) {
                    mListener.onIamEvent(EVENT_GET_ACCESS_TOKEN_RESULT,
                                         EVENT_RESULT_FAIL, null);
                    Log.e(TAG, "doIssueAccessToken, sendHttpRequest Exception:", e);
                }
            }
        });
    }

    /**
     * Do to obtain a new access token when the old access token expires.
     *
     */
    private void doRefreshAccessToken () {

        if (mUser == null) {
            mListener.onIamEvent(EVENT_REFRESH_ACCESS_TOKEN_RESULT, EVENT_RESULT_FAIL, null);
            log("doRefreshAccessToken, User never been created, cannot refresh access token");
            return;
        }

        if (mUser.getRefreshToken() == null || mUser.getRefreshToken().length() == 0) {
            mListener.onIamEvent(EVENT_REFRESH_ACCESS_TOKEN_RESULT, EVENT_RESULT_FAIL, null);
            log("doRefreshAccessToken, Refresh token never been set, cannot refresh access token");
            return;
        }

        mServerApi.createRequest()
        .refreshAccessToken(mUser)
        .execute(new IamServerApi.Callback() {
            @Override
            public void callback(IamServerApi.IamResponse iamResponse) {

                JSONObject respJsonObj;

                try {
                    iamResponse.respToString("doRefreshAccessToken");

                    if (iamResponse.isHttpSuccess()) {
                        byte[] data = iamResponse.getResponse().body().bytes();
                        if (data == null) {
                            mListener.onIamEvent(EVENT_REFRESH_ACCESS_TOKEN_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                            log("doRefreshAccessToken, empty byte array");
                            return;
                        }
                        String responseString = Utils.gzipDecompress(data);
                        log("doRefreshAccessToken, sendHttpRequest response=" + responseString);

                        respJsonObj = new JSONObject(responseString);
                        log("doRefreshAccessToken, respJsonObject:" + respJsonObj);

                        AccessToken accessToken = saveAccessToken(respJsonObj);

                        HashMap<String, AccessToken> accessTokenMap = new HashMap<String, AccessToken>();
                        accessTokenMap.put(mUser.getTmobileId(), accessToken);

                        Bundle bundle = new Bundle();
                        bundle.putString(EVENT_KEY_TMOBILE_ID, mUser.getTmobileId());
                        bundle.putSerializable(EVENT_KEY_ACCESS_TOKEN, accessTokenMap);

                        mListener.onIamEvent(EVENT_REFRESH_ACCESS_TOKEN_RESULT,
                                             EVENT_RESULT_SUCCEED, bundle);

                    } else {
                        Log.e(TAG, "doRefreshAccessToken, bad HTTP response:" +
                              iamResponse.getResponse() + ", bad HTTP response body:" +
                              iamResponse.getResponse().body().string());

                        mListener.onIamEvent(EVENT_REFRESH_ACCESS_TOKEN_RESULT,
                                             EVENT_RESULT_FAIL, null);
                    }
                    iamResponse.getResponse().body().close();

                } catch (Exception e) {
                    mListener.onIamEvent(EVENT_REFRESH_ACCESS_TOKEN_RESULT,
                                         EVENT_RESULT_FAIL, null);
                    Log.e(TAG, "doRefreshAccessToken, sendHttpRequest Exception:", e);
                }

            }
        });
    }

    /**
     * Do to use the access token to get consumer profile query.
     *
     */
    private void doGetConsumerProfile () {

        if (mUser == null) {
            mListener.onIamEvent(EVENT_GET_CONSUMER_PROFILE_RESULT, EVENT_RESULT_FAIL, null);
            log("doGetConsumerProfile, User never been created, cannot get consumer profile");
            return;
        }

        if (mUser.getAccessToken() == null ||
                mUser.getAccessToken().getTokenValue() == null ||
                mUser.getAccessToken().getTokenValue().length() == 0) {
            mListener.onIamEvent(EVENT_GET_CONSUMER_PROFILE_RESULT, EVENT_RESULT_FAIL, null);
            log("doGetConsumerProfile, Access token never been set, cannot get consumer profile");
            return;
        }

        mServerApi.createRequest()
        .getConsumerProfile(mUser)
        .execute(new IamServerApi.Callback() {
            @Override
            public void callback(IamServerApi.IamResponse iamResponse) {

                JSONObject respJsonObj;

                try {
                    iamResponse.respToString("doGetConsumerProfile");

                    if (iamResponse.isHttpSuccess()) {
                        byte[] data = iamResponse.getResponse().body().bytes();
                        if (data == null) {
                            mListener.onIamEvent(EVENT_GET_CONSUMER_PROFILE_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                            log("doGetConsumerProfile, empty byte array");
                            return;
                        }
                        String responseString = Utils.gzipDecompress(data);
                        log("doGetConsumerProfile, sendHttpRequest response=" + responseString);

                        respJsonObj = new JSONObject(responseString);
                        log("doGetConsumerProfile, respJsonObject:" + respJsonObj);

                        // save user data here
                        saveConsumerProfile(respJsonObj);

                        HashMap<String, UserProfile> userMap = new HashMap<String, UserProfile>();
                        userMap.put(mUser.getTmobileId(), mUser);

                        Bundle bundle = new Bundle();
                        bundle.putString(EVENT_KEY_TMOBILE_ID, mUser.getTmobileId());
                        bundle.putSerializable(EVENT_KEY_CONSUMER_PROFILE, userMap);
                        mListener.onIamEvent(EVENT_GET_CONSUMER_PROFILE_RESULT,
                                             EVENT_RESULT_SUCCEED, bundle);

                    } else {
                        Log.e(TAG, "doGetConsumerProfile, bad HTTP response:" +
                              iamResponse.getResponse() + ", bad HTTP response body:" +
                              iamResponse.getResponse().body().string());
                        mListener.onIamEvent(EVENT_GET_CONSUMER_PROFILE_RESULT,
                                             EVENT_RESULT_FAIL, null);
                    }
                    iamResponse.getResponse().body().close();

                } catch (Exception e) {
                    mListener.onIamEvent(EVENT_GET_CONSUMER_PROFILE_RESULT,
                                         EVENT_RESULT_FAIL, null);
                    Log.e(TAG, "doGetConsumerProfile, sendHttpRequest Exception:", e);
                }

            }
        });
    }

    /**
     * Do to cancel a transaction it had requested MSISDN authorication previously.
     *
     */
    private void doCancelTransaction(String transactionId, String msisdn) {

        JSONObject reqJsonObj = new JSONObject();

        if (mUser == null) {
            mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT, EVENT_RESULT_FAIL, null);
            log("doCancelTransaction, User never been created, cannot cancel transaction");
            return;
        }

        if (mUser.getAccessToken() == null ||
                mUser.getAccessToken().getTokenValue() == null ||
                mUser.getAccessToken().getTokenValue().length() == 0) {
            mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT, EVENT_RESULT_FAIL, null);
            log("doCancelTransaction, Access token never been set, cannot cancel transaction");
            return;
        }

        try {
            reqJsonObj.put("msisdn", msisdn);
            reqJsonObj.put("transactionId", transactionId);

        } catch (JSONException e) {
            mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT,
                                 EVENT_RESULT_FAIL, null);
            Log.e(TAG, "doCancelTransaction, reqJsonObj put Exception:", e);
        }

        mServerApi.createRequest()
        .cancelTransaction(mUser, reqJsonObj.toString())
        .execute(new IamServerApi.Callback() {
            @Override
            public void callback(IamServerApi.IamResponse iamResponse) {

                JSONObject respJsonObj;

                try {
                    iamResponse.respToString("doCancelTransaction");

                    if (iamResponse.isHttpSuccess()) {
                        byte[] data = iamResponse.getResponse().body().bytes();
                        if (data == null) {
                            mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                            log("doCancelTransaction, empty byte array");
                            return;
                        }
                        String responseString = Utils.gzipDecompress(data);
                        log("doCancelTransaction, sendHttpRequest response=" + responseString);

                        respJsonObj = new JSONObject(responseString);
                        log("doCancelTransaction, respJsonObject:" + respJsonObj);

                        if (respJsonObj.getString("statusCode").equals("success")) {
                            HashMap<String, MsisdnAuth> msisdnAuthMap = new HashMap<String, MsisdnAuth>();
                            MsisdnAuth msisdnAuth = new MsisdnAuth(
                                    msisdn,
                                    respJsonObj.getString("statusMessage"),
                                    transactionId);
                            msisdnAuthMap.put(transactionId, msisdnAuth);

                            Bundle bundle = new Bundle();
                            bundle.putSerializable(EVENT_KEY_MSISDNLIST, msisdnAuthMap);
                            mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT,
                                                 EVENT_RESULT_SUCCEED, bundle);
                        } else {
                            log("doCancelTransaction, fail with statusCode " +
                                respJsonObj.getString("statusCode"));
                            mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                        }

                    } else {
                        Log.e(TAG, "doCancelTransaction, bad HTTP response:" +
                              iamResponse.getResponse() + ", bad HTTP response body:" +
                              iamResponse.getResponse().body().string());
                        Bundle bundle = new Bundle();
                        bundle.putString(EVENT_KEY_MSISDN, msisdn);
                        mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT,
                                             EVENT_RESULT_FAIL, bundle);
                    }
                    iamResponse.getResponse().body().close();

                } catch (Exception e) {
                    mListener.onIamEvent(EVENT_CANCEL_TRANSACTION_RESULT,
                                         EVENT_RESULT_FAIL, null);
                    Log.e(TAG, "doCancelTransaction, sendHttpRequest Exception:", e);
                }

            }
        });
    }

    /**
     * Do logout for current user.
     *
     */
    private void doUserLogout() {

        if (mUser == null) {
            mListener.onIamEvent(EVENT_USER_LOGOUT_RESULT, EVENT_RESULT_FAIL, null);
            log("doUserLogout, User never been created, cannot do logout");
            return;
        }

        if (mUser.getAccessToken() == null ||
                mUser.getAccessToken().getTokenValue() == null ||
                mUser.getAccessToken().getTokenValue().length() == 0) {
            mListener.onIamEvent(EVENT_USER_LOGOUT_RESULT, EVENT_RESULT_FAIL, null);
            log("doUserLogout, Access token never been set, cannot do logout");
            return;
        }

        mServerApi.createRequest()
        .logout(mUser)
        .execute(new IamServerApi.Callback() {
            @Override
            public void callback(IamServerApi.IamResponse iamResponse) {

                JSONObject respJsonObj;

                try {
                    iamResponse.respToString("doUserLogout");

                    if (iamResponse.isHttpSuccess()) {
                        byte[] data = iamResponse.getResponse().body().bytes();
                        if (data == null) {
                            mListener.onIamEvent(EVENT_USER_LOGOUT_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                            log("doUserLogout, empty byte array");
                            return;
                        }
                        String responseString = Utils.gzipDecompress(data);
                        log("doUserLogout, sendHttpRequest response=" + responseString);

                        respJsonObj = new JSONObject(responseString);
                        log("doUserLogout, respJsonObject:" + respJsonObj);

                        if (respJsonObj.getString("isSignedOut").equals("true")) {
                            log("doUserLogout, isSignedOut true");
                            mListener.onIamEvent(EVENT_USER_LOGOUT_RESULT,
                                                 EVENT_RESULT_SUCCEED, null);
                        } else {
                            log("doUserLogout, isSignedOut not true");
                            mListener.onIamEvent(EVENT_USER_LOGOUT_RESULT,
                                                 EVENT_RESULT_FAIL, null);
                        }
                    } else {
                        Log.e(TAG, "doUserLogout, bad HTTP response:" +
                              iamResponse.getResponse() + ", bad HTTP response body:" +
                              iamResponse.getResponse().body().string());
                        mListener.onIamEvent(EVENT_USER_LOGOUT_RESULT,
                                             EVENT_RESULT_FAIL, null);
                    }
                    iamResponse.getResponse().body().close();

                } catch (Exception e) {
                    mListener.onIamEvent(EVENT_USER_LOGOUT_RESULT,
                                         EVENT_RESULT_FAIL, null);
                    Log.e(TAG, "doUserLogout, sendHttpRequest Exception:", e);
                }

            }
        });
    }

    /**
     * Save refresh token to SP for auto test
     *
     */
    public void setRefreshToken (String refreshToken) {

        if (mUser == null) {
            log("setRefreshToken, create new user to save refresh token");
            mUser = new UserProfile();
        }

        if (getRefreshTokenFromSP(mContext) != null) {
            log("setRefreshToken, there's original value for refresh token:" +
                    getRefreshTokenFromSP(mContext));
        }

        mUser.setRefreshToken(refreshToken);
        saveRefreshTokenToSP(mContext, refreshToken);
        log("setRefreshToken, save new refresh token value:" + refreshToken);
    }

    /**
     * Save access token after retrieve from server
     *
     */
    private AccessToken saveAccessToken (JSONObject respJsonObj) {
        AccessToken accessToken;
        long currentTime = System.currentTimeMillis();

        try {
            accessToken = new AccessToken(respJsonObj.getString("access_token"),
                                          respJsonObj.getString("token_type"),
                                          respJsonObj.getString("scope"),
                                          respJsonObj.getString("expires_in"),
                                          currentTime);
            mUser.setAccessToken(accessToken);
            mUser.setRefreshToken(respJsonObj.getString("refresh_token"));
            mUser.setTmobileId(respJsonObj.getString("tmobileid"));

            // Sync refresh token to share preference
            saveRefreshTokenToSP(mContext, respJsonObj.getString("refresh_token"));

            log("CMD_SAVE_ISSUE_TOKEN: Save access token value:" +
                respJsonObj.getString("access_token") + ", token type:" +
                respJsonObj.getString("token_type") + ", refresh token:" +
                respJsonObj.getString("refresh_token") + ", tmobileid:" +
                respJsonObj.getString("tmobileid") + ", current Time:" +
                Long.toString(currentTime));
            return mUser.getAccessToken();
        } catch (JSONException e) {
            log("CMD_SAVE_ISSUE_TOKEN: JSONException error " + e);
            return null;
        }
    }

    /**
     * Save consumer profile after retrieve user profile.
     *
     */
    private void saveConsumerProfile (JSONObject respJsonObj) {
        JSONArray linesArray;

        try {
            mUser.setTmobileId(respJsonObj.getString("tmobileid"));

            mUser.saveData(UserProfile.USER_LASTNAME_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("lastName"));
            mUser.saveData(UserProfile.USER_FIRSTNAME_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("firstName"));
            mUser.saveData(UserProfile.USER_ISTMO_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("isTMO"));
            mUser.saveData(UserProfile.USER_IMPU1_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("impu1"));
            mUser.saveData(UserProfile.USER_IMPU2_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("impu2"));
            mUser.saveData(UserProfile.USER_ORIG_TMOBILEID_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("orig_tmobileid"));
            mUser.saveData(UserProfile.USER_IMPI1_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("impi1"));
            mUser.saveData(UserProfile.USER_EMAIL_KEY,
                           respJsonObj.getJSONObject("TMO_ID_profile").getString("email"));
            mUser.saveData(UserProfile.USER_DEFAULT_TMOACCOUNT_KEY,
                           respJsonObj.getJSONObject("associated_lines").getString("defaultTMOAccount"));

            linesArray = respJsonObj.getJSONObject("associated_lines").getJSONArray("lines");
            for (int i = 0 ; i < linesArray.length(); i++) {
                JSONObject obj = linesArray.getJSONObject(i);
                Line line = new Line(obj.getString("MSISDN"),
                                     obj.getString("tmBAN"),
                                     obj.getString("SOC"),
                                     obj.getString("givenName"),
                                     obj.getString("tmIMSI"),
                                     obj.getString("tmSubscriberType"),
                                     obj.getString("lineId"),
                                     obj.getString("tmAccountStatus"),
                                     obj.getString("accountStatus"),
                                     obj.getString("tmEmail"),
                                     obj.getString("customerType"),
                                     obj.getString("tmAuthZipFailCount"),
                                     obj.getString("tmbillingbirthdate"),
                                     obj.getString("IMSI"),
                                     obj.getString("customerId"));
                mUser.addLine(line);
            }
        } catch (Exception e) {
            Log.e(TAG, "saveConsumerProfile, Exception:", e);
        }

    }

    /**
     * Save refresh token to share preference.
     *
     */
    private void saveRefreshTokenToSP(Context context, String refreshToken) {
        DigitsSharedPreference pref = DigitsSharedPreference.getInstance(context);
        if (pref != null) {
            pref.saveRefreshToken(refreshToken);
        }
    }

    /**
     * Get refresh token from share preference.
     *
     */
    private String getRefreshTokenFromSP(Context context) {
        DigitsSharedPreference pref = DigitsSharedPreference.getInstance(context);
        if (pref != null) {
            return pref.getRefreshToken();
        } else {
            return null;
        }
    }

    /**
     * Utils function execute Blowfish algorithm
     */
    private static String doBlowfish(String keyMd5, String plainText) {
        String encryptedString = null;
        SecretKeySpec secretKeySpec;
        Cipher cipher;
        byte[] encrypted;

        try {
            secretKeySpec = new SecretKeySpec(string2byte(keyMd5), "Blowfish");
            cipher = Cipher.getInstance("Blowfish/ECB/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, secretKeySpec);

            encrypted = cipher.doFinal(string2byte(plainText));
            encryptedString = Base64.encodeToString(encrypted, Base64.DEFAULT);
            log("doBlowfish result: " + encryptedString);

        } catch (GeneralSecurityException e) {
            Log.e(TAG, "doBlowfish, some general security exception");
        }
        return encryptedString;
    }

    /**
     * Get Blowfish key for digits service.
     */
    private static String getBlowfishKey(String timestamp, Context context) {
        String blowfishKey = null;

        DigitsConfig digitsConfig = DigitsConfig.getInstance();
        if (digitsConfig != null) {

            blowfishKey = MD5(digitsConfig.getBfEncText() +
                            MD5(Utils.base64Encode(mDeviceId)) +
                            MD5(timestamp));
            log("blowfishKey result: " + blowfishKey);
        } else {
            log("Cannot find DigitsConfig instance, return false!");
        }

        return blowfishKey;
    }

    /**
     * Utils function to execute MD5 algorithm
     */
    public static String MD5(String data) {
        if (data == null) {
            log("MD5 input data is null");
            return "";
        }

        try {
            MessageDigest md = MessageDigest.getInstance("MD5");
            byte[] array = md.digest(data.getBytes());
            StringBuffer sb = new StringBuffer();
            for (int i = 0; i < array.length; ++i) {
                sb.append(Integer.toHexString((array[i] & 0xFF) | 0x100).substring(1,3));
            }
            return sb.toString();
        } catch (NoSuchAlgorithmException e) {

        }
        return null;
    }

    /**
     * Utils function for string to byte format
     */
    private static byte[] string2byte(String s) {
        byte[] bytes = null;
        if (s != null) {
            try {
                bytes = s.getBytes("UTF-8");
            } catch (UnsupportedEncodingException e) {

            }
        }
        return bytes;
    }

    /**
     * Assign new request Id for public API
     */
    private int assignNewRequestId(String functionName) {
        int requestId;

        synchronized (mRequestIdLock) {
            requestId = mRequestIdNumber;
            mRequestIdNumber++;
        }
        if (DEBUG) {
            Log.d(TAG, functionName + "(): requestId = " + requestId);
        }
        return requestId;
    }

    private static void log(String s) {
        if (DEBUG) Log.d(TAG, s);
    }

    public static String eventToString(int event) {
        switch(event) {
            case EVENT_INIT_TMOID_CREATION_RESULT:
                return "EVENT_INIT_TMOID_CREATION_RESULT";
            case EVENT_INIT_MSISDN_AUTH_RESULT:
                return "EVENT_INIT_MSISDN_AUTH_RESULT";
            case EVENT_INIT_MSISDN_AUTH_AND_UPDATE_TMOID_RESULT:
                return "EVENT_INIT_MSISDN_AUTH_AND_UPDATE_TMOID_RESULT";
            case EVENT_GET_MSISDN_AUTH_LIST_RESULT:
                return "EVENT_GET_MSISDN_AUTH_LIST_RESULT";
            case EVENT_GET_CONSUMER_PROFILE_RESULT:
                return "EVENT_GET_CONSUMER_PROFILE_RESULT";
            case EVENT_GET_ACCESS_TOKEN_RESULT:
                return "EVENT_GET_ACCESS_TOKEN_RESULT";
            case EVENT_REFRESH_ACCESS_TOKEN_RESULT:
                return "EVENT_REFRESH_ACCESS_TOKEN_RESULT";
            case EVENT_CANCEL_TRANSACTION_RESULT:
                return "EVENT_CANCEL_TRANSACTION_RESULT";
            case EVENT_USER_LOGOUT_RESULT:
                return "EVENT_USER_LOGOUT_RESULT";
            default:
                return "UNKNOWN_EVENT(" + event + ")";
        }
    }
}
