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
 * MediaTek Inc. (C) 2019. All rights reserved.
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
#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IBSSCONTAINER_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IBSSCONTAINER_H_


#include <utils/RefBase.h> // android::RefBase

#include <vector> // std::vector
#include <cstdint> // int64_t, intptr_t

#include <mtkcam/def/common.h> //MBOOL

// ALGORITHM
#include <MTKBss.h>

using android::sp;
using std::vector;


struct MTKBssContainerInfo {
    BSS_OUTPUT_DATA bssdata;
    int64_t timestamps[MAX_FRAME_NUM];
    MTKBssContainerInfo ()
    {
        memset(&bssdata,   0, sizeof(BSS_OUTPUT_DATA));
        memset(&timestamps[0], 0, MAX_FRAME_NUM*sizeof(int64_t));
    };
    ~MTKBssContainerInfo () {};

    void clone(MTKBssContainerInfo& cloneInfo)
    {
        cloneInfo = *this;
    }
};
#define BSS_DATATYPE MTKBssContainerInfo



namespace NSCam {
class IBssContainer : public android::RefBase {
/* enums */
public:
    enum eBssContainer_Opt{
        eBssContainer_Opt_Read      = 0x1,
        eBssContainer_Opt_Write     = 0x1<<1,
        eBssContainer_Opt_RW        = eBssContainer_Opt_Read|eBssContainer_Opt_Write,
    };

/* interfaces */
public:

    /**
     *  For eBssContainer_Opt_Read
     *  To get the bss info for specific timestamp
     *  @param uniqueKey    specific uniqueKey for queried bss info
     *  @param timeout      return nullptr when timeout, < 0 means no timeout
     *
     *  notice:             the memory of bss info is managed by IBssContainer
     *                      others please don't delete it
     */
    virtual BSS_DATATYPE* queryLock(
            int64_t                         uniqueKey,
            int64_t                         timeout = 0
            ) = 0;

    /**
     *  For eBssContainer_Opt_Read
     *  To unregister the usage of a set of bss infos
     *  @param info         the bss info get from queryLock
     *
     *  notice:             the memory of bss info is managed by IBssContainer
     *                      others please don't delete it
     */
    virtual MBOOL queryUnlock(
                BSS_DATATYPE*               info
            ) = 0;


    /**
     *  For eBssContainer_Opt_Write
     *  To get the bss info for edit and assign the key as input timestamp
     *  @param uniqueKey    the unique key for query bss info
     *
     *  notice:             the memory of bss info is managed by IBssContainer
     *                      others please don't delete it
     */
    virtual BSS_DATATYPE* editLock(
            int64_t                         uniqueKey
            ) = 0;

    /**
     *  For eBssContainer_Opt_Write
     *  To publish the bss info editing
     *  @param info         the bss info get from editLock
     *
     *  notice:             the memory of bss info is managed by IBssContainer
     *                      others please don't delete it
     */
    virtual MBOOL editUnlock(
            BSS_DATATYPE*                   info
            ) = 0;

    /**
     *  To dump all bss infos
     */
    virtual void dumpInfo(
            void
            ) = 0;

public:
    static sp<IBssContainer> createInstance(char const* userId, eBssContainer_Opt opt);


    ~IBssContainer(){};
};
}; /* namespace NSCam */

#endif//_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IBSSCONTAINER_H_
