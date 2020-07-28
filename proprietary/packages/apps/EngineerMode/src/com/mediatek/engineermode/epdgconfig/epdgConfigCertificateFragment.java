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
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

public class epdgConfigCertificateFragment extends Fragment {
    private final static String TAG = "epdgConfig/CertificateFragment";
    private final static int DATA_SET_FAIL = 1;
    private static String mSetFailPara = "";
    private static boolean mChange = false;

    private Spinner mCertUsed;
    private Spinner mUrlcert;
    private Spinner mOcsp;
    private Spinner mNocert;
    private Spinner mDebugSkipcert;
    private Spinner mNoIdCheck;
    private Button mSet;
    private String[] mCfg = {"cert_used", "urlcert", "ocsp",
            "nocert", "skip_check_cert", "noid"};
    private Spinner[] mSpinner;
    private int[] mSpinnerStatus = {0, 0, 0, 0, 0, 0};

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.epdg_certificate, container, false);
        Elog.d(TAG, "epdgConfigCertificateFragment create");
        mCertUsed = (Spinner) view.findViewById(R.id.cert_used);
        mUrlcert = (Spinner) view.findViewById(R.id.urlcert);
        mOcsp = (Spinner) view.findViewById(R.id.ocsp);
        mNocert = (Spinner) view.findViewById(R.id.nocert);
        mDebugSkipcert = (Spinner) view.findViewById(R.id.debug_skipcert);
        mNoIdCheck = (Spinner) view.findViewById(R.id.no_id_check);
        mSpinner = new Spinner[]{mCertUsed, mUrlcert, mOcsp,
                mNocert, mDebugSkipcert, mNoIdCheck};

        mSpinnerStatus = new int[mCfg.length];

        mSet = (Button) view.findViewById(R.id.certificate_set);
        mSet.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View v) {
                mSetFailPara = "";
                for (int i = 0; i < mCfg.length; i++) {
                    int post = mSpinner[i].getSelectedItemPosition();
                    if (mSpinner[i].getSelectedItemPosition() == mSpinnerStatus[i]) {
                        continue;
                    } else {
                        mChange = true;
                        if (epdgConfig.setCfgValue(mCfg[i], post + "")) {
                            Elog.d(TAG, mCfg[i] + " set succeed");
                            mSpinnerStatus[i] = post;
                        } else {
                            mSetFailPara += mCfg[i];
                            Elog.d(TAG, mCfg[i] + " set failed");
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
            Elog.d(TAG, "epdgConfigCertificateFragment show");
            getCfgValue();
        }
    }

    private void getCfgValue() {
        String atCmd = "wodemget=" + epdgConfig.mPhoneId + ",";
        String finalAtCmd;
        for (int i = 0; i < mCfg.length; i++) {
            String respValue = epdgConfig.getCfgValue(mCfg[i]);
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

    protected Dialog showDialog(int id) {
        switch (id) {
            case DATA_SET_FAIL:
                return new AlertDialog.Builder(getActivity()).setTitle(
                        "Set fail").setMessage("Set " + mSetFailPara + " fail!")
                        .setPositiveButton("OK", null).create();
        }
        return null;
    }

}
