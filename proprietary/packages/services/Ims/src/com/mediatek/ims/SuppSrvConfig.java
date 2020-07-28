/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.ims;

import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources.NotFoundException;

import android.telephony.SubscriptionManager;
import android.telephony.Rlog;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import android.os.SystemProperties;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;

public class SuppSrvConfig {
    static final String LOG_TAG = "SuppSrvConfig";

    private static SuppSrvConfig sInstance = null;
    private Context mContext = null;

    private static final int PROP_BOOL_IDX_NOT_SUPPORT_XCAP             = 0;
    private static final int PROP_BOOL_IDX_USE_HTTP_PROTOCOL            = 1;
    private static final int PROP_BOOL_IDX_HANDLE_409                   = 2;
    private static final int PROP_BOOL_IDX_FILL_FORWARD_TO              = 3;
    private static final int PROP_BOOL_IDX_XCAP_PREFIX_SS               = 4;
    private static final int PROP_BOOL_IDX_IMS_DEREG                    = 5;
    private static final int PROP_BOOL_IDX_APPEND_COUNTRY_CODE          = 6;
    private static final int PROP_BOOL_IDX_MEDIA_TAG                    = 7;
    private static final int PROP_BOOL_IDX_PUT_WHOLE_CLIR               = 8;
    private static final int PROP_BOOL_IDX_QUERY_WHOLE_SIMSERV          = 9;
    private static final int PROP_BOOL_IDX_DISABLE_ETAG                 = 10;
    private static final int PROP_BOOL_IDX_HTTP_ERR_TO_UNKNOWN_HOST     = 11;
    private static final int PROP_BOOL_IDX_XCAP_APN                     = 12;
    private static final int PROP_BOOL_IDX_INTERNET_APN                 = 13;
    private static final int PROP_BOOL_IDX_NOREPLYTIMER_INSIDE_CFACTION = 14;
    private static final int PROP_BOOL_IDX_SUPPORT_TIME_SLOT            = 15;
    private static final int PROP_BOOL_IDX_SAVE_WHOLE_NODE              = 16;
    private static final int PROP_BOOL_IDX_SET_CFNRC_WITH_CFNL          = 17;
    private static final int PROP_BOOL_IDX_NOT_SUPPORT_CFNL             = 18;
    private static final int PROP_BOOL_IDX_SUPPORT_PUT_CF_ROOT          = 19;
    private static final int PROP_BOOL_IDX_ATTR_NEED_QUAOTATION_MARK    = 20;
    private static final int PROP_BOOL_IDX_FWD_NUM_USE_SIP_URI          = 21;
    private static final int PROP_BOOL_IDX_SUPPORT_PLUS_TO_URL          = 22;
    private static final int PROP_BOOL_IDX_SUPPORT_NON_URI_NUMBER       = 23;

    private static final int PROP_STR_IDX_ELEMENT_CONTENT_TYPE = 0;
    private static final int PROP_STR_IDX_AUID                 = 1;
    private static final int PROP_STR_IDX_XCAP_ROOT            = 2;
    private static final int PROP_STR_IDX_RULEID_CFU           = 3;
    private static final int PROP_STR_IDX_RULEID_CFB           = 4;
    private static final int PROP_STR_IDX_RULEID_CFNRY         = 5;
    private static final int PROP_STR_IDX_RULEID_CFNRC         = 6;
    private static final int PROP_STR_IDX_RULEID_CFNL          = 7;
    private static final int PROP_STR_IDX_DIGEST_ID            = 8;
    private static final int PROP_STR_IDX_DIGEST_PWD           = 9;
    private static final int PROP_STR_IDX_PHONE_CONTEXT        = 10;

    private static final int PROP_INT_IDX_XCAP_PORT                 = 0;
    private static final int PROP_INT_IDX_MEDIA_TYPE                = 1;
    private static final int PROP_INT_IDX_DATA_KEEP_ALIVE_TIMER     = 2;
    private static final int PROP_INT_IDX_REQUEST_DATA_TIMER        = 3;
    private static final int PROP_INT_IDX_DATA_COOL_DOWN_TIMER      = 4;

