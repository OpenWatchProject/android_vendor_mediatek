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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_SESSION_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_SESSION_H_

#include "tpi_type.h"

typedef struct TPI_NodeInfo_t
{
  TPI_NODE_ID      mNodeID = TPI_NODE_ID_UNKNOWN;
  TPI_BufferInfo    mBufferInfoList[TPI_BUFFER_INFO_LIST_SIZE];
  size_t            mBufferInfoListCount = 0;
  unsigned          mNodeOption = TPI_NODE_OPT_NONE;
  unsigned          mCustomOption = 0;
  unsigned          mExpectMS = 0;
  unsigned          mQueueCount = 0;
  unsigned          mMarginRatio = 0;

  TPI_NodeInfo_t() {}
  TPI_NodeInfo_t(unsigned id) : mNodeID(id) {}
} TPI_NodeInfo;

typedef struct TPI_Port_t
{
  TPI_NODE_ID       mNode = TPI_NODE_ID_UNKNOWN;
  TPI_BUFFER_ID     mPort = TPI_BUFFER_ID_UNKNOWN;
} TPI_Port;

typedef struct TPI_PathInfo_t
{
  TPI_Port          mSrc;
  TPI_Port          mDst;
} TPI_PathInfo;

typedef struct TPI_Session_t
{
  unsigned          mMgrVersion = TPI_VERSION;
  unsigned          mClientVersion = 0;

  unsigned          mSessionID = 0;
  unsigned          mLogicalSensorID = 0;
  TPI_SCENARIO_TYPE mScenario = TPI_SCENARIO_UNKNOWN;
  void*             mSessionCookie = NULL;

  unsigned          mMaxRawWidth = 0;
  unsigned          mMaxRawHeight = 0;
  unsigned          mMaxOutWidth = 0;
  unsigned          mMaxOutHeight = 0;

  TPI_NodeInfo      mNodeInfoList[TPI_NODE_INFO_LIST_SIZE];
  size_t            mNodeInfoListCount = 0;

  TPI_PathInfo      mPathInfoList[TPI_PATH_INFO_LIST_SIZE];
  size_t            mPathInfoListCount = 0;

} TPI_Session;

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_SESSION_H_
