#ifndef STEREO_CAPTURE_INFO_PARSER_H
#define STEREO_CAPTURE_INFO_PARSER_H

#include "IParser.h"
#include "PackUtils.h"
#include "MetaOperator.h"
#include "DataItem.h"
#include "StereoConfigInfoParser.h"
#include "StereoBufferInfoParser.h"
#include "StereoDepthInfoParser.h"
#include "StereoInfoJsonParser.h"
#include "GoogleStereoInfoParser.h"
#include "StereoCaptureInfo.h"

namespace stereo {

class StereoCaptureInfoParser : public IParser {

public:

    /**
     * StereoCaptureInfoParser Constructor.
     * @param standardBuffer
     *            use standardMeta to get or set standard XMP info value
     * @param extendedBuffer
     *            use extendedMeta to get or set extended XMP info value
     * @param customizedBuffer
     *            use custMeta to get or set customer XMP info value
     * @param info
     *            StereoCaptureInfo struct for set or get capture info
     */
    StereoCaptureInfoParser(
        const StereoBuffer_t &standardBuffer, const StereoBuffer_t &extendedBuffer,
        BufferMapPtr customizedBuffer, StereoCaptureInfo *info);
    virtual ~StereoCaptureInfoParser();
    virtual void read();
    virtual void write();
    virtual SerializedInfo* serialize();

private:
    IMetaOperator *pStandardMetaOperator = nullptr;
    IMetaOperator *pExtendedMetaOperator = nullptr;
    IMetaOperator *pCustomizedMetaOperator = nullptr;

    StereoCaptureInfo *pStereoCaptureInfo = nullptr;
    StereoBufferInfo *pStereoBufferInfo = nullptr;
    StereoConfigInfo *pStereoConfigInfo = nullptr;
    StereoDepthInfo *pStereoDepthInfo = nullptr;
    GoogleStereoInfo *pGoogleStereoInfo = nullptr;

    StereoBufferInfoParser *pStereoBufferInfoParser = nullptr;
    StereoConfigInfoParser *pStereoConfigInfoParser = nullptr;
    StereoDepthInfoParser *pStereoDepthInfoParser = nullptr;
    GoogleStereoInfoParser *pGoogleStereoInfoParser = nullptr;

    void writeInfo();
    StereoVector<FaceDetectionInfo*>* prepareFdInfo(
        StereoInfoJsonParser *parser, int faceCount);

    void dumpJsonBuffer(StereoString suffix);
};

}

#endif
