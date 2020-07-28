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
package com.mediatek.voicecommand.business;

import android.content.Context;
import android.os.Handler;
import android.os.SystemProperties;

import com.mediatek.common.voicecommand.VoiceCommandListener;
import com.mediatek.voicecommand.adapter.IVoiceAdapter;
import com.mediatek.voicecommand.data.DataPackage;
import com.mediatek.voicecommand.mgr.ConfigurationManager;
import com.mediatek.voicecommand.mgr.IMessageDispatcher;
import com.mediatek.voicecommand.mgr.VoiceMessage;
import com.mediatek.voicecommand.util.Log;

/**
 * Manage Voice Contacts Search business.
 * 
 */
public class VoiceContactsBusiness extends VoiceCommandBusiness {
    private static final String TAG = "VoiceContactsBusiness";

    private IVoiceAdapter mIJniVoiceAdapter;
    private IMessageDispatcher mIMessageDispatcher;

    private static final int VOICE_CONTACTS_RECOGNITION_DISABLE = 0;
    private static final int VOICE_CONTACTS_RECOGNITION_ENABLE = 1;
    public static final boolean MTK_VOICE_CONTACT_SEARCH_SUPPORT = SystemProperties.get(
            "ro.mtk_voice_contact_support").equals("1");

    /**
     * VoiceContacts constructor.
     * 
     * @param dispatcher
     *            NativeDataManager instance
     * @param cfgMgr
     *            ConfigurationManager instance
     * @param handler
     *            the handler to run voice contacts search message
     * @param adapter
     *            SwipAdapter instance
     * @param context
     *            context
     */
    public VoiceContactsBusiness(IMessageDispatcher dispatcher, ConfigurationManager cfgMgr,
            Handler handler, IVoiceAdapter adapter, Context context) {
        super(dispatcher, cfgMgr, handler);
        Log.i(TAG, "[VoiceContactsBusiness]new...");
        mIMessageDispatcher = dispatcher;
        mIJniVoiceAdapter = adapter;
    }

