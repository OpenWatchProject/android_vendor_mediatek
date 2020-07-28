#ifndef META_OPERATOR_FACTORY_H
#define META_OPERATOR_FACTORY_H

#include "MetaOperator.h"
#include "BufferManager.h"

extern const stereo::StereoBuffer_t INVALID_BUFFER;

namespace stereo {

class MetaOperatorFactory {
public:
    static IMetaOperator* getOperatorInstance(int metaType,
        const StereoBuffer_t &xmpBuffer, BufferMapPtr customizedBuffer);
};

}

#endif