#define LOG_TAG "MtkCam/MFCNode"
#include <mtkcam/utils/std/Log.h>

/* MtkCam related headers */
#include "MFCNodeImp.h"
#include "hwnode_utilities.h"

#include <mtkcam/drv/iopipe/SImager/IImageTransform.h> // IImageTransform
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>

#include <mtkcam/utils/metastore/IMetadataProvider.h>
#include <mtkcam/utils/metadata/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv/IHalSensor.h>

#include <mtkcam/middleware/v1/LegacyPipeline/StreamId.h>

#include <mtkcam/feature/eis/eis_ext.h> // for EIS_REGION_INDE

/* andorid */
#include <utils/RWLock.h>
#include <utils/Thread.h>
#include <sys/prctl.h>
#include <sys/resource.h>

/* std */
#include <vector>
#include <memory>
#include <functional>

/* SwNR */
#include <mtkcam/aaa/ICaptureNR.h>
#include <mtkcam/drv/iopipe/SImager/ISImager.h>
#include <mtkcam/feature/utils/ImageBufferUtils.h>


#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] @@@warning:@@@" fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] @@@error:@@@ " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NS3Av3;
using namespace std;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc;
using namespace NSCam::NSIoPipe::NSSImager;

using NSCam::NSIoPipe::Input;
using NSCam::NSIoPipe::Output;
using NSCam::NSIoPipe::QParams;
using NSCam::NSIoPipe::PortID;
using NSCam::NSIoPipe::NSSImager::IImageTransform;

//-----------------------------------------------------------------------------
CRequestFrame::FrameImage::FrameImage(
        std::map<CRequestFrame::eRequestImg, StreamId_T> &STREAMIDs)
: streamId(NULL)
, nodeId(0)
, bError(false)
, STREAMIDs(STREAMIDs)
{
}
//-----------------------------------------------------------------------------
CRequestFrame::FrameImage::~FrameImage()
{
    MY_LOGD("delete FrameImage streamId:%zu, nodeId:%d, callerName:%s",
            streamId,
            nodeId,
            callerName.c_str());

    if (imageBuffer.get()) {
        /* unlock image buffer & stream buffer */
        imageBuffer->unlockBuf(callerName.c_str());
        /* unlock image buffer */
        if (streamBuffer.get())
            streamBuffer->unlock(callerName.c_str(), imageBuffer->getImageBufferHeap());
    }
    else {
        MY_LOGD("destroying FrameImage, IImageBuffer is NULL");
    }

    if (streamBuffer.get()) {
        /* mark WRITE_OK if stream id is full raw or resized raw */
        if (streamId == STREAMIDs[eRequestImg_FullRaw] || streamId == STREAMIDs[eRequestImg_ResizedRaw])
            streamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        /* if mark as error, mark it */
        if (bError)
            streamBuffer->markStatus(STREAM_BUFFER_STATUS::ERROR);
    }
    else {
        MY_LOGD("destroying FrameImage, IImageStreamBuffer is NULL");
    }

lbExit:
    if (pipelineFrame.get()) {
        /* mark buffer as USED|RELEASE */
        pipelineFrame->getStreamBufferSet().markUserStatus(
                streamId,
                nodeId,
                IUsersManager::UserStatus::USED | IUsersManager::UserStatus::RELEASE);
        pipelineFrame->getStreamBufferSet().applyRelease(nodeId); // TODO: should this be ok??
    }
}
//-----------------------------------------------------------------------------
CRequestFrame::FrameMetadata::FrameMetadata(
        std::map<CRequestFrame::eRequestMeta, StreamId_T> &STREAMIDs)
: streamId(NULL)
, nodeId(0)
, pMetadata(NULL)
, bError(false)
, STREAMIDs(STREAMIDs)
{
}
//-----------------------------------------------------------------------------
CRequestFrame::FrameMetadata::~FrameMetadata()
{
    MY_LOGD("delete FrameMetadata streamId:%zu, nodeId:%d, callerName:%s",
            streamId,
            nodeId,
            callerName.c_str());

    if (streamBuffer.get()) {
        /* unlock metadata */
        streamBuffer->unlock(callerName.c_str(), pMetadata);
        /* mark WRITE_OK if stream is neither IN_APP_RESULT nor IN_HAL_RESULT */
        if (streamId != STREAMIDs[eRequestMeta_InAppRequest] &&
                streamId != STREAMIDs[eRequestMeta_InHalP1])
            streamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        /* if marked as error, mark it */
        if (bError)
            streamBuffer->markStatus(STREAM_BUFFER_STATUS::ERROR);
    }
    else {
        MY_LOGD("destroying FrameMetadata, IMetaStreamBuffer is NULL");
    }

lbExit:
    if (pipelineFrame.get()) {
        /* mark buffer as USED|RELEASE */
        pipelineFrame->getStreamBufferSet().markUserStatus(
                streamId,
                nodeId,
                IUsersManager::UserStatus::USED | IUsersManager::UserStatus::RELEASE);
        pipelineFrame->getStreamBufferSet().applyRelease(nodeId); // TODO: should this be ok?
    }
}
//-----------------------------------------------------------------------------
CRequestFrame::CRequestFrame(
        MFCNodeImp *caller,
        sp<IPipelineFrame> &f,
        std::function<std::map<eRequestImg, StreamId_T>(void)> imageStreamInfoProvider,
        std::function<std::map<eRequestMeta, StreamId_T>(void)> metadataStreamInfoProvider,
        int index /* -1 */)
