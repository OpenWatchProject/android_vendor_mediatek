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

/**
 * For operator requirements: add interface.
 * opeator like China Mobile,China Telecom, China Unicom, etc.
 */
package com.mediatek.settings.ext;

import android.preference.Preference;
import android.preference.PreferenceCategory;
import android.preference.PreferenceScreen;

import com.android.internal.telephony.OperatorInfo;

import java.util.List;


public interface INetworkSettingExt {

    /**
     * Let plug-in customize the OperatorInfo list before display.
     *
     * @param operatorInfoList The OperatorInfo list get from framework
     * @param subId The sub id user selected
     * @return new OperatorInfo list
     */
    public List<OperatorInfo> customizeNetworkList(List<OperatorInfo> operatorInfoList, int subId);

    /**
     * Let plug-in customize the OperatorInfo name when display.
     *
     * @param operatorInfo The operatorInfo from framework
     * @param subId The sub id user selected
     * @param networkName before networkName
     * @return operatorInfo new name
     */
    public String customizeNetworkName(OperatorInfo operatorInfo,
            int subId, String networkName);

    /**
     * CU feature, customize forbidden Preference click, pop up a toast.
     * @param operatorInfo Preference's operatorInfo
     * @param subId sub id
     * @return true It means the preference click will be done
     */
    public boolean onPreferenceTreeClick(OperatorInfo operatorInfo, int subId);

    /**
     * Handle the prefernce icon when clicked.
     * @param preferenceScreen parent preference screen
     * @param preference prefernce clicked
     * @return true if handle the preference by plugin
     * @internal
     */
    boolean onPreferenceTreeClick(PreferenceScreen preferenceScreen, Preference preference);

    /**
     * Update the preference screen UI while entering.
     * @param prefSet parent preference screen
     * @internal
     */
    void initOtherNetworkSetting(PreferenceScreen prefSet);

    /**
     * Update the preference screen UI while entering.
     * @param preferenceCategory parent preference category
     */
    void initOtherNetworkSetting(PreferenceCategory preferenceCategory);

    /**
     * remove select key.
     * @param object
     */
    void customizeNetworkSelectKey(Object object);
}
