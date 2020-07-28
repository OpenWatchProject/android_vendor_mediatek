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

package com.mediatek.engineermode.mdmcomponent;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.os.SystemService;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.EditText;
import android.widget.LinearLayout.LayoutParams;
import android.widget.Toast;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.R;

import com.mediatek.mdml.MONITOR_CMD_RESP;
import com.mediatek.mdml.MonitorCmdProxy;
import com.mediatek.mdml.MonitorTrapReceiver;
import com.mediatek.mdml.Msg;
import com.mediatek.mdml.PlainDataDecoder;
import com.mediatek.mdml.TRAP_TYPE;
import com.mediatek.mdml.TrapHandlerInterface;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.concurrent.TimeoutException;

import static com.mediatek.mdml.MONITOR_CMD_RESP.*;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_EM;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_EVENT;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_RECORD;


public class ComponentSelectActivity extends Activity implements OnCheckedChangeListener {
    public static final int MSG_UPDATE_UI_URC_EM = 0;
    public static final int MSG_UPDATE_UI_URC_ICD = 1;
    private static final String COMPONENT_RSRPSINR_CONFIG_SHAREPRE =
            "telephony_rsrpsinr_config_settings";
    private static final String TAG = "EmInfo/ComponentSelectActivity";
    private static final String PREF_FILE = "pref_file";
    private static final String KEY_SELECTED = "selected";
    private static final int CHECK_INFOMATION_ID = Menu.FIRST; // only use Menu.FIRST
    private static final int SELECT_ALL_ID = Menu.FIRST + 1;
    private static final int SELECT_NONE_ID = Menu.FIRST + 2;
    private static final int UPDATE_INTERVAL = 500;
    private static final int DIALOG_WAIT_INIT = 0;
    private static final int DIALOG_WAIT_SUBSCRIB = 1;
    private static final String SERVICE_NAME = "md_monitor";
    private static final int WAIT_TIMEOUT = 3000;
    /* Trap Info */
    private final static int LOADING_DONE = 1;
    private final static int SUBSCRIBE_DONE = 2;
    private final static int UPDATE_UI = 3;
    private static final int STRONG_POINT1_X_DEFAULT = -80;
    private static final int STRONG_POINT1_Y_DEFAULT = 30;
    private static final int STRONG_POINT2_X_DEFAULT = -80;
    private static final int STRONG_POINT2_Y_DEFAULT = 20;
    private static final int STRONG_POINT3_X_DEFAULT = -30;
    private static final int STRONG_POINT3_Y_DEFAULT = 20;
    private static final int MEDIUMWEAK_POINT1_X_DEFAULT = -140;
    private static final int MEDIUMWEAK_POINT1_Y_DEFAULT = 10;
    private static final int MEDIUMWEAK_POINT2_X_DEFAULT = -90;
    private static final int MEDIUMWEAK_POINT2_Y_DEFAULT = 10;
    private static final int MEDIUMWEAK_POINT3_X_DEFAULT = -90;
    private static final int MEDIUMWEAK_POINT3_Y_DEFAULT = -20;
    private static final int WEAK_POINT1_X_DEFAULT = -140;
    private static final int WEAK_POINT1_Y_DEFAULT = 5;
    private static final int WEAK_POINT2_X_DEFAULT = -100;
    private static final int WEAK_POINT2_Y_DEFAULT = 5;
    private static final int WEAK_POINT3_X_DEFAULT = -100;
    private static final int WEAK_POINT3_Y_DEFAULT = -20;
    public static MonitorCmdProxy m_cmdProxy;
    public static long m_sid;
    public static int mSimType;
    public static int mModemType;
    public static String mAutoRecordFlag = "0";
    public static String mAutoRecordFlagItem = "0";
    public static String[] mAutoRecordItem =
            {"Pcell and Scell basic info",
                    "Handover (Intra-LTE)",
                    "LTE ERRC OOS Event Info",
                    "LTE ERRC ELF Event Info",
                    "LTE EMAC Rach Failure Info",
                    "MD Feature Detection"
            };
    public static Set<String> mSelected_show_both_sim;
    public static ArrayList<Long> mCheckedEmTypes = new ArrayList<Long>();
    public static ArrayList<Long> mCheckedIcdRecordTypes = new ArrayList<Long>();
    public static ArrayList<Long> mCheckedIcdEventTypes = new ArrayList<Long>();
    private static int k = 1;
    private static int mStrongPoint1X = -80;
    private static int mStrongPoint1Y = 30;
    private static int mStrongPoint2X = -80;
    private static int mStrongPoint2Y = 20;
    private static int mStrongPoint3X = -30;
    private static int mStrongPoint3Y = 20;
    private static int mMediumWeakPoint1X = -140;
    private static int mMediumWeakPoint1Y = 10;
    private static int mMediumWeakPoint2X = -90;
    private static int mMediumWeakPoint2Y = 10;
    private static int mMediumWeakPoint3X = -90;
    private static int mMediumWeakPoint3Y = -20;
    private static int mWeakPoint1X = -140;
    private static int mWeakPoint1Y = 5;
    private static int mWeakPoint2X = -100;
    private static int mWeakPoint2Y = 5;
    private static int mWeakPoint3X = -100;
    private static int mWeakPoint3Y = -20;
    private static PlainDataDecoder m_plainDataDecoder = null;
    MonitorTrapReceiver m_trapReceiver;
    ArrayList<String> components = null;
    private int mSelectSim = 0;
    private List<CheckBox> mCheckBoxes;
    private SharedPreferences mPref;
    private Set<String> mSelected;
    private List<MDMComponent> allComponents;
    private ProgressDialog mProgressDialog;
    private ProgressDialog mProgressDialog1;
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case LOADING_DONE: {
                    removeDialog(DIALOG_WAIT_INIT);
                    Elog.d(TAG, "Wait MDML init done");
                    if ("1".equals(mAutoRecordFlag)) {
                        OptionsItemSelected();
                    }
                    break;
                }
                case SUBSCRIBE_DONE: {
                    removeDialog(DIALOG_WAIT_SUBSCRIB);
                    Elog.d(TAG, "Wait subscribe message done");
                    k = 1;
                    Intent intent = new Intent(ComponentSelectActivity.this,
                            MDMComponentDetailActivity.class);
                    intent.putExtra("mChecked", components);
                    intent.putExtra("mSimType", mSimType);
                    intent.putExtra("mModemType", mModemType);
                    ComponentSelectActivity.this.startActivity(intent);
                    break;
                }
                case UPDATE_UI: {
                    mProgressDialog1.setMessage("Wait subscribe message:" + (k++));
                    break;
                }
                default:
                    break;
            }
        }
    };

    public static MonitorCmdProxy getCmdProxy(Context context) {
        int i = 0;
        if (m_cmdProxy == null) {
            try {
                m_cmdProxy = new MonitorCmdProxy(context);
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
        return m_cmdProxy;
    }

    public static PlainDataDecoder getDecoder(Context context) {
        if (m_plainDataDecoder == null) {
            try {
                m_plainDataDecoder = PlainDataDecoder.getInstance(null, context);
            } catch (Exception ex) {
                ex.printStackTrace();
            }
        }
        return m_plainDataDecoder;
    }

    public static long[] toPrimitives(Long... objects) {
        long[] primitives = new long[objects.length];
        for (int i = 0; i < objects.length; i++)
            primitives[i] = objects[i];
        return primitives;
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        InitUI();
        Elog.d(TAG, "Init ComponentSelectActivity UI done");
        showDialog(DIALOG_WAIT_INIT);
        new LoadingTask().execute();
    }

    @Override
    protected void onResume() {
        super.onResume();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        new deleteTask().execute();
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
            case DIALOG_WAIT_INIT:
                Elog.d(TAG, "Wait MDML init");
                mProgressDialog = new ProgressDialog(this);
                mProgressDialog.setTitle("Waiting");
                mProgressDialog.setMessage("Wait MDML init");
                mProgressDialog.setCancelable(false);
                mProgressDialog.setIndeterminate(true);
                return mProgressDialog;
            case DIALOG_WAIT_SUBSCRIB:
                Elog.d(TAG, "Before Wait subscribe message..");
                mProgressDialog1 = new ProgressDialog(this);
                mProgressDialog1.setTitle("Waiting");
                mProgressDialog1.setMessage("Wait subscribe message..");
                mProgressDialog1.setCancelable(false);
                mProgressDialog1.setIndeterminate(true);
                return mProgressDialog1;
            default:
                return super.onCreateDialog(id);
        }
    }

    private void StartMDLService() {
        boolean isRunning = SystemService.isRunning(SERVICE_NAME);
        if (isRunning == false) {
            Elog.v(TAG, "start md_monitor prop");
            SystemProperties.set("ctl.start","md_monitor");
            try {
                SystemService.waitForState
                        (SERVICE_NAME, SystemService.State.RUNNING, WAIT_TIMEOUT);
            } catch (TimeoutException e) {
                e.printStackTrace();
            }
            isRunning = SystemService.isRunning(SERVICE_NAME);
            if (isRunning == false) {
                Elog.e(TAG, "start md_monitor failed time out");
            }
        }
    }

    private void InitMDML(Context context) {
        m_cmdProxy = getCmdProxy(context);
        m_sid = m_cmdProxy.onCreateSession();
        m_trapReceiver = new MonitorTrapReceiver(m_sid, "demo_receiver");
        m_trapReceiver.SetTrapHandler(new DemoHandler());
        m_cmdProxy.onSetTrapReceiver(m_sid, "demo_receiver");
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
        mModemType = intent.getIntExtra("mModemType", MDMSimSelectActivity.MODEM_PROTOCOL_1);
        mSelectSim = mSimType;
        mPref = getSharedPreferences(PREF_FILE, MODE_PRIVATE);
        mSelected = mPref.getStringSet(KEY_SELECTED, null);
        if (mSelected == null) {
            mSelected = new HashSet<String>();

        }
        if (mSelected_show_both_sim == null) {
            mSelected_show_both_sim = new HashSet<String>();
            mSelected_show_both_sim.clear();
        }
        if (FeatureSupport.isEngLoad() || FeatureSupport.isUserDebugLoad()) {
            mAutoRecordFlag = EmUtils.systemPropertyGet("persist.radio.record.auto", "0");
            mAutoRecordFlagItem = EmUtils.systemPropertyGet("persist.radio.record.auto.item",
                    "0");
            mSelected_show_both_sim.clear();
        }
        if ("1".equals(mAutoRecordFlag)) {
            mSelected.clear();
            if (mAutoRecordFlagItem.equals("0")) {
                for (int i = 0; i < mAutoRecordItem.length; i++) {
                    mSelected.add(mAutoRecordItem[i]);
                }
            } else {
                String autoRecordFlagItem[] = mAutoRecordFlagItem.split(",");
                for (int i = 0; autoRecordFlagItem != null && i < autoRecordFlagItem.length; i++) {
                    if (autoRecordFlagItem[i].equals("1")) {
                        mSelected.add(mAutoRecordItem[i]);
                    }
                }
            }
            mSelected_show_both_sim.addAll(mSelected);
            Elog.d(TAG, "mSelected = " + mSelected);
        }
        mSelected_show_both_sim.add("FT Network Info");

        allComponents = MDMComponent.getComponents(this);
        MDMComponent[] tmp = allComponents.toArray(new MDMComponent[allComponents.size()]);
        Arrays.sort(tmp);
        allComponents = Arrays.asList(tmp);

        List<String> groups = new ArrayList<String>();
        for (MDMComponent c : allComponents) {
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

            for (MDMComponent c : allComponents) {
                if ((mModemType == MDMSimSelectActivity.MODEM_PROTOCOL_2
                        || mModemType == MDMSimSelectActivity.MODEM_PROTOCOL_3)
                        && (!c.supportMultiSIM())) {
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

    private void InitDecoder(Context context) {
        m_plainDataDecoder = getDecoder(context);
    }

    /* decoder */

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

    void OptionsItemSelected() {
        ArrayList<String> components = new ArrayList<String>();
        boolean bNeedShowConfig = false;
        boolean bNeedShowSimConfig = false;
        for (int i = 0; i < mCheckBoxes.size(); i++) {
            CheckBox c = mCheckBoxes.get(i);
            if (c.isChecked() && c.getTag() instanceof MDMComponent) {
                components.add(c.getText().toString());
                if (c.getText().toString().equals("CC0/CC1 RSRP and SINR")) {
                    bNeedShowConfig = true;
                }
                if (c.getText().toString().equals("FT Network Info")) {
                    bNeedShowSimConfig = true;
                }
            }
        }
        if (bNeedShowConfig) {
            showRsrpSinrConfigDlg(components, bNeedShowSimConfig);
        } else if (bNeedShowSimConfig) {
            showSimSelectedDialog(getResources().getStringArray(R.array.sim_display_info));
        } else {
            if (components.size() > 0) {
                Elog.d(TAG, "check infomation id");
                showDialog(DIALOG_WAIT_SUBSCRIB);
                new SubscribeTask().execute();
            } else {
                Toast.makeText(this, getString(R.string.networkinfo_msg),
                        Toast.LENGTH_LONG).show();
            }
        }
    }

    private void showSimSelectedDialog(String[] items) {
        // TODO Auto-generated method stub
        Dialog myDialog = new AlertDialog.Builder(this)
                .setTitle("Sim Card for FT Network Info")
                .setSingleChoiceItems(items, mSelectSim, new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        mSelectSim = which;
                        Elog.d(TAG, "Do Select Sim Action, Select Sim " + which);
                    }
                })
                .setPositiveButton("OK", new DialogInterface.OnClickListener() {
                    @Override
                    public void onClick(DialogInterface dialog, int which) {
                        // TODO Auto-generated method stub
                        showDialog(DIALOG_WAIT_SUBSCRIB);
                        new SubscribeTask().execute();
                    }
                })
                .show();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem aMenuItem) {
        switch (aMenuItem.getItemId()) {
            case CHECK_INFOMATION_ID:
                OptionsItemSelected();
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

    private void showMultiInputDlg(CharSequence title, final InputDialogOnClickListener listener) {
        View inputView = getLayoutInflater().inflate(R.layout.em_multi_input_layout, null);
        Button okBtn = (Button) inputView.findViewById(R.id.em_multi_input_ok_btn);
        Button cancelBtn = (Button) inputView.findViewById(R.id.em_multi_input_cancel_btn);
        final EditText strongPoint1XEdit = (EditText) inputView.findViewById(
                R.id.strong_point1_x_edit);
        final EditText strongPoint1YEdit = (EditText) inputView.findViewById(
                R.id.strong_point1_y_edit);
        final EditText strongPoint2XEdit = (EditText) inputView.findViewById(
                R.id.strong_point2_x_edit);
        final EditText strongPoint2YEdit = (EditText) inputView.findViewById(
                R.id.strong_point2_y_edit);
        final EditText strongPoint3XEdit = (EditText) inputView.findViewById(
                R.id.strong_point3_x_edit);
        final EditText strongPoint3YEdit = (EditText) inputView.findViewById(
                R.id.strong_point3_y_edit);
        final EditText mediumweakPoint1XEdit = (EditText) inputView.findViewById(
                R.id.mediumweak_point1_x_edit);
        final EditText mediumweakPoint1YEdit = (EditText) inputView.findViewById(
                R.id.mediumweak_point1_y_edit);
        final EditText mediumweakPoint2XEdit = (EditText) inputView.findViewById(
                R.id.mediumweak_point2_x_edit);
        final EditText mediumweakPoint2YEdit = (EditText) inputView.findViewById(
                R.id.mediumweak_point2_y_edit);
        final EditText mediumweakPoint3XEdit = (EditText) inputView.findViewById(
                R.id.mediumweak_point3_x_edit);
        final EditText mediumweakPoint3YEdit = (EditText) inputView.findViewById(
                R.id.mediumweak_point3_y_edit);
        final EditText weakPoint1XEdit = (EditText) inputView.findViewById(
                R.id.weak_point1_x_edit);
        final EditText weakPoint1YEdit = (EditText) inputView.findViewById(
                R.id.weak_point1_y_edit);
        final EditText weakPoint2XEdit = (EditText) inputView.findViewById(
                R.id.weak_point2_x_edit);
        final EditText weakPoint2YEdit = (EditText) inputView.findViewById(
                R.id.weak_point2_y_edit);
        final EditText weakPoint3XEdit = (EditText) inputView.findViewById(
                R.id.weak_point3_x_edit);
        final EditText weakPoint3YEdit = (EditText) inputView.findViewById(
                R.id.weak_point3_y_edit);

        final SharedPreferences rsrpSinrConfigSh = getSharedPreferences(
                COMPONENT_RSRPSINR_CONFIG_SHAREPRE, android.content.Context.MODE_PRIVATE);
        int strongPoint1X = rsrpSinrConfigSh.getInt(getString(
                R.string.strong_point_1x), STRONG_POINT1_X_DEFAULT);
        int strongPoint1Y = rsrpSinrConfigSh.getInt(getString(
                R.string.strong_point_1y), STRONG_POINT1_Y_DEFAULT);
        int strongPoint2X = rsrpSinrConfigSh.getInt(getString(
                R.string.strong_point_2x), STRONG_POINT2_X_DEFAULT);
        int strongPoint2Y = rsrpSinrConfigSh.getInt(getString(
                R.string.strong_point_2y), STRONG_POINT2_Y_DEFAULT);
        int strongPoint3X = rsrpSinrConfigSh.getInt(getString(
                R.string.strong_point_3x), STRONG_POINT3_X_DEFAULT);
        int strongPoint3Y = rsrpSinrConfigSh.getInt(getString(
                R.string.strong_point_3y), STRONG_POINT3_Y_DEFAULT);
        int mediumWeakPoint1X = rsrpSinrConfigSh.getInt(getString(
                R.string.mediumweak_point_1x), MEDIUMWEAK_POINT1_X_DEFAULT);
        int mediumWeakPoint1Y = rsrpSinrConfigSh.getInt(getString(
                R.string.mediumweak_point_1y), MEDIUMWEAK_POINT1_Y_DEFAULT);
        int mediumWeakPoint2X = rsrpSinrConfigSh.getInt(getString(
                R.string.mediumweak_point_2x), MEDIUMWEAK_POINT2_X_DEFAULT);
        int mediumWeakPoint2Y = rsrpSinrConfigSh.getInt(getString(
                R.string.mediumweak_point_2y), MEDIUMWEAK_POINT2_Y_DEFAULT);
        int mediumWeakPoint3X = rsrpSinrConfigSh.getInt(getString(
                R.string.mediumweak_point_3x), MEDIUMWEAK_POINT3_X_DEFAULT);
        int mediumWeakPoint3Y = rsrpSinrConfigSh.getInt(getString(
                R.string.mediumweak_point_3y), MEDIUMWEAK_POINT3_Y_DEFAULT);
        int weakPoint1X = rsrpSinrConfigSh.getInt(getString(
                R.string.weak_point_1x), WEAK_POINT1_X_DEFAULT);
        int weakPoint1Y = rsrpSinrConfigSh.getInt(getString(
                R.string.weak_point_1y), WEAK_POINT1_Y_DEFAULT);
        int weakPoint2X = rsrpSinrConfigSh.getInt(getString(
                R.string.weak_point_2x), WEAK_POINT2_X_DEFAULT);
        int weakPoint2Y = rsrpSinrConfigSh.getInt(getString(
                R.string.weak_point_2y), WEAK_POINT2_Y_DEFAULT);
        int weakPoint3X = rsrpSinrConfigSh.getInt(getString(
                R.string.weak_point_3x), WEAK_POINT3_X_DEFAULT);
        int weakPoint3Y = rsrpSinrConfigSh.getInt(getString(
                R.string.weak_point_3y), WEAK_POINT3_Y_DEFAULT);

        strongPoint1XEdit.setText(String.valueOf(strongPoint1X));
        strongPoint1YEdit.setText(String.valueOf(strongPoint1Y));
        strongPoint2XEdit.setText(String.valueOf(strongPoint2X));
        strongPoint2YEdit.setText(String.valueOf(strongPoint2Y));
        strongPoint3XEdit.setText(String.valueOf(strongPoint3X));
        strongPoint3YEdit.setText(String.valueOf(strongPoint3Y));
        mediumweakPoint1XEdit.setText(String.valueOf(mediumWeakPoint1X));
        mediumweakPoint1YEdit.setText(String.valueOf(mediumWeakPoint1Y));
        mediumweakPoint2XEdit.setText(String.valueOf(mediumWeakPoint2X));
        mediumweakPoint2YEdit.setText(String.valueOf(mediumWeakPoint2Y));
        mediumweakPoint3XEdit.setText(String.valueOf(mediumWeakPoint3X));
        mediumweakPoint3YEdit.setText(String.valueOf(mediumWeakPoint3Y));
        weakPoint1XEdit.setText(String.valueOf(weakPoint1X));
        weakPoint1YEdit.setText(String.valueOf(weakPoint1Y));
        weakPoint2XEdit.setText(String.valueOf(weakPoint2X));
        weakPoint2YEdit.setText(String.valueOf(weakPoint2Y));
        weakPoint3XEdit.setText(String.valueOf(weakPoint3X));
        weakPoint3YEdit.setText(String.valueOf(weakPoint3Y));

        final AlertDialog dialog = new AlertDialog.Builder(this).setCancelable(
                false).setTitle(title).setView(inputView).create();
        okBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                listener.onClick(dialog, DialogInterface.BUTTON_POSITIVE,
                        strongPoint1XEdit.getText().toString(),
                        strongPoint1YEdit.getText().toString(),
                        strongPoint2XEdit.getText().toString(),
                        strongPoint2YEdit.getText().toString(),
                        strongPoint3XEdit.getText().toString(),
                        strongPoint3YEdit.getText().toString(),
                        mediumweakPoint1XEdit.getText().toString(),
                        mediumweakPoint1YEdit.getText().toString(),
                        mediumweakPoint2XEdit.getText().toString(),
                        mediumweakPoint2YEdit.getText().toString(),
                        mediumweakPoint3XEdit.getText().toString(),
                        mediumweakPoint3YEdit.getText().toString(),
                        weakPoint1XEdit.getText().toString(),
                        weakPoint1YEdit.getText().toString(),
                        weakPoint2XEdit.getText().toString(),
                        weakPoint2YEdit.getText().toString(),
                        weakPoint3XEdit.getText().toString(),
                        weakPoint3YEdit.getText().toString());
            }
        });

        cancelBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                listener.onClick(dialog, DialogInterface.BUTTON_NEGATIVE,
                        strongPoint1XEdit.getText().toString(),
                        strongPoint1YEdit.getText().toString(),
                        strongPoint2XEdit.getText().toString(),
                        strongPoint2YEdit.getText().toString(),
                        strongPoint3XEdit.getText().toString(),
                        strongPoint3YEdit.getText().toString(),
                        mediumweakPoint1XEdit.getText().toString(),
                        mediumweakPoint1YEdit.getText().toString(),
                        mediumweakPoint2XEdit.getText().toString(),
                        mediumweakPoint2YEdit.getText().toString(),
                        mediumweakPoint3XEdit.getText().toString(),
                        mediumweakPoint3YEdit.getText().toString(),
                        weakPoint1XEdit.getText().toString(),
                        weakPoint1YEdit.getText().toString(),
                        weakPoint2XEdit.getText().toString(),
                        weakPoint2YEdit.getText().toString(),
                        weakPoint3XEdit.getText().toString(),
                        weakPoint3YEdit.getText().toString());
            }
        });
        dialog.show();
    }

    private void showRsrpSinrConfigDlg(final ArrayList<String> componentPara, final boolean
            bNeedShowSimConfig) {
        String title = getString(R.string.rsrp_sinr_config_input_tip);
        showMultiInputDlg(title, new InputDialogOnClickListener() {
            @Override
            public void onClick(DialogInterface dialog, int which, String inputStrong1XStr,
                                String inputStrong1YStr, String inputStrong2XStr, String
                                        inputStrong2YStr,
                                String inputStrong3XStr, String inputStrong3YStr, String
                                        inputMediumWeak1XStr,
                                String inputMediumWeak1YStr, String inputMediumWeak2XStr, String
                                        inputMediumWeak2YStr,
                                String inputMediumWeak3XStr, String inputMediumWeak3YStr, String
                                        inputWeak1XStr,
                                String inputWeak1YStr, String inputWeak2XStr, String inputWeak2YStr,
                                String inputWeak3XStr,
                                String inputWeak3YStr) {
                if (which == DialogInterface.BUTTON_POSITIVE) {
                    boolean validInput = true;
                    try {
                        mStrongPoint1X = Integer.parseInt(inputStrong1XStr);
                        mStrongPoint1Y = Integer.parseInt(inputStrong1YStr);
                        mStrongPoint2X = Integer.parseInt(inputStrong2XStr);
                        mStrongPoint2Y = Integer.parseInt(inputStrong2YStr);
                        mStrongPoint3X = Integer.parseInt(inputStrong3XStr);
                        mStrongPoint3Y = Integer.parseInt(inputStrong3YStr);
                        mMediumWeakPoint1X = Integer.parseInt(inputMediumWeak1XStr);
                        mMediumWeakPoint1Y = Integer.parseInt(inputMediumWeak1YStr);
                        mMediumWeakPoint2X = Integer.parseInt(inputMediumWeak2XStr);
                        mMediumWeakPoint2Y = Integer.parseInt(inputMediumWeak2YStr);
                        mMediumWeakPoint3X = Integer.parseInt(inputMediumWeak3XStr);
                        mMediumWeakPoint3Y = Integer.parseInt(inputMediumWeak3YStr);
                        mWeakPoint1X = Integer.parseInt(inputWeak1XStr);
                        mWeakPoint1Y = Integer.parseInt(inputWeak1YStr);
                        mWeakPoint2X = Integer.parseInt(inputWeak2XStr);
                        mWeakPoint2Y = Integer.parseInt(inputWeak2YStr);
                        mWeakPoint3X = Integer.parseInt(inputWeak3XStr);
                        mWeakPoint3Y = Integer.parseInt(inputWeak3YStr);
                    } catch (NumberFormatException e) {
                        validInput = false;
                    }
                    if (validInput) {
                        if (mStrongPoint1X < -140 || mStrongPoint1X > -30
                                || mStrongPoint2X < -140 || mStrongPoint2X > -30
                                || mStrongPoint3X < -140 || mStrongPoint3X > -30
                                || mMediumWeakPoint1X < -140 || mMediumWeakPoint1X > -30
                                || mMediumWeakPoint2X < -140 || mMediumWeakPoint2X > -30
                                || mMediumWeakPoint3X < -140 || mMediumWeakPoint3X > -30
                                || mWeakPoint1X < -140 || mWeakPoint1X > -30
                                || mWeakPoint2X < -140 || mWeakPoint2X > -30
                                || mWeakPoint3X < -140 || mWeakPoint3X > -30) {
                            Toast.makeText(ComponentSelectActivity.this,
                                    "The Point X must be -140 ~ -30",
                                    Toast.LENGTH_SHORT).show();
                        } else if (mStrongPoint1Y < -20 || mStrongPoint1Y > 30
                                || mStrongPoint2Y < -20 || mStrongPoint2Y > 30
                                || mStrongPoint3Y < -20 || mStrongPoint3Y > 30
                                || mMediumWeakPoint1Y < -20 || mMediumWeakPoint1Y > 30
                                || mMediumWeakPoint2Y < -20 || mMediumWeakPoint2Y > 30
                                || mMediumWeakPoint3Y < -20 || mMediumWeakPoint3Y > 30
                                || mWeakPoint1Y < -20 || mWeakPoint1Y > 30
                                || mWeakPoint2Y < -20 || mWeakPoint2Y > 30
                                || mWeakPoint3Y < -20 || mWeakPoint3Y > 30) {
                            Toast.makeText(ComponentSelectActivity.this,
                                    "The Point Y must be -20 ~ 30",
                                    Toast.LENGTH_SHORT).show();
                        } else {
                            dialog.dismiss();
                            writeRsrpSinrSharedPreference(mStrongPoint1X, mStrongPoint1Y,
                                    mStrongPoint2X, mStrongPoint2Y,
                                    mStrongPoint3X, mStrongPoint3Y,
                                    mMediumWeakPoint1X, mMediumWeakPoint1Y,
                                    mMediumWeakPoint2X, mMediumWeakPoint2Y,
                                    mMediumWeakPoint3X, mMediumWeakPoint3Y,
                                    mWeakPoint1X, mWeakPoint1Y,
                                    mWeakPoint2X, mWeakPoint2Y,
                                    mWeakPoint3X, mWeakPoint3Y);
                            if (bNeedShowSimConfig) {
                                showSimSelectedDialog(getResources().getStringArray(R.array
                                        .sim_display_info));
                            } else {
                                showDialog(DIALOG_WAIT_SUBSCRIB);
                                new SubscribeTask().execute();
                            }
                        }
                    } else {
                        Toast.makeText(ComponentSelectActivity.this, "Invalid RSRP/SINR",
                                Toast.LENGTH_SHORT).show();
                    }
                } else if (which == DialogInterface.BUTTON_NEGATIVE) {
                    mStrongPoint1X = -80;
                    mStrongPoint1Y = 30;
                    mStrongPoint2X = -80;
                    mStrongPoint2Y = 20;
                    mStrongPoint3X = -30;
                    mStrongPoint3Y = 20;
                    mMediumWeakPoint1X = -140;
                    mMediumWeakPoint1Y = 10;
                    mMediumWeakPoint2X = -90;
                    mMediumWeakPoint2Y = 10;
                    mMediumWeakPoint3X = -90;
                    mMediumWeakPoint3Y = -20;
                    mWeakPoint1X = -140;
                    mWeakPoint1Y = 5;
                    mWeakPoint2X = -100;
                    mWeakPoint2Y = 5;
                    mWeakPoint3X = -100;
                    mWeakPoint3Y = -20;
                    dialog.dismiss();
                    writeRsrpSinrSharedPreference(mStrongPoint1X, mStrongPoint1Y,
                            mStrongPoint2X, mStrongPoint2Y,
                            mStrongPoint3X, mStrongPoint3Y,
                            mMediumWeakPoint1X, mMediumWeakPoint1Y,
                            mMediumWeakPoint2X, mMediumWeakPoint2Y,
                            mMediumWeakPoint3X, mMediumWeakPoint3Y,
                            mWeakPoint1X, mWeakPoint1Y,
                            mWeakPoint2X, mWeakPoint2Y,
                            mWeakPoint3X, mWeakPoint3Y);
                    if (bNeedShowSimConfig) {
                        showSimSelectedDialog(getResources().getStringArray(R.array
                                .sim_display_info));
                    } else {
                        showDialog(DIALOG_WAIT_SUBSCRIB);
                        new SubscribeTask().execute();
                    }
                }
            }
        });
    }

    private void writeRsrpSinrSharedPreference(int strongPoint1X,
                                               int strongPoint1Y, int strongPoint2X, int
                                                       strongPoint2Y,
                                               int strongPoint3X, int strongPoint3Y, int
                                                       mediumWeakPoint1X,
                                               int mediumWeakPoint1Y, int mediumWeakPoint2X, int
                                                       mediumWeakPoint2Y,
                                               int mediumWeakPoint3X, int mediumWeakPoint3Y, int
                                                       weakPoint1X,
                                               int weakPoint1Y, int weakPoint2X, int weakPoint2Y,
                                               int weakPoint3X, int weakPoint3Y) {
        final SharedPreferences rsrpSinrConfigSh = getSharedPreferences(
                COMPONENT_RSRPSINR_CONFIG_SHAREPRE, android.content.Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = rsrpSinrConfigSh.edit();
        editor.putInt(getString(R.string.strong_point_1x), strongPoint1X);
        editor.putInt(getString(R.string.strong_point_1y), strongPoint1Y);
        editor.putInt(getString(R.string.strong_point_2x), strongPoint2X);
        editor.putInt(getString(R.string.strong_point_2y), strongPoint2Y);
        editor.putInt(getString(R.string.strong_point_3x), strongPoint3X);
        editor.putInt(getString(R.string.strong_point_3y), strongPoint3Y);
        editor.putInt(getString(R.string.mediumweak_point_1x), mediumWeakPoint1X);
        editor.putInt(getString(R.string.mediumweak_point_1y), mediumWeakPoint1Y);
        editor.putInt(getString(R.string.mediumweak_point_2x), mediumWeakPoint2X);
        editor.putInt(getString(R.string.mediumweak_point_2y), mediumWeakPoint2Y);
        editor.putInt(getString(R.string.mediumweak_point_3x), mediumWeakPoint3X);
        editor.putInt(getString(R.string.mediumweak_point_3y), mediumWeakPoint3Y);
        editor.putInt(getString(R.string.weak_point_1x), weakPoint1X);
        editor.putInt(getString(R.string.weak_point_1y), weakPoint1Y);
        editor.putInt(getString(R.string.weak_point_2x), weakPoint2X);
        editor.putInt(getString(R.string.weak_point_2y), weakPoint2Y);
        editor.putInt(getString(R.string.weak_point_3x), weakPoint3X);
        editor.putInt(getString(R.string.weak_point_3y), weakPoint3Y);
        editor.commit();
    }

    private interface InputDialogOnClickListener {
        public void onClick(DialogInterface dialog, int which, String inputStrong1XStr,
                            String inputStrong1YStr, String inputStrong2XStr, String
                                    inputStrong2YStr,
                            String inputStrong3XStr, String inputStrong3YStr, String
                                    inputMediumWeak1XStr,
                            String inputMediumWeak1YStr, String inputMediumWeak2XStr, String
                                    inputMediumWeak2YStr,
                            String inputMediumWeak3XStr, String inputMediumWeak3YStr, String
                                    inputWeak1XStr,
                            String inputWeak1YStr, String inputWeak2XStr, String inputWeak2YStr,
                            String inputWeak3XStr,
                            String inputWeak3YStr);
    }

    private class DemoHandler implements TrapHandlerInterface {
        public void ProcessTrap(long timestamp, TRAP_TYPE type, int len, byte[] data, int offset) {
            switch (type) {
                case TRAP_TYPE_EM:
                    int msgID = (int) Utils.getIntFromByte(data, offset + 6, 2);
                    Msg EmMsg
                      = getDecoder(ComponentSelectActivity.this).msgInfo_getMsg(data, offset);
                    if (EmMsg == null) {
                        Elog.d(TAG, "msg is null in ProcessTrap");
                        return;
                    }
                    Message msgToUiEm = MDMComponentDetailActivity.mUpdateUiHandler.obtainMessage();
                    msgToUiEm.what = MSG_UPDATE_UI_URC_EM;
                    msgToUiEm.arg1 = msgID;
                    msgToUiEm.obj = EmMsg;
                    MDMComponentDetailActivity.mUpdateUiHandler
                            .sendMessageDelayed(msgToUiEm, UPDATE_INTERVAL);
                    break;
                case TRAP_TYPE_ICD_RECORD:
                case TRAP_TYPE_ICD_EVENT:
                    byte[] trap_data = Arrays.copyOfRange(data, offset, len);
                    ByteBuffer icdPacket = ByteBuffer.wrap(trap_data);
                    Message msgToUi =
                            MDMComponentDetailActivity.mUpdateUiHandler.obtainMessage();
                    msgToUi.what = MSG_UPDATE_UI_URC_ICD;
                    msgToUi.obj = icdPacket;
                    MDMComponentDetailActivity.mUpdateUiHandler
                            .sendMessageDelayed(msgToUi, UPDATE_INTERVAL);
                    break;
                case TRAP_TYPE_DISCARDINFO:
                    Elog.d(TAG, "[Discard ... ]");
                    break;
                default:
                    Elog.d(TAG, "[Unknown Type, Error ...]");
                    break;

            }
        }
    }

    /**
     * Background operation for attach.
     */
    private class LoadingTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {

            Elog.d(TAG, "before InitMDML");
            StartMDLService();
            InitMDML(ComponentSelectActivity.this);
            Elog.d(TAG, "after InitMDML");
            InitDecoder(ComponentSelectActivity.this);
            Elog.d(TAG, "after InitDecoder");
            mHandler.sendEmptyMessage(LOADING_DONE);
            return true;
        }
    }

    private class SubscribeTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            int k = 1;
            components = new ArrayList<String>();
            mCheckedEmTypes.clear();
            long msgId = 0;
            for (int i = 0; i < mCheckBoxes.size(); i++) {
                CheckBox c = mCheckBoxes.get(i);
                if (c.isChecked() && c.getTag() instanceof MDMComponent) {
                    mHandler.sendEmptyMessage(UPDATE_UI);
                    components.add(c.getText().toString());
                    for (MDMComponent com : allComponents) {
                        String[] emComponentName = com.getEmComponentName();
                        if ((c.getText().toString()).equals(com.getName())) {
                            if (com.hasEmType(MDMContentICD.MSG_TYPE_ICD_RECORD)) {
                                Elog.i(TAG, "the type is icd record," + com.getName());
                                for (int j = 1; j < emComponentName.length; j++) {
                                    long msgIdIcd = Long.parseLong(emComponentName[j], 16);
                                    if (!mCheckedIcdRecordTypes.contains(msgIdIcd)) {
                                        mCheckedIcdRecordTypes.add(msgIdIcd);
                                        Elog.d(TAG, "onSubscribeTrap ID = " + emComponentName[j]);
                                    }
                                }
                            } else if (com.hasEmType(MDMContentICD.MSG_TYPE_ICD_EVENT)) {
                                Elog.i(TAG, "the type is icd event," + com.getName());
                                for (int j = 1; j < emComponentName.length; j++) {
                                    long msgIdIcd = Long.parseLong(emComponentName[j], 16);
                                    if (!mCheckedIcdEventTypes.contains(msgIdIcd)) {
                                        mCheckedIcdEventTypes.add(msgIdIcd);
                                        Elog.d(TAG, "onSubscribeTrap ID = " + emComponentName[j]);
                                    }
                                }
                            } else {
                                for (int j = 0; j < emComponentName.length; j++) {
                                    Integer msgIdInt = getDecoder(ComponentSelectActivity.this)
                                            .msgInfo_getMsgID(emComponentName[j]);
                                    if (msgIdInt != null) {
                                        msgId = msgIdInt.longValue();
                                    } else {
                                        Elog.e(TAG, "The msgid is not support, msgName = " +
                                                emComponentName[j]);
                                        continue;
                                    }
                                    if (!mCheckedEmTypes.contains(msgId)) {
                                        mCheckedEmTypes.add(msgId);
                                        Elog.d(TAG, "onSubscribeTrap msg Name = " +
                                                emComponentName[j]);
                                    }
                                    if (!mCheckedEmTypes.contains(msgId)) {
                                        mCheckedEmTypes.add(msgId);
                                        Elog.d(TAG, "onSubscribeTrap msgName = "
                                               + emComponentName[j]);

                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }

            //Em type
            int size = mCheckedEmTypes.size();
            if (size > 0) {
                Long[] array = (Long[]) mCheckedEmTypes.toArray(new Long[size]);
                Elog.d(TAG, "onSubscribeTrap em msg id = " + Arrays.toString(array));
                m_cmdProxy.onSubscribeMultiTrap(m_sid, TRAP_TYPE_EM, toPrimitives(array));
            }
            //Icd record type
            size = mCheckedIcdRecordTypes.size();
            if (size > 0) {
                Long[] array = (Long[]) mCheckedIcdRecordTypes.toArray(new Long[size]);
                Elog.d(TAG, "onSubscribeTrap icd record msg id = " + Arrays.toString(array));
                m_cmdProxy.onSubscribeMultiTrap(m_sid, TRAP_TYPE_ICD_RECORD, toPrimitives(array));
            }
            //Icd event type
            size = mCheckedIcdEventTypes.size();
            if (size > 0) {
                Long[] array = (Long[]) mCheckedIcdEventTypes.toArray(new Long[size]);
                Elog.d(TAG, "onSubscribeTrap icd event msg id = " + Arrays.toString(array));
                m_cmdProxy.onSubscribeMultiTrap(m_sid, TRAP_TYPE_ICD_EVENT, toPrimitives(array));
            }
            Elog.d(TAG, "onSubscribeTrap done");
            mHandler.sendEmptyMessage(SUBSCRIBE_DONE);
            return true;
        }
    }

    private class deleteTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            MONITOR_CMD_RESP ret = m_cmdProxy.onDisableTrap(m_sid);
            if (MONITOR_CMD_RESP_SUCCESS != ret) {
                Elog.d(TAG, "Disable Trap fail");
            }
            ret = m_cmdProxy.onCloseSession(m_sid);
            if (MONITOR_CMD_RESP_SUCCESS != ret) {
                Elog.d(TAG, "Close Session fail");
            }
            SystemProperties.set("ctl.stop","md_monitor");
            Elog.d(TAG, "Disable mdm monitor");
            return true;
        }
    }
}
