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

#ifndef PIPE_MODULE_TAG
  #error "Must define PIPE_MODULE_TAG before include PipeLog.h"
#endif
#ifndef PIPE_CLASS_TAG
  #error "Must define PIPE_CLASS_TAG before include PipeLog.h"
#endif

#undef LOG_TAG
#define LOG_TAG PIPE_MODULE_TAG "/" PIPE_CLASS_TAG
#include <mtkcam/utils/std/Log.h>

#undef FP_LOG
#define FP_LOG(lv, fmt, arg...) CAM_LOG##lv("[%s]" fmt, __FUNCTION__, ##arg)

#undef FP_DO
#define FP_DO(cmd) do { cmd; } while(0)

#undef MY_LOGV
#undef MY_LOGD
#undef MY_LOGI
#undef MY_LOGW
#undef MY_LOGE
#undef MY_LOGA
#undef MY_LOGF
#undef MY_TRACE
#undef MY_LOGV_IF
#undef MY_LOGD_IF
#undef MY_LOGI_IF
#undef MY_LOGW_IF
#undef MY_LOGE_IF
#undef MY_LOGA_IF
#undef MY_LOGF_IF
#undef MY_TRACE_IF

#define MY_LOGV(fmt, arg...)        FP_DO(FP_LOG(V, fmt, ##arg);)
#define MY_LOGD(fmt, arg...)        FP_DO(FP_LOG(D, fmt, ##arg);)
#define MY_LOGI(fmt, arg...)        FP_DO(FP_LOG(I, fmt, ##arg);)
#define MY_LOGW(fmt, arg...)        FP_DO(FP_LOG(W, fmt, ##arg);)
#define MY_LOGE(fmt, arg...)        FP_DO(FP_LOG(E, fmt, ##arg);)
#define MY_LOGA(fmt, arg...)        FP_DO(FP_LOG(A, fmt, ##arg);)
#define MY_LOGF(fmt, arg...)        FP_DO(FP_LOG(F, fmt, ##arg);)
#define MY_TRACE(fmt, arg...)       FP_DO(FP_LOG(D, fmt, ##arg);)
#define MY_LOGV_IF(c, fmt, arg...)  FP_DO(if(c) FP_LOG(V, fmt, ##arg))
#define MY_LOGD_IF(c, fmt, arg...)  FP_DO(if(c) FP_LOG(D, fmt, ##arg))
#define MY_LOGI_IF(c, fmt, arg...)  FP_DO(if(c) FP_LOG(I, fmt, ##arg))
#define MY_LOGW_IF(c, fmt, arg...)  FP_DO(if(c) FP_LOG(W, fmt, ##arg))
#define MY_LOGE_IF(c, fmt, arg...)  FP_DO(if(c) FP_LOG(E, fmt, ##arg))
#define MY_LOGA_IF(c, fmt, arg...)  FP_DO(if(c) FP_LOG(A, fmt, ##arg))
#define MY_LOGF_IF(c, fmt, arg...)  FP_DO(if(c) FP_LOG(F, fmt, ##arg))
#define MY_TRACE_IF(c, fmt, arg...) FP_DO(if(c) FP_LOG(D, fmt, ##arg))

// redefine TRACE_FUNC according to PIPE_TRACE value
#undef TRACE_FUNC_ENTER
#undef TRACE_FUNC_EXIT
#undef TRACE_FUNC
#undef TRACE_N_FUNC_ENTER
#undef TRACE_N_FUNC_EXIT
#undef TRACE_N_FUNC
#if defined(PIPE_TRACE) && (PIPE_TRACE != 0 )
  #define TRACE_FUNC_ENTER()          MY_TRACE("+")
  #define TRACE_FUNC_EXIT()           MY_TRACE("-")
  #define TRACE_FUNC(fmt, arg...)     MY_TRACE(fmt, ##arg)
  #define TRACE_N_FUNC_ENTER(n)       MY_TRACE("(%s)+", n)
  #define TRACE_N_FUNC_EXIT(n)        MY_TRACE("(%s)-", n)
  #define TRACE_N_FUNC(n, f, arg...)  MY_TRACE("(%s)" f, n, ##arg)
#else
  #define TRACE_FUNC_ENTER()
  #define TRACE_FUNC_EXIT()
  #define TRACE_FUNC(fmt, arg...)
  #define TRACE_N_FUNC_ENTER(n)
  #define TRACE_N_FUNC_EXIT(n)
  #define TRACE_N_FUNC(n, f, arg...)
#endif
