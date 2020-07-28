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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#include <iostream>

#include "JpegCodec.h"
#include "mhal/MediaHal.h"
#include "enc/jpeg_hal.h"
#include <utils/Log.h>

using namespace jpeg_encoder;

#define LOG_TAG "JpegCodec"

JpegCodec::JpegCodec() {
    // m_pDebugHelper = new DebugHelper();
}

JpegCodec::~JpegCodec() {
    delete m_pDebugHelper;
}

bool JpegCodec::yv12ToJpg(ImageBuf* pYv12Buf, MUINT8 *pDstBuf, MUINT32 *pDstBufSize) {
    bool result = false;
    MINT32 isEnableSOI = 1;  // if set true, not need add exif
    size_t yuvAddr[3], yuvSize[3];

    MINT32 srcWidth = pYv12Buf->width;
    MINT32 srcHeight = pYv12Buf->height;
    size_t pSrcbuffer = (size_t)pYv12Buf->buffer;
    MINT32 dstSize = srcWidth * srcHeight;

    if (pSrcbuffer == NULL) {
        ALOGD("<yv12ToJpg> null yuv buffer, fail!!!");
        return false;
    }

    ALOGD("<yv12ToJpg>begin,pSrcbuffer:%p,srcWidth:%d,srcHeight:%d",
            pSrcbuffer, srcWidth, srcHeight);

    yuvSize[0] = srcWidth * srcHeight;
    yuvSize[1] = (srcWidth/2) * (srcHeight/2);
    yuvSize[2] = (srcWidth/2) * (srcHeight/2);

    yuvAddr[0] = pSrcbuffer;
    yuvAddr[1] = yuvAddr[0]+yuvSize[0];
    yuvAddr[2] = yuvAddr[1]+yuvSize[1];

    JpgEncHal* pJpgEncoder = new JpgEncHal();
    pJpgEncoder->unlock();
    if (!pJpgEncoder->lock()) {
        ALOGI("<yv12ToJpg><ERROR> can't lock jpeg resource!!!");
        delete pJpgEncoder;
        return false;
    }

    // pDstBuf = (MUINT8*)malloc(srcWidth * srcHeight);
    pJpgEncoder->setEncSize(srcWidth, srcHeight, JpgEncHal::kENC_YV12_Format);
    pJpgEncoder->setSrcAddr((void*)ALIGN16(yuvAddr[0]), (void*)ALIGN16(yuvAddr[1]), (void*)ALIGN16(yuvAddr[2]));
    pJpgEncoder->setSrcBufSize(srcWidth, yuvSize[0], yuvSize[1], yuvSize[2]);
    // set encoding quality , range should be [100:1]
    pJpgEncoder->setQuality(JPG_ENCODE_QUALITY);
    pJpgEncoder->setDstAddr((void *)pDstBuf);
    pJpgEncoder->setDstSize(dstSize);
    pJpgEncoder->enableSOI(isEnableSOI);
    pJpgEncoder->setSrcFD(-1, -1);
    pJpgEncoder->setDstFD(-1);

    if (pJpgEncoder->start(pDstBufSize)) {
        result = true;
    } else {
        ALOGI("<yv12ToJpg><ERROR> encode fail");
    }
    pJpgEncoder->unlock();
    delete pJpgEncoder;
    ALOGD("<yv12ToJpg>end result:%d, encode size:%d", result, *pDstBufSize);
    return result;
}

