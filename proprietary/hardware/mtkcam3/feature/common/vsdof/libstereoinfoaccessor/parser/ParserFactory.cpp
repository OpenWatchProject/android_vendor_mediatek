//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/ParserFactory"

#include "ParserFactory.h"
#include "StereoLog.h"
#include "GoogleStereoInfo.h"
#include "SegmentMaskInfo.h"
#include "StereoBufferInfo.h"
#include "StereoConfigInfo.h"
#include "StereoDepthInfo.h"
#include "StereoCaptureInfo.h"
#include "GoogleStereoInfoParser.h"
#include "SegmentMaskInfoParser.h"
#include "StereoBufferInfoParser.h"
#include "StereoConfigInfoParser.h"
#include "StereoDepthInfoParser.h"
#include "StereoCaptureInfoParser.h"

using namespace stereo;

IParser* ParserFactory::getParserInstance(
        int infoType, void *info, const StereoBuffer_t &standardBuffer,
        const StereoBuffer_t &extendedBuffer, BufferMapPtr customizedBuffer) {
    StereoLogD("<getParserInstance> type = %d", infoType);
    switch (infoType) {
        case GOOGLE_STEREO_INFO:
            return new GoogleStereoInfoParser(
                standardBuffer, extendedBuffer, (GoogleStereoInfo*)info);
        case SEGMENT_MASK_INFO:
            return new SegmentMaskInfoParser(
                standardBuffer, customizedBuffer, (SegmentMaskInfo*)info);
        case STEREO_BUFFER_INFO:
            return new StereoBufferInfoParser(
                customizedBuffer, (StereoBufferInfo*)info);
        case STEREO_CONFIG_INFO:
            return new StereoConfigInfoParser(
                standardBuffer, extendedBuffer, customizedBuffer, (StereoConfigInfo*)info);
        case STEREO_DEPTH_INFO:
            return new StereoDepthInfoParser(
                standardBuffer, extendedBuffer, customizedBuffer, (StereoDepthInfo*)info);
        case STEREO_CAPTURE_INFO:
            return new StereoCaptureInfoParser(
                standardBuffer, extendedBuffer, customizedBuffer, (StereoCaptureInfo*)info);
        default:
            return nullptr;
    }
}