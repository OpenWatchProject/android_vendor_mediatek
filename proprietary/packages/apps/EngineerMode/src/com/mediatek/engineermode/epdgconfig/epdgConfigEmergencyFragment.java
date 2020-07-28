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

public class epdgConfigEmergencyFragment extends Fragment {
    private final static String TAG = "epdgConfig/epdgConfigEmergencyFragment";
    private final static int DATA_SET_FAIL = 1;
    private final static int UI_DATA_INVALID = 0;
    private static int mInvalidPara = -1;
    private static String mSetFailPara = "";
    private static String mErrMsg = "";
    private static boolean mChange = false;

    private Spinner mEmergEpdg;
    private Spinner mEmergNai;
    private Spinner mEmergNoSim;
    private Spinner mEmergOnSameEpdg;
    private Spinner mEmergNumbers;

    private EditText mEpdgFqdnEm;
    private EditText mSelectInfoEm;
    private EditText mEpdgIdentifierEm;

    private TextView mEmergOnSameEpdgView;

    private Button mSet;
    private String[] mSpinnerCfg = {"emerg_epdg", "emerg_nai", "emerg_no_sim",
            "emerg_on_same_epdg", "emerg_numbers"};
    private String[] mEditCfg = {"epdg_fqdn_em", "select_info_em", "epdg_identifier_em"};
    private Spinner[] mSpinner;
    private EditText[] mEditText;
    private int[] mSpinnerStatus;
    private String[] mEditStatus;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.epdg_emergency, container, false);
        Elog.d(TAG, "epdgConfigCertificateFragment create");
        mEmergEpdg = (Spinner) view.findViewById(R.id.emerg_epdg);
        mEmergNai = (Spinner) view.findViewById(R.id.emerg_nai);
        mEmergNoSim = (Spinner) view.findViewById(R.id.emerg_no_sim);
        mEmergOnSameEpdg = (Spinner) view.findViewById(R.id.emerg_on_same_epdg);
        mEmergOnSameEpdgView = (TextView) view.findViewById(R.id.emerg_on_same_epdg_view);
        mEmergNumbers = (Spinner) view.findViewById(R.id.emerg_numbers);

        mEmergOnSameEpdg.setVisibility(View.GONE);
        mEmergOnSameEpdgView.setVisibility(View.GONE);

        mEpdgFqdnEm = (EditText) view.findViewById(R.id.epdg_fqdn_em);
        mSelectInfoEm = (EditText) view.findViewById(R.id.select_info_em);
        mEpdgIdentifierEm = (EditText) view.findViewById(R.id.epdg_identifier_em);

        mSpinner = new Spinner[]{mEmergEpdg, mEmergNai, mEmergNoSim,
                mEmergOnSameEpdg, mEmergNumbers};
        mEditText = new EditText[]{mEpdgFqdnEm, mSelectInfoEm, mEpdgIdentifierEm};

        mEditStatus = new String[mEditCfg.length];
        mSpinnerStatus = new int[mSpinnerCfg.length];
        mSet = (Button) view.findViewById(R.id.emergency_set);
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
                    if (mSpinner[i].getSelectedItemPosition() == mSpinnerStatus[i]) {
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
                        if (i == 0 || i == 1 || i == 2) {
                            set_value = "";
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
                            "Set successfully!", Toast.LENGTH_SHORT).show();
                } else if (!mChange) {
                    Toast.makeText(getActivity(),
                            "No item changes!", Toast.LENGTH_SHORT).show();
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
            Elog.d(TAG, "epdgConfigEmergencyFragment show");
            getCfgValue();
        }
    }

    private void getCfgValue() {
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
                mEditStatus[i]="";
                continue;
            }
        }
    }

    private int checkDataValid() {
        int ret = -1;
        for (int i = 0; i < mEditText.length; i++) {
            if (i == 0 || i == 1 || i == 2) {
                if ((mEditText[i].getText().toString().length() > 255)
                        || (mEditText[i].getText().toString().contains("\""))) {
                    ret = i;
                    mErrMsg = "The input should be no longer than 255 and not contain \".";
                    break;
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
