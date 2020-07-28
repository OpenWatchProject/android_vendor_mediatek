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
 * MediaTek Inc. (C) 2014. All rights reserved.
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
package com.mediatek.voicecommand.mgr;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Message;
import android.os.SystemProperties;
import android.os.UserHandle;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.adapter.JNICommandAdapter;
import com.mediatek.voicecommand.business.BootCompletedReceiver;
import com.mediatek.voicecommand.business.ScreenBroadcastReceiver;
import com.mediatek.voicecommand.business.VoiceCommandBusiness;
import com.mediatek.voicecommand.business.VoiceContactsBusiness;
import com.mediatek.voicecommand.business.VoiceRecognizeBusiness;
import com.mediatek.voicecommand.business.VoiceServiceInternalBusiness;
import com.mediatek.voicecommand.business.VoiceTrainingBusiness;
import com.mediatek.voicecommand.business.VoiceTriggerBusiness;
import com.mediatek.voicecommand.business.VoiceUiBusiness;
import com.mediatek.voicecommand.business.VoiceWakeupBusiness;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.service.TriggerService;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;
import com.mediatek.voicecommand.util.Log;

public class NativeDataManager extends VoiceDataManager implements IMessageDispatcher {
    private static final String TAG = "NativeDataManager";

    private Context mContext;
    private IVoiceAdapter mIJniVoiceAdapter;
    private IMessageDispatcher mIUpMsgDispatcher;
    private BootCompletedReceiver mBootCompletedReceiver;
    private ScreenBroadcastReceiver mScreenBroadcastReceiver;

    private VoiceCommandBusiness mVoiceUI;
    private VoiceCommandBusiness mVoiceTraining;
    private VoiceCommandBusiness mVoiceRecognize;
    // Used to deal with the logic happened in the Service itself
    private VoiceCommandBusiness mVoiceServiceInternal;
    private VoiceCommandBusiness mVoiceContacts;
    private VoiceCommandBusiness mVoiceWakeup = null;
    private VoiceCommandBusiness mVoiceTrigger = null;

    private boolean mIsBootReciverRegister = false;
    private boolean mIsKeyguardVerifiedRegister = false;
    private static final String PROP_SYS_BOOT_COMPLETED = "sys.boot_completed";
    private static final String SYS_BOOT_COMPLETED = "1";

    private static final int DELAY_MILLIS = 5000;

    public NativeDataManager(VoiceCommandManagerStub service) {
        super(service);
        Log.i(TAG, "[NativeDataManager]new ... ");

        mContext = service.mContext;
        mIJniVoiceAdapter = new JNICommandAdapter(this);

        mVoiceUI = new VoiceUiBusiness(this, service.mConfigManager, mHandler, mIJniVoiceAdapter);
        mVoiceTraining = new VoiceTrainingBusiness(this, service.mConfigManager, mHandler,
                mIJniVoiceAdapter, mContext);
        mVoiceRecognize = new VoiceRecognizeBusiness(this, service.mConfigManager, mHandler,
                mIJniVoiceAdapter);
        mVoiceContacts = new VoiceContactsBusiness(this, service.mConfigManager, mHandler,
                mIJniVoiceAdapter, mContext);
        if (service.mConfigManager.isDemoSupported()) {
            mVoiceWakeup = new VoiceWakeupBusiness(this, service.mConfigManager, mHandler,
                mIJniVoiceAdapter, mContext);
         } else {
            mVoiceTrigger = new VoiceTriggerBusiness(this, service.mConfigManager, mHandler,
                mIJniVoiceAdapter, mContext);
        }
        mVoiceServiceInternal = new VoiceServiceInternalBusiness(this, service.mConfigManager,
            mHandler, mIJniVoiceAdapter);

        initBroadcastReceiver();
    }

