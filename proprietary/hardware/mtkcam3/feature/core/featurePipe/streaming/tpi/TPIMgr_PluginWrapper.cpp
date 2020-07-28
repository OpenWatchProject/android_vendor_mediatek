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

#include "TPIMgr_PluginWrapper.h"
#include "TPIMgr_Util.h"

#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam3/feature/utils/p2/P2Trace.h>

#include <featurePipe/core/include/StringUtil.h>
#include <featurePipe/core/include/Timer.h>

#include "DebugControl.h"
#define PIPE_CLASS_TAG "TPI_MGR_Plugin"
#define PIPE_TRACE TRACE_TPI_MGR_PLUGIN
#include <featurePipe/core/include/PipeLog.h>

CAM_ULOG_DECLARE_MODULE_ID(MOD_FPIPE_STREAMING);

using namespace NSCam::NSPipelinePlugin;

using NSCam::NSPipelinePlugin::MetadataHandle;
using NSCam::NSPipelinePlugin::BufferHandle;
typedef MetadataHandle::Ptr T_MetaHandle;
typedef BufferHandle::Ptr T_BufferHandle;

namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe {

static const char* toFmtName(EImageFormat fmt)
{
    switch(fmt)
    {
    case eImgFmt_YV12:  return "yv12";
    case eImgFmt_NV21:  return "nv21";
    case eImgFmt_NV12:  return "nv12";
    case eImgFmt_YUY2:  return "yuy2";
    default:            return "unknown";
    }
}

void TPIMgr_PluginWrapper::initProperty(T_Property &prop)
{
    prop.mName = "unknown";
    prop.mFeatures = 0;
}

void initFormat(NSPipelinePlugin::BufferSelection &sel)
{
    sel.addSupportFormat(eImgFmt_YV12);
    sel.addSupportFormat(eImgFmt_NV21);
    sel.addSupportFormat(eImgFmt_NV12);
    sel.addSupportFormat(eImgFmt_YUY2);
}

void TPIMgr_PluginWrapper::initSelection(T_Selection &sel, const std::shared_ptr<NSCam::IMetadata> &meta)
{
  if( sel != NULL )
  {
    sel->mSelStage = NSPipelinePlugin::eSelStage_CFG;
    sel->mCfgOrder = 0;
    sel->mCfgJoinEntry = NSPipelinePlugin::eJoinEntry_S_YUV;
    sel->mCfgInplace = false;
    sel->mCfgCropInput = false;
    sel->mCfgEnableFD = 0;
    sel->mCfgExpectMS = 0;
    sel->mCfgAsyncType = eAsyncType_FIX_RATE;
    sel->mCfgQueueCount = 0;
    sel->mCfgMarginRatio = 0;
    sel->mCfgRun = false;
    sel->mP2Run = false;
    sel->mIMetadataApp.setControl(meta);
  }
}

void TPIMgr_PluginWrapper::initCfgSelection(T_Selection &sel, const std::shared_ptr<NSCam::IMetadata> &meta)
{
  if( sel != NULL )
  {
    initSelection(sel, meta);
    initFormat(sel->mIBufferMain1);
    initFormat(sel->mIBufferMain2);
    initFormat(sel->mOBufferMain1);
    initFormat(sel->mOBufferMain2);
    initFormat(sel->mOBufferDisplay);
    initFormat(sel->mOBufferRecord);
    sel->mSelStage = NSPipelinePlugin::eSelStage_CFG;
  }
}

void TPIMgr_PluginWrapper::initP2Selection(T_Selection &sel, const std::shared_ptr<NSCam::IMetadata> &meta)
{
  if( sel != NULL )
  {
    initSelection(sel, meta);
    sel->mSelStage = NSPipelinePlugin::eSelStage_P2;
    sel->mP2Run = true;
  }
}

void TPIMgr_PluginWrapper::initConfigParam(T_ConfigParam &param, const TPI_ConfigInfo &info)
{
    param.mIBufferMain1.mFormat = info.mBufferInfo.mFormat;
    param.mIBufferMain1.mSize = info.mBufferInfo.mSize;
    param.mIBufferMain1.mStride = info.mBufferInfo.mStride;
}

void TPIMgr_PluginWrapper::print(const T_Property &prop)
{
    MY_LOGD("name=[%s] feature=[0x%016" PRIx64 "]", prop.mName, prop.mFeatures);
}

const char* toEntryName(MUINT32 entry)
{
    switch(entry)
    {
    case eJoinEntry_S_YUV:        return "yuv";
    case eJoinEntry_S_RAW:        return "raw";
    case eJoinEntry_S_ASYNC:      return "async";
    case eJoinEntry_S_DISP_ONLY:  return "disp";
    case eJoinEntry_S_META_ONLY:  return "meta";
    case eJoinEntry_S_DIV_2:      return "div";
    default:                      return "unknown";
    };
}

const char* toAsyncName(MUINT32 async)
{
    switch(async)
    {
    case eAsyncType_FIX_RATE: return "fix";
    case eAsyncType_WAITING:  return "wait";
    case eAsyncType_POLLING:  return "poll";
    default:                  return "unknown";
    };
}

const char* toSelStageName(MUINT32 stage)
{
  switch(stage)
  {
  case eSelStage_CFG:   return "stage_cfg";
  case eSelStage_P1:    return "stage_p1";
  case eSelStage_P2:    return "stage_p2";
  default:              return "stage_unknown";
  };
}

TPIOEntry toTPIOEntry(MUINT32 entry)
{
    switch(entry)
    {
    case eJoinEntry_S_YUV:        return TPIOEntry::YUV;
    case eJoinEntry_S_RAW:        return TPIOEntry::RAW;
    case eJoinEntry_S_ASYNC:      return TPIOEntry::ASYNC;
    case eJoinEntry_S_DISP_ONLY:  return TPIOEntry::DISP;
    case eJoinEntry_S_META_ONLY:  return TPIOEntry::META;
    case eJoinEntry_S_DIV_2:      return TPIOEntry::YUV;
    default:                      return TPIOEntry::UNKNOWN;
    };
}

void TPIMgr_PluginWrapper::print(const T_Selection &sel)
{
    if( sel == NULL )
    {
        MY_LOGD("sel=NULL");
    }
    else
    {
        MY_LOGD("sel->mSelStage=%d(%s)", sel->mSelStage, toSelStageName(sel->mSelStage));
        MY_LOGD("sel->mCfgOrder=%d", sel->mCfgOrder);
        MY_LOGD("sel->mCfgJoinEntry=%d(%s)", sel->mCfgJoinEntry, toEntryName(sel->mCfgJoinEntry));
        MY_LOGD("sel->mCfgInplace=%d", sel->mCfgInplace);
        MY_LOGD("mCfgCropIntput=%d", sel->mCfgCropInput);
        MY_LOGD("sel->mCfgEnableFD=%d", sel->mCfgEnableFD);
        MY_LOGD("sel->mCfgExpectMS=%d", sel->mCfgExpectMS);
        MY_LOGD("sel->mCfgAsyncType=%d(%s)", sel->mCfgAsyncType, toAsyncName(sel->mCfgAsyncType));
        MY_LOGD("sel->mCfgQueueCount=%d", sel->mCfgQueueCount);
        MY_LOGD("sel->mCfgMarginRatio=%d", sel->mCfgMarginRatio);
        MY_LOGD("sel->mCfgRun=%d", sel->mCfgRun);
        MY_LOGD("sel->mP2Run=%d", sel->mP2Run);

        print(sel->mIBufferMain1,   "sel->in_main1");
        print(sel->mIBufferMain2,   "sel->in_main2");
        print(sel->mIBufferDepth,   "sel->in_depth");
        print(sel->mOBufferMain1,   "sel->out_main1");
        print(sel->mOBufferMain2,   "sel->out_main2");
        print(sel->mOBufferDepth,   "sel->out_depth");
        print(sel->mOBufferDisplay, "sel->out_display");
        print(sel->mOBufferRecord,  "sel->out_record");
    }
}

void TPIMgr_PluginWrapper::print(const NSPipelinePlugin::BufferSelection &sel, const char *name)
{
    if( sel.getRequired() )
    {
        std::vector<MINT> fmts = sel.getFormats();
        EImageFormat fmt = fmts.size() ? (EImageFormat)fmts[0] : eImgFmt_UNKNOWN;
        MSize size = sel.getSpecifiedSize();
        MUINT32 alignW = 0, alignH = 0;
        sel.getAlignment(alignW, alignH);

        MY_LOGD("%s req=1 size(%d,%d) align(%d,%d) fmtList(#=%zu)[0]=%x/%s)", name, size.w, size.h, alignW, alignH, fmts.size(), fmt, toFmtName(fmt));
    }
}

const char* toStr(JoinQueueCmd cmd)
{
    switch(cmd)
    {
    case eJoinQueueCmd_NONE:      return "none";
    case eJoinQueueCmd_PUSH:      return "push";
    case eJoinQueueCmd_PUSH_POP:  return "push_pop";
    case eJoinQueueCmd_POP_ALL:   return "pop_all";
    default:                      return "unknown";
    }
}

void TPIMgr_PluginWrapper::print(const T_Request &req)
{
    MY_LOGD("req->queueCmd=%d(%s)", req->mOQueueCmd, toStr(req->mOQueueCmd));
    print("req->in_main1", req->mIBufferMain1, req->mIBufferMain1_Info);
    print("req->in_main2", req->mIBufferMain2, req->mIBufferMain2_Info);
    print("req->in_depth", req->mIBufferDepth, req->mIBufferDepth_Info);
    print("req->out_main1", req->mOBufferMain1, req->mOBufferMain1_Info);
    print("req->out_main2", req->mOBufferMain2, req->mOBufferMain2_Info);
    print("req->out_display", req->mOBufferDisplay, req->mOBufferDisplay_Info);
    print("req->out_record", req->mOBufferRecord, req->mOBufferRecord_Info);
}

void TPIMgr_PluginWrapper::print(const char* name, const T_BufferHandle &buffer, const JoinImageInfo &info)
{
    if( buffer != NULL )
    {
        MY_LOGD("%s: sensor=id(%d)size(%dx%d)clip(%dx%d@%d,%d) "
                "src=id(%d)inplace(%d) "
                "src=roi(%dx%d@%d,%d)clip(%dx%d@%d,%d) "
                "dst=roi(%dx%d@%d,%d)clip(%dx%d@%d,%d)",
            name, info.mISensorID,
            (int)info.mISensorSize.w, (int)info.mISensorSize.h,
            (int)info.mISensorClip.s.w, (int)info.mISensorClip.s.h,
            (int)info.mISensorClip.p.x, (int)info.mISensorClip.p.y,
            info.mOSrcImageID, info.mOUseSrcImageBuffer,
            (int)info.mISrcZoomROI.s.w, (int)info.mISrcZoomROI.s.h,
            (int)info.mISrcZoomROI.p.x, (int)info.mISrcZoomROI.p.y,
            (int)info.mOSrcImageClip.s.w, (int)info.mOSrcImageClip.s.h,
            (int)info.mOSrcImageClip.p.x, (int)info.mOSrcImageClip.p.y,
            (int)info.mODstZoomROI.s.w, (int)info.mODstZoomROI.s.h,
            (int)info.mODstZoomROI.p.x, (int)info.mODstZoomROI.p.y,
            (int)info.mODstImageClip.s.w, (int)info.mODstImageClip.s.h,
            (int)info.mODstImageClip.p.x, (int)info.mODstImageClip.p.y);
    }
}

class MyBufferHandle : public BufferHandle
{
public:
    MyBufferHandle() : BufferHandle() {}
    MyBufferHandle(const TPI_Image &img) : BufferHandle(),
        mData(img.mBufferPtr) {}
    virtual ~MyBufferHandle() {}
    NSCam::IImageBuffer* acquire(MINT) { return mData; }
    MVOID release() {}
    MVOID dump(std::ostream&) const {}
private:
    IImageBuffer *mData = NULL;
};

class MyMetaHandle : public MetadataHandle
{
public:
    MyMetaHandle() : MetadataHandle() {}
    MyMetaHandle(const TPI_Meta &meta) : MetadataHandle(),
        mData(meta.mMetaPtr) {}
    virtual ~MyMetaHandle() {}
    NSCam::IMetadata* acquire() { return mData; }
    MVOID release() {}
    MVOID dump(std::ostream&) const {}
private:
    IMetadata *mData = NULL;
};

T_BufferHandle makeBufferHandle(const TPI_Image &img)
{
    return std::make_shared<MyBufferHandle>(img);
}

T_MetaHandle makeMetaHandle(const TPI_Meta &meta)
{
    return std::make_shared<MyMetaHandle>(meta);
}

TPIMgr_PluginWrapper::PluginInfo::PluginInfo()
{
    initProperty(mProperty);
    initSelection(mSelection, NULL);
}

TPIMgr_PluginWrapper::PluginInfo::PluginInfo(const T_Plugin &plugin, const T_Property &prop, const T_Selection &selection)
    : mPlugin(plugin)
    , mProperty(prop)
    , mSelection(selection)
{
}

bool TPIMgr_PluginWrapper::PluginInfo::init(T_NodeID id, const T_ConfigParam &param)
{
  bool ret = false;
  if( mPlugin == NULL )
  {
    MY_LOGW("Invalid node[0x%x)](%s) = NULL inited=%d", id, mProperty.mName, mInited);
  }
  else if( !mInited )
  {
    P2_CAM_TRACE_FMT_BEGIN(TRACE_ADVANCED, "TPI init(0x%x/%s)", id, mProperty.mName);
    mPlugin->init();
    mPlugin->config(param);
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    mInited = true;
    ret = true;
  }
  return ret;
}

bool TPIMgr_PluginWrapper::PluginInfo::uninit(T_NodeID id)
{
  bool ret = false;
  if( mPlugin == NULL )
  {
    MY_LOGE("Invalid node[0x%x](%s) = NULL inited=%d", id, mProperty.mName, mInited);
  }
  else if( mInited )
  {
    P2_CAM_TRACE_FMT_BEGIN(TRACE_ADVANCED, "TPI uninit(0x%x/%s)", id, mProperty.mName);
    mPlugin->uninit();
    P2_CAM_TRACE_END(TRACE_ADVANCED);
    mInited = false;
    ret = true;
  }
  return ret;
}

const char* TPIMgr_PluginWrapper::PluginInfo::getName() const
{
  return mProperty.mName;
}

const char* TPIMgr_PluginWrapper::PluginInfo::getEntryName() const
{
  return mSelection != NULL ? toEntryName(mSelection->mCfgJoinEntry) : "x";
}

TPIMgr_PluginWrapper::TPIMgr_PluginWrapper()
  : TPIMgr()
{
}

TPIMgr_PluginWrapper::~TPIMgr_PluginWrapper()
{
  if( mState == STATE_INIT )
  {
    MY_LOGD("User should call uninit before exit");
    uninitSession();
  }
  if( mState == STATE_UNINIT )
  {
    MY_LOGD("User should call destroy before exit");
    destroySession();
  }
}

bool TPIMgr_PluginWrapper::createSession(unsigned mainSensorID, const std::vector<MUINT32> &allSensorIDs, unsigned numSensor, TP_MASK_T masks, MFLOAT marginRatio, const MSize &maxRawSize, const MSize &maxOutSize, const IMetadata &meta, const NSPipelinePlugin::TPICustomConfig &tpiCustomConfig)
{
  bool ret = false;
  MY_LOGI("+ sensor(#%d:main=%d) mask=[0x%016" PRIx64 "] size(raw:%dx%d app:%dx%d) margin(%.3f)", numSensor, mainSensorID, masks, maxRawSize.w, maxRawSize.h, maxOutSize.w, maxOutSize.h, marginRatio);
  if( mState == TPIMgr_PluginWrapper::STATE_IDLE )
  {
    mSession.mSessionID = generateSessionID();
    mSession.mLogicalSensorID = mainSensorID;
    mSession.mScenario = TPI_SCENARIO_STREAMING_SINGLE;
    mSession.mMaxRawWidth = maxRawSize.w;
    mSession.mMaxRawHeight = maxRawSize.h;
    mSession.mMaxOutWidth = maxOutSize.w;
    mSession.mMaxOutHeight = maxOutSize.h;

    createPluginSession(mainSensorID, allSensorIDs, masks, meta, marginRatio, tpiCustomConfig);
    convertPluginToSession();
    NSFeaturePipe::print(mSession);

    mState = TPIMgr_PluginWrapper::STATE_CREATE;
    ret = true;
  }
  MY_LOGI("- ret=%d", ret);
  return ret;
}

bool TPIMgr_PluginWrapper::destroySession()
{
  bool ret = false;
  MY_LOGI("+");
  if( mState == TPIMgr_PluginWrapper::STATE_UNINIT )
  {
    destroyPluginSession();
    mSession.mSessionCookie = NULL;
    mState = TPIMgr_PluginWrapper::STATE_DESTROY;
    ret = true;
  }
  MY_LOGI("- ret=%d", ret);
  return ret;
}

bool TPIMgr_PluginWrapper::initSession()
{
  bool ret = false;
  if( mState == TPIMgr_PluginWrapper::STATE_CREATE )
  {
    mState = TPIMgr_PluginWrapper::STATE_INIT;
    ret = true;
  }
  return ret;
}

bool TPIMgr_PluginWrapper::uninitSession()
{
  bool ret = false;
  if( mState == TPIMgr_PluginWrapper::STATE_INIT )
  {
    mState = TPIMgr_PluginWrapper::STATE_UNINIT;
    ret = true;
  }
  return ret;
}

bool TPIMgr_PluginWrapper::initNode(T_NodeID nodeID, const TPI_ConfigInfo &config)
{
  bool ret = false;
  if( mState == TPIMgr_PluginWrapper::STATE_INIT )
  {
    Timer timer(MTRUE);
    T_ConfigParam configParam;
    initConfigParam(configParam, config);
    ret = mPluginMap[nodeID].init(nodeID, configParam);
    timer.stop();
    MY_LOGI("Plugin (%s) init %d ms", mPluginMap[nodeID].getName(), timer.getElapsed());
  }
  if(!ret)
  {
    MY_LOGW("init Plugin Node(%u) fail", nodeID);
  }
  return ret;
}

bool TPIMgr_PluginWrapper::uninitNode(T_NodeID nodeID)
{
  bool ret = false;
  if( mState == TPIMgr_PluginWrapper::STATE_INIT )
  {
    ret = mPluginMap[nodeID].uninit(nodeID);
  }
  if(!ret)
  {
    MY_LOGW("uninit Plugin Node(%u) fail", nodeID);
  }
  return ret;
}

bool TPIMgr_PluginWrapper::genFrame(TPIOFrame &frame, const IMetadata *meta)
{
  bool ret = false;
  T_Selection sel;
  std::shared_ptr<NSCam::IMetadata> metaPtr;

  sel = mPluginPool != NULL ? mPluginPool->createSelection() : NULL;
  metaPtr = meta ? std::make_shared<NSCam::IMetadata>(*meta) : NULL;
  if( mState == TPIMgr_PluginWrapper::STATE_INIT &&
      sel != NULL && metaPtr != NULL)
  {
    StringUtil str;
    frame.setID(++mFrameID);
    for( auto &info : mPluginMap )
    {
      if( info.second.mInited &&
          info.second.mSelection != NULL &&
          info.second.mPlugin != NULL )
      {
        bool run = true;
        MUINT32 entry = info.second.mSelection->mCfgJoinEntry;
        if( entry == eJoinEntry_S_YUV ||
            entry == eJoinEntry_S_DIV_2 ||
            entry == eJoinEntry_S_ASYNC )
        {
          initP2Selection(sel, metaPtr);
          P2_CAM_TRACE_FMT_BEGIN(TRACE_ADVANCED, "TPI query(0x%x/%s)", info.first, info.second.mProperty.mName);
          info.second.mPlugin->negotiate(*sel);
          P2_CAM_TRACE_END(TRACE_ADVANCED);
          run = sel->mP2Run;
          if( run )
          {
            frame.add(toTPIOEntry(entry), info.first);
          }
        }
        str.printf(" [%d/%s(%s)run=%d]" , info.first, info.second.getName(), info.second.getEntryName(), run);
      }
    }
    MY_LOGD("TPIFrame#%d(meta=%p)(yuv=%d)(async=%d):%s", mFrameID, meta, frame.needEntry(TPIOEntry::YUV), frame.needEntry(TPIOEntry::ASYNC), str.c_str());
  }
  TRACE_FUNC_EXIT();
  return ret;
}

TPIMgr_PluginWrapper::T_Request TPIMgr_PluginWrapper::makeRequest(TPI_Meta meta[], size_t metaCount, TPI_Image img[], size_t imgCount, const TPI_Data &data)
{
    (void)data;
    T_Request request = mPluginPool->createRequest();
    initRequest(request);
    for( unsigned i = 0; i < metaCount; ++i )
    {
        fillMeta(request, meta[i]);
    }
    for( unsigned i = 0; i < imgCount; ++i )
    {
        fillBuffer(request, img[i]);
    }
    return request;
}

bool TPIMgr_PluginWrapper::enqueNode(T_NodeID nodeID, unsigned frame, TPI_Meta meta[], size_t metaCount, TPI_Image img[], size_t imgCount, TPI_Data &data)
{
  (void)frame;
  bool ret = false;
  TRACE_FUNC("+ node:%d frame:%d", nodeID, frame);
  if( mState == TPIMgr_PluginWrapper::STATE_INIT )
  {
    T_Plugin plugin = mPluginMap[nodeID].mPlugin;
    if( plugin != NULL )
    {
      T_Request request;
      request = makeRequest(meta, metaCount, img, imgCount, data);
      if( plugin->process(request) == OK )
      {
          ret = true;
          updateResult(request, img, imgCount, data);
      }
    }
  }
  TRACE_FUNC("- node:%d frame:%d ret:%d", nodeID, frame, ret);
  return ret;
}

bool TPIMgr_PluginWrapper::getSessionInfo(TPI_Session &session) const
{
  bool ret = false;
  if( mState != STATE_IDLE &&
      mState != STATE_DESTROY )
  {
    session = mSession;
    ret = true;
  }
  return ret;
}

unsigned TPIMgr_PluginWrapper::generateSessionID()
{
  static std::atomic<unsigned> sSessionID(0);
  return sSessionID++;
}

bool TPIMgr_PluginWrapper::checkSession(const TPI_Session &session) const
{
  bool ret = true;
  if( session.mMgrVersion != TPI_VERSION ||
      session.mClientVersion != TPI_VERSION )
  {
    MY_LOGE("Invalid API VERSION");
    ret = false;
  }
  else if( session.mScenario == TPI_SCENARIO_UNKNOWN )
  {
    MY_LOGE("Invalid scenario");
    ret = false;
  }
  else if( session.mNodeInfoListCount > TPI_NODE_INFO_LIST_SIZE ||
      session.mPathInfoListCount > TPI_PATH_INFO_LIST_SIZE )
  {
    MY_LOGE("Invalid info list count");
    ret = false;
  }
  for( size_t i = 0; i < session.mNodeInfoListCount; ++i )
  {
    (void)session.mNodeInfoList[i];
  }
  for( size_t i = 0; i < session.mPathInfoListCount; ++i )
  {
    (void)session.mPathInfoList[i];
  }
  return ret;
}

bool isDualPlugin(MUINT64 plugin)
{
    return !!(plugin &
              ( MTK_FEATURE_BOKEH | TP_FEATURE_BOKEH |
                MTK_FEATURE_DUAL_YUV | TP_FEATURE_DUAL_YUV |
                MTK_FEATURE_DUAL_HWDEPTH | TP_FEATURE_DUAL_HWDEPTH ));
}

void TPIMgr_PluginWrapper::createPluginSession(unsigned mainSensorID, const std::vector<MUINT32> &allSensorIDs, TP_MASK_T masks, const IMetadata &meta, MFLOAT marginRatio, const  NSPipelinePlugin::TPICustomConfig &tpiCustomConfig)
{
    TRACE_FUNC_ENTER();
    mPluginPool = T_PluginType::getInstance(mainSensorID);
    if( mPluginPool != NULL )
    {
        T_NodeID id = 0; // must be between 0~1000
        std::shared_ptr<NSCam::IMetadata> metaPtr = std::make_shared<NSCam::IMetadata>(meta);
        for( auto &plugin : mPluginPool->getProviders(masks) )
        {
            ++id;
            T_Selection sel;
            T_Property prop = plugin->property();
            bool dual = isDualPlugin(prop.mFeatures);
            bool match = !!(prop.mFeatures&masks);
            bool run = false;
            MY_LOGI("negotiate[ID:%d]+ [0x%016" PRIx64 "][%s] match=%d isDualPlugin=%d", id, prop.mFeatures, prop.mName, match, dual);
            if( match )
            {
                sel = mPluginPool->createSelection();
                initCfgSelection(sel, metaPtr);
                configSelection(sel, allSensorIDs, tpiCustomConfig);
                plugin->negotiate(*sel);
                if( sel->mCfgRun )
                {
                    run = true;
                    mPluginMap[id] = PluginInfo(plugin, prop, sel);
                }
            }
            MY_LOGI("negotiate[ID:%d]- name=[%s] match=%d run=%d", id, prop.mName, match, run);
            if( match && run )
            {
                print(sel);
                checkPluginConfig(sel, marginRatio);
            }
        }
    }
    TRACE_FUNC_EXIT();
}

void TPIMgr_PluginWrapper::configSelection(const T_Selection &sel, const std::vector<MUINT32> &allSensorIDs, const  NSPipelinePlugin::TPICustomConfig &tpiCustomConfig)
{
    TRACE_FUNC_ENTER();
    if( sel != NULL )
    {
        sel->mCfgInfo.mMaxOutSize = MSize(mSession.mMaxOutWidth, mSession.mMaxOutHeight);
        sel->mCfgInfo.mStreamingSize = MSize(mSession.mMaxRawWidth, mSession.mMaxRawHeight);
        sel->mAllSensorIDs = allSensorIDs;
        sel->mTPICustomConfig = tpiCustomConfig;
    }
    TRACE_FUNC_EXIT();
}

void TPIMgr_PluginWrapper::checkPluginConfig(const T_Selection &sel, MFLOAT marginRatio)
{
    TRACE_FUNC_ENTER();
    if( sel != NULL )
    {
        MFLOAT ratio = (sel->mCfgMarginRatio+100)/100.0;
        if( sel->mCfgJoinEntry == eJoinEntry_S_DIV_2 &&
            ratio > marginRatio )
        {
            MY_LOGW("Plugin margin(%d) ratio(%.3f) exceeds Scenario margin ratio(%.3f)", sel->mCfgMarginRatio, ratio, marginRatio);
        }
    }
    TRACE_FUNC_EXIT();
}

void TPIMgr_PluginWrapper::destroyPluginSession()
{
  for( auto &info : mPluginMap )
  {
    info.second.uninit(info.first);
    info.second.mPlugin = NULL;
  }
  mPluginMap.clear();
  mPluginPool = NULL;
}

void TPIMgr_PluginWrapper::convertPluginToSession()
{
    NodeIDList yuvList;
    NodeIDList asyncList;
    NodeIDList dispList;
    NodeIDList metaList;
    NodeIDList divList;

    for( const auto &info : mPluginMap )
    {
        if( info.second.mSelection != NULL )
        {
            switch( info.second.mSelection->mCfgJoinEntry )
            {
            case NSPipelinePlugin::eJoinEntry_S_YUV:
                yuvList.push_back(info.first);
                break;
            case NSPipelinePlugin::eJoinEntry_S_ASYNC:
                asyncList.push_back(info.first);
                break;
            case NSPipelinePlugin::eJoinEntry_S_DISP_ONLY:
                dispList.push_back(info.first);
                break;
            case NSPipelinePlugin::eJoinEntry_S_META_ONLY:
                metaList.push_back(info.first);
                break;
            case NSPipelinePlugin::eJoinEntry_S_DIV_2:
                divList.push_back(info.first);
                break;
            default:
                break;
            }
        }
    }

    if( divList.size() > 1 )
    {
        MY_LOGW("Entry:%s count(%zu) exceeds limit(1), skip", toEntryName(eJoinEntry_S_DIV_2), divList.size());
        divList.clear();
    }

    build(mSession, mPluginMap, PathType::YUV, yuvList, divList);
    build(mSession, mPluginMap, PathType::ASYNC, asyncList);
    build(mSession, mPluginMap, PathType::DISP, dispList);
    build(mSession, mPluginMap, PathType::META, metaList);
}

void TPIMgr_PluginWrapper::updateResult(const T_Request &request, TPI_Image img[], size_t imgCount, TPI_Data &data)
{
    for( unsigned i = 0; i < imgCount; ++i )
    {
        updateImageInfo(request, img[i]);
    }
    updateData(request, data);
}

void TPIMgr_PluginWrapper::initRequest(T_Request &request)
{
    request->mOQueueCmd = eJoinQueueCmd_NONE;
}

void TPIMgr_PluginWrapper::fillMeta(T_Request &request, const TPI_Meta &meta)
{
    switch( meta.mMetaID )
    {
    case TPI_META_ID_MTK_IN_APP:
        request->mIMetadataApp = makeMetaHandle(meta);
        break;
    case TPI_META_ID_MTK_IN_P1_APP:
        request->mIMetadataDynamic1 = makeMetaHandle(meta);
        break;
    case TPI_META_ID_MTK_IN_P1_HAL:
        request->mIMetadataHal1 = makeMetaHandle(meta);
        break;
    case TPI_META_ID_MTK_IN_P1_APP_2:
        request->mIMetadataDynamic2 = makeMetaHandle(meta);
        break;
    case TPI_META_ID_MTK_IN_P1_HAL_2:
        request->mIMetadataHal2 = makeMetaHandle(meta);
        break;
    case TPI_META_ID_MTK_OUT_P2_APP:
        request->mOMetadataApp = makeMetaHandle(meta);
        break;
    case TPI_META_ID_MTK_OUT_P2_HAL:
        request->mOMetadataHal = makeMetaHandle(meta);
        break;
    default:
        break;
    }
}

void TPIMgr_PluginWrapper::fillBuffer(T_Request &request, const TPI_Image &img)
{
    switch( img.mBufferID )
    {
    case TPI_BUFFER_ID_MTK_YUV:
        request->mIBufferMain1 = makeBufferHandle(img);
        initImageInfo(img, request->mIBufferMain1_Info);
        break;
    case TPI_BUFFER_ID_MTK_YUV_2:
        request->mIBufferMain2 = makeBufferHandle(img);
        initImageInfo(img, request->mIBufferMain2_Info);
        break;
    case TPI_BUFFER_ID_MTK_DEPTH:
        request->mIBufferDepth = makeBufferHandle(img);
        initImageInfo(img, request->mIBufferDepth_Info);
        break;
    case TPI_BUFFER_ID_MTK_DEPTH_INTENSITY:
        // request->mIBufferDepthIntensity = makeBufferHandle(img);
        break;
    case TPI_BUFFER_ID_MTK_PURE:
        //request->mIBufferMain1 = makeBufferHandle(img);
        break;
    case TPI_BUFFER_ID_MTK_PURE_2:
        //request->mIBufferMain1 = makeBufferHandle(img);
        break;
    case TPI_BUFFER_ID_MTK_OUT_YUV:
        request->mOBufferMain1 = makeBufferHandle(img);
        initImageInfo(img, request->mOBufferMain1_Info);
        break;
    case TPI_BUFFER_ID_MTK_OUT_YUV_2:
        request->mOBufferMain2 = makeBufferHandle(img);
        initImageInfo(img, request->mOBufferMain2_Info);
        break;
    case TPI_BUFFER_ID_MTK_OUT_DISPLAY:
        request->mOBufferDisplay = makeBufferHandle(img);
        initImageInfo(img, request->mOBufferDisplay_Info);
        break;
    case TPI_BUFFER_ID_MTK_OUT_RECORD:
        request->mOBufferRecord = makeBufferHandle(img);
        initImageInfo(img, request->mOBufferRecord_Info);
        break;
    default:
        break;
    }
}

void TPIMgr_PluginWrapper::initImageInfo(const TPI_Image &img, JoinImageInfo &info)
{
    info.mISensorID = img.mViewInfo.mSensorID;
    info.mISensorSize = img.mViewInfo.mSensorSize;
    info.mISensorClip = img.mViewInfo.mSensorClip;

    info.mISrcZoomROI = img.mViewInfo.mSrcZoomROI;

    info.mOSrcImageID = eJoinImageID_DEFAULT;
    info.mOSrcImageClip = img.mViewInfo.mSrcImageClip;
    info.mODstImageClip = img.mViewInfo.mDstImageClip;
    info.mODstZoomROI = img.mViewInfo.mDstZoomROI;

    info.mOUseSrcImageBuffer = false;
}

void TPIMgr_PluginWrapper::updateImageInfo(const T_Request &request, TPI_Image &img)
{
    switch( img.mBufferID )
    {
    case TPI_BUFFER_ID_MTK_OUT_YUV:
        updateImageInfo(img, request->mOBufferMain1_Info);
        break;
    case TPI_BUFFER_ID_MTK_OUT_YUV_2:
        updateImageInfo(img, request->mOBufferMain2_Info);
        break;
    case TPI_BUFFER_ID_MTK_OUT_DISPLAY:
        updateImageInfo(img, request->mOBufferDisplay_Info);
        break;
    case TPI_BUFFER_ID_MTK_OUT_RECORD:
        updateImageInfo(img, request->mOBufferRecord_Info);
        break;
    default:
        break;
    }
}

void TPIMgr_PluginWrapper::updateImageInfo(TPI_Image &img, const JoinImageInfo &info)
{
    img.mUseSrcImageBuffer = info.mOUseSrcImageBuffer;
    if( img.mUseSrcImageBuffer )
    {
        switch(info.mOSrcImageID)
        {
        case eJoinImageID_MAIN1:
            img.mViewInfo.mSrcImageID = TPI_BUFFER_ID_MTK_YUV;
            break;
        case eJoinImageID_MAIN2:
            img.mViewInfo.mSrcImageID = TPI_BUFFER_ID_MTK_YUV_2;
            break;
        default:
            MY_LOGW("img(%d) use Src img replace, but unknown srcImgId(%d), ignore src inplace!!", img.mBufferID, info.mOSrcImageID);
            img.mUseSrcImageBuffer = MFALSE;
            break;
        }
    }
    img.mViewInfo.mSrcImageClip = info.mOSrcImageClip;
    img.mViewInfo.mDstImageClip = info.mODstImageClip;
    img.mViewInfo.mDstZoomROI = info.mODstZoomROI;
}

TPI_DATA_QUEUE_CMD toQueueCmd(JoinQueueCmd cmd)
{
  switch(cmd)
  {
  case eJoinQueueCmd_NONE:        return TPI_DATA_QUEUE_CMD_NONE;
  case eJoinQueueCmd_PUSH:        return TPI_DATA_QUEUE_CMD_PUSH;
  case eJoinQueueCmd_PUSH_POP:    return TPI_DATA_QUEUE_CMD_PUSH_POP;
  case eJoinQueueCmd_POP_ALL:     return TPI_DATA_QUEUE_CMD_POP_ALL;
  default:                        return TPI_DATA_QUEUE_CMD_NONE;
  }
}

void TPIMgr_PluginWrapper::updateData(const T_Request &request, TPI_Data &data)
{
    data.mQueueCmd = toQueueCmd(request->mOQueueCmd);
}

} // namespace NSFeaturePipe
} // namespace NSCamFeature
} // namespace NSCam
