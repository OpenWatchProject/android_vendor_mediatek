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
package com.mediatek.settings.ext;

/**
 * Operator requirements add interface for NetworkSettings UI.
 * opeator like China Mobile,China Telecom, China Unicom, etc.
 */
public interface IMobileNetworkSettingsExt {

    /**
     * china mobile feature
     * return true means "4/3/2 g auto" grey out item show
     * common mobilenetworksetting can not has this item
     */
    boolean customizeAutoSelectNetworkTypePreference();

    /**
     * For CMCC VOLTE feature.
     * VOLTE item just can show on CMCC card
     * @param subId sub id
     * @return true if this is CMCC card.
    */
    boolean customizeVolteState(int subId);

    /**
    * For CMCC VOLTE feature.
    * when is CMCC card, VOLTE show enable.
    * else VOLTE show disable.
    * SIM_STATE_CHANGED broadcast dual with.
    * @return true if SIM_STATE_CHAGNED.
    */
    boolean customizeVolteStateReceiveIntent();

    /**
     * For CMCC mobile data feature.
     * when cmcc+non-cmcc insert, grey out non-cmcc mobile data.
     * @param subId
     * @return true if this is CMCC card.
     */
    boolean customizaMobileDataState(int subId);

    /**
     * For TMO feature.
     * TMO has self prefernetwork mode
     * @param subId
     * @return true if TMO feature.
     */
    boolean customizeTMONetworkModePreference(int subId);

    /**
     * For AT&T feature.
     * ATT remove OM preferred network, enablenetworkmode, plmn
     * @param subId
     * @return true if AT&T  feature.
     */
    boolean customizeATTNetworkModePreference(int subId);
    /**
     * For AT & T feature.
     * update mobile data title and summary
     * @param object,switchPreference
     * @param subId subId
     * defaultmobilenetworksettings do nothing
     */
    void customizaMobileDataSummary(Object object, int subId);

    /**
     * For AT & T feature.
     * update roaming preference title and summary
     * @param object, switchPreference
     * @param subId subId
     * defaultmobilenetworksettings do nothing
     */
    void customizaRoamingPreference(Object object, int subId);

    /**
     * For AT & T feature.
     * update roaming preference dialog summary
     * @param summary
     * @param subId
     * @return default return summary, at&t return requirement string
     */
    String customizaRoamingPreferenceDialogSummary(String summary, int subId);

    /**
     * For AT & T feature.
     * update Enhanced4gLTE Preference
     * @param object, switchPreference
     * @param subId subId
     * defaultmobilenetworksettings do nothing
     */
    void customizaEnhanced4gLTEPreference(Object object, int subId);

    /**
     * For AT&T feature.
     * add feature "disable 2g" 
     * @param subId
     * @return true if AT&T feature.
     * @param subId
     * @return true if AT&T project
     */
    boolean customizeATTDisable2Gvisible(int subId);

    /**
     * For AT&T feature.
     * add ATT manual femto cell selection
     * @param subId
     */
    boolean customizeATTManualFemtoCellSelectionPreference(int subId);

    /**
     * For Verzion feature.
     * remove google aosp network type select preference.
     * add new type select preference for verzion
     * @param subId
     * @return true if Verzion project
     */
    boolean customizeVWZNetworkModePreference(int subId);

    /**
     * For Version feature.
     * add Femto cell feature when ro.vendor.mtk_femto_cell_support = 1
     * @param subId
     */
    boolean customizeVersionFemtoCellSelectionPreference(int subId);

    /**
     * For RJIO feature RJIO has self prefernetwork mode.
     * @param subId sim subid
     * @return true if RJIO feature.
     */
    public boolean customizeRjioNetworkModePreference(int subId);

    /**
     * For Spint roaming feature.
     * remove google default roaming if spint support 
     * @param mSubId
     * @return
     */
    public boolean customizaSpintRoaming(int mSubId);
}