    private void initBroadcastReceiver() {
        // register headset plug receiver
        IntentFilter filter = new IntentFilter();
        filter.addAction(Intent.ACTION_HEADSET_PLUG);
        mContext.registerReceiver(mHeadsetPlugReceiver, filter);

        // register boot complete receiver
        mBootCompletedReceiver = new BootCompletedReceiver(mContext, mHandler);

        String isBootCompleted = SystemProperties.get(PROP_SYS_BOOT_COMPLETED);
        Log.i(TAG, "[initBroadcastReceiver]isBootCompleted = " + isBootCompleted);
        if (isBootCompleted != null && isBootCompleted.equals(SYS_BOOT_COMPLETED)) {
            mHandler.sendEmptyMessageDelayed(
                    VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BOOT_COMPLETED,
                    DELAY_MILLIS);
        } else {
            filter = new IntentFilter();
            filter.addAction(Intent.ACTION_BOOT_COMPLETED);
            mContext.registerReceiver(mBootCompletedReceiver, filter);
            mIsBootReciverRegister = true;
        }

        // register User Switched receiver
        filter = new IntentFilter();
        filter.addAction(Intent.ACTION_USER_SWITCHED);
        mContext.registerReceiver(mUserSwitchedReceiver, filter);

        //register training updated receiver
        filter = new IntentFilter();
        filter.addAction("com.mediatek.vow.ACTION_SWITCH_USER");
        mContext.registerReceiver(mTrainingUpdateReceiver, filter);

        //register keyguard iniitialized receiver
        mScreenBroadcastReceiver = new ScreenBroadcastReceiver(mContext, mHandler);
        if (!mIsKeyguardVerifiedRegister && VoiceTriggerBusiness.isTriggerSupport(mContext)) {
            filter = new IntentFilter();
            filter.addAction(Intent.ACTION_SCREEN_ON);
            filter.addAction(Intent.ACTION_SCREEN_OFF);
            filter.addAction(Intent.ACTION_USER_PRESENT);
            mContext.registerReceiver(mScreenBroadcastReceiver, filter);
            mIsKeyguardVerifiedRegister = true;
        }
    }

    /*
     * Send message to native via JNICommandAdapter
     */
    @Override
    public int dispatchMessageDown(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (message.mMainAction >= VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER) {
            Log.i(TAG,
                    "[dispatchMessageDown]message== "
                            + VoiceTriggerBusiness.sDumpMsg(message));
        } else {
            Log.i(TAG, "[dispatchMessageDown]message="
                    + VoiceServiceInternalBusiness.sDumpMsg(message));
        }
        switch (message.mMainAction) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
            errorid = mVoiceUI.handleSyncVoiceMessage(message);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            errorid = mVoiceTraining.handleSyncVoiceMessage(message);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
            errorid = mVoiceRecognize.handleSyncVoiceMessage(message);
            break;

        case VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_SERVICE:
            errorid = mVoiceServiceInternal.handleSyncVoiceMessage(message);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
            errorid = mVoiceContacts.handleSyncVoiceMessage(message);
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP:
            if (mVoiceWakeup != null) {
                errorid = mVoiceWakeup.handleSyncVoiceMessage(message);
            }
            break;

        case VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER:
            if (mVoiceTrigger != null) {
                errorid = mVoiceTrigger.handleSyncVoiceMessage(message);
            }
            break;

        /*
         * Special case where AppDataManager needs to communicate with TriggerBusiness
         */
        case VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER:
            if (message.mSubAction == VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_START_ADAPTER) {
                if (mVoiceTrigger != null) {
                    errorid = mVoiceTrigger.handleSyncVoiceMessage(message);
                }
            }
            break;

        default:
            break;
        }