    @Override
    public int handleSyncVoiceMessage(VoiceMessage message) {
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Log.i(TAG, "[handleSyncVoiceMessage]mSubAction = " + message.mSubAction);

        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_START:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_STOP:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ENABLE:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_DISABLE:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_INTENSITY:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SELECTED:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SEARCH_COUNT:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ORIENTATION:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_RECOGNITION_ENABLE:
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_RECOGNITION_DISABLE:
            errorid = sendMessageToHandler(message);
            break;

        default:
            break;
        }
        Log.i(TAG, "[handleSyncVoiceMessage]errorid = " + errorid);
        return errorid;
    }

    @Override
    public int handleAsyncVoiceMessage(VoiceMessage message) {
        Log.i(TAG, "[handleAsyncVoiceMessage]message.mSubAction = " + message.mSubAction);
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        if (!MTK_VOICE_CONTACT_SEARCH_SUPPORT) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_VOICE_INVALID;
            Log.i(TAG, "[handleAsyncVoiceMessage]Voice Contacts feature is off, return!");
            sendMessageToApps(message, errorid);
            return errorid;
        }

        switch (message.mSubAction) {
        case VoiceCommandListener.ACTION_VOICE_CONTACTS_START:
            errorid = handleContactsStart(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_STOP:
            errorid = handleContactsStop(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ENABLE:
            errorid = handleContactsEnable(message, true);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_DISABLE:
            errorid = handleContactsEnable(message, false);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_INTENSITY:
            handleContactsIntensity(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SELECTED:
            handleContactsSelected(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_NAME:
            handleContactsName(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_SEARCH_COUNT:
            handleContactsSearchCnt(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_ORIENTATION:
            handleContactsOrientation(message);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_RECOGNITION_ENABLE:
            handleContactsRecogEnable(message, true);
            break;

        case VoiceCommandListener.ACTION_VOICE_CONTACTS_RECOGNITION_DISABLE:
            handleContactsRecogEnable(message, false);
            break;

        default:
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_INVALID_ACTION;
            break;
        }
        Log.i(TAG, "[handleAsyncVoiceMessage]errorid = " + errorid);

        return errorid;
    }

    private int handleContactsStart(VoiceMessage message) {
        Log.d(TAG, "[handleContactsStart]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCfgMgr.isProcessEnable(message.mPkgName)) {
            if (message.mExtraData == null) {
                errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SEND_INVALID;
            } else {
                String modelpath = mCfgMgr.getContactsModelFile();
                String contactsdbPath = mCfgMgr.getContactsdbFilePath();
                if (modelpath == null || contactsdbPath == null) {
                    Log.i(TAG, "[handleContactsName] error modelpath=" + modelpath
                            + ", contactsdbPath=" + contactsdbPath);
                    errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
                } else {
                    int screenOrientation = message.mExtraData
                            .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
                    errorid = mIJniVoiceAdapter.startVoiceContacts(message.mPkgName, message.pid,
                            screenOrientation, modelpath, contactsdbPath);
                }
            }
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_PROCESS_OFF;
        }
        Log.d(TAG, "[handleContactsStart]errorid = " + errorid);
        sendMessageToApps(message, errorid);

        return errorid;
    }

    private int handleContactsStop(VoiceMessage message) {
        Log.d(TAG, "[handleContactsStop]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (mCfgMgr.isProcessEnable(message.mPkgName)) {
            errorid = mIJniVoiceAdapter.stopVoiceContacts(message.mPkgName, message.pid);
        } else {
            errorid = VoiceCommandListener.VOICE_ERROR_COMMON_PROCESS_OFF;
        }
        sendMessageToApps(message, errorid);
        Log.d(TAG, "[handleContactsStop]errorid = " + errorid);
        return errorid;
    }

    private int handleContactsEnable(VoiceMessage message, boolean isEnable) {
        Log.d(TAG, "[handleContactsEnable]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        Boolean isCurEnable = mCfgMgr.isProcessEnable(message.mPkgName);
        if (!(isCurEnable & isEnable)) {
            errorid = mCfgMgr.updateFeatureEnable(message.mPkgName, isEnable) ? VoiceCommandListener.VOICE_NO_ERROR
                    : VoiceCommandListener.VOICE_ERROR_COMMON_ILLEGAL_PROCESS;
            if (errorid != VoiceCommandListener.VOICE_NO_ERROR) {
                sendMessageToApps(message, errorid);
            }
        }

        if (isEnable) {
            handleContactsStart(message);
        } else {
            handleContactsStop(message);
        }
        Log.d(TAG, "[handleContactsEnable]errorid = " + errorid);

        return errorid;
    }

    private int handleContactsIntensity(VoiceMessage message) {
        Log.d(TAG, "[handleContactsIntensity]...");
        int intensity = mIJniVoiceAdapter.getNativeIntensity();

        message.mExtraData = DataPackage.packageResultInfo(
                VoiceCommandListener.ACTION_EXTRA_RESULT_SUCCESS, intensity, 0);
        mIMessageDispatcher.dispatchMessageUp(message);

        return VoiceCommandListener.VOICE_NO_ERROR;
    }

    private int handleContactsSelected(VoiceMessage message) {
        Log.d(TAG, "[handleContactsSelected]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SEND_INVALID;
        } else {
            String selectedName = message.mExtraData
                    .getString(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            errorid = mIJniVoiceAdapter.sendContactsSelected(selectedName);
        }

        sendMessageToApps(message, errorid);
        Log.d(TAG, "[handleContactsSelected]errorid = " + errorid);

        return errorid;
    }

    /*
     * Send all contacts name to next dispatcher
     * 
     * @param contactsNameList contacts name list
     */
    private int handleContactsName(VoiceMessage message) {
        Log.d(TAG, "[handleContactsName]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SEND_INVALID;
        } else {
            String modelpath = mCfgMgr.getContactsModelFile();
            String contactsdbPath = mCfgMgr.getContactsdbFilePath();
            if (modelpath == null || contactsdbPath == null) {
                Log.i(TAG, "[handleContactsName]error modelpath=" + modelpath + ", contactsdbPath="
                        + contactsdbPath);
                errorid = VoiceCommandListener.VOICE_ERROR_COMMON_SERVICE;
            } else {
                String[] allContactsNames = message.mExtraData
                        .getStringArray(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
                errorid = mIJniVoiceAdapter.sendContactsName(modelpath, contactsdbPath,
                        allContactsNames);
            }
        }
        Log.d(TAG, "[handleContactsName]errorid = " + errorid);
        return errorid;
    }

    private int handleContactsSearchCnt(VoiceMessage message) {
        Log.d(TAG, "[handleContactsSearchCnt]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SEND_INVALID;
        } else {
            int searchCnt = message.mExtraData.getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            errorid = mIJniVoiceAdapter.sendContactsSearchCnt(searchCnt);
        }

        sendMessageToApps(message, errorid);
        Log.d(TAG, "[handleContactsSearchCnt]errorid = " + errorid);
        return errorid;
    }

    private int handleContactsOrientation(VoiceMessage message) {
        Log.d(TAG, "[handleContactsOrientation]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;

        if (message.mExtraData == null) {
            errorid = VoiceCommandListener.VOICE_ERROR_CONTACTS_SEND_INVALID;
        } else {
            int orientation = message.mExtraData
                    .getInt(VoiceCommandListener.ACTION_EXTRA_SEND_INFO);
            errorid = mIJniVoiceAdapter.sendContactsOrientation(orientation);
        }

        sendMessageToApps(message, errorid);
        Log.d(TAG, "[handleContactsOrientation]errorid = " + errorid);

        return errorid;
    }

    private int handleContactsRecogEnable(VoiceMessage message, boolean isEnable) {
        Log.d(TAG, "[handleContactsRecogEnable]...");
        int errorid = VoiceCommandListener.VOICE_NO_ERROR;
        int recognitionEnable = VOICE_CONTACTS_RECOGNITION_DISABLE;
        if (isEnable) {
            recognitionEnable = VOICE_CONTACTS_RECOGNITION_ENABLE;
        }
        errorid = mIJniVoiceAdapter.sendContactsRecogEnable(recognitionEnable);
        sendMessageToApps(message, errorid);
        Log.d(TAG, "[handleContactsRecogEnable]errorid = " + errorid);

        return errorid;
    }
}
