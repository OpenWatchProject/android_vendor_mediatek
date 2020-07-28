#ifndef STEREO_BUFFER_INFO_PARSER_H
#define STEREO_BUFFER_INFO_PARSER_H

#include "IParser.h"
#include "PackUtils.h"
#include "MetaOperator.h"
#include "DataItem.h"
#include "StereoBufferInfo.h"

namespace stereo {

class StereoBufferInfoParser : public IParser {

public:

    /**
     * StereoBufferInfoParser Constructor.
     * @param customizedBuffer
     *  use custMeta to get or set customer XMP info value
     * @param info
     *  StereoBufferInfo struct for set or get stereo buffer info
     */
    StereoBufferInfoParser(
        BufferMapPtr customizedBuffer, StereoBufferInfo *info);
    /**
     * StereoBufferInfoParser Constructor.
     * @param customizedMetaOperator
     *            use custMeta to get or set customer XMP info value
     * @param info
     *            StereoBufferInfo struct for set or get stereo buffer info
     */
    StereoBufferInfoParser(IMetaOperator *customizedMetaOperator,
            StereoBufferInfo *info);
    virtual ~StereoBufferInfoParser();
    virtual void read();
    virtual void write();
    virtual SerializedInfo* serialize();

private:
    const StereoString ATTRIBUTE_SEGMENT_MASK_BUFFER = PackUtils::TYPE_JPS_MASK;
    const StereoString ATTRIBUTE_JPS_BUFFER = PackUtils::TYPE_JPS_DATA;
    IMetaOperator *pCustomizedMetaOperator = nullptr;
    DataCollections *pCustomizedDataCollections = nullptr;
    StereoVector<BufferItem*> *pListOfCustomDataItem = nullptr;
    StereoBufferInfo *pStereoBufferInfo = nullptr;

    void initCustDataItem();
    void readCustDataItem();
    void writeCustDataItem();

    void dumpValuesAndBuffers(StereoString suffix);
};

}

#endif