    private static final int PROP_LONG_IDX_CACHE_VALID_TIME = 0;

    private static final String SYS_PROP_BOOL_VALUE         = "persist.vendor.ss.boolvalue";
    private static final String SYS_PROP_BOOL_CONFIG        = "persist.vendor.ss.boolconfig";

    private static final String SYS_PROP_CONTENT_TYPE       = "persist.vendor.ss.contenttype";
    private static final String SYS_PROP_AUID               = "persist.vendor.ss.auid";
    private static final String SYS_PROP_XCAP_ROOT          = "persist.vendor.ss.xcaproot";
    private static final String SYS_PROP_RULEID_CFU         = "persist.vendor.ss.ruleid.cfu";
    private static final String SYS_PROP_RULEID_CFB         = "persist.vendor.ss.ruleid.cfb";
    private static final String SYS_PROP_RULEID_CFNRY       = "persist.vendor.ss.ruleid.cfnry";
    private static final String SYS_PROP_RULEID_CFNRC       = "persist.vendor.ss.ruleid.cfnrc";
    private static final String SYS_PROP_RULEID_CFNL        = "persist.vendor.ss.ruleid.cfnl";
    private static final String SYS_PROP_DIGEST_ID          = "persist.vendor.ss.digest.id";
    private static final String SYS_PROP_DIGEST_PWD         = "persist.vendor.ss.digest.pwd";

    private static final String SYS_PROP_XCAP_PORT               = "persist.vendor.ss.xcapport";
    private static final String SYS_PROP_MEDIA_TYPE              = "persist.vendor.ss.mediatype";
    private static final String SYS_PROP_DATA_KEEP_ALIVE_TIMER   = "persist.vendor.ss.alivetimer";
    private static final String SYS_PROP_REQUEST_DATA_TIMER      = "persist.vendor.ss.reqtimer";
    private static final String SYS_PROP_DATA_COOL_DOWN_TIMER    = "persist.vendor.ss.cdtimer";
    private static final String SYS_PROP_CACHE_VALID_TIME        = "persist.vendor.ss.cachetime";

    public static final String RULEID_CFU = "CFU";
    public static final String RULEID_CFB = "CFB";
    public static final String RULEID_CFNRy = "CFNRy";
    public static final String RULEID_CFNRc = "CFNRc";
    public static final String RULEID_CFNL = "CFNL";

    private boolean mNotSupportXcap                   = false;
    private boolean mHttpProtocolScheme               = true;
    private boolean mHandleError409                   = false;
    private boolean mFillCompleteForwardTo            = false;
    private boolean mXcapNsPrefixSS                   = false;
    private boolean mNeedIMSDereg                     = false;
    private boolean mAppendCountryCode                = false;
    private boolean mSupportMediaTag                  = true;
    private boolean mPutWholeCLIR                     = false;
    private boolean mQueryWholeSimServ                = false;
    private boolean mDisableEtag                      = false;
    private boolean mHttpErrToUnknownHostErr          = false;
    private boolean mUseXCAPTypeApn                   = true;
    private boolean mUseInternetTypeApn               = false;
    private boolean mNoReplyTimeInsideCFAction        = false;
    private boolean mTimeSlot                         = false;
    private boolean mSaveWholeNode                    = false;
    private boolean mSetCFNRcWithCFNL                 = false;
    private boolean mNotSupportCFNotRegistered        = false;
    private boolean mSupportPutCfRoot                 = true;
    private boolean mAttrNeedQuotationMark            = false;
    private boolean mFwdNumUseSipUri                  = false;
    private boolean mSupportPutNonUriNumber           = false;

    private String  mElementContentType               = null;
    private String  mAUID                             = null;
    private String  mXcapRoot                         = null;
    private Map     mRuleId                           = null;
    private String  mDigestId                         = null;
    private String  mDigestPwd                        = null;
    private String  mPhoneContext                     = null;

