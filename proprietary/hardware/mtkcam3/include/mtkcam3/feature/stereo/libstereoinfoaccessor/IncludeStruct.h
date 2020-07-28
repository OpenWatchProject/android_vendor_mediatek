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

#ifndef INCLUDE_STRUCT_H
#define INCLUDE_STRUCT_H

/* === include === */
/* StereoCaptureInfo.h */
#include <sstream>
/* types.h */
#include <map>
#include <vector>
#include <string>
/* Utils.h */
#include <sys/time.h>
/* BufferManager.h */
#include <utils/Mutex.h>
#include <utils/threads.h>

/* === define === */
/* types.h */
#define StereoVector std::vector
#define StereoMap std::map

/* === typedef === */
/* types.h */
typedef std::string StereoString;
typedef unsigned char   S_UINT8;
typedef unsigned int    S_UINT32;

namespace stereo {

/* === struct & class === */
/* BufferManager.h */
typedef struct StereoBuffer {
public:
    StereoBuffer() : data(nullptr), size(0) {}
    StereoBuffer(S_UINT8 *_data, S_UINT32 _size)     : data(_data), size(_size) {}
    bool isValid() const {
        return data != nullptr && size != 0;
    }
    S_UINT8 *data;
    S_UINT32 size;
} StereoBuffer_t;

class StereoBigBuffer {
public:
    // buffer start address
    S_UINT8 *data;
    // buffer total size
    S_UINT32 totalSize;
    // next avaliable buffer start position
    S_UINT32 next;
    // remaining available memory bytes
    S_UINT32 availables;

    StereoBigBuffer(S_UINT32 size);

    virtual ~StereoBigBuffer();

    bool isAvaliable(S_UINT32 size);

    void allocate(S_UINT32 size, StereoBuffer_t &buffer);
};

class BufferManager {

public:
    static void createBuffer(S_UINT32 size, StereoBuffer_t &buffer);
    static void releaseAll();
    static StereoVector<StereoBigBuffer*>* findBigBuffers();
    static StereoVector<StereoBigBuffer*>* popBigBuffers();
    static void pushBigBuffer(StereoBigBuffer* pBuffer);
private:
    static android::Mutex mBufferLock;
    static StereoMap<pid_t, StereoVector<StereoBigBuffer*>*> mBuffers;
};

/* BufferManager.h */
typedef StereoMap<StereoString, StereoBuffer_t> BufferMap;
typedef BufferMap * BufferMapPtr;

/* StereoCaptureInfo.h */
class StereoCaptureInfo {
public:
    StereoString debugDir;
    StereoBuffer_t jpgBuffer;
    StereoBuffer_t jpsBuffer;
    StereoBuffer_t configBuffer;
    StereoBuffer_t clearImage;
    StereoBuffer_t depthMap;
    StereoBuffer_t ldc;
    // add for kibo+
    StereoBuffer_t depthBuffer; // mtk depth
    StereoBuffer_t debugBuffer;

    StereoCaptureInfo() {}

    virtual ~StereoCaptureInfo();

    StereoString toString() {
        std::stringstream ss;
        ss << "~StereoCaptureInfo:";
        ss << "\n    jpgBuffer length = " << jpgBuffer.size;
        ss << "\n    jpsBuffer length = " << jpsBuffer.size;
        ss << "\n    jsonBuffer length = " << configBuffer.size;
        ss << "\n    clearImage length = " << clearImage.size;
        ss << "\n    depthMap length = " << depthMap.size;
        ss << "\n    depthBuffer length = " << depthBuffer.size;
        ss << "\n    ldc length = " << ldc.size;
        ss << "\n    debugBuffer length = " << debugBuffer.size;
        return ss.str();
    }
};

/* StereoDepthInfo.h */
class StereoDepthInfo {
public:
    StereoString debugDir;

    int metaBufferWidth;
    int metaBufferHeight;

    int touchCoordXLast;
    int touchCoordYLast;
    int depthOfFieldLast;

    int depthBufferWidth;
    int depthBufferHeight;
    StereoBuffer_t depthBuffer;

    int depthMapWidth;
    int depthMapHeight;
    StereoBuffer_t depthMap;

    // add for kibo+
    StereoBuffer_t debugBuffer;

    StereoDepthInfo() :
        metaBufferWidth(0),
        metaBufferHeight(0),
        touchCoordXLast(0),
        touchCoordYLast(0),
        depthOfFieldLast(0),
        depthBufferWidth(0),
        depthBufferHeight(0),
        depthMapWidth(0),
        depthMapHeight(0) {}

    virtual ~StereoDepthInfo();

