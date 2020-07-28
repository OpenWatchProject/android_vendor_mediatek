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
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.mediatek.engineermode.Elog;
import com.mediatek.engineermode.R;

import java.util.ArrayList;
import java.util.List;

public class MDMComponentDetailActivity extends Activity
                        implements OnClickListener, MDMCoreOperation.IDataUpdate {
    private static final String TAG = "EM_DiagnosticMetric_Info";
    private static final int DIALOG_WAIT_UNSUBSCRIB = 0;

    private Button mPageUp;
    private Button mPageDown;
    private FrameLayout mInfoFrameLayout;
    private Toast mToast = null;
    private TextView mTitle;

    private int mItemCount = 0;
    private int mCurrentItemIndex = 0;
    private MdmBaseComponent mCurrentItem;
    private List<MdmBaseComponent> mComponents;

    private int screenWidth;
    private int screenHeight;

    private int leftEdge;
    private int rightEdge = 0;
    private int menuPadding = 80;
    private View content;
    private View menu;
    private ListView mMenuListView;
    private LinearLayout.LayoutParams menuParams;

    private ProgressDialog mProgressDialog;
    private static boolean isMsgShow = false;
    private ArrayList<String> items;

    private Handler mHandler = new Handler() {
        public void handleMessage(Message msg) {
            switch (msg.what) {
            case MDMCoreOperation.UNSUBSCRIBE_DONE: {
                removeDialog(DIALOG_WAIT_UNSUBSCRIB);
                Elog.d(TAG, "Wait unSubscribe message done");
                for (MdmBaseComponent com : mComponents) {
                    if(com != null) {
                        com.resetView();
                    }
                }
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
        initUI();

        initValues();
        MDMComponentSelectActivity.getMdmInstance().setOnDataUpdateListener(this);
        MDMComponentSelectActivity.getMdmInstance().mdmlEnableSubscribe();
    }

    public void initUI() {
        mTitle = (TextView) findViewById(R.id.detail_title_mdm);
        mInfoFrameLayout = (FrameLayout) findViewById(R.id.detail_frame_mdm);
        mPageUp = (Button) findViewById(R.id.NetworkInfo_PageUp_mdm);
        mPageDown = (Button) findViewById(R.id.NetworkInfo_PageDown_mdm);
        mPageUp.setOnClickListener(this);
        mPageDown.setOnClickListener(this);
    }

    @Override
    public void onBackPressed() {
        isMsgShow = false;
        showDialog(DIALOG_WAIT_UNSUBSCRIB);
        MDMComponentSelectActivity.getMdmInstance().mdmlUnSubscribe();
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
        Elog.d(TAG, "[updateUI]: " + mCurrentItemIndex + ", " + mComponents.size());
        if (mCurrentItem != null) {
            mCurrentItem.removeView();
        }
        mCurrentItem = mComponents.get(mCurrentItemIndex);
        mTitle.setText(mCurrentItem.getName());
        if(mInfoFrameLayout.getChildCount() > 0) {
            mInfoFrameLayout.removeAllViews();
        }
        View view = mCurrentItem.getView();
        if(view != null && view.getParent() != null) {
            ((ViewGroup) view.getParent()).removeView(view);
        }
        mInfoFrameLayout.addView(view);
        Elog.d(TAG, "updateUI done");
    }

    private void initValues() {
        items = new ArrayList<String>();
        Intent intent = getIntent();
        mComponents = MDMComponentSelectActivity.getMdmInstance().getSelectedComponents();
        mItemCount = mComponents.size();
        Elog.d(TAG, "[initValues]mItemCount = " + mItemCount);

        WindowManager window = (WindowManager) getSystemService(Context.WINDOW_SERVICE);
        screenWidth = window.getDefaultDisplay().getWidth();
        screenHeight = window.getDefaultDisplay().getHeight()-200;
        Elog.d(TAG, "screenWidth = " + screenWidth + "screenHeight = " + screenHeight);
        content = findViewById(R.id.content_mdm);
        menu = findViewById(R.id.menu_mdm);
        menuParams = (LinearLayout.LayoutParams) menu.getLayoutParams();
        menuParams.width = screenWidth - menuPadding;
        leftEdge = -menuParams.width;
        menuParams.leftMargin = leftEdge;
        content.getLayoutParams().width = screenWidth;
        content.getLayoutParams().height = screenHeight;

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

    @Override
    public void onDataUpdate(int msg_id) {
        switch(msg_id) {
        case MDMCoreOperation.UNSUBSCRIBE_DONE:
            mHandler.sendEmptyMessage(msg_id);
            break;
        case MDMCoreOperation.ENABLE_SUBSCRIBE_DONE:
            updateUI();
        }
    }
}
