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
package com.mediatek.voicecommand.service;

import android.content.Context;
import android.os.Binder;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;

import com.mediatek.common.voicecommand.IVoiceCommandListener;
import com.mediatek.common.voicecommand.IVoiceCommandManagerService;
import com.mediatek.voicecommand.business.VoiceCommandBusiness;
import com.mediatek.voicecommand.business.VoiceServiceInternalBusiness;
import com.mediatek.voicecommand.mgr.AppDataManager;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.NativeDataManager;
import com.mediatek.voicecommand.mgr.ServiceDataManager;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

public final class VoiceCommandManagerStub extends IVoiceCommandManagerService.Stub implements
        IVoiceCommandManagerService { // ,
                                      // Watchdog.Monitor

    private static final String TAG = "VoiceCommandManagerStub";

    public final Context mContext;
    public ConfigurationManager mConfigManager;

    AppDataManager mAppDataManager;
    ServiceDataManager mServiceDataManager;
    NativeDataManager mNativeDataManager;

    // VoiceDataManager dataManager

    public VoiceCommandManagerStub(Context context) {
        super();
        Log.i(TAG, "[VoiceCommandManagerStub]new,package name = " + context.getPackageName());
        mContext = context;
        mConfigManager = ConfigurationManager.getInstance(context);

        mAppDataManager = new AppDataManager(this);
        mServiceDataManager = new ServiceDataManager(this);
        mNativeDataManager = new NativeDataManager(this);

        // set the order of dispatcher;
        mAppDataManager.setDownDispatcher(mServiceDataManager);
        mServiceDataManager.setUpDispatcher(mAppDataManager);
        mServiceDataManager.setDownDispatcher(mNativeDataManager);
        mNativeDataManager.setUpDispatcher(mServiceDataManager);

        Log.i(TAG, "[VoiceCommandManagerStub]new End !");

    }

    @Override
    public int registerListener(String pkgName, IVoiceCommandListener listener)
            throws RemoteException {
        Log.i(TAG, "[registerListener]pkgName =" + pkgName + ",register listener " + listener);

        return mAppDataManager.registerListener(pkgName, Binder.getCallingUid(),
                Binder.getCallingPid(), listener);

    }

    @Override
    public int unregisterListener(String pkgName, IVoiceCommandListener listener)
            throws RemoteException {
        Log.i(TAG, "[unregisterListener]pkgName =" + pkgName + ",unregister listener " + listener);

        return mAppDataManager.unRegisterListener(pkgName, Binder.getCallingUid(),
                Binder.getCallingPid(), listener, false);
    }

    @Override
    public IBinder asBinder() {
        return this;
    }

    @Override
    public int sendCommand(String pkgName, int mainAction, int subAction, Bundle extraData)
            throws RemoteException {
        VoiceMessage message = new VoiceMessage();
        message.mPkgName = pkgName;
        message.mMainAction = mainAction;
        message.mSubAction = subAction;
        message.mExtraData = extraData;
        message.pid = Binder.getCallingPid();
        message.uid = Binder.getCallingUid();
        Log.i(TAG, "[sendCommand]message = " + message + ",pid = "
                + message.pid + ",uid = " + message.uid);

        return mAppDataManager.dispatchMessageDown(message);

    }

    /*
     * We need to release the native memory when destroyed
     */
    public void release() {
        Log.i(TAG, "[release]...");
        // Release memory from native to app
        VoiceMessage message = new VoiceMessage();
        message.mMainAction = VoiceServiceInternalBusiness.ACTION_MAIN_VOICE_SERVICE;
        message.mSubAction = VoiceServiceInternalBusiness.ACTION_VOICE_SERVICE_SELFEXIT;
        mNativeDataManager.dispatchMessageUp(message);
        mConfigManager.release();
    }
}
