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
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.PendingIntent;
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
import android.widget.TextView;

import com.mediatek.ims.pco.Constants;
import com.mediatek.ims.pco.PCONwUtils;
import com.mediatek.ims.pco.PCOService;
import com.mediatek.ims.pco.R;

/**
 *  This is the main activity. It is called from PCO service.
 *  Service will provide all the information in intent.
 *  Which screen must be shown.
 *
 *  Activity communicates with service via messager.
 */
public class PCOStartUPWizardMainActivity extends Activity {

    private static final String TAG = "PCOStartUPWizardMainActivity";

    private Intent mIntent;

    private Context mContext;
    private String mUIScreen = null;

    /* values : registered or notRegistered */
    private String mActivationStatus = "unknown";
    private static final String ACTIVATED = "Activated";
    private static final String NOT_ACTIVATED = "notActivated";

    /* Network to use for registration.
    *  values - 1. useWifiNWOnly - use wifi network for activation.
    *           2. anyNW  - Use any network for activation.
    *
    */

    private static final String USE_WIFI_NW_ONLY = "useWifiNWOnly";
    private static final String USE_MOBILE_NW_ONLY = "useMobileNW";
    private String mNWToUse = "unknown";

    /*Acts as Positive button based on context*/
    private Button mOKButton;
    /*Acts as Next button based on context*/
    private Button mCancelButton;
    /* To show the error notification to user */
    private TextView mNwRejectedInfo;
    /* Information to user, what to do next? */
    private TextView mUserInfo;
    private TextView mActInfo;

    /**
     * Single layout is used. In order to save the user interest.
     *
     * @param savedInstanceState not in used
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        mContext = this;

        setContentView(R.layout.pco_startup_wizard_activity);
        mOKButton = (Button) this.findViewById(R.id.ok_btn);
        mCancelButton = (Button) this.findViewById(R.id.cancel_btn);
        mUserInfo = (TextView) this.findViewById(R.id.user_notification_info_id);
        mActInfo = (TextView) this.findViewById(R.id.setup_status_id);
        mNwRejectedInfo = (TextView) this.findViewById(R.id.setup_nw_err_info_id);
        mOKButton.setAllCaps(false);
        mCancelButton.setAllCaps(false);
        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LOCKED);
         mIntent = getIntent();
         if ((mIntent != null) && (mIntent.getAction() == null)) {
             return;
         }

         if (mIntent.getAction() == Constants.ACTION_SIGN_UP) {
             mActivationStatus =
                     mIntent.getStringExtra(Constants.EXTRA_ACTIVATION_STATUS);
             mNWToUse =
                     mIntent.getStringExtra(Constants.EXTRA_NETWORK_TYPE_TO_USE);
             if (null == mNWToUse){
                 mNWToUse = "unknown";
             }
         }
         log("[<==]onCreate:mActivationStatus=" + mActivationStatus + ", mNWToUse ==" + mNWToUse);
         uiController();
//         bindToService();
      }

    /**
     *
     */
    @Override
    public void onResume() {
        super.onResume();
        log("onResume:mActivationStatus=" + mActivationStatus + ", mNWToUse=" + mNWToUse);
        uiController();
//        bindToService();

    }

