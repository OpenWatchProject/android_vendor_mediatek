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
 * @file CircularBufEx.h
 *
 * @author mtk10567 (Roger Kuo)
 *
 * @brief The header file defines @c CircularBufEx class which is widely used for data buffer.
 *
 * @copyright MediaTek Inc. (C) 2015. All rights reserved.
 */

#ifndef __CIRCULARBUFFER_EX_H__
#define __CIRCULARBUFFER_EX_H__
#include "libmdmonitor.h"
namespace libmdmonitor {

/**
 * @author mtk10567 (Roger Kuo)
 *
 * @brief Circular buffer class for Producer/Consumer applications.
 *
 * It provides check-in/check-out related APIs. However,<br>
 * there're no critical section implemented in those functions.<br>
 * Please be aware of the race condition issue when multiple threads access the buffer.
 */
class CircularBufEx {
  public:
    /**
     * @brief Create an empty circular buffer.
     */
    CircularBufEx();

    /**
     * @brief Cleanup of the circular buffer.
     */
    ~CircularBufEx();

    /**
     * @brief Initialize the circular buffer.
     *
     * @param size [IN] buffer size in bytes
     *
     * @return false: Failed to allocate memory.<br>
     *         true: Buffer was successfully initialized.
     *
     */
    bool InitBuffer(int size);

    /**
     * @brief Clear the circular buffer.
     */
    void Clear();

    /**
     * @brief Check if the buffer is full.
     *
     * @return true: Buffer is full.<br>
     *         false: Buffer is not full.
     */
    bool IsFull();

    /**
     * @brief Check if the buffer is empty.
     *
     * @return true: Buffer is empty.<br>
     *        false: Buffer is not empty.
     */
    bool IsEmpty();

    /**
     * @brief Check-in data into buffer.
     *
     * @b CheckIn is non-blocking. Please make sure there're enough space before check-in.
     * @param pData [IN] Pointer to data array. Should not be NULL or false will return.
     * @param nLen [IN] Length of data. Can be <=0 and true will return.
     *
     * @return true: Data was successfully check-in.<br>
     *        false: No enough space for check-in.
     */
    bool CheckIn(const unsigned char *pData, int nLen);

    /**
     * @brief Check-out data non-blocking.
     *
     * Suppose we check-in 3 times of data in 4, 10, 3 bytes respectively already.<br>
     * It's necessary to call @b CheckOut three times to fetch all data in the buffer.<br>
     * That is the first call of @b CheckOut will only obtain 4 bytes of data.<br>
     * Such property can reduce much parsing effort for consumer.
     *
     * @param pData [OUT] The data buffer for check-out. Should not be NULL or -1 will return.
     * @param nLen [IN] The size of data buffer. Can be <=0 and true will return.
     *
     * @return The size of data check-out.
     */
    int CheckOut(unsigned char *pData, int nLen);

    /**
     * @brief Get the size of buffer in empty.
     *
     * @return The size of buffer in empty.
     */
    int GetEmptySpace();

  protected:
    /**
     * @brief The main data buffer.
     */
    unsigned char	 *m_Buffer;

    /**
     * @brief Indicator of the end position of each check-in unit.
     */
    unsigned char	 *m_isEnd;

    /**
     * @brief The buffer size.
     */
    int					m_max_size;

    /**
     * @brief The index of buffer for check-in.
     *
     */
    volatile int		m_pIn;

    /**
     * @brief The index of buffer for check-out.
     *
     */
    volatile int		m_pOut;
};

} //namespace libmdmonitor {
#endif
