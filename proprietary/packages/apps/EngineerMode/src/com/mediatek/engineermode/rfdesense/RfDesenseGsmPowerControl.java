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

package com.mediatek.engineermode.rfdesense;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.AsyncResult;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemSelectedListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.R;

import java.util.HashMap;

public class RfDesenseGsmPowerControl extends Activity implements
        OnItemSelectedListener, OnClickListener {
    public static final String TAG = "RfDesense/GsmPowerControl";

    private static final int DIALOG_CONFIRM = 0;
    private static final int RF_DESENSE_GSM_SET = 0;
    private static final int RF_DESENSE_GSM_SET_MAX_POWER = 1;
    private static final int RF_DESENSE_GSM_QUERY = 2;
    private static final String BUNDLE_GSM_QUERY_RESULT = "pcl_result";

    private static final int DIALOG_QUERY_PCL = 0;


    private Button btnSet;
    private Button btnSetMaxPower;
    private Button btnQuery;
    private Spinner mSpPowerLevel;

    private Spinner mSpPowerBand;
    private ArrayAdapter<CharSequence> adapterPower;
    private ArrayAdapter<CharSequence> adapterLevel;
    private int selectedGsmBand;
    private int selectedGsmLevel;
    private boolean mIsActive = true;
    private final HashMap<Integer, String> gsmBandMapping = new HashMap<Integer, String>() {
        {
            put(1, "Band 850");
            put(2, "Band 900");
            put(3, "Band 1800");
            put(4, "Band 1900");

        }
    };
    private final HashMap<Integer, String> gsmModTypeMapping = new HashMap<Integer, String>() {
        {
            put(0, "GMSK");
            put(1, "8PSK");
        }
    };

    // private ProgressDialog mIndicator = null;

    private final Handler mATHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (!mIsActive) {
                return;
            }
            AsyncResult ar = (AsyncResult) msg.obj;
            String text = "";
            switch (msg.what) {
            case RF_DESENSE_GSM_SET:
                text = "Set PCL Value ";
                if (ar != null && ar.exception == null) {
                    if (ar.result != null) {
                        text += "Succeed.";
                    }
                } else {
                    text += "Failed.";
                }
                showToast(text);
                Elog.d(TAG, text);
                break;
            case RF_DESENSE_GSM_SET_MAX_POWER:
                text = "Set Max PCL Value ";
                if (ar != null && ar.exception == null) {
                    if (ar.result != null) {
                        text += "Succeed.";
                    }
                } else {
                    text += "Failed.";
                }
                showToast(text);
                Elog.d(TAG, text);
                break;
            case RF_DESENSE_GSM_QUERY:
                text = "Query PCL Value ";
                if (ar != null && ar.exception == null && ar.result != null) {
                    String result = ((String[]) ar.result)[0].substring(
                            "+EPCL:".length()).trim();
                    Elog.d(TAG, result);
                    Bundle bundle = new Bundle();
                    bundle.putString(BUNDLE_GSM_QUERY_RESULT, result);
                    showDialog(DIALOG_QUERY_PCL, bundle);
                    text += "Succeed.";
                } else {
                    text += "Failed.";
                    showToast(text);
                }
                Elog.d(TAG, text);
                break;
            }
        }
    };

    public int[] stringToIntArray(String src) {

        String[] strArray = src.split(",");
        int intArray[] = new int[strArray.length];
        for (int i = 0; i < strArray.length; i++) {
            try {
                intArray[i] = Integer.parseInt(strArray[i].trim());
            } catch (NumberFormatException e) {
                intArray[i] = -1;
            }
        }
        return intArray;
    }

    @Override
    protected void onPrepareDialog(int id, Dialog dialog, Bundle bundle) {
        switch (id) {
        case DIALOG_QUERY_PCL:
            String result = (String) bundle.getSerializable(BUNDLE_GSM_QUERY_RESULT);
            int[] pclResult = stringToIntArray(result);
            DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    dialog.dismiss();
                }
            };
            String alert1 = "GSM Band: " + gsmBandMapping.get(pclResult[0]);
            String alert2 = "PCL Value: "
                    + (pclResult[1] == -1 ? "Invalid" : "" + pclResult[1]);
            String alert3 = "Mod Type: " + gsmModTypeMapping.get(pclResult[2]);
            String alert4 = "APC DAC: "
                    + (pclResult[3] == -1 ? "Invalid" : "" + pclResult[3]);
            ((AlertDialog)dialog).setMessage(alert1 + "\n\n" + alert2 + "\n\n" + alert3 + "\n\n"
                    + alert4);
        }
        super.onPrepareDialog(id, dialog, bundle);
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle bundle) {
        switch (id) {
        case DIALOG_QUERY_PCL:
            String result = (String) bundle.getSerializable(BUNDLE_GSM_QUERY_RESULT);
            int[] pclResult = stringToIntArray(result);
            DialogInterface.OnClickListener listener = new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int whichButton) {
                    dialog.dismiss();
                }
            };
            String alert1 = "GSM Band: " + gsmBandMapping.get(pclResult[0]);
            String alert2 = "PCL Value: "
                    + (pclResult[1] == -1 ? "Invalid" : "" + pclResult[1]);
            String alert3 = "Mod Type: " + gsmModTypeMapping.get(pclResult[2]);
            String alert4 = "APC DAC: "
                    + (pclResult[3] == -1 ? "Invalid" : "" + pclResult[3]);
            AlertDialog.Builder builder = new AlertDialog.Builder(this);
            return builder
                    .setTitle("PCL Query Result")
                    .setMessage(
                            alert1 + "\n\n" + alert2 + "\n\n" + alert3 + "\n\n"
                                    + alert4).setPositiveButton("OK", listener)
                    .create();

        default:
            break;
        }
        return null;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.rf_desense_gsm_power_control);
        initUI();

    }

    private void initUI() {

        mSpPowerBand = (Spinner) findViewById(R.id.rf_desense_power_band);
        adapterPower = ArrayAdapter.createFromResource(this,
                R.array.rf_desense_test_gsm_power_control_band,
                android.R.layout.simple_spinner_item);
        adapterPower
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPowerBand.setAdapter(adapterPower);
        mSpPowerBand.setOnItemSelectedListener(this);

        btnSet = (Button) findViewById(R.id.rf_desense_gsm_set);
        btnSetMaxPower = (Button) findViewById(R.id.rf_desense_gsm_set_max_power);
        btnQuery = (Button) findViewById(R.id.rf_desense_gsm_query);
        btnSet.setOnClickListener(this);
        btnSetMaxPower.setOnClickListener(this);
        btnQuery.setOnClickListener(this);

        mSpPowerLevel = (Spinner) findViewById(R.id.rf_desense_gsm_power_level);
        adapterLevel = ArrayAdapter.createFromResource(this,
                R.array.rf_desense_test_gsm_power_control_level,
                android.R.layout.simple_spinner_item);
        adapterLevel
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPowerLevel.setAdapter(adapterLevel);
        mSpPowerLevel.setOnItemSelectedListener(this);
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onDestroy() {
        mIsActive = false;
        super.onDestroy();
    }

    private void sendAtCommand(String[] command, int msg) {
        Elog.d(TAG, "sendAtCommand() " + command[0]);
        showToast("sendAtCommand: " + command[0]);

        EmUtils.invokeOemRilRequestStringsEm(command, mATHandler.obtainMessage(msg));
    }

    private void showToast(String msg) {
        Toast mToast = Toast.makeText(this, msg, Toast.LENGTH_SHORT);
        mToast.show();
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position,
            long id) {
        // TODO Auto-generated method stub
        if (parent.getId() == R.id.rf_desense_power_band) {
            selectedGsmBand = position + 1;
            setOptionalGsmLevel(selectedGsmBand, parent);
        } else if (parent.getId() == R.id.rf_desense_gsm_power_level) {
            selectedGsmLevel = Integer.valueOf(parent.getItemAtPosition(
                    position).toString());
        }
    }

    private void setOptionalGsmLevel(int selectedGsmBand2, AdapterView<?> parent) {
        // TODO Auto-generated method stub
        if ((selectedGsmBand == 1 || selectedGsmBand == 2)) {
            String s1[] = new String[15];
            for (int i = 0; i < s1.length; i++) {
                s1[i] = String.valueOf(i + 5);
            }
            adapterLevel = new ArrayAdapter<CharSequence>(this,
                    android.R.layout.simple_spinner_item, s1);
        } else if (selectedGsmBand == 3 || selectedGsmBand == 4) {
            String s1[] = new String[16];
            for (int i = 0; i < s1.length; i++) {
                s1[i] = String.valueOf(i);
            }
            adapterLevel = new ArrayAdapter<CharSequence>(this,
                    android.R.layout.simple_spinner_item, s1);
        }
        adapterLevel
                .setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpPowerLevel.setAdapter(adapterLevel);

    }

    @Override
    public void onNothingSelected(AdapterView<?> parent) {
        // TODO Auto-generated method stub

    }

    @Override
    public void onClick(View view) {
        // TODO Auto-generated method stub
        String[] cmd = new String[2];
        switch (view.getId()) {
        case R.id.rf_desense_gsm_set:
            cmd[0] = "AT+EPCL=0," + selectedGsmBand + "," + selectedGsmLevel;
            cmd[1] = "";
            sendAtCommand(cmd, RF_DESENSE_GSM_SET);
            break;
        case R.id.rf_desense_gsm_set_max_power:
            cmd[0] = "AT+EPCL=1," + selectedGsmBand;
            cmd[1] = "";
            sendAtCommand(cmd, RF_DESENSE_GSM_SET_MAX_POWER);
            break;
        case R.id.rf_desense_gsm_query:
            cmd[0] = "AT+EPCL=2";
            cmd[1] = "+EPCL:";
            sendAtCommand(cmd, RF_DESENSE_GSM_QUERY);
            break;
        default:
            break;
        }
    }

}
