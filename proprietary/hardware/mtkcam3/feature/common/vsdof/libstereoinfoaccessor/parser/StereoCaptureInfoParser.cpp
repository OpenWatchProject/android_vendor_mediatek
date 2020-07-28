//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/StereoCaptureInfoParser"

#include "StereoCaptureInfoParser.h"
#include "StereoLog.h"
#include <stdlib.h>
#include "MetaOperatorFactory.h"
#include "Utils.h"

using namespace stereo;

StereoCaptureInfoParser:: StereoCaptureInfoParser(
        const StereoBuffer_t &standardBuffer, const StereoBuffer_t &extendedBuffer,
        BufferMapPtr customizedBuffer, StereoCaptureInfo *info) {
    StereoLogD("<StereoCaptureInfoParser> by buffer");

    instantiationWay = INSTANTIATION_BY_BUFFER;

    // new data collections
    pStereoCaptureInfo = info;
    pStereoBufferInfo = new StereoBufferInfo();
    pStereoConfigInfo = new StereoConfigInfo();
    pStereoDepthInfo = new StereoDepthInfo();

    pStandardMetaOperator = MetaOperatorFactory::getOperatorInstance(
        XMP_META_OPERATOR, standardBuffer, nullptr);

    pCustomizedMetaOperator = MetaOperatorFactory::getOperatorInstance(
        CUSTOMIZED_META_OPERATOR, INVALID_BUFFER, customizedBuffer);

    if (Utils::ENABLE_GDEPTH) {
        pGoogleStereoInfo = new GoogleStereoInfo();
        pExtendedMetaOperator = MetaOperatorFactory::getOperatorInstance(
                XMP_META_OPERATOR, extendedBuffer, nullptr);
    }
}

StereoCaptureInfoParser::~StereoCaptureInfoParser() {
    StereoLogD("<~StereoCaptureInfoParser>");
    // delete pStereoBufferInfo
    if (pStereoBufferInfo != nullptr) {
        delete pStereoBufferInfo;
        pStereoBufferInfo = nullptr;
    }
    // delete pStereoConfigInfo
    if (pStereoConfigInfo != nullptr) {
        delete pStereoConfigInfo;
        pStereoConfigInfo = nullptr;
    }
    // delete pStereoDepthInfo
    if (pStereoDepthInfo != nullptr) {
        delete pStereoDepthInfo;
        pStereoDepthInfo = nullptr;
    }
    // delete pGoogleStereoInfo
    if (pGoogleStereoInfo != nullptr) {
        delete pGoogleStereoInfo;
        pGoogleStereoInfo = nullptr;
    }

    // delete parser
    if (pStereoBufferInfoParser != nullptr) {
        delete pStereoBufferInfoParser;
        pStereoBufferInfoParser = nullptr;
    }
    if (pStereoDepthInfoParser != nullptr) {
        delete pStereoDepthInfoParser;
        pStereoDepthInfoParser = nullptr;
    }
    if (pStereoConfigInfoParser != nullptr) {
        delete pStereoConfigInfoParser;
        pStereoConfigInfoParser = nullptr;
    }
    if (pGoogleStereoInfoParser != nullptr) {
        delete pGoogleStereoInfoParser;
        pGoogleStereoInfoParser = nullptr;
    }

    // delete operator
    if (pStandardMetaOperator != nullptr) {
        delete pStandardMetaOperator;
        pStandardMetaOperator = nullptr;
    }
    if (pExtendedMetaOperator != nullptr) {
        delete pExtendedMetaOperator;
        pExtendedMetaOperator = nullptr;
    }
    if (pCustomizedMetaOperator != nullptr) {
        delete pCustomizedMetaOperator;
        pCustomizedMetaOperator = nullptr;
    }
}

void StereoCaptureInfoParser::read() {
    StereoLogD("<read>");
}

void StereoCaptureInfoParser::write() {
    StereoLogD("<write>");
    if (pStereoCaptureInfo == nullptr) {
        StereoLogD("<write> mStereoCaptureInfo is null!");
        return;
    }
    dumpJsonBuffer("write");
    writeInfo();
    pStereoBufferInfoParser = new StereoBufferInfoParser(
        pCustomizedMetaOperator, pStereoBufferInfo);
    pStereoBufferInfoParser->write();
    pStereoDepthInfoParser = new StereoDepthInfoParser(
        pStandardMetaOperator, nullptr, pCustomizedMetaOperator, pStereoDepthInfo);
    pStereoDepthInfoParser->write();
    pStereoConfigInfoParser = new StereoConfigInfoParser(
        pStandardMetaOperator, nullptr, pCustomizedMetaOperator, pStereoConfigInfo);
    pStereoConfigInfoParser->write();
    if (Utils::ENABLE_GDEPTH) {
        pGoogleStereoInfoParser = new GoogleStereoInfoParser(
            pStandardMetaOperator, pExtendedMetaOperator, pGoogleStereoInfo);
        pGoogleStereoInfoParser->write();
    }
}

