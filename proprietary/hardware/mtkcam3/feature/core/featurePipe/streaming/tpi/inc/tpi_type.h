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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_TYPE_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_TYPE_H_

#include <mtkcam/def/common.h>
#include <mtkcam/utils/imgbuf/IImageBuffer.h>
#include <mtkcam/utils/metadata/IMetadata.h>

#include "tpi_define.h"
#include "tpi_id.h"

using NSCam::EImageFormat;
using NSCam::IImageBuffer;
using NSCam::IMetadata;
using NSCam::MRectF;
using NSCam::MSize;

typedef unsigned TPI_NODE_ID;
typedef unsigned TPI_META_ID;
typedef unsigned TPI_BUFFER_ID;

enum TPI_SCENARIO_TYPE
{
  TPI_SCENARIO_UNKNOWN,
  TPI_SCENARIO_STREAMING_SINGLE,
  TPI_SCENARIO_STREAMING_DUAL_BASIC,
  TPI_SCENARIO_STREAMING_DUAL_DEPTH,
  TPI_SCENARIO_STREAMING_DUAL_VSDOF,
  TPI_SCENARIO_SLOWMOTION,
};

enum TPI_NODE_TYPE
{
  TPI_NODE_MTK,
  TPI_NODE_VENDOR,
};

enum TPI_BUFFER_OPT
{
  TPI_BUFFER_OPT_NONE         = 0,
  TPI_BUFFER_OPT_READ_WRITE   = 1<<0,
};

typedef struct TPI_BufferInfo_t
{
  bool              mNeed = false;
  TPI_BUFFER_ID     mBufferID = TPI_BUFFER_ID_UNKNOWN;
  EImageFormat      mFormat = NSCam::eImgFmt_UNKNOWN;
  MSize             mSize = MSize(0,0);
  unsigned          mStride = 0;
  unsigned          mBufferSetting = 0;
  unsigned          mCustomSetting = 0;

  TPI_BufferInfo_t() {}
  TPI_BufferInfo_t(TPI_BUFFER_ID id) : mBufferID(id) {}
  TPI_BufferInfo_t(TPI_BUFFER_ID id, const TPI_BufferInfo_t &src)
  {
      *this = src;
      mBufferID = id;
  }
} TPI_BufferInfo;

typedef struct TPI_ViewInfo_t
{
  unsigned          mSensorID = (unsigned)-1;
  MSize             mSensorSize;
  MRectF            mSensorClip;
  MRectF            mSrcZoomROI;

  TPI_BUFFER_ID     mSrcImageID = TPI_BUFFER_ID_UNKNOWN;
  MRectF            mSrcImageClip;
  MRectF            mDstImageClip;
  MRectF            mDstZoomROI;
} TPI_ViewInfo;

typedef struct TPI_Meta_t
{
  TPI_META_ID       mMetaID = TPI_META_ID_UNKNOWN;
  IMetadata*        mMetaPtr = NULL;
} TPI_Meta;

typedef struct TPI_Image_t
{
  TPI_BUFFER_ID     mBufferID = TPI_BUFFER_ID_UNKNOWN;
  IImageBuffer*     mBufferPtr = NULL;
  TPI_ViewInfo      mViewInfo;
  bool              mUseSrcImageBuffer = false;
} TPI_Image;

typedef struct TPI_Data_t
{
  TPI_DATA_QUEUE_CMD        mQueueCmd = TPI_DATA_QUEUE_CMD_NONE;
} TPI_Data;

typedef struct TPI_ConfigInfo_t
{
  TPI_BufferInfo mBufferInfo;
} TPI_ConfigInfo;

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_TYPE_H_
