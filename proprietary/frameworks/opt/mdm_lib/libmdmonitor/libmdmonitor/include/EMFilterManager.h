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
 * @file EMFilterManager.h
 *
 * @author mtk03685 (Roger Lo)
 *
 * @brief The header file defines @c EMFilterManager class providing modem filter related functions to modem.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */
#ifndef __EMFILTER_MANAGER_H__
#define __EMFILTER_MANAGER_H__
#include <pthread.h>
#include "libmdmonitor.h"
#include "FrameDecoder.h"
namespace libmdmonitor {

/**
 * @author mtk03585(Roger Lo)
 *
 * @brief The class provides modem filter related functions to modem.
 */
class EMFilterManager
{

typedef struct {
    unsigned int sync_pattern;
    unsigned char flag;
    unsigned char type;
    unsigned short payload_langth;
} DHL_CMD_HEADER;

typedef struct {
    unsigned int header;
    unsigned short reserved;
    unsigned char data_chksum;
    unsigned char header_chksum;
} DHL_CHKSUM_PKT;

public:
    /**
     * @brief Constructor of EM filter manager.
     */
    EMFilterManager();

    /**
     * @brief Destructor of EM filter manager.
     */
    virtual ~EMFilterManager();

    /**
     * @brief Check whether monitor core is successfully initialized.
     *
     * It checks if @a m_decoder is initialized only.
     *
     * @return true: Success.<br>
     *        false: Errors.
     */
    bool IsInitialized() const { return bInitialized; }

    /**
     * @brief XXXXXXXXXXXXXXXXXXXXXX.
     */
    int LoadDefaultModemEMFilterBinFile();

    /**
     * @brief Load em_filter.bin as the initital content of buffer.
     */
    static bool SearchModemEMFilterBinFile(const char *szSearchDir, char *pathBuffer, size_t pathBufferSize);

    /**
     * @brief Disable all EMs for specific sim index in filter buffer.
     */
    bool Filter_DisableAllEM(FrameDecoder *pDecoder, uint32_t simIdx);

    /**
     * @brief Enable all EMs for specific sim index in filter buffer.
     */
    bool Filter_EnableAllEM(FrameDecoder *pDecoder, uint32_t simIdx);

    /**
     * @brief Enable specific EM for specific sim index in filter buffer.
     */
    bool Filter_EnableEM(FrameDecoder *pDecoder, uint32_t simIdx, uint64_t emIdx);

    /**
     * @brief Disable specific EM for specific sim index in filter buffer.
     */
    bool Filter_DisableEM(FrameDecoder *pDecoder, uint32_t simIdx, uint64_t emIdx);

    /**
     * @brief Update each DHL command's data checksum in filter buffer.
     */
    bool Filter_UpdateDHLDataCheckSum();


    void DumpEMFilterBufferToFile();  // for debug

    /**
     * @brief Get em filter buffer pointer.
     */
    char *GetFilterBufferPoint() {return m_emFilterBuffer;}

    /**
     * @brief Get em filter buffer size.
     */
    size_t GetFilterBufferSize() {return m_emFilterBufferSize;}
private:
    /**
     * @brief object initialized flag.
     */

    bool bInitialized;
    /**
     * @brief EM filter buffer for dynamic change.
     */
    char *m_emFilterBuffer;

    /**
     * @brief EM filter buffer size.
     */
    size_t m_emFilterBufferSize;
};
} //namespace libmdmonitor {

#endif
