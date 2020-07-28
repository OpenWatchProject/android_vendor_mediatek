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

#ifndef _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_H_
#define _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_H_

#include "CamNode_t.h"
#include "CamGraph_t.h"
#include "DebugUtil.h"
#include <cutils/properties.h>

#include "PipeLogHeaderBegin.h"
#include "DebugControl.h"
#define PIPE_TRACE TRACE_CAM_NODE
#define PIPE_CLASS_TAG "CamNode"
#include "PipeLog.h"

namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSFeaturePipe {

#define CAM_NODE_START_SEQ (1)
#define CAM_NODE_PROP_PREFIX "vendor.debug"

/*******************************************************************************
*
*******************************************************************************/
template <typename Handler_T>
CamNode<Handler_T>::CamNode(const char* name)
  : mStage(STAGE_IDLE)
  , mAllowDataFlow(MTRUE)
  , mPropValue(0)
  , mSeqHandler(CAM_NODE_START_SEQ)
{
  snprintf(mName, sizeof(mName), "%s", (name != NULL) ? name : "NA");
}

template <typename Handler_T>
CamNode<Handler_T>::~CamNode()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("%s", getName());
  this->stop();
  this->uninit();
  this->disconnect();
  TRACE_FUNC_EXIT();
}

template <typename Handler_T>
const char* CamNode<Handler_T>::getName() const
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mName;
}

template <typename Handler_T>
MINT32 CamNode<Handler_T>::getPropValue()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mPropValue;
}

