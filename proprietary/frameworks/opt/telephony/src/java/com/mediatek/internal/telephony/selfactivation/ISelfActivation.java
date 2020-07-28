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

package com.mediatek.internal.telephony.selfactivation;

import android.content.Context;

import android.os.Bundle;

import com.android.internal.telephony.CommandsInterface;

public interface ISelfActivation {
    /**
     * Actions for selfActivationAction API
     */
    public int ACTION_ADD_DATA_SERVICE = 0;
    // Extra key for add data operation
    public String EXTRA_KEY_ADD_DATA_OP = "key_add_data_operation";
    public int ADD_DATA_DECLINE = 0;
    public int ADD_DATA_AGREE = 1;

    public int ACTION_MO_CALL = 1;
    public int ACTION_RESET_520_STATE = 2;
    // Extra key for call type
    public String EXTRA_KEY_MO_CALL_TYPE = "key_mo_call_type";
    public int CALL_TYPE_NORMAL = 0;
    public int CALL_TYPE_EMERGENCY = 1;

    /**
     * Definition for activation states
     */
    public int STATE_NONE = 0;
    public int STATE_ACTIVATED = 1;
    public int STATE_NOT_ACTIVATED = 2;
    public int STATE_UNKNOWN = -1;

    /**
     * Definition for pco5to0 states
     */
    public int STATE_520_NONE = 0;
    public int STATE_520_ACTIVATED = 1;
    public int STATE_520_UNKNOWN = -1;

    /**
     * Trigger actions for self activation scenario
     * @param action The action to take
     *  0: add data service
     *  1: MO call
     * @param param The parameters for the action
     *
     * @return Return nagtive value if failed
     */
    public int selfActivationAction(int action, Bundle param);

    /**
     * Query the activation state
     *
     * @return Return the state
     *  0: STATE_NONE (no activation required)
     *  1: STATE_ACTIVATED
     *  2: STATE_NOT_ACTIVATED
     *  -1: STATE_UNKNOWN
     */
    public int getSelfActivateState();

    /**
     * Query the PCO5to0 state
     *
     * @return Return the state
     *  0: FivetoZero_NONE (no pco change form 5 to 0 required)
     *  1: FivetoZero_ACTIVATED
     *  -1: FivetoZero_UNKNOWN
     */
    public int getPCO520State();

    /**
     * Set required parameters, call buildParams() when
     * all parameters are set.
     *
     * @param context The context application reside.
     *
     *  @return The instance.
     */
    public ISelfActivation setContext(Context context);

    /**
     * Set required parameters, call buildParams() when
     * all parameters are set.
     *
     * @param ci The commands interface for vendor RIL.
     *
     * @return The instance.
     */
    public ISelfActivation setCommandsInterface(CommandsInterface ci);

    /**
     * All the parameters are ready and build the instance.
     *
     * @return The instance.
     */
    public ISelfActivation buildParams();
}
