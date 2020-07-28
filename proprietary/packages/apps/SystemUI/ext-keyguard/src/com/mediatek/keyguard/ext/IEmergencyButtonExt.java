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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.keyguard.ext;

import android.content.Intent;
import android.view.View;

/**
 * This is for operator customization on EmergencyButton.
 */
public interface IEmergencyButtonExt {

    /**
     * For common, this function means if any sim in service
     * so that we can show emergency button.
     * This is a rule from CTA. However, operators may override it to customize this rule.
     *
     * For OP09, it only allow current sim of PIN/PUK lock to dial ecc,
     * so it returns whether current sim is in service.
     *
     * @param isServiceSupportEcc
     *          the boolean array of values of all sims service support Ecc button to show
     * @param slotId
     *          id of PIN/PUK lock current sim
     *
     * @return ture if sim is in service
     *
     * @internal
     */
    boolean showEccByServiceState(boolean[] isServiceSupportEcc, int slotId);

    /**
     * Add current sim slot id of PIN/PUK lock into intent
     * which starts emergency dialer.
     *
     * @param intent intent which starts emergency dialer
     * @param slotId current sim slot id of PIN/PUK lock
     *
     * @internal
     */
    void customizeEmergencyIntent(Intent intent, int slotId);

    /**
     * Whether we need show emergency button on slide screen.
     *
     * @return true if we need show emergency button
     *
     * @internal
     */
    boolean showEccInNonSecureUnlock();

    /**
     * Set the emergency call button's visibility.
     *
     * @param eccButtonView the emergency call button's view
     * @param alpha the view's alpha value
     *
     * @internal
     */
    void setEmergencyButtonVisibility(View eccButtonView, float alpha);
}