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
 * @file ApiInfo.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c ApiInfo class for conversion between @c MONITOR_CMD_CODE and api-method-version tuple.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __API_INFO_H__
#define __API_INFO_H__
#include "libmdmonitor.h"
namespace libmdmonitor {
/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used for conversion between @c MONITOR_CMD_CODE and api-method-version tuple.
 */
class ApiInfo 
{
public:
    /**
     * @brief Create the API info by specifying api-method-version tuple.
     *
     * @param szApi [IN] String of api. Should not be NULL.
     * @param szMethod [IN] String of method. Should not be NULL.
     * @param version [IN] Version that greater than 0. Default to 1.
     *
     * @par Examples
     * JsonCmdDecoder.cpp
     * @snippet JsonCmdDecoder.cpp ApiInfo::ApiInfo(3)
     */
    ApiInfo(std::string szApi, std::string szMethod, uint32_t version = 1);

    /**
     * @brief Create the API info by specifying @c MONITOR_CMD_CODE. Version will be set to 1.
     *
     * @param code [IN] The monitor command code.
     */
    ApiInfo(MONITOR_CMD_CODE code);

    /**
     * @brief Get command code.
     *
     * @return Command code.
     *
     * @par Examples
     * JsonCmdDecoder.cpp
     * @snippet JsonCmdDecoder.cpp ApiInfo::GetCmdCode
     */
    MONITOR_CMD_CODE GetCmdCode() const { return m_cmdCode; }

    /**
     * @brief Get api string.
     *
     * @return !NULL: Api string.<br>
     *          NULL: No mapping for command code to api-method-version tuple.
     */
    std::string GetApi() const { return m_api; }

    /**
     * @brief Get method string
     *
     * @return !NULL: Method string.<br>
     *          NULL: No mapping for command code to api-method-version tuple.
     */
    std::string GetMethod() const { return m_method; }

    /**
     * @brief Get version 
     *
     * @return >0: Version.<br>
     *          0: No mapping for command code to api-method-version tuple.
     */
    uint32_t GetVersion() const { return m_version; }

protected:
    /**
     * @brief Command code.
     */
    MONITOR_CMD_CODE m_cmdCode;

    /**
     * @brief Api string.
     */
    std::string m_api;

    /**
     * @brief Method string.
     */
    std::string m_method;

    /**
     * @brief Version.
     */
    uint32_t m_version;
};
} //namespace libmdmonitor {

#endif