    StereoString toString() {
        std::stringstream ss;
        ss << "StereoDepthInfo:";
        ss << "\n    metaBufferWidth  = " << metaBufferWidth;
        ss << "\n    metaBufferHeight = " << metaBufferHeight;
        ss << "\n    touchCoordXLast = " << touchCoordXLast;
        ss << "\n    touchCoordYLast = " << touchCoordYLast;
        ss << "\n    depthOfFieldLast = " << depthOfFieldLast;
        ss << "\n    depthBufferWidth = " << depthBufferWidth;
        ss << "\n    depthBufferHeight = " << depthBufferHeight;
        ss << "\n    depthMapWidth = " << depthMapWidth;
        ss << "\n    depthMapHeight = " << depthMapHeight;
        ss << "\n    depthBuffer length = " << depthBuffer.size;
        ss << "\n    depthMap length = " << depthMap.size;
        ss << "\n    debugBuffer length = " << debugBuffer.size;
        return ss.str();
    }
};

/* SegmentMaskInfo.h */
class SegmentMaskInfo {
public:
    StereoString debugDir;
    int maskWidth;
    int maskHeight;
    int segmentX;
    int segmentY;
    int segmentLeft;
    int segmentTop;
    int segmentRight;
    int segmentBottom;
    StereoBuffer_t maskBuffer;

    SegmentMaskInfo() :
        maskWidth(0),
        maskHeight(0),
        segmentX(0),
        segmentY(0),
        segmentLeft(0),
        segmentTop(0),
        segmentRight(0),
        segmentBottom(0) {
    }

    virtual ~SegmentMaskInfo();

    StereoString toString() {
        std::stringstream ss;
        ss << "SegmentMaskInfo:";
        ss << "\n    maskWidth  = "<< maskWidth;
        ss << "\n    maskHeight = " << maskHeight;
        ss << "\n    segmentX = " << segmentX;
        ss << "\n    segmentY = " << segmentY;
        ss << "\n    segmentLeft = " << segmentLeft;
        ss << "\n    segmentTop = " << segmentTop;
        ss << "\n    segmentRight = " << segmentRight;
        ss << "\n    segmentBottom = " << segmentBottom;
        ss << "\n    maskBuffer length = " << maskBuffer.size;
        return ss.str();
    }
};

/* StereoConfigInfo.h */
class FaceDetectionInfo {
public:
    int faceLeft;
    int faceTop;
    int faceRight;
    int faceBottom;
    int faceRip;

    FaceDetectionInfo() :
        faceLeft(0),
        faceTop(0),
        faceRight(0),
        faceBottom(0),
        faceRip(0) {
    }

    FaceDetectionInfo(int left, int top, int right, int bottom, int rip) {
        faceLeft = left;
        faceTop = top;
        faceRight = right;
        faceBottom = bottom;
        faceRip = rip;
    }

    StereoString toString() {
        std::stringstream ss;
        ss << "FaceDetectionInfo:";
        ss << "\n    faceLeft = " << faceLeft;
        ss << "\n    faceTop = " << faceTop;
        ss << "\n    faceRight = " << faceRight;
        ss << "\n    faceBottom = " << faceBottom;
        ss << "\n    faceRip = " << faceRip;
        return ss.str();
    }
};

class FocusInfo {

public:
    int focusType;
    int focusTop;
    int focusLeft;
    int focusRight;
    int focusBottom;

    FocusInfo() :
        focusType(0),
        focusTop(0),
        focusLeft(0),
        focusRight(0),
        focusBottom(0) {
    }

    FocusInfo(int type, int left, int top, int right, int bottom) {
        focusLeft = left;
        focusTop = top;
        focusRight = right;
        focusBottom = bottom;
        focusType = type;
    }

    StereoString toString() {
        std::stringstream ss;
        ss << "FocusInfo:";
        ss << "(left,top,right,bottom|type): ";
        ss << "(" << focusLeft << "," << focusTop << "," << focusRight << "," << focusBottom
                << "|" << focusType << ")";
        return ss.str();
    }
};

class StereoConfigInfo {
public:
    StereoString debugDir;
    int jpsWidth;
    int jpsHeight;
    int maskWidth;
    int maskHeight;
    int posX;
    int posY;
    int viewWidth;
    int viewHeight;
    int imageOrientation;
    int depthOrientation;
    int mainCamPos;
    int touchCoordX1st;
    int touchCoordY1st;
    int faceCount;
    FocusInfo *focusInfo;
    StereoVector<FaceDetectionInfo*> *fdInfoArray;
    int dofLevel;
    float convOffset;
    int ldcWidth;
    int ldcHeight;
    StereoBuffer_t ldcBuffer;
    StereoBuffer_t clearImage;

    // FocusInfo
    bool isFace;
    float faceRatio;
    int curDac;
    int minDac;
    int maxDac;

