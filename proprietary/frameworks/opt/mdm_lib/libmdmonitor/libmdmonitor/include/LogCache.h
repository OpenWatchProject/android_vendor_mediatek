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
 * @file LogCache.h
 *
 * @author 
 *
 * @brief The header file defines @c LogCache class used to cache msg log information.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __LOG_CACHE_H__
#define __LOG_CACHE_H__
#include <string>
#include <map>
#include <vector>
#include <set>
#include "libmdmonitor.h"
namespace libmdmonitor {

#define LOG_CACHE_SIZE 6

/**
 * @author mtk80144 (Yaling Fang)
 *
 * @brief The class is used to cache log msg information.
 */
class LogCache
{
public:
    /**
     * @brief Modem DHL frame log Cache Information
     */
    struct DHL_MSG_INFO_CACHE{
        /**
         * @brief The log cache structure
         */
        uint64_t rtostime[LOG_CACHE_SIZE];
        uint64_t msgId[LOG_CACHE_SIZE];
        uint64_t traceId[LOG_CACHE_SIZE];
        uint32_t simIdx[LOG_CACHE_SIZE];
        size_t len[LOG_CACHE_SIZE];
        size_t cache_index;
        unsigned char dhlFrameType[LOG_CACHE_SIZE];
    };
    void PrintLogCacheinfo(unsigned char dhlFrameType, uint64_t rtosTime, size_t len, uint32_t simIdx, uint64_t msgId, uint64_t traceId);
     /**
     * @brief LogCache destructor.
     */
    LogCache() {m_dhlMsgCache.cache_index = 0;}
    /**
     * @brief LogCache destructor.
     */
    virtual ~LogCache() {}

protected:

     /**
     * @brief DHL log cache information.
     */
    DHL_MSG_INFO_CACHE m_dhlMsgCache;

    bool IsLogCacheFull();
    void PrintLogCache();
    void ResetLogCache();
};

} //namespace libmdmonitor {
#endif
