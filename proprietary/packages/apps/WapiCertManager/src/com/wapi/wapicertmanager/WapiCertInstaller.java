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

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.SharedPreferences;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.security.KeyStore;
import android.text.TextUtils;
import android.util.Log;

import android.view.View;
import android.view.Window;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Toast;

import java.util.ArrayList;

import com.wapi.wapicertstore.*;

public class WapiCertInstaller extends Activity {
    private static final String TAG = "WapiCertInstaller";

    public static final String INSTALL_ACTION = "com.wapi.certificate.INSTALL";

    public static final String DELETE_ACTION = "com.wapi.certificate.DELETE";

    public static final String GET_ALIASES_ACTION = "com.wapi.certificate.GET_ALIASES";

    public static final String RESET_CFG_ACTION = "com.wapi.certificate.RESET_CFG";

    private static final int REQUEST_SYSTEM_INSTALL_CODE = 1;

    // certificate rename dialog
    private AlertDialog mCertRenameDlg;
    // certificate delete dialog
    private AlertDialog mCertDelDlg;
    // certificate rename view
    private View mCertRenameView;
    // user certificate
    private byte[] mUserCert = null;
    // private key
    private byte[] mPriKey = null;
    // ca certificate
    private byte[] mCaCert = null;
    // certificate alias
    private String mCertAlias = "";

    // wapi cert store interface
    private WapiCertStore mWapiCertStore = new WapiCertStore();

    // keystore instance
    private final KeyStore mKeyStore = KeyStore.getInstance();

    @Override
    protected void onCreate(Bundle savedStates) {
        super.onCreate(savedStates);
        requestWindowFeature(Window.FEATURE_NO_TITLE);
        Intent intent = getIntent();
        String action = (intent == null) ? null : intent.getAction();
        Log.d(TAG, "Recv action: " + action);
        // Install wapi certificate
        if (INSTALL_ACTION.equals(action)) {
            Bundle bundle = intent.getExtras();
            if ((bundle == null) || bundle.isEmpty()) {
                Log.e(TAG, "Bundle is null: ");
                setResult(0);
                finish();
            } else {
                mUserCert = bundle.getByteArray("USER_CERT");
                mCaCert = bundle.getByteArray("CA_CERT");
                mPriKey = bundle.getByteArray("PRIVATE_KEY");
                mCertAlias = bundle.getString("ALIAS");
                Log.d(TAG, "alias: " + mCertAlias);
                if (!mWapiCertStore
                        .checkUserCaCert(mUserCert, mPriKey, mCaCert)) {
                    setResult(0);
                    finish();
                }

                if (mKeyStore.state() != KeyStore.State.UNLOCKED) {
                    Toast.makeText(WapiCertInstaller.this,
                            R.string.missing_private_key,
                            Toast.LENGTH_LONG).show();
                    setResult(0);
                    finish();
                }
                nameCert();
            }
        }
        // Delete wapi certificate
        else if (DELETE_ACTION.equals(action)) {
            Bundle bundle = intent.getExtras();
            if ((bundle == null) || bundle.isEmpty()) {
                Log.e(TAG, "Bundle is null: ");
                setResult(0);
                finish();
            } else {
                mCertAlias = bundle.getString("ALIAS");
                Log.d(TAG, "alias: " + mCertAlias);
                onCreateDelCertDlg();
            }
        }
        // Get wapi certificate alias list
        else if (GET_ALIASES_ACTION.equals(action)) {
            String[] certAliasArray = mWapiCertStore.getCertAliasList();
            if (certAliasArray == null) {
                Log.e(TAG, "getCertAliasList: certAliasArray is null");
                setResult(0);
                finish();
            }
            Log.d(TAG, "certAliasArray length: " + certAliasArray.length);
            intent.putExtra("ALIASES", certAliasArray);
            setResult(1, intent);
            finish();
        }
        // reset wapi cert info cfg
        else if (RESET_CFG_ACTION.equals(action)) {
            clearCertInfo();
            setResult(1, intent);
            finish();
        }
    }

    @Override
    protected void onResume() {
        Log.d(TAG, "onResume");
        super.onResume();
    }

    @Override
    protected void onPause() {
        Log.d(TAG, "onPause");
        super.onPause();
    }

    private void nameCert() {
        onCreateNameCertDlg();
    }

