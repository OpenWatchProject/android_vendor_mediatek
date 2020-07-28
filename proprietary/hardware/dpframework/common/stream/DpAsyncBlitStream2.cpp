#include "DpAsyncBlitStream2.h"
#include "DpTileScaler.h"
#include "DpMutex.h"
#include "DpLogger.h"
#include <sync.h>

#define PQ_ENHANCE_FULL_FRAME      1


bool DpAsyncBlitStream2::queryHWSupport(uint32_t         srcWidth,
                                        uint32_t         srcHeight,
                                        uint32_t         dstWidth,
                                        uint32_t         dstHeight,
                                        int32_t          Orientation,
                                        DpColorFormat    srcFormat,
                                        DpColorFormat    dstFormat,
                                        DpPqParam        *PqParam,
                                        DpRect           *srcCrop)
{
    return DpAsyncBlitStream::queryHWSupport(srcWidth,
                                             srcHeight,
                                             dstWidth,
                                             dstHeight,
                                             Orientation,
                                             srcFormat,
                                             dstFormat,
                                             PqParam,
                                             srcCrop);
}

int32_t DpAsyncBlitStream2::queryPaddingSide(uint32_t transform)
{
    return DpAsyncBlitStream::queryPaddingSide(transform);
}

DP_STATUS_ENUM DpAsyncBlitStream2::queryTheoreticalExecTime(uint32_t    srcWidth,
                                       uint32_t          srcHeight,
                                       uint32_t          dstWidth,
                                       uint32_t          dstHeight,
                                       std::vector<uint32_t >* pExecTimeList)
{
    return DpAsyncBlitStream::queryTheoreticalExecTime(srcWidth,
                                       srcHeight,
                                       dstWidth,
                                       dstHeight,
                                       pExecTimeList);
}


DpAsyncBlitStream2::BaseConfig::BaseConfig()
{
    int32_t index;

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        rotation[index] = 0;
        flip[index] = false;
        dstFormat[index] = DP_COLOR_RGB888;
        dstEnabled[index] = false;
        cropEnabled[index] = false;
    }
}


DpAsyncBlitStream2::DpAsyncBlitStream2()
    : m_pJobMutex(new DpMutex()),
      m_pCurJob(NULL)
{
}


DpAsyncBlitStream2::~DpAsyncBlitStream2()
{
    delete m_pJobMutex;
    m_pJobMutex = NULL;
}


DP_STATUS_ENUM DpAsyncBlitStream2::createJob(uint32_t &jobID, int32_t &fence)
{
    DP_STATUS_ENUM status;
    struct AsyncBlitJobPair *pJob;
    int32_t fenceFD, fenceFD2;

    pJob = new struct AsyncBlitJobPair;
    if (pJob == NULL)
    {
        DPLOGE("DpAsyncBlitStream2: cannot allocate job\n");
        return DP_STATUS_OUT_OF_MEMORY;
    }

    status = m_stream.createJob(pJob->jobID, fenceFD);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        delete pJob;
        return status;
    }
    status = m_stream2.createJob(pJob->jobID2, fenceFD2);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        m_stream.cancelJob(pJob->jobID);
        delete pJob;
        return status;
    }
    pJob->fenceFD = sync_merge("AsyncBlit2", fenceFD, fenceFD2);
    close(fenceFD);
    close(fenceFD2);

    if (pJob->fenceFD < 0)
    {
        m_stream.cancelJob(pJob->jobID);
        m_stream2.cancelJob(pJob->jobID2);
        delete pJob;
        return DP_STATUS_OPERATION_FAILED;
    }

    jobID = pJob->jobID;
    fence = pJob->fenceFD;

    AutoMutex lock(m_pJobMutex);
    m_newJobList.push_back(pJob);

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream2::cancelJob(uint32_t jobID)
{
    AutoMutex lock(m_pJobMutex);

    DP_STATUS_ENUM    status;
    JobList::iterator iterator;
    uint32_t          jobID2;

    if (jobID)
    {
        for (iterator = m_newJobList.begin(); iterator != m_newJobList.end(); iterator++)
        {
            if (jobID == (*iterator)->jobID)
            {
                break;
            }
        }

        if (iterator == m_newJobList.end())
        {
            DPLOGE("DpAsyncBlitStream2: cannot find job with ID %d\n", jobID);
            return DP_STATUS_INVALID_PARAX;
        }

        jobID2 = (*iterator)->jobID2;
        iterator++;
    }
    else
    {
        jobID2 = jobID;
        iterator = m_newJobList.end();
    }

    status = m_stream.cancelJob(jobID);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }
    status = m_stream2.cancelJob(jobID2);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    for (JobList::iterator it = m_newJobList.begin(); it != iterator; it++)
    {
        delete *it;
    }
    m_newJobList.erase(m_newJobList.begin(), iterator);

    return DP_STATUS_RETURN_SUCCESS;
}


