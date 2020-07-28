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
* @file HalIspImp.h
* @brief Declarations of Implementation of ISP Hal Class
*/

#ifndef __HAL_ISP_RES_WRAPPER_H__
#define __HAL_ISP_RES_WRAPPER_H__

#include <IResultPool.h>
#include <ResultPool4LSCConfig.h>
#include <ResultPool4Module.h>

namespace NS3Av3
{

class ResultPoolImpWrapper : public ResultPoolImp
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:

    /**
     * @brief Create instance of ResultPoolImpWrapper
     * @param [in] i4SensorIdx: sensor index
     * @param [in] strUser: user name
     * @return
     * - HalIspImp an instance of HalispImp
     */
    static ResultPoolImpWrapper* getInstance(MINT32 const i4SensorDev);

    MINT32 getAllHQCResultWithType(MUINT32 frmId, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, AllResult_T** pAllResult, const char* pFunctionName);
    MINT32 getOldestAllResultWithType(const E_PARTIAL_RESULT_OF_MODULE_T& eModule, AllResult_T** pAllResult, const char* pFunctionName);
    MINT32 getAllResultWithType(MUINT32 frmId, const E_PARTIAL_RESULT_OF_MODULE_T& eModule, AllResult_T** pAllResult, const char* pFunctionName);
    MVOID  getPreviousResultWithType(const E_PARTIAL_RESULT_OF_MODULE_T& eModule, ISPResultToMeta_T** pISPResult);

protected:

    //constructor
    ResultPoolImpWrapper(MINT32 const i4SensorIdx);

private:

    MINT32                                          m_i4SensorDev;
};

};

#endif //__HAL_ISP_RES_WRAPPER_H__

