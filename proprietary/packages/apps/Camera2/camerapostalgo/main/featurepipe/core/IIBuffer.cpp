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

#include <core/IIBuffer.h>

#include <core/DebugControl.h>
#define PIPE_TRACE TRACE_IIBUFFER
#define PIPE_CLASS_TAG "IIBuffer"
#include <core/PipeLog.h>


namespace com {
namespace mediatek {
namespace campostalgo {
namespace NSBufferPool {

IIBuffer::IIBuffer()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

IIBuffer::~IIBuffer()
{
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
}

IImageBuffer* IIBuffer::getImageBufferPtr() const
{
  TRACE_FUNC_ENTER();
  IImageBuffer *ptr = NULL;
  ptr = this->getImageBuffer().get();
  TRACE_FUNC_EXIT();
  return ptr;
}

MBOOL IIBuffer::getOwnerID(const SharedBufferPool *pool, MUINT32 &id) const
{
  (void)pool;
  (void)id;
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MFALSE;
}

MBOOL IIBuffer::changeOwnerID(const SharedBufferPool *pool, unsigned newID, unsigned &oldID)
{
  (void)pool;
  (void)newID;
  (void)oldID;
  TRACE_FUNC_ENTER();
  TRACE_FUNC_EXIT();
  return MFALSE;
}

MSize IIBuffer::getImgSize() const
{
  TRACE_FUNC_ENTER();
  MSize ret(0,0);
  android::sp<IImageBuffer> buffer = this->getImageBuffer();
  if( buffer != NULL )
  {
    ret = buffer->getImgSize();
  }
  TRACE_FUNC_EXIT();
  return ret;
}

MBOOL IIBuffer::setExtParam(const MSize &size)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::sp<IImageBuffer> buffer = this->getImageBuffer();
  if( buffer != NULL )
  {
    ret = buffer->setExtParam(size);
  }
  TRACE_FUNC_EXIT();
  return ret;
}

MINT64 IIBuffer::getTimestamp() const
{
  TRACE_FUNC_ENTER();
  android::sp<IImageBuffer> buffer = this->getImageBuffer();
  TRACE_FUNC_EXIT();
  return (buffer != NULL) ? buffer->getTimestamp() : 0;
}

MVOID IIBuffer::setTimestamp(MINT64 timestamp)
{
  TRACE_FUNC_ENTER();
  android::sp<IImageBuffer> buffer = this->getImageBuffer();
  if( buffer != NULL )
  {
    buffer->setTimestamp(timestamp);
  }
  TRACE_FUNC_EXIT();
}

MBOOL IIBuffer::syncCache(NSCam::eCacheCtrl ctrl)
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::sp<IImageBuffer> buffer = this->getImageBuffer();
  if( buffer != NULL )
  {
    ret = buffer->syncCache(ctrl);
  }
  TRACE_FUNC_EXIT();
  return ret;
}

MBOOL IIBuffer::erase()
{
  TRACE_FUNC_ENTER();
  MBOOL ret = MFALSE;
  android::sp<IImageBuffer> buffer = this->getImageBuffer();
  if( buffer != NULL )
  {
    for( unsigned i = 0, n = buffer->getPlaneCount(); i < n; ++i )
    {
      void *ptr = (void*)buffer->getBufVA(i);
      unsigned size = buffer->getBufSizeInBytes(i);
      memset(ptr, 0, size);
    }
    ret = MTRUE;
  }
  TRACE_FUNC_EXIT();
  return ret;
}

NB_SPTR IIBuffer::getGraphicBufferAddr()
{
  TRACE_FUNC_ENTER();
  NB_SPTR addr = NULL;
  android::sp<IImageBuffer> buffer;
  buffer = this->getImageBuffer();
  if( buffer != NULL && buffer->getImageBufferHeap() )
  {
    addr = (NB_SPTR)buffer->getImageBufferHeap()->getHWBuffer();
  }
  TRACE_FUNC_EXIT();
  return addr;
}

IIBuffer_IImageBuffer::IIBuffer_IImageBuffer(IImageBuffer *buffer)
  : mBuffer(buffer)
{
}

IIBuffer_IImageBuffer::~IIBuffer_IImageBuffer()
{
}

sp<IImageBuffer> IIBuffer_IImageBuffer::getImageBuffer() const
{
  return mBuffer;
}


}
}
}
}

