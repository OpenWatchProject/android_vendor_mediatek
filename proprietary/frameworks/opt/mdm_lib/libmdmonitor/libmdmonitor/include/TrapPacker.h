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
 * @file TrapPacker.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c TrapPacker class used for conversion between traps and MCP (Modem Monitor Communication Packet).
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __TRAP_PACKER_H__
#define __TRAP_PACKER_H__
#include "libmdmonitor.h"
namespace libmdmonitor {

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to organized traps into transmission unit (MCP, Modem Monitor Communication Packet).
 *
 * A MCP is always consist of three parts:<br>
 * <ul>
 * <li> Trap data (TRAP_TYPE_OTA/TRAP_TYPE_EM/TRAP_TYPE_VOLTE)
 * <li> Discard Info (TRAP_TYPE_DISCARDINFO)
 * </ul>
 */
class TrapPacker
{
public:
    /**
     * @brief Create a @c TrapPacker instance from MCP raw data.
     *
     * The constructor will analyze and parse MCP raw data to enable users to obtain timestamp/trap/discard information by following APIs.<br>
     * If @a rawData could not be successfully parsed, the initialization flag @a m_isInitialized will remain to false.
     *
     * @param rawData [IN] The MCP raw data. Should not be NULL or the trap packer will not be initialized.
     * @param rawLen [IN] The length of MCP raw data. Should not be 0 or the trap packer will not be initialized.
     *
     */
	TrapPacker(const uint8_t *rawData, size_t rawLen);

    /**
     * @brief Create a @c TrapPacker instance from trap data.
     *
     * The constructor will automatically fill the timestamp in MCP raw buffer.<br>
     * Discard information can be added by @b SetDiscardInfo API later.
     *
     * @param type [IN] Trap types: TRAP_TYPE_OTA/TRAP_TYPE_EM/TRAP_TYPE_VOLTE.
     * @param trapData [IN] The trap data. Should not be NULL or the trap packer will not be initialized.
     * @param trapLen [IN] The length of trap data. Should not be 0 or the trap packer will not be initialized.
     *
     */
	TrapPacker(TRAP_TYPE type, const uint8_t *trapData, size_t trapLen);

    /**
     * @brief Cleanup.
     */
	virtual ~TrapPacker();

    /**
     * @brief Get the pointer to the MCP raw data buffer.
     *
     * @return NULL: The trap packer was not successfully initialized.<br>
     *        !NULL: The pointer to the MCP raw data buffer.
     */
	const uint8_t *GetRawData() const { return m_isInitialized?m_rawData:NULL; }

    /**
     * @brief Get the length of MCP raw data buffer.
     *
     * @return 0: The trap packer was not successfully initialized.<br>
     *        >0: The length of MCP raw data buffer.
     */
	size_t GetRawLen() const { return m_isInitialized?m_rawLen:0; }

    /**
     * @brief Get the trap data buffer 
     *
     * @return NULL: The trap packer was not successfully initialized.<br>
     *        !NULL: The pointer to the trap data buffer.
     */
	const uint8_t *GetTrapData() const { return m_isInitialized?m_trapData:NULL; }

    /**
     * @brief Get the length of trap data.
     *
     * @return 0: The trap packer was not successfully initialized.<br>
     *        >0: The length of trap data.
     */
	size_t GetTrapLen() const { return m_isInitialized?m_trapLen:0; }

    /**
     * @brief Get trap type.
     *
     * @return TRAP_TYPE_UNDEFINED: The trap packer was not successfully initialized.<br>
     *                            Otherwise: The trap type.
     */
	TRAP_TYPE GetTrapType() const { return m_isInitialized?m_trapType:TRAP_TYPE_UNDEFINED; }

    /**
     * @brief Set discard count.
     *
     * @param type [IN] The type of discarded trap.
     * @param count [IN] The discard count. Can be 0.
     *
     * @return false: The trap packer was not successfully initialized.<br>
     *          true: Success.
     */
	bool SetDiscardInfo(TRAP_TYPE type, size_t count);

    /**
     * @brief Get discard count 
     *
     * @param type [IN] The type of discarded trap.
     *
     * @return The discard count.
     *
     */
	size_t GetDiscardInfo(TRAP_TYPE type) const;

    /**
     * @brief Whether the @c TrapPacker instance was initialized successfully.
     *
     * @return true: Initialized successfully.<br>
     *        false: Not initialized.
     */
    bool IsInitialized() const { return m_isInitialized; }

protected:
    /**
     * @brief Parse the MCP raw data buffer.
     *
     * If @a rawData could not be successfully parsed, the initialization flag @a m_isInitialized will remain to false.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
	bool Parse();

#pragma pack(push,1)
    /**
     * @brief The trap header struct.
     */
    typedef struct {
        uint8_t type;
        uint32_t len;
    } trap_header_struct;

    /**
     * @brief The discard info struct.
     */
	typedef struct {
		uint8_t type;
		uint32_t count;
	} discard_info_struct;
#pragma pack(pop)

    /**
     * @brief Indicates whether the instance was constructed successfully.
     */
	bool m_isInitialized;

    /**
     * @brief The MCP raw data buffer.
     *
     * The layout is as following:<br>
     * <br>
     * [5B]   trap_header_struct<br>
     * [Vary] trap_data<br>
     * [5B]   discard_info_struct (discard header)<br>
     * [5B]   discard_info_struct (discard info for xxx type)<br>
     * [5B]   discard_info_struct (discard info for yyy type)<br>
     *                :<br>
     *                :<br>
     * <br>
     * Note: Discard info should always be the last part.
     */
	uint8_t *m_rawData;

    /**
     * @brief The length of MCP raw data.
     */
	size_t m_rawLen;

    /**
     * @brief The pointer to the beginning of trap data in @a m_rawData.
     */
	const uint8_t *m_trapData;

    /**
     * @brief The length of trap data.
     */
	size_t m_trapLen;

    /**
     * @brief The pointer to the discard info header in @a m_rawData. May be NULL if no discard info exists.
     */
	discard_info_struct *m_pDiscardInfo;

    /**
     * @brief The trap type.
     */
	TRAP_TYPE m_trapType;
};
} //namespace libmdmonitor {

#endif
