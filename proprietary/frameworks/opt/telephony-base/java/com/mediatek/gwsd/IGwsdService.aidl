/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.gwsd;

import com.mediatek.gwsd.IGwsdListener;

interface IGwsdService {

    /**
     * register callback to listen event from service
     * @param IGwsdListener: a callback
     */
    void addListener(IGwsdListener listener);

    /**
     * unRegister callback
     */
    void removeListener();

    /**
     * turn on /off user selection mode
     * @param action false is deactivation, true is activation
     */
    void setUserModeEnabled(boolean action);

    /**
     * turn on /off auto rejection mode
     * @param action false is deactivation, true is activation
     */
    void setAutoRejectModeEnabled(boolean action);

    /**
     * Set call forwarding from non-default data SIM card to default data SIM card
     * It's an API for the feature of Gaming with Smooth LTE Mobile Data (GWSD).
     *
     * @param userEnable     false is deactivation, true is activation
     * @param autoReject     false is deactivation, true is activation
     */
    void syncGwsdInfo(boolean userEnable, boolean autoReject);

    /**
     * Set timer for call valid
     * @param timer 1 ~ 255sec(default 7sec)
     */
    void setCallValidTimer(int timer);

    /**
     * Set interval for ignore same phone number
     * @param interval 1 ~ 255sec(default 3sec)
     */
    void setIgnoreSameNumberInterval(int interna);

    /**
     * GwsdDaulSim
     * @param action
     * action=false: disable GwsdDaulSim
     * action=true: enable GwsdDaulSim
     */
    void setGwsdDualSimEnabled(boolean action);

    /**
     * Check if GwsdDaulSim data available.
     * @param boolean gwsdDualSimStatus
     */
    boolean isDataAvailableForGwsdDualSim(boolean gwsdDualSimStatus);
}