bool JpegCodec::yv12ToJpg(IonInfo* pYv12Buf, MUINT32 srcWidth, MUINT32 srcHeight,
        IonInfo *pDstBuf, MUINT32 dstSize, MUINT32 *pDstBufSize) {
    bool result = false;
    MINT32 isEnableSOI = 1;  // if set true, not need add exif
    MINT32 quality = 80;
    size_t yuvAddr[3], yuvSize[3];
    ALOGD("<yv12ToJpg> pYv12Buf:%p, srcWidth:%d, srcHeight:%d", pYv12Buf, srcWidth, srcHeight);

    yuvSize[0] = srcWidth * srcHeight;
    yuvSize[1] = (srcWidth/2) * (srcHeight/2);
    yuvSize[2] = (srcWidth/2) * (srcHeight/2);

    size_t pSrcbuffer = (size_t)pYv12Buf->virAddr;
    yuvAddr[0] = pSrcbuffer;
    yuvAddr[1] = yuvAddr[0]+yuvSize[0];
    yuvAddr[2] = yuvAddr[1]+yuvSize[1];

    JpgEncHal* pJpgEncoder = new JpgEncHal();
    pJpgEncoder->unlock();
    if (!pJpgEncoder->lock()) {
        ALOGI("<yv12ToJpg><ERROR> can't lock jpeg resource!!!");
        delete pJpgEncoder;
        return false;
    }

    if (NULL != m_pDebugHelper)
    {
       m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yv12ToJpg_in.yuv", (uint8_t*)pYv12Buf->virAddr,
               srcWidth * srcHeight * 3 /2);
    }

    pJpgEncoder->setEncSize(srcWidth, srcHeight, JpgEncHal::kENC_YV12_Format);
    pJpgEncoder->setSrcAddr((void*)yuvAddr[0], (void*)yuvAddr[1], (void*)yuvAddr[2]);
    pJpgEncoder->setSrcBufSize(srcWidth, yuvSize[0], yuvSize[1], yuvSize[2]);
    // set encoding quality , range should be [100:1]
    pJpgEncoder->setQuality(quality);
    pJpgEncoder->setDstAddr((void *)pDstBuf->virAddr);
    pJpgEncoder->setDstSize(dstSize);
    pJpgEncoder->enableSOI(isEnableSOI);
    pJpgEncoder->setIonMode(true);
    pJpgEncoder->setSrcFD(pYv12Buf->ionFd, pYv12Buf->ionFd);
    pJpgEncoder->setDstFD(pDstBuf->ionFd);

    if (pJpgEncoder->start(pDstBufSize)) {
        result = true;
    } else {
        ALOGI("<yv12ToJpg><ERROR> encode fail");
    }
    pJpgEncoder->unlock();

    delete pJpgEncoder;
    ALOGD("<yv12ToJpg>end result:%d, encode size:%d", result, *pDstBufSize);
    if (NULL != m_pDebugHelper)
    {
       m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yv12ToJpg_out.jpg", (uint8_t*)pDstBuf->virAddr,
               *pDstBufSize);
    }
    return result;
}

// use M4U buffer to decode
bool JpegCodec::jpgToYv12(MUINT8 *pJpgBuf, MUINT32 jpgBufSize, ImageBuf* pOutBuf, MUINT32 sampleSize) {
    MHAL_JPEG_DEC_INFO_OUT outInfo;
    MHAL_JPEG_DEC_START_IN inParams;
    MHAL_JPEG_DEC_SRC_IN srcInfo;

    ALOGD("<jpgToYV12> start pJpgBuf:%p,bufSize:%d ", pJpgBuf, jpgBufSize);

    srcInfo.jpgDecHandle = NULL;
    srcInfo.srcBuffer = pJpgBuf;
    srcInfo.srcLength = jpgBufSize;

    if (MHAL_NO_ERROR != mHalJpeg(MHAL_IOCTL_JPEG_DEC_PARSER, (void *)&srcInfo, sizeof(srcInfo),
            NULL, 0, NULL)) {
        ALOGD("<jpgToYV12><ERROR> parser file error!!!");
        return false;
    }

    outInfo.jpgDecHandle = srcInfo.jpgDecHandle;
    if (MHAL_NO_ERROR != mHalJpeg(MHAL_IOCTL_JPEG_DEC_GET_INFO, NULL, 0, (void *)&outInfo,
            sizeof(outInfo), NULL)) {
        ALOGD("<jpgToYV12><ERROR>:get info error!!!");
        return false;
    }

    pOutBuf->width = ALIGN16(outInfo.srcWidth / sampleSize);
    pOutBuf->height = ALIGN16(outInfo.srcHeight / sampleSize);
    pOutBuf->bufferSize = pOutBuf->width * pOutBuf->height * 3 / 2;
    ALOGD("<jpgToYV12>outInfo.srcWidth:%d,outInfo.srcHeight:%d,pOutBuf->width:%d,pOutBuf->height:%d "
            "pOutBuf->bufferSize:%d", outInfo.srcWidth, outInfo.srcHeight, pOutBuf->width,
            pOutBuf->height, pOutBuf->bufferSize);
    pOutBuf->buffer = (MUINT8*)malloc(pOutBuf->bufferSize);

    memset(&inParams, 0, sizeof(inParams));
    inParams.srcFD = -1;
    inParams.dstFD = -1;
    inParams.dstFormat = JPEG_OUT_FORMAT_I420;
    inParams.srcBuffer = pJpgBuf;
    inParams.srcBufSize = ALIGN128(jpgBufSize) + 512;  // why?
    inParams.srcLength = jpgBufSize;
    inParams.dstPhysAddr = NULL;
    inParams.doDithering = 0;
    inParams.doRangeDecode = 0;
    inParams.doPostProcessing = 0;
    inParams.postProcessingParam = NULL;
    inParams.PreferQualityOverSpeed = 0;
    inParams.jpgDecHandle = srcInfo.jpgDecHandle;
    inParams.dstWidth = pOutBuf->width;
    inParams.dstHeight = pOutBuf->height;
    inParams.dstVirAddr = pOutBuf->buffer;
    int ret = mHalJpeg(MHAL_IOCTL_JPEG_DEC_START, (void *)&inParams, sizeof(inParams),
                          NULL, 0, NULL);
    if (MHAL_NO_ERROR != mHalJpeg(MHAL_IOCTL_JPEG_DEC_START, (void *)&inParams, sizeof(inParams),
            NULL, 0, NULL)) {
        ALOGI("<jpgToYv12><ERROR>: JPEG HW not support this image");
        return false;
    }
    return true;
}

