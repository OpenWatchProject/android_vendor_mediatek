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

import android.app.ActionBar;
import android.app.Activity;
import android.app.Dialog;
import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.content.ComponentName;
import android.content.Context;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Environment;
import android.os.Process;
import android.os.RemoteException;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.SimpleAdapter;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.EditText;
import android.widget.Toast;
import android.text.TextUtils;
import android.text.Html;
import android.security.KeyStore;

import java.io.File;
import java.io.FileInputStream;
import java.io.ObjectInputStream;
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.ObjectOutputStream;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.wapi.wapicertstore.*;

public class WapiCertManagerActivity extends Activity implements
        WapiCertManagerDlg.WapiCertListenner {

    // log tag
    private static final String TAG = "WapiCertManagerActivity";

    // menu item
    private static final int MENU_ID_ADD_CERT = Menu.FIRST;

    private static final int DETAIL_INFO = 0;
    private static final int DELETE_CERT = 1;
    // is activity first resume
    private boolean mbFirstResume = true;

    // wapi certificate listview
    private ListView mWapiCertListView;
    private ArrayList<HashMap<String, Object>> mWapiCertList;
    private SimpleAdapter adapter;
    private String mSelCertName;
    private int mSelItem;

    // add cert dialog
    private WapiCertManagerDlg mAddCertDlg;
    // p12 dialog
    private AlertDialog mP12PwdDlg;
    // cert rename dialog
    private AlertDialog mCertRenameDlg;
    // cert rename view
    private View mCertRenameView;

    // user certificate file path
    private String mUserCertPath;
    // ca certificate file path
    private String mCaCertPath;
    // user certificate file name
    private String mUserCertName;
    // user certificate alias
    private String mUserCertAlias;
    // alias of the certificate to be deleted
    private String mDelCertName;
    // user certificate
    private byte[] mUserCert;
    // user private key
    private byte[] mPriKey;
    // CA certificate
    private byte[] mCaCert;

    // keystore instance
    private final KeyStore mKeyStore = KeyStore.getInstance();

    // wapi certificate store interface
    private WapiCertStore mWapiCertStore = new WapiCertStore();

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        setContentView(R.layout.wapi_cert_list);
        mbFirstResume = true;
        mWapiCertListView = (ListView) findViewById(R.id.wapi_cert_list);
        mWapiCertList = new ArrayList<HashMap<String, Object>>();

        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);

        onGetWapiCertList();

        if (mWapiCertList.size() == 0) {
            // reset cert info config
            clearCertInfo();
        }

        adapter = new SimpleAdapter(this, mWapiCertList,
                R.layout.wapi_cert_list_item, new String[] { "cert_name",
                        "valid_to" },
                new int[] { R.id.cert_name, R.id.valid_to });
        mWapiCertListView.setAdapter(adapter);

        mWapiCertListView
                .setOnItemClickListener(new AdapterView.OnItemClickListener() {
                    public void onItemClick(AdapterView<?> arg0, View arg1,
                            int arg2, long arg3) {
                        HashMap<String, String> map = (HashMap<String, String>) mWapiCertListView
                                .getItemAtPosition(arg2);
                        mSelItem = arg2;
                        mSelCertName = map.get("cert_name");

                        DialogInterface.OnClickListener onSelect = new DialogInterface.OnClickListener() {
                            // @Override
                            public void onClick(DialogInterface dialog,
                                    int which) {
                                switch (which) {
                                case DETAIL_INFO:
                                    WapiCertManagerActivity.this
                                            .onStartCertDetailsActivity(mSelItem);
                                    break;
                                case DELETE_CERT:
                                    WapiCertManagerActivity.this
                                            .onDeleteWapiCertItem(mSelItem);
                                    break;
                                }
                            }
                        };

                        // cert options
                        String[] choices;
                        choices = new String[2];
                        choices[0] = getString(R.string.cert_option_details);
                        choices[1] = getString(R.string.cert_option_delete);

                        AlertDialog dialog = new AlertDialog.Builder(
                                WapiCertManagerActivity.this)
                                .setTitle(mSelCertName)
                                .setItems(choices, onSelect)
                                .setPositiveButton(
                                        getString(R.string.button_caption_cancel),
                                        new DialogInterface.OnClickListener() {
                                            public void onClick(
                                                    DialogInterface dialoginterface,
                                                    int i) {
                                            }
                                        }).create();
                        dialog.show();
                    }
                });
    }

    @Override
    protected void onResume() {
        super.onResume();

        Log.d(TAG, "onResume");
        if (mbFirstResume) {
            mbFirstResume = false;
        } else {
            onGetWapiCertList();
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    protected void onStart() {
        super.onStart();

        Log.d(TAG, "onStart");
    }

    @Override
    protected void onPause() {
        // TODO Auto-generated method stub
        super.onPause();

        Log.d(TAG, "onPause");
    }

    @Override
    protected void onStop() {
        // TODO Auto-generated method stub
        super.onPause();

        Log.d(TAG, "onStop");
    }

    @Override
    protected void onDestroy() {
        // TODO Auto-generated method stub
        super.onPause();

        Log.d(TAG, "onDestroy");
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // TODO Auto-generated method stub
        super.onCreateOptionsMenu(menu);
        menu.add(0, MENU_ID_ADD_CERT, 0, R.string.menu_item_add_cert)
                .setIcon(android.R.drawable.ic_menu_add)
                .setShowAsAction(MenuItem.SHOW_AS_ACTION_IF_ROOM);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // TODO Auto-generated method stub
        super.onOptionsItemSelected(item);

        switch (item.getItemId()) {
        case MENU_ID_ADD_CERT:
            onAddWapiCertItem("");
            return true;
        case android.R.id.home:
            finish();
        default:
            return false;
        }
    }

    public boolean onAddWapiCertItem(String userCertName) {
        if (onFindWapiCertFromSDRoot()) {
            onShowAddWapiCertDialog(userCertName);
            return true;
        } else {
            return false;
        }
    }

    private void onShowAddWapiCertDialog(String userCertName) {
        WapiCertManagerDlg dialog = new WapiCertManagerDlg(this, this,
                userCertName);
        dialog.setWapiCertStore(mWapiCertStore);

        dialog.setMode(WapiCertManagerDlg.MODE_ADD_CERT);
        dialog.setTitle(R.string.add_cert_dlg_title_name);
        mAddCertDlg = dialog;
        dialog.show();
    }

    public boolean onDeleteWapiCertItem(int nDelItemIndex) {
        int size = mWapiCertList.size();
        if (nDelItemIndex >= size) {
            return false;
        }

        HashMap<String, Object> map;
        map = (HashMap) mWapiCertList.get(nDelItemIndex);
        mDelCertName = (String) map.get("cert_name");
        Log.d(TAG, "Delete cert: " + mDelCertName);

        onCreateDelCertDlg();

        return true;
    }

    public void onStartCertDetailsActivity(int nSelItem) {
        Intent intent = new Intent();
        intent.setClass(WapiCertManagerActivity.this, CertDetailsActivity.class);
        Bundle bundle = new Bundle();

        HashMap<String, Object> map;
        String tmp;

        map = (HashMap) mWapiCertList.get(nSelItem);

        tmp = (String) map.get("cert_name");
        bundle.putString("cert_name", tmp);

        tmp = (String) map.get("type");
        bundle.putString("type", tmp);

        tmp = (String) map.get("sn");
        bundle.putString("sn", tmp);

        tmp = (String) map.get("issuer");
        bundle.putString("issuer", tmp);

        tmp = (String) map.get("subject");
        bundle.putString("subject", tmp);

        tmp = (String) map.get("valid_from");
        bundle.putString("valid_from", tmp);

        tmp = (String) map.get("valid_to");
        String preValidTo = getString(R.string.text_cert_valid_date);
        tmp = tmp.replace(preValidTo, "");
        bundle.putString("valid_to", tmp);

        tmp = (String) map.get("authority");
        bundle.putString("authority", tmp);

        // tmp=(String)map.get("action_state");
        // bundle.putString("action_state", tmp);

        intent.putExtras(bundle);
        startActivity(intent);
    }

    @Override
    protected Dialog onCreateDialog(int id) {
        switch (id) {
        default:
            return null;
        }
    }

    private AlertDialog onCreateNameCertDlg() {
        mCertRenameView = getLayoutInflater().inflate(R.layout.cert_rename_dlg,
                null);
        final EditText nameInput = (EditText) mCertRenameView
                .findViewById(R.id.cert_rename);
        nameInput.setText(getDefaultName());
        AlertDialog.Builder ad = new AlertDialog.Builder(this);
        ad.setTitle(getString(R.string.cert_rename_dlg_title_name));
        ad.setView(mCertRenameView);
        mCertRenameDlg = ad.create();

        mCertRenameDlg.setButton(DialogInterface.BUTTON_POSITIVE,
                getString(R.string.button_caption_ok), new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                        mUserCertAlias = getViewText(mCertRenameView,
                                R.id.cert_rename);
                        if (TextUtils.isEmpty(mUserCertAlias)) {
                            Toast.makeText(WapiCertManagerActivity.this,
                                    R.string.text_enter_cert_rename,
                                    Toast.LENGTH_LONG).show();
                            onCreateNameCertDlg();
                        } else if (isCertAliasExist(mUserCertAlias)) {
                            // the cert alias has exsists
                            Toast.makeText(WapiCertManagerActivity.this,
                                    R.string.text_name_has_exists,
                                    Toast.LENGTH_LONG).show();
                            onCreateNameCertDlg();
                        } else {
                            if (!checkCertAliasFormat(mUserCertAlias)) {
                                Toast.makeText(WapiCertManagerActivity.this,
                                        R.string.text_user_as_cert_rename_max,
                                        Toast.LENGTH_LONG).show();
                                onCreateNameCertDlg();
                            } else {
                                if (installWapiCert()) {
                                    // install success
                                    onGetWapiCertList();
                                    adapter.notifyDataSetChanged();
                                } else {
                                    // install faild
                                    Toast.makeText(WapiCertManagerActivity.this,
                                            R.string.text_install_cert_fail,
                                            Toast.LENGTH_LONG).show();
                                }
                            }
                        }
                    }
                });

        mCertRenameDlg.setButton(DialogInterface.BUTTON_NEGATIVE,
                getString(R.string.button_caption_cancel),
                new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface arg0, int arg1) {
                    }
                });

        mCertRenameDlg.show();
        return mCertRenameDlg;
    }

    private void onCreateDelCertDlg() {
        new AlertDialog.Builder(this)
                // .setTitle(R.string.wapi_cert_title_name)
                // .setIcon(android.R.drawable.ic_dialog_alert)
                .setMessage(R.string.text_confirm_del_cert)
                .setPositiveButton(R.string.button_caption_delete,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface Dialog, int id) {
                                Dialog.dismiss();

                                if (mWapiCertStore.deleteCert(mDelCertName)) {
                                    Log.d(TAG, "Delete cert " + mDelCertName
                                            + " success");
                                    removeCertInfoItem(mDelCertName);
                                    // mWapiCertList.clear();
                                    onGetWapiCertList();
                                    adapter.notifyDataSetChanged();
                                } else {
                                    Log.e(TAG, "Delete cert " + mDelCertName
                                            + " faild!");
                                }
                            }
                        })
                .setNegativeButton(R.string.button_caption_cancel,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface Dialog, int id) {
                                Dialog.dismiss();
                            }
                        }).show();
    }

    private void onGetWapiCertList() {
        HashMap<String, Object> map;

        String[] certAliasArray = mWapiCertStore.getCertAliasList();

        if (certAliasArray == null) {
            Log.d(TAG, "getCertListString: certAliasArray is null");
            return;
        }

        mWapiCertList.clear();
        int length = certAliasArray.length;
        Log.d(TAG, "certAliasArray length: " + length);
        for (int i = 0; i < length; i++) {
            String certAlias = certAliasArray[i];
            Log.d(TAG, "cert" + i + "= " + certAlias);
            String certInfo = getCertInfoItem(certAlias);
            if (certInfo.equals("")) {
                continue;
            }
            String[] result = certInfo.split("\t");
            if (result.length < 6) {
                continue;
            }
            map = new HashMap<String, Object>();
            map.put("cert_name", certAlias);
            map.put("type", "X.509 v3");
            map.put("sn", result[0]);
            map.put("issuer", result[1]);
            map.put("subject", result[2]);
            String preValidTo = getString(R.string.text_cert_valid_date);
            map.put("valid_to", preValidTo + result[3]);
            map.put("valid_from", result[4]);
            map.put("authority", result[5]);
            mWapiCertList.add(map);
        }
    }

    public void notifyInstallCert(String userCertName, String userCertPath,
            String caCertPath, byte[] userCert, byte[] priKey, byte[] caCert) {
        mUserCertName = userCertName;
        mUserCertPath = userCertPath;
        mCaCertPath = caCertPath;
        mUserCert = userCert;
        mPriKey = priKey;
        mCaCert = caCert;

        if (mKeyStore.state() != KeyStore.State.UNLOCKED) {
            //Credentials.getInstance().unlock(this);
            Toast.makeText(WapiCertManagerActivity.this,
                    R.string.missing_private_key,
                    Toast.LENGTH_LONG).show();
            return;
        }

        onCreateNameCertDlg();
    }

    public void notifyReopenDlg(String userCertName) {
        onAddWapiCertItem(userCertName);
    }

    private boolean installWapiCert() {
        int ret = 0;
        Log.d(TAG, "Install wapi cert [" + mUserCertAlias + "]");
        if (mWapiCertStore.installCert(mUserCert, mPriKey, mCaCert,
                mUserCertAlias)) {
            Log.d(TAG, "install cert success");
            // save wapi cert info
            if (!saveCertInfoItem(mUserCert, mUserCertAlias)) {
                Log.e(TAG, "save cert info faild");
                return false;
            }
            /*
             * File userCertFile = new File(mUserCertPath); File caCertFile =
             * new File(mCaCertPath); userCertFile.delete();
             * caCertFile.delete();
             */
            return true;
        } else {
            Log.e(TAG, "install cert faild.");
            return false;
        }
    }

    private String getDefaultName() {
        String defName = "";
        if (TextUtils.isEmpty(mUserCertName)) {
            return null;
        } else {
            // remove the extension from the file name
            int index = mUserCertName.lastIndexOf(".");
            if (index > 0)
                defName = mUserCertName.substring(0, index);
            return defName;
        }
    }

    private String getViewText(View view, int viewId) {
        return ((TextView) view.findViewById(viewId)).getText().toString();
    }

    private boolean isCertAliasExist(String alias) {
        String[] certAliasArray = mWapiCertStore.getCertAliasList();
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

    private boolean checkCertAliasFormat(String alias) {
        if (alias == null) {
            return false;
        }

        if (alias.length() >= 64) {
            return false;
        }

        return true;
    }

    private boolean onFindWapiCertFromSDRoot() {
        if (!Environment.getExternalStorageState().equals(
                Environment.MEDIA_MOUNTED)) {
            Log.e(TAG, "No SD card found.");
            Toast.makeText(WapiCertManagerActivity.this,
                    R.string.text_not_access_to_SD, Toast.LENGTH_LONG).show();
            return false;
        }

        File sdRoot = Environment.getExternalStorageDirectory();
        if (sdRoot == null) {
            Log.e(TAG, "sd root file is null.");
            return false;
        }

        try {
            boolean bFind = false;
            File[] fileList = sdRoot.listFiles();
            int length = fileList.length;
            for (int i = 0; i < length; i++) {
                if (WapiCertUtil.isTheSuffix(fileList[i].getAbsoluteFile()
                        .toString(), ".cer")) {
                    byte[] certData = WapiCertUtil.readFile(fileList[i]);
                    if (certData == null) {
                        continue;
                    }
                    byte[] cert = WapiCertUtil.getCertElement(certData);
                    if (cert == null) {
                        continue;
                    }
                    int certType = WapiCertUtil.getCertType(cert);
                    if (certType == WapiCertUtil.CERT_TYPE_WAPI_USER) {
                        // Log.d(TAG, "Found a wapi user cert");
                        bFind = true;
                        break;
                    }
                } else if (WapiCertUtil.isTheSuffix(fileList[i]
                        .getAbsoluteFile().toString(), ".p12")) {
                    // Log.d(TAG, "Found a p12 cert ");
                    bFind = true;
                    break;
                } else {
                    continue;
                }
            }

            if (bFind) {
                return true;
            } else {
                Toast.makeText(this, R.string.text_not_find_valid_cert_in_SD,
                        Toast.LENGTH_LONG).show();
                return false;
            }
        } catch (Exception e) {
            e.printStackTrace();
            return false;
        }
    }

    // save wapi cert info item
    private boolean saveCertInfoItem(byte[] userCert, String alias) {
        String certInfo = mWapiCertStore.getCertInfo(userCert);
        if (certInfo == null || certInfo.equals("")) {
            Log.e(TAG, "saveCertInfoItem: get cert info faild");
            return false;
        }
        Log.d(TAG, "saveCertInfoItem alias: " + alias);
        Log.d(TAG, "saveCertInfoItem certInfo: " + certInfo);
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
        Log.d(TAG, "Clear cert info config file.");
        SharedPreferences settings = getSharedPreferences(
                WapiCertUtil.CERT_INFO_PREFS_NAME, 0);
        SharedPreferences.Editor certInfoCfg = settings.edit();
        certInfoCfg.clear();
        certInfoCfg.commit();
        return true;
    }
}