void DpAsyncBlitStream2::transToRotFlip(uint32_t transform, int32_t &rotation, bool &flip)
{
    uint32_t f = 0;
    uint32_t rot = 0;

    // operate on FLIP_H, FLIP_V and ROT_90 respectively
    // to achieve the final orientation
    if (DpAsyncBlitStream::FLIP_H & transform)
    {
        f ^= 1;
    }

    if (DpAsyncBlitStream::FLIP_V & transform)
    {
        // FLIP_V is equivalent to a 180-degree rotation with a horizontal flip
        rot += 180;
        f ^= 1;
    }

    if (DpAsyncBlitStream::ROT_90 & transform)
    {
        rot += 90;
    }

    flip = (0 != f) ? true: false;
    rotation = rot %= 360;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setConfigBegin(uint32_t jobID,
                                                  int32_t  enhancePos,
                                                  int32_t  enhanceDir)
{
    AutoMutex lock(m_pJobMutex);

    DP_STATUS_ENUM status;
    struct AsyncBlitJobPair *pJob = NULL;
    bool enSplit = false;
    JobList::iterator iterator;

    for (iterator = m_newJobList.begin(); iterator != m_newJobList.end(); iterator++)
    {
        if (jobID == (*iterator)->jobID)
        {
            pJob = *iterator;
            break;
        }
    }

    if (pJob == NULL)
    {
        DPLOGE("DpAsyncBlitStream2: cannot find job with ID %d\n", jobID);
        return DP_STATUS_INVALID_PARAX;
    }

    status = m_stream.setConfigBegin(jobID);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }
    enSplit = (0 < enhancePos && enhancePos < PQ_ENHANCE_STEPS);
    if (enSplit)
    {
        status = m_stream2.setConfigBegin(pJob->jobID2);
    }
    else
    {
        status = m_stream2.cancelJob(pJob->jobID2);
    }
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    for (JobList::iterator it = m_newJobList.begin(); it != iterator; it++)
    {
        delete *it;
    }
    m_newJobList.erase(m_newJobList.begin(), ++iterator);

    if (m_pCurJob)
    {
        delete m_pCurJob;
    }

    m_pCurJob = pJob;
    m_pCurJob->enhancePos = enhancePos;
    transToRotFlip(enhanceDir, m_pCurJob->enhanceRot, m_pCurJob->enhanceFlip);
    m_pCurJob->enhanceSplit = enSplit;

    return DP_STATUS_RETURN_SUCCESS;
}


inline void DpAsyncBlitStream2::calculateCropROI(struct CropROI &crs, struct CropROI &crs2,
            int32_t rotation, DpColorFormat dstFormat)
{
    int src_dir = (rotation + m_pCurJob->enhanceRot) / 90 % 4;
    bool sh2w = (1 == src_dir || 3 == src_dir);
    bool sw2o = (1 == src_dir || 2 == src_dir);
    bool dh2w = (90 == m_pCurJob->enhanceRot || 270 == m_pCurJob->enhanceRot);
    bool dw2o = (90 == m_pCurJob->enhanceRot || 180 == m_pCurJob->enhanceRot);

    _calcCropROI(crs, crs2,
                 dstFormat, sh2w, sw2o, dh2w, dw2o);
}


inline void DpAsyncBlitStream2::calculateCropROI2(struct CropROI &crs, struct CropROI &crs2,
            int32_t rotation, DpColorFormat dstFormat)
{
    bool sh2w = false;
    bool sw2o = false;
    bool dh2w = ( 90 == rotation || 270 == rotation);
    bool dw2o = (180 == rotation || 270 == rotation);

    _calcCropROI(crs, crs2,
                 dstFormat, sh2w, sw2o, dh2w, dw2o);
}


