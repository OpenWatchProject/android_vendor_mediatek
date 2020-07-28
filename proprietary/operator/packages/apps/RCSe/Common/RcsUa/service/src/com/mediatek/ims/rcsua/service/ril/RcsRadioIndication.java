/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2014. All rights reserved.
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

package com.mediatek.ims.rcsua.service.ril;

import com.mediatek.ims.rcsua.service.RoiHandler;
import com.mediatek.ims.rcsua.service.ril.RcsRIL;
import com.mediatek.ims.rcsua.service.utils.Logger;
import vendor.mediatek.hardware.radio_op.V2_0.IRadioOp;
import vendor.mediatek.hardware.radio_op.V2_0.IRcsRadioIndication;

public class RcsRadioIndication extends IRcsRadioIndication.Stub {

    private final static String TAG = "RcsRadioIndication";
    private Logger logger = Logger.getLogger(this.getClass().getName());

    private RcsRIL mRil;
    private int mPhoneId;

    public RcsRadioIndication(RcsRIL ril, int phoneId) {
        mRil = ril;
        mPhoneId = phoneId;
        logger.debug("RcsRadioIndication, phone = " + mPhoneId);
    }

    /*
     * Indication for IMS VoPS
     * RIL_UNSOL_IMSVOPS_INDICATION
     *
     * @param vops IMS VoPS value
     */
    public void imsVoPSIndication(int indicationType, int vops) {
        logger.debug("imsVoPSIndication vops: " + vops);
        RoiHandler.getInstance().notifyVoPSChanged(vops==1?true:false);
    }

    /*
     * Indication for Digits Line Information (activation/deactivation)
     * RIL_UNSOL_RCS_DIGITS_LINE_INFO
     *
     * @param digitsinfo content of AT+DIGITSLINE
     */
    public void digitsLineIndication(int indicationType, String digitsinfo) {
        logger.debug("digitsLineIndication digitsinfo: " + digitsinfo);
        RoiHandler.getInstance().sendRegDigitLineInfo(digitsinfo);
    }
}
