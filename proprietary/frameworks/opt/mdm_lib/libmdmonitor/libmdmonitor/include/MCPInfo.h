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
 * @file MCPInfo.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c MCPInfo class that manages the information of modem monitor communication packet.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */
#ifndef __MCP_INFO_H__
#define __MCP_INFO_H__
#include "libmdmonitor.h"
namespace libmdmonitor {
class DataQueue;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to manage the information of modem monitor communication packet.
 *
 * @see ConnectionUserData
 */
class MCPInfo
{
public:
    /**
     * @brief Create an empty @c MCPInfo instance.
     */
    MCPInfo();

    /**
     * @brief Cleanup.
     */
    virtual ~MCPInfo();

    /**
     * @brief Clear all data and reset to default.
     */
	void Reset();

    /**
     * @brief Get MCP type. 
     *
     * @return MCP type.
     */
	MCP_TYPE GetType() const { return m_type; }

    /**
     * @brief Get the length of MCP data. 
     *
     * @return The length of MCP data.
     */
	size_t GetLen() const { return m_len; }

    /**
     * @brief Get the pointer to MCP data. 
     *
     * @return The pointer to MCP data.
     */
	const uint8_t *GetData() const { return m_data; }

    /**
     * @brief Set the information of type, length and data.
     *
     * @param type [IN] The MCP type.
     * @param len [IN] The length of MCP data.
     * @param data [IN] The pointer to MCP data. NULL is acceptable.
     * @param bCopyData [IN] true: Copy the content of @a data into internal buffer.<br>
     *                      false: Do not copy and only set @a m_data to @a data by default.
     */
	void SetData(MCP_TYPE type, size_t len, const uint8_t *data, bool bCopyData = false);

protected:
    /**
     * @brief The MCP type.
     */
    MCP_TYPE m_type;

    /**
     * @brief The length of MCP data.
     */
    size_t m_len;

    /**
     * @brief The pointer to MCP data.
     */
    uint8_t *m_data;

    /**
     * @brief true indicates @a m_data is internally allocated and should be deleted when reset.
     */
	bool m_isCopyData;
};
} //namespace libmdmonitor {

#endif
