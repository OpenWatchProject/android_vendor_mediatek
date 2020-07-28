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
 * @file DataQueue.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c DataQueue class used in @c BufferedConnection as the internal buffer.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __DATA_QUEUE_H__
#define __DATA_QUEUE_H__
#include "libmdmonitor.h"
namespace libmdmonitor {
class SocketConnection;

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief A queue class for data read from @c BufferedConnection.
 *
 * To improve the efficiency, @c BufferedConnection provides @b ReadToBuffer API to read<br>
 * a chunk of data into the @c DataQueue instance, an internal buffer to keep the data for further operations.
 */
class DataQueue
{
public:
    /**
     * @brief Create a empty data queue.
     */
    DataQueue();

    /**
     * @brief Cleanup
     */
	virtual ~DataQueue();

    /**
     * @brief Clear the queue.
     */
    void Clear();

    /**
     * @brief Get the size of space in use.
     * @return The size of space in use.
     */
    size_t Size() const;

    /**
     * @brief Get the size of space not in use.
     * @return The size of space not in use. 
     */
	size_t SpaceSize() const;

    /**
     * @brief Get the pointer to the beginning of the data in queue.
     * @return The pointer to the beginning of the data in queue.
     */
    const uint8_t *Front() const;

    /**
     * @brief Drop first @a size bytes of data. 
     * @param size [IN] To indicate how much bytes of data to drop.<br>
     *                  Default to drop one byte of data.
     */
    void Pop(size_t size = 1); 

    /**
     * @brief Append data to the end of the data in queue.
     *
     * @param data [IN] The data to append. Should not be NULL or false will return.
     * @param nLen [IN] The length of the data to append. Can be 0 and true will return.
     *
     * @return true: Success.<br>
     *        false: Not enough space in queue for append.<br>
     */
	bool Push(const uint8_t *data, size_t nLen);

    /**
     * @brief Allow SocketConnection to access private data.
     */
	friend class SocketConnection;
protected:
    /**
     * @brief Get the pointer to the beginning of space not in use. 
     * @return The pointer to the beginning of space not in use.
     */
    uint8_t *SpaceStart();

    /**
     * @brief Increase the size of space in use.
     * @param size [IN] The number of size to increase.
     */
	void IncSize(size_t size);

    /**
     * @brief The size of space in use.
     */
    size_t m_size;

    /**
     * @brief The size of space not in use.
     */
	size_t m_unusedSize;

    /**
     * @brief The space for data.
     */
    uint8_t m_data[MAX_READ_BUFFER_SIZE];
};
} //namespace libmdmonitor {

#endif
