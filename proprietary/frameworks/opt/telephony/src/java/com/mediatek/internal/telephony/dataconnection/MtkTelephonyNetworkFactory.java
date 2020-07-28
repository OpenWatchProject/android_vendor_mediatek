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
/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.mediatek.internal.telephony.dataconnection;

import android.content.Context;
import android.net.NetworkCapabilities;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneSwitcher;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.SubscriptionMonitor;
import com.android.internal.telephony.dataconnection.DcTracker;
import com.android.internal.telephony.dataconnection.TelephonyNetworkFactory;

public class MtkTelephonyNetworkFactory extends TelephonyNetworkFactory {
    public MtkTelephonyNetworkFactory(SubscriptionMonitor subscriptionMonitor, Looper looper,
                                   Phone phone) {
        super(subscriptionMonitor, looper, phone);
    }

    @Override
    protected NetworkCapabilities makeNetworkFilter(int subscriptionId) {
        NetworkCapabilities nc = super.makeNetworkFilter(subscriptionId);
        nc.addCapability(NetworkCapabilities.NET_CAPABILITY_BIP);
        nc.addCapability(NetworkCapabilities.NET_CAPABILITY_VSIM);
        return nc;
    }

    @Override
    protected boolean mtkIgnoreCapabilityCheck(NetworkCapabilities capabilities, int action) {
        // IMS @{
        if (capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_IMS) ||
                capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_EIMS)) {
            if(action == ACTION_REQUEST) {
                if(MtkDcHelper.getInstance().isMultiPsAttachSupport()) {
                    log("ignoreCapabilityCheck() allow IMS/EIMS pdn activation");
                    return true;
                }

                // Always allow for EIMS
                if(capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_EIMS) &&
                        (TelephonyManager.getDefault().getPhoneCount() == 1)) {
                    log("ignoreCapabilityCheck() allow IMS/EIMS pdn activation in ss load");
                    return true;
                }

                log("ignoreCapabilityCheck() reject IMS/EIMS pdn activation");
                return false;
            }
            log("ignoreCapabilityCheck() ignore IMS/EIMS PDN");
            return true;
        }
        // @}

        // VSIM @{
        if (capabilities.hasCapability(NetworkCapabilities.NET_CAPABILITY_VSIM)) {
            log("ignoreCapabilityCheck() ignore VSIM PDN");
            return true;
        }
        // @}
        return false;
    }
}
