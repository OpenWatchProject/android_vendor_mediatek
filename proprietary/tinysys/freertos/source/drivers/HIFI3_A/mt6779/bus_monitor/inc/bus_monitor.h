/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2018
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

#ifndef __BUS_MONITOR_H__
#define __BUS_MONITOR_H__

#include <stdint.h>

/*****************************************************************************\
|   Function:                                                                 |
|       bus_monitor_init                                                      |
|                                                                             |
|   Description:                                                              |
|       Initialize the bus monitor driver.                                    |
|                                                                             |
\*****************************************************************************/
void bus_monitor_init (void);

/*****************************************************************************\
|   Function:                                                                 |
|       bus_monitor_dump                                                      |
|                                                                             |
|   Description:                                                              |
|       Dump bus monitor registers and status to log.                         |
|                                                                             |
\*****************************************************************************/
void bus_monitor_dump (void);

/*****************************************************************************\
|   Function:                                                                 |
|       is_bus_monitor_triggered                                              |
|                                                                             |
|   Description:                                                              |
|       Check if bus monitor handler is triggered or not.                     |
|                                                                             |
|   Return Values:                                                            |
|       0 --> Bus monitor handler is not triggered.                           |
|       1 --> Bus monitor handler is triggered.                               |
|                                                                             |
\*****************************************************************************/
int32_t is_bus_monitor_triggered (void);

/*****************************************************************************\
|   Function:                                                                 |
|       bus_monitor_2nd_stage_handler                                         |
|                                                                             |
|   Description:                                                              |
|       Handle the bus monitor 2nd stage timeout                              |
|                                                                             |
\*****************************************************************************/
void bus_monitor_2nd_stage_handler(void);

#endif /* __BUS_MONITOR_H__ */