    private int     mPort                   = 0;
    private int     mMediaTagType           = -1;
    private int     mDataKeepAliveTimer     = 0;
    private int     mRequestDataTimer       = 0;
    private int     mDataCoolDownTimer      = 0;

    private long    mCacheValidTime         = 0;

    /** [Standard]
     *      SERVICE_CLASS_VOICE  : (audio), (audio & video)
     *      SERVICE_CLASS_VIDEO  : (video), (audio & video)
     *      SERVICE_CLASS_NONE   : (audio), (video), (audio & video)
     *      illegal :
     */
    public final static int MEDIA_TYPE_STANDARD         = 0;
    /** [Only Audio]
     *      SERVICE_CLASS_VOICE  : (audio)
     *      SERVICE_CLASS_VIDEO  : (audio)
     *      SERVICE_CLASS_NONE   : (audio)
     *      illegal : (video), (audio & video)
     */
    public final static int MEDIA_TYPE_ONLY_AUDIO       = 1;
    /** [Seperate]
     *      SERVICE_CLASS_VOICE  : (audio)
     *      SERVICE_CLASS_VIDEO  : (video)
     *      SERVICE_CLASS_NONE   : (audio), (video)
     *      illegal : (audio & video)
     */
    public final static int MEDIA_TYPE_SEPERATE         = 2;
    /** [Video with Audio]
     *      SERVICE_CLASS_VOICE  : (audio)
     *      SERVICE_CLASS_VIDEO  : (audio & video)
     *      SERVICE_CLASS_NONE   : (audio), (audio & video)
     *      illegal : (video)
     */
    public final static int MEDIA_TYPE_VIDEO_WITH_AUDIO = 3;

    private String propBoolIdxToString(int idx) {
        switch (idx) {
            case PROP_BOOL_IDX_NOT_SUPPORT_XCAP:
                return "PROP_BOOL_IDX_NOT_SUPPORT_XCAP";
            case PROP_BOOL_IDX_USE_HTTP_PROTOCOL:
                return "PROP_BOOL_IDX_USE_HTTP_PROTOCOL";
            case PROP_BOOL_IDX_HANDLE_409:
                return "PROP_BOOL_IDX_HANDLE_409";
            case PROP_BOOL_IDX_FILL_FORWARD_TO:
                return "PROP_BOOL_IDX_FILL_FORWARD_TO";
            case PROP_BOOL_IDX_XCAP_PREFIX_SS:
                return "PROP_BOOL_IDX_XCAP_PREFIX_SS";
            case PROP_BOOL_IDX_IMS_DEREG:
                return "PROP_BOOL_IDX_IMS_DEREG";
            case PROP_BOOL_IDX_APPEND_COUNTRY_CODE:
                return "PROP_BOOL_IDX_APPEND_COUNTRY_CODE";
            case PROP_BOOL_IDX_MEDIA_TAG:
                return "PROP_BOOL_IDX_MEDIA_TAG";
            case PROP_BOOL_IDX_PUT_WHOLE_CLIR:
                return "PROP_BOOL_IDX_PUT_WHOLE_CLIR";
            case PROP_BOOL_IDX_QUERY_WHOLE_SIMSERV:
                return "PROP_BOOL_IDX_QUERY_WHOLE_SIMSERV";
            case PROP_BOOL_IDX_DISABLE_ETAG:
                return "PROP_BOOL_IDX_DISABLE_ETAG";
            case PROP_BOOL_IDX_HTTP_ERR_TO_UNKNOWN_HOST:
                return "PROP_BOOL_IDX_HTTP_ERR_TO_UNKNOWN_HOST";
            case PROP_BOOL_IDX_XCAP_APN:
                return "PROP_BOOL_IDX_XCAP_APN";
            case PROP_BOOL_IDX_INTERNET_APN:
                return "PROP_BOOL_IDX_INTERNET_APN";
            case PROP_BOOL_IDX_NOREPLYTIMER_INSIDE_CFACTION:
                return "PROP_BOOL_IDX_NOREPLYTIMER_INSIDE_CFACTION";
            case PROP_BOOL_IDX_SUPPORT_TIME_SLOT:
                return "PROP_BOOL_IDX_SUPPORT_TIME_SLOT";
            case PROP_BOOL_IDX_SAVE_WHOLE_NODE:
                return "PROP_BOOL_IDX_SAVE_WHOLE_NODE";
            case PROP_BOOL_IDX_SET_CFNRC_WITH_CFNL:
                return "PROP_BOOL_IDX_SET_CFNRC_WITH_CFNL";
            case PROP_BOOL_IDX_NOT_SUPPORT_CFNL:
                return "PROP_BOOL_IDX_NOT_SUPPORT_CFNL";
            case PROP_BOOL_IDX_ATTR_NEED_QUAOTATION_MARK:
                return "PROP_BOOL_IDX_ATTR_NEED_QUAOTATION_MARK";
            case PROP_BOOL_IDX_FWD_NUM_USE_SIP_URI:
                return "PROP_BOOL_IDX_FWD_NUM_USE_SIP_URI";
            case PROP_BOOL_IDX_SUPPORT_NON_URI_NUMBER:
                return "PROP_BOOL_IDX_SUPPORT_NON_URI_NUMBER";
            default:
                return "NOT_FOUND";
        }
    }

