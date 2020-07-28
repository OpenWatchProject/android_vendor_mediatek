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

package com.mediatek.ims.pco.ui;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;

import com.mediatek.ims.pco.Constants;
import com.mediatek.ims.pco.PCOService;
import com.mediatek.ims.pco.PCOSharedPreference;
import com.mediatek.ims.pco.R;

/**
 *  Operator Settings activity. Activation now button would be shown
 *  if phone is not activated otherwise it is not shown.
 */
public class PCOAccountSettings extends Activity {

    private static final String TAG = "PCOAccountSettings";

    private Button mActivateNowButton;
    private static PCOSharedPreference sPCOInfoPref;

    /**
     *  Creates settings activity and it binds to PCO service.
     * @param savedInstanceState : not used.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        sPCOInfoPref = PCOSharedPreference.getInstance(getApplicationContext());

        setContentView(
                R.layout.pco_operator_account_settings);

        mActivateNowButton = (Button) this.findViewById(R.id.activate_now_btn);
        mActivateNowButton.setOnClickListener(mAccountViewOnClickListener);
        mActivateNowButton.setAllCaps(false);
        LinearLayout mBtnArea = (LinearLayout) this.findViewById(R.id.btn_area);
        View mView = (View) this.findViewById(R.id.divider);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LOCKED);
        if (sPCOInfoPref.isActivated() == true) {
            mActivateNowButton.setVisibility(View.GONE);
            mBtnArea.setVisibility(View.GONE);
            mView.setVisibility(View.GONE);
        } else {
            bindToService();
        }

      }

    /**
     *
     */
    @Override
    public void onResume() {
        super.onResume();

    }

    private View.OnClickListener mAccountViewOnClickListener = new View.OnClickListener() {

        /**
         *
         * @param v
         */
        public void onClick(View v) {
              switch (v.getId()) {
                case R.id.activate_now_btn:
                    log("[==>]IMS PCO Status Check");
                    sendMessageToService(Constants.MSG_ID_REQ_CHECK_IMS_PCO_STATUS);
                    finish();
                    break;
                default:
                break;
            }
        }
    };

    @Override
    public void onDestroy() {
        // Unregister since the activity is about to be closed.
        super.onDestroy();
        unbindFromService();
    }

    /**
     *  Messager, Bind/unbind code is present.
     */
    private Messenger mService = null;
    private boolean mBound;
    private ServiceConnection mServiceConnection;

    private void sendMessageToService(int mReqID) {
        log("[==>]Request to service, MSG ID" + mReqID);

        if (mService != null) {
            Message message = new Message();
            message.what = mReqID;
            try {
                mService.send(message);
            } catch (RemoteException e) {
                Log.d(TAG, e.toString());
            }

        }
    }

    /**
     *
     */
    private void unbindFromService() {

        if (mServiceConnection != null) {
            log("unbind service");
            if (mBound) {
                unbindService(mServiceConnection);
                mBound = false;
            }
        }
    }

    /**
     * Starts the service and binds to it.
     */
    private void bindToService() {
        if (mServiceConnection == null) {

            mServiceConnection = new ServiceConnection() {
                /**
                 * Called when the service is connected.
                 * Allows us to retrieve the binder to talk to the service.
                 */
                @Override
                public void onServiceConnected(ComponentName name, IBinder iBinder) {
                   log("Activity connected to service");
                    mService = new Messenger(iBinder);
                    mBound = true;
                }

                /**
                 * Called when the service got disconnected, e.g. because it crashed.
                 * This is <em>not</em> called when we unbind from the service.
                 */
                @Override
                public void onServiceDisconnected(ComponentName name) {
                    log("Activity disconnected from service");
                    mService = null;
                    mBound = false;
                }
            };
        }

        // Start service so that it doesn't stop when we unbind
        log("start requested & bind service");
        Intent service = new Intent(this, PCOService.class);
        bindService(service,
                mServiceConnection,
                Context.BIND_AUTO_CREATE);
    }
    private void log(String s) {
          Log.d(TAG, s);
    }
}