template <typename Handler_T>
MINT32 CamNode<Handler_T>::getPropValue(DataID_T id)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return mDataPropValues[id];
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::connectData(DataID_T src, DataID_T dst, Handler_T *handler, ConnectionType type)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock _l(mNodeLock);
  MBOOL ret = MFALSE;
  if( mStage == STAGE_IDLE )
  {
    mHandlerMap[src] = HandlerEntry(src, dst, handler, type);
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::registerInputDataID(DataID_T id)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock _l(mNodeLock);
  MBOOL ret = MFALSE;
  if( mStage == STAGE_IDLE )
  {
    mSourceSet.insert(id);
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::disconnect()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock _l(mNodeLock);
  if( mStage == STAGE_IDLE )
  {
    mHandlerMap.clear();
    mSourceSet.clear();
    mSeqHandler.clear();
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MVOID CamNode<Handler_T>::setDataFlow(MBOOL allow)
{
  TRACE_FUNC_ENTER();
  android::Mutex::Autolock _l(mNodeLock);
  mAllowDataFlow = allow;
  TRACE_FUNC_EXIT();
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::init()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("%s", getName());
  MBOOL ret = MFALSE;
  android::Mutex::Autolock _l(mNodeLock);
  if( mStage == STAGE_IDLE )
  {
    this->updatePropValues();
    if( !this->onInit() )
    {
      MY_LOGE("%s onInit() failed", this->getName());
    }
    else
    {
      mStage = STAGE_READY;
      ret = MTRUE;
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::uninit()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::Mutex::Autolock _l(mNodeLock);
  if( mStage == STAGE_READY )
  {
    this->onUninit();
    mStage = STAGE_IDLE;
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::start()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("%s", getName());
  MBOOL ret = MFALSE;
  android::Mutex::Autolock _l(mNodeLock);
  if( mStage == STAGE_READY )
  {
    if( !this->onStart() )
    {
      MY_LOGE("%s onStart() failed", this->getName());
    }
    else
    {
      mStage = STAGE_RUNNING;
      ret = MTRUE;
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::stop()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("%s", getName());
  MBOOL ret = MFALSE;
  android::Mutex::Autolock _l(mNodeLock);
  if( mStage == STAGE_RUNNING )
  {
    if( !this->onStop() )
    {
      MY_LOGE("%s onStop() failed", this->getName());
    }
    mStage = STAGE_READY;
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MBOOL CamNode<Handler_T>::isRunning()
{
  TRACE_FUNC_ENTER();
  MBOOL ret;
  ret = (mStage == STAGE_RUNNING);
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
MVOID CamNode<Handler_T>::updatePropValues()
{
  TRACE_FUNC_ENTER();
  const char* name;
  name = this->getName();
  mPropValue = getFormattedPropertyValue("%s.%s", CAM_NODE_PROP_PREFIX, name);
  mDataPropValues.clear();
  {
    HANDLER_MAP_ITERATOR it, end;
    for( it = mHandlerMap.begin(), end = mHandlerMap.end(); it != end; ++it )
    {
      DataID_T id = it->first;
      MINT32 prop = getFormattedPropertyValue("%s.%s.%s", CAM_NODE_PROP_PREFIX, name, Handler_T::ID2Name(id));
      mDataPropValues[id] = prop;
    }
  }
  {
    typename SOURCE_SET::iterator it, end;
    for( it = mSourceSet.begin(), end = mSourceSet.end(); it != end; ++it )
    {
      DataID_T id = *it;
      MINT32 prop = getFormattedPropertyValue("%s.%s.%s", CAM_NODE_PROP_PREFIX, name, Handler_T::ID2Name(id));
      mDataPropValues[id] = prop;
    }
  }

  TRACE_FUNC_EXIT();
}

template <typename Handler_T>
template <typename BUFFER_T>
MBOOL CamNode<Handler_T>::handleData(DataID_T id, const BUFFER_T &buffer)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("%s", getName());
  MBOOL ret = MFALSE;
  android::Mutex::Autolock _l(mNodeLock);
  if( mStage == STAGE_RUNNING && mAllowDataFlow )
  {
    HANDLER_MAP_ITERATOR it = mHandlerMap.find(id);
    if( it != mHandlerMap.end() )
    {
      Handler_T *handler = it->second.mHandler;
      DataID_T dstID = it->second.mDstID;
      ConnectionType type = it->second.mType;
      if( handler )
      {
        if( type == CONNECTION_DIRECT )
        {
          ret = handler->onData(dstID, buffer);
        }
        else if( type == CONNECTION_SEQUENTIAL )
        {
          ret = mSeqHandler.onData(dstID, buffer, handler);
        }
        else
        {
          MY_LOGE("%s: (%d:%s) use unsupported connection type(%d)", getName(), id, Handler_T::ID2Name(id), type);
        }
      }
    }

    if( !ret )
    {
      MY_LOGE("%s: handleData(%d:%s) failed", getName(), id, Handler_T::ID2Name(id));
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}

template <typename Handler_T>
template <typename BUFFER_T>
MBOOL CamNode<Handler_T>::handleData(DataID_T id, BUFFER_T &buffer)
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC("%s", getName());
  MBOOL ret = MFALSE;
  android::Mutex::Autolock _l(mNodeLock);
  if( mStage == STAGE_RUNNING && mAllowDataFlow )
  {
    HANDLER_MAP_ITERATOR it = mHandlerMap.find(id);
    if( it != mHandlerMap.end() )
    {
      Handler_T *handler = it->second.mHandler;
      DataID_T dstID = it->second.mDstID;
      ConnectionType type = it->second.mType;
      if( handler )
      {
        if( type == CONNECTION_DIRECT )
        {
          ret = handler->onData(dstID, buffer);
        }
        else if( type == CONNECTION_SEQUENTIAL )
        {
          ret = mSeqHandler.onData(dstID, buffer, handler);
        }
        else
        {
          MY_LOGE("%s: (%d:%s) use unsupported connection type(%d)", getName(), id, Handler_T::ID2Name(id), type);
        }
      }
    }

    if( !ret )
    {
      MY_LOGE("%s: handleData(%d:%s) failed", getName(), id, Handler_T::ID2Name(id));
    }
  }
  TRACE_FUNC_EXIT();
  return ret;
}


} // NSFeaturePipe
} // campostalgo
} // mediatek
} // com

#include "PipeLogHeaderEnd.h"
#endif // _MTK_CAMERA_FEATURE_PIPE_CORE_CAM_NODE_H_
