//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/StereoInfo"
#include "StereoLog.h"

#include "StereoConfigInfo.h"
#include "SerializedInfo.h"
#include "StereoBufferInfo.h"
#include "StereoDepthInfo.h"
#include "SegmentMaskInfo.h"
#include "GoogleStereoInfo.h"
#include "StereoCaptureInfo.h"

using namespace stereo;

StereoConfigInfo::~StereoConfigInfo() {
    StereoLogD("~StereoConfigInfo");
    if (focusInfo != nullptr) {
        delete focusInfo;
        focusInfo = nullptr;
    }
    if (fdInfoArray != nullptr) {
        for (int i = 0; i < fdInfoArray->size(); i++) {
            if ((*fdInfoArray)[i] != nullptr) {
                delete (*fdInfoArray)[i];
                (*fdInfoArray)[i] = nullptr;
            }
        }
        fdInfoArray->clear();
        delete fdInfoArray;
        fdInfoArray = nullptr;
    }
}

StereoCaptureInfo::~StereoCaptureInfo() {
    StereoLogD("~StereoCaptureInfo");
}

StereoDepthInfo::~StereoDepthInfo() {
    StereoLogD("~StereoDepthInfo");
}
SegmentMaskInfo::~SegmentMaskInfo() {
    StereoLogD("~SegmentMaskInfo");
}

StereoBufferInfo::~StereoBufferInfo() {
    StereoLogD("~StereoBufferInfo");
}

GoogleStereoInfo::~GoogleStereoInfo() {
    StereoLogD("~GoogleStereoInfo");
}