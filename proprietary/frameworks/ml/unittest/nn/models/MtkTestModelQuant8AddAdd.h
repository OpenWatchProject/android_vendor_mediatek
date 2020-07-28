/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef __QUANT8_ADD_ADD_H__
#define __QUANT8_ADD_ADD_H__

#include "MtkTestModelBase.h"

class MtkTestModelQuant8AddAdd : public MtkTestModelBase {
public:
    MtkTestModelQuant8AddAdd();
    ~MtkTestModelQuant8AddAdd();

protected:
    int createModelForTest() override;
    int createRequestForTest() override;
    int CompareMatrices() override;

private:
    uint8_t mInputRow = 0;
    uint8_t mInputCol = 0;
    uint8_t mOutputRow = 0;
    uint8_t mOutputCol = 0;

    const uint8_t mInput1[4][4] = {
        {1, 2, 3, 4},
        {3, 4, 5, 6},
        {1, 2, 3, 4},
        {3, 4, 5, 6},
    };

    const uint8_t mInput2[4][4] = {
        {2, 4, 6, 8},
        {1, 3, 5, 7},
        {2, 4, 6, 8},
        {1, 3, 5, 7}
    };

    const uint8_t mInput3[4][4] = {
        { 3,  5,  7, 9},
        {11, 22, 33, 44},
        { 3,  5,  7, 9},
        {11, 22, 33, 44},
    };

    const uint8_t mExpectedMat[4][4] = {
        { 6, 11, 16, 21},
        {15, 29, 43, 57},
        { 6, 11, 16, 21},
        {15, 29, 43, 57},
    };

    uint8_t mOutputMat[4][4] = {
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
        {0, 0, 0, 0},
    };
};

#endif  // __QUANT8_ADD_ADD_H__