    StereoConfigInfo() :
        jpsWidth(0), jpsHeight(0), maskWidth(0), maskHeight(0), posX(0),
        posY(0), viewWidth(0), viewHeight(0), imageOrientation(0), depthOrientation(0),
        mainCamPos(0), touchCoordX1st(0), touchCoordY1st(0), faceCount(0), focusInfo(nullptr),
        fdInfoArray(nullptr), dofLevel(0), convOffset(0.0), ldcWidth(0), ldcHeight(0),
        isFace(false), faceRatio(0.0), curDac(0), minDac(0), maxDac(0) {}

    virtual ~StereoConfigInfo();

    StereoString toString() {
        std::stringstream ss;
        ss << "StereoConfigInfo:";
        ss << "\n    jpsWidth  = " << jpsWidth;
        ss << "\n    jpsHeight = " << jpsHeight;
        ss << "\n    maskWidth = " << maskWidth;
        ss << "\n    maskHeight = " << maskHeight;
        ss << "\n    posX = " << posX ;
        ss << "\n    posY = " << posY;
        ss << "\n    viewWidth = " << viewWidth;
        ss << "\n    viewHeight = " << viewHeight;
        ss << "\n    imageOrientation = " << imageOrientation;
        ss << "\n    depthOrientation = " << depthOrientation;
        ss << "\n    mainCamPos = " << mainCamPos;
        ss << "\n    touchCoordX1st = " << touchCoordX1st;
        ss << "\n    touchCoordY1st = " << touchCoordY1st;
        ss << "\n    faceCount = " << faceCount;
        if (focusInfo != nullptr) {
            ss << "\n    " << focusInfo->toString();
        } else {
            ss << "\n    focusInfo == null";
        }
        ss << "\n    dofLevel = " << dofLevel;
        ss << "\n    convOffset = " << convOffset;
        ss << "\n    ldcWidth = " << ldcWidth;
        ss << "\n    ldcHeight = " << ldcHeight;
        ss << "\n    ldcBuffer length = " << ldcBuffer.size;
        ss << "\n    clearImage length = " << clearImage.size;
        // focus info
        ss << "\n    isFace = " << isFace;
        ss << "\n    faceRatio = " << faceRatio;
        ss << "\n    curDac = " << curDac;
        ss << "\n    minDac = " << minDac;
        ss << "\n    maxDac = " << maxDac;

        if (fdInfoArray != nullptr) {
            int fdInfoCount = fdInfoArray->size();
            for (int i = 0; i < fdInfoCount; i++) {
                ss << "\n    fdInfoArray[" << i << "] = " << (*fdInfoArray)[i]->toString();
            }
        } else {
            ss << "\n    fdInfoArray = null";
        }
        return ss.str();
    }
};

/* StereoBufferInfo.h */
class StereoBufferInfo {
public:
    StereoString debugDir;
    StereoBuffer_t jpsBuffer;
    StereoBuffer_t maskBuffer;

    StereoBufferInfo() {}
    virtual ~StereoBufferInfo();

    StereoString toString() {
        std::stringstream ss;
        ss << "StereoBufferInfo:";
        ss << "\n    jpsBuffer length = " << jpsBuffer.size;
        ss << "\n    maskBuffer length = " << maskBuffer.size;
        return ss.str();
    }
};

/* GoogleStereoInfo.h */
class GoogleStereoInfo {
public:
    StereoString debugDir;
    double focusBlurAtInfinity;
    double focusFocalDistance;
    double focusFocalPointX;
    double focusFocalPointY;
    StereoString imageMime;
    StereoString depthFormat;
    double depthNear;
    double depthFar;
    StereoString depthMime;

    StereoBuffer_t clearImage;
    StereoBuffer_t depthMap;

    GoogleStereoInfo() :
        focusBlurAtInfinity(0.0),
        focusFocalDistance(0.0),
        focusFocalPointX(0.0),
        focusFocalPointY(0.0),
        depthNear(0.0),
        depthFar(0.0) {
    }

    virtual ~GoogleStereoInfo();

    StereoString toString() {
        std::stringstream ss;
        ss.precision(std::numeric_limits<double>::digits10);
        ss << "GoogleStereoInfo:";
        ss << "\n    focusBlurAtInfinity = " << focusBlurAtInfinity;
        ss << "\n    focusFocalDistance = " << focusFocalDistance;
        ss << "\n    focusFocalPointX = " << focusFocalPointX;
        ss << "\n    focusFocalPointY = " << focusFocalPointY;
        ss << "\n    imageMime = " << imageMime;
        ss << "\n    depthFormat = " << depthFormat;
        ss << "\n    depthNear = " << depthNear;
        ss << "\n    depthFar = " << depthFar;
        ss << "\n    depthMime = " << depthMime;
        ss << "\n    clearImage length = " << clearImage.size;
        ss << "\n    depthMap length = " << depthMap.size;
        return ss.str();
    }
};
} //namespace stereo

