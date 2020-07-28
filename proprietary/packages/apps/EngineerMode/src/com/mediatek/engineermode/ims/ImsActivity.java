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

package com.mediatek.engineermode.ims;

import android.app.Activity;
import android.content.ContentUris;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.database.Cursor;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.provider.Telephony;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.RadioButton;
import android.widget.TextView;
import android.widget.Toast;

import com.android.ims.ImsManager;
import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

/**
 * Ims Configuration.
 */
public class ImsActivity extends Activity implements View.OnClickListener, OnItemClickListener {
    private static final String IMS_VOLTE_SETTING_SHAREPRE = "telephony_ims_volte_settings";
    private static final String IMS_OVER_SGS_PREFERE_92MODEM = "ims_over_sgs_prefere_92modem";
    private static final String TAG = "Ims/ImsActivity";

    private static final String PROP_SS_MODE = "persist.vendor.radio.ss.mode";
    private static final String PROP_SS_DISABLE_METHOD = "persist.vendor.radio.ss.xrdm";
    private static final String PROP_SS_CFNUM = "persist.vendor.radio.xcap.cfn";
    private static final String PROP_DYNAMIC_SBP = "persist.vendor.radio.mtk_dsbp_support";
    private static final String PROP_IMS_MODE = "persist.vendor.radio.imstestmode";
    private static final String PROP_SMS_OVER_IMS_TEST_MODE = "persist.vendor.radio.smsformat";
    private static final String TYPE_IP = "IP";
    private static final String TYPE_IPV6 = "IPV6";
    private static final String TYPE_IPV4V6 = "IPV4V6";
    private static final String MODE_SS_XCAP = "Prefer XCAP";
    private static final String MODE_SS_CS = "Prefer CS";
    private static final String SMS_FORMAT_NONE = "none";
    private static final String SMS_FORMAT_3GPP = "3gpp";
    private static final String SMS_FORMAT_3GPP2 = "3gpp2";
    private static final int DISABLE_MODE_DELETE_RULE = 1;
    private static final int DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG = 2;
    private static final int IMS_MODE_DISABLED = 0;
    private static final int IMS_MODE_ENABLED = 1;
    private static final String SET_OPERATOR_CODE = "operator_code";
    private static final String SET_IMS_SIGNAL = "ims_signaling_qci";
    private static final String SET_PRECONDITION = "UA_call_precondition";

    private static final String OPERATOR_CODE_VALUE = "16386";
    private static final String IMS_SIGNAL_VALUE = "5";
    private static final String PRECONDITION_VALUE = "0";

    private static final String OPERATOR_CODE_DEFAULT = "0";
    private static final String IMS_SIGNAL_DEFAULT = "5";
    private static final String PRECONDITION_DEFAULT = "1";

    private static final int MSG_SET_OPERATOR_CODE = 0;
    private static final int MSG_SET_IMS_SIGNAL = 1;
    private static final int MSG_SET_PRECONDITION = 2;
    private static final int MSG_SET_VOLTE_SETTING = 3;

    private static final int MSG_SET_IMS_OVER_SGS_MODEM_90 = 0x10;
    private static final int MSG_SET_IMS_OVER_SGS_MODEM_91 = 0x11;
    private static final int MSG_SET_IMS_OVER_SGS_MODEM_92 = 0x12;
    private static final int MSG_SET_IMS_OVER_SGS_MODEM_93 = 0x13;

    private static final int MSG_QUERY_IMS_OVER_SGS_MODEM_90 = 0x20;
    private static final int MSG_QUERY_IMS_OVER_SGS_MODEM_91 = 0x21;
    private static final int MSG_QUERY_IMS_OVER_SGS_MODEM_92 = 0x22;
    private static final int MSG_QUERY_IMS_OVER_SGS_MODEM_93 = 0x23;

    private static final int MSG_SET_VDP = 4;
    private static final int MSG_GET_VDP = 5;
    private static final int MSG_SET_IMS_MODE = 6;

    private static final int MSG_IMS_OVER_SGS_ENABLE = 0x30;
    private static final int MSG_IMS_OVER_SGS_DISABLE = 0x31;
    private static final String[] mImsOverSGSPrefereEnabled = {
            "AT+ESBP=1,88,1",
            "AT+ESBP=5,\"SBP_SDM_PREFER_SMS_OVER_SGS_TO_IMS\",1",
            "AT+EGCMD=6,1,\"SDM_ADS_PREFER_SMS_OVER_SGS_TO_IMS\"",
            "AT+ECFGSET=\"sdm_profile_prefer_sms_over_sgs_to_ims\",\"1\""
    };
    private static final String[] mImsOverSGSPrefereDisabled = {
            "AT+ESBP=1,88,0",
            "AT+ESBP=5,\"SBP_SDM_PREFER_SMS_OVER_SGS_TO_IMS\",0",
            "AT+EGCMD=6,2,\"SDM_ADS_PREFER_SMS_OVER_SGS_TO_IMS\"",
            "AT+ECFGSET=\"sdm_profile_prefer_sms_over_sgs_to_ims\",\"0\""
    };
    private static final String[] mImsOverSGSPrefereQuerry = {
            "AT+ESBP=3,88",
            "AT+ESBP=7,\"SBP_SDM_PREFER_SMS_OVER_SGS_TO_IMS\"",
            "",
            "AT+ECFGGET=\"sdm_profile_prefer_sms_over_sgs_to_ims\""
    };
    private static final String[] mImsOverSGSPrefereQuerryRes = {
            "+ESBP:",
            "+ESBP:",
            "",
            "+ECFGGET:"
    };
    private static int mVolteSettingFlag = 0;
    private static int mImsOverSGSPrefereModemType = 0; //0:90  1:91  2:92  3:93 modem
    private RadioButton mRadioIp;
    private RadioButton mRadioIpv6;
    private RadioButton mRadioIpv4v6;
    private RadioButton mRadioSSXcap;
    private RadioButton mRadioSSCs;
    private RadioButton mRadioSSDisableTag;
    private RadioButton mRadioSSDisableDel;
    private RadioButton mRadioSetVolteOff;
    private RadioButton mRadioSetVolteOn;
    private RadioButton mRadioDynamicSbpEnabled;
    private RadioButton mRadioDynamicSbpDisabled;
    private RadioButton mRadioImsModeEnabled;
    private RadioButton mRadioImsModeDisabled;
    private RadioButton mRadioImsOverSGSPrefereEnabled;
    private RadioButton mRadioImsOverSGSPrefereDisabled;

