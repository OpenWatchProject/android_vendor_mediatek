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

#ifndef _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_MGR_PLUGIN_WRAPPER_H_
#define _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_MGR_PLUGIN_WRAPPER_H_

#include "TPIMgr.h"
#include <mtkcam3/3rdparty/plugin/PipelinePluginType.h>


namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

class TPIMgr_PluginWrapper : public TPIMgr
{
public:
  typedef NSCam::NSPipelinePlugin::JoinPlugin T_PluginType;
  typedef T_PluginType::Ptr                   T_PluginPool;
  typedef T_PluginType::Property              T_Property;
  typedef T_PluginType::IProvider::Ptr        T_Plugin;
  typedef T_PluginType::Selection::Ptr        T_Selection;
  typedef T_PluginType::Request::Ptr          T_Request;
  typedef T_PluginType::ConfigParam           T_ConfigParam;
  typedef unsigned                            T_NodeID;
  typedef NSCam::NSPipelinePlugin::BufferHandle::Ptr T_BufferHandle;
  typedef NSCam::NSPipelinePlugin::JoinImageInfo JoinImageInfo;

private:

  class PluginInfo
  {
  public:
      PluginInfo();
      PluginInfo(const T_Plugin&, const T_Property&, const T_Selection&);
      bool init(T_NodeID id, const T_ConfigParam &param);
      bool uninit(T_NodeID id);
      const char* getName() const;
      const char* getEntryName() const;
  public:
      T_Plugin mPlugin = NULL;
      T_Property mProperty;
      T_Selection mSelection = NULL;
      bool mInited = false;
  };
  typedef std::map<T_NodeID, PluginInfo> PluginMap;
  typedef std::list<T_NodeID> NodeIDList;

  enum class PathType { YUV, ASYNC, DISP, META };

private:
  static void initProperty(T_Property &prop);
  static void initSelection(T_Selection &sel, const std::shared_ptr<NSCam::IMetadata> &meta);
  static void initCfgSelection(T_Selection &sel, const std::shared_ptr<NSCam::IMetadata> &meta);
  static void initP2Selection(T_Selection &sel, const std::shared_ptr<NSCam::IMetadata> &meta);
  static void initConfigParam(T_ConfigParam &param, const TPI_ConfigInfo &info);
  static void print(const T_Property &prop);
  static void print(const T_Selection &sel);
  static void print(const NSPipelinePlugin::BufferSelection &sel, const char *name);
  static void print(const T_Request &req);
  static void print(const char *name, const T_BufferHandle &buffer, const JoinImageInfo &info);

  static void initRequest(T_Request &request);
  static void fillMeta(T_Request &request, const TPI_Meta &meta);
  static void fillBuffer(T_Request &request, const TPI_Image &img);
  static void initImageInfo(const TPI_Image &img, JoinImageInfo &info);
  static void updateImageInfo(const T_Request &request, TPI_Image &img);
  static void updateImageInfo(TPI_Image &img, const JoinImageInfo &info);
  static void updateData(const T_Request &request, TPI_Data &data);
  static bool build(TPI_Session &session, const PluginMap &pluginMap, PathType pathType, const NodeIDList &list, const NodeIDList &list2 = NodeIDList());
  T_Request makeRequest(TPI_Meta meta[], size_t metaCount, TPI_Image img[], size_t imgCount, const TPI_Data &data);

public:
  TPIMgr_PluginWrapper();
  ~TPIMgr_PluginWrapper();
  bool createSession(unsigned mainSensorID, const std::vector<MUINT32> &allSensorIDs, unsigned numSensor, TP_MASK_T masks, MFLOAT marginRatio, const MSize &maxRawSize, const MSize &maxOutSize, const IMetadata &meta, const NSPipelinePlugin::TPICustomConfig &tpiCustomConfig);
  bool destroySession();

  bool initSession();
  bool uninitSession();

  bool initNode(T_NodeID nodeID, const TPI_ConfigInfo &config);
  bool uninitNode(T_NodeID nodeID);

  bool start();
  bool stop();

  bool genFrame(TPIOFrame &frame, const IMetadata *meta);

  bool enqueNode(T_NodeID nodeID, unsigned frame, TPI_Meta meta[], size_t metaCount, TPI_Image img[], size_t imgCount, TPI_Data &data);

  bool getSessionInfo(TPI_Session &session) const;

private:
  static unsigned generateSessionID();

private:
  bool checkSession(const TPI_Session &session) const;

private:
  void createPluginSession(unsigned mainSensorID, const std::vector<MUINT32> &allSensorIDs, TP_MASK_T masks, const IMetadata &meta, MFLOAT marginRatio, const  NSPipelinePlugin::TPICustomConfig &tpiCustomConfig);
  void checkPluginConfig(const T_Selection &sel, MFLOAT marginRatio);
  void configSelection(const T_Selection &sel, const std::vector<MUINT32> &allSensorIDs, const  NSPipelinePlugin::TPICustomConfig &tpiCustomConfig);
  void destroyPluginSession();
  void convertPluginToSession();
  void updateResult(const T_Request &request, TPI_Image img[], size_t imgCount, TPI_Data &data);

private:
  enum eState {
    STATE_IDLE,
    STATE_CREATE,
    STATE_INIT,
    STATE_UNINIT,
    STATE_DESTROY,
  };

private:
  eState mState = TPIMgr_PluginWrapper::STATE_IDLE;
  TPI_Session mSession;
  unsigned mFrameID = 0;

private:
  T_PluginPool mPluginPool = 0;
  PluginMap mPluginMap;

  friend class PluginPathBuilder;
};

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam

#endif // _MTK_CAMERA_STREAMING_FEATURE_PIPE_TPI_MGR_PLUGIN_WRAPPER_H_
