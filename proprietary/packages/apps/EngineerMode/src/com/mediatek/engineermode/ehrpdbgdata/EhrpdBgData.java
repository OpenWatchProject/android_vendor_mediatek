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

package com.mediatek.engineermode.ehrpdbgdata;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.os.RemoteException;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import vendor.mediatek.hardware.netdagent.V1_0.INetdagent;

public class EhrpdBgData extends Activity implements OnCheckedChangeListener {
    private static final String TAG = "EhrpdBgData";
    private static final String BG_DATA_DISABLED_PROPERTY = "persist.vendor.radio.bgdata.disabled";
    private static boolean mEhrpdBgDataEnable = false;
    private static INetdagent agent = null;
    private CheckBox mCheckBox;

    public static void setDataDisable(boolean isEnable) {
        try {
            agent = INetdagent.getService();
            if (agent == null) {
                Elog.e(TAG, "agnet is null");
                return;
            }
            if (!isEnable) {
                Elog.d(TAG, "clearIotFirewall");
                agent.dispatchNetdagentCmd("netdagent firewall clear_nsiot_firewall");
            } else {
                Elog.d(TAG, "setIotFirewall");
                agent.dispatchNetdagentCmd("netdagent firewall set_nsiot_firewall");
            }
        } catch (RemoteException e) {
            Elog.d(TAG, "RomoteException");
        }
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.ehrpd_bg_data);
        mCheckBox = (CheckBox) findViewById(R.id.ehrpd_bg_data_botton);
        mCheckBox.setOnCheckedChangeListener(this);
        String value = EmUtils.systemPropertyGet(BG_DATA_DISABLED_PROPERTY, "0");
        mEhrpdBgDataEnable = value.equals("1") ? true : false;
        mCheckBox.setChecked(mEhrpdBgDataEnable);
        setDataDisable(mEhrpdBgDataEnable);
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.getId() == R.id.ehrpd_bg_data_botton) {
            final boolean ehrpdBgDataEnable = mCheckBox.isChecked();
            Elog.d(TAG, "ehrpdBgDataEnable is " + ehrpdBgDataEnable);
            if (isChecked) {
                showCheckInfoDlg(getString(R.string.ehrpd_bg_data),
                        getString(R.string.ehrpd_bg_data_disable_hint));
            }

            EmUtils.systemPropertySet(BG_DATA_DISABLED_PROPERTY, (ehrpdBgDataEnable == true) ? "1" :
                    "0");
            setDataDisable(ehrpdBgDataEnable);
        }
    }

    private void showCheckInfoDlg(String title, String message) {
        AlertDialog.Builder dialog = new AlertDialog.Builder(this);
        dialog.setTitle(title);
        dialog.setMessage(message);
        dialog.setCancelable(false);
        dialog.setPositiveButton("OK", new OnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which) {
                if (which == DialogInterface.BUTTON_POSITIVE) {
                    dialog.dismiss();
                }
            }
        });
        dialog.show();
    }

}
