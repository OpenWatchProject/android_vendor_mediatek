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

#ifndef __FLOAT16_ADD_CONV_H__
#define __FLOAT16_ADD_CONV_H__

#include "MtkTestModelBase.h"

class MtkTestModelFloat16AddConv : public MtkTestModelBase {
public:
    MtkTestModelFloat16AddConv();
    ~MtkTestModelFloat16AddConv();

protected:
    int createModelForTest() override;
    int createRequestForTest() override;
    int CompareMatrices() override;

private:
    uint8_t mInputRow = 0;
    uint8_t mInputCol = 0;
    uint8_t mOutputRow = 0;
    uint8_t mOutputCol = 0;

    const _Float16 mInputMat[4][4] = {
        {1.f, 2.f, 3.f, 4.f},
        {5.f, 6.f, 7.f, 8.f},
        {9.f, 10.f, 11.f, 12.f},
        {13.f, 14.f, 15.f, 16.f},
    };

    const _Float16 mFilterMat[2][2] = {
        {1.f, 1.f},
        {1.f, 1.f}
    };

    const _Float16 mMatrix[2][2] = {
        {2.f, 4.f},
        {6.f, 8.f}
    };

    const _Float16 mBiasMat[1] = {1};

    const _Float16 mExpectedMat[2][2] = {
        {17.f, 27.f},
        {53.f, 63.f},
    };

    _Float16 mOutputMat[2][2] = {
        {0.f, 0.f},
        {0.f, 0.f},
    };
};

#endif  // __FLOAT16_ADD_CONV_H__

