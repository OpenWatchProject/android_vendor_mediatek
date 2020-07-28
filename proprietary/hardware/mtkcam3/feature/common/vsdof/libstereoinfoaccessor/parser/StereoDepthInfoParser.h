#ifndef STEREO_DEPTH_INFO_PARSER_H
#define STEREO_DEPTH_INFO_PARSER_H

#include "IParser.h"
#include "PackUtils.h"
#include "MetaOperator.h"
#include "DataItem.h"
#include "StereoDepthInfo.h"

namespace stereo {

class StereoDepthInfoParser : public IParser {

public:

    /**
     * StereoDepthInfoParser Constructor.
     * @param standardBuffer
     *  use standardMeta to get or set standard XMP info value
     * @param extendedBuffer
     *  use extendedMeta to get or set extended XMP info value
     * @param customizedBuffer
     *  use custMeta to get or set customer XMP info value
     * @param info
     *  StereoDepthInfo struct for set or get stereo depth info
     */
    StereoDepthInfoParser(
        const StereoBuffer_t &standardBuffer, const StereoBuffer_t &extendedBuffer,
        BufferMapPtr customizedBuffer, StereoDepthInfo *info);
    /**
     * StereoDepthInfoParser Constructor.
     * @param standardMetaOperator standard meta operator
     * @param extendedMetaOperator extended meta operator
     * @param customizedMetaOperator cust meta operator
     * @param info StereoDepthInfo
     */
    StereoDepthInfoParser(IMetaOperator *standardMetaOperator,
        IMetaOperator *extendedMetaOperator, IMetaOperator *customizedMetaOperator,
        StereoDepthInfo *info);
    virtual ~StereoDepthInfoParser();
    virtual void read();
    virtual void write();
    virtual SerializedInfo* serialize();

private:
    const StereoString NS_GDEPTH = "http://ns.google.com/photos/1.0/depthmap/";
    const StereoString NS_STEREO = "http://ns.mediatek.com/refocus/jpsconfig/";
    const StereoString PRIFIX_GDEPTH = "GDepth";
    const StereoString PRIFIX_STEREO = "MRefocus";
    const StereoString ATTRIBUTE_META_BUFFER_WIDTH = "MetaBufferWidth";
    const StereoString ATTRIBUTE_META_BUFFER_HEIGHT = "MetaBufferHeight";
    const StereoString ATTRIBUTE_TOUCH_COORDX_LAST = "TouchCoordXLast";
    const StereoString ATTRIBUTE_TOUCH_COORDY_LAST = "TouchCoordYLast";
    const StereoString ATTRIBUTE_DEPTH_OF_FIELD_LAST = "DepthOfFieldLast";
    const StereoString ATTRIBUTE_DEPTH_BUFFER_WIDTH = "DepthBufferWidth";
    const StereoString ATTRIBUTE_DEPTH_BUFFER_HEIGHT = "DepthBufferHeight";
    const StereoString ATTRIBUTE_DEPTH_MAP_WIDTH = "XmpDepthWidth";
    const StereoString ATTRIBUTE_DEPTH_MAP_HEIGHT = "XmpDepthHeight";
    const StereoString ATTRIBUTE_DEPTH_BUFFER = PackUtils::TYPE_DEPTH_DATA;
    const StereoString ATTRIBUTE_DEBUG_BUFFER = PackUtils::TYPE_DEBUG_BUFFER;
    const StereoString ATTRIBUTE_DEPTH_MAP = "Data";

    IMetaOperator *pStandardMetaOperator = nullptr;
    IMetaOperator *pExtendedMetaOperator = nullptr;
    IMetaOperator *pCustomizedMetaOperator = nullptr;

    DataCollections *pStandardDataCollections = nullptr;
    DataCollections *pExtendardDataCollections = nullptr;
    DataCollections *pCustomizedDataCollections = nullptr;

    StereoVector<SimpleItem*> *pListOfSimpleValue = nullptr;
    StereoVector<BufferItem*> *pListOfBufferItem = nullptr;
    StereoVector<BufferItem*> *pListOfCustomDataItem = nullptr;

    StereoDepthInfo *pStereoDepthInfo = nullptr;

    SimpleItem* getSimpleValueInstance();

    void initSimpleValue();
    void initBufferItem();
    void initCustDataItem();

    void readSimpleValue();
    void readBufferItem();
    void readCustDataItem();

    void writeSimpleValue();
    void writeBufferItem();
    void writeCustDataItem();

    void dumpValuesAndBuffers(StereoString suffix);
};

}

#endif