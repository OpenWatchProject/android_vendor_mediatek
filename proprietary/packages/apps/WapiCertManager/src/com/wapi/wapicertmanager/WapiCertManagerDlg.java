/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
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
 */

package com.wapi.wapicertmanager;

import android.app.AlertDialog;
import android.content.Context;
import android.content.Intent;
import android.content.DialogInterface;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Bundle;
import android.os.Environment;
import android.os.Process;

import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TextView;
import android.widget.Toast;

import android.text.TextUtils;
import android.text.format.Formatter;
import android.text.method.PasswordTransformationMethod;
import android.text.method.TransformationMethod;
import android.util.Log;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.BufferedReader;
import java.util.ArrayList;

import com.wapi.wapicertstore.*;

public class WapiCertManagerDlg extends AlertDialog implements
        DialogInterface.OnClickListener, AdapterView.OnItemSelectedListener {
    private static final String TAG = "WapiCertManagerDlg";

    private static final String SDCARD_ROOT_PATH = "/sdcard";

    // wapi cert manage interface
    private WapiCertStore mWapiCertStore = null;

    private CharSequence mCustomTitle;

    // dialog init mode
    public static final int MODE_ADD_CERT = 0;
    private int mMode = MODE_ADD_CERT;

    // General views
    private View mView;
    // p12 password EditText widget
    private EditText mP12EditText;
    private TextView mP12RemindText;
    // user certificate spinner
    private Spinner mUserCertSpinner;
    private ArrayList<String> mUserCertArray = new ArrayList<String>();

    // ca certificate list
    // Spinner mIssuerCertSpinner;
    private ArrayList<String> mIssuerCertArray = new ArrayList<String>();
    private String mIssuerCertName = "";

    // user certificate name
    private String mUserCertName = "";
    // user certificate alias
    private String mUserCertAlias = "";
    // user certificate path
    private String mUserCertPath = "";
    // ca certificate path
    private String mCaCertpath = "";
    // p12 certificate flag
    private boolean mbP12Cert = false;
    // p12 password
    private String mP12Pwd = null;
    // user certificate
    private byte[] mUserCert;
    // user private key
    private byte[] mPriKey;
    // CA certificate
    private byte[] mCaCert;

    // buttons
    private static final int INSTALL_CERT_BUTTON = BUTTON1;
    private static final int CANCEL_BUTTON = BUTTON2;
    private int mInstallCertButtonPos = Integer.MAX_VALUE;
    private int mCancelButtonPos = Integer.MAX_VALUE;

    // define the listenner for calling main activity
    private WapiCertListenner mWapiCertListenner;

    interface WapiCertListenner {
        public void notifyReopenDlg(String userCertName);

        public void notifyInstallCert(String userCertName, String userCertPath,
                String caCertPath, byte[] userCert, byte[] priKey, byte[] caCert);
    };

    public WapiCertManagerDlg(Context context, WapiCertListenner listenner,
            String userCertName) {
        // public WapiCertManagerDlg(Context context) {
        super(context);
        mWapiCertListenner = listenner;
        mUserCertName = userCertName;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        onLayout();
        super.onCreate(savedInstanceState);
    }

    private void onLayout() {
        int positiveButtonResId = 0;
        int negativeButtonResId = 0;
        int neutralButtonResId = 0;

        setInverseBackgroundForced(true);

        if (mMode == MODE_ADD_CERT) {
            setLayout(R.layout.add_cert_dlg);
            setTitle(R.string.add_cert_dlg_title_name);
            positiveButtonResId = R.string.button_caption_install;
            mInstallCertButtonPos = INSTALL_CERT_BUTTON;
        }

        negativeButtonResId = R.string.button_caption_cancel;
        mCancelButtonPos = CANCEL_BUTTON;

        setButtons(positiveButtonResId, negativeButtonResId, neutralButtonResId);
    }

    private void setLayout(int layoutResId) {
        setView(mView = getLayoutInflater().inflate(layoutResId, null));
        onReferenceViews(mView);
    }

    /** Called when we need to set our member variables to point to the views. */
    private void onReferenceViews(View view) {
        if (mMode == MODE_ADD_CERT) {
            Log.v(TAG, "MODE_ADD_CERT");

            mUserCertSpinner = (Spinner) view
                    .findViewById(R.id.sp_sel_user_cert);
            mUserCertSpinner.setOnItemSelectedListener(this);
            setUserCertSpinnerAdapter();
            mP12RemindText = (TextView) view
                    .findViewById(R.id.p12_password_text);
            mP12RemindText.setVisibility(View.GONE);
            mP12EditText = (EditText) view.findViewById(R.id.p12_password);
            mP12EditText.setVisibility(View.GONE);
        }
    }

    public void setMode(int mode) {
        mMode = mode;
    }

    public void setWapiCertStore(WapiCertStore wapiCertStore) {
        mWapiCertStore = wapiCertStore;
    }

    public void setUserCerRename(String UserCertRename) {
        mUserCertAlias = UserCertRename;
    }

    private void setButtons(int positiveResId, int negativeResId,
            int neutralResId) {
        final Context context = getContext();

        if (positiveResId > 0) {
            setButton(context.getString(positiveResId), this);
        }

        if (neutralResId > 0) {
            setButton3(context.getString(neutralResId), this);
        }

        if (negativeResId > 0) {
            setButton2(context.getString(negativeResId), this);
        }
    }

    public void onClick(DialogInterface dialog, int which) {
        Log.v(TAG, "onClick which " + which);

        if (which == mInstallCertButtonPos) {
            if (handleInstallCert()) {
                mWapiCertListenner
                        .notifyInstallCert(mUserCertName, mUserCertPath,
                                mCaCertpath, mUserCert, mPriKey, mCaCert);
            } else {
                mWapiCertListenner.notifyReopenDlg(mUserCertName);
            }
        } else if (which == mCancelButtonPos) {
            handleCancle();
        }
    }

    public void onItemSelected(AdapterView parent, View view, int position,
            long id) {
        Log.v(TAG, "onItemSelected ");

        if (parent == mUserCertSpinner) {
            handleUserCertChange(getUserCertFromSpinner());
        }
    }

    @Override
    public void setTitle(CharSequence title) {
        super.setTitle(title);
        mCustomTitle = title;
    }

    @Override
    public void setTitle(int titleId) {
        setTitle(getContext().getString(titleId));
    }

    private boolean handleInstallCert() {
        Log.d(TAG, "Handle install user cert: " + mUserCertName);

        if (null == mUserCertName || TextUtils.isEmpty(mUserCertName)) {
            Toast.makeText(getContext(),
                    R.string.text_not_find_valid_user_cert_in_SD,
                    Toast.LENGTH_LONG).show();
            return false;
        }

        mUserCertPath = SDCARD_ROOT_PATH + "/" + mUserCertName;
        Log.d(TAG, "user cert path: " + mUserCertPath);

        if (mbP12Cert) {
            mP12Pwd = mP12EditText.getText().toString();
            if ((mP12Pwd == null) || TextUtils.isEmpty(mP12Pwd)) {
                Toast.makeText(getContext(), R.string.text_p12_password_empty,
                        Toast.LENGTH_LONG).show();
                return false;
            }

            File userCertFile = new File(mUserCertPath);
            byte[] p12UserCertData = WapiCertUtil.readFile(userCertFile);
            byte[] userCertData = mWapiCertStore.parseP12Cert(p12UserCertData, mP12Pwd);
            if (userCertData == null) {
                Toast.makeText(getContext(), R.string.text_invalid_user_cert,
                        Toast.LENGTH_LONG).show();
                return false;
            } else if(userCertData.length == 0) {
                Toast.makeText(getContext(), R.string.text_p12_password_error,
                        Toast.LENGTH_LONG).show();
                return false;
            }
            mUserCert = WapiCertUtil.getCertElement(userCertData);
            mPriKey = WapiCertUtil.getPriKeyElement(userCertData);

            int certType = WapiCertUtil.getCertType(mUserCert);
            if (certType != WapiCertUtil.CERT_TYPE_WAPI_USER) {
                Log.e(TAG, "Invalid user cert! ");
                Toast.makeText(getContext(), R.string.text_invalid_user_cert,
                        Toast.LENGTH_LONG).show();
                return false;
            }
        } else {
            File userCertFile = new File(mUserCertPath);
            byte[] userCertData = WapiCertUtil.readFile(userCertFile);
            mUserCert = WapiCertUtil.getCertElement(userCertData);
            mPriKey = WapiCertUtil.getPriKeyElement(userCertData);
        }

        return matchWapiCert(mUserCert, mPriKey);
    }

    private boolean matchWapiCert(byte[] userCert, byte[] priKey) {
        int index = 0;
        int caCertCount = 0;

        caCertCount = mIssuerCertArray.size();
        Log.d(TAG, "ca cert count: " + caCertCount);

        for (index = 0; index < caCertCount; index++) {
            String caCertName = mIssuerCertArray.get(index);
            String caCertPath = SDCARD_ROOT_PATH + "/" + caCertName;
            Log.d(TAG, "ca cert path: " + caCertPath);

            File caCertFile = new File(caCertPath);
            byte[] caCertData = WapiCertUtil.readFile(caCertFile);
            if (mWapiCertStore.checkUserCaCert(userCert, priKey, caCertData)) {
                mCaCertpath = caCertPath;
                mCaCert = caCertData;
                return true;
            }
        }

        Toast.makeText(getContext(),
                R.string.text_not_match_valid_ca_cert_in_SD, Toast.LENGTH_LONG)
                .show();
        return false;
    }

    private String getInput(EditText edit) {
        return (edit != null) ? (edit.getText().toString()) : null;
    }

    private void setSelection(Spinner spinner, String value) {
        if (value != null && !TextUtils.isEmpty(value)) {
            @SuppressWarnings("unchecked")
            ArrayAdapter<String> adapter = (ArrayAdapter<String>) spinner
                    .getAdapter();
            for (int i = adapter.getCount() - 1; i >= 0; --i) {
                if (value.equals(adapter.getItem(i))) {
                    spinner.setSelection(i);
                    break;
                }
            }
        }
    }

    private void setUserCertSpinnerAdapter() {
        Context context = getContext();
        File certificateList[];
        int i = 0;

        mUserCertArray.clear();
        mIssuerCertArray.clear();
        File certificatePath = Environment.getExternalStorageDirectory();
        try {
            if (certificatePath != null) 
            {
                // Log.d(TAG, "From sd card, cert list: ");
                certificateList = certificatePath.listFiles();
                for (i = 0; i < certificateList.length; i++) 
                {
                    if (WapiCertUtil.isTheSuffix(certificateList[i].getAbsoluteFile().toString(), ".cer"))
                    {
                        // Log.d(TAG, "cert name: " + certificateList[i].getName());
                        byte[] certData = WapiCertUtil.readFile(certificateList[i]);
                        if (certData == null) {
                            continue;
                        }
                        byte[] cert = WapiCertUtil.getCertElement(certData);
                        if (cert == null) {
                            continue;
                        }
                        int certType = WapiCertUtil.getCertType(cert);
                        if (certType == WapiCertUtil.CERT_TYPE_WAPI_USER) {
                            // Log.d(TAG, "Add a wapi user cert.");
                            mUserCertArray.add(certificateList[i].getName());
                        }
                        else if (certType == WapiCertUtil.CERT_TYPE_WAPI_CA) {
                            // Log.d(TAG, "Add a wapi ca cert.");
                            mIssuerCertArray.add(certificateList[i].getName());
                        }
                    }
                    else if(WapiCertUtil.isTheSuffix(certificateList[i].getAbsoluteFile().toString(), ".p12"))
                    {
                        // Log.d(TAG, "cert name: " + certificateList[i].getName());
                        // Log.d(TAG, "Add a p12 cert.");
                        mUserCertArray.add(certificateList[i].getName());
                    }
                    else
                    {
                        continue;
                    }
                }
            }

            ArrayAdapter<CharSequence> adapter = new ArrayAdapter<CharSequence>(
                    context, android.R.layout.simple_spinner_item,
                    (String[]) mUserCertArray.toArray(new String[0]));
            adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
            mUserCertSpinner.setAdapter(adapter);

            if (!TextUtils.isEmpty(mUserCertName)) {
                Log.d(TAG, "init, mUserCertName is not empty, set selection");
                setSelection(mUserCertSpinner, mUserCertName);
            }
        } catch (Exception e) {
            setMessage(e.toString());
        }
    }

    private void showP12PasswordFields() {

        int index;
        index = mUserCertSpinner.getSelectedItemPosition();
        if (index < 0) {
            return;
        }

        String[] certArray = (String[]) mUserCertArray.toArray(new String[0]);
        mUserCertName = certArray[index];
        String userCertNamepath = SDCARD_ROOT_PATH + "/" + mUserCertName;

        File userCertFile = new File(userCertNamepath);
        byte[] userCertData = WapiCertUtil.readFile(userCertFile);
        if(mWapiCertStore.isP12Cert(userCertData))
        {
            mbP12Cert = true;
            mP12EditText.setEnabled(true);
            mP12EditText.setVisibility(View.VISIBLE);
            mP12RemindText.setVisibility(View.VISIBLE);
        } else {
            mbP12Cert = false;
            mP12EditText.setVisibility(View.GONE);
            mP12EditText.setEnabled(false);
            mP12RemindText.setVisibility(View.GONE);
        }
    }

    private int getUserCertFromSpinner() {
        int position = mUserCertSpinner.getSelectedItemPosition();
        return position;
    }

    private void handleUserCertChange(int UserIdx) {
        try {
            String[] StringA = (String[]) mUserCertArray.toArray(new String[0]);
            if (UserIdx >= 0 && UserIdx < StringA.length) {
                mUserCertName = StringA[UserIdx];
            }
            showP12PasswordFields();
        } catch (Exception e) {
            setMessage(e.toString());
        }
    }

    private void handleCancle() {
        Log.v(TAG, "handleCancle");
    }

    public void onNothingSelected(AdapterView parent) {
        Log.v(TAG, "onNothingSelected ");
    }
}