SerializedInfo* StereoCaptureInfoParser::serialize() {
    StereoLogD("<serialize>");
    SerializedInfo *info = new SerializedInfo();

    if (pStandardMetaOperator != nullptr) {
        BufferMapPtr standardData = pStandardMetaOperator->serialize();
        auto iter = standardData->find(XMP_KEY);
        if (iter != standardData->end()) {
            info->standardXmpBuf = iter->second;
        } else {
            StereoLogW("can not find property from standard data");
        }
    }
    if (pExtendedMetaOperator != nullptr) {
        BufferMapPtr extendedData = pExtendedMetaOperator->serialize();
        auto iter = extendedData->find(XMP_KEY);
        if (iter != extendedData->end()) {
            info->extendedXmpBuf = iter->second;
        } else {
            StereoLogW("can not find property from extended data");
        }
    }

    if (pCustomizedMetaOperator != nullptr) {
        info->customizedBufMap = pCustomizedMetaOperator->serialize();
    }
    return info;
}

void StereoCaptureInfoParser::writeInfo() {
    StereoLogD("<writeInfo>");
    pStereoBufferInfo->debugDir = pStereoCaptureInfo->debugDir;
    pStereoBufferInfo->jpsBuffer = pStereoCaptureInfo->jpsBuffer;

    pStereoConfigInfo->debugDir = pStereoCaptureInfo->debugDir;
    pStereoConfigInfo->ldcBuffer = pStereoCaptureInfo->ldc;
    if (!Utils::ENABLE_GDEPTH) {
        pStereoConfigInfo->clearImage = pStereoCaptureInfo->clearImage;
    }

    // we don't use configBuffer now, so move depthBuffer copy statement before check configBuffer.isValid()
    pStereoDepthInfo->depthBuffer = pStereoCaptureInfo->depthBuffer;

    if (!pStereoCaptureInfo->configBuffer.isValid()) {
        return;
    }

    // use json parser to parse StereoCaptureInfo.configBuffer
    StereoInfoJsonParser stereoInfoJsonParser(pStereoCaptureInfo->configBuffer);
    stereoInfoJsonParser.getMaskBuffer(pStereoBufferInfo->maskBuffer);
    pStereoConfigInfo->jpsWidth = stereoInfoJsonParser.getJpsWidth();
    pStereoConfigInfo->jpsHeight = stereoInfoJsonParser.getJpsHeight();
    pStereoConfigInfo->maskWidth = stereoInfoJsonParser.getMaskWidth();
    pStereoConfigInfo->maskHeight = stereoInfoJsonParser.getMaskHeight();
    pStereoConfigInfo->posX = stereoInfoJsonParser.getPosX();
    pStereoConfigInfo->posY = stereoInfoJsonParser.getPosY();
    pStereoConfigInfo->viewWidth = stereoInfoJsonParser.getViewWidth();
    pStereoConfigInfo->viewHeight = stereoInfoJsonParser.getViewHeight();
    pStereoConfigInfo->imageOrientation = stereoInfoJsonParser.getOrientation();
    pStereoConfigInfo->depthOrientation = stereoInfoJsonParser.getDepthRotation();
    pStereoConfigInfo->mainCamPos = stereoInfoJsonParser.getMainCamPos();
    pStereoConfigInfo->touchCoordX1st = stereoInfoJsonParser.getTouchCoordX1st();
    pStereoConfigInfo->touchCoordY1st = stereoInfoJsonParser.getTouchCoordY1st();
    pStereoConfigInfo->faceCount = stereoInfoJsonParser.getFaceRectCount();
    pStereoConfigInfo->fdInfoArray = prepareFdInfo(&stereoInfoJsonParser, pStereoConfigInfo->faceCount);
    FocusInfo *focusInfo = new FocusInfo();
    focusInfo->focusType = stereoInfoJsonParser.getFocusType();
    focusInfo->focusLeft = stereoInfoJsonParser.getFocusLeft();
    focusInfo->focusTop = stereoInfoJsonParser.getFocusTop();
    focusInfo->focusRight = stereoInfoJsonParser.getFocusRight();
    focusInfo->focusBottom = stereoInfoJsonParser.getFocusBottom();
    pStereoConfigInfo->focusInfo = focusInfo;
    pStereoConfigInfo->dofLevel = stereoInfoJsonParser.getDof();
    pStereoConfigInfo->convOffset = stereoInfoJsonParser.getConvOffset();
    pStereoConfigInfo->ldcWidth = stereoInfoJsonParser.getLdcWidth();
    pStereoConfigInfo->ldcHeight = stereoInfoJsonParser.getLdcHeight();

    pStereoConfigInfo->isFace = stereoInfoJsonParser.getFaceFlag();
    pStereoConfigInfo->faceRatio = (float) stereoInfoJsonParser.getFaceRatio();
    pStereoConfigInfo->curDac = stereoInfoJsonParser.getCurDac();
    pStereoConfigInfo->minDac = stereoInfoJsonParser.getMinDac();
    pStereoConfigInfo->maxDac = stereoInfoJsonParser.getMaxDac();

    // add for kibo+
    pStereoDepthInfo->debugDir = pStereoCaptureInfo->debugDir;
    pStereoDepthInfo->metaBufferWidth = stereoInfoJsonParser.getMetaBufferWidth();
    pStereoDepthInfo->metaBufferHeight = stereoInfoJsonParser.getMetaBufferHeight();
    pStereoDepthInfo->depthBufferWidth = stereoInfoJsonParser.getDepthBufferWidth();
    pStereoDepthInfo->depthBufferHeight = stereoInfoJsonParser.getDepthBufferHeight();
    // pStereoDepthInfo->depthBuffer = pStereoCaptureInfo->depthBuffer; // move to position before check configBuffer.isValid()
    pStereoDepthInfo->debugBuffer = pStereoCaptureInfo->debugBuffer;

    // add for GDepth
    // the clear image and depth map will got from camera app
    // other simple value will set default value temporarily
    if (Utils::ENABLE_GDEPTH) {
        pGoogleStereoInfo->clearImage = pStereoCaptureInfo->clearImage;
        // encode depth map to PNG format
        if (pStereoCaptureInfo->depthMap.isValid()) {
             Utils::encodePng(pStereoCaptureInfo->depthMap,
                    stereoInfoJsonParser.getGoogleDepthWidth(),
                    stereoInfoJsonParser.getGoogleDepthHeight(),
                    pGoogleStereoInfo->depthMap);
        }
        pGoogleStereoInfo->focusBlurAtInfinity = 0.055234075;
        pGoogleStereoInfo->focusFocalDistance = 23.359299;
        pGoogleStereoInfo->focusFocalPointX = 0.5416667;
        pGoogleStereoInfo->focusFocalPointY = 0.4586397;
        pGoogleStereoInfo->imageMime = "image/jpeg";
        pGoogleStereoInfo->depthFormat = "RangeInverse";
        pGoogleStereoInfo->depthNear = 17.3202400207519;
        pGoogleStereoInfo->depthFar = 111.881546020507;
        pGoogleStereoInfo->depthMime = "image/jpeg";
    }
}