: m_node(caller)
, m_pipelineFrame(f)
, m_frameStatus(-1)
, m_index(index)
{
    /* move std::map elements */
    m_streamIdImages = imageStreamInfoProvider();
    m_streamIdMetadatas = metadataStreamInfoProvider();

    /* process IPipelineFrame */
    m_processFrame = std::async(std::launch::async, [this]()->void {
        MERROR err = OK;
        /**
         *  Notice that, IPipelineFrame frame has a mechanism that all the resource
         *  should be released and then eMSG_FRAME_RELEASED will be signed. If not,
         *  perhaps a deadlock will happen during waiting request drain.
         *
         *  If any error happened, release all resource only (applyRelease)
         */
        bool bReleaseOnly = false;

        /* check if m_pipelineFrame exists */
        if (m_pipelineFrame.get() == NULL) {
            return; // status will be -1
        }

        const MUINT32 FRAME_NO = m_pipelineFrame->getFrameNo();
        IStreamBufferSet &rStreamBufferSet = m_pipelineFrame->getStreamBufferSet();
        IPipelineFrame::InfoIOMapSet infoIOMapSet;
        /* get PipelineFrame's InfoIOMapSet */
        if (OK != [this](IPipelineFrame::InfoIOMapSet &rIOMapSet)->MERROR
        {
            if (OK != m_pipelineFrame->queryInfoIOMapSet(m_node->getNodeId(), rIOMapSet)) {
                MY_LOGE("queryInfoIOMap failed, returns NAME_NOT_FOUND");
                return NAME_NOT_FOUND;
            }

            /* check image part */
            IPipelineFrame::ImageInfoIOMapSet &imageIOMapSet = rIOMapSet.mImageInfoIOMapSet;
            if (imageIOMapSet.size() <= 0) {
                MY_LOGE("no imageIOMap in the IPipelineFrame");
                return BAD_VALUE;
            }
            return OK;
        }(infoIOMapSet))
        {
            MY_LOGE("get PipelineFrame's InfoIOMapSet failed");
            return;
        }

        /* BaseNodeWrapper, to makes these two methods public */
        // {{{
        class BaseNodeWrapper : public BaseNode {
        public:
            virtual MERROR ensureImageBufferAvailable_(
                    MUINT32 const frameno,
                    StreamId_T const streamId,
                    IStreamBufferSet &streamBufferSet,
                    sp<IImageStreamBuffer> &imageStreamBuffer,
                    MBOOL b = MTRUE) override
            {
                return BaseNode::ensureImageBufferAvailable_(
                        frameno,
                        streamId,
                        streamBufferSet,
                        imageStreamBuffer,
                        b);
            };
            virtual MERROR ensureMetaBufferAvailable_(
                    MUINT32 const frameno,
                    StreamId_T const streamId,
                    IStreamBufferSet &streamBufferSet,
                    sp<IMetaStreamBuffer> &metaStreamBuffer,
                    MBOOL b = MTRUE) override
            {
                return BaseNode::ensureMetaBufferAvailable_(
                        frameno,
                        streamId,
                        streamBufferSet,
                        metaStreamBuffer,
                        b);
            };
        } *nodeWrapper =
            reinterpret_cast<BaseNodeWrapper*>
                (
                 static_cast<BaseNode*>(m_node) // cast MFCNodeImp to Base Node first
                );
        // }}}

        /* metadata part */
        do { // {{{
            IPipelineFrame::MetaInfoIOMapSet const &mapSet =
                infoIOMapSet.mMetaInfoIOMapSet;

            /* traverse metadata info set */
            for (size_t i = 0; i < mapSet.size(); i++) {
                /* declare a lambda function to retrieve metadata */
                // {{{
                auto retrieveMetadata =
                    [this, &mapSet, &i, &FRAME_NO, &rStreamBufferSet, &nodeWrapper]
                    (int inout, int index, bool bReleaseOnly) -> int
                {
                    MERROR err = OK;

                    std::shared_ptr<FrameMetadata> m =
                        std::shared_ptr<FrameMetadata>(new FrameMetadata(m_streamIdMetadatas));

                    /* configure FrameMetadata */
                    m->pMetadata     = NULL;
                    m->callerName    = m_node->getNodeName();
                    m->nodeId        = m_node->getNodeId();
                    m->pipelineFrame = m_pipelineFrame;
                    m->streamId      = (inout == 0
                        ? mapSet[i].vIn[index]->getStreamId()
                        : mapSet[i].vOut[index]->getStreamId());

                    if (bReleaseOnly)
                        return -1;

                    err = nodeWrapper->ensureMetaBufferAvailable_(
                        FRAME_NO, m->streamId, rStreamBufferSet, m->streamBuffer);
                    if (err != OK) {
                        MY_LOGE("src metadata err = %d(%s)", err, strerror(-err));
                        if (m->streamBuffer == NULL)
                            MY_LOGE("metaStreamBuffer is NULL");
                        return -1;
                    }

                    /* get metadata from meta stream buffer */
                    err = [this, &inout, &m, &i, &index]()->MERROR
                    {
                        if (inout == 0)
                            m->pMetadata = m->streamBuffer->tryReadLock(m->callerName.c_str());
                        else
                            m->pMetadata = m->streamBuffer->tryWriteLock(m->callerName.c_str());

                        if (m->pMetadata == NULL) {
                            MY_LOGE("node(%d) strea buffer(%s): cannot get metadata", m->nodeId, m->callerName.c_str());
                            return INVALID_OPERATION;
                        }
                        return OK;
                    }();

                    if (err != OK) {
                        MY_LOGE("get metadata failed");
                        return -1;
                    }

                    /* check MTK_HAL_REQUEST_ERROR_FRAME if is 1, if yes, discard this frame */
                    {
                        MUINT8 isDrop = 0;
                        IMetadata::getEntry<MUINT8>(m->pMetadata, MTK_HAL_REQUEST_ERROR_FRAME, isDrop);
                        if (isDrop != 0)
                            return -1;
                    }

                    /* check the appMeta is empty or not, if yes, this frame should be discarded */
                    if (m->pMetadata->isEmpty()) {
                        if (m->streamId == m_streamIdMetadatas[eRequestMeta_InAppRequest] ||
                                m->streamId == m_streamIdMetadatas[eRequestMeta_InHalP1]) {
                            MY_LOGW("metadata InAppRequest is empty, discard this frame");
                            return -1;
                        }
                    }

                    MY_LOGD("process metadata streamId:%zu", m->streamId);
                    // matching by value
                    do {
                        bool bMatched = false;
                        for (size_t i = 0; i < (size_t)eRequestMeta_Size; i++) {
                            if (m->streamId == m_streamIdMetadatas[(eRequestMeta)i]) {
                                bMatched = true;
                                m_frameMetadatas[(eRequestMeta)i] = m;
                                break;
                            }
                        }
                        if (bMatched == false) { // not found
                            MY_LOGE("CRequestFrame: unhandled metadata stream ID %#x", (MUINT32)m->streamId);
                            m_frameMetadataOthers.enque(m);
                        }
                    } while(0);
                    return 0;
                };// retrieveMetadata
                //}}}

                /* input metadata */
                for (size_t j = 0; j < mapSet[i].vIn.size(); j++) {
                    err |= retrieveMetadata(0, j, bReleaseOnly);
                    if (err != OK)
                        bReleaseOnly = true;

                }
                /* output metadata */
                for (size_t j = 0; j < mapSet[i].vOut.size(); j++) {
                    err |= retrieveMetadata(1, j, bReleaseOnly);
                    if (err != OK)
                        bReleaseOnly = true;
                }
            }// for (size_t i = 0; i < mapSet.size(); i++)
        } while(0); // metadata part }}}

        /* images parts */
        do { // {{{
            IPipelineFrame::ImageInfoIOMapSet const &mapSet =
                infoIOMapSet.mImageInfoIOMapSet;
            // traverse image info set
            for (size_t i = 0; i < mapSet.size(); i++) {
                /* declare a retrieveImageBuffer helper lambda function */
                // {{{
                auto retrieveImageBuffer =
                    [this, &mapSet, &i, &FRAME_NO, &rStreamBufferSet, &nodeWrapper]
                    (int inout, int index, bool bReleaseOnly = false) -> int
                {
                    MERROR err = OK;
                    /* create FrameImage instance based on in/out selector */
                    std::shared_ptr<FrameImage> in(new FrameImage(m_streamIdImages));
                    /* construct FrameImage */
                    in->nodeId = m_node->getNodeId();
                    in->pipelineFrame = m_pipelineFrame;
                    in->callerName = m_node->getNodeName();
                    in->streamId = inout == 0
                        ? mapSet[i].vIn[index]->getStreamId()
                        : mapSet[i].vOut[index]->getStreamId();

                    /* If release only, just invoke destructor of FrameImage */
                    if (bReleaseOnly)
                        return -1;

                    /* ensure image buffer is available */
                    err = nodeWrapper->ensureImageBufferAvailable_(
                            FRAME_NO, in->streamId, rStreamBufferSet, in->streamBuffer);

                    if (OK != err) {
                        MY_LOGE("src buffer err = %d(%s)", err, strerror(-err));
                        if (in->streamBuffer.get() == NULL)
                            MY_LOGE("pStreamBuffer is NULL");
                        return -1;
                    }

                    /* get imagebuffer from image buffer heap which is belong to a streamBuffer */
                    err = [this, &inout, &in, &i, &index]()->MERROR
                    {
                        MUINT const groupUsage = in->streamBuffer->queryGroupUsage(m_node->getNodeId());
                        if (inout == 0)
                            in->imageBufferHeap = in->streamBuffer->tryReadLock(m_node->getNodeName());
                        else
                            in->imageBufferHeap = in->streamBuffer->tryWriteLock(m_node->getNodeName());

                        if (in->imageBufferHeap == NULL) {
                            MY_LOGE("get image buffer heap failed. inout=%d, i=%zu, index=%d", inout, i, index);
                            return INVALID_OPERATION;
                        }

                        /* create imageBuffer from image buffer heap */
                        in->imageBuffer = in->imageBufferHeap->createImageBuffer();
                        if (in->imageBuffer == NULL) {
                            MY_LOGE("get image buffer failed. inout=%d, i=%zu, index=%d", inout, i, index);
                            return INVALID_OPERATION;
                        }

                        in->imageBuffer->lockBuf(m_node->getNodeName(), groupUsage);
                        return OK;
                    }();

                    if (err != OK) {
                        MY_LOGE("get imageBuffer failed");
                        return -1;
                    }

                    MY_LOGD("process image streamId:%zu", in->streamId);
                    do {
                        bool bMatched = false;
                        for (size_t i = 0; i < (size_t)eRequestImg_Size; i++) {
                            if (in->streamId == m_streamIdImages[(eRequestImg)i]) {
                                bMatched = true;
                                m_frameImages[(eRequestImg)i] = in;
                                break;
                            }
                        }
                        if (bMatched == false) {
                            MY_LOGE("CRequestFrame: unhandled image stream ID %#x", (MUINT32)in->streamId);
                            m_frameImageOthers.enque(in);
                        }
                    } while(0);
                    return 0;
                };// retrieveImageBuffer
                // }}}

                /* retrieve input image buffer */
                for (size_t j = 0; j < mapSet[i].vIn.size(); j++) {
                    err |= retrieveImageBuffer(0, j, bReleaseOnly);
                if (err != OK)
                    bReleaseOnly = true;
                }

                /* retrieve output image buffer */
                for (size_t j = 0; j < mapSet[i].vOut.size(); j++) {
                    err |= retrieveImageBuffer(1, j, bReleaseOnly);
                    if (err != OK)
                        bReleaseOnly = true;
                }

            }//for (size_t i = 0; i < mapSet.size(); i++)
        } while(0); // image part }}}

        /* parse IPipelineFrame ok, returns 0 */
        m_frameStatus = err;
    });
}
//-----------------------------------------------------------------------------
CRequestFrame::~CRequestFrame()
{
    m_processFrame.wait();

    MY_LOGD("release CRequestFrame index=%d +", index());

    /**
     *  before invoke IStreamBufferSet::applyRelease, we need remove all FrameImage
     *  and FrameMetadata
     */
    m_frameImageOthers.clear(); // for those IImageStreamBuffer that we don't care but received
    m_frameMetadataOthers.clear(); // for those IMetaStreamBuffer that we don't care but received
    m_frameImages.clear();
    m_frameMetadatas.clear();
    MY_LOGD("release CRequestFrame index=%d -", index());
}
//-----------------------------------------------------------------------------
IImageBuffer* CRequestFrame::imageBuffer(const eRequestImg &req)
{
    m_processFrame.wait();
    std::shared_ptr<FrameImage> &img = m_frameImages[req];
    if (img.get() == NULL)
        return NULL;
    Mutex::Autolock(&img->mx);
    return img->imageBuffer.get();
}
//-----------------------------------------------------------------------------
sp<IPipelineFrame>& CRequestFrame::pipelineFrame()
{
    m_processFrame.wait();
    return m_pipelineFrame;
}
//-----------------------------------------------------------------------------
IMetadata* CRequestFrame::metadata(const eRequestMeta &req)
{
    m_processFrame.wait();
    std::shared_ptr<FrameMetadata> &meta = m_frameMetadatas[req];
    if (meta.get() == NULL)
        return NULL;
    Mutex::Autolock(&meta->mx);
    return meta->pMetadata;
}
//-----------------------------------------------------------------------------
int CRequestFrame::markError(const eRequestImg& req)
{
    m_processFrame.wait();
    if (req == eRequestImg_All) {
        for (int i = 0; i < static_cast<int>(eRequestImg_All); i++)
            markError(static_cast<eRequestImg>(i));
        return 0;
    }
    std::shared_ptr<FrameImage>& img = m_frameImages[req];
    if (img.get()) {
        Mutex::Autolock(&img->mx);
        img->bError = true;
        return 0;
    }
    else {
        return -1;
    }
}
//-----------------------------------------------------------------------------
int CRequestFrame::releaseImage(const eRequestImg &req)
{
    m_processFrame.wait();
    if (req == eRequestImg_All) {
        for (unsigned int i = 0; i < (unsigned int)eRequestImg_All; i++)
            releaseImage((eRequestImg)i);
        return 0;
    }


    std::shared_ptr<FrameImage> &img = m_frameImages[req];
    if (img.get() == NULL)
        return -1;
    img = 0;
    return 0;
}
//-----------------------------------------------------------------------------
int CRequestFrame::releaseMetadata(const eRequestMeta &req)
{
    m_processFrame.wait();
    if (req == eRequestMeta_All) {
        for (unsigned int i = 0; i < (unsigned int)eRequestMeta_All; i++)
            releaseMetadata((eRequestMeta)i);
        return 0;
    }


    std::shared_ptr<FrameMetadata> &meta = m_frameMetadatas[req];
    if (meta.get() == NULL)
        return -1;
    meta = 0;
    return 0;
}
//-----------------------------------------------------------------------------
int CRequestFrame::status()
{
    m_processFrame.wait();
    return m_frameStatus;
}
//-----------------------------------------------------------------------------
android::sp<MFCNode> MFCNode::createInstance(const MfcAttribute &attr /* = MfcAttribute() */)
{
    MY_LOGD("To be determined, is time share=%d", attr.is_time_sharing);
    return NULL;
}

