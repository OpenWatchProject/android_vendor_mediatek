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

#include <sys/stat.h>
#include <sys/types.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <core/DebugUtil.h>
#include <core/DebugControl.h>
#define PIPE_TRACE TRACE_DEBUG_UTIL
#define PIPE_CLASS_TAG "DebugUtil"
#include "core/PipeLog.h"

#define DEFAULT_PROPERTY_VALUE 0

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

MINT32 getPropertyValue(const char *key)
{
  return getPropertyValue(key, DEFAULT_PROPERTY_VALUE);
}

MINT32 getPropertyValue(const char *key, MINT32 defVal)
{
  TRACE_FUNC_ENTER();
  MINT32 value = defVal;
  if( key && *key )
  {
    value = property_get_int32(key, defVal);
    if( value != defVal )
    {
     MY_LOGD("getPropertyValue %s=%d", key, value);
    }
  }
  TRACE_FUNC_EXIT();
  return value;
}

MINT32 getFormattedPropertyValue(const char *fmt, ...)
{
  TRACE_FUNC_ENTER();
  const int maxLen = PROPERTY_KEY_MAX*2;
  char key[maxLen];
  va_list args;
  int keyLen;
  MINT32 value = DEFAULT_PROPERTY_VALUE;

  if( fmt && *fmt )
  {
    va_start(args, fmt);
    keyLen = vsnprintf(key, sizeof(key), fmt, args);
    va_end(args);

    if( keyLen >= maxLen )
    {
      MY_LOGE("Property key[%s...] length exceed %d char. Can not get prop!", key, maxLen);
    }
    if( keyLen > 0 && keyLen < maxLen )
    {
      value = getPropertyValue(key, DEFAULT_PROPERTY_VALUE);
    }
  }

  TRACE_FUNC_EXIT();
  return value;
}
static bool do_mkdir(char const*const path, uint_t const mode)
{
    struct stat st;
    //
    if  ( 0 != ::stat(path, &st) )
    {
        //  Directory does not exist.
        if  ( 0 != ::mkdir(path, mode) && EEXIST != errno )
        {
            return  false;
        }
    }
    else if ( ! S_ISDIR(st.st_mode) )
    {
        return  false;
    }
    //
    return  true;
}
//
bool
makePath(char const*const path, uint_t const mode)
{
    bool ret = true;
    if(path == NULL) {
        MY_LOGE("error: path is Null");
        return false;
    }
    char*copypath = strdup(path);
    char*pp = copypath;
    if(pp == NULL) {
        MY_LOGE("error: copypath is Null");
        return false;
    }
    char*sp;
    while ( ret && 0 != (sp = strchr(pp, '/')) )
    {
        if (sp != pp)
        {
            *sp = '\0';
            ret = do_mkdir(copypath, mode);
            *sp = '/';
        }
        pp = sp + 1;
    }
    if (ret)
        ret = do_mkdir(path, mode);
    free(copypath);
    return  ret;
}


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com
