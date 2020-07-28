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

package com.mediatek.engineermode.epdgconfig;

import android.app.AlertDialog;
import android.app.Dialog;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

public class epdgConfigOtherFragment extends Fragment {

    private final static int UI_DATA_INVALID = 0;
    private final static int DATA_SET_FAIL = 1;
    private static int mInvalidPara = -1;
    private static String mSetFailPara = "";
    private static String mErrMsg = "";
    private static boolean mChange = false;
    private String TAG = "epdgConfig/OtherFragment";
    private EditText mIkev2if;
    private EditText mPort;
    private EditText mPortNatt;
    private EditText mCpaNm;
    private EditText mCpaDns4;
    private EditText mCpaDns6;
    private Spinner mNoic;
    private Spinner mNoeap;
    private Spinner mFragment;
    private Spinner mMobike;
    private Spinner mDnsType;
    private EditText mMss4Off;
    private EditText mMss6Off;
    private EditText mSelectInfo;
    private EditText mEpdgIdent;
    private EditText mRetryVector;
    private EditText mDnsMaxCount;
    private EditText mMaxRedirects;
    private EditText mRedirectLoopDetect;
    private EditText mIke_dscp;
    private EditText mFdpd_retransTo;
    private EditText mFdpd_retransTries;
    private EditText mFdpd_retransBase;

    private TextView mSelectInfo_text;
    private TextView mEpdgIdent_text;
    private TextView mRetryVector_text;

    private TextView mDnsMaxCount_text;
    private Spinner mLeftauth;
    private Spinner mEap;
    private Spinner mFastReauth;
    private Spinner mForceTsi64;
    private Spinner mForceTsiFull;
    private Spinner mUseCfgVip;
    private Spinner mReauthAddr;
    private Spinner mWdrvKeepAlive;
    private Spinner mKeyDisplay;
    private Spinner mKeyDump;
    private Spinner mDebugDpd;
    private Spinner mLogLevel;
    private TextView mPrePostPing_text;
    private TextView mDnsCache_text;
    private TextView mPcscfRestore_text;
    private TextView mEmergencyInd_text;
    private TextView mLivenessCheck_text;
    private TextView mDeviceIdentity_text;

    private TextView mKeyDisplay_text;
    private TextView mKeyDump_text;
    private TextView mDebugDpd_text;

    private Spinner mPrePostPing;
    private Spinner mDnsCache;
    private Spinner mPcscfRestore;
    private Spinner mEmergencyInd;
    private Spinner mLivenessCheck;
    private Spinner mDeviceIdentity;
    private Spinner mRedirect;

    private Spinner mAbort_mode;
    private Spinner mHoIpDisc;
    private Spinner mVisitedEpdg;
    private Spinner mTryEpdgPolicy;
    private Spinner mLeaveStandbyDpd;
    private Button mSet;
    private String mAtCmd = "wodemset=";
    private String[] mEditCfg = null;
    private String[] mEditCfg93before = {"ikev2if", "port", "port_natt",
            "cpa_nm", "cpa_dns4", "cpa_dns6", "mss4_off", "mss6_off"};
    private String[] mEditCfg93later = {"ikev2if", "port", "port_natt",
            "cpa_nm", "cpa_dns4", "cpa_dns6", "mss4_off", "mss6_off",
            "select_info", "epdg_identifier", "retry_vector", "dns_max_count", "max_redirects",
            "redirect_loop_detect", "ike_dscp", "fdpd_retrans_to", "fdpd_retrans_tries",
            "fdpd_retrans_base"};