MFCNodeImp::~MFCNodeImp(void)
{
}

MERROR MFCNodeImp::init(InitParams const& rParams)
{
    mOpenId   = rParams.openId;
    mNodeId   = rParams.nodeId;
    mNodeName = rParams.nodeName;
    return OK;
}

MERROR MFCNodeImp::uninit(void)
{
    if (flush() != OK) {
        MY_LOGE("Flush failed");
    }
    return OK;
}

MERROR MFCNodeImp::config(const ConfigParams& rParams)
{
    CAM_TRACE_CALL();

    MY_LOGD("config param");

    // check ConfigParams
    MERROR const err = verifyConfigParams(rParams);
    if (err != OK)
    {
        MY_LOGE("verifyConfigParams failed: err = %d", err);
        return err;
    }

    flush();

    // configure streams into node
    {
        // meta
        mpInAppMeta_Request = rParams.pInAppMeta;
        mpInHalMeta_P1      = rParams.pInHalMeta;
        mpOutAppMeta_Result = rParams.pOutAppMeta;
        mpOutHalMeta_Result = rParams.pOutHalMeta; // not necessary
        // image
        mpInFullRaw         = rParams.vInFullRaw[0];
        mpInResizedRaw      = rParams.pInResizedRaw;
        mvOutImages         = rParams.vOutImage;
        // only if lcso RAW is available, catch it
        if (rParams.vInLcsoRaw.size() > 0) {
            mpInLcsoRaw = rParams.vInLcsoRaw[0];
    }
    }

    return OK;
}

