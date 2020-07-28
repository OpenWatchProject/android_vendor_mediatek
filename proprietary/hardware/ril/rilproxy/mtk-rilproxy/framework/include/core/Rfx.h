/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
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
/*
 * File name:  Rfx.h
 * Author: Jun Liu (MTK80064)
 * Description:
 * Define interfaces to initialize RIL Proxy framework and enqueue messages from
 * external modules
 */

#ifndef __RFX_H__
#define __RFX_H__

#include <telephony/mtk_ril.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void rfx_init(void);

extern void rfx_enqueue_request_message(int request, void *data, size_t datalen,
        RIL_Token t, RIL_SOCKET_ID socket_id);

extern void rfx_enqueue_request_message_old(int request, int slotId,
        RILD_RadioTechnology_Group dest, int token, void *data);

extern void rfx_enqueue_client_request_message(int request, int slotId,
            RILD_RadioTechnology_Group dest, int token, void *data, int clientId);

extern void rfx_process_raw_data(RILD_RadioTechnology_Group source, int slotId, void *data,
        int datalen);

extern void rfx_set_socket(RILD_RadioTechnology_Group group, int slotId, int fd);

extern void rfx_close_socket(RILD_RadioTechnology_Group group, int slotId);

extern void rfx_set_sap_socket(RIL_SOCKET_ID socketId, RILD_RadioTechnology_Group group,
        int fd);

extern void rfx_sap_to_rild_socket(void * data, int datalen, RIL_SOCKET_ID socketId);

extern void rfx_update_connection_state(RIL_SOCKET_ID socketId, int isConnected);
#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // __RFX_H__