void DpAsyncBlitStream2::_calcCropROI(struct CropROI &crs, struct CropROI &crs2,
            DpColorFormat dstFormat, bool sh2w, bool sw2o, bool dh2w, bool dw2o)
{
    DpRect &crop = crs.crop, &roi = crs.roi;
    DpRect &crop2 = crs2.crop, &roi2 = crs2.roi;
    uint32_t align_mask;
    int32_t min_w, dst_len, src_len, dst_w, dst_w2;

    if (sh2w)
    {
        crop = DpRect(crop.y, crop.x, crop.h, crop.w,
                      crop.sub_y, crop.sub_x, crop.sub_h, crop.sub_w);
    }
    if (dh2w)
    {
        roi = DpRect(roi.y, roi.x, roi.h, roi.w);
        min_w = 1 << max(DP_COLOR_GET_H_SUBSAMPLE(dstFormat) /* 90, 270 degrees & YUV422 */,
                         DP_COLOR_GET_V_SUBSAMPLE(dstFormat));
        dst_w = crs.height;
    }
    else
    {
        min_w = 1 << DP_COLOR_GET_H_SUBSAMPLE(dstFormat);
        dst_w = crs.width;
    }
    align_mask = ~(min_w - 1);
    dst_len = roi.w;
    src_len = crop.w;

    if (sw2o)
    {
        roi2.w = ((dst_len * (PQ_ENHANCE_STEPS - m_pCurJob->enhancePos) / PQ_ENHANCE_STEPS) + ~align_mask) & align_mask;
        roi2.w = min(max(roi2.w, min_w), dst_len - min_w);
        roi.w = dst_len - roi2.w;
        dst_w = dst_w - roi2.w;
        dst_w2 = roi2.w;
    }
    else
    {
        roi.w = (dst_len * m_pCurJob->enhancePos / PQ_ENHANCE_STEPS) & align_mask;
        roi.w = min(max(roi.w, min_w), dst_len - min_w);
        roi2.w = dst_len - roi.w;
        dst_w2 = dst_w - roi.w;
        dst_w = roi.w;
    }
    if (dw2o)
    {
        roi2.x = roi.x;
        roi.x = roi2.x + dst_w2;
    }
    else
    {
        roi2.x = roi.x + dst_w;
    }
    roi2.y = roi.y;
    roi2.h = roi.h;

    crop.w = src_len * roi.w / dst_len;
    crop.sub_w = (int32_t)((((int64_t)src_len * roi.w % dst_len) << TILE_SCALER_SUBPIXEL_SHIFT) / dst_len);
#if !PQ_ENHANCE_FULL_FRAME
    if (crop.w <= 0)
    {
        crop.w = 1;
        crop.sub_w = 0;
    }
#endif
    if (crop.w >= src_len - 1)
    {
        crop.w = src_len - 1;
        crop.sub_w = 0;
    }
    crop2.w = src_len - crop.w;
    if (crop.sub_w)
    {
        crop2.w -= 1;
        crop2.sub_w = (1 << TILE_SCALER_SUBPIXEL_SHIFT) - crop.sub_w;
    }
    if (sw2o)
    {
        crop2.x = crop.x;
        crop2.sub_x = crop.sub_x;
        crop.x = crop2.x + crop2.w;
        crop.sub_x = crop2.sub_x + crop2.sub_w;
        if (crop.sub_x >= (1 << TILE_SCALER_SUBPIXEL_SHIFT))
        {
            crop.x += 1;
            crop.sub_x -= (1 << TILE_SCALER_SUBPIXEL_SHIFT);
        }
    }
    else
    {
        crop2.x = crop.x + crop.w;
        crop2.sub_x = crop.sub_x + crop.sub_w;
        if (crop2.sub_x >= (1 << TILE_SCALER_SUBPIXEL_SHIFT))
        {
            crop2.x += 1;
            crop2.sub_x -= (1 << TILE_SCALER_SUBPIXEL_SHIFT);
        }
    }
    crop2.y = crop.y;
    crop2.sub_y = crop.sub_y;
    crop2.h = crop.h;
    crop2.sub_h = crop.sub_h;

    if (sh2w)
    {
        crop = DpRect(crop.y, crop.x, crop.h, crop.w,
                      crop.sub_y, crop.sub_x, crop.sub_h, crop.sub_w);
        crop2 = DpRect(crop2.y, crop2.x, crop2.h, crop2.w,
                       crop2.sub_y, crop2.sub_x, crop2.sub_h, crop2.sub_w);
    }
    if (dh2w)
    {
        roi = DpRect(roi.y, roi.x, roi.h, roi.w);
        roi2 = DpRect(roi2.y, roi2.x, roi2.h, roi2.w);
        crs.height = dst_w;
        crs2.height = dst_w2;
        crs2.width = crs.width;
    }
    else
    {
        crs.width = dst_w;
        crs2.width = dst_w2;
        crs2.height = crs.height;
    }
}