/* SerializedInfo.h */
typedef struct SerializedInfo {
    // Used to saving serialized Standard Xmp buffer.
    stereo::StereoBuffer_t standardXmpBuf;
    // Used to saving serialized Extended Xmp buffer.
    stereo::StereoBuffer_t extendedXmpBuf;
    // Used to saving serialized CustomizedBuf map.
    stereo::BufferMapPtr customizedBufMap;

    SerializedInfo() :
        customizedBufMap(nullptr) {
    }
} SerializedInfo_t;

namespace stereo {

/* IPacker.h */
/**
 * Pack parameter for packer and unpacker.
 *
 * Packing ------------------------------------>---------------------------------->
 *                 |serialize          |Cust/XmpPacker          |JpgPacker
 *                 (meta serialize)    (append header)          (append APPX tag)
 *
 * StandardXMP     |StereoBuffer       |StereoBuffer            |StereoBuffer
 * ExtendedXMP     |StereoBuffer       |vector<StereoBuffer>    |vector<StereoBuffer>
 * CustomizedData  |BufferMap          |vector<StereoBuffer>    |vector<StereoBuffer>
 *
 * <-------------------------------------------<--------------------------unpacking
 */
class PackInfo {

public:
    // input for packing
    StereoBuffer_t unpackedJpgBuf;
    StereoBuffer_t unpackedBlurImageBuf;
    StereoBuffer_t unpackedStandardXmpBuf;
    StereoBuffer_t unpackedExtendedXmpBuf;
    BufferMapPtr unpackedCustomizedBufMap;

    // input for unpacking
    StereoBuffer_t packedJpgBuf;
    StereoBuffer_t packedStandardXmpBuf;
    StereoVector<StereoBuffer_t>* packedExtendedXmpBufArray;
    StereoVector<StereoBuffer_t>* packedCustomizedBufArray;

    PackInfo() : unpackedCustomizedBufMap(nullptr),
        packedExtendedXmpBufArray(nullptr), packedCustomizedBufArray(nullptr) {}

    ~PackInfo() {
        if (unpackedCustomizedBufMap != nullptr) {
            delete unpackedCustomizedBufMap;
            unpackedCustomizedBufMap = nullptr;
        }
        if (packedExtendedXmpBufArray != nullptr) {
            delete packedExtendedXmpBufArray;
            packedExtendedXmpBufArray = nullptr;
        }
        if (packedCustomizedBufArray != nullptr) {
            delete packedCustomizedBufArray;
            packedCustomizedBufArray = nullptr;
        }
        // other buffers will released in JNI
    }

    void dump() {
        if (unpackedCustomizedBufMap != nullptr) {
            //StereoLogD("<dump> unpackedCustomizedBufMap");
            for (auto iter = unpackedCustomizedBufMap->begin();
                iter != unpackedCustomizedBufMap->end(); iter++) {
                StereoString type = iter->first;
                StereoBuffer_t buffer = iter->second;
                //StereoLogD("type = %s, buffer size = %d", type.c_str(), buffer->size());
            }
        }
    }
};

/* IParser.h */
class IParser {

public:
    virtual ~IParser() {}
    virtual void read() = 0;
    virtual void write() = 0;
    virtual SerializedInfo* serialize() = 0;

protected:
    const int INSTANTIATION_BY_BUFFER = 0;
    const int INSTANTIATION_BY_OPERATOR = 1;
    int instantiationWay;
};

/* Utils.h */
class Utils {

public:
    static const bool ENABLE_BUFFER_DUMP;
    static const bool ENABLE_GDEPTH;

    static void int2str(const int &intVal, StereoString &strVal);
    static void d2str(const double &dVal, StereoString &strVal);
    static StereoString buffer2Str(const StereoBuffer_t &buffer);
    static StereoString buffer2Str(const StereoBuffer_t &buffer, S_UINT32 size);
    static StereoString buffer2Str(
            const StereoBuffer_t &buffer, S_UINT32  offset, S_UINT32 size);
    static StereoString intToHexString(int val);
    static int isFileExist(const char *filePath);
    static int isDirExist(const char *dirPath);
    static int checkOrCreateDir(const char *dirPath);
    static void writeBufferToFile(const StereoString &destFile, const StereoBuffer_t &buffer);
    static void writeStringToFile(const StereoString &destFile, const StereoString &value);
    static void readFileToBuffer(
        const StereoString &filePath, StereoBuffer_t &outBuffer);
    static StereoString getFileNameFromPath(const StereoString &filePath);
    static void encodePng(const StereoBuffer_t &inputBuffer,
        int width, int height, StereoBuffer_t &outBuffer);

    static long getCurrentTime() {
        struct timeval tv;
        gettimeofday(&tv,nullptr);
        return tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
};
} //namespace stereo

#endif