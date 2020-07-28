#ifndef SEGMENT_MASK_INFO_PARSER_H
#define SEGMENT_MASK_INFO_PARSER_H

#include "IParser.h"
#include "PackUtils.h"
#include "MetaOperator.h"
#include "DataItem.h"
#include "SegmentMaskInfo.h"

namespace stereo {

class SegmentMaskInfoParser : public IParser {

public:

    /**
     * SegmentMaskInfoParser Constructor.
     * @param standardBuffer
     *  use standardMeta to get or set standard XMP info value
     * @param customBuffer
     *  use custMeta to get or set customer XMP info value
     * @param info
     *  SegmentMaskInfo struct for set or get segment and mask info
     */
    SegmentMaskInfoParser(
        const StereoBuffer_t &standardBuffer, BufferMapPtr customizedBuffer,
        SegmentMaskInfo *info);
    virtual ~SegmentMaskInfoParser();
    virtual void read();
    virtual void write();
    virtual SerializedInfo* serialize();

private:
    const StereoString NS_STEREO = "http://ns.mediatek.com/segment/";
    const StereoString PRIFIX_STEREO = "MSegment";
    const StereoString ATTRIBUTE_SEGMENT_X = "SegmentX";
    const StereoString ATTRIBUTE_SEGMENT_Y = "SegmentY";
    const StereoString ATTRIBUTE_SEGMENT_LEFT = "SegmentLeft";
    const StereoString ATTRIBUTE_SEGMENT_TOP = "SegmentTop";
    const StereoString ATTRIBUTE_SEGMENT_RIGHT = "SegmentRight";
    const StereoString ATTRIBUTE_SEGMENT_BOTTOM = "SegmentBottom";
    const StereoString ATTRIBUTE_SEGMENT_MASK_BUFFER = PackUtils::TYPE_JPS_MASK;
    const StereoString ATTRIBUTE_MASK_WIDTH = "SegmentMaskWidth";
    const StereoString ATTRIBUTE_MASK_HEIGHT = "SegmentMaskHeight";

    IMetaOperator *pStandardMetaOperator = nullptr;
    IMetaOperator *pCustomizedMetaOperator = nullptr;

    DataCollections *pStandardDataCollections = nullptr;
    DataCollections *pCustomizedDataCollections = nullptr;

    StereoVector<SimpleItem*> *pListOfSimpleValue = nullptr;
    StereoVector<BufferItem*> *pListOfCustomDataItem = nullptr;

    SegmentMaskInfo *pSegmentMaskInfo = nullptr;

    SimpleItem* getSimpleValueInstance();

    void initSimpleValue();
    void initCustDataItem();

    void readSimpleValue();
    void readCustDataItem();

    void writeSimpleValue();
    void writeCustDataItem();

    void dumpValuesAndBuffers(StereoString suffix);
};

}

#endif