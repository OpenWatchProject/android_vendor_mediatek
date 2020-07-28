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
#ifndef __RTST_G_RIL_H__
#define __RTST_G_RIL_H__
/*****************************************************************************
 * Include
 *****************************************************************************/
#include <Parcel.h>
#include <telephony/mtk_ril.h>

/*****************************************************************************
 * Name Space
 *****************************************************************************/
using ::android::Parcel;

/*****************************************************************************
 * Class Declarations
 *****************************************************************************/
struct RequestInfo;

/*****************************************************************************
 * Typedef
 *****************************************************************************/

/**
 * The following enum & struct
 *   -WakeType
 *   -CommandInfo
 *   -UnsolResonseInfo
 *   -RequestInfo
 * are copied from ril.cpp in google libril.
 * We need these type to emulate the RIL request from RILJ in
 * test framework.
 */

enum WakeType {DONT_WAKE, WAKE_PARTIAL};

typedef struct {
    int requestNumber;
    void (*dispatchFunction) (Parcel &p, struct RequestInfo *pRI);
    int(*responseFunction) (Parcel &p, void *response, size_t responselen);
    RILChannelId gsmProxyId; // Share ril command between c2k/gsm
    RILChannelId proxyId;
} CommandInfo;

typedef struct {
    int requestNumber;
    int (*responseFunction) (Parcel &p, void *response, size_t responselen);
    WakeType wakeType;
} UnsolResponseInfo;

/*****************************************************************************
 * Class RfxTestGRil
 *****************************************************************************/

/*
 * Class used to access the command table in libril
 */
class RtstGRil {
// External Method
public:
    // Get the CommandInfo of RIL request.
    //
    // RETURNS: CommandInfo of the specified request ID.
    static CommandInfo* getCommandInfo(
        int requestId  // [IN] ril request ID
    );

    // Get the UnsolResponseInfo of the URC.
    //
    // RETURNS: UnsolResponseInfo the specified URC
    static UnsolResponseInfo *getUrcInfo(
        int urc        // [IN] urc ID
    );

    // Set the on request callback
    //
    // RETURNS: void
    static void setOnRequestCallback(
        RIL_RequestFunc onRequest   // [IN] on request callback
    );


    // Set the version number of RILD
    //
    // RETURNS: void
    static void setVersion(
        int version           // [IN] the version number
    );
};

#endif /* __RTST_G_RIL_H__ */