// use ion buffer to decode
bool JpegCodec::jpgToYv12(IonInfo* pJpgBuf, MUINT32 jpgBufSize, IonInfo** pOutBuf,
            MUINT32 sampleSize, int32_t ionHandle) {
    MHAL_JPEG_DEC_INFO_OUT outInfo;
    MHAL_JPEG_DEC_START_IN inParams;
    MHAL_JPEG_DEC_SRC_IN srcInfo;

    ALOGD("<jpgToYV12> start ion pJpgBuf:%p,bufSize:%d ", pJpgBuf->virAddr, jpgBufSize);

    srcInfo.jpgDecHandle = NULL;
    srcInfo.srcBuffer = (MUINT8 *)pJpgBuf->virAddr;
    srcInfo.srcLength = jpgBufSize;

    if (MHAL_NO_ERROR != mHalJpeg(MHAL_IOCTL_JPEG_DEC_PARSER, (void *)&srcInfo, sizeof(srcInfo),
            NULL, 0, NULL)) {
        ALOGD("<jpgToYV12><ERROR> parser file error!!!");
        return false;
    }

    outInfo.jpgDecHandle = srcInfo.jpgDecHandle;
    if (MHAL_NO_ERROR != mHalJpeg(MHAL_IOCTL_JPEG_DEC_GET_INFO, NULL, 0, (void *)&outInfo,
            sizeof(outInfo), NULL)) {
        ALOGD("<jpgToYV12><ERROR>:get info error!!!");
        return false;
    }

    // allocate dst ion buffer instead of M4U
    int width = ALIGN16(outInfo.srcWidth / sampleSize);
    int height = ALIGN16(outInfo.srcHeight / sampleSize);
    int size = width * height * 3 / 2;
    *pOutBuf = allocateIon(ALIGN512(size) + 512, ionHandle);
    if (*pOutBuf == NULL) {
        ALOGD("<jpgToYV12><error> allocate pOutBuf fail");
        return false;
    }
    (*pOutBuf)->width = width;
    (*pOutBuf)->height = height;
    (*pOutBuf)->size = size;

    ALOGD("<jpgToYV12>outInfo.srcWidth:%d,outInfo.srcHeight:%d,pOutBuf->width:%d,pOutBuf->height:%d "
            "pOutBuf->bufferSize:%d", outInfo.srcWidth, outInfo.srcHeight, (*pOutBuf)->width,
            (*pOutBuf)->height, (*pOutBuf)->size);
    memset(&inParams, 0, sizeof(inParams));
    inParams.srcFD = pJpgBuf->ionFd;
    inParams.dstFD = (*pOutBuf)->ionFd;
    inParams.dstFormat = JPEG_OUT_FORMAT_I420;
    inParams.srcBuffer = (MUINT8 *)pJpgBuf->virAddr;
    inParams.srcBufSize = ALIGN512(jpgBufSize) + 512;
    inParams.srcLength = jpgBufSize;
    inParams.dstPhysAddr = NULL;
    inParams.doDithering = 0;
    inParams.doRangeDecode = 0;
    inParams.doPostProcessing = 0;
    inParams.postProcessingParam = NULL;
    inParams.PreferQualityOverSpeed = 0;
    inParams.jpgDecHandle = srcInfo.jpgDecHandle;
    inParams.dstWidth = (*pOutBuf)->width;
    inParams.dstHeight = (*pOutBuf)->height;
    inParams.dstVirAddr = (MUINT8 *)(*pOutBuf)->virAddr;

    if (MHAL_NO_ERROR != mHalJpeg(MHAL_IOCTL_JPEG_DEC_START, (void *)&inParams, sizeof(inParams),
            NULL, 0, NULL)) {
        ALOGI("<jpgToYv12><ERROR>: JPEG HW not support this image");
        return false;
    }
    return true;
}