    /**
     * UI controllers sets the strings and onClickListeners.
     */
    private void uiController() {

        Intent ii = mIntent;
        String mAction = mIntent.getAction();
        String mScreen = "";
        log(" UIcontroller:mAction = " + mAction);
        log("UIcontroller:mActivationStatus=" + mActivationStatus + ", mNWToUse=" + mNWToUse);

        if (ii == null || mAction == null) {
            return;
        }

        switch (mAction) {
        case Constants.OPERATOR_APPLICATION_SETTING:
            mScreen = Constants.SCREEN_SHOW_URL_DIALOGUE;
            break;
        case Constants.ACTION_SIGN_UP:
            /**
             *
             */
            if (mNWToUse.equals(Constants.USE_WIFI_NW_ONLY)) {
                if (PCONwUtils.isWIFIConnected() == false) {
                    mScreen = Constants.SCREEN_SHOW_WIFI_DIALOGUE;
                } else {
                    mScreen = Constants.SCREEN_SHOW_URL_DIALOGUE;
                }

            } else if (mNWToUse.equals(Constants.USE_MOBILE_NW_ONLY)) {
                /**
                 *
                 */
                log("dialog  = SCREEN_SHOW_DATA_DIALOGUE ");

                mScreen = Constants.SCREEN_SHOW_DATA_DIALOGUE;

            }
        break;
        case Constants.ACTION_SIGN_UP_EXIT:
            /**
             * This is called when SIM is activated
             */
            mScreen = Constants.SCREEN_SHOW_ACTIVATED;
            break;
        case Constants.SIGNUP_USER_NOTIFICATION_ID:
            mScreen = Constants.SCREEN_SHOW_URL_DIALOGUE;
            break;
        default:
            break;
        }

        if (mScreen == null) {
            return;
        }

        switch (mScreen) {
        case Constants.SCREEN_SHOW_WELCOME:
            break;
        case Constants.SCREEN_SHOW_WIFI_DIALOGUE:
             setViewForWifiActivation();
            break;
        case Constants.SCREEN_SHOW_DATA_DIALOGUE:
            setViewForMobileActivation();
            break;
        case Constants.SCREEN_SHOW_URL_DIALOGUE:
            setViewForOnlineSignup();
            break;
        case Constants.SCREEN_SHOW_ACTIVATED:
            setViewForActivated();
            break;
        default:
            break;
        }
        log("UIcontroller:Requested Screen = " + mScreen);
    }

    /**
     *
     */
    private View.OnClickListener mSignUpOnClickListener = new View.OnClickListener() {

        public void onClick(View v) {
              switch (v.getId()) {
                case R.id.ok_btn:
                    sendMessageToService(Constants.MSG_ID_REQ_OPEN_END_POINT_URL);
                    break;
                case R.id.cancel_btn:
                    sendMessageToService(Constants.MSG_ID_REQ_OPEN_END_POINT_URL_DISAGREE);
                    finish();
                    break;
                default:
                    break;
            }

        }
    };

