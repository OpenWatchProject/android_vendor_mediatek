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
/*****************************************************************************
 * Include
 *****************************************************************************/
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic ignored "-Wunused-label"
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wformat"
#pragma GCC diagnostic ignored "-Wwritable-strings"
#pragma GCC diagnostic ignored "-Wswitch"
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#pragma GCC diagnostic ignored "-Wmacro-redefined"
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wignored-qualifiers"
#pragma GCC diagnostic ignored "-Wtautological-constant-out-of-range-compare"


#include "../../libc2kril/ril.cpp"
/*****************************************************************************
 * Name Space
 *****************************************************************************/
using android::s_commands;
using android::s_unsolResponses;
using android::CommandInfo;
using android::UnsolResponseInfo;
using android::s_c2k_local_commands;
using android::s_mtk_commands;
using android::s_c2k_local_unsolResponses;
using android::s_mtk_unsolResponses;

/*****************************************************************************
 * External Function
 *****************************************************************************/
extern "C" CommandInfo *RIL_getCommandInfo(int requestId) {
    int i;
    if (requestId >= RIL_LOCAL_REQUEST_VENDOR_C2K_BASE) {
        for (i = 0; i < (int32_t)NUM_ELEMS(s_c2k_local_commands); i++) {
            if (requestId == s_c2k_local_commands[i].requestNumber) {
                return &(s_c2k_local_commands[i]);
            }
        }
    } else if (requestId >= RIL_REQUEST_VENDOR_BASE) {
        for (i = 0; i < (int32_t)NUM_ELEMS(s_mtk_commands); i++) {
            if (requestId == s_mtk_commands[i].requestNumber) {
                return &(s_mtk_commands[i]);
            }
        }
    } else if (requestId >= 1) {
        for (i = 0; i < (int32_t)NUM_ELEMS(s_commands); i++) {
            if (requestId == s_commands[i].requestNumber) {
                return &(s_commands[i]);
            }
        }
    }
    return NULL;
}

extern "C" UnsolResponseInfo *RIL_getUrcInfo(int urc) {
    int i;
    if (urc >= RIL_LOCAL_C2K_UNSOL_VENDOR_BASE) {
        for (i = 0; i < (int32_t)NUM_ELEMS(s_c2k_local_unsolResponses); i++) {
            if (urc == s_c2k_local_unsolResponses[i].requestNumber) {
                return &s_c2k_local_unsolResponses[i];
            }
        }
    } else if (urc >= RIL_UNSOL_VENDOR_BASE) {
        for (i = 0; i < (int32_t)NUM_ELEMS(s_mtk_unsolResponses); i++) {
           if (urc == s_mtk_unsolResponses[i].requestNumber) {
               return &s_mtk_unsolResponses[i];
           }
        }
    } else {
        for (i = 0; i < (int32_t)NUM_ELEMS(s_unsolResponses); i++) {
            if (urc == s_unsolResponses[i].requestNumber) {
                return &s_unsolResponses[i];
            }
        }
    }
    return NULL;
}
