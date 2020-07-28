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
package com.mediatek.engineermode.cxp;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.RadioGroup.LayoutParams;
import android.widget.Spinner;
import android.widget.Toast;

import com.mediatek.engineermode.R;
import com.mediatek.common.carrierexpress.CarrierExpressManager;
import com.mediatek.common.carrierexpress.ICarrierExpressService;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * CarrierExpressActivity that allows operator switching using carrier express mechanism.
 */
public class CarrierExpressActivity extends Activity {
    private static final String TAG = "PhoneConfigurationSettings";

    private int mSelectedIndex;
    private AlertDialog mCarrierConfigAlertDialog;
    private ICarrierExpressService mCarrierExpressManager;

    private Spinner mOptrListSpinner;
    private Spinner mSubIdListSpinner;
    private RadioGroup mSimRadioGr;
    private String[] mChoices;
    private String[] mValues;
    Context mContext;
    private String mSelectedOptr;
    private String mSelectedSubId;
    private int mSelectedSim;
    private static final String SERVICE_PKG_NAME = "com.mediatek.carrierexpress";
    private static final String SERVICE_NAME = "com.mediatek.carrierexpress.CarrierExpressApp";

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder binder) {
            mCarrierExpressManager = ICarrierExpressService.Stub.asInterface(binder);
            Log.d(TAG, "onServiceConnected with mSesService = " + mCarrierExpressManager);

            prepareOperatorList();
            prepareOperatorSubIdList();
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            Log.d(TAG, "onServiceDisconnected()");
            if (mCarrierExpressManager != null) {
                mCarrierExpressManager = null;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.carrier_express_content);
        mContext = this;
        mSelectedIndex = 0;

        //mRadioGr = (RadioGroup) findViewById(R.id.cxp_radiogroup);
        mOptrListSpinner = (Spinner) findViewById(R.id.optr_list_spinner);
        mOptrListSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view,
                    int pos, long id) {
                mSelectedOptr = mValues[pos].toString();
                Log.d(TAG, "onItemSelected: " + mSelectedOptr);
                // Need to update sub ID list
                prepareOperatorSubIdList();
            }

            public void onNothingSelected(AdapterView<?> parent) {
                // Another interface callback
            }
        });

        mSubIdListSpinner = (Spinner) findViewById(R.id.subid_list_spinner);
        mSubIdListSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            public void onItemSelected(AdapterView<?> parent, View view,
                    int pos, long id) {
                mSelectedSubId = parent.getItemAtPosition(pos).toString();
            }

            public void onNothingSelected(AdapterView<?> parent) {
                // Another interface callback
            }
        });

        mSimRadioGr = (RadioGroup) findViewById(R.id.cxp_sim_radiogroup);
        Button button = (Button) findViewById(R.id.apply_button_id);
         button.setOnClickListener(new View.OnClickListener() {
             public void onClick(View v) {
                 Log.d(TAG, "onClick Apply: " + mSelectedOptr);
                 setOpPackActive(mSelectedOptr, mSelectedSubId, mSelectedSim);
             }
         });

        startCXPService();
        bindCXPService();
     }

    @Override
    public void onDestroy() {
        super.onDestroy();
        unbindCXPService();
    }

    private void startCXPService(){
        Log.d(TAG,"start CXP service");
        Intent startIntent = new Intent();
        startIntent.setClassName(SERVICE_PKG_NAME, SERVICE_NAME);
        startService(startIntent);
    }

    private void stopCXPService(){
        Log.d(TAG,"stop CXP service");
        Intent stopIntent = new Intent();
        stopIntent.setClassName(SERVICE_PKG_NAME, SERVICE_NAME);
        stopService(stopIntent);
    }

    private void bindCXPService(){
        Log.d(TAG,"Bind CXP service");
        Intent bindIntent = new Intent();
        bindIntent.setClassName(SERVICE_PKG_NAME, SERVICE_NAME);
        if(!bindService(bindIntent, mServiceConnection, Context.BIND_AUTO_CREATE)) {
            Log.d(TAG, "Bind CXP service failed");
            Toast.makeText(mContext, mContext.getResources().getString(
                    R.string.cxp_service_not_ready), Toast.LENGTH_SHORT).show();
            setResult(Activity.RESULT_CANCELED);
            finish();
        }
    }

    private void unbindCXPService() {
        if(mCarrierExpressManager != null) {
            Log.d(TAG, "Unbind CXP service, instance = " + mCarrierExpressManager);
            unbindService(mServiceConnection);
            mCarrierExpressManager = null;
        }
    }

    private void prepareOperatorSubIdList() {
        // reassemble list
        List<String> options = getOperatorSubIdList(mSelectedOptr);

        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_spinner_item, options);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSubIdListSpinner.setAdapter(adapter);
        if (mSelectedOptr != null && mSelectedOptr.equals(getActiveOpPack())) {
            mSelectedSubId = SystemProperties.get("persist.vendor.operator.subid", "0");
        } else {
            mSelectedSubId = new String("0");
        }
        mSubIdListSpinner.setSelection(adapter.getPosition(mSelectedSubId));
    }

    @Override
    protected void onStart() {
        super.onStart();
    }

    private void prepareOperatorList() {
      Map<String, String> opList = getAllOpPackList();
        Log.d(TAG, "opList:" + opList);
        TelephonyManager telephonyManager =
                    (TelephonyManager) getSystemService(Context.TELEPHONY_SERVICE);
        int phoneCount = telephonyManager.getPhoneCount();
        int mainSlot = 0;
        if (phoneCount == 2) {
            mainSlot = SystemProperties.getInt("persist.vendor.mtk_usp_ds_main_slot", 0);
        }

        if (opList != null) {
            String mccMnc = telephonyManager.getSimOperatorNumericForPhone(mainSlot);
            // Operator Id of operator whose sim is present
            String currentCarrierId = getOpPackFromSimInfo(mccMnc);
            Log.d(TAG, "mccMnc:" + mccMnc + ",currentCarrierId:" + currentCarrierId);
            int size = opList.size();
            String selectedOpPackId = getActiveOpPack();
            if (selectedOpPackId == null || selectedOpPackId.isEmpty()) {
                selectedOpPackId = new String("");
            }
            Log.d(TAG, "selectedOpPackId: " + selectedOpPackId);

            mChoices = new String[size];
            mValues = new String[size];
            int index = 0;
            ArrayList<String> options = new ArrayList<String>();
            for (Map.Entry<String, String> pair : opList.entrySet()) {
                String choice = pair.getValue();
                if (currentCarrierId != null && currentCarrierId.equals(pair.getKey())) {
                    choice += getResources().getString(R.string.recommended);
                }
                // OP ID
                mValues[index] = pair.getKey();
                // OP Name
                mChoices[index] = choice;
                Log.d(TAG, "value[" + index + "]: " + mValues[index]
                        + "-->Choice[" + index + "]: " + mChoices[index]);
                options.add(mChoices[index]);
                if (selectedOpPackId != null && selectedOpPackId.equals(mValues[index])) {
                    mSelectedIndex = index;
                }
                index++;
            }

            // use default spinner item to show options in spinner
            ArrayAdapter<String> adapter = new ArrayAdapter<String>(this,
                    android.R.layout.simple_spinner_item, options);
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mOptrListSpinner.setAdapter(adapter);
            mOptrListSpinner.setSelection(mSelectedIndex);
            mSelectedOptr = mValues[mSelectedIndex];
            Log.d(TAG, "mSelectedIndex: " + mSelectedIndex);
        }
        // Add primary SIM selection information for user
        // create radio button for this item here
        if (phoneCount != 2) {
          phoneCount = 1;
        }
        int primarySim = SystemProperties.getInt("persist.vendor.mtk_usp_ds_main_slot", 0);
        if (primarySim != 1) {
            primarySim = 0;
        }
        for (int id = 0; id < phoneCount; ++id) {
            RadioButton radioButton = new RadioButton(this);
            radioButton.setText("SIM card " + (id + 1));
            radioButton.setId(id);
            radioButton.setOnClickListener(new View.OnClickListener() {
                public void onClick(View v) {
                    // Is the button now checked?
                    Log.d(TAG, "onClick: for SIM: " + v.getId());
                    // set SIM value here based on selection
                    mSelectedSim = v.getId();
                }
            });
            RadioGroup.LayoutParams rprms = new RadioGroup.LayoutParams(LayoutParams.WRAP_CONTENT,
                    LayoutParams.WRAP_CONTENT);
            mSimRadioGr.addView(radioButton);
        }
        mSimRadioGr.check(primarySim);
        mSelectedSim = primarySim;
    }

    private String getActiveOpPack() {
        try {
            return mCarrierExpressManager.getActiveOpPack();
        } catch (RemoteException e) {
            Log.e(TAG, "getActiveOpPack: " + e.toString());
            return null;
        }
    }

    private String getOpPackFromSimInfo(String mccMnc) {
        try {
            return mCarrierExpressManager.getOpPackFromSimInfo(mccMnc);
        } catch (RemoteException e) {
            Log.e(TAG, "getOpPackFromSimInfo: " + e.toString());
            return null;
        }
    }

    private void setOpPackActive(String opPack, String opSubId, int mainSlot) {
        try {
            mCarrierExpressManager.setOpPackActive(opPack, opSubId, mainSlot);
        } catch (RemoteException e) {
            Log.e(TAG, "setOpPackActive: " + e.toString());
        }
    }

    private Map<String, String> getAllOpPackList() {
        try {
            return mCarrierExpressManager.getAllOpPackList();
        } catch (RemoteException e) {
            Log.e(TAG, "getAllOpPackList: " + e.toString());
            return null;
        }
    }

    private List<String> getOperatorSubIdList(String opPack) {
        try {
            return mCarrierExpressManager.getOperatorSubIdList(opPack);
        } catch (RemoteException e) {
            Log.e(TAG, "getOperatorSubIdList: " + e.toString());
            return null;
        }
    }
}