bool JpegCodec::nv21ToJpg(MUINT8 *srcBuffer, MUINT32 srcWidth, MUINT32 srcHeight,
        MUINT8 *dstBuffer, MUINT32 dstSize, MUINT32* u4EncSize) {
    bool ret = false;
    MINT32 fIsAddSOI = 1;  // if set true, not need add exif
    MINT32 quality = 80;
    size_t src_size[2];
    size_t src_addr[2];

    src_size[0] = srcWidth * srcHeight;
    src_size[1] = (srcWidth) * (srcHeight / 2);

    src_addr[0] = (size_t)srcBuffer;
    src_addr[1] = (size_t )(srcBuffer + src_size[0]);

    ALOGD("<nv21ToJpg>begin,srcBuffer:%p,srcWidth:%d,srcHeight:%d,dstBuffer:%p,"
            "dstSize:%d,u4EncSize:%d", srcBuffer, srcWidth, srcHeight, dstBuffer,
            dstSize, *u4EncSize);

    JpgEncHal* pJpgEncoder = new JpgEncHal();
    // (1). Lock
    pJpgEncoder->unlock();
    if (!pJpgEncoder->lock()) {
        ALOGI("<nv21ToJpg>ERROR:can't lock jpeg resource!!!");
        delete pJpgEncoder;
        return false;
    }
    pJpgEncoder->setEncSize(srcWidth, srcHeight, JpgEncHal::kENC_NV21_Format);
    pJpgEncoder->setSrcAddr((void*)src_addr[0], (void*)src_addr[1], NULL);
    pJpgEncoder->setSrcBufSize(srcWidth, src_size[0], src_size[1]);
    pJpgEncoder->setQuality(quality);
    pJpgEncoder->setDstAddr((void *)dstBuffer);
    pJpgEncoder->setDstSize(dstSize);
    // pJpgEncoder->enableSOI((fIsAddSOI > 0) ? 1 : 0);
    pJpgEncoder->enableSOI(fIsAddSOI);
    if (pJpgEncoder->start(u4EncSize)) {
        ALOGD("<nv21ToJpg>Jpeg encode done, size = %d", *u4EncSize);
        ret = true;
    } else {
        ALOGI("<nv21ToJpg><ERROR>:encode fail");
    }
    pJpgEncoder->unlock();
    delete pJpgEncoder;
    ALOGD("<nv21ToJpg>end ret:%d, u4EncSize:%d", ret, *u4EncSize);
    return ret;
}

