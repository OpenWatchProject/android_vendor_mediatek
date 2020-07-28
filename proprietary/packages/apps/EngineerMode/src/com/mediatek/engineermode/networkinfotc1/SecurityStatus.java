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

package com.mediatek.engineermode.networkinfotc1;

import android.app.Activity;
import android.os.Bundle;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;
import com.mediatek.mdml.Msg;

import java.util.ArrayList;
import java.util.List;


public class SecurityStatus extends Activity implements MDMCoreOperation.IMDMSeiviceInterface {
    private static final String TAG = "SecurityStatus";

    String[] m2gCipherInfo = new String[]{
            "No Ciphering", "A5/1", "A5/2", "A5/3", "A5/4", "A5/5", "A5/6", "A5/7"};
    String[] m2gGprsInfo = new String[]{"No Ciphering", "GEA1", "GEA2", "GEA3"};
    String[] m3gCipherInfo = new String[]{"No Ciphering", "UEA0", "UEA1", "", "UEA2"};
    String[] m3gIntegrityInfo = new String[]{"No Integrity", "UIA1", "UIA2"};
    String[] m4gEnasCipherInfo = new String[]{
            "EEA0(NULL)", "EEA1(SNOW3G)", "EEA2(AES)", "EEA3(ZUC)"};
    String[] m4gEnasIntegrityInfo = new String[]{
            "EIA0(NULL)", "EIA1(SNOW3G)", "EIA2(AES)", "EIA3(ZUC)"};
    String[] m4gErrcCipherInfo = new String[]{
            "EEA0(NULL)", "EEA1(SNOW3G)", "EEA2(AES)", "EEA3(ZUC)"};
    String[] m4gErrcIntegrityInfo = new String[]{
            "EIA0(NULL)", "EIA1(SNOW3G)", "EIA2(AES)", "EIA3(ZUC)"};
    private TextView m2gCipher;
    private TextView m2gGprs;
    private TextView m3gCipher;
    private TextView m3gIntegrity;
    private TextView m4gEnasCipher;
    private TextView m4gEnasIntegrity;
    private TextView m4gErrcCipher;
    private TextView m4gErrcIntegrity;

