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
 * MediaTek Inc. (C) 2018. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IFVCONTAINER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IFVCONTAINER_H_

#include <utils/RefBase.h> // android::RefBase

#include <vector> // std::vector
#include <cstdint> // int32_t, int64_t, intptr_t

#include <mtkcam/def/common.h> //MBOOL
#include <mtkcam/aaa/aaa_hal_common.h>

using android::sp;
using std::vector;


#define FV_DATATYPE NS3Av3::AF_FRAME_INFO_T


namespace NSCam {
class IFVContainer : public android::RefBase {
/* enums */
public:
    enum eFVContainer_Opt{
        eFVContainer_Opt_Read       = 0x1,
        eFVContainer_Opt_Write      = 0x1<<1,
        eFVContainer_Opt_RW         = eFVContainer_Opt_Read|eFVContainer_Opt_Write,
    };

/* interfaces */
public:

    /**
     *  For eFVContainer_Opt_Read
     *  To get all avaliable focus values
     */
    virtual vector<FV_DATATYPE> query(
            void
            ) = 0;

    /**
     *  For eFVContainer_Opt_Read
     *  To get the focus values in range [mg_start, mg_end]
     *  @param mg_start     magicNum from
     *  @param mg_end       magicNum until
     *
     *  magicNum is halMeta::MTK_P1NODE_PROCESSOR_MAGICNUM
     */
    virtual vector<FV_DATATYPE> query(
            const int32_t&                  mg_start,
            const int32_t&                  mg_end
            ) = 0;

    /**
     *  For eFVContainer_Opt_Read
     *  To get the focus values in the giving set
     *  i-th return value = FV_DATA_ERROR if the timestamps vecMgs[i] is not found
     *  @param vecMgs       a set of magicNum
     *
     *  magicNum is halMeta::MTK_P1NODE_PROCESSOR_MAGICNUM
     */
    virtual vector<FV_DATATYPE> query(
            const vector<int32_t>&          vecMgs
            ) = 0;


    /**
     *  For eFVContainer_Opt_Write
     *  To push focus value into fv container and assign the key as input magicNum
     *  @param magicNum     the unique key for query focus values
     *  @param fv           the focus value
     *
     *  magicNum is halMeta::MTK_P1NODE_PROCESSOR_MAGICNUM
     */
    virtual MBOOL push(
            int32_t                         magicNum,
            FV_DATATYPE                     fv
            ) = 0;

    /**
     *  To clear all focus values
     */
    virtual void clear(
            void
            ) = 0;

    /**
     *  To dump all focus values
     */
    virtual void dumpInfo(
            void
            ) = 0;

public:
    static sp<IFVContainer> createInstance(char const* userId, eFVContainer_Opt opt);


    ~IFVContainer(){};
};
}; /* namespace NSCam */

#endif//_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IFVCONTAINER_H_