    /**
     *
     */
    private View.OnClickListener mWifiOnClickListener = new View.OnClickListener() {
        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.ok_btn:
                    sendMessageToService(Constants.MSG_ID_REQ_TRUN_ON_WIFI);
                    break;
                case R.id.cancel_btn:
                    sendMessageToService(Constants.MSG_ID_REQ_CALL_911);
                     break;
                default:
                    break;
            }
        }
    };

    /**
     *
     */
    private View.OnClickListener mOnClickListenerMobileData = new View.OnClickListener() {

        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.ok_btn:
                    log("mOnClickListenerMobileData"  + "turnOnCellularRadio");
//                    sendMessageToService(Constants.MSG_ID_REQ_TRUN_ON_MOBILE_DATA);
                    PCONwUtils.turnOnCellularRadio();
//                    PCONwUtils.enableWirelessConnectivity();
                    break;
                case R.id.cancel_btn:
                    log("mOnClickListenerMobileData"  + "turnOffCellularRadio");
//                    sendMessageToService(Constants.MSG_ID_REQ_CALL_911);
                    PCONwUtils.turnOffCellularRadio();
                    break;
                default:
                    break;
            }
        }
    };

    /**
     *
     */
    private View.OnClickListener mActivatedOnClickListener = new View.OnClickListener() {

        public void onClick(View v) {
            switch (v.getId()) {
                case R.id.ok_btn:
                    finish();
                    break;
                default:
                    break;
            }
        }
    };

    /**
     * Set view when activation using Mobile data.
     * */
    private void setViewForWifiActivation() {

        mNwRejectedInfo.setText(mContext.getResources().getString(
                R.string.nw_rejected_err_details_to_user));
        mUserInfo.setText(mContext.getResources().getString(
                R.string.setup_on_wifi_nw_only));
        mOKButton.setText(mContext.getResources().getString(
                R.string.use_wifi));

        mCancelButton.setText(getString(R.string.call_911));
        mOKButton.setOnClickListener(mWifiOnClickListener);
        mCancelButton.setOnClickListener(mWifiOnClickListener);

        updateNotification();
    }

    /**
     * Set view when activation using Mobile data.
     * */
    private void setViewForMobileActivation() {
        //nw_rejected_err_details_to_user
        mOKButton.setText(mContext.getResources().getString(
                R.string.use_mobile_data));

        mNwRejectedInfo.setText(
                getString(R.string.nw_rejected_signup_via_mobile_data));
        mCancelButton.setText(getString(R.string.turnoff_mobile_data));
        mOKButton.setOnClickListener(mOnClickListenerMobileData);
        mCancelButton.setOnClickListener(mOnClickListenerMobileData);
        mUserInfo.setText(mContext.getResources().getString(
                R.string.setup_on_mobile_data_nw_only));
        updateNotification();
    }

    /**
     * Set view when activation using Mobile data.
     * */
    private void setViewForOnlineSignup() {

        /* Shows the details to use why Activation is failed
         What to do next ? * in upper half of the screen */

        mNwRejectedInfo.setText(mContext.getResources().getString(
                R.string.nw_rejected_err_details_to_user));
        mActInfo.setText(mContext.getResources().getString(
                R.string.setup_nw_status_not_reg));

        mOKButton.setText(mContext.getResources().getString(
                R.string.signup_now));
        mOKButton.setOnClickListener(mSignUpOnClickListener);

        mCancelButton.setText(mContext.getResources().getString(
                R.string.later_not_now));
        mCancelButton.setOnClickListener(mSignUpOnClickListener);
        mUserInfo.setText(mContext.getResources().getString(
                R.string.signup_user_info));
         updateNotification();

    }

    /**
     * Set view when activation using Mobile data.
     * */
    private void setViewForActivated() {
        mUserInfo.setText(mContext.getResources().getString(
                R.string.user_nw_info_activated));
        mOKButton.setText(mContext.getResources().getString(
                android.R.string.ok));
        mCancelButton.setVisibility(View.GONE);
        mOKButton.setOnClickListener(mActivatedOnClickListener);
        mActInfo.setText(mContext.getResources().getString(
                R.string.setup_nw_status_registered));
        cancelNotifications();
    }

    /**
     *
     */
    @Override
    public void onDestroy() {
        // Unregister since the activity is about to be closed.
        super.onDestroy();
        unbindFromService();
    }

    /**
     * Updates the notification in the notification pane.
     */
    private void updateNotification() {
        log("adding the notification ");
        Notification.Builder mBuilder = new Notification.Builder(mContext.getApplicationContext(),
                Constants.SIGNUP_USER_NOTIFICATION_ID);
              Intent mQueryIntent = new Intent(
                      Constants.ACTION_QUERY_IMS_PCO_STATUS_FRM_NOTIFICATION);
        PendingIntent pendingIntent = PendingIntent.getBroadcast(mContext, 0, mQueryIntent, 0);
        mBuilder.setContentIntent(pendingIntent);
        mBuilder.setContentText(getString(R.string.notification_sigup_user_info));
        mBuilder.setSmallIcon(R.mipmap.ic_launcher_round);
        mBuilder.setAutoCancel(true);
        NotificationManager mNotificationManager = getNotificationManager(mContext);
        NotificationChannel channel = new NotificationChannel(Constants.SIGNUP_USER_NOTIFICATION_ID,
                mContext.getResources().getString(R.string.notification_sigup_title),
                NotificationManager.IMPORTANCE_DEFAULT);
        mNotificationManager.createNotificationChannel(channel);
        mNotificationManager.notify(0, mBuilder.build());
    }

    /**
     * Cancels the notification.
     */
    public void cancelNotifications() {
        log("[-]canceling notification");
        getNotificationManager(mContext).cancel(0);

    }

    /**
     * Returns the system's notification manager needed to add/remove notifications.
     */
    private NotificationManager getNotificationManager(Context context) {
        return (NotificationManager) context.getSystemService(Context.NOTIFICATION_SERVICE);
    }

    /**
     * Unbinds from the service but does not actually stop the service.
     * This lets us have it run in the background even if this isn't the active activity.
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
    protected void bindToService() {
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
