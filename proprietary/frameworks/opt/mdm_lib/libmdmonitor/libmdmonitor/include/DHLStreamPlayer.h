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
 * @file DHLStreamPlayer.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c DHLStreamPlayer class used to replay a DHL stream from a file of raw data DU.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __DHL_STREAM_PLAYER_H__
#define __DHL_STREAM_PLAYER_H__
#include "libmdmonitor.h"
#include <stdio.h>
namespace libmdmonitor {
/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The class is used to replay a DHL stream from a file of raw data DU.
 */
class DHLStreamPlayer 
{
public:
    /**
     * @brief Open a file of raw data DU and set the times to rewind automatically.
     *
     * When reaching EOF, the player is able to rewind automatically if @a autoRewind is proper set.
     *
     * @param szPath [IN] The file path of raw data DU. Should not be NULL or false will return.
     * @param autoRewind [IN] >0: The times to auto rewind when player reaches EOF.<br>
     *                        =0: Do not auto rewind. (Default) <br>
     *                        -1: Auto rewind.
     *
     * @return true: The file was opened successfully.<br>
     *        false: Error.
     */
    bool OpenFile(const char *szPath, int autoRewind = 0);

    /**
     * @brief Close the opened raw data DU file.
     */
    void CloseFile();

    /**
     * @brief Get the DHL frame from the DHL stream.
     *
     * @param frameType [OUT] The read frame type.
     * @param frameSize [OUT] The read frame length.
     *
     * @return NULL: Read error.<br>
     *        !NULL: The pointer to the frame data.
     */
    const unsigned char *GetFrame(unsigned char &frameType, size_t &frameSize);

    /**
     * @brief Create a DHLStreamPlayer.
     */
    DHLStreamPlayer();

    /**
     * @brief Cleanup.
     */
    virtual ~DHLStreamPlayer();

protected:
    /**
     * @brief File descriptor of opened file.
     */
    int m_fd;

    /**
     * @brief The times to auto rewind. value of -1 means to rewind forever.
     */
    int m_autoRewind;

    /**
     * @brief DHL stream parsing states.
     */
    typedef enum {
        /**
         * @brief The state to search SYNC words in the stream.
         */
        DHL_PARSER_STATE_SEARCH_SYNC_WORDS = 1,

        /**
         * @brief The state expecting the following data is a DHL frame.
         */
        DHL_PARSER_STATE_GET_TLV_FRAME
    } DHLParserState;

    /**
     * @brief Current DHL parsing state.
     */
    DHLParserState m_parserState;

    /**
     * @brief The buffer for read data.
     */
    unsigned char *m_readBuffer;

#define UNKNOWN_FRAME_FLUSH_THRESHOLD (DHL_MAX_FRAME_SIZE<<1) /* 32kb */
    /**
     * @brief The size of data in @a m_readBuffer.
     */
    int m_readBufferDataSize;

    /**
     * @brief Current read position in @a m_readBuffer.
     */
    int m_readBufferReadPosition;

    /**
     * @brief The data buffer for frame data.
     */
    unsigned char m_preParsedFrameBuffer[DHL_MAX_FRAME_SIZE+UNKNOWN_FRAME_FLUSH_THRESHOLD];

    /**
     * @brief The size of data in @a m_preParsedFrameBuffer.
     */
    int m_preParsedFrameBufferSize;

    /**
     * @brief Drop data in @a m_preParsedFrameBuffer.
     *
     * @param forceFlush [IN] true: drop data always.<br>
     *                       false: drop data when @a m_preParsedFrameBufferSize is greater than @a UNKNOWN_FRAME_FLUSH_THRESHOLD.
     */
    void FlushUnknownFrame( bool forceFlush );

    /**
     * @brief Check out one byte from @a m_readBuffer.
     *
     * @param byteData [OUT] The byte data of checked out.
     *
     * @return 1: Check out data successfully.<br>
     *         0: No data to check out.<br>
     *        -1: Error.
     */
    int CheckOut( unsigned char &byteData );

    /**
     * @brief Check out one byte into @a m_preParsedFrameBuffer.
     *
     * @param byteData [OUT] The read byte data.
     *
     * @return true: Read data successfully.<br>
     *        false: No data to read.
     */
    bool ReadByte( unsigned char &byteData );

    /**
     * @brief Try to get DHL Sync words. 
     *
     * @return  1: DHL Sync words were found.<br>
     *          0: Unexpected byte data.<br>
     *         -1: @b ReadByte Error.
     */ 
    int GetDHLSyncWords(void);

    /**
     * @brief Try to get DHL frame data.
     *
     * @param pFrameData [OUT] The pointer to frame data.
     * @param frameType [OUT] The frame type.
     * @param frameSize [OUT] The frame size.
     *
     * @return  1: One DHL frame is found.<br>
     *          0: Unexpected byte data.<br>
     *         -1: @b ReadByte Error.
     */
    int GetDHLTlvFrame(const unsigned char *&pFrameData, unsigned char &frameType, size_t &frameSize);

#define DHL_TYPE_EVENT_FLAG (0x20)
    /**
     * @brief GET_DHL_TLV_HEADER_EVENT_FLAG 
     *
     * @param flagFieldByte [IN] The DHL flag byte
     *
     * @return 1: There is event info in the frame.<br>
     *         0: No event info in the frame.
     */
    unsigned char GET_DHL_TLV_HEADER_EVENT_FLAG(unsigned char flagFieldByte);
};
} //namespace libmdmonitor {

#endif
