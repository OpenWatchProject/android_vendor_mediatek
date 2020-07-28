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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#undef __Q
#undef _Q
#define __Q(v) #v
#define _Q(v) __Q(v)

#ifndef ILOG_MODULE_TAG
  #error "Must define ILOG_MODULE_TAG before include ILogHeader.h"
#endif

#define ILOG_CLASS_TAG_MUST 0

#ifdef ILOG_CLASS_TAG
  #define LOG_TAG _Q(ILOG_MODULE_TAG) "/" _Q(ILOG_CLASS_TAG)
#elif !(ILOG_CLASS_TAG_MUST)
  #define LOG_TAG _Q(ILOG_MODULE_TAG)
#else
  #error "Must define ILOG_CLASS_TAG before include ILogHeader.h"
#endif

#include <mtkcam/utils/std/Log.h>
#include <mtkcam/utils/std/ULog.h>
#include <string>
#include <cutils/properties.h>

#ifndef ILOG_TRACE
#define ILOG_TRACE 0
#endif

#undef FP_STR
#undef FF_LOG
#undef FPS_LOG
#undef XPS_LOG
#undef FSS_LOG
#undef XSS_LOG
#undef FP_LOG
#undef XP_LOG
#undef FS_LOG
#undef XS_LOG
#undef FP_DO
#undef PT_DO

#define FP_STR ::NSCam::Feature::getLogStr
#define FF_LOG(v,f,...)       CAM_ULOGM##v(f, __FUNCTION__, ##__VA_ARGS__)
#define FPS_LOG(v,f,...)      FF_LOG(v, "[%s]" f, ##__VA_ARGS__)
#define XPS_LOG(v,x,f,...)    FF_LOG(v, "[%s%s]" f, x, ##__VA_ARGS__)
#define FSS_LOG(v,s,f,...)    FF_LOG(v, "[%s] %s: " f, s, ##__VA_ARGS__)
#define XSS_LOG(v,x,s,f,...)  FF_LOG(v, "[%s%s] %s: " f, x, s, ##__VA_ARGS__)
#define FP_LOG(v,...)         FPS_LOG(v, "" __VA_ARGS__)
#define XP_LOG(v,x,...)       XPS_LOG(v, x, "" __VA_ARGS__)
#define FS_LOG(v,s,...)       FSS_LOG(v, FP_STR(s), "" __VA_ARGS__)
#define XS_LOG(v,x,s,...)     XSS_LOG(v, x, FP_STR(s), "" __VA_ARGS__)
#define FP_DO(cmd) do { cmd; } while(0)

#if ILOG_TRACE
#define PT_DO(lv, cmd)        FP_DO(cmd)
#else
#define PT_DO(lv, cmd)
#endif

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF
#undef MY_LOGV_IF
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF
#undef MY_LOGA_IF
#undef MY_LOGF_IF
#undef MY_S_LOGV
#undef MY_S_LOGD
#undef MY_S_LOGI
#undef MY_S_LOGW
#undef MY_S_LOGE
#undef MY_S_LOGA
#undef MY_S_LOGF
#undef MY_S_LOGV_IF
#undef MY_S_LOGD_IF
#undef MY_S_LOGI_IF
#undef MY_S_LOGW_IF
#undef MY_S_LOGE_IF
#undef MY_S_LOGA_IF
#undef MY_S_LOGF_IF
#undef MY_LOG_FUNC_ENTER
#undef MY_LOG_FUNC_EXIT
#undef MY_LOG_S_FUNC_ENTER
#undef MY_LOG_S_FUNC_EXIT