bool JpegCodec::nv21ToJpg(IonInfo *srcBuffer, MUINT32 srcWidth, MUINT32 srcHeight,
        IonInfo *dstBuffer, MUINT32 dstSize, MUINT32* u4EncSize) {
    bool ret = false;
    MINT32 fIsAddSOI = 1;  // if set true, not need add exif
    MINT32 quality = 80;
    size_t src_size[2];
    size_t src_addr[2];

    src_size[0] = srcWidth * srcHeight;
    src_size[1] = (srcWidth) * (srcHeight / 2);

    size_t addr = (size_t)srcBuffer->virAddr;
    src_addr[0] = addr;
    src_addr[1] = (size_t)(addr + src_size[0]);

    ALOGD("<nv21ToJpg>begin,ion, srcBuffer:%p,srcWidth:%d,srcHeight:%d,dstBuffer:%p,"
            "dstSize:%d,u4EncSize:%d, src_fd:%d,dst_fd:%d", srcBuffer->virAddr, srcWidth, srcHeight, dstBuffer->virAddr,
            dstSize, *u4EncSize, srcBuffer->ionFd, dstBuffer->ionFd);

    JpgEncHal* pJpgEncoder = new JpgEncHal();
    pJpgEncoder->unlock();
    if (!pJpgEncoder->lock()) {
        ALOGI("<nv21ToJpg>ERROR:can't lock jpeg resource!!!");
        delete pJpgEncoder;
        return false;
    }
    // ALOGD("<setEncSize> srcWidth:%d,srcHeight:%d,format:%d", srcWidth, srcHeight, JpgEncHal::kENC_NV21_Format);
    pJpgEncoder->setEncSize(srcWidth, srcHeight, JpgEncHal::kENC_NV21_Format);
    // ALOGD("<setSrcAddr> addr0:%p, addr1:%p", (void*)src_addr[0], (void*)src_addr[1]);
    pJpgEncoder->setSrcAddr((void*)src_addr[0], (void*)src_addr[1], NULL);
    // ALOGD("<setSrcBufSize>srcWidth:%d, size0:%d, size1:%d", srcWidth, src_size[0], src_size[1]);
    pJpgEncoder->setSrcBufSize(srcWidth, src_size[0], src_size[1]);
    // ALOGD("<setQuality>:%d", quality);
    pJpgEncoder->setQuality(quality);
    // ALOGD("<setDstAddr> :%p", (void *)dstBuffer->virAddr);
    pJpgEncoder->setDstAddr((void *)dstBuffer->virAddr);
    // ALOGD("<setDstSize>:%d", dstSize);
    pJpgEncoder->setDstSize(dstSize);
    // ALOGD("<enableSOI>:%d", (fIsAddSOI ? 1 : 0));
    pJpgEncoder->enableSOI(fIsAddSOI);
    pJpgEncoder->setIonMode(true);
    // ALOGD("<setSrcFD>:%d,%d", srcBuffer->ionFd, srcBuffer->ionFd);
    pJpgEncoder->setSrcFD(srcBuffer->ionFd, srcBuffer->ionFd);
    // ALOGD("<setDstFD> %d", dstBuffer->ionFd);
    pJpgEncoder->setDstFD(dstBuffer->ionFd);
    if (pJpgEncoder->start(u4EncSize)) {
        ALOGD("<nv21ToJpg>Jpeg encode done, size = %d", *u4EncSize);
        ret = true;
    } else {
        ALOGI("<nv21ToJpg><ERROR>:encode fail");
    }
    pJpgEncoder->unlock();
    delete pJpgEncoder;
    ALOGD("<nv21ToJpg>end ret:%d, u4EncSize:%d", ret, *u4EncSize);

     if (NULL != m_pDebugHelper) {
        m_pDebugHelper->dumpBufferToFile("/sdcard/effect/nv21ToJpg_out.jpg", (uint8_t*)dstBuffer->virAddr,
                *u4EncSize);
    }

    return ret;
}

bool JpegCodec::yuvToRgba(MUINT8 *yuvBuf, MUINT32 width, MUINT32 height, MUINT8 *outBuf) {
    DpBlitStream bltStream;
    DpColorFormat dp_out_fmt = eRGBA8888;
    DpColorFormat dp_in_fmt = eI420;  // eYV12;//eI420; //eYUV_420_3P
    MUINT32 plane_num = 3;
    MUINT32 src_size[3];
    DP_STATUS_ENUM rst;
    void* src_addr[3];

    if (NULL != m_pDebugHelper) {
        m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yuvToRgba_in.yuv", yuvBuf, width * height * 3 / 2);
    }
    src_size[0] = width * height;
    src_size[1] = (width / 2) * (height / 2);
    src_size[2] = (width / 2) * (height / 2);
    src_addr[0] = yuvBuf;
    src_addr[1] = yuvBuf + width * height;
    src_addr[2] = yuvBuf + width * height + (width / 2) * (height / 2);

    bltStream.setSrcBuffer((void**) src_addr, src_size, plane_num);
    bltStream.setSrcConfig(width, height, width, width/2, dp_in_fmt,
                           DP_PROFILE_JPEG);
    // bltStream.setSrcConfig(width, height, dp_in_fmt);

    bltStream.setDstBuffer((void*)outBuf, width * height * 4);
    bltStream.setDstConfig(width, height, width * 4, 0, dp_out_fmt,
                           DP_PROFILE_JPEG);
    // bltStream.setDstConfig(width, height, dp_out_fmt);
    rst = bltStream.invalidate();
    if (rst < 0)
    {
        ALOGI("<yuvToRgba><ERROR> invalidate fail!!");
        return false;
    }
    if (NULL != m_pDebugHelper) {
            m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yuvToRgba_out.raw", outBuf,
                    width * height * 4);
    }
    return true;
}

