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

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.business.VoiceCommandBusiness;
import com.mediatek.voicecommand.business.VoiceCommonBusiness;
import com.mediatek.voicecommand.service.VoiceCommandManagerStub;
import com.mediatek.voicecommand.util.Log;

public class ServiceDataManager extends VoiceDataManager implements IMessageDispatcher {
    private static final String TAG = "ServiceDataManager";

    private IMessageDispatcher mIDownMsgDispatcher;
    private IMessageDispatcher mIUpMsgDispatcher;

    // used to handle common businesse
    private VoiceCommandBusiness mVoiceCommandBusiness;

    public ServiceDataManager(VoiceCommandManagerStub service) {
        super(service);
        Log.i(TAG, "[ServiceDataManager]new ... ");
        mVoiceCommandBusiness = new VoiceCommonBusiness(this, service.mConfigManager, null);
    }

    @Override
    public int dispatchMessageDown(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[dispatchMessageDown]message=" + message);
        switch (message.mMainAction) {
        case VoiceCommandListener.ACTION_MAIN_VOICE_COMMON:
            errorid = mVoiceCommandBusiness.handleSyncVoiceMessage(message);
            break;
        default:
            if (mIDownMsgDispatcher != null) {
                errorid = mIDownMsgDispatcher.dispatchMessageDown(message);
            }
            break;
        }

        return errorid;
    }

    @Override
    public int dispatchMessageUp(VoiceMessage message) {
        Log.i(TAG, "[dispatchMessageUp]message=" + message);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        // Do we need to filter here?

        if (errorid == VoiceCommandListener.VOICE_NO_ERROR) {
            if (mIUpMsgDispatcher != null) {
                errorid = mIUpMsgDispatcher.dispatchMessageUp(message);
            }
        }

        return errorid;
    }

    @Override
    public void setDownDispatcher(IMessageDispatcher dispatcher) {
        Log.i(TAG, "[setDownDispatcher]dispatcher = " + dispatcher);
        mIDownMsgDispatcher = dispatcher;
    }

    @Override
    public void setUpDispatcher(IMessageDispatcher dispatcher) {
        Log.i(TAG, "[setUpDispatcher]dispatcher = " + dispatcher);
        mIUpMsgDispatcher = dispatcher;
    }

}
