/* Copyright Statement:
*
* This oftware/firmware and related documentation ("MediaTek Software") are
* proteed under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MedTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
* AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESLY DISCLAIMS ANY AND ALL WARRANTIES,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
* NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESCT TO THE
* SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
* SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
* THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
* THAT IT IS RECER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
* CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALLALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
* SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
* CUMULATIVE LIABILITY WITH RESPECT TO THEEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
* AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE T ISSUE,
* OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
* MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*/

#include<platform_mtk.h>

#define DO_ID_LENGTH 32
#define DO_MAGIC "MKDynObj"
#define DO_MAGIC_LENGTH 8

typedef struct do_part {
    char magic[DO_MAGIC_LENGTH];
    char id[DO_ID_LENGTH];
    uint32_t len;
    uint32_t feat_list_len;
} DO_part;

enum DoMsgType{
    DO_MSG_INFO = 1,
    DO_MSG_GET_INFO,
    DO_MSG_LOAD_MODULE,
    DO_MSG_UNLOAD_MODULE,
    DO_MSG_UPDATE_CURRENT_DO,
    DO_MSG_DRAM_ADDR,
    DO_MSG_DO_DISABLED = 99
};

typedef struct do_msg{
    uint32_t scp_num;
    uint32_t type;
    uint32_t len;
    uint8_t payload;
} DoMsgHeader_t;

typedef struct do_header{
    //uint32_t do_num;
    uint8_t id[DO_ID_LENGTH];
    uint32_t dram_addr;
    uint32_t sram_addr;
    uint32_t size;
    uint32_t featlist_len;
}DOHeader_t;

typedef struct do_infos{
    uint32_t num_of_do;
    DOHeader_t headers;
}DOInfo_t;


