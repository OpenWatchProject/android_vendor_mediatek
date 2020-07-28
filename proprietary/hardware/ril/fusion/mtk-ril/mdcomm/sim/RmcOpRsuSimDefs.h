/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2016. All rights reserved.
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
#ifndef __RMC_SIM_OP_DEFS_H__
#define __RMC_SIM_OP_DEFS_H__

/**
 * Operator Id
 */
typedef enum {          // Operator ID
    RSU_OP_VZW = 1,     // Verizon
    RSU_OP_KDDI = 2,    // KDDI
    RSU_OP_TMO = 3,     // TMO RSU
} RIL_RSU_OPERATOR;

typedef enum {                        // Request function ID
    RSU_REQUEST_INIT_REQUEST = 0,     // VzW only now
    RSU_REQUEST_GET_SHARED_KEY,       // VzW(HMAC), TMO/ATT(SHARED KEY)
    RSU_REQUEST_UPDATE_LOCK_DATA,     // VzW, TMO, ATT, RJIO
    RSU_REQUEST_GET_LOCK_VERSION,     // VzW, TMO, ATT, RJIO
    RSU_REQUEST_RESET_LOCK_DATA,      // TMO, ATT
    RSU_REQUEST_GET_LOCK_STATUS,      // VzW, TMO, ATT, RJIO
    RSU_REQUEST_UNLOCK_TIMER = 50,    // VzW only now for delay timer. Not defined request
                                      // index by customer, extension start from index 50
    RSU_REQUEST_SET_SIM_LOCK_DATA,    // KDDI
    RSU_REQUEST_SET_GET_ALLOWED_LOCK_DATA,    // KDDI
    RSU_REQUEST_SET_GET_EXCLUDED_LOCK_DATA,    // KDDI
} RIL_RSU_REQUEST;

typedef enum {                            // Request function ID
    RSU_MODEM_REQUEST_INIT_REQUEST = 1,   // VzW only now
    RSU_MODEM_REQUEST_UPDATE_LOCK_DATA,   // VzW, TMO, ATT, RJIO
    RSU_MODEM_REQUEST_GET_LOCK_STATUS,    // VzW, TMO, ATT, RJIO
    RSU_MODEM_REQUEST_UNLOCK_TIMER,       // VzW only now for delay timer. Not defined request
                                          // index by customer, extension start from index 50
    RSU_MODEM_REQUEST_GET_SHARED_KEY,     // VzW(HMAC), TMO/ATT(SHARED KEY)
    RSU_MODEM_REQUEST_GET_LOCK_VERSION,   // VzW, TMO, ATT, RJIO
    RSU_MODEM_REQUEST_GET_LOCK_DATA,   // KDDI
    RSU_MODEM_REQUEST_SET_LOCK_DATA_MORE,   // KDDI
    RSU_MODEM_REQUEST_SET_LOCK_DATA_DONE,   // KDDI
} RIL_RSU_MODEM_REQUEST;
#endif /* __RMC_SIM_OP_DEFS_H__ */