#define MY_LOGV(f, arg...)            FP_DO(FP_LOG(V, f, ##arg))
#define MY_LOGD(f, arg...)            FP_DO(FP_LOG(D, f, ##arg))
#define MY_LOGI(f, arg...)            FP_DO(FP_LOG(I, f, ##arg))
#define MY_LOGW(f, arg...)            FP_DO(FP_LOG(W, f, ##arg))
#define MY_LOGE(f, arg...)            FP_DO(FP_LOG(E, f, ##arg))
#define MY_LOGA(f, arg...)            FP_DO(FP_LOG(A, f, ##arg))
#define MY_LOGF(f, arg...)            FP_DO(FP_LOG(F, f, ##arg))
#define MY_LOGV_IF(c, f, arg...)      FP_DO(if(c) FP_LOG(V, f, ##arg))
#define MY_LOGD_IF(c, f, arg...)      FP_DO(if(c) FP_LOG(D, f, ##arg))
#define MY_LOGI_IF(c, f, arg...)      FP_DO(if(c) FP_LOG(I, f, ##arg))
#define MY_LOGW_IF(c, f, arg...)      FP_DO(if(c) FP_LOG(W, f, ##arg))
#define MY_LOGE_IF(c, f, arg...)      FP_DO(if(c) FP_LOG(E, f, ##arg))
#define MY_LOGA_IF(c, f, arg...)      FP_DO(if(c) FP_LOG(A, f, ##arg))
#define MY_LOGF_IF(c, f, arg...)      FP_DO(if(c) FP_LOG(F, f, ##arg))
#define MY_S_LOGV(s, f, arg...)       FP_DO(FS_LOG(V, s, f, ##arg))
#define MY_S_LOGD(s, f, arg...)       FP_DO(FS_LOG(D, s, f, ##arg))
#define MY_S_LOGI(s, f, arg...)       FP_DO(FS_LOG(I, s, f, ##arg))
#define MY_S_LOGW(s, f, arg...)       FP_DO(FS_LOG(W, s, f, ##arg))
#define MY_S_LOGE(s, f, arg...)       FP_DO(FS_LOG(E, s, f, ##arg))
#define MY_S_LOGA(s, f, arg...)       FP_DO(FS_LOG(A, s, f, ##arg))
#define MY_S_LOGF(s, f, arg...)       FP_DO(FS_LOG(F, s, f, ##arg))
#define MY_S_LOGV_IF(c, s, f, arg...) FP_DO(if(c) FS_LOG(V, s, f, ##arg))
#define MY_S_LOGD_IF(c, s, f, arg...) FP_DO(if(c) FS_LOG(D, s, f, ##arg))
#define MY_S_LOGI_IF(c, s, f, arg...) FP_DO(if(c) FS_LOG(I, s, f, ##arg))
#define MY_S_LOGW_IF(c, s, f, arg...) FP_DO(if(c) FS_LOG(W, s, f, ##arg))
#define MY_S_LOGE_IF(c, s, f, arg...) FP_DO(if(c) FS_LOG(E, s, f, ##arg))
#define MY_S_LOGA_IF(c, s, f, arg...) FP_DO(if(c) FS_LOG(A, s, f, ##arg))
#define MY_S_LOGF_IF(c, s, f, arg...) FP_DO(if(c) FS_LOG(F, s, f, ##arg))
#define MY_LOG_FUNC_ENTER(...)        FP_DO(XP_LOG(D, "+", ##__VA_ARGS__))
#define MY_LOG_FUNC_EXIT(...)         FP_DO(XP_LOG(D, "-", ##__VA_ARGS__))
#define MY_LOG_S_FUNC_ENTER(s,...)    FP_DO(XS_LOG(D, "+", s, ##__VA_ARGS__))
#define MY_LOG_S_FUNC_EXIT(s,...)     FP_DO(XS_LOG(D, "-", s, ##__VA_ARGS__))
#define S_LOGD(s,f,...)               CAM_ULOGMD("%s " f, FP_STR(s), ##__VA_ARGS__)

#undef TRACE_FUNC_ENTER
#undef TRACE_FUNC_ENTER_2
#undef TRACE_FUNC_EXIT
#undef TRACE_FUNC_EXIT_2
#undef TRACE_FUNC
#undef TRACE_FUNC_2
#undef TRACE_S_FUNC_ENTER
#undef TRACE_S_FUNC_ENTER_2
#undef TRACE_S_FUNC_EXIT
#undef TRACE_S_FUNC_EXIT_2
#undef TRACE_S_FUNC
#undef TRACE_S_FUNC_2

#define TRACE_FUNC_ENTER(...)       PT_DO(1, XP_LOG(D, "+", ##__VA_ARGS__))
#define TRACE_FUNC_EXIT(...)        PT_DO(1, XP_LOG(D, "-", ##__VA_ARGS__))
#define TRACE_FUNC(...)             PT_DO(1, FP_LOG(D, ##__VA_ARGS__))
#define TRACE_S_FUNC_ENTER(s,...)   PT_DO(1, XS_LOG(D, "+", s, ##__VA_ARGS__))
#define TRACE_S_FUNC_EXIT(s,...)    PT_DO(1, XS_LOG(D, "-", s, ##__VA_ARGS__))
#define TRACE_S_FUNC(s,...)         PT_DO(1, FS_LOG(D, s, ##__VA_ARGS__))

#define TRACE_FUNC_ENTER_2(...)     PT_DO(2, XP_LOG(D, "+", ##__VA_ARGS__))
#define TRACE_FUNC_EXIT_2(...)      PT_DO(2, XP_LOG(D, "-", ##__VA_ARGS__))
#define TRACE_FUNC_2(...)           PT_DO(2, FP_LOG(D, ##__VA_ARGS__))
#define TRACE_S_FUNC_ENTER_2(s,...) PT_DO(2, XS_LOG(D, "+", s, ##__VA_ARGS__))
#define TRACE_S_FUNC_EXIT_2(s,...)  PT_DO(2, XS_LOG(D, "-", s, ##__VA_ARGS__))
#define TRACE_S_FUNC_2(s,...)       PT_DO(2, FS_LOG(D, s, ##__VA_ARGS__))