    private String propStrIdxToString(int idx) {
        switch (idx) {
            case PROP_STR_IDX_ELEMENT_CONTENT_TYPE:
                return "PROP_STR_IDX_ELEMENT_CONTENT_TYPE";
            case PROP_STR_IDX_AUID:
                return "PROP_STR_IDX_AUID";
            case PROP_STR_IDX_XCAP_ROOT:
                return "PROP_STR_IDX_XCAP_ROOT";
            case PROP_STR_IDX_RULEID_CFU:
                return "PROP_STR_IDX_RULEID_CFU";
            case PROP_STR_IDX_RULEID_CFB:
                return "PROP_STR_IDX_RULEID_CFB";
            case PROP_STR_IDX_RULEID_CFNRY:
                return "PROP_STR_IDX_RULEID_CFNRY";
            case PROP_STR_IDX_RULEID_CFNRC:
                return "PROP_STR_IDX_RULEID_CFNRC";
            case PROP_STR_IDX_RULEID_CFNL:
                return "PROP_STR_IDX_RULEID_CFNL";
            case PROP_STR_IDX_DIGEST_ID:
                return "PROP_STR_IDX_DIGEST_ID";
            case PROP_STR_IDX_DIGEST_PWD:
                return "PROP_STR_IDX_DIGEST_PWD";
            case PROP_STR_IDX_PHONE_CONTEXT:
                return "PROP_STR_IDX_PHONE_CONTEXT";

            default:
                return "NOT_FOUND";
        }
    }

    private String propIntIdxToString(int idx) {
        switch (idx) {
            case PROP_INT_IDX_XCAP_PORT:
                return "PROP_INT_IDX_XCAP_PORT";
            case PROP_INT_IDX_MEDIA_TYPE:
                return "PROP_INT_IDX_MEDIA_TYPE";
            case PROP_INT_IDX_DATA_KEEP_ALIVE_TIMER:
                return "PROP_INT_IDX_DATA_KEEP_ALIVE_TIMER";
            case PROP_INT_IDX_REQUEST_DATA_TIMER:
                return "PROP_INT_IDX_REQUEST_DATA_TIMER";
            case PROP_INT_IDX_DATA_COOL_DOWN_TIMER:
                return "PROP_INT_IDX_DATA_COOL_DOWN_TIMER";
            default:
                return "NOT_FOUND";
        }
    }

    private String propLongIdxToString(int idx) {
        switch (idx) {
            case PROP_LONG_IDX_CACHE_VALID_TIME:
                return "PROP_LONG_IDX_CACHE_VALID_TIME";
            default:
                return "NOT_FOUND";
        }
    }

    private SuppSrvConfig(Context context) {
        mContext = context;
    }

