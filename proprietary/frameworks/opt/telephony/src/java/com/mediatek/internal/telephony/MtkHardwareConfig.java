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

import android.os.SystemProperties;

import com.android.internal.telephony.HardwareConfig;

public final class MtkHardwareConfig extends HardwareConfig {
    /**
     * default constructor.
     */
    public MtkHardwareConfig(int type) {
        super(type);
    }

    /**
     * create from a resource string format.
     */
    public MtkHardwareConfig(String res) {
        super(res);
    }

    /*
     * M: Query if modem supports the capability to handle IPv6 RA failure.
     *
     * @return true  - support, framework will keep AOSP logics.
     * @return false - not support, framework goes legacy platform logics.
     */
    public boolean hasRaCapability() {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return true;
        } else {
            return false;
        }
    }

    /*
     * M: Query if modem support deacting PDN capability for Multi-PS feature.
     *    For new MPS design, MD will deactivate all non-IMS PDN on old default SIM
     *    during default SIM switch and framework just follow AOSP to setup data call
     *    without checking data allow flag.
     * @return true  - support, framework will keep AOSP logics.
     * @return false - not support, framework goes legacy platform logics.
     */
    public boolean hasModemDeactPdnCapabilityForMultiPS() {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return true;
        } else {
            return false;
        }
    }

    /*
     * Query if modem support IA(Initial Attach) operator related capability.
     * In new design, MD should take care all of operator requirement,
     * and framework just follow AOSP to do set initial attach.
     * @return true  - support, framework will keep AOSP logics.
     * @return false - not support, framework goes legacy platform logics.
     */
    public boolean hasOperatorIaCapability() {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Check if IMS stack has the capability of parsing conference event package.
     * @return true true if IMS stack has the capability of parsing conference event package
     */
    public boolean hasParsingCEPCapability() {
        return hasMdAutoSetupImsCapability();
    }

    /// M: CC @{
    /**
     * Check if has AT+EVASMOD need to have 2nd parameter for extended preferred mode
     * @return true for support
     */
    public boolean hasC2kOverImsModem() {
        if (SystemProperties.get("ro.vendor.mtk_ril_mode").equals("c6m_1rild")) {
            return true;
        } else {
            return false;
        }
    }
    /// @}

    /*
     * M: Query if modem support auto setup ims pdn capability.
     *    In new design, MD should take care ims pdn interactions with imcb,
     *    and framework just follow AOSP to do.
     * @return true  - support, framework will keep AOSP logics.
     * @return false - not support, framework goes legacy platform logics.
     */
    public boolean hasMdAutoSetupImsCapability() {
        if (SystemProperties.get("ro.vendor.md_auto_setup_ims").equals("1")) {
            return true;
        } else {
            return false;
        }
    }

    /**
     * Check if modem supports CDMA 3g dual activation.
     * @return true for support and false for non-support.
     */
    public boolean isCdma3gDualActivationSupported() {
        if (SystemProperties.get("vendor.ril.cdma.3g.dualact").equals("1")) {
            return true;
        } else {
            return false;
        }
    }
}
