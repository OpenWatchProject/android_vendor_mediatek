/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2017
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef _RCS_VOLTE_CORE_INTERFACE_AP_H
#define _RCS_VOLTE_CORE_INTERFACE_AP_H

#include "volte_core_interface.h"


#define roi_volte_sip_stack_create(i, c, cb)    _roi_volte_sip_stack_create((i),(c),(cb),__FILE__,__LINE__)

int (*roi_volte_sip_info_free)(VoLTE_Stack_Sip_Info_t*);
int (*roi_volte_sip_stack_init)();
void *(*_roi_volte_sip_stack_create)(VoLTE_Stack_Channel_Info_t*, VoLTE_Stack_Ua_Capability_t*, VoLTE_Stack_Callback_t*, char*, int);
int (*roi_volte_sip_stack_destroy)(void*);
int (*roi_volte_sip_stack_send_sip)(void *,VoLTE_Stack_Sip_Info_t *);
int (*roi_volte_sip_stack_send_msg)(void *,VoLTE_Stack_Message_t *);
int (*roi_volte_sip_stack_reg_state)(void *);
int (*roi_volte_sip_stack_reg_bind)(void *, int );
int (*roi_volte_sip_stack_reg_capability)(void *, int , char *);
int (*ROI_Rule_Capability_Init)(VoLTE_Stack_Ua_Capability_t*);
int (*ROI_Rule_Capability_Deinit)(VoLTE_Stack_Ua_Capability_t*);
int (*ROI_Rule_Level0_Set)(VoLTE_Stack_Ua_Capability_t*, char*, char*);
int (*ROI_Rule_Level1_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
int (*ROI_Rule_Level2_Set)(VoLTE_Stack_Ua_Capability_t*, const int , const int , char*);
int (*ROI_Rule_Level3_Set)(VoLTE_Stack_Ua_Capability_t*, char*);
#endif //_RCS_VOLTE_CORE_INTERFACE_AP_H
