/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2015. All rights reserved.
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

#ifndef __META_RAT_H_
#define __META_RAT_H_

#include "MetaPub.h"

#ifdef __cplusplus
extern "C" {
#endif

/* The TestCase Enum define of RAT_module */
typedef enum {
    RAT_OP_READ_OPTR = 0,
    RAT_OP_READ_OPTRSEG = 1,
    RAT_OP_GET_CURRENT_RAT = 2,
    RAT_OP_SET_NEW_RAT = 3,
    RAT_OP_END
} RAT_OP;

typedef struct{
    unsigned char m_setRatValue[64];
} RAT_SetRat_REQ_T;

typedef union {
    RAT_SetRat_REQ_T m_rat;
} META_RAT_CMD_U;

typedef struct {
    FT_H    header;  //module do not need care it
    RAT_OP  op;
    META_RAT_CMD_U  cmd;
} RAT_REQ;

typedef struct{
    unsigned char m_optr[64];
} RAT_OPTR_CNF_T;

typedef struct{
    unsigned char m_optrseg[64];
} RAT_OPTRSEG_CNF_T;

typedef struct{
    unsigned char m_RatValue[64];
} RAT_GetRat_CNF_T;

typedef struct{
    unsigned int m_Ret; //0:Fail; 1:Success and need reboot phone; 2:Success and no need to reboot
} RAT_SetRat_CNF_T;

typedef union {
    RAT_OPTR_CNF_T m_op;
    RAT_OPTRSEG_CNF_T m_seg;
    RAT_GetRat_CNF_T m_rat;
    RAT_SetRat_CNF_T m_set;
} META_RAT_CNF_U;

typedef struct {
    FT_H    header;  //module do not need care it
    RAT_OP    op;
    META_RAT_CNF_U   RAT_result;   //rat->FT
    int drv_status;
    unsigned int    status;
} RAT_CNF;

bool META_RAT_init();
void META_RAT_deinit();
void META_RAT_OP(RAT_REQ *req, char *peer_buff, unsigned short peer_len) ;

#ifdef __cplusplus
};
#endif

#endif
