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

#ifndef MTK_RILOP_H
#define MTK_RILOP_H

#include <telephony/ril.h>

#ifndef MTK_TELEPHONYWARE_SUPPORT
#include <utils/Log.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Used by RIL_REQUEST_DIAL_FROM */
typedef struct {
    char * address;
    char * fromAddress;
    int clir;
            /* (same as 'n' paremeter in TS 27.007 7.7 "+CLIR"
             * clir == 0 on "use subscription default value"
             * clir == 1 on "CLIR invocation" (restrict CLI presentation)
             * clir == 2 on "CLIR suppression" (allow CLI presentation)
             */
    bool isVideoCall;
} RIL_DialFrom;

typedef struct {           // Default -1 for int and null for char*
    int opId;              // Operator ID
    int requestId;         // Request function ID
    int requestType;       // Reques type
    char* data;            // In hex string format ([a-fA-F0-9]*)
    int reserveInt1;       // RFU
    int reserveInt2;       // RFU
    char* reserveString1;  // RFU
} RIL_RsuRequestInfo;

typedef struct {           // Default -1 for int and null for char*
    int opId;              // Operator ID
    int requestId;         // Request function ID
    int errCode;           // Error code
    char* data;            // In hex string format ([a-fA-F0-9]*)
    long long time;        // Currently used to indicate temporary unlock time remaining (s)
    int version;           // 2 x MSB for major and 2 x LSB for minor
    int status;            // Lock status
    int reserveInt1;       // RFU
    int reserveInt2;       // RFU
    char* reserveString1;  // RFU
} RIL_RsuResponseInfo;

#define MAX_TRN_LENGTH 100
#define GET_TRN_TIMEOUT 30000  // Set timeout as 30s temporarily

#define MAX_INCOMING_LINE_LENGTH 40
#define INCOMING_LINE_TIMEOUT 30000   // Set timeout as 30s temporarily


/*********************************************************************************/
/*  Vendor request                                                               */
/*********************************************************************************/
#define RIL_REQUEST_VENDOR_OP_BASE 11000

#define RIL_REQUEST_SET_DIGITS_LINE (RIL_REQUEST_VENDOR_OP_BASE + 1)
#define RIL_REQUEST_SET_TRN (RIL_REQUEST_VENDOR_OP_BASE + 2)
#define RIL_REQUEST_DIAL_FROM (RIL_REQUEST_VENDOR_OP_BASE + 3)
#define RIL_REQUEST_SEND_USSI_FROM (RIL_REQUEST_VENDOR_OP_BASE + 4)
#define RIL_REQUEST_CANCEL_USSI_FROM (RIL_REQUEST_VENDOR_OP_BASE + 5)
#define RIL_REQUEST_SET_EMERGENCY_CALL_CONFIG (RIL_REQUEST_VENDOR_OP_BASE + 6)
#define RIL_REQUEST_SET_DISABLE_2G (RIL_REQUEST_VENDOR_OP_BASE + 7)
#define RIL_REQUEST_GET_DISABLE_2G (RIL_REQUEST_VENDOR_OP_BASE + 8)
#define RIL_REQUEST_DEVICE_SWITCH (RIL_REQUEST_VENDOR_OP_BASE + 9)
#define RIL_REQUEST_CANCEL_DEVICE_SWITCH (RIL_REQUEST_VENDOR_OP_BASE + 10)
#define RIL_REQUEST_SET_INCOMING_VIRTUAL_LINE (RIL_REQUEST_VENDOR_OP_BASE + 11)
#define RIL_REQUEST_SET_DIGITS_REG_STATUS (RIL_REQUEST_VENDOR_OP_BASE + 12)
#define RIL_REQUEST_EXIT_SCBM (RIL_REQUEST_VENDOR_OP_BASE + 13)
#define RIL_REQUEST_SEND_RSU_REQUEST (RIL_REQUEST_VENDOR_OP_BASE + 14)
#define RIL_REQUEST_SWITCH_RCS_ROI_STATUS (RIL_REQUEST_VENDOR_OP_BASE + 15)
#define RIL_REQUEST_UPDATE_RCS_CAPABILITIES (RIL_REQUEST_VENDOR_OP_BASE + 16)
#define RIL_REQUEST_UPDATE_RCS_SESSION_INFO (RIL_REQUEST_VENDOR_OP_BASE + 17)
/*********************************************************************************/
/*  Vendor unsol                                                                 */
/*********************************************************************************/
#define RIL_UNSOL_VENDOR_OP_BASE 12000

#define RIL_UNSOL_DIGITS_LINE_INDICATION (RIL_UNSOL_VENDOR_OP_BASE + 1)
#define RIL_UNSOL_GET_TRN_INDICATION (RIL_UNSOL_VENDOR_OP_BASE + 2)
#define RIL_UNSOL_RCS_DIGITS_LINE_INFO (RIL_UNSOL_VENDOR_OP_BASE + 3)
#define RIL_UNSOL_ENTER_SCBM (RIL_UNSOL_VENDOR_OP_BASE + 4)
#define RIL_UNSOL_EXIT_SCBM (RIL_UNSOL_VENDOR_OP_BASE + 5)
#define RIL_UNSOL_RSU_EVENT (RIL_UNSOL_VENDOR_OP_BASE + 6)

#ifdef __cplusplus
}
#endif

#endif /*MTK_RILOP_H*/
