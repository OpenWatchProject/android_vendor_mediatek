/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2017. All rights reserved.
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

package com.mediatek.internal.telephony;

import android.content.Context;

import com.android.internal.telephony.CommandsInterface;

import com.mediatek.internal.telephony.dataconnection.DataConnectionExt;
import com.mediatek.internal.telephony.dataconnection.IDataConnectionExt;
import com.mediatek.internal.telephony.datasub.IDataSubSelectorOPExt;
import com.mediatek.internal.telephony.datasub.DataSubSelectorOpExt;
import com.mediatek.internal.telephony.datasub.ISimSwitchForDSSExt;
import com.mediatek.internal.telephony.datasub.ISmartDataSwitchAssistantOpExt;
import com.mediatek.internal.telephony.datasub.SimSwitchForDSSExt;
import com.mediatek.internal.telephony.datasub.SmartDataSwitchAssistantOpExt;
import com.mediatek.internal.telephony.IServiceStateTrackerExt;
import com.mediatek.internal.telephony.DefaultTelephonyExt;
import com.mediatek.internal.telephony.IDefaultTelephonyExt;
import com.mediatek.internal.telephony.MtkGsmCdmaCallTrackerExt;
import com.mediatek.internal.telephony.IMtkGsmCdmaCallTrackerExt;
import com.mediatek.internal.telephony.MtkProxyControllerExt;
import com.mediatek.internal.telephony.IMtkProxyControllerExt;
import com.mediatek.internal.telephony.devreg.IDeviceRegisterExt;
import com.mediatek.internal.telephony.devreg.DefaultDeviceRegisterExt;
import com.mediatek.internal.telephony.devreg.DeviceRegisterController;
import com.mediatek.internal.telephony.uicc.IMtkRsuSml;
import com.mediatek.internal.telephony.uicc.IMtkSimHandler;
import com.mediatek.internal.telephony.uicc.MtkSimHandler;
import android.telephony.Rlog;

// M: Self Activation Feature @{
import com.mediatek.internal.telephony.digits.DigitsUtil;
import com.mediatek.internal.telephony.digits.DigitsUtilBase;
import com.mediatek.internal.telephony.selfactivation.ISelfActivation;
import com.mediatek.internal.telephony.selfactivation.SelfActivationDefault;
// @|
import com.mediatek.internal.telephony.digits.DigitsUssdManager;
import com.mediatek.internal.telephony.digits.DigitsUssdManagerBase;
import com.mediatek.internal.telephony.scbm.ISCBMManager;
import com.mediatek.internal.telephony.scbm.SCBMManagerDefault;

public class OpTelephonyCustomizationFactoryBase {
    public IServiceStateTrackerExt makeServiceStateTrackerExt(Context context) {
        return new ServiceStateTrackerExt(context);
    }

    /**
     *Make DataConnectionExt as plug in for Mobile Data Service.
     *@param context The context of caller.
     *@return IDataConnectionExt The instance of DataConnectionExt.
     */
    public IDataConnectionExt makeDataConnectionExt(Context context) {
         return new DataConnectionExt(context);
    }

     /**
     *Make DataConnectionExt as plug in for Mobile Data Service.
     *@param context The context of caller.
     *@return IDataConnectionExt The instance of DataConnectionExt.
     */
    public IDefaultTelephonyExt makeTelephonyExt(Context context) {
         return new DefaultTelephonyExt(context);
    }

    /**
     * Make DeviceRegisterExt as plug in for Device Register.
     * @param context The context of caller.
     * @param controller the device register controller
     * @return IDeviceRegisterExt The instance of DefaultDeviceRegisterExt.
     */
    public IDeviceRegisterExt makeDeviceRegisterExt(Context context,
            DeviceRegisterController controller) {
        return new DefaultDeviceRegisterExt(context, controller);
    }

    /**
     * Make implementation instance for self-activation.
     * @param phoneId The phone ID for the instance.
     * @return ISelfActivation The instance, return NULL if not ready.
     */
    public ISelfActivation makeSelfActivationInstance(int phoneId) {
        ISelfActivation instance = null;
        instance = new SelfActivationDefault(phoneId);
        return instance;
    }

    public IDataSubSelectorOPExt makeDataSubSelectorOPExt(Context context) {
        return new DataSubSelectorOpExt(context);
    }

    public ISimSwitchForDSSExt makeSimSwitchForDSSOPExt(Context context) {
        return new SimSwitchForDSSExt(context);
    }

    /**
     *Make MtkGsmCdmaCallTrackerExt as plug in for Gsm Call Control Service.
     *@param context The context of caller.
     *@return IMtkGsmCdmaCallTrackerExt The instance of MtkGsmCdmaCallTrackerExt.
     */
    public IMtkGsmCdmaCallTrackerExt makeMtkGsmCdmaCallTrackerExt(Context context) {
         return new MtkGsmCdmaCallTrackerExt(context);
    }

    public DigitsUtil makeDigitsUtil() {
        return new DigitsUtilBase();
    }

    public DigitsUssdManager makeDigitsUssdManager() {
        return new DigitsUssdManagerBase();
    }

    /**
     *Make MtkSimHandler as plug in for Sim frameworks.
     *@param context The context of caller.
     *@return IMtkSimHandler The instance of MtkSimHandler.
     */
    public IMtkSimHandler makeMtkSimHandler(Context context, CommandsInterface ci) {
         Rlog.d("makeMtkSimHandler", "[makeMtkSimHandler] ");
         return new MtkSimHandler(context, ci);
    }

    /**
     *Make MtkProxyControllerExt as plug in for sim switch.
     *@param context The context of caller.
     *@return IMtkProxyControllerExt The instance of MtkProxyControllerExt.
     */
    public IMtkProxyControllerExt makeMtkProxyControllerExt(Context context) {
        return new MtkProxyControllerExt(context);
    }

    /**
     *Make SmartDataSwitchAssistantOpExt as plug in for smart data switch.
     *@param context The context of caller.
     *@return ISmartDataSwitchAssistantOpExt The instance of SmartDataSwitchAssistantOpExt.
     */
    public ISmartDataSwitchAssistantOpExt makeSmartDataSwitchAssistantOpExt(Context context) {
        return new SmartDataSwitchAssistantOpExt(context);
    }

    public ISCBMManager makeSCBMManager(Context context , int phoneId, CommandsInterface ci) {
        return new SCBMManagerDefault(context, phoneId, ci);
    }

    /**
     * Get remote sim unlock instatnce.
     *
     * @param context The context of caller.
     * @param ci The cis of all slots.
     * @return remote sim unlock instatnce
     *
     */
    public IMtkRsuSml makeRsuSml(Context context, CommandsInterface[] ci) {
        return null;
    }
}
