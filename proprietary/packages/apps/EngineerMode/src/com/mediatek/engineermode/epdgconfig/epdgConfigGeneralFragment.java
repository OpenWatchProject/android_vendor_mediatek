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
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.os.Bundle;
import android.support.v4.app.Fragment;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.EditText;
import android.widget.ListAdapter;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import java.util.ArrayList;

public class epdgConfigGeneralFragment extends Fragment implements OnItemClickListener {

    private final static int UI_DATA_INVALID = 0;
    private final static int DATA_SET_FAIL = 1;
    private final static int UI_IKE_ESP_INVALID = 2;
    private static int mInvalidPara = -1;
    private static String mSetFailPara = "";
    private static String mErrMsg = "";
    private static boolean mIkeValid = true;
    private static boolean mEspValid = true;
    private static boolean mFirstEntry = true;
    private static boolean mChange = false;
    private String TAG = "epdgConfig/GeneralFragment";
    private ListView mList;
    private EditText mEpdgFgdn;
    private EditText esp_algo_edit;
    private EditText ike_algo_edit;
    private Spinner mIdi;
    private Spinner mIdr;
    private EditText mRetransTo;
    private EditText mRetransTries;
    private EditText mRetransBase;
    private EditText mCustPcscf4;
    private EditText mCustPcscf6;
    private EditText mCustImeiCp;
    private EditText mMtu;
    private TableLayout mTableLayout;
    private Button mSet;
    private String mIkeCmd = "";
    private String mEspCmd = "";
    private String mIkeCmdRsp = "";
    private String mEspCmdRsp = "";
    private String[] mEditCfg = {"epdg_fqdn",
            "retrans_to", "retrans_tries", "retrans_base", "cust_pcscf_4",
            "cust_pcscf_6", "cust_imei_cp", "mtu"};
    private String[] mSpinnerCfg = {"IDi", "IDr"};
    private String[] mListCfg = {"ike_algo", "esp_algo"};
    private Spinner[] mSpinner;
    private EditText[] mEditText;
    private String[] mIkeTitle = {"Encryption", "Integrity", "pseudo_random_function",
            "diffie_hellman_group"};
    private String[] mEspTitle = {"Encryption", "Integrity", "diffie_hellman_group",
            "esn_mode"};
    private String[][] mIkeCfgItem = {{"aes128", "sha1", "prfaesxcbc", "modp768"},
            {"aes192", "sha1_160", "prfmd5", "modp1024"},
            {"aes256", "sha256", "prfsha1", "modp1536"},
            {"aes128ctr", "sha384", "prfsha256", "modp2048"},
            {"aes192ctr", "sha512", "prfsha384", "modp3072"},
            {"aes256ctr", "aesxcbc", "prfsha512", "modp4096"},
            {"des", "md5", null, "modp6144"},
            {"3des", "md5_128", null, "modp8192"},
            {"blowfish", null, null, "modp1027s160"},
            {"null", null, null, "modp2048s224"},
            {null, null, null, "modp2048s256"}};
    private String[][] mEspCfgItem = {{"aes128", "sha1", "modp768", "esn"},
            {"aes192", "sha1_160", "modp1024", "noesn"},
            {"aes256", "sha256", "modp1536", null},
            {"aes128ctr", "sha384", "modp2048", null},
            {"aes192ctr", "sha512", "modp3072", null},
            {"aes256ctr", "aesxcbc", "modp4096", null},
            {"des", "md5", "modp6144", null},
            {"3des", "md5_128", "modp8192", null},
            {"blowfish", null, "modp1027s160", null},
            {"null", null, "modp2048s224", null},
            {null, null, "modp2048s256", null}};
    private boolean[][] mIkeSelected = {{false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
    };
    private boolean[][] mEspSelected = {{false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
            {false, false, false, false},
    };
    private String[] mEditStatus;
    private int[] mSpinnerStatus;
    private boolean mIkeStatus = false;
    private boolean mEspStatus = false;
    private String[] mIkeStrArr = new String[]{"", "", "", ""};
    private String[] mEspStrArr = new String[]{"", "", "", ""};

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Elog.d(TAG, "onCreate");
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {

        View view = inflater.inflate(R.layout.epdg_general, container, false);
        mList = (ListView) view.findViewById(R.id.general_list);
        Elog.d(TAG, "epdgConfigGeneralFragment create");
        ArrayList<String> items = new ArrayList<String>();
        items.add(getString(R.string.ike_algo));
        items.add(getString(R.string.esp_algo));
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(
                getActivity(), android.R.layout.simple_list_item_1, items);
        mList.setAdapter(adapter);
        mList.setOnItemClickListener(this);
        setListViewItemsHeight(mList);

        mEpdgFgdn = (EditText) view.findViewById(R.id.epdg_fgdn);
        esp_algo_edit = (EditText) view.findViewById(R.id.esp_algo_edit);
        ike_algo_edit = (EditText) view.findViewById(R.id.ike_algo_edit);
        mIdi = (Spinner) view.findViewById(R.id.IDi);
        mIdr = (Spinner) view.findViewById(R.id.IDr);
        mRetransTo = (EditText) view.findViewById(R.id.retrans_to);
        mRetransTries = (EditText) view.findViewById(R.id.retrans_tries);
        mRetransBase = (EditText) view.findViewById(R.id.retrans_base);
        mCustPcscf4 = (EditText) view.findViewById(R.id.cust_pcscf_4);
        mCustPcscf6 = (EditText) view.findViewById(R.id.cust_pcscf_6);
        mCustImeiCp = (EditText) view.findViewById(R.id.cust_imei_cp);
        mMtu = (EditText) view.findViewById(R.id.mtu);

        mSpinner = new Spinner[]{mIdi, mIdr};
        mEditText = new EditText[]{mEpdgFgdn, mRetransTo, mRetransTries, mRetransBase,
                mCustPcscf4, mCustPcscf6, mCustImeiCp, mMtu};
        mEditStatus = new String[mEditCfg.length];
        mSpinnerStatus = new int[mSpinnerCfg.length];
        mSet = (Button) view.findViewById(R.id.general_set);
        mSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                mInvalidPara = checkDataValid();
                Elog.d(TAG, "mInvalidPara = " + mInvalidPara);
                if (mInvalidPara >= 0) {
                    if (mInvalidPara < mEditText.length) {
                        showDialog(UI_DATA_INVALID).show();
                    } else {
                        showDialog(UI_IKE_ESP_INVALID).show();
                    }
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
                            mSetFailPara += mSpinnerCfg[i] + ",";
                            Elog.d(TAG, mSetFailPara + " set failed");
                        }
                    }
                }
                for (int i = 0; i < mEditCfg.length; i++) {
                    String set_value = mEditText[i].getText().toString();
                    if ((i == 0) && "".equals(set_value)
                            && !(set_value.equals(mEditStatus[i]))) {
                        if (FeatureSupport.is93Modem()) {
                            set_value = "";
                        } else {
                            set_value = "%EMPTY%";
                        }

                    } else if ("".equals(set_value)
                            || set_value.equals(mEditStatus[i])) {
                        continue;
                    }
                    mChange = true;
                    if (epdgConfig.setCfgValue(mEditCfg[i], set_value)) {
                        Elog.d(TAG, mEditCfg[i] + " set succeed");
                        mEditStatus[i] = set_value;
                    } else {
                        mSetFailPara += mEditCfg[i] + ",";
                        Elog.d(TAG, mSetFailPara + " set failed");
                        break;
                    }

                }
                Elog.d(TAG, "mIkeStatus = " + mIkeStatus);
                String temp_ike = ike_algo_edit.getText().toString();

                if (!temp_ike.equals("")) {
                    if (!temp_ike.equals(mIkeCmd)) {
                        mIkeCmd = mIkeCmdRsp = temp_ike;
                        mIkeStatus = true;
                    } else if (!temp_ike.equals(mIkeCmdRsp)) {
                        mIkeCmd = mIkeCmdRsp = temp_ike;
                        mIkeStatus = true;
                    }
                } else {
                    if (!mIkeCmd.equals("")) {
                        mIkeCmd = temp_ike;
                        mIkeStatus = true;
                    }
                }

                if (mIkeStatus) {
                    if (!mChange) {
                        mChange = true;
                    }

                    if (epdgConfig.setCfgValue(mListCfg[0], mIkeCmd)) {
                        Elog.d(TAG, mListCfg[0] + " set succeed");
                        mIkeStatus = false;
                    } else {
                        mSetFailPara += mListCfg[0] + ",";
                        Elog.d(TAG, mSetFailPara + " set failed");
                        mIkeStatus = true;
                    }
                }
                Elog.d(TAG, "mEspStatus = " + mEspStatus);
                String temp_esp = esp_algo_edit.getText().toString();

                if (!temp_esp.equals("")) {

                    if (!temp_esp.equals(mEspCmd)) {
                        mEspCmd = mEspCmdRsp = temp_esp;
                        mEspStatus = true;
                    } else if (!temp_esp.equals(mEspCmdRsp)) {
                        mEspCmd = mEspCmdRsp = temp_esp;
                        mEspStatus = true;
                    }
                } else {
                    if (!mEspCmd.equals("")) {
                        mEspCmd = temp_esp;
                        mEspStatus = true;
                    }
                }

                if (mEspStatus) {
                    if (!mChange) {
                        mChange = true;
                    }
                    if (epdgConfig.setCfgValue(mListCfg[1], mEspCmd)) {
                        Elog.d(TAG, mListCfg[1] + " set succeed");
                        mEspStatus = false;
                    } else {
                        mSetFailPara += mListCfg[1] + ",";
                        Elog.d(TAG, mSetFailPara + " set failed");
                        mEspStatus = true;
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
    public void onResume() {
        super.onResume();
        Elog.d(TAG, "epdgConfigGeneralFragment onResume");
        getCfg();
    }

    private void setListViewItemsHeight(ListView listview) {
        if (listview == null) {
            return;
        }
        ListAdapter adapter = listview.getAdapter();
        int totalHeight = 0;
        for (int i = 0; i < adapter.getCount(); i++) {
            View itemView = adapter.getView(i, null, listview);
            itemView.measure(0, 0);
            totalHeight += itemView.getMeasuredHeight();
        }
        totalHeight += (adapter.getCount() - 1) * listview.getDividerHeight();
        ViewGroup.LayoutParams params = listview.getLayoutParams();
        params.height = totalHeight;
        listview.setLayoutParams(params);
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {

        for (int i = 0; i < mIkeStrArr.length; i++) {
            mIkeStrArr[i] = "";
            mEspStrArr[i] = "";
        }
        switch (arg2) {
            case 0:
                mIkeCmd = "";
                onCreateDialog(arg2, mIkeSelected).show();
                break;
            case 1:
                mEspCmd = "";
                onCreateDialog(arg2, mEspSelected).show();
                break;
            default:
                break;
        }
    }

    private Dialog onCreateDialog(int id, boolean[][] selected) {
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        LayoutInflater factory = LayoutInflater.from(getActivity());
        final View view = factory.inflate(R.layout.epdg_general_dialog, null);
        builder.setView(view);
        mTableLayout = (TableLayout) view.findViewById(R.id.epdg_table_layout);
        switch (id) {
            case 0:
                builder.setTitle("ike_algo");
                mIkeStatus = false;
                addTableTitle(mIkeTitle);
                for (int i = 0; i < mIkeCfgItem.length; i++) {
                    addTableRow(mIkeCfgItem[i], i, selected);
                }
                builder.setPositiveButton(android.R.string.ok, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (which == DialogInterface.BUTTON_POSITIVE) {
                            mIkeStatus = saveState(mIkeCfgItem, mIkeSelected, 0, mIkeStatus);
                        }
                    }
                });
                break;
            case 1:
                builder.setTitle("esp_algo");
                mEspStatus = false;
                addTableTitle(mEspTitle);
                for (int i = 0; i < mEspCfgItem.length; i++) {
                    addTableRow(mEspCfgItem[i], i, selected);
                }
                builder.setPositiveButton(android.R.string.ok, new OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        if (which == DialogInterface.BUTTON_POSITIVE) {
                            mEspStatus = saveState(mEspCfgItem, mEspSelected, 1, mEspStatus);
                        }
                    }
                });
                break;
            default:
                break;
        }
        return builder.create();
    }

