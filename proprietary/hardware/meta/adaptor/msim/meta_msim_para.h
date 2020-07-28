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

#ifndef __META_MSIM_H_
#define __META_MSIM_H_


#ifdef __cplusplus
extern "C" {
#endif

/* The TestCase Enum define of MSIM_module */
typedef enum {
    MSIM_OP_GET_MSIM = 0,
    MSIM_OP_SET_MSIM = 1,
    MSIM_OP_END
} MSIM_OP;

typedef struct{
    char m_MsimValue[16];
} MSIM_SetMsim_REQ_T;

typedef union {
    MSIM_SetMsim_REQ_T m_msim;
} META_MSIM_CMD_U;

typedef struct {
    FT_H    header;  //module do not need care it
    MSIM_OP  op;
    META_MSIM_CMD_U  cmd;
} MSIM_REQ;

typedef struct{
    unsigned char m_MsimValue[16];
} MSIM_GetMsim_CNF_T;

typedef struct{
    unsigned int m_Ret; //0:Fail; 1:Success and need reboot phone; 2:Success and no need to reboot
} MSIM_SetMsim_CNF_T;

typedef union {
    MSIM_GetMsim_CNF_T m_getMsim;
    MSIM_SetMsim_CNF_T m_setMsim;
} META_MSIM_CNF_U;

typedef struct {
    FT_H    header;  //module do not need care it
    MSIM_OP    op;
    META_MSIM_CNF_U   MSIM_result;
    int drv_status;
    unsigned int    status;
} MSIM_CNF;

bool META_MSIM_init();
void META_MSIM_deinit();
void META_MSIM_OP(MSIM_REQ *req, char *peer_buff, unsigned short peer_len) ;

#ifdef __cplusplus
};
#endif

#endif
