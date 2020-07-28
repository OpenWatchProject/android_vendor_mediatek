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

/**
 * @file BaseBufferPoolMgr.h
 * @brief base classes of BufferPoolMgr
*/
#ifndef _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BUFFERPOOL_MGR_H_
#define _MTK_CAMERA_FEATURE_PIPE_DEPTH_MAP_BUFFERPOOL_MGR_H_

// Standard C header file
#include <vector>
// Android system/core header file
#include <utils/RefBase.h>
// mtkcam custom header file

// mtkcam global header file
#include <mtkcam3/feature/stereo/pipe/IDepthMapPipe.h>
#include <mtkcam3/feature/stereo/pipe/IDepthMapEffectRequest.h>

// Module header file

// Local header file
#include "BufferPool.h"
#include "../DepthMapPipe_Common.h"

/*******************************************************************************
* Namespace start.
********************************************************************************/
namespace NSCam {
namespace NSCamFeature {
namespace NSFeaturePipe_DepthMap {

using namespace NSCam::NSCamFeature::NSFeaturePipe;

#define CREATE_IMGBUF_POOL(rBufPool, name, size, format, usage, bSingle) \
    VSDOF_MDEPTH_LOGD("Create FatImageBufferPool:: "#name " size=%dx%d format=%x usage=%x", \
                size.w, size.h, format, usage); \
    rBufPool = FatImageBufferPool::create(name, size.w, size.h, format, usage);

#define CREATE_GRABUF_POOL(rBufPool, name, size, format, usage) \
    VSDOF_MDEPTH_LOGD("Create GraphicBufferPool:: "#name " size=%dx%d format=%x usage=%x", \
                size.w, size.h, format, usage); \
    rBufPool = GraphicBufferPool::create(name, size.w, size.h, format, usage);

#define ALLOCATE_BUFFER_POOL(bufPool, size) \
    VSDOF_MDEPTH_LOGD("ALLOCATE_BUFFER_POOL~" #bufPool " ---- %d", size);\
    if(!bufPool->allocate(size))\
    { \
        VSDOF_MDEPTH_LOGE("Allocate "#bufPool" image buffer failed"); \
        return MFALSE; \
    }

/*******************************************************************************
* Enum Definition
********************************************************************************/
typedef enum eBufferPoolScenario
{
    eBUFFER_POOL_SCENARIO_PREVIEW,   /*!< indicate the pool is used for preview */
    eBUFFER_POOL_SCENARIO_RECORD,    /*!< indicate the pool is used for record */
    eBUFFER_POOL_SCENARIO_CAPTURE    /*!< indicate the pool is used for capture */
} BufferPoolScenario;

typedef enum eDepthBufferType
{
    eBUFFER_IMAGE,      /*!< indicate the image buffer */
    eBUFFER_GRAPHIC,    /*!< indicate the gralloc buffer */
    eBUFFER_TUNING,     /*!< indicate the tuning buffer */

    // any buffer need to be define above this line.
    eBUFFER_SIZE

} DepthBufferType;

/*******************************************************************************
* Type Definition
********************************************************************************/

/**
 * @brief Buffer id map to record which buffers are inside the EffectRequest
 *        for all scenarios or for specific scenario
 */
typedef KeyedVector<DepthMapBufferID, BufferIOType> BufferIDMap;
typedef KeyedVector<BufferPoolScenario, BufferIDMap> RequestBufferMap;


/*******************************************************************************
* Class Definition
********************************************************************************/
class BaseBufferHandler;
/**
 * @class BaseBufferPoolMgr
 * @brief Base class of BufferPoolMgr
 */
class BaseBufferPoolMgr : public virtual android::VirtualLightRefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    BaseBufferPoolMgr() {}
    virtual ~BaseBufferPoolMgr() {}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  BaseBufferPoolMgr Public Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    /**
     * @brief request buffer from the bufferPool belong to the buffer id
     * @param [in] id buffer id used to request buffer.
     * @return
     * - requested buffer from buffer pool
     */
    virtual SmartFatImageBuffer request(
                            DepthMapBufferID id,
                            BufferPoolScenario scen
                            ) = 0;

    /**
     * @brief request graphic buffer from the bufferPool belong to the buffer id
     * @param [in] id buffer id used to request buffer.
     * @return
     * - requested buffer from buffer pool
     */
    virtual SmartGraphicBuffer requestGB(
                            DepthMapBufferID id,
                            BufferPoolScenario scen
                            ) = 0;

    /**
     * @brief request tuning buffer from the bufferPool belong to the buffer id
     * @param [in] id buffer id used to request buffer.
     * @return
     * - requested buffer from buffer pool
     */
    virtual SmartTuningBuffer requestTB(
                            DepthMapBufferID id,
                            BufferPoolScenario scen
                            ) = 0;

    /**
     * @brief create the BufferPoolHandler of this mgr
     * @return
     * - BufferHandler pointer
     */
    virtual sp<BaseBufferHandler> createBufferPoolHandler() = 0;

    /**
     * @brief query the buffer type of the buffer under specific scenario
     * @param [in] bid buffer id
     * @param [in] scen buffer scenario
     * @param [out] rOutBufType buffer type
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates the buffer is not working buffer.
     */
    virtual MBOOL queryBufferType(
                            DepthMapBufferID bid,
                            BufferPoolScenario scen,
                            DepthBufferType& rOutBufType) = 0;

    /**
     * @brief get the buffer pool for the bufferID
     * @param [int] bufferID buffer id
     * @return
     * - FatImageBufferPool for the buffer id
     */
    virtual sp<FatImageBufferPool> getImageBufferPool(
                            DepthMapBufferID bufferID,
                            BufferPoolScenario scenario=eBUFFER_POOL_SCENARIO_PREVIEW) = 0;

    /**
     * @brief get the graphic buffer pool for the bufferID
     * @param [int] bufferID buffer id
     * @return
     * - FatImageBufferPool for the buffer id
     */
    virtual sp<GraphicBufferPool> getGraphicImageBufferPool(
                            DepthMapBufferID bufferID,
                            BufferPoolScenario scenario=eBUFFER_POOL_SCENARIO_PREVIEW) = 0;

    /**
     * @brief query the FEO buffer size from the input buffer size and block size
     * @param [in] iBufSize fe input buffer size
     * @param [in] iBlockSize FE block size
     * @param [out] rFEOSize FEO buffer size
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL queryFEOBufferSize(MSize iBufSize, MUINT iBlockSize, MSize& rFEOSize)
    {

        rFEOSize.w = iBufSize.w/iBlockSize*40;
        rFEOSize.h = iBufSize.h/iBlockSize;

        VSDOF_MDEPTH_LOGD("iBufSize=%dx%d iBlockSize=%d rFEOSize=%dx%d",
                iBufSize.w, iBufSize.h, iBlockSize, rFEOSize.w, rFEOSize.h);
        return MTRUE;
    }
    /**
     * @brief query the FMO buffer size from the input FEO buffer size and block size
     * @param [in] szFEOBuffer feo buffer size
     * @param [out] rFMOSize FMO buffer size
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    MBOOL queryFMOBufferSize(MSize szFEOBuffer, MSize& rFMOSize)
    {
        rFMOSize.w = (szFEOBuffer.w/40) * 2;
        rFMOSize.h = szFEOBuffer.h;
        VSDOF_MDEPTH_LOGD("szFEOBuffer=%dx%d rFMOSize=%dx%d", szFEOBuffer.w, szFEOBuffer.h, rFMOSize.w, rFMOSize.h);
        return MTRUE;
    }

    /**
     * @brief get the all the image buffer of the buffer id's pool of all buffer scenario
     * @param [in] id buffer id
     * @param [in] scen scenario
     * @param [out] rVec output buffer vector
     * @return
     * - MTRUE indicates success.
     * - MFALSE indicates failure.
     */
    virtual MBOOL getAllPoolImageBuffer(
                        DepthMapBufferID id,
                        BufferPoolScenario scen,
                        std::vector<IImageBuffer*>& rImgVec
                        ) = 0;
};

typedef KeyedVector<BufferPoolScenario,sp<FatImageBufferPool> > ScenarioToImgBufPoolMap;
typedef KeyedVector<BufferPoolScenario,sp<GraphicBufferPool> > ScenarioToGraBufPoolMap;
typedef KeyedVector<BufferPoolScenario, DepthBufferType> BufScenarioToTypeMap;

}; // NSFeaturePipe_DepthMap
}; // NSCamFeature
}; // NSCam

#endif