MERROR MFCNodeImp::queue(sp<IPipelineFrame> pFrame)
{
    CAM_TRACE_NAME("queue(MFC)");

    MY_LOGD("queue frame %p", pFrame.get());

    if (!pFrame.get()) {
        MY_LOGE("null frame");
        return BAD_VALUE;
    }

    std::shared_ptr<CRequestFrame> rqframe(
        new CRequestFrame(
            this,  // caller
            pFrame, // IPipelineFrame pointer
            [this]()->std::map<CRequestFrame::eRequestImg, StreamId_T> {
                std::map<CRequestFrame::eRequestImg, StreamId_T> set;
                set[CRequestFrame::eRequestImg_FullRaw] = eSTREAMID_IMAGE_PIPE_RAW_OPAQUE;
                set[CRequestFrame::eRequestImg_ResizedRaw] = eSTREAMID_IMAGE_PIPE_RAW_RESIZER;
                set[CRequestFrame::eRequestImg_LcsoRaw] = eSTREAMID_IMAGE_PIPE_RAW_LCSO;
                set[CRequestFrame::eRequestImg_YuvJpeg] = eSTREAMID_IMAGE_PIPE_YUV_JPEG;
                set[CRequestFrame::eRequestImg_YuvThumbnail] = eSTREAMID_IMAGE_PIPE_YUV_THUMBNAIL;
                set[CRequestFrame::eRequestImg_YuvPostview] = eSTREAMID_IMAGE_PIPE_YUV_00;
                return set;
            }, // image buffer stream id set provider
            [this]()->std::map<CRequestFrame::eRequestMeta, StreamId_T> {
                std::map<CRequestFrame::eRequestMeta, StreamId_T> set;
                if (mpInAppMeta_Request.get())
                    set[CRequestFrame::eRequestMeta_InAppRequest] = mpInAppMeta_Request->getStreamId();
                if (mpInHalMeta_P1.get())
                    set[CRequestFrame::eRequestMeta_InHalP1] = mpInHalMeta_P1->getStreamId();
                if (mpInFullRaw.get())
                    set[CRequestFrame::eRequestMeta_InFullRaw] = mpInFullRaw->getStreamId();
                if (mpInResizedRaw.get())
                    set[CRequestFrame::eRequestMeta_InResizedRaw] = mpInResizedRaw->getStreamId();
                if (mpOutAppMeta_Result.get())
                    set[CRequestFrame::eRequestMeta_OutAppResult] = mpOutAppMeta_Result->getStreamId();
                if (mpOutHalMeta_Result.get())
                    set[CRequestFrame::eRequestMeta_OutHalResult] = mpOutHalMeta_Result->getStreamId();
                return set;
            }, // metadata stream id set provide
            m_index++  // index for debug
        )
    );

    /* saves the first IPipelineFrame as main IPipelineFrame */
    if (mPipelineFrame.get() == NULL) {
        mPipelineFrame = pFrame;
    }
    mRequestFrameQueue.enque(rqframe);

    CAM_TRACE_INT("request(MFC)", mRequestFrameQueue.size());
    return OK;
}

