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

package com.mediatek.engineermode.mdmdiagnosticinfo;

import android.app.Activity;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout.LayoutParams;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class MDMComponentSelectActivity extends Activity implements OnCheckedChangeListener,
        MDMCoreOperation.IDataTransaction {
    private static final String COMPONENT_RSRPSINR_CONFIG_SHAREPRE=
        "telephony_rsrpsinr_config_settings";
    private static final String TAG = "EM_DiagnosticMetric_Info";
    private static final String PREF_FILE = "pref_file";
    private static final String KEY_SELECTED = "selected";
    private static final int CHECK_INFOMATION_ID = Menu.FIRST; // only use Menu.FIRST
    private static final int SELECT_ALL_ID = Menu.FIRST + 1;
    private static final int SELECT_NONE_ID = Menu.FIRST + 2;
    private List<CheckBox> mCheckBoxes;
    public static int mSimType;
    private SharedPreferences mPref;
    private Set<String> mSelected;
    private List<MdmBaseComponent> allComponents;
    private List<MdmBaseComponent> components = new ArrayList<MdmBaseComponent>();
    private ProgressDialog mdmInitProgressDialog;
    private ProgressDialog mdmSubscribeProgressDialog;
    private static final int DIALOG_WAIT_INIT = 0;
    private static final int DIALOG_WAIT_SUBSCRIB = 1;
    private static MDMCoreOperation mdmInstance;
    private MDMCoreOperation.IDataTransaction dataTransaction;

    private static int k = 1;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        InitUI();
        Elog.d(TAG, this.getClass().getSimpleName() + " onCreate");
        showDialog(DIALOG_WAIT_INIT);
        dataTransaction = this;
        getMdmInstance().setOnTransactionListener(this);
        getMdmInstance().mdmlLoading(this);
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        Elog.d(TAG, this.getClass().getSimpleName() + " onDestroy");
        mdmInstance.mdmlClosing();
        destroyMdmInstance();
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
        case DIALOG_WAIT_INIT:
            Elog.d(TAG, "Wait MDML init");
            mdmInitProgressDialog = new ProgressDialog(this);
            mdmInitProgressDialog.setTitle("Waiting");
            mdmInitProgressDialog.setMessage("Wait MDML init");
            mdmInitProgressDialog.setCancelable(false);
            mdmInitProgressDialog.setIndeterminate(true);
            return mdmInitProgressDialog;
        case DIALOG_WAIT_SUBSCRIB:
            Elog.d(TAG, "Before Wait subscribe message..");
            mdmSubscribeProgressDialog = new ProgressDialog(this);
            mdmSubscribeProgressDialog.setTitle("Waiting");
            mdmSubscribeProgressDialog.setMessage("Wait subscribe message..");
            mdmSubscribeProgressDialog.setCancelable(false);
            mdmSubscribeProgressDialog.setIndeterminate(true);
            return mdmSubscribeProgressDialog;
        default:
            return super.onCreateDialog(id);
        }
    }

    private void InitUI() {
        setContentView(R.layout.component_select);
        ViewGroup list = (ViewGroup) findViewById(R.id.list);
        LayoutParams param = new LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
        param.setMargins(0, 5, 0, 5);
        LayoutParams param2 = new LayoutParams(
                LayoutParams.MATCH_PARENT, LayoutParams.WRAP_CONTENT);
        param2.setMargins(30, 5, 0, 5);

        Intent intent = getIntent();
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);

        mPref = getSharedPreferences(PREF_FILE, MODE_PRIVATE);
        mSelected = mPref.getStringSet(KEY_SELECTED, null);
        if (mSelected == null) {
            mSelected = new HashSet<String>();
        }

        allComponents = MdmComponentUtils.getComponents(this);
        MdmBaseComponent[] tmp = allComponents.toArray(new MdmBaseComponent[allComponents.size()]);