StereoVector<FaceDetectionInfo*>* StereoCaptureInfoParser::prepareFdInfo(
        StereoInfoJsonParser *parser, int faceCount) {
    if (faceCount <= 0 || parser == nullptr) {
        StereoLogW("<prepareFdInfo> invalid params!!");
        return nullptr;
    }
    StereoVector<FaceDetectionInfo*>* faceInfo = new StereoVector<FaceDetectionInfo*>();
    faceInfo->reserve(faceCount);
    Rect *fd = nullptr;
    int rip = 0;
    for (int i = 0; i < faceCount; i++) {
        fd = parser->getFaceRect(i);
        rip = parser->getFaceRip(i);
        if (fd != nullptr) {
            FaceDetectionInfo *info = new FaceDetectionInfo(
                fd->left, fd->top, fd->right, fd->bottom, rip);
            faceInfo->push_back(info);
            StereoLogD("<prepareFdInfo> faceInfo-%d: %s", i, info->toString().c_str());
        }
        delete fd;
    }
    return faceInfo;
}

void StereoCaptureInfoParser::dumpJsonBuffer(StereoString suffix) {
    if (!Utils::ENABLE_BUFFER_DUMP) {
        return;
    }
    StereoString dumpFolder;
    StereoString dumpPath;
    dumpFolder.append(DUMP_FILE_FOLDER).append("/")
        .append(pStereoCaptureInfo->debugDir).append("/");

    if (pStereoCaptureInfo->configBuffer.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoCaptureInfo_jsonConfigBuffer_")
            .append(suffix).append(".txt"),
            pStereoCaptureInfo->configBuffer);
    } else {
        StereoLogD("<dumpJsonBuffer> configBuffer is null!");
    }

}
