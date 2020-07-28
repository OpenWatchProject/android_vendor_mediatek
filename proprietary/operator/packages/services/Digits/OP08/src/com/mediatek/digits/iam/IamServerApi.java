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

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.text.TextUtils;

import com.android.okhttp.FormEncodingBuilder;
import com.android.okhttp.Headers;
import com.android.okhttp.HttpUrl;
import com.android.okhttp.MediaType;
import com.android.okhttp.OkHttpClient;
import com.android.okhttp.Protocol;
import com.android.okhttp.Request;
import com.android.okhttp.RequestBody;
import com.android.okhttp.Response;

import com.mediatek.digits.DigitsConfig;
import com.mediatek.digits.utils.Utils;

import java.io.IOException;
import java.io.UnsupportedEncodingException;
import java.net.URLEncoder;
import java.util.HashMap;
import java.util.Map;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

public class IamServerApi {
    private static final String TAG = "IamServerApi";
    private static final boolean DEBUG = true;

    private final Context mContext;
    private HandlerThread mThread;
    private Handler mThreadHandler;

    public static final MediaType MEDIA_TYPE_JSON =
            MediaType.parse("application/json");
    public static final MediaType MEDIA_TYPE_DEFAULT =
            MediaType.parse("application/x-www-form-urlencoded");

    private final String mDeviceId;
    private final String mDeviceName;

    public IamServerApi(Context context, String deviceName, String deviceId) {
        mContext = context;
        mDeviceName = deviceName;
        mDeviceId = deviceId;
        log("retrieve mDeviceName=" + mDeviceName + ", mDeviceId=" + mDeviceId);

        mThread = new HandlerThread("IamServerApi-worker");
        mThread.start();
        mThreadHandler = new Handler(mThread.getLooper());
    }

    public IamMethodBuilder createRequest() {
        return new IamMethodBuilder(this);
    }

    boolean execute(final Request request, final Callback callback) {
        if (checkNoHandlerThread()) {
            return false;
        }

        mThreadHandler.post(new Runnable() {
            public void run() {
                OkHttpClient httpClient = new OkHttpClient();
                IamResponse rsp = null;

                rsp = sendHttpRequest(httpClient, request);

                if (callback != null && rsp != null) {
                    log("execute: callback the response");
                    callback.callback(rsp);
                }
            }
        });

        return true;
    }

    public interface Callback {
        void callback(IamResponse response);
    }

    public class IamResponse {
        private Request mRequest = null;
        private Response mResponse = null;

        IamResponse(Request request, Response response) {
            mRequest = request;
            mResponse = response;
        }

        public boolean isHttpSuccess() {
            return mResponse.isSuccessful();
        }

        public int getRespStatusCode() {
            return mResponse.code();
        }

        public Headers getRespHeaders() {
            return mResponse.headers();
        }

        public HttpUrl getReqHttpUrl() {
            return mRequest.httpUrl();
        }

        public Response getResponse() {
            return mResponse;
        }

        public void respToString(String tagName) {
            log(tagName + ", IamHttpResponse: code=" + mResponse.code());
            log(tagName + ", IamHttpResponse: message=" + mResponse.message());
            log(tagName + ", IamHttpResponse: headers=" + mResponse.headers().toString());
        }
    }

    public class IamMethodBuilder {
        private IamServerApi mService;
        private Request mRequest = null;

        private IamMethodBuilder(IamServerApi service) {
            mService = service;
        }

        public boolean execute(Callback callback) {
            if (mService != null) {
                return mService.execute(mRequest, callback);
            }
            return false;
        }

        public IamMethodBuilder issueAccessToken(String authCode) {
            log("issueAccessToken:");

            DigitsConfig digitsConfig = DigitsConfig.getInstance();
            if (digitsConfig != null) {
                RequestBody formBody = new FormEncodingBuilder()
                        .add("redirect_uri", digitsConfig.getEntitlementServerRedirectURI())
                        .add("code", authCode)
                        .add("client_id", digitsConfig.getIAMOAuthClientIDNSDS())
                        .add("scope", digitsConfig.getIAMOAuthClientIDNSDSScope())
                        .add("grant_type", "authorization_code")
                        .add("access_type", "offline")
                        .build();
                log("issueAccessToken, authCode: " + authCode);

                mRequest = new Request.Builder()
                        .url(digitsConfig.getEntitlementServerTokenURI())
                        .header("Content-type", "application/x-www-form-urlencoded")
                        .post(formBody)
                        .build();

            } else {
                log("Cannot find DigitsConfig instance, return false!");
            }

            return this;
        }