    private void addTableRow(String[] arr, int index, boolean[][] selected) {
        TableRow tableRow = new TableRow(getActivity());
        for (int i = 0; i < arr.length; i++) {
            if (arr[i] != null) {
                CheckBox r1 = new CheckBox(getActivity());
                r1.setText(arr[i]);
                r1.setGravity(Gravity.CENTER);
                // Elog.d(TAG, "addTableRow:selected[" + (index) + "][" + i +
                // "] = "
                // + selected[index][i]);
                if (selected[index][i]) {
                    r1.setChecked(true);
                } else {
                    r1.setChecked(false);
                }
                tableRow.addView(r1);
            } else {
                TextView textnull = new TextView(getActivity());
                textnull.setGravity(Gravity.CENTER);
                tableRow.addView(textnull);
            }
        }
        mTableLayout.addView(tableRow,
                new TableLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT));
    }

    private void addTableTitle(String[] mTitle) {
        TableRow tableRow = new TableRow(getActivity());
        for (int i = 0; i < mTitle.length; i++) {
            TextView title = new TextView(getActivity());
            title.setText(mTitle[i]);
            title.setGravity(Gravity.CENTER);
            tableRow.addView(title);

        }
        mTableLayout.addView(tableRow,
                new TableLayout.LayoutParams(ViewGroup.LayoutParams.FILL_PARENT,
                        ViewGroup.LayoutParams.WRAP_CONTENT));
    }

    private boolean saveState(String[][] item, boolean[][] selected,
                              int mListCmd, boolean status) {
        int len = mIkeCmd.length();
        for (int i = 0; i < item.length + 2; i++) {
            TableRow tableRow = (TableRow) mTableLayout.getChildAt(i);
            for (int j = 0; j < 4; j++) {
                // Elog.d(TAG, "saveState: in for i = " + i);
                if ((tableRow.getChildAt(j) instanceof CheckBox)) {
                    // Elog.d(TAG, "saveState: tableRow is CheckBox");
                } else if ((tableRow.getChildAt(j) instanceof TextView)) {
                    // Elog.d(TAG, "saveState: tableRow is TextView");
                } else {
                    Elog.d(TAG, "saveState: tableRow is others");
                }
                if ((tableRow.getChildAt(j) instanceof CheckBox) && (item[i - 2][j] != null)) {
                    // Elog.d(TAG, "saveState: in if i = " + i);
                    if (((CheckBox) tableRow.getChildAt(j)).isChecked() ^ selected[i - 2][j]) {
                        Elog.d(TAG, "saveState: ^ is true ");
                        selected[i - 2][j] = ((CheckBox) tableRow.getChildAt(j)).isChecked();
                        status = true;
                    }
                    if (selected[i - 2][j]) {
                        if (mListCmd == 0) {
                            mIkeStrArr[j] = mIkeStrArr[j] + item[i - 2][j] + "-";
                        } else if (mListCmd == 1) {
                            mEspStrArr[j] = mEspStrArr[j] + item[i - 2][j] + "-";
                        }
                    }
                }
            }
        }
        if (mListCmd == 0) {

            for (int i = 0; i < mIkeStrArr.length; i++) {
                mIkeCmd = mIkeCmd + mIkeStrArr[i];
            }
            // the length of ike parameter should be less than 256, and keep \0
            // for native code
            if (!mIkeCmd.equals("")) {
                mIkeCmd = mIkeCmd.substring(0, mIkeCmd.length() - 1) + "!";
                Elog.d(TAG, "mIkeCmd1 = " + mIkeCmd);
                if (mIkeCmd.length() > len + 255) {
                    mIkeValid = false;
                } else {
                    mIkeValid = true;
                }
                ike_algo_edit.setText(mIkeCmd);
            } else {
                ike_algo_edit.setText("");
            }

            Elog.d(TAG, "mIkeCmd = " + mIkeCmd);
        } else if (mListCmd == 1) {
            for (int i = 0; i < mIkeStrArr.length; i++) {
                mEspCmd = mEspCmd + mEspStrArr[i];
            }
            // the length of esp parameter should be less than 256, and keep \0
            // for native code

            if (!mEspCmd.equals("")) {
                mEspCmd = mEspCmd.substring(0, mEspCmd.length() - 1) + "!";
                Elog.d(TAG, "mEspCmd1 = " + mEspCmd);
                if (mEspCmd.length() > len + 127) {
                    mEspValid = false;
                } else {
                    mEspValid = true;
                }
                esp_algo_edit.setText(mEspCmd);
            } else {
                esp_algo_edit.setText("");
            }

            Elog.d(TAG, "mEspCmd = " + mEspCmd);
        }
        return status;
    }

    private void getCfg() {
        Elog.d(TAG, "get the currect value of General: ");
        for (int i = 0; i < mEditCfg.length; i++) {
            String response = epdgConfig.getCfgValue(mEditCfg[i]);
            if (response != null) {
                if (response.contains("\"")) {
                    String[] values = response.split("\"");
                    if (values.length > 1) {
                        mEditText[i].setText(values[1]);
                        mEditStatus[i] = values[1];
                    } else {
                        mEditText[i].setText("");
                        mEditStatus[i] = "";
                    }
                } else {
                    mEditText[i].setText(response);
                    mEditStatus[i] = response;
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
            }
        }

        for (int j = 0; j < mIkeCfgItem.length; j++) {
            for (int k = 0; k < mIkeCfgItem[j].length; k++) {
                mIkeSelected[j][k] = false;
            }
        }
        String respValue = epdgConfig.getCfgValue(mListCfg[0]);
        if (respValue != null) {
            String[] getValue;
            if (respValue.contains("\"")) {
                getValue = respValue.split("\"");
            } else {
                getValue = new String[]{"", respValue};
            }
            if (getValue.length > 1) {
                String[] getCfgFirstValue = getValue[1].split(",");

                if (getCfgFirstValue.length > 0) {
                    String cfgFirstValue = getCfgFirstValue[0];
                    mIkeCmdRsp = mIkeCmd = cfgFirstValue;
                    ike_algo_edit.setText(mIkeCmdRsp);
                    if (cfgFirstValue.contains("!")) {
                        cfgFirstValue = cfgFirstValue.substring(0, cfgFirstValue.length() - 1);
                    }
                    final String[] getCfgValue = cfgFirstValue.split("-");
                    for (int i = 0; i < getCfgValue.length; i++) {
                        Elog.d(TAG, "getCfgValue[" + i + "] = " + getCfgValue[i]);
                        for (int j = 0; j < mIkeCfgItem.length; j++) {
                            for (int k = 0; k < mIkeCfgItem[j].length; k++) {
                                if (getCfgValue[i].equals(mIkeCfgItem[j][k])) {
                                    mIkeSelected[j][k] = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

        for (int j = 0; j < mEspCfgItem.length; j++) {
            for (int k = 0; k < mEspCfgItem[j].length; k++) {
                mEspSelected[j][k] = false;
            }
        }

        respValue = epdgConfig.getCfgValue(mListCfg[1]);
        if (respValue != null) {
            String[] getValue;
            if (respValue.contains("\"")) {
                getValue = respValue.split("\"");
            } else {
                getValue = new String[]{"", respValue};
            }
            if (getValue.length > 1) {
                String[] getCfgFirstValue = getValue[1].split(",");
                if (getCfgFirstValue.length > 0) {
                    String cfgFirstValue = getCfgFirstValue[0];
                    mEspCmdRsp = mEspCmd = cfgFirstValue;
                    esp_algo_edit.setText(mEspCmdRsp);
                    if (cfgFirstValue.contains("!")) {
                        cfgFirstValue = cfgFirstValue.substring(0, cfgFirstValue.length() - 1);
                    }
                    final String[] getCfgValue = cfgFirstValue.split("-");
                    for (int i = 0; i < getCfgValue.length; i++) {
                        Elog.d(TAG, "getCfgValue[" + i + "] = " + getCfgValue[i]);
                        for (int j = 0; j < mEspCfgItem.length; j++) {
                            for (int k = 0; k < mEspCfgItem[j].length; k++) {
                                if (getCfgValue[i].equals(mEspCfgItem[j][k])) {
                                    mEspSelected[j][k] = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }

    }

    private int checkDataValid() {
        int ret = -1;

        for (int i = 0; i < mEditText.length; i++) {
            if ((mEditText[i].getText().toString() != null)
                    && ("".equals(mEditText[i].getText().toString()))) {
                if (i != 0)
                    mEditText[i].setText(mEditStatus[i]);
                continue;
            }
            if (i == 0) {
                if (mEditText[i].getText().toString().length() > 255) {
                    ret = i;
                    mErrMsg = "The length of epdg_fqdn should be less than 255";
                    break;
                }
            } else {
                long data = 0;
                try {
                    data = Long.valueOf(mEditText[i].getText().toString());
                } catch (NumberFormatException e) {
                    data = -1;
                }
                Elog.d(TAG, "data = " + data);
                if (i == 1) {
                    if (data < 0 || data > 4294967294L) {
                        ret = i;
                        mErrMsg = "The range is 0 ~ 4294967294.";
                        return i;
                    }
                } else if ((i == 2) || (i == 3)) {
                    if ((data < 0) || (data > 254)) {
                        ret = i;
                        mErrMsg = "The range is 0 ~ 254.";
                        return i;
                    }
                } else {
                    if ((data < 0) || (data > 65534)) {
                        ret = i;
                        mErrMsg = "The range is 0 ~ 65534.";
                        return i;
                    }
                }
                if (i > 0) {
                    mEditText[i].setText(String.valueOf(data));
                }
            }
        }
        if (!mIkeValid || ike_algo_edit.getText().toString().length() > 255) {
            ret = 1000;
            mErrMsg = "Check too much items. not allow > 255";
            return ret;
        }
        if (!mEspValid || esp_algo_edit.getText().toString().length() > 255) {
            ret = 1001;
            mErrMsg = "Check too much items.not allow > 255";
            return ret;
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
            case UI_IKE_ESP_INVALID:
                return new AlertDialog.Builder(getActivity())
                        .setTitle(
                                "Check")
                        .setMessage(
                                "The input of " + mListCfg[mInvalidPara - 1000] + " is invalid! "
                                        + mErrMsg)
                        .setPositiveButton("OK", null).create();
        }
        return null;
    }

}