        return errorid;
    }

    @Override
    public int dispatchMessageUp(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (message.mMainAction >= VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER) {
            Log.i(TAG, "[dispatchMessageUp]message== "
                    + VoiceTriggerBusiness.sDumpMsg(message));
        } else {
            Log.i(TAG, "[dispatchMessageUp]message="
                    + VoiceServiceInternalBusiness.sDumpMsg(message));
        }
        switch (message.mMainAction) {
        case VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_SERVICE:
            if (message.mSubAction == VoiceServiceInternalBusiness.ACTION_VOICE_SERVICE_SELFEXIT) {
                mContext.unregisterReceiver(mHeadsetPlugReceiver);
                mContext.unregisterReceiver(mTrainingUpdateReceiver);
                if (mIsBootReciverRegister) {
                    mContext.unregisterReceiver(mBootCompletedReceiver);
                }
                if (mIsKeyguardVerifiedRegister) {
                    mContext.unregisterReceiver(mScreenBroadcastReceiver);
                }
                mBootCompletedReceiver.handleDataRelease();
                // Need to release memory here. Such as: unbind VoiceWakeupInteractionSerivce.
                if (mVoiceWakeup != null) mVoiceWakeup.handleDataRelease();
                if (mVoiceTrigger != null) mVoiceTrigger.handleDataRelease();
            }
            errorid = mVoiceServiceInternal.handleSyncVoiceMessage(message);
            break;

        /*Special case where JNI adapter needs to communicate with Training Business*/
        case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
            if (message.mSubAction == VoiceCommandListener.ACTION_VOICE_TRAINING_ENROLL_START) {
                errorid = mVoiceTraining.handleSyncVoiceMessage(message);
            } else {
                errorid = mIUpMsgDispatcher.dispatchMessageUp(message);
            }
            break;

        /*Special case where JNI adapter needs to communicate with Trigger Business*/
        case VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER:
            if (message.mSubAction == VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_NOTIFY_RETRAIN) {
                if (mVoiceTrigger != null) {
                    errorid = mVoiceTrigger.handleSyncVoiceMessage(message);
                }
            } else {
                errorid = mIUpMsgDispatcher.dispatchMessageUp(message);
            }
            break;

        default:
            errorid = mIUpMsgDispatcher.dispatchMessageUp(message);
            break;
        }

        return errorid;
    }

    @Override
    public void setDownDispatcher(IMessageDispatcher dispatcher) {
        Log.i(TAG, "[setDownDispatcher]dispatcher: " + dispatcher);
        // Don't need next dispatcher because this dispatcher send message to
        // native directly
    }

    @Override
    public void setUpDispatcher(IMessageDispatcher dispatcher) {
        Log.i(TAG, "[setUpDispatcher]dispatcher: " + dispatcher);
        mIUpMsgDispatcher = dispatcher;
    }

    private BroadcastReceiver mHeadsetPlugReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            VoiceMessage msg = new VoiceMessage();
            msg.mMainAction = VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BROADCAST;
            if (intent.getIntExtra("state", 0) == 0) {
                msg.mSubAction = VoiceServiceInternalBusiness.ACTION_VOICE_BROADCAST_HEADSETPLUGOUT;
            } else {
                msg.mSubAction = VoiceServiceInternalBusiness.ACTION_VOICE_BROADCAST_HEADSETPLUGIN;
            }
            Log.d(TAG, "[onReceive]mHeadsetPlugReceiver, msg = "
                    + VoiceServiceInternalBusiness.sDumpMsg(msg));
            mVoiceServiceInternal.handleSyncVoiceMessage(msg);
        }
    };

    private BroadcastReceiver mTrainingUpdateReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.d(TAG, "[onReceive]Training updated action: " + action);
            if ("com.mediatek.vow.ACTION_SWITCH_USER".equals(action)) {
                VoiceMessage msg = new VoiceMessage();
                msg.mMainAction = VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING;
                msg.mSubAction = VoiceCommandListener.ACTION_VOICE_TRAINING_UPDATE_MODEL;
                mVoiceTraining.handleAsyncVoiceMessage(msg);
            }
        }
    };


    private BroadcastReceiver mUserSwitchedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            int newUserId = intent.getIntExtra(Intent.EXTRA_USER_HANDLE, -1);
            Log.d(TAG, "[onReceive]User Switched. New user Id: " + newUserId);
            VoiceMessage msg = new VoiceMessage();
            msg.mExtraData = DataPackage.packageSendInfo(newUserId);
            /*NOTE: We need to figure out a mechanism to pass new user id here.*/
            if (newUserId == UserHandle.USER_SYSTEM) {
                if (mVoiceWakeup != null) {
                    msg.mMainAction = VoiceWakeupBusiness.ACTION_MAIN_INTERNAL_WAKEUP;
                    msg.mSubAction = VoiceWakeupBusiness.ACTION_VOICE_WAKEUP_USER_SWITCHED_OWNER;
                    mVoiceWakeup.handleAsyncVoiceMessage(msg);
                }
                if (mVoiceTrigger != null) {
                    msg.mMainAction = VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER;
                    msg.mSubAction = VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_USER_SWITCHED_OWNER;
                    mVoiceTrigger.handleAsyncVoiceMessage(msg);
                }
            } else {
                if (mVoiceWakeup != null) {
                    msg.mMainAction = VoiceWakeupBusiness.ACTION_MAIN_INTERNAL_WAKEUP;
                    msg.mSubAction =
                        VoiceWakeupBusiness.ACTION_VOICE_WAKEUP_USER_SWITCHED_NON_OWNER;
                    mVoiceWakeup.handleAsyncVoiceMessage(msg);
                }
                if (mVoiceTrigger != null) {
                    msg.mMainAction = VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER;
                    msg.mSubAction =
                        VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_USER_SWITCHED_NON_OWNER;
                    mVoiceTrigger.handleAsyncVoiceMessage(msg);
                }
            }
        }
    };

    // After OnReceive bootcompleted then unreigster boot completed receiver
    private void handleAsyncVoiceMessage() {
        if (VoiceTriggerBusiness.isTriggerSupport(mContext)) {
            mContext.startService(new Intent(mContext, TriggerService.class));
        }
        mContext.unregisterReceiver(mBootCompletedReceiver);
        mIsBootReciverRegister = false;
    }

    // After OnReceive bootcompleted then unreigster boot completed receiver
    private void handleAsyncKeyguardVerified() {
        Log.d(TAG, "[handleAsyncKeyguardVerified]...");
        // If the keyguard is not secure, then unlock will be sent.
        // Action needed here only if Trigger service is supported.
        if (VoiceTriggerBusiness.isTriggerSupport(mContext)) {
            mBootCompletedReceiver.sendTriggerInitMessage();
            {   //Initialize the adapter service here
                VoiceMessage msg = new VoiceMessage();
                msg.mMainAction = VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER;
                msg.mSubAction = VoiceTriggerBusiness.ACTION_VOICE_TRIGGER_START_ADAPTER;
                if (mVoiceTrigger != null) mVoiceTrigger.handleAsyncVoiceMessage(msg);
            }
        }
        if (mIsKeyguardVerifiedRegister) {
            mContext.unregisterReceiver(mScreenBroadcastReceiver);
            mIsKeyguardVerifiedRegister = false;
        }
    }

    // Service accident dead, need registerobserver once again
    private void handleAsyncVoiceBootCompleted() {
        mBootCompletedReceiver.registerContactsObserver();
    }

    private Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "[handleMessage]msg.what = " + msg.what);
            if (msg.what >= VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER) {
                Log.d(TAG,
                        "[handleMessage]msg.what = "
                                + VoiceTriggerBusiness
                                        .getMainActionName(msg.what));
            } else if (msg.what >= VoiceWakeupBusiness.ACTION_MAIN_INTERNAL_WAKEUP) {
                Log.d(TAG,
                        "[handleMessage]msg.what = "
                                + VoiceWakeupBusiness
                                        .getMainActionName(msg.what));
            } else if (msg.what >= VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_SERVICE_BASE) {
                Log.d(TAG,
                        "[handleMessage]msg.what = "
                                + VoiceServiceInternalBusiness
                                        .getMainActionName(msg.what));
            } else {
                Log.d(TAG,
                        "[handleMessage]msg.what = "
                                + VoiceCommandListener
                                        .getMainActionName(msg.what));
            }
            switch (msg.what) {
            case VoiceCommandListener.ACTION_MAIN_VOICE_UI:
                mVoiceUI.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_TRAINING:
                mVoiceTraining.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_RECOGNITION:
                mVoiceRecognize.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_CONTACTS:
                mVoiceContacts.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_WAKEUP:
            case VoiceWakeupBusiness.ACTION_MAIN_INTERNAL_WAKEUP:
                if (mVoiceWakeup != null)
                    mVoiceWakeup.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;

            case VoiceCommandListener.ACTION_MAIN_VOICE_TRIGGER:
            case VoiceTriggerBusiness.ACTION_MAIN_INTERNAL_TRIGGER:
                if (mVoiceTrigger != null)
                    mVoiceTrigger.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;

            case VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_SERVICE:
            case VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BROADCAST:
                mVoiceServiceInternal.handleAsyncVoiceMessage((VoiceMessage) msg.obj);
                break;

            case VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BROADCAST_BOOT_COMPLETED:
                handleAsyncVoiceMessage();
                break;

            case VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BOOT_COMPLETED:
                handleAsyncVoiceBootCompleted();
                break;

            case VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_BROADCAST_KEYGUARD_VERIFIED:
                handleAsyncKeyguardVerified();
                break;

            default:
                break;
            }
        }
    };
}
