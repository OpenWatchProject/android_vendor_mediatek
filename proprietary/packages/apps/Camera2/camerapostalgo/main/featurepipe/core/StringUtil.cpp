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
 * MediaTek Inc. (C) 2018. All rights reserved.
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

#include "core/StringUtil.h"

#include "core/DebugControl.h"
#define PIPE_TRACE TRACE_STRING_UTIL
#define PIPE_CLASS_TAG "StringUtil"
#include "core/PipeLog.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

StringUtil::StringUtil()
{
}

StringUtil::StringUtil(size_t size)
  : mSize(size ? size : 1024)
{
  TRACE_FUNC_ENTER();
  mPtr = new char[mSize];
  mPtr[0] = '\0';
  TRACE_FUNC_EXIT();
}

StringUtil::~StringUtil()
{
  TRACE_FUNC_ENTER();
  if( mPtr != mStr )
  {
    delete [] mPtr;
    mPtr = NULL;
  }
  TRACE_FUNC_EXIT();
}

int StringUtil::printf(const char *fmt, ...)
{
  TRACE_FUNC_ENTER();
  int len = 0;
  if( mPtr && !mFull && fmt )
  {
    va_list args;
    va_start(args, fmt);
    len = vsnprintf(mPtr+mLen, mSize-mLen, fmt, args);
    va_end(args);

    if( len >= 0 )
    {
      mLen += len;
    }
    else
    {
      mFull = true;
    }
  }
  TRACE_FUNC_EXIT();
  return len;
}

const char* StringUtil::c_str() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mPtr;
}

size_t StringUtil::length() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mLen;
}

size_t StringUtil::size() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mSize;
}


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