std::shared_ptr<CRequestFrame> MFCNodeImp::deque(const int &timeoutsec)
{
    CAM_TRACE_NAME("deque(MFC)");
    MY_LOGD("deque frame");
    return mRequestFrameQueue.deque(timeoutsec);
}

MERROR MFCNodeImp::flush()
{
    MY_LOGD("flush +");
    /* flush all IPipelineFrame of CRequestFrame in queue*/
    CRequestFrame::flushClearAll(mRequestFrameQueue, this);
    MY_LOGD("flush -");
    return OK;
}

MERROR MFCNodeImp::flush(sp<IPipelineFrame> const &pFrame)
{
    MY_LOGD("flush TBD+");
    if(pFrame.get()) {
        MY_LOGD("Flush with arg to be determined");
    }
    MY_LOGD("flush TBD-");
    return OK;
}

MVOID MFCNodeImp::onDispatchFrame()
{
    releaseResource();
    if (mPipelineFrame.get() == NULL) {
        MY_LOGE("%s: mPipelineFrame is NULL", __FUNCTION__);
    }
    /* invoke BaseNode::onDispatchFrame(sp<IPipelineFrame>&) */
    BaseNode::onDispatchFrame(mPipelineFrame);
    mPipelineFrame = 0; // release the last IPipelineFrame ASAP
}

MVOID MFCNodeImp::onDispatchFrame(sp<IPipelineFrame> const& pFrame)
{
    MY_LOGD("onDispatchFrame TBD+");
    if(pFrame.get()) {
        MY_LOGD("onDispatchFrame with arg to be determined");
    }
    MY_LOGD("onDispatchFrame TBD-");
    return;
}

MVOID MFCNodeImp::releaseResource()
{
    /* we need clear all CRequestFrame here */
    mRequestFrameQueue.clear();
}

void MFCNodeImp::dumpStreamIfExist(const char* str, const sp<IStreamInfo>& streamInfo)
{
    MY_LOGD_IF(streamInfo.get(), "%s: streamId(%#" PRIx64 ") %s",
            str, streamInfo->getStreamId(), streamInfo->getStreamName());
}

void MFCNodeImp::dumpCaptureParam(const CaptureParam_T &captureParam, const char* msg /* = NULL */)
{
    if (msg) MY_LOGD("%s", msg);

    MY_LOGD("=================");
    MY_LOGD("u4ExposureMode(%d)", captureParam.u4ExposureMode);
    MY_LOGD("u4Eposuretime(%d)", captureParam.u4Eposuretime);
    MY_LOGD("u4AfeGain(%d)", captureParam.u4AfeGain);
    MY_LOGD("u4IspGain(%d)", captureParam.u4IspGain);
    MY_LOGD("u4RealISO(%d)", captureParam.u4RealISO);
    MY_LOGD("u4FlareGain(%d)", captureParam.u4FlareGain);
    MY_LOGD("u4FlareOffset(%d)", captureParam.u4FlareOffset);
    MY_LOGD("i4LightValue_x10(%d)", captureParam.i4LightValue_x10);
}

void MFCNodeImp::dumpExifInfo(const IMetadata &halMeta)
{
    IMetadata exifMeta;
    MBOOL result = MFALSE;
    struct T {
        MINT32 val;
        MBOOL result;
        T() : val(-1), result(MFALSE) {};
    } iso, exp;

    result = IMetadata::getEntry<IMetadata>(
            const_cast<IMetadata*>(&halMeta),
            MTK_3A_EXIF_METADATA,
            exifMeta);
    if (result == MFALSE) {
        goto lbExit;
    }

    iso.result = IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED,        iso.val);
    exp.result = IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_CAP_EXPOSURE_TIME,   exp.val);

lbExit:
    MY_LOGD("%s:=========================", __FUNCTION__);
    MY_LOGD("%s: Has 3A exif metadata: %d", __FUNCTION__, result);
    MY_LOGD("%s: Get Iso -> %d, iso->: %d", __FUNCTION__, iso.result, iso.val);
    MY_LOGD("%s: Get Exp -> %d, exp->: %d", __FUNCTION__, exp.result, exp.val);
    MY_LOGD("%s:=========================", __FUNCTION__);
}

MERROR MFCNodeImp::verifyConfigParams(const ConfigParams &rParams)
{
    if (!rParams.pInAppMeta.get()) {
        MY_LOGE("out in app metadata");
        return BAD_VALUE;
    }

    if (!rParams.pInHalMeta.get()) {
        MY_LOGE("no in hal metadata");
        return BAD_VALUE;
    }

    // TODO: need to check out metadata?
    //if  ( ! rParams.pOutAppMeta.get() ) {
    //    return BAD_VALUE;
    //}
    //if  ( ! rParams.pOutHalMeta.get() ) {
    //    return BAD_VALUE;
    //}

    if (rParams.vInFullRaw.size() == 0) {
        MY_LOGE("no in image fullraw");
        return BAD_VALUE;
    }

    if (rParams.vInLcsoRaw.size() <= 0) {
        MY_LOGD("no lcso buffer");
    }
    else {
        MY_LOGD("enable lcso buffer");
    }

    // resized raw can be null; just log for notification
    if (!rParams.pInResizedRaw.get()) {
        MY_LOGD("no in resized raw");
    }

    if (rParams.vOutImage.size() == 0) {
        MY_LOGD("no out yuv image for postview(maybe...)");
    }

    // dump all streams
    size_t count;

    dumpStreamIfExist("[meta] in app", rParams.pInAppMeta);
    dumpStreamIfExist("[meta] in hal", rParams.pInHalMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);

    count = rParams.vInFullRaw.size();
    for (size_t i = 0; i < count; i++) {
        dumpStreamIfExist("[image] in full", rParams.vInFullRaw[i]);
    }

    count = rParams.vInLcsoRaw.size();
    for (size_t i = 0; i < count; i++) {
        dumpStreamIfExist("[image] in lcso", rParams.vInLcsoRaw[i]);
    }

    dumpStreamIfExist("[image] in resized", rParams.pInResizedRaw);

    count = rParams.vOutImage.size();
    for (size_t i = 0; i < count; i++) {
        dumpStreamIfExist("[image] out yuv", rParams.vOutImage[i]);
    }

    return OK;
}