bool JpegCodec::yuvToNv21(MUINT8 *yuvBuf, MUINT32 width, MUINT32 height, MUINT8 *outBuf) {
    DpBlitStream bltStream;
    DpColorFormat dp_out_fmt = eNV21;
    DpColorFormat dp_in_fmt = eI420;  // eYV12;//eI420; //eYUV_420_3P
    unsigned int src_plane_num = 3;
    unsigned int dst_plane_num = 2;
    unsigned int src_size[3];
    unsigned int dst_size[2];
    DP_STATUS_ENUM rst;
    void* src_addr[3];
    void* dst_addr[2];
    ALOGD("<yuvToNv21> start,yuvBuf:%p, width:%d, height:%d,outBuf:%p", yuvBuf, width, height,
            outBuf);
    if (NULL != m_pDebugHelper) {
        m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yuvToNv21_in.yuv", yuvBuf, width*height*3/2);
    }

    src_size[0] = width * height;
    src_size[1] = (width / 2) * (height / 2);
    src_size[2] = (width / 2) * (height / 2);
    src_addr[0] = yuvBuf;
    src_addr[1] = yuvBuf + width * height;
    src_addr[2] = yuvBuf + width * height + (width / 2) * (height / 2);

    bltStream.setSrcBuffer((void**) src_addr, src_size, src_plane_num);
    bltStream.setSrcConfig(width, height, width, width/2, dp_in_fmt,
                           DP_PROFILE_JPEG);
    dst_size[0] = width * height;
    dst_size[1] = width * (height / 2);
    dst_addr[0] = outBuf;
    dst_addr[1] = outBuf + width * height;
    bltStream.setDstBuffer((void**)dst_addr, dst_size, dst_plane_num);
    bltStream.setDstConfig(width, height, width, width, dp_out_fmt, DP_PROFILE_JPEG);
    rst = bltStream.invalidate();
    if (rst < 0) {
        ALOGI("<yuvToNv21><error> invalidate fail!!");
        return false;
    }
    if (NULL != m_pDebugHelper) {
        m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yuvToNv21_out.yuv", outBuf, width*height*3/2);
    }
    ALOGD("<yuvToNv21> end");
    return true;
}

bool JpegCodec::yv12ToNv21(MUINT8 *yuvBuf, MUINT32 width, MUINT32 height, MUINT8 *outBuf) {
    DpBlitStream *pbltStream = new DpBlitStream();
    if (NULL == pbltStream) {
        ALOGE("<yv12ToNv21><ERROR>create DpBlitStream fail!!");
        return false;
    }

    DpColorFormat dp_out_fmt = eNV21;
    DpColorFormat dp_in_fmt = eYV12;  // eYV12;//eI420; //eYUV_420_3P
    DP_STATUS_ENUM rst;
    unsigned int src_plane_num = 3;
    unsigned int dst_plane_num = 2;
    unsigned int src_size[3];
    unsigned int dst_size[2];

    void* src_addr[3];
    void* dst_addr[2];
    ALOGD("<yv12ToNv21> start,yuvBuf:%p, width:%d, height:%d,outBuf:%p", yuvBuf, width, height,
            outBuf);
    if (NULL != m_pDebugHelper) {
        m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yuvToNv21_in.yuv", yuvBuf, width*height*3/2);
    }

    src_size[0] = width * height;
    src_size[1] = (width / 2) * (height / 2);
    src_size[2] = (width / 2) * (height / 2);
    src_addr[0] = yuvBuf;
    src_addr[1] = yuvBuf + width * height;
    src_addr[2] = yuvBuf + width * height + (width / 2) * (height / 2);

    pbltStream->setSrcBuffer((void**) src_addr, src_size, src_plane_num);
    pbltStream->setSrcConfig(width, height, width, width/2, dp_in_fmt,
                           DP_PROFILE_JPEG);
    dst_size[0] = width * height;
    dst_size[1] = width * (height / 2);
    dst_addr[0] = outBuf;
    dst_addr[1] = outBuf + width * height;
    pbltStream->setDstBuffer((void**)dst_addr, dst_size, dst_plane_num);
    pbltStream->setDstConfig(width, height, width, width, dp_out_fmt, DP_PROFILE_JPEG);
    rst = pbltStream->invalidate();
    delete pbltStream;
    pbltStream = NULL;
    if (rst < 0) {
        ALOGI("<yv12ToNv21><error> invalidate fail!!");
        return false;
    }
    if (NULL != m_pDebugHelper) {
        m_pDebugHelper->dumpBufferToFile("/sdcard/effect/yuvToNv21_out.yuv", outBuf, width*height*3/2);
    }
    ALOGD("<yv12ToNv21> end");

    return true;
}

