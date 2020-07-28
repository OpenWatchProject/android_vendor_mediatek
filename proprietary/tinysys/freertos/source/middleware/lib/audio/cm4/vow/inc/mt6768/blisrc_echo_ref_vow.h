/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2019
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

/*******************************************************************************
 *
 * Filename:
 * ---------
 *   blisrc_echo_ref_vow.h
 *
 * Project:
 * --------
 *
 *
 * Description:
 * ------------
 *   BLISRC Interface for VOW Echo Reference
 *
 * Author:
 * -------
 *   Scholar Chang
 *
 *==============================================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *==============================================================================
 *******************************************************************************/

#ifndef __BLISRC_ECHO_REF_VOW_H__
#define __BLISRC_ECHO_REF_VOW_H__

#include <stdint.h>

typedef struct {
    uint32_t input_sampling_rate;
    uint32_t output_sampling_rate;
    uint32_t input_channel_number;
    uint32_t output_channel_number;
} blisrc_echo_ref_vow_init_param_t;

/********************************************************************************************\
|   Function:                                                                                |
|       blisrc_echo_ref_vow_get_buffer_size                                                  |
|                                                                                            |
|   Description:                                                                             |
|       Get the required internal buffer size for BLISRC ECHO REF VOW SWIP.                  |
|                                                                                            |
|   Return Value:                                                                            |
|       Internal buffer size (unit: byte)                                                    |
|                                                                                            |
\********************************************************************************************/
uint32_t blisrc_echo_ref_vow_get_buffer_size (void);

/********************************************************************************************\
|   Function:                                                                                |
|       blisrc_echo_ref_vow_open                                                             |
|                                                                                            |
|   Description:                                                                             |
|       Open BLISRC ECHO REF VOW SWIP handle with the given initial parameters.              |
|                                                                                            |
|   Parameters:                                                                              |
|       p_handle        Output  Pointer to the handle of the SWIP                            |
|       p_internal_buf  Input   Pointer to the internal buffer                               |
|       p_param         Input   Pointer to the initial parameter structure                   |
|                                                                                            |
|   Return Value:                                                                            |
|       Error code                                                                           |
|       >= 0    normal                                                                       |
|       <  0    error                                                                        |
|                                                                                            |
\********************************************************************************************/
int32_t blisrc_echo_ref_vow_open (void **p_handle, uint8_t *p_internal_buf, blisrc_echo_ref_vow_init_param_t *p_param);

/********************************************************************************************\
|   Function:                                                                                |
|       blisrc_echo_ref_vow_process                                                          |
|                                                                                            |
|   Description:                                                                             |
|       Process the BLISRC ECHO REF VOW SWIP.                                                |
|                                                                                            |
|   Parameters:                                                                              |
|       handle              Input   Handle of the SWIP                                       |
|       p_in_buf            Input   Pointer to the input buffer                              |
|       available_in_size   Input   Available input buffer size (unit: byte)                 |
|                           Output  Consumed input buffer size (unit: byte)                  |
|       p_ou_buf            Input   Pointer to the output buffer                             |
|       available_ou_size   Input   Available output buffer size (unit: byte)                |
|                           Output  Produced input buffer size (unit: byte)                  |
|                                                                                            |
|   Return Value:                                                                            |
|       Error code                                                                           |
|       >= 0    normal                                                                       |
|       <  0    error                                                                        |
|                                                                                            |
\********************************************************************************************/
int32_t blisrc_echo_ref_vow_process (void *handle, int16_t *p_in_buf, uint32_t *available_in_size, int16_t *p_ou_buf, uint32_t *available_ou_size);

#endif  /* __BLISRC_ECHO_REF_VOW_H__ */