//        Arrays.sort(tmp);
//        allComponents = Arrays.asList(tmp);

        List<String> groups = new ArrayList<String>();
        for (MdmBaseComponent c : allComponents) {
            if (!groups.contains(c.getGroup())) {
                groups.add(c.getGroup());
            }
        }
        String[] groupName = groups.toArray(new String[groups.size()]);
        Arrays.sort(groupName);
        groups = Arrays.asList(groupName);

        mCheckBoxes = new ArrayList<CheckBox>();
        for (String g : groups) {
            CheckBox checkBox = new CheckBox(this);
            checkBox.setText(g);
            ArrayList<CheckBox> groupComponents = new ArrayList<CheckBox>();
            checkBox.setTag(groupComponents);
            checkBox.setOnCheckedChangeListener(this);
            checkBox.setChecked(mSelected.contains(g));
            list.addView(checkBox, param);
            mCheckBoxes.add(checkBox);

            for (MdmBaseComponent c : allComponents) {
                if (mSimType == PhoneConstants.SIM_ID_2 && (!c.supportMultiSIM())) {
                    continue;
                }
                if (c.getGroup().equals(g)) {
                    checkBox = new CheckBox(this);
                    checkBox.setText(c.getName());
                    checkBox.setTag(c);
                    checkBox.setOnCheckedChangeListener(this);
                    checkBox.setChecked(mSelected.contains(c.getName()));
                    list.addView(checkBox, param2);
                    mCheckBoxes.add(checkBox);
                    groupComponents.add(checkBox);
                }
            }
        }
    }

    @Override
    public List<MdmBaseComponent> onItemSelect(){
        return (ArrayList<MdmBaseComponent>) components;
    }

    @Override
    public void onUpdateUI(int msg_id){
        switch (msg_id) {
        case MDMCoreOperation.LOADING_DONE: {
            removeDialog(DIALOG_WAIT_INIT);
            Elog.d(TAG, "Wait MDML init done");
            break;
        }
        case MDMCoreOperation.SUBSCRIBE_DONE: {
            removeDialog(DIALOG_WAIT_SUBSCRIB);
            Elog.d(TAG, "Wait subscribe message done");
            k = 1;
            Intent intent = new Intent(MDMComponentSelectActivity.this,
                MDMComponentDetailActivity.class);
            intent.putExtra("mSimType", mSimType);
            MDMComponentSelectActivity.this.startActivity(intent);
            break;
        }
        case MDMCoreOperation.UPDATE_UI: {
            mdmSubscribeProgressDialog.setMessage("Wait subscribe message:" + (k++));
            break;
        }
        default:
            break;
       }
    }

    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (buttonView.getTag() != null && buttonView.getTag() instanceof ArrayList) {
            ArrayList<CheckBox> groupComponents = (ArrayList<CheckBox>) buttonView.getTag();
            for (CheckBox c : groupComponents) {
                c.setChecked(isChecked);
            }
        }
        if (isChecked) {
            mSelected.add(buttonView.getText().toString());
        } else {
            mSelected.remove(buttonView.getText().toString());
        }
        mPref.edit().putStringSet(KEY_SELECTED, mSelected).commit();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        menu.add(0, CHECK_INFOMATION_ID, 0, getString(R.string.networkinfo_check));
        menu.add(0, SELECT_ALL_ID, 0, "Select All");
        menu.add(0, SELECT_NONE_ID, 0, "Select None");
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
        case CHECK_INFOMATION_ID:
            components.clear();
            boolean bNeedShowConfig = false;
            for (int i = 0; i < mCheckBoxes.size(); i++) {
                CheckBox c = mCheckBoxes.get(i);
                if (c.isChecked() &&
                        c.getTag() instanceof MdmBaseComponent &&
                        !components.contains((MdmBaseComponent) c.getTag())) {
                    components.add((MdmBaseComponent) c.getTag());
                }
            }
            if (components.size() > 0) {
                Elog.d(TAG, "check infomation id");
                showDialog(DIALOG_WAIT_SUBSCRIB);
                getMdmInstance().mdmlSubscribe();
            } else {
                Toast.makeText(this, getString(R.string.networkinfo_msg),
                               Toast.LENGTH_LONG).show();
            }
            break;
        case SELECT_ALL_ID:
            for (CheckBox c : mCheckBoxes) {
                c.setChecked(true);
            }
            break;
        case SELECT_NONE_ID:
            for (CheckBox c : mCheckBoxes) {
                c.setChecked(false);
            }
            break;
        default:
            break;
        }
        return super.onOptionsItemSelected(aMenuItem);
    }

    public static MDMCoreOperation getMdmInstance() {
        if(mdmInstance == null) {
            createMdmInstance();
        }
        return mdmInstance;
    }

    public static void createMdmInstance() {
        if(mdmInstance == null) {
            mdmInstance = new MDMCoreOperation();
        }
    }

    public static void destroyMdmInstance() {
        if(mdmInstance != null) {
            mdmInstance = null;
        }
    }
}