    public static SuppSrvConfig getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new SuppSrvConfig(context);
        }

        return sInstance;
    }

    public void update(int phoneId) {
        updateResourceBySim(getMccMnc(phoneId));
        initAllPara();
    }

    private void initAllPara() {
        mNotSupportXcap = initParaBool(PROP_BOOL_IDX_NOT_SUPPORT_XCAP,
                R.bool.conf_not_support_xcap);
        mHttpProtocolScheme = initParaBool(PROP_BOOL_IDX_USE_HTTP_PROTOCOL,
                R.bool.conf_xcap_http);
        mHandleError409 = initParaBool(PROP_BOOL_IDX_HANDLE_409,
                R.bool.conf_handle409);
        mFillCompleteForwardTo = initParaBool(PROP_BOOL_IDX_FILL_FORWARD_TO,
                R.bool.conf_fill_complete_forward_to);
        mXcapNsPrefixSS = initParaBool(PROP_BOOL_IDX_XCAP_PREFIX_SS,
                R.bool.conf_xcap_nsprefixss);
        mNeedIMSDereg = initParaBool(PROP_BOOL_IDX_IMS_DEREG,
                R.bool.conf_imsdereg);
        mAppendCountryCode = initParaBool(PROP_BOOL_IDX_APPEND_COUNTRY_CODE,
                R.bool.conf_append_countrycode);
        mSupportMediaTag = initParaBool(PROP_BOOL_IDX_MEDIA_TAG,
                R.bool.conf_mediatag);
        mPutWholeCLIR = initParaBool(PROP_BOOL_IDX_PUT_WHOLE_CLIR,
                R.bool.conf_putwholeclir);
        mQueryWholeSimServ = initParaBool(PROP_BOOL_IDX_QUERY_WHOLE_SIMSERV,
                R.bool.conf_querywholesimserv);
        mDisableEtag = initParaBool(PROP_BOOL_IDX_DISABLE_ETAG,
                R.bool.conf_disable_etag);
        mHttpErrToUnknownHostErr = initParaBool(PROP_BOOL_IDX_HTTP_ERR_TO_UNKNOWN_HOST,
                R.bool.conf_httperr_to_unknownhost);
        mUseXCAPTypeApn = initParaBool(PROP_BOOL_IDX_XCAP_APN,
                R.bool.conf_use_xcap_apn);
        mUseInternetTypeApn = initParaBool(PROP_BOOL_IDX_INTERNET_APN,
                R.bool.conf_use_internet_apn);
        mNoReplyTimeInsideCFAction = initParaBool(PROP_BOOL_IDX_NOREPLYTIMER_INSIDE_CFACTION,
                R.bool.conf_noreplytimer_inside_cf);
        mTimeSlot = initParaBool(PROP_BOOL_IDX_SUPPORT_TIME_SLOT,
                R.bool.conf_support_time_slot);
        mSaveWholeNode = initParaBool(PROP_BOOL_IDX_SAVE_WHOLE_NODE,
                R.bool.conf_save_whole_node);
        mSetCFNRcWithCFNL = initParaBool(PROP_BOOL_IDX_SET_CFNRC_WITH_CFNL,
                R.bool.conf_set_cfnrc_with_cfnl);
        mNotSupportCFNotRegistered = initParaBool(PROP_BOOL_IDX_NOT_SUPPORT_CFNL,
                R.bool.conf_not_support_cfnl);
        mSupportPutCfRoot = initParaBool(PROP_BOOL_IDX_SUPPORT_PUT_CF_ROOT,
                R.bool.conf_support_put_cf_root);
        mAttrNeedQuotationMark = initParaBool(PROP_BOOL_IDX_ATTR_NEED_QUAOTATION_MARK,
                R.bool.conf_attr_need_quaotation_mark);
        mFwdNumUseSipUri = initParaBool(PROP_BOOL_IDX_FWD_NUM_USE_SIP_URI,
                R.bool.conf_fwd_num_use_sip_uri);
        if (initParaBool(PROP_BOOL_IDX_SUPPORT_PLUS_TO_URL, R.bool.conf_support_plus_to_url)) {
            SystemProperties.set("persist.vendor.mtk.xcap.rawurl", "true");
        } else {
            SystemProperties.set("persist.vendor.mtk.xcap.rawurl", "false");
        }
        mSupportPutNonUriNumber = initParaBool(PROP_BOOL_IDX_SUPPORT_NON_URI_NUMBER,
                R.bool.conf_support_put_non_uri_number);
        mElementContentType = initParaString(PROP_STR_IDX_ELEMENT_CONTENT_TYPE,
                R.string.conf_eleupdatetype);
        mAUID = initParaString(PROP_STR_IDX_AUID,
                R.string.conf_auid);
        mXcapRoot = initParaString(PROP_STR_IDX_XCAP_ROOT,
                R.string.conf_xcap_root);
        initRuleId();
        mDigestId = initParaString(PROP_STR_IDX_DIGEST_ID,
                R.string.conf_digest_id);
        mDigestPwd = initParaString(PROP_STR_IDX_DIGEST_PWD,
                R.string.conf_digest_pwd);
        mPhoneContext = initParaString(PROP_STR_IDX_PHONE_CONTEXT,
                R.string.conf_phone_context);

        mPort = initParaInteger(PROP_INT_IDX_XCAP_PORT,
                R.integer.conf_xcap_port);
        mMediaTagType = initParaInteger(PROP_INT_IDX_MEDIA_TYPE,
                R.integer.conf_mediatag_type);
        mDataKeepAliveTimer = initParaInteger(PROP_INT_IDX_DATA_KEEP_ALIVE_TIMER,
                R.integer.conf_data_keep_alive_timer);
        mRequestDataTimer = initParaInteger(PROP_INT_IDX_REQUEST_DATA_TIMER,
                R.integer.conf_request_data_timer);
        mDataCoolDownTimer = initParaInteger(PROP_INT_IDX_DATA_COOL_DOWN_TIMER,
                R.integer.conf_data_cool_down_timer);

        mCacheValidTime = initParaLong(PROP_LONG_IDX_CACHE_VALID_TIME,
                R.string.conf_cache_value_time);
    }

    private boolean initParaBool(int idx, int res) {
        boolean r = false;
        int prop = getSysPropForBool(idx);
        if (prop != -1) {
            r = (prop == 1);
        } else {
            r = mContext.getResources().getBoolean(res);
        }
        Rlog.i(LOG_TAG, "initParaBool: " + propBoolIdxToString(idx) + " = " + r);
        return r;
    }

    private String initParaString(int idx, int res) {
        String r = getSysPropForString(idx);
        if (r == null || r.isEmpty()) {
            r = mContext.getResources().getString(res);
        }
        Rlog.i(LOG_TAG, "initParaString: " + propStrIdxToString(idx) + " = " + r);
        return r;
    }

    private int initParaInteger(int idx, int res) {
        int r = getSysPropForInteger(idx);
        if (r == -1) {
            r = mContext.getResources().getInteger(res);
        }
        Rlog.i(LOG_TAG, "initParaInteger: " + propIntIdxToString(idx) + " = " + r);
        return r;
    }

    private long initParaLong(int idx, int res) {
        long r = getSysPropForLong(idx);
        if (r == -1) {
            String longString = mContext.getResources().getString(res);
            r = Long.parseLong(longString);
        }
        Rlog.i(LOG_TAG, "initParaLong: " + propLongIdxToString(idx) + " = " + r);
        return r;
    }

    private void updateResourceBySim(String mncmcc) {
        Rlog.d(LOG_TAG, "updateResourceBySim: " + mncmcc);
        if (mncmcc != null && !mncmcc.isEmpty()) {
            Configuration config = mContext.getResources().getConfiguration();
            int mcc = Integer.valueOf(mncmcc.substring(0, 3));
            int mnc = Integer.valueOf(mncmcc.substring(3));
            config.mcc = mcc;
            config.mnc = (mnc == 0 ? Configuration.MNC_ZERO : mnc);
            Rlog.d(LOG_TAG, "Config mcc : " + config.mcc + ", mnc = " + config.mnc);
            mContext.getResources().updateConfiguration(config,
                    mContext.getResources().getDisplayMetrics());
        }
    }

    private int getSysPropForBool(int idx) {
        int r = -1;
        if (idx > -1) {  // No need to get from system property if -1
            int which = SystemProperties.getInt(SYS_PROP_BOOL_CONFIG, 0);
            if (which > 0 && ((which & (1 << idx)) != 0)) {
                int value = SystemProperties.getInt(SYS_PROP_BOOL_VALUE, 0);
                r = (value & (1 << idx)) != 0 ? 1 : 0;
                Rlog.i(LOG_TAG, "getSysPropForBool: " + propBoolIdxToString(idx)
                        + "=" + (r == 1 ? "TRUE" : "FALSE"));
            }
        }
        return r;
    }

    private String getSysPropForString(int idx) {
        String r = null;
        switch (idx) {
            case PROP_STR_IDX_ELEMENT_CONTENT_TYPE:
                r = SystemProperties.get(SYS_PROP_CONTENT_TYPE);
                break;
            case PROP_STR_IDX_AUID:
                r = SystemProperties.get(SYS_PROP_AUID);
                break;
            case PROP_STR_IDX_XCAP_ROOT:
                r = SystemProperties.get(SYS_PROP_XCAP_ROOT);
                break;
            case PROP_STR_IDX_RULEID_CFU:
                r = SystemProperties.get(SYS_PROP_RULEID_CFU);
                break;
            case PROP_STR_IDX_RULEID_CFB:
                r = SystemProperties.get(SYS_PROP_RULEID_CFB);
                break;
            case PROP_STR_IDX_RULEID_CFNRY:
                r = SystemProperties.get(SYS_PROP_RULEID_CFNRY);
                break;
            case PROP_STR_IDX_RULEID_CFNRC:
                r = SystemProperties.get(SYS_PROP_RULEID_CFNRC);
                break;
            case PROP_STR_IDX_RULEID_CFNL:
                r = SystemProperties.get(SYS_PROP_RULEID_CFNL);
                break;
            default:
                break;
        }
        Rlog.i(LOG_TAG, "getSysPropForString: " + propStrIdxToString(idx) + "=" + r);
        return r;
    }

    private int getSysPropForInteger(int idx) {
        int r = -1;
        switch (idx) {
            case PROP_INT_IDX_XCAP_PORT:
                r = SystemProperties.getInt(SYS_PROP_XCAP_PORT, -1);
                break;
            case PROP_INT_IDX_MEDIA_TYPE:
                r = SystemProperties.getInt(SYS_PROP_MEDIA_TYPE, -1);
                break;
            case PROP_INT_IDX_DATA_KEEP_ALIVE_TIMER:
                r = SystemProperties.getInt(SYS_PROP_DATA_KEEP_ALIVE_TIMER, -1);
                break;
            case PROP_INT_IDX_REQUEST_DATA_TIMER:
                r = SystemProperties.getInt(SYS_PROP_REQUEST_DATA_TIMER, -1);
                break;
            case PROP_INT_IDX_DATA_COOL_DOWN_TIMER:
                r = SystemProperties.getInt(SYS_PROP_DATA_COOL_DOWN_TIMER, -1);
                break;
            default:
                break;
        }
        Rlog.i(LOG_TAG, "getSysPropForInteger: " + propIntIdxToString(idx) + "=" + r);
        return r;
    }

    private long getSysPropForLong(int idx) {
        long r = -1;
        switch (idx) {
            case PROP_LONG_IDX_CACHE_VALID_TIME:
                r = SystemProperties.getLong(SYS_PROP_CACHE_VALID_TIME, -1);
                break;
            default:
                break;
        }
        Rlog.i(LOG_TAG, "getSysPropForInteger: " + propLongIdxToString(idx) + "=" + r);
        return r;
    }

    private String getMccMnc(int phoneId) {
        return TelephonyManager.getDefault().getSimOperatorNumericForPhone(phoneId);
    }

    public boolean isNotSupportXcap() {
        return mNotSupportXcap;
    }

    public boolean isUseHttpProtocolScheme() {
        return mHttpProtocolScheme;
    }

    public String getElementContentType() {
        return mElementContentType;
    }

    public boolean isHandleError409() {
        return mHandleError409;
    }

    public boolean isFillCompleteForwardTo() {
        return mFillCompleteForwardTo;
    }

    public boolean isXcapNsPrefixSS() {
        return mXcapNsPrefixSS;
    }

    public String getAUID() {
        return mAUID;
    }

    public String getXcapRoot() {
        return mXcapRoot;
    }

    public int getPort() {
        return mPort;
    }

    public boolean isNeedIMSDereg() {
        return mNeedIMSDereg;
    }

    public boolean isAppendCountryCode() {
        return mAppendCountryCode;
    }

    private void initRuleId() {
        String cfu = initParaString(PROP_STR_IDX_RULEID_CFU, R.string.conf_ruleid_cfu);
        String cfb = initParaString(PROP_STR_IDX_RULEID_CFB, R.string.conf_ruleid_cfb);
        String cfnry = initParaString(PROP_STR_IDX_RULEID_CFNRY, R.string.conf_ruleid_cfnry);
        String cfnrc = initParaString(PROP_STR_IDX_RULEID_CFNRC, R.string.conf_ruleid_cfnrc);
        String cfnl = initParaString(PROP_STR_IDX_RULEID_CFNL, R.string.conf_ruleid_cfnl);
        mRuleId = new HashMap<String, String>();
        mRuleId.put(RULEID_CFU, cfu);
        mRuleId.put(RULEID_CFB, cfb);
        mRuleId.put(RULEID_CFNRy, cfnry);
        mRuleId.put(RULEID_CFNRc, cfnrc);
        mRuleId.put(RULEID_CFNL, cfnl);
        return;
    }

    public Map getRuleId() {
        return mRuleId;
    }

    public String getDigestId() {
        return mDigestId;
    }

    public String getDigestPwd() {
        return mDigestPwd;
    }

    public String getPhoneContext() {
        return mPhoneContext;
    }

    public int getMediaTagType() {
        return mMediaTagType;
    }

    public boolean isSupportMediaTag() {
        return mSupportMediaTag;
    }

    public boolean isPutWholeCLIR() {
        return mPutWholeCLIR;
    }

    public boolean isQueryWholeSimServ() {
        return mQueryWholeSimServ;
    }

    public boolean isDisableEtag() {
        return mDisableEtag;
    }

    public boolean isSaveWholeNode() {
        return mSaveWholeNode;
    }

    public boolean isSetCFNRcWithCFNL() {
        return mSetCFNRcWithCFNL;
    }

    public int getDataKeepAliveTimer() {
        return mDataKeepAliveTimer;
    }

    public int getRequestDataTimer() {
        return mRequestDataTimer;
    }

    public int getDataCoolDownTimer() {
        return mDataCoolDownTimer;
    }

    public long getCacheValidTime() {
        return mCacheValidTime;
    }

    public boolean isHttpErrToUnknownHostErr() {
        return mHttpErrToUnknownHostErr;
    }

    public boolean isUseXCAPTypeApn() {
        return mUseXCAPTypeApn;
    }

    public boolean isUseInternetTypeApn() {
        return mUseInternetTypeApn;
    }

    public boolean isNoReplyTimeInsideCFAction() {
        return mNoReplyTimeInsideCFAction;
    }

    public boolean isSupportTimeSlot() {
        return mTimeSlot;
    }

    public boolean isNotSupportCFNotRegistered() {
        return mNotSupportCFNotRegistered;
    }

    public boolean isSupportPutCfRoot() {
        return mSupportPutCfRoot;
    }

    public boolean isAttrNeedQuotationMark() {
        return mAttrNeedQuotationMark;
    }

    public boolean isFwdNumUseSipUri() {
        return mFwdNumUseSipUri;
    }

    public boolean isSupportPutNonUriNumber() {
        return mSupportPutNonUriNumber;
    }
}