    private List<MdmBaseComponent> componentsArray = new ArrayList<MdmBaseComponent>();
    private int mSimTypeToShow = 0;
    private String SubscribeMsgIdName[] = {"MSG_ID_EM_RRM_CHANNEL_DESCR_INFO_IND",
            "MSG_ID_EM_LLC_STATUS_IND",
            "MSG_ID_EM_RRCE_3G_SECURITY_CONFIGURATION_STATUS_IND",
            "MSG_ID_EM_ERRC_SEC_PARAM_IND",
            "MSG_ID_EM_EMM_SEC_INFO_IND",
            "MSG_ID_EM_ERRC_STATE_IND"};

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.security_status);
        m2gCipher = (TextView) findViewById(R.id.security_status_2g_cipher);
        m2gGprs = (TextView) findViewById(R.id.security_status_2g_gprs);
        m3gCipher = (TextView) findViewById(R.id.security_status_3g_cipher);
        m3gIntegrity = (TextView) findViewById(R.id.security_status_3g_integrity);
        m4gEnasCipher = (TextView) findViewById(R.id.security_status_4g_enas_cipher);
        m4gEnasIntegrity = (TextView) findViewById(R.id.security_status_4g_enas_integrity);
        m4gErrcCipher = (TextView) findViewById(R.id.security_status_4g_errc_cipher);
        m4gErrcIntegrity = (TextView) findViewById(R.id.security_status_4g_errc_integrity);

        mSimTypeToShow = PhoneConstants.SIM_ID_1;

        MDMCoreOperation.getInstance().mdmInitialize(this);
        MDMCoreOperation.getInstance().setOnMDMChangedListener(this);
        MDMCoreOperation.getInstance().mdmParametersSeting(componentsArray, mSimTypeToShow);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onPause() {
        super.onPause();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        MDMCoreOperation.getInstance().mdmlUnSubscribe();
    }

    private void registerNetworkInfo() {
        MdmBaseComponent components = new MdmBaseComponent();
        components.setEmComponentName(SubscribeMsgIdName);
        componentsArray.add(components);

        MDMCoreOperation.getInstance().mdmlSubscribe();
    }


    int getFieldValue(Msg data, String key, boolean signed) {
        return MDMCoreOperation.getInstance().getFieldValue(data, key, signed);
    }

    int getFieldValue(Msg data, String key) {
        return MDMCoreOperation.getInstance().getFieldValue(data, key, false);
    }

    @Override
    public void onUpdateMDMData(String name, Msg data) {
        Elog.d(TAG, "update = " + name);
        if (name.equals("MSG_ID_EM_RRM_CHANNEL_DESCR_INFO_IND")) {
            int cipher_algo = getFieldValue(data, "rr_em_channel_descr_info.cipher_algo");
            m2gCipher.setText(m2gCipherInfo[cipher_algo]);
            Elog.d(TAG, "2G cipher_algo = " + cipher_algo + " = " + m2gCipherInfo[cipher_algo]);
        } else if (name.equals("MSG_ID_EM_LLC_STATUS_IND")) {
            int cipher_algo = getFieldValue(data, "cipher_algo");
            m2gGprs.setText(m2gGprsInfo[cipher_algo]);
            Elog.d(TAG, "2G cipher_algo = " + cipher_algo + " = " + m2gGprsInfo[cipher_algo]);
        } else if (name.equals("MSG_ID_EM_RRCE_3G_SECURITY_CONFIGURATION_STATUS_IND")) {
            int ciphering_alg = getFieldValue(data, "uCipheringAlgorithm");
            int integrity_alg = getFieldValue(data, "uIntegrityAlgorithm");
            m3gCipher.setText(m3gCipherInfo[ciphering_alg]);
            m3gIntegrity.setText(m3gIntegrityInfo[integrity_alg]);
            Elog.d(TAG, "3G ciphering_alg = " + ciphering_alg + " = " +
                    ((ciphering_alg == 0xff) ? "N/A" : m3gCipherInfo[ciphering_alg]));
            Elog.d(TAG, "3G integrity_alg = " + integrity_alg + " = " +
                    ((integrity_alg == 0xff) ? "N/A" : m3gIntegrityInfo[integrity_alg]));
        } else if (name.equals("MSG_ID_EM_EMM_SEC_INFO_IND")) {
            int ciphering_alg = getFieldValue(data, "ciphering_alg");
            int integrity_alg = getFieldValue(data, "integrity_alg");
            if (ciphering_alg == 0xff) {
                m4gEnasCipher.setText("N/A");
            } else {
                m4gEnasCipher.setText(m4gEnasCipherInfo[ciphering_alg]);
            }
            if (integrity_alg == 0xff) {
                m4gEnasIntegrity.setText("N/A");
            } else {
                m4gEnasIntegrity.setText(m4gEnasIntegrityInfo[integrity_alg]);
            }
            Elog.d(TAG, "4G ciphering_alg = " + ciphering_alg + " = " +
                    ((ciphering_alg == 0xff) ? "N/A" : m4gEnasCipherInfo[ciphering_alg]));
            Elog.d(TAG, "4G integrity_alg = " + integrity_alg + " = " +
                    ((integrity_alg == 0xff) ? "N/A" : m4gEnasIntegrityInfo[integrity_alg]));
        } else if (name.equals("MSG_ID_EM_ERRC_SEC_PARAM_IND")) {
            int enc_algo = getFieldValue(data, "enc_algo");
            int int_algo = getFieldValue(data, "int_algo");
            if (enc_algo == 0xff) {
                m4gErrcCipher.setText("N/A");
                return;
            } else {
                m4gErrcCipher.setText(m4gErrcCipherInfo[enc_algo]);
            }
            if (int_algo == 0xff) {
                m4gErrcIntegrity.setText("N/A");
                return;
            } else {
                m4gErrcIntegrity.setText(m4gErrcIntegrityInfo[int_algo]);
            }
            Elog.d(TAG, "enc_algo = " + enc_algo + " = " +
                    ((enc_algo == 0xff) ? "N/A" : m4gErrcCipherInfo[enc_algo]));
            Elog.d(TAG, "int_algo = " + int_algo + " = " +
                    ((int_algo == 0xff) ? "N/A" : m4gErrcIntegrityInfo[int_algo]));

        } else if (name.equals("MSG_ID_EM_ERRC_STATE_IND")) {
            int errc_sts = getFieldValue(data, "errc_sts");
            Elog.d(TAG, "errc_sts = " + errc_sts);
            if (errc_sts != 3 && errc_sts != 6) {
                m4gErrcCipher.setText("N/A");
                m4gErrcIntegrity.setText("N/A");
            }
        }

    }

    @Override
    public void onUpdateMDMStatus(int msg_id) {
        switch (msg_id) {
            case MDMCoreOperation.MDM_SERVICE_INIT: {
                Elog.d(TAG, "MDM Service init done");
                registerNetworkInfo();
                break;
            }
            case MDMCoreOperation.SUBSCRIBE_DONE: {
                Elog.d(TAG, "Subscribe message id done");
                MDMCoreOperation.getInstance().mdmlEnableSubscribe();
                break;
            }
            case MDMCoreOperation.UNSUBSCRIBE_DONE:
                Elog.d(TAG, "UnSubscribe message id done");
                MDMCoreOperation.getInstance().mdmlClosing();
                break;
            default:
                break;
        }
    }
}