    private String[] mSpinnerCfg = null;
    private String[] mSpinnerCfg93before = {"no_ic", "no_eap", "fragment", "mobike",
            "dns_type", "leftauth", "eap", "fastreauth", "force_tsi_64",
            "force_tsi_full", "use_cfg_vip", "reauth_addr",
            "wdrv_keep_alive", "key.display", "key.dump", "debug.dpd",
            "log_level"
    };
    private String[] mSpinnerCfg93later = {"no_ic", "no_eap", "fragment", "mobike",
            "dns_type", "leftauth", "eap", "fastreauth", "force_tsi_64",
            "force_tsi_full", "use_cfg_vip", "reauth_addr",
            "wdrv_keep_alive", "key.display", "key.dump", "debug.dpd",
            "log_level", "pre_post_ping", "dns_cache", "pcscf_restore",
            "emergency_ind", "liveness_check", "device_identity", "redirect",
            "abort_mode", "ho_ip_disc", "visited_epdg", "try_epdg_policy", "leave_standby_dpd"
    };
    private Spinner[] mSpinner;
    private EditText[] mEditText;
    private String[] mEditStatus;
    private int[] mSpinnerStatus;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.epdg_other, container, false);
        Elog.d(TAG, "epdgConfigOtherFragment create");
        mIkev2if = (EditText) view.findViewById(R.id.ikev2if);
        mPort = (EditText) view.findViewById(R.id.port);
        mPortNatt = (EditText) view.findViewById(R.id.port_natt);
        mCpaNm = (EditText) view.findViewById(R.id.cpa_nm);
        mCpaDns4 = (EditText) view.findViewById(R.id.cpa_dns4);
        mCpaDns6 = (EditText) view.findViewById(R.id.cpa_dns6);

        mMaxRedirects = (EditText) view.findViewById(R.id.max_redirects);
        mRedirectLoopDetect = (EditText) view.findViewById(R.id.redirect_loop_detect);
        mFdpd_retransTo = (EditText) view.findViewById(R.id.fdpd_retrans_to);
        mFdpd_retransTries = (EditText) view.findViewById(R.id.fdpd_retrans_tries);
        mFdpd_retransBase = (EditText) view.findViewById(R.id.fdpd_retrans_base);

        mNoic = (Spinner) view.findViewById(R.id.no_ic);
        mNoeap = (Spinner) view.findViewById(R.id.no_eap);
        mFragment = (Spinner) view.findViewById(R.id.fragment);
        mMobike = (Spinner) view.findViewById(R.id.mobike);
        mDnsType = (Spinner) view.findViewById(R.id.dns_type);


        mRedirect = (Spinner) view.findViewById(R.id.redirect);

        mAbort_mode = (Spinner) view.findViewById(R.id.abort_mode);
        mHoIpDisc = (Spinner) view.findViewById(R.id.ho_ip_disc);
        mVisitedEpdg = (Spinner) view.findViewById(R.id.visited_epdg);
        mTryEpdgPolicy = (Spinner) view.findViewById(R.id.try_epdg_policy);
        mLeaveStandbyDpd = (Spinner) view.findViewById(R.id.leave_standby_dpd);


        mMss4Off = (EditText) view.findViewById(R.id.mss4_off);
        mMss6Off = (EditText) view.findViewById(R.id.mss6_off);

        mSelectInfo = (EditText) view.findViewById(R.id.select_info);
        mEpdgIdent = (EditText) view.findViewById(R.id.epdg_ident);
        mRetryVector = (EditText) view.findViewById(R.id.retry_vector);
        mDnsMaxCount = (EditText) view.findViewById(R.id.dns_max_count);
        mDnsMaxCount_text = (TextView) view.findViewById(R.id.dns_max_count_text);
        mIke_dscp = (EditText) view.findViewById(R.id.ike_dscp);
