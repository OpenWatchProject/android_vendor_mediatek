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

/**
* @file IHal3ACb.h
* @brief Declarations of Abstraction of 3A Hal Callback Class and Top Data Structures
*/

#ifndef __IHAL_3A_CB_V3_H__
#define __IHAL_3A_CB_V3_H__

#include <mtkcam/def/common.h>


namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IHal3ACb
{
public:
    virtual             ~IHal3ACb() {}

public:
    virtual void        doNotifyCb (
                            MINT32  _msgType,
                            MINTPTR _ext1,
                            MINTPTR _ext2,
                            MINTPTR _ext3
                        ) = 0;

    enum ECb_T
    {
        eID_NOTIFY_3APROC_FINISH    = 0,
        /*
            _ext1: magic number of current request
            _ext2: bit[0] OK/ERROR; bit[1] Init Ready
            _ext3: magic number of current used statistic magic number
        */

        eID_NOTIFY_READY2CAP        = 1,

        eID_NOTIFY_CURR_RESULT      = 2,
        /*
            _ext1: magic number of current result
            _ext2: metadata tag (key)
            _ext3: value
        */
        eID_NOTIFY_AE_RT_PARAMS     = 3,
        /*
            _ext1: pointer of RT params.
            _ext2:
            _ext3:
        */
        eID_NOTIFY_VSYNC_DONE       = 4,
        eID_NOTIFY_HDRD_RESULT      = 5,
        eID_NOTIFY_LCS_ISP_PARAMS   = 6,
        /*
            _ext1: magic number of current result
            _ext2: pointer of LCS params.
        */
        eID_NOTIFY_AF_FSC_INFO      = 7,
        /*
            _ext1: AF FSC INFO struct
        */
        eID_MSGTYPE_NUM
    };
};

}; // namespace NS3Av3

#endif