    private AlertDialog onCreateNameCertDlg() {
        mCertRenameView = getLayoutInflater().inflate(R.layout.cert_rename_dlg,
                null);
        final EditText nameInput = (EditText) mCertRenameView
                .findViewById(R.id.cert_rename);
        nameInput.setText(mCertAlias);
        nameInput.selectAll();
        AlertDialog.Builder ad = new AlertDialog.Builder(this);
        ad.setTitle(getString(R.string.cert_rename_dlg_title_name));
        ad.setView(mCertRenameView);
        mCertRenameDlg = ad.create();

        mCertRenameDlg.setButton(DialogInterface.BUTTON_POSITIVE,
                getString(R.string.button_caption_ok),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        String name = getViewText(mCertRenameView,
                                R.id.cert_rename);
                        String[] certAliasArray = mWapiCertStore
                                .getCertAliasList();
                        if (certAliasArray == null
                                || certAliasArray.length == 0) {
                            clearCertInfo();
                        }

                        if (TextUtils.isEmpty(name)) {
                            Toast.makeText(WapiCertInstaller.this,
                                    R.string.text_enter_cert_rename,
                                    Toast.LENGTH_LONG).show();
                            onCreateNameCertDlg();
                        } else if (isCertAliasExist(name, certAliasArray)) {
                            Toast.makeText(WapiCertInstaller.this,
                                    R.string.text_name_has_exists,
                                    Toast.LENGTH_LONG).show();
                            onCreateNameCertDlg();
                        } else {
                            Log.d(TAG, "The input name is: " + name);
                            if (mWapiCertStore.installCert(mUserCert, mPriKey,
                                    mCaCert, name)) {
                                if (!saveCertInfoItem(mUserCert, name)) {
                                    Log.e(TAG, "save cert info faild. ");
                                    setResult(0);
                                    finish();
                                } else {
                                    Log.d(TAG, "cert has saved, success. ");
                                    setResult(1);
                                    finish();
                                }
                            } else {
                                Log.e(TAG, "cert install faild. ");
                                setResult(0);
                                finish();
                            }
                        }
                    }
                });

        mCertRenameDlg.setButton(DialogInterface.BUTTON_NEGATIVE,
                getString(R.string.button_caption_cancel),
                new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        Log.d(TAG, "cert install canceled. ");
                        setResult(0);
                        finish();
                    }
                });

        mCertRenameDlg
                .setOnCancelListener(new DialogInterface.OnCancelListener() {
                    @Override
                    public void onCancel(DialogInterface dialog) {
                        Log.d(TAG, "cert install faild. ");
                        setResult(0);
                        finish();
                    }
                });

        mCertRenameDlg.show();
        return mCertRenameDlg;
    }

    private AlertDialog onCreateDelCertDlg() {
        AlertDialog.Builder ad = new AlertDialog.Builder(this);
        // ad.setTitle(R.string.wapi_cert_title_name);
        // ad.setIcon(android.R.drawable.ic_dialog_alert);
        ad.setMessage(R.string.text_confirm_del_cert);
        mCertDelDlg = ad.create();

        ad.setPositiveButton(R.string.button_caption_delete,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface Dialog, int id) {
                        Dialog.dismiss();

                        if (mWapiCertStore.deleteCert(mCertAlias)) {
                            removeCertInfoItem(mCertAlias);
                            Log.d(TAG, "delete cert success.");
                            setResult(1);
                        } else {
                            Log.e(TAG, "delete cert faild!");
                            setResult(0);
                        }

                        finish();
                    }
                });

        ad.setNegativeButton(R.string.button_caption_cancel,
                new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface Dialog, int id) {
                        Log.d(TAG, "delete cert option canceled.");
                        setResult(0);
                        finish();
                    }
                });

        ad.setOnCancelListener(new DialogInterface.OnCancelListener() {
            @Override
            public void onCancel(DialogInterface dialog) {
                Log.d(TAG, "delete cert option canceled.");
                setResult(0);
                finish();
            }
        });

        ad.show();

        return mCertDelDlg;
    }

    private String getViewText(View view, int viewId) {
        return ((TextView) view.findViewById(viewId)).getText().toString();
    }

    private boolean isCertAliasExist(String alias, String[] certAliasArray) {
        // String[] certAliasArray = mWapiCertStore.getCertAliasList();
        boolean bNameExist = false;
        if (certAliasArray != null && certAliasArray.length > 0) {
            int length = certAliasArray.length;
            for (int i = 0; i < length; i++) {
                if (alias.equals(certAliasArray[i])) {
                    bNameExist = true;
                    Log.w(TAG, "The input name [" + alias + "] has exists");
                    break;
                }
            }
        }

        return bNameExist;
    }

    // save wapi cert info item
    private boolean saveCertInfoItem(byte[] userCert, String alias) {
        String certInfo = mWapiCertStore.getCertInfo(userCert);
        if (certInfo == null || certInfo.equals("")) {
            Log.e(TAG, "saveCertInfoItem: get cert info faild");
            return false;
        }

        SharedPreferences settings = getSharedPreferences(
                WapiCertUtil.CERT_INFO_PREFS_NAME, 0);
        SharedPreferences.Editor certInfoCfg = settings.edit();
        certInfoCfg.putString(alias, certInfo);
        certInfoCfg.commit();

        return true;
    }

    // get wapi cert info item
    private String getCertInfoItem(String alias) {
        SharedPreferences settings = getSharedPreferences(
                WapiCertUtil.CERT_INFO_PREFS_NAME, 0);
        return settings.getString(alias, "");
    }

    // remove wapi cert info item
    private boolean removeCertInfoItem(String alias) {
        SharedPreferences settings = getSharedPreferences(
                WapiCertUtil.CERT_INFO_PREFS_NAME, 0);
        SharedPreferences.Editor certInfoCfg = settings.edit();
        certInfoCfg.remove(alias);
        certInfoCfg.commit();
        return true;
    }

    private boolean clearCertInfo() {
        SharedPreferences settings = getSharedPreferences(
                WapiCertUtil.CERT_INFO_PREFS_NAME, 0);
        SharedPreferences.Editor certInfoCfg = settings.edit();
        certInfoCfg.clear();
        certInfoCfg.commit();
        return true;
    }
}