DP_STATUS_ENUM DpAsyncBlitStream2::refreshCropROI(int32_t index)
{
    DP_STATUS_ENUM status;
    const BaseConfig &config = m_pCurJob->config;
    struct CropROI crs, crs2;

    if (!config.dstEnabled[index])
        return DP_STATUS_RETURN_SUCCESS;

    crs.crop = config.cropEnabled[index] ? config.crop[index] : config.src;
    crs.roi = config.roi[index];
    crs.width = config.dstWidth[index];
    crs.height = config.dstHeight[index];
    DPLOGI("DpAsyncBlit2: pos %d rot %d out%d crop:(%d, %d, %d, %d, %d, %d, %d, %d) roi:(%d %d %d %d) size:(%d %d)\n",
        m_pCurJob->enhancePos, m_pCurJob->enhanceRot,
        index, crs.crop.x, crs.crop.y, crs.crop.w, crs.crop.h, crs.crop.sub_x, crs.crop.sub_y, crs.crop.sub_w, crs.crop.sub_h,
        crs.roi.x, crs.roi.y, crs.roi.w, crs.roi.h, crs.width, crs.height);
#if 0
    static int32_t s_enhancePos = 0;
    static int32_t s_enhanceRot = 0;
    static int32_t s_enhanceStep = 1;

    s_enhancePos += s_enhanceStep;
    if (0 >= s_enhancePos || s_enhancePos >= PQ_ENHANCE_STEPS)
    {
        s_enhanceStep = -s_enhanceStep;
        s_enhancePos += s_enhanceStep * 2;
        s_enhanceRot = (s_enhanceRot + 90) % 360;
    }
    m_pCurJob->enhancePos = s_enhancePos;
    m_pCurJob->enhanceRot = s_enhanceRot;
#endif
    calculateCropROI(crs, crs2,
                     config.rotation[index], config.dstFormat[index]);
    DPLOGI("DpAsyncBlit2: out%d pq crop:(%d, %d, %d, %d, %d, %d, %d, %d) roi:(%d %d %d %d) size:(%d %d)\n",
        index, crs.crop.x, crs.crop.y, crs.crop.w, crs.crop.h, crs.crop.sub_x, crs.crop.sub_y, crs.crop.sub_w, crs.crop.sub_h,
        crs.roi.x, crs.roi.y, crs.roi.w, crs.roi.h, crs.width, crs.height);
    DPLOGI("DpAsyncBlit2: out%d no crop:(%d, %d, %d, %d, %d, %d, %d, %d) roi:(%d %d %d %d) size:(%d %d)\n",
        index, crs2.crop.x, crs2.crop.y, crs2.crop.w, crs2.crop.h, crs2.crop.sub_x, crs2.crop.sub_y, crs2.crop.sub_w, crs2.crop.sub_h,
        crs2.roi.x, crs2.roi.y, crs2.roi.w, crs2.roi.h, crs2.width, crs2.height);

#if !PQ_ENHANCE_FULL_FRAME
    status = m_stream.setSrcCrop(index, crs.crop);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;
    status = m_stream.setDstROI(index, crs.width, crs.height, crs.roi);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;
#endif
    status = m_stream2.setSrcCrop(index, crs2.crop);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;
    status = m_stream2.setDstROI(index, crs2.width, crs2.height, crs2.roi);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream2::refreshCropROIs()
{
    DP_STATUS_ENUM status;
    int32_t index;

    for (index = 0; index < ISP_MAX_OUTPUT_PORT_NUM; index++)
    {
        status = refreshCropROI(index);
        if (DP_STATUS_RETURN_SUCCESS != status)
            return status;
    }
    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setSrcBuffer(void     *pVABase,
                                                uint32_t size,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setSrcBuffer(pVABase, size, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setSrcBuffer(pVABase, size);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setSrcBuffer(void     **pVAList,
                                                uint32_t *pSizeList,
                                                uint32_t planeNumber,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setSrcBuffer(pVAList, pSizeList, planeNumber, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setSrcBuffer(pVAList, pSizeList, planeNumber);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setSrcBuffer(void**   pVAddrList,
                                                void**   pMVAddrList,
                                                uint32_t *pSizeList,
                                                uint32_t planeNumber,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setSrcBuffer(pVAddrList, pMVAddrList, pSizeList, planeNumber, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setSrcBuffer(pVAddrList, pMVAddrList, pSizeList, planeNumber);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setSrcBuffer(int32_t  fileDesc,
                                                uint32_t *sizeList,
                                                uint32_t planeNumber,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setSrcBuffer(fileDesc, sizeList, planeNumber, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setSrcBuffer(fileDesc, sizeList, planeNumber);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setSrcConfig(int32_t           width,
                                                int32_t           height,
                                                DpColorFormat     format,
                                                DpInterlaceFormat field)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setSrcConfig(width, height, format, field);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    status = m_stream2.setSrcConfig(width, height, format, field);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.src = DpRect(0, 0, width, height);

    status = refreshCropROIs();
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setSrcConfig(int32_t           width,
                                                int32_t           height,
                                                int32_t           yPitch,
                                                int32_t           uvPitch,
                                                DpColorFormat     format,
                                                DP_PROFILE_ENUM   profile,
                                                DpInterlaceFormat field,
                                                DpSecure          secure,
                                                bool              doFlush)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setSrcConfig(width, height, yPitch, uvPitch, format, profile, field, secure, doFlush);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    status = m_stream2.setSrcConfig(width, height, yPitch, uvPitch, format, profile, field, secure, doFlush);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.src = DpRect(0, 0, width, height);

    status = refreshCropROIs();
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setSrcCrop(int32_t portIndex, DpRect  roi)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setSrcCrop(portIndex, roi);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.crop[portIndex] = roi;
    m_pCurJob->config.cropEnabled[portIndex] = true;

    status = refreshCropROI(portIndex);
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDstBuffer(int32_t  portIndex,
                                                void     *pVABase,
                                                uint32_t size,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setDstBuffer(portIndex, pVABase, size, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setDstBuffer(portIndex, pVABase, size);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDstBuffer(int32_t  portIndex,
                                                void     **pVABaseList,
                                                uint32_t *pSizeList,
                                                uint32_t planeNumber,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setDstBuffer(portIndex, pVABaseList, pSizeList, planeNumber, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setDstBuffer(portIndex, pVABaseList, pSizeList, planeNumber);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDstBuffer(int32_t  portIndex,
                                                void**   pVABaseList,
                                                void**   pMVABaseList,
                                                uint32_t *pSizeList,
                                                uint32_t planeNumber,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setDstBuffer(portIndex, pVABaseList, pMVABaseList, pSizeList, planeNumber, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setDstBuffer(portIndex, pVABaseList, pMVABaseList, pSizeList, planeNumber);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDstBuffer(int32_t  portIndex,
                                                int32_t  fileDesc,
                                                uint32_t *pSizeList,
                                                uint32_t planeNumber,
                                                int32_t  fenceFd)
{
    DP_STATUS_ENUM status;

    status = m_stream.setDstBuffer(portIndex, fileDesc, pSizeList, planeNumber, fenceFd);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setDstBuffer(portIndex, fileDesc, pSizeList, planeNumber);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDstConfig(int32_t           portIndex,
                                                int32_t           width,
                                                int32_t           height,
                                                DpColorFormat     format,
                                                DpInterlaceFormat field,
                                                DpRect            *pROI)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setDstConfig(portIndex, width, height, format, field, pROI);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    status = m_stream2.setDstConfig(portIndex, width, height, format, field, NULL);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.roi[portIndex] = pROI ? *pROI : DpRect(0, 0, width, height);
    m_pCurJob->config.dstWidth[portIndex] = width;
    m_pCurJob->config.dstHeight[portIndex] = height;
    m_pCurJob->config.dstFormat[portIndex] = format;
    m_pCurJob->config.dstEnabled[portIndex] = true;

    status = refreshCropROI(portIndex);
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDstConfig(int32_t           portIndex,
                                                int32_t           width,
                                                int32_t           height,
                                                int32_t           yPitch,
                                                int32_t           uvPitch,
                                                DpColorFormat     format,
                                                DP_PROFILE_ENUM   profile,
                                                DpInterlaceFormat field,
                                                DpRect            *pROI,
                                                DpSecure          secure,
                                                bool              doFlush)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setDstConfig(portIndex, width, height, yPitch, uvPitch, format, profile, field, pROI, secure, doFlush);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    status = m_stream2.setDstConfig(portIndex, width, height, yPitch, uvPitch, format, profile, field, NULL, secure, doFlush);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.roi[portIndex] = pROI ? *pROI : DpRect(0, 0, width, height);
    m_pCurJob->config.dstWidth[portIndex] = width;
    m_pCurJob->config.dstHeight[portIndex] = height;
    m_pCurJob->config.dstFormat[portIndex] = format;
    m_pCurJob->config.dstEnabled[portIndex] = true;

    status = refreshCropROI(portIndex);
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDstROI(int32_t      portIndex,
                                             int32_t      width,
                                             int32_t      height,
                                             const DpRect &roi)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setDstROI(portIndex, width, height, roi);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.roi[portIndex] = roi;
    m_pCurJob->config.dstWidth[portIndex] = width;
    m_pCurJob->config.dstHeight[portIndex] = height;

    status = refreshCropROI(portIndex);
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setRotate(int32_t portIndex, int32_t rotation)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setRotate(portIndex, rotation);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    status = m_stream2.setRotate(portIndex, rotation);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.rotation[portIndex] = rotation;

    status = refreshCropROI(portIndex);
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setFlip(int32_t portIndex, int flip)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setFlip(portIndex, flip);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    status = m_stream2.setFlip(portIndex, flip);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    backup = m_pCurJob->config;
    m_pCurJob->config.flip[portIndex] = flip ? true : false;

    status = refreshCropROI(portIndex);
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setOrientation(int32_t  portIndex,
                                                  uint32_t transform)
{
    DP_STATUS_ENUM status;
    BaseConfig backup;

    status = m_stream.setOrientation(portIndex, transform);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    status = m_stream2.setOrientation(portIndex, transform);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;

    backup = m_pCurJob->config;
    transToRotFlip(transform, m_pCurJob->config.rotation[portIndex], m_pCurJob->config.flip[portIndex]);

    status = refreshCropROI(portIndex);
    if (DP_STATUS_RETURN_SUCCESS != status)
        m_pCurJob->config = backup;
    return status;
}


DP_STATUS_ENUM DpAsyncBlitStream2::setPQParameter(int32_t portIndex,
                                                  const DpPqParam &pqParam)
{
#if 0
    {
        AutoMutex lock(m_pJobMutex);
        if (m_pCurJob == NULL)
            return DP_STATUS_INVALID_STATE;
    }
    if (m_pCurJob->enhancePos <= 0)
        return DP_STATUS_RETURN_SUCCESS;
#endif
    return m_stream.setPQParameter(portIndex, pqParam);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setDither(int32_t portIndex,
                                             bool    enDither)
{
    DP_STATUS_ENUM status;

    status = m_stream.setDither(portIndex, enDither);
    if (DP_STATUS_RETURN_SUCCESS != status || !m_pCurJob->enhanceSplit)
        return status;
    return m_stream2.setDither(portIndex, enDither);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setUser(uint32_t eID)
{
    DP_STATUS_ENUM status;

    status = m_stream.setUser(eID);
    if (DP_STATUS_RETURN_SUCCESS != status)
        return status;
    return m_stream2.setUser(eID);
}


DP_STATUS_ENUM DpAsyncBlitStream2::setConfigEnd()
{
    DP_STATUS_ENUM status;
    AutoMutex lock(m_pJobMutex);

    if (m_pCurJob == NULL)
    {
        DPLOGE("DpAsyncBlitStream2: setConfigEnd called before setConfigBegin\n");
        return DP_STATUS_INVALID_STATE;
    }

    status = m_stream.setConfigEnd();
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }
    if (m_pCurJob->enhanceSplit)
    {
        status = m_stream2.setConfigEnd();
    }
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    // current job done
    m_jobList.push_back(m_pCurJob);
    m_pCurJob = NULL;

    return DP_STATUS_RETURN_SUCCESS;
}


DP_STATUS_ENUM DpAsyncBlitStream2::invalidate(struct timeval *endTime)
{
    DP_STATUS_ENUM status;
    struct AsyncBlitJobPair *pJob;

    {
        AutoMutex lock(m_pJobMutex);

        if (m_jobList.empty())
        {
            DPLOGE("DpAsyncBlitStream2: no job to invalidate!\n");
            return DP_STATUS_INVALID_STATE;
        }

        pJob = m_jobList.front();
    }

    status = m_stream.invalidate(endTime);
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }
    if (pJob->enhanceSplit)
    {
        status = m_stream2.invalidate(endTime);
    }
    if (DP_STATUS_RETURN_SUCCESS != status)
    {
        return status;
    }

    {
        AutoMutex lock(m_pJobMutex);

        m_jobList.erase(m_jobList.begin());

        delete pJob;
    }

    return DP_STATUS_RETURN_SUCCESS;
}