bool JpegCodec::RGBAToNv21(uint8_t *pRGBABuf, uint32_t width, uint32_t height, uint8_t *outBuf) {
    DpBlitStream *pbltStream = new DpBlitStream();
    if (NULL == pbltStream) {
        ALOGE("<RGBAToNv21><ERROR>create DpBlitStream fail!!");
        return false;
    }

    DpColorFormat dp_out_fmt = eNV21;
    DpColorFormat dp_in_fmt = eRGBA8888;  // eYV12;//eI420; //eYUV_420_3P
    unsigned int src_plane_num = 1;
    unsigned int dst_plane_num = 2;
    unsigned int src_size[3];
    unsigned int dst_size[2];
    DP_STATUS_ENUM rst;
    void* src_addr[3];
    void* dst_addr[2];

    ALOGD("<RGBAToNv21> start,yuvBuf:%p, width:%d, height:%d,outBuf:%p,xxx", pRGBABuf, width, height,
            outBuf);
     if (NULL != m_pDebugHelper) {
         m_pDebugHelper->dumpBufferToFile("/sdcard/effect/RGBAToNv21_in.yuv", pRGBABuf, width*height*4);
     }

    pbltStream->setSrcBuffer((void*)pRGBABuf, width * height * 4);
    pbltStream->setSrcConfig(width, height, width*4, 0, dp_in_fmt,
                           DP_PROFILE_JPEG);
    dst_size[0] = width * height;
    dst_size[1] = width * (height / 2);
    dst_addr[0] = outBuf;
    dst_addr[1] = outBuf + width * height;
    pbltStream->setDstBuffer((void**)dst_addr, dst_size, dst_plane_num);
    pbltStream->setDstConfig(width, height, width, width, dp_out_fmt, DP_PROFILE_JPEG);
    rst = pbltStream->invalidate();
    delete pbltStream;
    pbltStream = NULL;

    if (rst < 0) {
        ALOGI("<RGBAToNv21><error> invalidate fail!!");
        return false;
    }
    if (NULL != m_pDebugHelper) {
        m_pDebugHelper->dumpBufferToFile("/sdcard/effect/RGBAToNv21_out.yuv", outBuf, width*height*3/2);
    }

    ALOGD("<RGBAToNv21> dump end");
    return true;
}

IonInfo* JpegCodec::allocateIon(int size, int32_t ionHandle) {
    if (ionHandle < 0) {
        ALOGD("<allocateIon><error> ionHandle < 0, return NULL");
        return NULL;
    }
    ion_user_handle_t ionAllocHandle;
    int32_t ionFd = -1;
    int allocRet = ion_alloc_mm(ionHandle, size, 1, 0, &ionAllocHandle);
    if (allocRet) {
        ALOGD("<allocateIon> src Ion allocate failed, allocRet:%d, return", allocRet);
        return NULL;
    }
    int shareRet = ion_share(ionHandle, ionAllocHandle, &ionFd);
    if (shareRet) {
        ALOGD("src Ion share failed, shareRet:%d, return", shareRet);
        return NULL;
    }
    ALOGD("<allocateIon> allocate ION success, ionFd=%d,ionHandle=%d", ionFd, ionHandle);
    void *virtualAddr = ion_mmap(ionHandle, 0, size,
            PROT_READ | PROT_WRITE, MAP_SHARED, ionFd, 0);
    IonInfo *info = (IonInfo *)malloc(sizeof(IonInfo));
    info->ionFd = ionFd;
    info->size = size;
    info->virAddr = virtualAddr;
    info->ionHandle = ionAllocHandle;
    return info;
}

void JpegCodec::destroyIon(IonInfo* info, int32_t ionHandle) {
    if (ionHandle < 0) {
        ALOGD("<destroyIon><ERROR> ionHandle < 0");
        return;
    }
    ion_munmap(ionHandle, info->virAddr, info->size);
    ion_share_close(ionHandle, info->ionFd);
    ion_free(ionHandle, info->ionHandle);
    free(info);
}