static MINT32 getIso(IMetadata *metaHal)
{
    MINT32 iso = 0;
    IMetadata exifMeta;
    MBOOL result = MFALSE;

    /* get exif metadata first, which contains ISO information */
    result = IMetadata::getEntry<IMetadata>(
            metaHal,
            MTK_3A_EXIF_METADATA,
            exifMeta);
    if (result == MFALSE) {
        MY_LOGW("ignore SWNR due to no exif metadata");
        goto lbExit;
    }

    result = IMetadata::getEntry<MINT32>(&exifMeta, MTK_3A_EXIF_AE_ISO_SPEED, iso);
    if (result != MTRUE) {
        MY_LOGW("ignore SWNR due to query ISO fail");
        goto lbExit;
    }

lbExit:
    MY_LOGD("[%s] iso = %d", __FUNCTION__, iso);
    return iso;
}

MINT32 MFCNodeImp::nrStrategy(MINT32 iso, IMetadata *metaHal)
{
    MINT32 nrType = MTK_NR_MODE_OFF;

    // query NR mode
    if (!IMetadata::getEntry<MINT32>(metaHal, MTK_NR_MODE, nrType))
        MY_LOGW("no MTK_NR_MODE");

    if (nrType == MTK_NR_MODE_AUTO) {
        MINT32 thr_mnr = 0x7FFFFFFF;
        MINT32 thr_swnr = 0x7FFFFFFF;

        if (!IMetadata::getEntry<MINT32>(metaHal, MTK_NR_SWNR_THRESHOLD_ISO, thr_swnr))
            MY_LOGW("no MTK_NR_SWNR_THRESHOLD_ISO");

        if (!IMetadata::getEntry<MINT32>(metaHal, MTK_NR_MNR_THRESHOLD_ISO, thr_mnr))
            MY_LOGW("no MTK_NR_MNR_THRESHOLD_ISO");

        if (iso >= thr_swnr)
            nrType = MTK_NR_MODE_SWNR;
        else if (iso > thr_mnr)
            nrType = MTK_NR_MODE_MNR;
        else
            nrType = MTK_NR_MODE_OFF;
    }

    MY_LOGD("[%s] nrType = %d", __FUNCTION__, nrType);
    return nrType;
}

static MBOOL formatConverter(
        IImageBuffer *imgSrc,
        IImageBuffer *imgDst)
{
    MY_LOGD("%s +", __FUNCTION__);
    MBOOL ret = MFALSE;

    ISImager *pISImager = NULL;
    pISImager = ISImager::createInstance(imgSrc);
    if (!pISImager) {
        MY_LOGE("ISImager::createInstance() failed!!!");
        goto lbExit;
    }

    if (!pISImager->setTargetImgBuffer(imgDst)) {
        MY_LOGE("setTargetImgBuffer failed!!!");
        goto lbExit;
    }

    if (!pISImager->execute()) {
        MY_LOGE("execute failed!!!");
        goto lbExit;
    }

    ret = MTRUE;

lbExit:
    if (pISImager) {
        pISImager->destroyInstance();
    }

    MY_LOGD("%s -", __FUNCTION__);
    return ret;
}

