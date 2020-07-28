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

#define LOG_TAG "iio/ifunc"

#include "isp_function.h"
#include <mtkcam/def/PriorityDefs.h>
//open syscall
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//close syscall
#include <unistd.h>
//mmap syscall
#include <sys/mman.h>

#include <cutils/properties.h>  // For property_get().
#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

DECLARE_DBG_LOG_VARIABLE(function);
//EXTERN_DBG_LOG_VARIABLE(function);

// Clear previous define, use our own define.
#undef ISP_FUNC_VRB
#undef ISP_FUNC_DBG
#undef ISP_FUNC_INF
#undef ISP_FUNC_WRN
#undef ISP_FUNC_ERR
#undef ISP_FUNC_AST
#define ISP_FUNC_VRB(fmt, arg...)        do { if (function_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define ISP_FUNC_DBG(fmt, arg...)        do { if (function_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define ISP_FUNC_INF(fmt, arg...)        do { if (function_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define ISP_FUNC_WRN(fmt, arg...)        do { if (function_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define ISP_FUNC_ERR(fmt, arg...)        do { if (function_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define ISP_FUNC_AST(cond, fmt, arg...)  do { if (function_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

Mutex   IspFunction_B::fbcPhyLock[CAM_MAX][_dmao_max_];

IspFunction_B::IspFunction_B()
{
    m_bypass=MFALSE;
    m_pIspDrv=NULL;
    m_hwModule = MAX_ISP_HW_MODULE;
    DBG_LOG_CONFIG(imageio, function);
}


MINT32 IspFunction_B::config( void )
{
    MINT32 retval = 0;
    if(this->m_bypass == MTRUE){
        ISP_FUNC_INF("%s:bypass this setting\n",this->name_Str());
        return retval;
    }
    retval = this->_config();       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:config fail!\n",this->name_Str());
    }
    return retval;
}

MINT32 IspFunction_B::enable( void* pParam )
{
    MINT32 retval;
    retval = this->_enable(pParam);       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:_enable fail!\n",this->name_Str());
    }
    return retval;
}

MINT32 IspFunction_B::disable( void* pParam )
{
    MINT32 retval;
    retval = this->_disable(pParam);       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:_disable fail!\n",this->name_Str());
    }
    return retval;
}

MINT32 IspFunction_B::write2CQ(void)
{
    MINT32 retval;
    retval = this->_write2CQ();       //virtual
    if( retval != 0 ){
        ISP_FUNC_ERR("%s:_write2CQ fail!\n",this->name_Str());
    }
    return retval;
}