        public IamMethodBuilder refreshAccessToken(UserProfile userProfile) {
            log("refreshAccessToken:");

            DigitsConfig digitsConfig = DigitsConfig.getInstance();
            if (digitsConfig != null) {
                RequestBody formBody = new FormEncodingBuilder()
                        .add("client_id", digitsConfig.getIAMOAuthClientIDNSDS())
                        .add("grant_type", "refresh_token")
                        .add("refresh_token", userProfile.getRefreshToken())
                        .build();
                log("refreshAccessToken, refresh_token: " + userProfile.getRefreshToken());

                mRequest = new Request.Builder()
                        .url(digitsConfig.getEntitlementServerTokenURI())
                        .header("Content-type", "application/x-www-form-urlencoded")
                        .post(formBody)
                        .build();

            } else {
                log("Cannot find DigitsConfig instance, return false!");
            }

            return this;
        }

        public IamMethodBuilder getConsumerProfile(UserProfile userProfile) {
            log("getConsumerProfile:");

            DigitsConfig digitsConfig = DigitsConfig.getInstance();
            if (digitsConfig != null) {

                mRequest = new Request.Builder()
                        .url(digitsConfig.getIAMConsumerProfileURI())
                        .header("Content-type", "application/json")
                        .header("Authorization", "Bearer " +
                                userProfile.getAccessToken().getTokenValue())
                        .build();

            } else {
                log("Cannot find DigitsConfig instance, return false!");
            }

            return this;
        }

        public IamMethodBuilder initiateMsisdnAuth(UserProfile userProfile, String msisdn, String jsonBody) {
            log("initiateMsisdnAuth:");

            DigitsConfig digitsConfig = DigitsConfig.getInstance();
            if (digitsConfig != null) {

                RequestBody body = RequestBody.create(MEDIA_TYPE_JSON, jsonBody);
                mRequest = new Request.Builder()
                        .url(digitsConfig.getCNSMWAuthorizeURL())
                        .header("Content-type", "application/json")
                        .header("Accept", "application/json")
                        .header("Authorization", "Bearer " +
                                userProfile.getAccessToken().getTokenValue())
                        .post(body)
                        .build();

            } else {
                log("Cannot find DigitsConfig instance, return false!");
            }

            return this;
        }

        public IamMethodBuilder getMsisdnAuthList(UserProfile userProfile) {
            log("getMsisdnAuthList:");

            DigitsConfig digitsConfig = DigitsConfig.getInstance();
            if (digitsConfig != null) {

                mRequest = new Request.Builder()
                        .url(digitsConfig.getCNSMWQueryURL())
                        .header("Content-type", "application/json")
                        .header("Accept", "application/json")
                        .header("Authorization", "Bearer " +
                                userProfile.getAccessToken().getTokenValue())
                        .build();

            } else {
                log("Cannot find DigitsConfig instance, return false!");
            }

            return this;
        }

        public IamMethodBuilder cancelTransaction(UserProfile userProfile, String jsonBody) {
            log("cancelTransaction:");

            DigitsConfig digitsConfig = DigitsConfig.getInstance();
            if (digitsConfig != null) {

                RequestBody body = RequestBody.create(MEDIA_TYPE_JSON, jsonBody);
                mRequest = new Request.Builder()
                        .url(digitsConfig.getCNSMWCancelURL())
                        .header("Content-type", "application/json")
                        .header("Accept", "application/json")
                        .header("Authorization", "Bearer " +
                                userProfile.getAccessToken().getTokenValue())
                        .post(body)
                        .build();

            } else {
                log("Cannot find DigitsConfig instance, return false!");
            }

            return this;
        }

        public IamMethodBuilder logout(UserProfile userProfile) {
            log("logout:");

            mRequest = new Request.Builder()
                        .url("https://account.t-mobile.com/oauth2/v1/signout?token="+
                             userProfile.getAccessToken().getTokenValue())
                        .header("Accept", "application/json")
                        .build();

            return this;
        }
    }

    /**
     * This function shall not be run at main thread.
     */
    private IamResponse sendHttpRequest(OkHttpClient client, Request request) {
        log("IamHttpRequest: url=" + request.url().toString());
        log("IamHttpRequest: method=" + request.method());
        log("IamHttpRequest: headers=" + request.headers().toString());

        IamResponse result = null;

        try {
            Response response = client.newCall(request).execute();
            result = new IamResponse(request, response);
        } catch (Exception e) {
            Log.e(TAG, "sendHttpRequest: Exception:", e);
            Response response = new Response.Builder()
                .code(408) //Request Timeout
                .request(request)
                .protocol(Protocol.HTTP_1_1)
                .message("OKHTTP exception occurs. Search log for detail information.")
                .build();
            result = new IamResponse(request, response);
        }

        return result;
    }

    private boolean checkNoHandlerThread() {
        if (mThreadHandler == null) {
            log("No Handler Thread, return directly");
            return true;
        } else {
            return false;
        }
    }

    private static void log(String s) {
        if (DEBUG) Log.d(TAG, s);
    }
}