//        mDnsMaxCount_text.setVisibility(View.GONE);
//        mDnsMaxCount.setVisibility(View.GONE);

        mSelectInfo_text = (TextView) view.findViewById(R.id.select_info_text);
        mEpdgIdent_text = (TextView) view.findViewById(R.id.epdg_ident_text);
        mRetryVector_text = (TextView) view.findViewById(R.id.retry_vector_text);

        mKeyDisplay_text = (TextView) view.findViewById(R.id.key_display_view);
        mKeyDump_text = (TextView) view.findViewById(R.id.key_dump_view);
        mDebugDpd_text = (TextView) view.findViewById(R.id.debug_dpd_view);

        mLeftauth = (Spinner) view.findViewById(R.id.leftauth);
        mEap = (Spinner) view.findViewById(R.id.eap);
        mFastReauth = (Spinner) view.findViewById(R.id.fastreauth);
        mForceTsi64 = (Spinner) view.findViewById(R.id.force_tsi_64);
        mForceTsiFull = (Spinner) view.findViewById(R.id.force_tsi_full);
        mUseCfgVip = (Spinner) view.findViewById(R.id.use_cfg_vip);
        mReauthAddr = (Spinner) view.findViewById(R.id.reauth_addr);
        mWdrvKeepAlive = (Spinner) view.findViewById(R.id.wdrv_keep_alive);
        mKeyDisplay = (Spinner) view.findViewById(R.id.key_display);
        mKeyDump = (Spinner) view.findViewById(R.id.key_dump);
        mDebugDpd = (Spinner) view.findViewById(R.id.debug_dpd);
        mLogLevel = (Spinner) view.findViewById(R.id.log_level);

        mPrePostPing = (Spinner) view.findViewById(R.id.pre_post_ping);
        mDnsCache = (Spinner) view.findViewById(R.id.dns_cache);
        mPcscfRestore = (Spinner) view.findViewById(R.id.pcscf_restore);
        mEmergencyInd = (Spinner) view.findViewById(R.id.emergency_ind);
        mLivenessCheck = (Spinner) view.findViewById(R.id.liveness_check);
        mDeviceIdentity = (Spinner) view.findViewById(R.id.device_identity);

        mPrePostPing_text = (TextView) view.findViewById(R.id.pre_post_ping_text);
        mDnsCache_text = (TextView) view.findViewById(R.id.dns_cache_text);
        mPcscfRestore_text = (TextView) view.findViewById(R.id.pcscf_restore_text);
        mEmergencyInd_text = (TextView) view.findViewById(R.id.emergency_ind_text);
        mLivenessCheck_text = (TextView) view.findViewById(R.id.liveness_check_text);
        mDeviceIdentity_text = (TextView) view.findViewById(R.id.device_identity_text);

        mSet = (Button) view.findViewById(R.id.other_set);

        if (FeatureSupport.is93Modem()) {
            mSpinnerCfg = mSpinnerCfg93later;
            mEditCfg = mEditCfg93later;
            mPrePostPing.setVisibility(View.VISIBLE);
            mDnsCache.setVisibility(View.VISIBLE);
            mPcscfRestore.setVisibility(View.VISIBLE);
            mEmergencyInd.setVisibility(View.GONE);
            mLivenessCheck.setVisibility(View.VISIBLE);
            mDeviceIdentity.setVisibility(View.VISIBLE);

            mPrePostPing_text.setVisibility(View.VISIBLE);
            mDnsCache_text.setVisibility(View.VISIBLE);
            mPcscfRestore_text.setVisibility(View.VISIBLE);
            mEmergencyInd_text.setVisibility(View.GONE);
            mLivenessCheck_text.setVisibility(View.VISIBLE);
            mDeviceIdentity_text.setVisibility(View.VISIBLE);

            mSelectInfo.setVisibility(View.VISIBLE);
            mEpdgIdent.setVisibility(View.VISIBLE);
            mRetryVector.setVisibility(View.VISIBLE);

            mSelectInfo_text.setVisibility(View.VISIBLE);
            mEpdgIdent_text.setVisibility(View.VISIBLE);
            mRetryVector_text.setVisibility(View.VISIBLE);

            mKeyDisplay_text.setVisibility(View.GONE);
            mKeyDump_text.setVisibility(View.GONE);
            mDebugDpd_text.setVisibility(View.GONE);
            mKeyDisplay.setVisibility(View.GONE);
            mKeyDump.setVisibility(View.GONE);
            mDebugDpd.setVisibility(View.GONE);
        } else {
            mSpinnerCfg = mSpinnerCfg93before;
            mEditCfg = mEditCfg93before;
            mPrePostPing.setVisibility(View.GONE);
            mDnsCache.setVisibility(View.GONE);
            mPcscfRestore.setVisibility(View.GONE);
            mEmergencyInd.setVisibility(View.GONE);
            mLivenessCheck.setVisibility(View.GONE);
            mDeviceIdentity.setVisibility(View.GONE);

            mPrePostPing_text.setVisibility(View.GONE);
            mDnsCache_text.setVisibility(View.GONE);
            mPcscfRestore_text.setVisibility(View.GONE);
            mEmergencyInd_text.setVisibility(View.GONE);
            mLivenessCheck_text.setVisibility(View.GONE);
            mDeviceIdentity_text.setVisibility(View.GONE);

            mSelectInfo.setVisibility(View.GONE);
            mEpdgIdent.setVisibility(View.GONE);
            mRetryVector.setVisibility(View.GONE);

            mSelectInfo_text.setVisibility(View.GONE);
            mEpdgIdent_text.setVisibility(View.GONE);
            mRetryVector_text.setVisibility(View.GONE);
        }

        mSpinner = new Spinner[]{mNoic, mNoeap, mFragment, mMobike, mDnsType,
                mLeftauth, mEap, mFastReauth, mForceTsi64, mForceTsiFull,
                mUseCfgVip, mReauthAddr, mWdrvKeepAlive, mKeyDisplay,
                mKeyDump, mDebugDpd, mLogLevel, mPrePostPing, mDnsCache,
                mPcscfRestore, mEmergencyInd, mLivenessCheck, mDeviceIdentity, mRedirect,
                mAbort_mode, mHoIpDisc, mVisitedEpdg, mTryEpdgPolicy, mLeaveStandbyDpd};

        mEditText = new EditText[]{mIkev2if, mPort, mPortNatt, mCpaNm,
                mCpaDns4, mCpaDns6, mMss4Off, mMss6Off, mSelectInfo, mEpdgIdent, mRetryVector,
                mDnsMaxCount, mMaxRedirects, mRedirectLoopDetect, mIke_dscp, mFdpd_retransTo,
                mFdpd_retransTries, mFdpd_retransBase};

        mEditStatus = new String[mEditCfg.length];
        mSpinnerStatus = new int[mSpinnerCfg.length];

        mSet.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mInvalidPara = checkDataValid();
                Elog.d(TAG, "mInvalidPara = " + mInvalidPara);
                if (mInvalidPara >= 0) {
                    showDialog(UI_DATA_INVALID).show();
                    return;
                }
                mSetFailPara = "";
                for (int i = 0; i < mSpinnerCfg.length; i++) {
                    int post = mSpinner[i].getSelectedItemPosition();
                    if (post == mSpinnerStatus[i]) {
                        continue;
                    } else {
                        mChange = true;
                        if (epdgConfig.setCfgValue(mSpinnerCfg[i], post + "")) {
                            Elog.d(TAG, mSpinnerCfg[i] + " set succeed");
                            mSpinnerStatus[i] = post;
                        } else {
                            mSetFailPara += mSpinnerCfg[i];
                            Elog.d(TAG, mSpinnerCfg[i] + " set failed");
                        }
                    }
                }
                for (int i = 0; i < mEditCfg.length; i++) {
                    String set_value = mEditText[i].getText().toString();
                    if ("".equals(set_value) && !(set_value.equals(mEditStatus[i]))) {
                        if (i == 0 || i == 8 || i == 9 || i == 10) {
                            if (FeatureSupport.is93Modem()) {
                                set_value = "";
                            } else {
                                set_value = "%EMPTY%";
                            }
                            mChange = true;
                        } else {
                            mEditText[i].setText(mEditStatus[i]);
                            continue;
                        }
                    } else {
                        if (set_value.equals(mEditStatus[i])) {
                            continue;
                        } else {
                            mChange = true;
                        }
                    }

                    if (mChange == true) {
                        if (epdgConfig.setCfgValue(mEditCfg[i], set_value)) {
                            Elog.d(TAG, mEditCfg[i] + " set succeed");
                            mEditStatus[i] = set_value;
                        } else {
                            mSetFailPara += mEditCfg[i];
                            Elog.d(TAG, mEditCfg[i] + " set failed");
                        }
                    }
                }
                if (mChange && mSetFailPara.equals("")) {
                    if (FeatureSupport.is93Modem()) {
                        epdgConfig.setCfgValue("", "");
                    }
                    Toast.makeText(getActivity(),
                            "Set successfully!",
                            Toast.LENGTH_SHORT).show();
                } else if (!mChange) {
                    Toast.makeText(getActivity(),
                            "No item changes!",
                            Toast.LENGTH_SHORT).show();
                } else {
                    mSetFailPara = mSetFailPara.substring(0, mSetFailPara.length() - 1);
                    showDialog(DATA_SET_FAIL).show();
                }
                mChange = false;
            }
        });

        return view;
    }

    @Override
    public void setUserVisibleHint(boolean isVisibleToUser) {
        super.setUserVisibleHint(isVisibleToUser);
        if (isVisibleToUser) {
            Elog.d(TAG, "epdgConfigOtherFragment show");
            getCfgValue();
        }
    }

    private void getCfgValue() {
        for (int i = 0; i < mEditCfg.length; i++) {
            String respValue = epdgConfig.getCfgValue(mEditCfg[i]);
            if (respValue != null) {
                Elog.d(TAG, "respValue = " + respValue);
                if (respValue.contains("\"")) {
                    String[] values = respValue.split("\"");
                    if (values.length > 1) {
                        mEditText[i].setText(values[1]);
                        mEditStatus[i] = values[1];
                    } else {
                        mEditText[i].setText(respValue);
                        mEditStatus[i] = respValue;
                    }
                } else {
                    mEditText[i].setText(respValue);
                    mEditStatus[i] = respValue;
                }
            } else {
                mEditText[i].setEnabled(false);
                continue;
            }
        }

        for (int i = 0; i < mSpinnerCfg.length; i++) {
            String respValue = epdgConfig.getCfgValue(mSpinnerCfg[i]);
            if (respValue != null) {
                int select = 0;
                try {
                    select = Integer.valueOf(respValue);
                } catch (NumberFormatException e) {
                    select = 0;
                }
                mSpinner[i].setSelection(select, true);
                mSpinnerStatus[i] = select;
            } else {
                mSpinner[i].setEnabled(false);
                continue;
            }
        }
    }

    private int checkDataValid() {
        int ret = -1;
        for (int i = 0; i < mEditText.length; i++) {
            if ((mEditText[i].getText().toString() != null)
                    && ("".equals(mEditText[i].getText().toString()))) {
                if (i != 0 && i != 8 && i != 9 && i != 10)
                    mEditText[i].setText(mEditStatus[i]);
                continue;
            }
            if (i == 0) {
                if ((mEditText[i].getText().toString().length() > 15)
                        || (mEditText[i].getText().toString().contains("\""))) {
                    ret = i;
                    mErrMsg = "The input should be no longer than 15 and not contain \".";
                    break;
                }
            } else if (i == 8 || i == 9 || i == 10) {
                if ((mEditText[i].getText().toString().length() > 255)
                        || (mEditText[i].getText().toString().contains("\""))) {
                    ret = i;
                    mErrMsg = "The input should be no longer than 255 and not contain \".";
                    break;
                }
            } else {
                long data = 0;
                try {
                    data = Integer.valueOf(mEditText[i].getText().toString());
                } catch (NumberFormatException e) {
                    data = -1;
                }

                Elog.d(TAG, "data = " + data);

                if (i == 16 || i == 17) {
                    if ((data < 0) || (data > 254)) {
                        ret = i;
                        mErrMsg = "The range is 0 ~ 254.";
                        break;
                    } else {
                        mEditText[i].setText(String.valueOf(data));
                    }
                } else if (i == 15) {
                    if ((data < 0) || (data > 4294967294L)) {
                        ret = i;
                        mErrMsg = "The range is 0 ~ 4294967294.";
                        break;
                    } else {
                        mEditText[i].setText(String.valueOf(data));
                    }
                } else if (i == 14) {
                    if ((data < 0) || (data > 63)) {
                        ret = i;
                        mErrMsg = "The range is 0 ~ 63.";
                        break;
                    } else {
                        mEditText[i].setText(String.valueOf(data));
                    }
                }
                else {
                    if ((data < 0) || (data > 65534)) {
                        ret = i;
                        mErrMsg = "The range is 0 ~ 65534.";
                        break;
                    } else {
                        mEditText[i].setText(String.valueOf(data));
                    }
                }

            }
        }
        return ret;
    }

    protected Dialog showDialog(int id) {
        switch (id) {
            case UI_DATA_INVALID:
                return new AlertDialog.Builder(getActivity()).setTitle(
                        "Check").setMessage(
                        "The input of " + mEditCfg[mInvalidPara] + " is invalid! " + mErrMsg)
                        .setPositiveButton("OK", null).create();
            case DATA_SET_FAIL:
                return new AlertDialog.Builder(getActivity()).setTitle(
                        "Set fail").setMessage("Set " + mSetFailPara + " fail!")
                        .setPositiveButton("OK", null).create();

        }
        return null;
    }

}
