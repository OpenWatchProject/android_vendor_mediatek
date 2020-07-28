/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _DBG_CAM_RESERVEC_PARAM_H
#define _DBG_CAM_RESERVEC_PARAM_H

//ReserveC Parameter Structure
typedef enum
{
    RESERVEC_TAG_VERSION = 0,
    /* add tags here */
    
    RESERVEC_TAG_END
}DEBUG_RESERVEC_TAG_T;

// TEST_C debug info
#define RESERVEC_DEBUG_TAG_SIZE     (RESERVEC_TAG_END+10)
#define RESERVEC_DEBUG_TAG_VERSION  (0)

//gmv
#define MF_MAX_FRAME            8
enum {MF_GMV_DEBUG_TAG_GMV_X
    , MF_GMV_DEBUG_TAG_GMV_Y
    , MF_GMV_DEBUG_TAG_ITEM_SIZE
    };
#define MF_GMV_DEBUG_TAG_SIZE               (MF_GMV_DEBUG_TAG_ITEM_SIZE)

//eis
#define MF_EIS_DEBUG_TAG_WINDOW             32
enum {MF_EIS_DEBUG_TAG_MV_X    
    , MF_EIS_DEBUG_TAG_MV_Y
    , MF_EIS_DEBUG_TAG_TRUST_X
    , MF_EIS_DEBUG_TAG_TRUST_Y
    , MF_EIS_DEBUG_TAG_ITEM_SIZE
    };
#define MF_EIS_DEBUG_TAG_SIZE               (MF_EIS_DEBUG_TAG_WINDOW*MF_EIS_DEBUG_TAG_ITEM_SIZE)

struct DEBUG_RESERVEC_INFO_T {
    const MUINT32 count;
    const MUINT32 gmvCount;
    const MUINT32 eisCount;
    const MUINT32 gmvSize;
    const MUINT32 eisSize;
    MINT32  gmvData[MF_MAX_FRAME][MF_GMV_DEBUG_TAG_ITEM_SIZE];
    MUINT32 eisData[MF_MAX_FRAME][MF_EIS_DEBUG_TAG_WINDOW][MF_EIS_DEBUG_TAG_ITEM_SIZE];

    DEBUG_RESERVEC_INFO_T() : count(2)  //gmvCount + eisCount
                            //
                            , gmvCount(MF_MAX_FRAME)
                            , eisCount(MF_MAX_FRAME)
                            //
                            , gmvSize(MF_GMV_DEBUG_TAG_SIZE)
                            , eisSize(MF_EIS_DEBUG_TAG_SIZE)
    {
        
    }
};


#endif //_DBG_CAM_TESTC_PARAM_H
