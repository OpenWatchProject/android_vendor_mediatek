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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_MGR_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_MGR_H_

#include "inc/tpi_session.h"
#include "TPIO.h"

#include <mtkcam3/3rdparty/plugin/TPICustomInfo.h>
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

typedef MUINT64 TP_MASK_T;

class TPIMgr
{
public:
  static TPIMgr* createInstance();
  static void destroyInstance(TPIMgr* &mgr);

  static bool isVendorNode(unsigned nodeID);

public:
  TPIMgr();
  virtual ~TPIMgr();
  virtual bool createSession(unsigned mainSensorID, const std::vector<MUINT32> &allSensorIDs, unsigned numSensor, TP_MASK_T masks, MFLOAT marginRatio, const MSize &maxRawSize, const MSize &maxOutSize, const IMetadata &meta, const  NSPipelinePlugin::TPICustomConfig &tpiCustomConfig) = 0;
  virtual bool destroySession() = 0;
  virtual bool initSession() = 0;
  virtual bool uninitSession() = 0;
  virtual bool initNode(unsigned nodeID, const TPI_ConfigInfo &config) = 0;
  virtual bool uninitNode(unsigned nodeID) = 0;

  virtual bool genFrame(TPIOFrame &frame, const IMetadata *meta) = 0;

  virtual bool enqueNode(unsigned nodeID, unsigned frame, TPI_Meta meta[], size_t metaCount, TPI_Image img[], size_t imgCount, TPI_Data &data) = 0;

  virtual bool getSessionInfo(TPI_Session &session) const = 0;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_MGR_H_
