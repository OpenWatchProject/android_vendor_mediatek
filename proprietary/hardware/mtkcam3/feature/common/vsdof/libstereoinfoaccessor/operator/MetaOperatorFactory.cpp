//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/MetaOperatorFactory"

#include "MetaOperatorFactory.h"
#include "StereoLog.h"

using namespace stereo;

const stereo::StereoBuffer_t INVALID_BUFFER;

IMetaOperator* MetaOperatorFactory::getOperatorInstance(int metaType,
        const StereoBuffer_t &xmpBuffer, BufferMapPtr customizedBuffer) {
    StereoLogD("getOperatorInstance, metaType = %d", metaType);
    IMetaOperator* metaOperator = nullptr;
    switch (metaType) {
        case XMP_META_OPERATOR:
            metaOperator = new XmpMetaOperator(xmpBuffer);
            break;
        case CUSTOMIZED_META_OPERATOR:
            metaOperator = new CustomizedMetaOperator(customizedBuffer);
            break;
    }
    return metaOperator;
}

