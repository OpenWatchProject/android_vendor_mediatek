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

package com.mediatek.internal.telephony.uicc;

public class IccServiceInfo {

    //RuimRecords, SIMRecords and UiccCardApplication use it.
    public enum IccServiceStatus {
        NOT_EXIST_IN_SIM,
        NOT_EXIST_IN_USIM,
        ACTIVATED,
        INACTIVATED,
        UNKNOWN;
    }
    //RuimRecords, SIMRecords, UiccCardApplication use it.
    public enum IccService {
        CHV1_DISABLE_FUNCTION,      //(0)CHV1 disable function (SIM only)
        SPN,                        //(1)Service Provider Name
        PNN,                        //(2)PLMN Network Name
        OPL,                        //(3)Operator PLMN List
        MWIS,                       //(4)Message Waiting Indication Status
        CFIS,                       //(5)Call Forwarding Indication Status
        SPDI,                       //(6)Service Provider Display Information
        EPLMN,                      //(7)Equivalent HPLMN (USIM only)
        SMSP,                       //(8)[ALPS01206315]Short Message Service Parameters
        FDN,                        //(9)FDN
        //MTK-START: ATT UICC requirement
        PLMNsel,                    //(10)PLMN Selector
        OPLMNwACT,
        //MTK-END
        UNSUPPORTED_SERVICE;        //(10)

        public int getIndex() {
            int nIndex = -1;
        switch(this) {
        case CHV1_DISABLE_FUNCTION:
            nIndex = 0;
            break;
        case SPN:
            nIndex = 1;
            break;
        case PNN:
            nIndex = 2;
            break;
        case OPL:
            nIndex = 3;
            break;
        case MWIS:
            nIndex = 4;
            break;
        case CFIS:
            nIndex = 5;
            break;
        case SPDI:
            nIndex = 6;
            break;
        case EPLMN:
            nIndex = 7;
            break;
        case SMSP:
            nIndex = 8;
            break;
        case FDN:
            nIndex = 9;
            break;
        //MTK-START: ATT UICC requirement
        case PLMNsel:
            nIndex = 10;
            break;
        case OPLMNwACT:
            nIndex = 11;
            break;
        //MTK-END
        case UNSUPPORTED_SERVICE:
            nIndex = 10;
            break;
        default:
            break;
        }
            return nIndex;
        }
    }
}