MERROR MFCNodeImp::generateOutputYuv(
            int             sensorId,
            IImageBuffer    *imgSrc,
            IImageBuffer    *imgDst1,
            IImageBuffer    *imgDst2,
            const MRect     &crop1,
            const MRect     &crop2,
            int             orientation1,
            IMetadata       *metaApp,
            IMetadata       *metaHal,
            int             isMfll /* = 0 */
            )
{
    MY_LOGD("%s +", __FUNCTION__);
    MERROR err = UNKNOWN_ERROR;
    MINT32 iso = getIso(metaHal);
    MINT32 nrType = nrStrategy(iso, metaHal);
    sp<IImageBuffer> imgTemp;

    /* For normal cas and SWNR */
    if (nrType == MTK_NR_MODE_OFF || nrType == MTK_NR_MODE_SWNR) {
        if (nrType == MTK_NR_MODE_SWNR) {
            MY_LOGD("%s: process w/ SwNR", __FUNCTION__);

            /* to do SWNR */
            do {
                /* check source image first, SWNR only supports I420 and YV12 */
                if ((imgSrc->getImgFormat() != eImgFmt_YV12) &&
                        (imgSrc->getImgFormat() != eImgFmt_I420)) {

                    /* create temp image buffer */
                    const MSize &imgSrcSize = imgSrc->getImgSize();
                    ImageBufferUtils::getInstance().allocBuffer(
                            imgTemp,
                            imgSrcSize.w,
                            imgSrcSize.h, eImgFmt_I420);
                    if (imgTemp == NULL) {
                        MY_LOGE("image buffer is NULL");
                        break;
                    }

                    /* convert format */
                    if (!formatConverter(imgSrc, imgTemp.get())) {
                        MY_LOGE("SwNR format convert failed");
                        break;
                    }

                    imgSrc = imgTemp.get();
                }

                /* SwNR */
                ISwNR::SWNRParam swnrParam;
                swnrParam.isMfll = isMfll;
                std::unique_ptr<ISwNR> swnr = std::unique_ptr<ISwNR>(MAKE_SwNR(sensorId));

                swnrParam.iso = iso;
                MY_LOGD("Run SwNR with iso=%d", swnrParam.iso);
                CAM_TRACE_BEGIN("SWNR");
                {
                    if (!swnr->doSwNR(swnrParam, imgSrc)) {
                        MY_LOGE("SWNR failed");
                    }
                    else {
                        MY_LOGD("applied SWNR");
                    }
                }
                CAM_TRACE_END();
            } while(0);
        }

        /* create JPEG YUV and thumbnail YUV using MDP (IImageTransform) */
        std::unique_ptr<IImageTransform, std::function<void(IImageTransform*)>> t(
               IImageTransform::createInstance(),
               [](IImageTransform *p)mutable->void {
                    if (p) p->destroyInstance();
               });

        if (t.get() == NULL) {
            MY_LOGE("%s: cannot create IImageTransform instance", __FUNCTION__);
            goto lbExit;
        }

        MBOOL ret = MTRUE;
        if (imgDst2)
            ret = t->execute(imgSrc, imgDst1, imgDst2, crop1, crop2, orientation1, 0, 3000);
        else
            ret = t->execute(imgSrc, imgDst1, NULL, crop1, orientation1, 3000);

        if (ret != MTRUE) {
            MY_LOGE("%s: IImageTransform::execute failed", __FUNCTION__);
            goto lbExit;
        }

    } // nrType == 0, normal case
    /* HWNR case */
    else if (nrType == MTK_NR_MODE_MNR) {
        MY_LOGD("%s: prcoess w/ MNR", __FUNCTION__);

        QParams qParams;
        FrameParams params;
        std::unique_ptr<char> registerChunk;
        TuningParam rTuningParam;

        // create INormalStream instance
        std::unique_ptr<INormalStream, std::function<void(INormalStream*)>> ns(
                INormalStream::createInstance(sensorId),
                [](INormalStream *p)->void{ if(p) p->destroyInstance(); });

        if (ns.get() == NULL) {
            MY_LOGE("%s: create INormalStream instance failed", __FUNCTION__);
            goto lbExit;
        }

        /* check source image first, MNR only supports YUY2 */
        if (imgSrc->getImgFormat() != eImgFmt_YUY2) {
            /* create temp image buffer */
            ImageBufferUtils::getInstance().allocBuffer(
                    imgTemp,
                    imgSrc->getImgSize().w,
                    imgSrc->getImgSize().h, eImgFmt_YUY2);
            if (imgTemp == NULL) {
                MY_LOGE("image buffer is NULL");
                goto lbExit;
            }

            /* convert format */
            if (!formatConverter(imgSrc, imgTemp.get())) {
                MY_LOGE("SwNR format convert failed");
                goto lbExit;
            }

            imgSrc = imgTemp.get();
        }

        /* QParams tag */
        params.mStreamTag = ENormalStreamTag_Normal; // TODO: time sharing need using ENormalStreamTag_Vss ??

        /* retrieving register map */
        {
            /* create an unique_ptr contains IHal3A instance, and give a deleter too */
            std::unique_ptr<IHal3A, std::function<void(IHal3A*)>> aaa(
                    MAKE_Hal3A(sensorId, LOG_TAG),
                    [](IHal3A *p)->void {
                    if(p) p->destroyInstance(LOG_TAG);
                    });

            MetaSet_T metaset;

            if (aaa.get() == NULL) {
                MY_LOGE("%s: create IHal3A instance fail", __FUNCTION__);
                goto lbExit;
            }

            /* retrieving tuning data (register map) */
            registerChunk = std::unique_ptr<char>(new char[ns->getRegTableSize()]{0});
            rTuningParam.pRegBuf = registerChunk.get();
            MUINT8 restore_3a_isp_profile = 0;

            IMetadata::getEntry<MUINT8>(metaHal, MTK_3A_ISP_PROFILE, restore_3a_isp_profile);
            IMetadata::setEntry<MUINT8>(
                    metaHal, MTK_3A_ISP_PROFILE, EIspProfile_Capture_MultiPass_HWNR);

            metaset.appMeta = *metaApp;
            metaset.halMeta = *metaHal;

            if (0 > aaa->setIsp(0, metaset, &rTuningParam, NULL)) {
                MY_LOGE("%s: cannot set ISP profile and get register map", __FUNCTION__);
                goto lbExit;
            }

            /* restore ISP profile back */
            IMetadata::setEntry<MUINT8>(
                    metaHal, MTK_3A_ISP_PROFILE, restore_3a_isp_profile);

            params.mTuningData = static_cast<MVOID*>(rTuningParam.pRegBuf);
        }

        /* Input */
        {
            Input p;
            p.mPortID       = PORT_IMGI;
            p.mPortID.group = 0;
            p.mBuffer       = imgSrc;
            params.mvIn.push_back(p);
            /* cropping info */
            MCrpRsInfo crop;
            crop.mGroupID       = 1; // source cropping should be 1, only works with IMGI port
            crop.mCropRect.p_integral.x = 0; // position pixel, in integer
            crop.mCropRect.p_integral.y = 0;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = imgSrc->getImgSize().w;
            crop.mCropRect.s.h  = imgSrc->getImgSize().h;
            crop.mResizeDst.w   = imgSrc->getImgSize().w;
            crop.mResizeDst.h   = imgSrc->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);
        }

        /* Output1: YUV JPEG */
        {
            Output p;
            p.mPortID       = PORT_WROTO;
            p.mPortID.group = 0;
            p.mBuffer       = imgDst1;
            p.mTransform    = orientation1;
            params.mvOut.push_back(p);
            /* cropping info */
            MCrpRsInfo crop;
            crop.mGroupID       = 3; // WROTO always use 3
            crop.mCropRect.p_integral.x = crop1.p.x; // position pixel, in integer
            crop.mCropRect.p_integral.y = crop1.p.y;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = crop1.s.w;
            crop.mCropRect.s.h  = crop1.s.h;
            crop.mResizeDst.w   = imgDst1->getImgSize().w;
            crop.mResizeDst.h   = imgDst1->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);
        }

        /* Output2 (thumbnail) */
        if (imgDst2) {
            Output p;
            p.mPortID       = PORT_WDMAO;
            p.mPortID.group = 0;
            p.mBuffer       = imgDst2;
            params.mvOut.push_back(p);
            /* cropping info */
            MCrpRsInfo crop;
            crop.mGroupID       = 2; // WDMA always use 2
            crop.mCropRect.p_integral.x = crop2.p.x; // position pixel, in integer
            crop.mCropRect.p_integral.y = crop2.p.y;
            crop.mCropRect.p_fractional.x = 0; // always be 0
            crop.mCropRect.p_fractional.y = 0;
            crop.mCropRect.s.w  = crop2.s.w;
            crop.mCropRect.s.h  = crop2.s.h;
            crop.mResizeDst.w   = imgDst2->getImgSize().w;
            crop.mResizeDst.h   = imgDst2->getImgSize().h;
            crop.mFrameGroup    = 0;
            params.mvCropRsInfo.push_back(crop);
        }

        /* execute pass 2 driver */
        {
            Mutex mx;
            Mutex::Autolock _l(&mx);

            qParams.mpCookie = &mx;
            qParams.mpfnCallback = [](QParams& p){
                Mutex *mx = reinterpret_cast<Mutex*>(p.mpCookie);
                mx->unlock();
            };

            qParams.mvFrameParams.push_back(params);

            if (!ns->enque(qParams)) {
                MY_LOGE("%s: enque pass2 failed", __FUNCTION__);
                err = UNKNOWN_ERROR;
                goto lbExit;
            }

            // lock again, and wait unlocked, synchronizing
            mx.lock();
            mx.unlock();
        }
    } // nrType == 1, HWNR
    else {
        MY_LOGE("%s: not support NR type %d", __FUNCTION__, nrType);
        goto lbExit;
    }

    err = OK;

