/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */
#ifndef __MfllImageBuffer_H__
#define __MfllImageBuffer_H__

#include "IMfllImageBuffer.h"

/* mtkcam related headers */
#include <mtkcam/utils/imgbuf/IImageBuffer.h>

#include <utils/RWLock.h> // android::RWLock

#include <string>

using NSCam::IImageBuffer;
using NSCam::IImageBufferHeap;
using android::sp;
using android::RWLock;
using std::string;

namespace mfll {

class MfllImageBuffer : public IMfllImageBuffer {
public:
    MfllImageBuffer(const char *bufferName = "");
    virtual ~MfllImageBuffer(void);

/* attributes */
private:
    unsigned int m_width;
    unsigned int m_height;
    unsigned int m_alignedWidth;
    unsigned int m_alignedHeight;
    unsigned int m_bufferSize;
    enum ImageFormat m_format;
    sp<IMfllEvents> m_spEventDispatcher;
    IMfllCore *m_pCore;
    bool m_bHasOwnership; // flag for the MfllImageBuffer is used for setImageBuffer
    std::string m_bufferName;

    /* saves alias image buffer */
    IImageBuffer *m_imgBuffer;
    IImageBufferHeap *m_imgHeap; // Notice, ion fd is related with heap

    /* saves REAL memory chunk */
    sp<IImageBuffer> m_imgChunk; // saves blob chunk
    sp<IImageBuffer> m_imgOwnFromOther; // contains ownership is not belong to MfllImageBuffer

/* if the MfllImageBuffer should be placed to queue while destroying */
private:
    bool m_handleByQueue; // default is false
public:
    inline void handleByQueue(const bool &b)
    { m_handleByQueue = b; }
    inline bool isHandleByQueue(void)
    { return m_handleByQueue; }

/* thread-safe mutex */
private:
    mutable RWLock m_mutex;

/* implementations from IMfllImageBuffer */
public:
    enum MfllErr setResolution(unsigned int w, unsigned int h);
    enum MfllErr setAligned(unsigned int aligned_w, unsigned int aligned_h);
    enum MfllErr setImageFormat(enum ImageFormat f);
    unsigned int getWidth(void) const { return m_width; }
    unsigned int getHeight(void) const { return m_height; }
    unsigned int getAlignedWidth(void) const;
    unsigned int getAlignedHeight(void) const;
    enum MfllErr getResolution(unsigned int &w, unsigned int &h);
    enum MfllErr getAligned(unsigned int &w, unsigned int &h);
    enum ImageFormat getImageFormat(void);
    enum MfllErr initBuffer(void);
    bool isInited(void);
    enum MfllErr convertImageFormat(const enum ImageFormat &f);
    enum MfllErr setImageBuffer(void *lpBuffer);
    void* getVa(void);
    size_t getRealBufferSize(void);
    size_t getBufStridesInBytes(size_t index);
    void* getImageBuffer(void);
    enum MfllErr releaseBuffer(void);
    enum MfllErr syncCache(void);
    enum MfllErr registerEventDispatcher(const sp<IMfllEvents> &e);
    enum MfllErr saveFile(const char *name);
    enum MfllErr loadFile(const char *name);

    void setName(const char *name)
    { m_bufferName = name; }

    enum MfllErr setMfllCore(IMfllCore *c)
    { m_pCore = c; return MfllErr_Ok; }

/* assign operator, for moving image chunk */
public:
    void operator=(const MfllImageBuffer &in);

private:
    /**
     *  To create a continuous image buffer, only for YUV2 and YV16.
     */
    enum MfllErr createContinuousImageBuffer(const enum ImageFormat &f, unsigned int &imageSize);
    enum MfllErr convertImageBufferFormat(const enum ImageFormat &f);
    enum MfllErr releaseBufferNoLock(void);

public:
    /**
     *  helper function to map mfll::ImageFormat to NSCam::eImgFmt
     *  @param fmt              Format.
     *  @param dontCare         If true, even though cannot mapping, the error
     *                          message won't be displayed.
     */
    static MUINT32 mapImageFormat(const enum ImageFormat &fmt, bool dontCare = false);
    static enum ImageFormat mapImageFormat(const MUINT32 &fmt, bool dontCare = false);
}; /* MfllImageBuffer */

}; /* namespace mfll */
#endif /* __MfllImageBuffer_H__ */