    private RadioButton mRadioImsFormatNone;
    private RadioButton mRadioImsFormat3gpp;
    private RadioButton mRadioImsFormat3gpp2;
    private TextView mImsStatus;
    private TextView mImsMultStatus;
    private EditText mApn;
    private EditText mXcapCFNum;
    private EditText mVdp;
    private Button mButtonSetApn;
    private Button mButtonSetType;
    private Button mButtonSetSSMode;
    private Button mButtonSetSSDisable;
    private Button mButtonSetXcapCFNum;
    private Button mButtonSetVolte;
    private Button mButtonSetDynamicSbp;
    private Button mButtonSetImsMode;

    private Button mButtonSetImsOverSGSPrefere;
    private Button mButtonSetImsFormat;

    private Button mButtonSetVdp;

    private ListView mCategoryList;
    private Toast mToast = null;

    private android.net.Uri mUri = null;
    private final Handler mSettingHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_SET_VOLTE_SETTING) {
                set4gLte();
                setImsApn();
                if (mVolteSettingFlag == 1) {
                    showToast("Set CMW500 setting successful.");
                } else {
                    showToast("Set Default setting successful.");
                }
            } else if (msg.what == MSG_IMS_OVER_SGS_ENABLE) {
                mRadioImsOverSGSPrefereEnabled.setChecked(true);
            } else if (msg.what == MSG_IMS_OVER_SGS_DISABLE) {
                mRadioImsOverSGSPrefereDisabled.setChecked(true);
            }
        }
    };
    private android.net.Uri mPdnUri = null;
    private String mPdnName;
    private String mPdnType;
    private String mPdnApn;
    private String mPdnProtocol;
    private int mSimType;
    private final Handler mATHandler = new Handler() {
        private String[] mReturnData = new String[2];

        @Override
        public void handleMessage(Message msg) {
            if (msg.what == MSG_SET_OPERATOR_CODE) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.d(TAG, "Set operator_code successful.");
                    sendCommand(SET_IMS_SIGNAL,
                            ((mVolteSettingFlag == 1) ? IMS_SIGNAL_VALUE : IMS_SIGNAL_DEFAULT),
                            MSG_SET_IMS_SIGNAL);
                } else {
                    showToast("Set operator_code failed.");
                }
            } else if (msg.what == MSG_SET_IMS_SIGNAL) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.d(TAG, "Set ims_signaling_qci successful.");
                    sendCommand(SET_PRECONDITION,
                            ((mVolteSettingFlag == 1) ? PRECONDITION_VALUE : PRECONDITION_DEFAULT),
                            MSG_SET_PRECONDITION);
                } else {
                    showToast("Set ims_signaling_qci failed.");
                }
            } else if (msg.what == MSG_SET_PRECONDITION) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    Elog.d(TAG, "Set UA_call_precondition successful.");
                    Message msgSetting = new Message();
                    msgSetting.what = MSG_SET_VOLTE_SETTING;
                    mSettingHandler.sendMessage(msgSetting);
                } else {
                    showToast("Set UA_call_precondition failed.");
                }
            } else if (msg.what == MSG_SET_IMS_OVER_SGS_MODEM_90 ||
                    msg.what == MSG_SET_IMS_OVER_SGS_MODEM_91 ||
                    msg.what == MSG_SET_IMS_OVER_SGS_MODEM_92 ||
                    msg.what == MSG_SET_IMS_OVER_SGS_MODEM_93
                    ) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    showToast("Set succeed.");
                    Elog.d(TAG, "Set ims over SGS prefer successful.");
                } else {
                    showToast("Set failed.");
                    Elog.d(TAG, "Set ims over SGS prefer failed.");
                }
            } else if (msg.what == MSG_SET_VDP) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    showToast("Set VDP successfully.");
                } else {
                    showToast("Set VDP failed.");
                }
            } else if (msg.what == MSG_GET_VDP) {
                AsyncResult asyncResult = (AsyncResult) msg.obj;
                if (asyncResult != null && asyncResult.exception == null
                        && asyncResult.result != null) {
                    final String[] result = (String[]) asyncResult.result;
                    mVdp.setText(result[0].substring("+CEVDP:".length()).trim());
                }
            } else if (msg.what == MSG_SET_IMS_MODE) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    byte[] rawData = (byte[]) ar.result;
                    String error_num = "";
                    String txt = new String(rawData);
                    Elog.d(TAG, "Result(byte): " + txt);

                    if (txt.trim().equals("OK")) {
                        error_num = "0";
                    } else {
                        try {
                            error_num = txt.split(": ")[1].trim();
                            Elog.d(TAG, "error_num: " + error_num);
                        } catch (Exception e) {
                            e.printStackTrace();
                        }
                    }

                    if (error_num.equals("100") || error_num.equals("0")) {
                        try {
                            EmUtils.getEmHidlService()
                                .setImsTestMode(mRadioImsModeEnabled.isChecked() ? "1" : "0");
                        } catch (Exception e) {
                            e.printStackTrace();
                            Elog.e(TAG, "set property failed ...");
                        }
                        showToast("Set test mode done");
                        Elog.i(TAG, "Set test mode done");
                    } else {
                        showToast("Set ims test mode failed.");
                        Elog.i(TAG, "Set test mode failed");
                    }
                } else {
                    showToast("Set ims test mode failed.");
                    Elog.i(TAG, "Set test mode failed");
                }
            } else if (msg.what == MSG_QUERY_IMS_OVER_SGS_MODEM_90 ||
                    msg.what == MSG_QUERY_IMS_OVER_SGS_MODEM_91 ||
                    msg.what == MSG_QUERY_IMS_OVER_SGS_MODEM_93) {
                AsyncResult ar = (AsyncResult) msg.obj;
                if (ar.exception == null) {
                    mReturnData = (String[]) ar.result;
                    if (mReturnData.length > 0) {
                        Elog.d(TAG, "mReturnData = " + mReturnData[0]);
                        String result = "";
                        try {
                            if (FeatureSupport.is93Modem()) {
                                result = mReturnData[0].split(",")[1];
                                if (result.equals("\"0\"")) {
                                    result = "0";
                                } else if (result.equals("\"1\"")) {
                                    result = "1";
                                }
                            } else {
                                result = mReturnData[0].split(": ")[1];
                            }
                            Elog.d(TAG, "result = " + result);
                        } catch (Exception e) {
                            Elog.e(TAG, "mReturnData error ");
                        }

                        Message msgSetting = new Message();
                        if (result.equals("1")) {
                            msgSetting.what = MSG_IMS_OVER_SGS_ENABLE;
                        } else if (result.equals("0")) {
                            msgSetting.what = MSG_IMS_OVER_SGS_DISABLE;
                        }
                        mSettingHandler.sendMessage(msgSetting);
                    }

                } else {
                    Elog.d(TAG, "quary QUERY_IMS_OVER_SGS failed.");
                    Elog.d(TAG, ar.exception.getMessage());
                }
            }
        }
    };
    private int mSubId;

    public static void
    write92modemImsOverSGSPrefereSharedPreference(Context context, boolean check) {
        final SharedPreferences ImsOverSGSPrefereSh = context.getSharedPreferences(
                IMS_OVER_SGS_PREFERE_92MODEM, android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = ImsOverSGSPrefereSh.edit();
        editor.putBoolean(context.getString(R.string.ims_over_SGS_prefer), check);
        editor.commit();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Elog.d(TAG, "onCreate()");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ims);

        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        mImsStatus = (TextView) findViewById(R.id.ims_status);
        mImsMultStatus = (TextView) findViewById(R.id.ims_mult_status);
        mApn = (EditText) findViewById(R.id.ims_pdn_apn);
        mXcapCFNum = (EditText) findViewById(R.id.ims_ss_cf_num);
        mButtonSetApn = (Button) findViewById(R.id.ims_pdn_set_apn);
        mButtonSetApn.setOnClickListener(this);
        mRadioIp = (RadioButton) findViewById(R.id.ims_pdn_type_ip);
        mRadioIpv6 = (RadioButton) findViewById(R.id.ims_pdn_type_ipv6);
        mRadioIpv4v6 = (RadioButton) findViewById(R.id.ims_pdn_type_ipv4v6);
        mButtonSetType = (Button) findViewById(R.id.ims_pdn_set_type);
        mButtonSetType.setOnClickListener(this);
        mRadioSSXcap = (RadioButton) findViewById(R.id.ims_ss_mode_xcap);
        mRadioSSCs = (RadioButton) findViewById(R.id.ims_ss_mode_cs);
        mButtonSetSSMode = (Button) findViewById(R.id.ims_ss_set_mode);
        mButtonSetSSMode.setOnClickListener(this);
        mRadioSSDisableTag = (RadioButton) findViewById(R.id.ims_ss_disable_tag);
        mRadioSSDisableDel = (RadioButton) findViewById(R.id.ims_ss_disable_del);
        mButtonSetSSDisable = (Button) findViewById(R.id.ims_ss_set_disable);
        mButtonSetSSDisable.setOnClickListener(this);
        mButtonSetXcapCFNum = (Button) findViewById(R.id.ims_set_ss_cf_num);
        mButtonSetXcapCFNum.setOnClickListener(this);
        mRadioSetVolteOff = (RadioButton) findViewById(R.id.volte_set_off);
        mRadioSetVolteOn = (RadioButton) findViewById(R.id.volte_set_on);
        mButtonSetVolte = (Button) findViewById(R.id.volte_set);
        mButtonSetVolte.setOnClickListener(this);
        mButtonSetDynamicSbp = (Button) findViewById(R.id.ims_dynamic_sbp_set);
        mButtonSetDynamicSbp.setOnClickListener(this);
        mRadioDynamicSbpEnabled = (RadioButton) findViewById(R.id.ims_dynamic_sbp_enable);
        mRadioDynamicSbpDisabled = (RadioButton) findViewById(R.id.ims_dynamic_sbp_disable);
        mButtonSetImsMode = (Button) findViewById(R.id.ims_test_mode_set);
        mButtonSetImsMode.setOnClickListener(this);
        mButtonSetImsFormat = (Button) findViewById(R.id.ims_test_format);
        mButtonSetImsFormat.setOnClickListener(this);
        mRadioImsModeEnabled = (RadioButton) findViewById(R.id.ims_test_mode_enable);
        mRadioImsModeDisabled = (RadioButton) findViewById(R.id.ims_test_mode_disable);
        mRadioImsFormatNone = (RadioButton) findViewById(R.id.ims_format_none);
        mRadioImsFormat3gpp = (RadioButton) findViewById(R.id.ims_format_3gpp);
        mRadioImsFormat3gpp2 = (RadioButton) findViewById(R.id.ims_format_3gpp2);

        mButtonSetImsOverSGSPrefere = (Button) findViewById(R.id.ims_over_SGS_prefer_set);
        mButtonSetImsOverSGSPrefere.setOnClickListener(this);
        mRadioImsOverSGSPrefereEnabled
                = (RadioButton) findViewById(R.id.ims_over_SGS_prefer_enable);
        mRadioImsOverSGSPrefereDisabled
                = (RadioButton) findViewById(R.id.ims_over_SGS_prefer_disable);


        mVdp = (EditText) findViewById(R.id.ims_vdp);
        mButtonSetVdp = (Button) findViewById(R.id.ims_vdp_set);
        mButtonSetVdp.setOnClickListener(this);

        final SharedPreferences volteSettingSh = getSharedPreferences(IMS_VOLTE_SETTING_SHAREPRE,
                android.content.Context.MODE_PRIVATE);
        boolean volteSetChecked = volteSettingSh.getBoolean(getString(
                R.string.volte_set_check), false);
        if (volteSetChecked) {
            mRadioSetVolteOn.setChecked(true);
        } else {
            mRadioSetVolteOff.setChecked(true);
        }
        mCategoryList = (ListView) findViewById(R.id.ims_category_list);
        ArrayList<String> items = new ArrayList<String>();
        items.add(getString(R.string.ims_category_common));
        items.add(getString(R.string.ims_category_registration));
        items.add(getString(R.string.ims_category_call));
        items.add(getString(R.string.ims_category_sms));
        items.add(getString(R.string.ims_category_bearer));
        items.add(getString(R.string.ims_category_pcscf));
        items.add(getString(R.string.ims_category_ussd));
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                this, android.R.layout.simple_list_item_1, items);
        mCategoryList.setAdapter(adapter);
        mCategoryList.setOnItemClickListener(this);
        setListViewItemsHeight(mCategoryList);
    }

    private void setListViewItemsHeight(ListView listview) {
        if (listview == null) {
            return;
        }
        ListAdapter adapter = listview.getAdapter();
        int totalHeight = 0;
        for (int i = 0; i < adapter.getCount(); i++) {
            View itemView = adapter.getView(i, null, listview);
            itemView.measure(0, 0);
            totalHeight += itemView.getMeasuredHeight();
        }
        totalHeight += (adapter.getCount() - 1) * listview.getDividerHeight();
        ViewGroup.LayoutParams params = listview.getLayoutParams();
        params.height = totalHeight;
        listview.setLayoutParams(params);
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        Intent intent = new Intent(this, ConfigIMSActivity.class);
        intent.putExtra("mSimType", mSimType);
        switch (arg2) {
            case 0:
                intent.putExtra("category", getString(R.string.ims_category_common));
                break;
            case 1:
                intent.putExtra("category", getString(R.string.ims_category_registration));
                break;
            case 2:
                intent.putExtra("category", getString(R.string.ims_category_call));
                break;
            case 3:
                intent.putExtra("category", getString(R.string.ims_category_sms));
                break;
            case 4:
                intent.putExtra("category", getString(R.string.ims_category_bearer));
                break;
            case 5:
                intent.putExtra("category", getString(R.string.ims_category_pcscf));
                break;
            case 6:
                intent.putExtra("category", getString(R.string.ims_category_ussd));
                break;
            default:
                break;
        }
        startActivity(intent);
    }

    @Override
    protected void onResume() {
        Elog.d(TAG, "onResume()");
        super.onResume();
        Elog.d(TAG, "mSimType " + mSimType);
        int[] SubId = SubscriptionManager.getSubId(mSimType);
        if (SubId != null && SubId.length > 0) {
            mSubId = SubId[0];
        }
        Elog.d(TAG, "mSubId " + mSubId);


        boolean status = TelephonyManager.getDefault().isImsRegistered(mSubId);
        Elog.d(TAG, "getImsRegInfo(): " + status);
        mImsStatus.setText(getString(R.string.ims_status) + (status ? "true" : "false"));
        sendATCommand("AT+CEVDP?", "+CEVDP:", MSG_GET_VDP);

        mImsMultStatus.setText(SimSelectActivity.MULT_IMS_SUPPORT + ": "
                + SystemProperties.get(SimSelectActivity.MULT_IMS_SUPPORT, "-"));

        String ssmode = SystemProperties.get(PROP_SS_MODE, MODE_SS_XCAP);
        String ssdisableMethod = SystemProperties.get(PROP_SS_DISABLE_METHOD,
                Integer.toString(DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG));
        String xcapCFNum = SystemProperties.get(PROP_SS_CFNUM, "");
        String dynamicSbp = SystemProperties.get(PROP_DYNAMIC_SBP, "0");
        String imsMode = SystemProperties.get(PROP_IMS_MODE, "0");
        String smsformat = SystemProperties.get(PROP_SMS_OVER_IMS_TEST_MODE, "none");

        Elog.d(TAG, PROP_SS_MODE + ": " + ssmode);
        Elog.d(TAG, PROP_SS_DISABLE_METHOD + ": " + ssdisableMethod);
        Elog.d(TAG, PROP_SS_CFNUM + ":" + xcapCFNum);
        Elog.d(TAG, PROP_DYNAMIC_SBP + ":" + dynamicSbp);
        Elog.d(TAG, PROP_IMS_MODE + ":" + imsMode);
        Elog.d(TAG, PROP_SMS_OVER_IMS_TEST_MODE + ":" + smsformat);

        queryPdnInfo();
        queryImsOverSGSPrefereStatus();
        mApn.setText(mPdnApn);

        if (TYPE_IP.equals(mPdnProtocol)) {
            mRadioIp.setChecked(true);
        } else if (TYPE_IPV6.equals(mPdnProtocol)) {
            mRadioIpv6.setChecked(true);
        } else if (TYPE_IPV4V6.equals(mPdnProtocol)) {
            mRadioIpv4v6.setChecked(true);
        } else {
            showToast("Got invalid IP type: \"" + mPdnProtocol + "\"");
        }

        if (MODE_SS_XCAP.equals(ssmode)) {
            mRadioSSXcap.setChecked(true);
        } else if (MODE_SS_CS.equals(ssmode)) {
            mRadioSSCs.setChecked(true);
        } else {
            showToast("Got invalid SS Mode: \"" + ssmode + "\"");
        }

        if (DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG == Integer.parseInt(ssdisableMethod)) {
            mRadioSSDisableTag.setChecked(true);
        } else if (DISABLE_MODE_DELETE_RULE == Integer.parseInt(ssdisableMethod)) {
            mRadioSSDisableDel.setChecked(true);
        } else {
            showToast("Got invalid SS Disable Method: \"" + ssdisableMethod + "\"");
        }

        if (IMS_MODE_ENABLED == Integer.parseInt(dynamicSbp)) {
            mRadioDynamicSbpEnabled.setChecked(true);
        } else if (IMS_MODE_DISABLED == Integer.parseInt(dynamicSbp)) {
            mRadioDynamicSbpDisabled.setChecked(true);
        }
        if (IMS_MODE_ENABLED == Integer.parseInt(imsMode)) {
            mRadioImsModeEnabled.setChecked(true);
        } else if (IMS_MODE_DISABLED == Integer.parseInt(imsMode)) {
            mRadioImsModeDisabled.setChecked(true);
        }
        if (SMS_FORMAT_NONE.equals(smsformat)) {
            mRadioImsFormatNone.setChecked(true);
        } else if (SMS_FORMAT_3GPP.equals(smsformat)) {
            mRadioImsFormat3gpp.setChecked(true);
        } else if (SMS_FORMAT_3GPP2.equals(smsformat)) {
            mRadioImsFormat3gpp2.setChecked(true);
        }
        mXcapCFNum.setText(xcapCFNum);
    }

    @Override
    public void onDestroy() {
        writeVolteSettingSharedPreference(mRadioSetVolteOn.isChecked());
        super.onDestroy();
    }

    @Override
    public void onClick(View v) {
        if (v == mButtonSetApn) {
            setPdnAPN(mApn.getText().toString());
            Elog.d(TAG, "Set  PDN Protocol to" + mApn.getText().toString());
        } else if (v == mButtonSetType) {
            if (mRadioIp.isChecked()) {
                mPdnProtocol = "IP";
            } else if (mRadioIpv6.isChecked()) {
                mPdnProtocol = "IPV6";
            } else if (mRadioIpv4v6.isChecked()) {
                mPdnProtocol = "IPV4V6";
            } else {
                return;
            }
            if (mPdnProtocol != null && mPdnProtocol.length() > 0) {
                setPdnProtocol(mPdnProtocol);
            }
        } else if (v == mButtonSetSSMode) {
            String ssmode = "";
            if (mRadioSSXcap.isChecked()) {
                ssmode = MODE_SS_XCAP;
            } else if (mRadioSSCs.isChecked()) {
                ssmode = MODE_SS_CS;
            } else {
                return;
            }
            SystemProperties.set(PROP_SS_MODE, ssmode);
            Elog.d(TAG, "Set " + PROP_SS_MODE + " = " + ssmode);
            showToast("Set SS Mode done");
        } else if (v == mButtonSetSSDisable) {
            String ssdisableMethod = "";
            if (mRadioSSDisableTag.isChecked()) {
                ssdisableMethod = Integer.toString(DISABLE_MODE_ADD_RULE_DEACTIVATED_TAG);
            } else if (mRadioSSDisableDel.isChecked()) {
                ssdisableMethod = Integer.toString(DISABLE_MODE_DELETE_RULE);
            } else {
                return;
            }
            SystemProperties.set(PROP_SS_DISABLE_METHOD, ssdisableMethod);
            Elog.d(TAG, "Set " + PROP_SS_DISABLE_METHOD + " = " + ssdisableMethod);
            showToast("Set SS Disable done");
        } else if (v == mButtonSetXcapCFNum) {
            SystemProperties.set(PROP_SS_CFNUM, mXcapCFNum.getText().toString());
            Elog.d(TAG, "Set " + PROP_SS_CFNUM + " = " + mXcapCFNum.getText().toString());
            showToast("Set SS CF Number done");
        } else if (v == mButtonSetVolte) {
            if (mRadioSetVolteOff.isChecked()) {
                mVolteSettingFlag = 0;
            } else if (mRadioSetVolteOn.isChecked()) {
                mVolteSettingFlag = 1;
            } else {
                return;
            }
            Elog.d(TAG, "Set VOLTE");
            sendCommand(SET_OPERATOR_CODE,
                    ((mVolteSettingFlag == 1) ? OPERATOR_CODE_VALUE : OPERATOR_CODE_DEFAULT),
                    MSG_SET_OPERATOR_CODE);
        } else if (v == mButtonSetDynamicSbp) {
            Elog.d(TAG, "Set " + PROP_DYNAMIC_SBP + " = " +
                    (mRadioDynamicSbpEnabled.isChecked() ? "1" : "0"));
            try {
                EmUtils.getEmHidlService()
                      .setDsbpSupport(mRadioDynamicSbpEnabled.isChecked() ? "1" : "0");
            } catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
            showToast("Set Dynamic SBP done");
        } else if (v == mButtonSetImsMode) {

            if (FeatureSupport.is93Modem()) {
                if (mRadioImsModeEnabled.isChecked()) {
                    sendAtCommandRaw("AT+EIMSTESTMODE=1", MSG_SET_IMS_MODE);
                } else {
                    sendAtCommandRaw("AT+EIMSTESTMODE=0", MSG_SET_IMS_MODE);
                }
            } else {
                try {
                    EmUtils.getEmHidlService()
                        .setImsTestMode(mRadioImsModeEnabled.isChecked() ? "1" : "0");
                } catch (Exception e) {
                    e.printStackTrace();
                    Elog.e(TAG, "set property failed ...");
                }
                showToast("Set test mode done");
            }
        } else if (v == mButtonSetImsFormat) {
            String smsformat = "";
            if (mRadioImsFormatNone.isChecked()) {
                smsformat = "none";
            } else if (mRadioImsFormat3gpp.isChecked()) {
                smsformat = "3gpp";
            } else if (mRadioImsFormat3gpp2.isChecked()) {
                smsformat = "3gpp2";
            } else {
                return;
            }
            try {
                EmUtils.getEmHidlService().setSmsFormat(smsformat);
            } catch (Exception e) {
                e.printStackTrace();
                Elog.e(TAG, "set property failed ...");
            }
            showToast("Set ims format test mode done");
        } else if (v == mButtonSetImsOverSGSPrefere) {
            setImsOverSGSPrefereMode();
        } else if (v == mButtonSetVdp) {
            int select = 0;
            try {
                select = Integer.valueOf(mVdp.getText().toString());
            } catch (NumberFormatException e) {
                select = 0;
            }
            if (select > 4 || select < 1) {
                showToast("The input of VDP is wrong, please check!");
            } else {
                sendATCommand("AT+CEVDP=" + select, "", MSG_SET_VDP);
            }
        }
    }

    private void showToast(String msg) {
        mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    private void sendCommand(String name, String value, int msgtype) {
        Message msg = mATHandler.obtainMessage(msgtype);

        EmUtils.invokeOemRilRequestStringsEm(mSimType, new String[]{"AT+ECFGSET=\"" + name + "\"," +
                "\"" + value + "\"", ""}, msg);
    }

    private void sendATCommand(String cmd, String value, int msgtype) {
        Elog.d(TAG, "cmd = " + cmd);
        Message msg = mATHandler.obtainMessage(msgtype);
        EmUtils.invokeOemRilRequestStringsEm(mSimType, new String[]{cmd, value}, msg);
    }

    private void sendAtCommandRaw(String str, int message) {
        Elog.d(TAG, "sendAtCommand() " + str);

        byte[] rawData = str.getBytes();
        byte[] cmdByte = new byte[rawData.length + 1];
        System.arraycopy(rawData, 0, cmdByte, 0, rawData.length);
        cmdByte[cmdByte.length - 1] = 0;
        EmUtils.invokeOemRilRequestRawEm(mSimType,cmdByte, mATHandler.obtainMessage(message));
    }

    private void set4gLte() {
        Elog.d(TAG, "set4gLte mVolteSettingFlag = " + mVolteSettingFlag);
        if (mVolteSettingFlag == 1) {
            ImsManager.setEnhanced4gLteModeSetting(this, true);
        } else {
            ImsManager.setEnhanced4gLteModeSetting(this, false);
        }
    }

    private void setImsApn() {
        if (mVolteSettingFlag == 1) {
            insertImsApn();
        } else {
            resetImsApn();
        }
    }

    private void insertImsApn() {
        ContentValues values = new ContentValues();
        values.put(Telephony.Carriers.NAME, "ims");
        values.put(Telephony.Carriers.APN, "ims");
        values.put(Telephony.Carriers.TYPE, "ia,ims");
        values.put(Telephony.Carriers.PROTOCOL, "IPV6");
        values.put(Telephony.Carriers.ROAMING_PROTOCOL, "IPV6");

        TelephonyManager tm = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
        // SUBID: the subId for the sim inserted
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        Elog.d(TAG, "insertImsApn sub id " + subId);
        String numeric = "";
        String mcc = "";
        String mnc = "";
        numeric = tm.getSimOperator(subId);
        if (numeric != null && numeric.length() > 4) {
            mcc = numeric.substring(0, 3);
            mnc = numeric.substring(3);
        }
        values.put(Telephony.Carriers.MCC, mcc);
        values.put(Telephony.Carriers.MNC, mnc);
        values.put(Telephony.Carriers.NUMERIC, numeric);

        mUri = this.getContentResolver().insert(Telephony.Carriers.CONTENT_URI, values);
        Elog.d(TAG, "insertImsApn mUri " + mUri);
        writeImsApnSharedPreference(mUri.toString());
    }

    private void resetImsApn() {
        final SharedPreferences volteSettingSh = getSharedPreferences(IMS_VOLTE_SETTING_SHAREPRE,
                android.content.Context.MODE_PRIVATE);
        String uri = volteSettingSh.getString(getString(
                R.string.volte_set_ims_apn), null);
        if (uri == null) {
            return;
        }
        mUri = android.net.Uri.parse(uri);
        Elog.d(TAG, "resetImsApn mUri " + mUri);
        if (mUri != null) {
            this.getContentResolver().delete(mUri, null, null);
            mUri = null;
        }
    }

    private void writeVolteSettingSharedPreference(boolean check) {
        final SharedPreferences volteSettingSh = getSharedPreferences(
                IMS_VOLTE_SETTING_SHAREPRE, android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = volteSettingSh.edit();
        editor.putBoolean(getString(R.string.volte_set_check), check);
        editor.commit();
    }

    private void writeImsApnSharedPreference(String imsApnUri) {
        final SharedPreferences volteSettingSh = getSharedPreferences(
                IMS_VOLTE_SETTING_SHAREPRE, android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = volteSettingSh.edit();
        editor.putString(getString(R.string.volte_set_ims_apn), imsApnUri);
        editor.commit();
    }

    private void setImsOverSGSPrefereMode() {

        if (FeatureSupport.is90Modem()) {
            Elog.i(TAG, "it is 90 modem");
            mImsOverSGSPrefereModemType = 0;
        } else if (FeatureSupport.is91Modem()) {
            Elog.i(TAG, "it is 91 modem");
            mImsOverSGSPrefereModemType = 1;
        } else if (FeatureSupport.is92Modem()) {
            Elog.i(TAG, "it is 92 modem");
            mImsOverSGSPrefereModemType = 2;
        } else if (FeatureSupport.is93Modem()) {
            Elog.i(TAG, "it is 93 modem");
            mImsOverSGSPrefereModemType = 3;
        } else {
            Elog.i(TAG, "it is 90 before modem");
            mImsOverSGSPrefereModemType = 2;
        }

        if (mRadioImsOverSGSPrefereEnabled.isChecked()) {
            sendAtCommand(
                    new String[]{mImsOverSGSPrefereEnabled[mImsOverSGSPrefereModemType], ""},
                    mImsOverSGSPrefereModemType | 0x10);
            if (mImsOverSGSPrefereModemType == 2)
                write92modemImsOverSGSPrefereSharedPreference(ImsActivity.this, true);

        } else if (mRadioImsOverSGSPrefereDisabled.isChecked()) {
            sendAtCommand(
                    new String[]{mImsOverSGSPrefereDisabled[mImsOverSGSPrefereModemType], ""},
                    mImsOverSGSPrefereModemType | 0x10);
            if (mImsOverSGSPrefereModemType == 2)
                write92modemImsOverSGSPrefereSharedPreference(ImsActivity.this, false);
        } else
            showToast("please select the ebabled or disabled");
    }

    private void queryImsOverSGSPrefereStatus() {

        if (FeatureSupport.is90Modem()) {
            Elog.i(TAG, "it is 90 modem");
            mImsOverSGSPrefereModemType = 0;
        } else if (FeatureSupport.is91Modem()) {
            Elog.i(TAG, "it is 91 modem");
            mImsOverSGSPrefereModemType = 1;
        } else if (FeatureSupport.is92Modem()) {
            Elog.i(TAG, "it is 92 modem");
            mImsOverSGSPrefereModemType = 2;
        } else if (FeatureSupport.is93Modem()) {
            Elog.i(TAG, "it is 93 modem");
            mImsOverSGSPrefereModemType = 3;
        } else {
            Elog.i(TAG, "it is 90 before modem");
            mImsOverSGSPrefereModemType = 2;
        }

        if (mImsOverSGSPrefereModemType == 2) {
            if (read92modemImsOverSGSPrefereSharedPreference() == true) {
                mRadioImsOverSGSPrefereEnabled.setChecked(true);
            } else
                mRadioImsOverSGSPrefereDisabled.setChecked(true);
        } else
            sendAtCommand(new String[]{
                    mImsOverSGSPrefereQuerry[mImsOverSGSPrefereModemType],
                    mImsOverSGSPrefereQuerryRes[mImsOverSGSPrefereModemType]
            }, mImsOverSGSPrefereModemType | 0x20);
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand() " + command[0]);
        EmUtils.invokeOemRilRequestStringsEm(mSimType, command, mATHandler.obtainMessage(msg));
    }

    private boolean read92modemImsOverSGSPrefereSharedPreference() {
        final SharedPreferences ImsOverSGSPrefereSh =
                getSharedPreferences(IMS_OVER_SGS_PREFERE_92MODEM, android.content.Context
                        .MODE_PRIVATE);
        boolean ImsOverSGSPrefereShtChecked = ImsOverSGSPrefereSh.getBoolean(getString(
                R.string.ims_over_SGS_prefer), false);
        return ImsOverSGSPrefereShtChecked;
    }

    // get the IA APN for default data sub
    private void queryPdnInfo() {
        Cursor cursor = null;
        String Mccmnc = null;
        String where = null;
        String[] projection = new String[]{
                Telephony.Carriers._ID,
                Telephony.Carriers.NAME,
                Telephony.Carriers.APN,
                Telephony.Carriers.TYPE,
                Telephony.Carriers.PROTOCOL,
        };

        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        if (mSubId > SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
            Mccmnc = telephonyManager.getSimOperator(mSubId);
            Elog.d(TAG, "Mccmnc: " + Mccmnc);
            where = "type=? AND numeric=?";
            String[] args = {"emergency", Mccmnc};
            cursor = this.getContentResolver()
                    .query(Telephony.Carriers.CONTENT_URI, projection, where, args, null);
            if (cursor != null) {
                Elog.d(TAG, "Mccmnc cursor.getCount() " + cursor.getCount());
                cursor.moveToFirst();
                if (cursor.getCount() == 0) {
                    Elog.d(TAG, "queryPDNInfo where " + where + " is not found");
                    cursor.close();
                    cursor = null;
                }
            }
        } else {
            cursor = null;
        }
        if (cursor == null) {
            where = "type=\"emergency\"";
            cursor = this.getContentResolver().query(Telephony.Carriers.CONTENT_URI,
                    projection, where, null, null);
            Elog.d(TAG, "queryPDNInfo where " + where);
        }

        if (cursor != null) {
            cursor.moveToFirst();
            Elog.d(TAG, "default cursor.getCount() " + cursor.getCount());
            // get uri
            if (cursor.getCount() > 0) {
                int index = cursor.getInt(0);
                mPdnUri = ContentUris.withAppendedId(Telephony.Carriers.CONTENT_URI, index);
                // get the apn data if want to display on UI
                mPdnName = cursor.getString(1);
                mPdnApn = cursor.getString(2);
                mPdnType = cursor.getString(3);
                mPdnProtocol = cursor.getString(4);
                Elog.d(TAG, "queryIAInfo mPdnName: " + mPdnName + "\nmPdnApn: " + mPdnApn +
                        "\nmPdnType: " + mPdnType + "\nmPdnProtocol: " + mPdnProtocol);
            }
            cursor.close();
        }
    }

    // set protocol of the PDN protocol
    private void setPdnProtocol(String protocol) {
        if (protocol != null) {
            if (mPdnUri != null) {
                ContentValues values = new ContentValues();
                values.put(Telephony.Carriers.PROTOCOL, protocol);
                this.getContentResolver().update(mPdnUri, values, null, null);
                Toast.makeText(ImsActivity.this, "set PDN Protocol success",
                        Toast.LENGTH_SHORT).show();
                return;
            }
        }
        Toast.makeText(ImsActivity.this, "set PDN Protocol fail", Toast.LENGTH_SHORT).show();
    }

    // set protocol of the PDN APN
    private void setPdnAPN(String apn) {
        if (apn != null) {
            if (mPdnUri != null) {
                ContentValues values = new ContentValues();
                values.put(Telephony.Carriers.APN, apn);
                this.getContentResolver().update(mPdnUri, values, null, null);
                Toast.makeText(ImsActivity.this, "set PDN Protocol success",
                        Toast.LENGTH_SHORT).show();
                return;
            }
        }
        Toast.makeText(ImsActivity.this, "set PDN Protocol fail", Toast.LENGTH_SHORT).show();
    }
}
