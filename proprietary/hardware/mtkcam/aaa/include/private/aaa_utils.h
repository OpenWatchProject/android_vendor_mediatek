/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein is
* confidential and proprietary to MediaTek Inc. and/or its licensors. Without
* the prior written permission of MediaTek inc. and/or its licensors, any
* reproduction, modification, use or disclosure of MediaTek Software, and
* information contained herein, in whole or in part, shall be strictly
* prohibited.
*
* MediaTek Inc. (C) 2010. All rights reserved.
*
* BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
* THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
* RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
* ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
* WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
* NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
* RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
* INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
* TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
* RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
* OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
* SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
* RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
* STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
* ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
* RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
* MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
* CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
* The following software/firmware and/or related documentation ("MediaTek
* Software") have been modified by MediaTek Inc. All revisions are subject to
* any receiver's applicable license agreements with MediaTek Inc.
*/

#ifndef _MTK_HARDWARE_MTKCAM_AAA_INCLUDE_PRIVATE_AAA_UTILS_H_
#define _MTK_HARDWARE_MTKCAM_AAA_INCLUDE_PRIVATE_AAA_UTILS_H_

#include <mtkcam/def/common.h>
#include <isp_tuning/isp_tuning.h>  //ESensorDev_T

/******************************************************************************
 *
 ******************************************************************************/
namespace NS3Av3
{


/******************************************************************************
 *
 ******************************************************************************/
MINT32 mapSensorIdxToDev(MINT32 idx);
MINT32 mapSensorDevToIdx(MINT32 devId);
MINT32 mapSensorIdxToSensorId(MINT32 idx);
MINT32 mapSensorDevToSensorId(MINT32 devId);
MINT32 mapSensorIdxToFace(MINT32 idx, MINT32& facing, MINT32& num); // face: 0 is back, 1 is front, num: The number of sensor on the same facing
                                                                    // e.g. main1: face = 0, num = 0
                                                                    // e.g. main2: face = 0, num = 1
                                                                    // e.g. sub1: face = 1, num = 0
                                                                    // e.g. sub2: face = 1, num = 1
MINT32 mapSensorDevToFace(MINT32 devId, MINT32& facing, MINT32& num);
};  //namespace
#endif //_MTK_HARDWARE_MTKCAM_AAA_INCLUDE_PRIVATE_AAA_UTILS_H_
