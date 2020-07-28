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
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.android.internal.telephony.PhoneConstants;
import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.EmUtils;
import com.mediatek.engineermode.FeatureSupport;
import com.mediatek.engineermode.ModemCategory;
import com.mediatek.engineermode.R;
import com.mediatek.mdml.Msg;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_EM;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_EVENT;
import static com.mediatek.mdml.TRAP_TYPE.TRAP_TYPE_ICD_RECORD;

public class MDMComponentDetailActivity extends Activity
        implements OnClickListener {
    private static final String TAG = "EmInfo/MDMComponentDetailActivity";

    private static final int DIALOG_WAIT_UNSUBSCRIB = 0;
    private final static int UNSUBSCRIBE_DONE = 1;
    public static Handler mUpdateUiHandler;
    public static int mSimType;
    public static int mModemType;
    public static String mSimMccMnc[] = new String[2];
    private static boolean isMsgShow = false;
    ArrayList<Long> mCheckedEmInteger = new ArrayList<Long>();
    List<MDMComponent> mComponents;
    private int mCurrentItemIndex = 0;
    private Button mPageUp;
    private Button mPageDown;
    private FrameLayout mInfoFrameLayout;
    private TextView mTitle;
    private int mItemCount = 0;
    private MDMComponent mCurrentItem;
    private int screenWidth;
    private int screenHeight;
    private int leftEdge;
    private int menuPadding = 80;
    private View content;
    private View menu;
    private LinearLayout.LayoutParams menuParams;
    private ProgressDialog mProgressDialog;
    private ArrayList<String> checked;
    private List<MDMComponent> allComponents;
    private InitTask mInitTask = null;
    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case UNSUBSCRIBE_DONE: {
                    removeDialog(DIALOG_WAIT_UNSUBSCRIB);
                    Elog.d(TAG, "Wait unSubscribe message done");
                    MDMComponentDetailActivity.this.finish();
                    break;
                }
                default:
                    break;
            }
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Elog.d(TAG, "init MDMComponentDetailActivity ..");
        setContentView(R.layout.mdm_component_detail);
        mTitle = (TextView) findViewById(R.id.detail_title_mdm);
        mInfoFrameLayout = (FrameLayout) findViewById(R.id.detail_frame_mdm);
        mPageUp = (Button) findViewById(R.id.NetworkInfo_PageUp_mdm);
        mPageDown = (Button) findViewById(R.id.NetworkInfo_PageDown_mdm);
        mPageUp.setOnClickListener(this);
        mPageDown.setOnClickListener(this);

        Intent intent = getIntent();
        checked = (ArrayList<String>) intent.getSerializableExtra("mChecked");
        mSimType = intent.getIntExtra("mSimType", PhoneConstants.SIM_ID_1);
        mModemType = intent.getIntExtra("mModemType", MDMSimSelectActivity.MODEM_PROTOCOL_1);

        allComponents = MDMComponent.getComponents(this);
        mComponents = new ArrayList<MDMComponent>();

        mUpdateUiHandler = new Handler() {
            public void handleMessage(Message msg) {
                switch (msg.what) {
                    case ComponentSelectActivity.MSG_UPDATE_UI_URC_EM:
                        Msg msgObj = (Msg) msg.obj;
                        if (msgObj != null) {
                            String name = msgObj.getMsgName();
                            int msgID = ComponentSelectActivity.getDecoder(
                                    MDMComponentDetailActivity.this).msgInfo_getMsgID(name);
                            for (MDMComponent c : mComponents) {
                                if (c.hasEmType(name) && isMsgShow == true) {
                                    if (msgObj.getSimIdx() != mModemType &&
                                            !ComponentSelectActivity.mSelected_show_both_sim
                                                    .contains(c.getName())) {
                                        //Elog.d(TAG, "Msg has a unSelected sim id: SIM" +
                                        //            msgObj.getSimIdx());
                                        return;
                                    }
                                    Elog.v(TAG, "Update data: " + c.getName() + ",type = " + name
                                            + ",Slot = " + (mSimType + 1) + ",Protocol = " +
                                            mModemType + ",getSimIdx = " + msgObj.getSimIdx()
                                    );
                                    c.update(name, msg.obj);
                                } else {
                                    Elog.d(TAG, "name:" + name + "[" + msgID + "], isMsgShow:" + isMsgShow);
                                    Elog.d(TAG, "not find in EmComponentName: " +
                                            Arrays.toString(c.getEmComponentName()));
                                }
                            }
                        } else {
                            Elog.e(TAG, "msgObj is null");
                        }
                        break;
                    case ComponentSelectActivity.MSG_UPDATE_UI_URC_ICD:
                        ByteBuffer icdPacket = (ByteBuffer) msg.obj;
                        MDMContentICD.MDMHeaderICD IcdHeader = null;
                        IcdHeader = DecodeICDRecordPacketHeader(icdPacket);
                        String msgIDStr = Integer.toHexString(IcdHeader.msg_id);
                        Elog.d(TAG, "msgIDStr = " + msgIDStr);
                        for (MDMComponent c : mComponents) {
                            if (c.hasEmType(msgIDStr) && isMsgShow == true) {
                                if (IcdHeader.protocol_id != mModemType) {
                                    Elog.d(TAG, "Msg has a unSelected icd id");
                                    return;
                                }
                                Elog.v(TAG, "[UpdateUI][ICD][" + msgIDStr + "] Update data to "
                                     + c.getGroup() + "[" + c.getName() + "], type = " + msgIDStr);
                                c.update(msgIDStr, msg.obj);
                            }
                        }
                        break;
                    default:
                        break;
                }
            }
        };
        mInitTask = new InitTask();
        mInitTask.execute();
    }

    public static  MDMContentICD.MDMHeaderICD DecodeICDRecordPacketHeader(ByteBuffer icdPacket) {
        MDMContentICD.MDMHeaderICD headIcd = new MDMContentICD.MDMHeaderICD();
        headIcd.type = icdPacket.get(0) & 0x0F;
        headIcd.version = (icdPacket.get(0) & 0xF0) >> 4;
        headIcd.total_size = icdPacket.get(1) | (icdPacket.get(2) << 8) | (icdPacket.get(3) << 16);
        headIcd.timeestamp_type = icdPacket.get(4) & 0x0F; //lower 4 bit
        headIcd.protocol_id = (icdPacket.get(4) & 0xF0) >> 4; //upper 4 bit
        headIcd.msg_id = icdPacket.get(6) | icdPacket.get(7) << 8;
        //Elog.d(TAG, "type = " + Integer.toHexString(headIcd.type));
        //Elog.d(TAG, "version = " + Integer.toHexString(headIcd.version));
        //Elog.d(TAG, "total_size = " + headIcd.total_size);
        //Elog.d(TAG, "timeestamp_type = " + Integer.toHexString(headIcd.timeestamp_type));
        //Elog.d(TAG, "protocol_id = " + Integer.toHexString(headIcd.protocol_id));
        //Elog.d(TAG, "msg_id = " + Integer.toHexString(headIcd.msg_id));
        return headIcd;
    }

    @Override
    protected void onResume() {
        super.onResume();
        Elog.d(TAG, "onResume");

        TelephonyManager telephonyManager = (TelephonyManager) getSystemService(TELEPHONY_SERVICE);
        int phoneIdMain = ModemCategory.getCapabilitySim();
        mSimMccMnc[0] = telephonyManager.getSimOperatorNumericForPhone(phoneIdMain);
        mSimMccMnc[1] = telephonyManager.getSimOperatorNumericForPhone((phoneIdMain + 1) % 2);
        Elog.d(TAG, "Read SIM MCC+MNC(PS1):" + mSimMccMnc[0]);
        Elog.d(TAG, "Read SIM MCC+MNC(PS2):" + mSimMccMnc[1]);
        updateModemTypeStatus();
    }

    @Override
    public void onStop() {
        super.onStop();
        Elog.d(TAG, "onStop");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        Elog.d(TAG, "onDestroy");
    }

    @Override
    public void onBackPressed() {
        isMsgShow = false;
        showDialog(DIALOG_WAIT_UNSUBSCRIB);
        new unSubscribeTask().execute();
    }

    @Override
    public void onClick(View arg0) {
        if (arg0.getId() == mPageUp.getId()) {
            mCurrentItemIndex = (mCurrentItemIndex - 1 + mItemCount) % mItemCount;
            updateUI();
        } else if (arg0.getId() == mPageDown.getId()) {
            mCurrentItemIndex = (mCurrentItemIndex + 1) % mItemCount;
            updateUI();
        }
    }

    public void updateUI() {
        if (mComponents.size() == 0) {
            Elog.e(TAG, "mComponents size =0");
            return;
        }
        if (mCurrentItem != null) {
            mCurrentItem.removeView();
        }
        mCurrentItem = mComponents.get(mCurrentItemIndex);
        if (mCurrentItem instanceof FTNetworkInfo) {
            EmUtils.checkLocationProxyAppPermission(this, true);
            Elog.d(TAG, "Selected Sim: " + getIntent().getIntExtra("mSelectSim", -1));
            ((FTNetworkInfo) mCurrentItem).setCurrentSimID(getIntent().getIntExtra("mSelectSim",
                    -1));
        }
        mTitle.setText(mCurrentItem.getName()
                + "(SimSlot=" + (mSimType + 1)
                + ",ModemProtocol=" + mModemType + ")");
        View view = mCurrentItem.getView();
        ViewGroup parent = (ViewGroup) view.getParent();
        if (parent != null) {
            parent.removeAllViews();
        }
        mInfoFrameLayout.removeAllViews();
        mInfoFrameLayout.addView(view);
        Elog.d(TAG, "updateUI done!");
    }

    public void unSubscribeTrap() {
        Elog.d(TAG, "Before unSubscribeTrap..");
        int size = ComponentSelectActivity.mCheckedEmTypes.size();
        Long[] array = (Long[]) ComponentSelectActivity.mCheckedEmTypes.toArray(new Long[size]);
        Elog.d(TAG, "unSubscribeTrap msg id = " + Arrays.toString(array));
        ComponentSelectActivity.getCmdProxy(this).onUnsubscribeMultiTrap
                (ComponentSelectActivity.m_sid, TRAP_TYPE_EM,
                        ComponentSelectActivity.toPrimitives(array));
        size = ComponentSelectActivity.mCheckedIcdRecordTypes.size();
        array = (Long[]) ComponentSelectActivity.mCheckedIcdRecordTypes.toArray(new Long[size]);
        Elog.d(TAG, "unSubscribeTrap icd record msg id = " + Arrays.toString(array));
        ComponentSelectActivity.getCmdProxy(this).onUnsubscribeMultiTrap
                (ComponentSelectActivity.m_sid, TRAP_TYPE_ICD_RECORD,
                        ComponentSelectActivity.toPrimitives(array));

        size = ComponentSelectActivity.mCheckedIcdEventTypes.size();
        array = (Long[]) ComponentSelectActivity.mCheckedIcdEventTypes.toArray(new Long[size]);
        Elog.d(TAG, "unSubscribeTrap icd event msg id = " + Arrays.toString(array));
        ComponentSelectActivity.getCmdProxy(this).onUnsubscribeMultiTrap
                (ComponentSelectActivity.m_sid, TRAP_TYPE_ICD_EVENT,
                        ComponentSelectActivity.toPrimitives(array));
        mHandler.sendEmptyMessage(UNSUBSCRIBE_DONE);
    }

    private void initValues() {
        WindowManager window = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        screenWidth = window.getDefaultDisplay().getWidth();
        screenHeight = window.getDefaultDisplay().getHeight() - 200;
        content = findViewById(R.id.content_mdm);
        menu = findViewById(R.id.menu_mdm);
        menuParams = (LinearLayout.LayoutParams) menu.getLayoutParams();
        menuParams.width = screenWidth - menuPadding;
        leftEdge = -menuParams.width;
        menuParams.leftMargin = leftEdge;
        content.getLayoutParams().width = screenWidth;
        content.getLayoutParams().height = screenHeight;
        //content.setOnTouchListener(this);
        if (checked == null) {
            Elog.d(TAG, "checked is null");
        } else {
            for (String s : checked) {
                for (MDMComponent c : allComponents) {
                    if (s.equals(c.getName())) {
                        mComponents.add(c);
                        break;
                    }
                }
            }
        }
    }

    @Override
    protected Dialog onCreateDialog(int id, Bundle args) {
        switch (id) {
            case DIALOG_WAIT_UNSUBSCRIB:
                Elog.d(TAG, "Wait unSubscribe message..");
                mProgressDialog = new ProgressDialog(this);
                mProgressDialog.setTitle("Waiting");
                mProgressDialog.setMessage("Wait unSubscribe message..");
                mProgressDialog.setCancelable(false);
                mProgressDialog.setIndeterminate(true);
                return mProgressDialog;
            default:
                return super.onCreateDialog(id);
        }
    }

    private void updateModemTypeStatus() {
        Elog.d(TAG, "updateModemTypeStatus");
        int capabilitySims = ModemCategory.getCapabilitySim();
        if (capabilitySims == PhoneConstants.SIM_ID_1) {
            if (mSimType == PhoneConstants.SIM_ID_1) {
                mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_1;
            } else if (mSimType == PhoneConstants.SIM_ID_2) {
                mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_2;
            } else {
                mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_3;
            }
        } else if (capabilitySims == PhoneConstants.SIM_ID_2) {
            if (mSimType == PhoneConstants.SIM_ID_1) {
                mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_2;
            } else if (mSimType == PhoneConstants.SIM_ID_2) {
                mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_1;
            } else {
                mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_3;
            }
        } else if (capabilitySims == PhoneConstants.SIM_ID_3) {
            if (FeatureSupport.is93Modem()) {
                if (mSimType == PhoneConstants.SIM_ID_1) {
                    mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_2;
                } else if (mSimType == PhoneConstants.SIM_ID_2) {
                    mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_3;
                } else {
                    mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_1;
                }
            } else {
                if (mSimType == PhoneConstants.SIM_ID_1) {
                    mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_3;
                } else if (mSimType == PhoneConstants.SIM_ID_2) {
                    mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_2;
                } else {
                    mModemType = MDMSimSelectActivity.MODEM_PROTOCOL_1;
                }
            }
        }
        updateUI();
        Elog.d(TAG, "mSimType = " + mSimType + ",mModemType = " + mModemType);
    }

    private class InitTask extends AsyncTask<Void, Void, Boolean> {
        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            initValues();
            return true;
        }

        @Override
        protected void onPostExecute(Boolean result) {
            mItemCount = mComponents.size();
            updateUI();
            try {
                ComponentSelectActivity.getCmdProxy(MDMComponentDetailActivity.this).onEnableTrap
                        (ComponentSelectActivity.m_sid);
            } catch (Exception e) {
                Elog.e(TAG, "ComponentSelectActivity.m_cmdProxy.onEnableTrap failed ");
            }
            isMsgShow = true;
            Elog.d(TAG, "init MDMComponentDetailActivity done");
        }
    }

    private class unSubscribeTask extends AsyncTask<Void, Void, Boolean> {

        @Override
        protected Boolean doInBackground(Void... params) {
            // TODO Auto-generated method stub
            unSubscribeTrap();
            return true;
        }
    }

}
