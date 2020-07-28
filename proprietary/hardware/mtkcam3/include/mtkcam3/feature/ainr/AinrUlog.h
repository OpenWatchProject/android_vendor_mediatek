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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#ifndef __AINRULOG_H__
#define __AINRULOG_H__

#include <atomic>
#include <utils/RefBase.h>
#include <mtkcam/utils/std/ULog.h>

#   define CC_LIKELY( exp )    (__builtin_expect( !!(exp), true ))
#   define CC_UNLIKELY( exp )  (__builtin_expect( !!(exp), false ))


CAM_ULOG_DECLARE_MODULE_ID(MOD_LIB_AINR);

#define ainrLogV(fmt, arg...)                  CAM_ULOGMV("[%s] " fmt, __FUNCTION__, ##arg)
#define ainrLogD(fmt, arg...)                  CAM_ULOGMD("[%s] " fmt, __FUNCTION__, ##arg)
#define ainrLogI(fmt, arg...)                  CAM_ULOGMI("[%s] " fmt, __FUNCTION__, ##arg)
#define ainrLogW(fmt, arg...)                  CAM_ULOGMW("[%s] " fmt, __FUNCTION__, ##arg)
#define ainrLogE(fmt, arg...)                  CAM_ULOGME("[%s] " fmt, __FUNCTION__, ##arg)
#define ainrLogA(fmt, arg...)                  CAM_ULOGM_ASSERT("[%s] " fmt, __FUNCTION__, ##arg)
#define ainrLogF(fmt, arg...)                  CAM_ULOGM_FATAL("[%s] " fmt, __FUNCTION__, ##arg)

#define ainrLogV_IF(cond, ...)       do { if ( (cond) ) { ainrLogV(__VA_ARGS__); } }while(0)
#define ainrLogD_IF(cond, ...)       do { if ( (cond) ) { ainrLogD(__VA_ARGS__); } }while(0)
#define ainrLogI_IF(cond, ...)       do { if ( (cond) ) { ainrLogI(__VA_ARGS__); } }while(0)
#define ainrLogW_IF(cond, ...)       do { if ( (cond) ) { ainrLogW(__VA_ARGS__); } }while(0)
#define ainrLogE_IF(cond, ...)       do { if ( (cond) ) { ainrLogE(__VA_ARGS__); } }while(0)
#define ainrLogA_IF(cond, ...)       do { if ( (cond) ) { ainrLogA(__VA_ARGS__); } }while(0)
#define ainrLogF_IF(cond, ...)       do { if ( (cond) ) { ainrLogF(__VA_ARGS__); } }while(0)


#endif