lbExit:
    if (imgTemp.get()) {
        ImageBufferUtils::getInstance().deallocBuffer(imgTemp);
    }

    MY_LOGD("%s: -", __FUNCTION__);
    return err;
}

const char* MFCNodeImp::getFileFormatName(const MINT &f)
{
    switch (f) {
    case eImgFmt_YUY2:
        return "yuy2";
    case eImgFmt_YV12:
        return "yv12";
    case eImgFmt_YV16:
        return "yv16";
    case eImgFmt_I420:
        return "i420";
    default:
        return "N/A";
    }
}

MBOOL MFCNodeImp::retrieveScalerCropRgn(IMetadata* pMetadata, MRect& outRgn)
{
    if (!IMetadata::getEntry<MRect>(pMetadata, MTK_SCALER_CROP_REGION, outRgn)) {
        return MFALSE;
    }
    else {
        MY_LOGD("cropRgn->(x,y,w,h)=(%d,%d,%d,%d)",
                outRgn.p.x, outRgn.p.y,
                outRgn.s.w, outRgn.s.h);
        return MTRUE;;
    }

}

MBOOL MFCNodeImp::retrieveGmvInfo(IMetadata *pMetadata, int &x, int &y, MSize &size)
{
    MBOOL  ret = MTRUE;
    MSize  rzoSize;
    IMetadata::IEntry entry;
    struct __confidence{
        MINT32 x;
        MINT32 y;
        __confidence() : x(0), y(0) {}
    } confidence;

    /* get size first */
    ret = IMetadata::getEntry(pMetadata, MTK_P1NODE_RESIZER_SIZE, rzoSize);
    if (ret != MTRUE) {
        MY_LOGE("%s: cannot get rzo size", __FUNCTION__);
        goto lbExit;
    }

    entry = pMetadata->entryFor(MTK_EIS_REGION);

    /* check if a valid EIS_REGION */
    if (entry.count() < EIS_REGION_INDEX_SIZE) {
        MY_LOGE("%s: entry is not a valid EIS_REGION, size = %d",
                __FUNCTION__,
                entry.count());
        ret = MFALSE;
        goto lbExit;
    }

    /* read confidence */
    confidence.x = static_cast<MINT32>(entry.itemAt(EIS_REGION_INDEX_CONFX, Type2Type<MINT32>()));
    confidence.y = static_cast<MINT32>((MINT32)entry.itemAt(EIS_REGION_INDEX_CONFY, Type2Type<MINT32>()));

    /* to read GMV if confidence is enough */
    if (confidence.x > MFC_GMV_CONFX_TH) {
    x = entry.itemAt(EIS_REGION_INDEX_GMVX, Type2Type<MINT32>());
    }

    if (confidence.y > MFC_GMV_CONFY_TH) {
    y = entry.itemAt(EIS_REGION_INDEX_GMVY, Type2Type<MINT32>());
    }

    size = rzoSize;

    MY_LOGD("EIS info conf(x,y) = (%d, %d), gmv(x, y) = (%d, %d)",
            confidence.x, confidence.y, x, y);

lbExit:
    return ret;
}

MRect MFCNodeImp::calCrop(MRect const &rSrc, MRect const &rDst)
{
    #define ROUND_TO_2X(x) ((x) & (~0x1))
    MRect rCrop;

    // srcW/srcH < dstW/dstH
    if (rSrc.s.w * rDst.s.h < rDst.s.w * rSrc.s.h) {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.w * rDst.s.h / rDst.s.w;
    }
    // srcW/srcH > dstW/dstH
    else if (rSrc.s.w * rDst.s.h > rDst.s.w * rSrc.s.h) {
        rCrop.s.w = rSrc.s.h * rDst.s.w / rDst.s.h;
        rCrop.s.h = rSrc.s.h;
    }
    // srcW/srcH == dstW/dstH
    else {
        rCrop.s.w = rSrc.s.w;
        rCrop.s.h = rSrc.s.h;
    }

    rCrop.s.w =  ROUND_TO_2X(rCrop.s.w);
    rCrop.s.h =  ROUND_TO_2X(rCrop.s.h);

    rCrop.p.x = (rSrc.s.w - rCrop.s.w) / 2;
    rCrop.p.y = (rSrc.s.h - rCrop.s.h) / 2;

    rCrop.p.x += ROUND_TO_2X(rSrc.p.x);
    rCrop.p.y += ROUND_TO_2X(rSrc.p.y);

    #undef ROUND_TO_2X
    return rCrop;
}
