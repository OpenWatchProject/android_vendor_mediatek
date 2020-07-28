#ifndef GOOGLE_STEREO_INFO_PARSER_H
#define GOOGLE_STEREO_INFO_PARSER_H

#include "IParser.h"
#include "PackUtils.h"
#include "MetaOperator.h"
#include "DataItem.h"
#include "GoogleStereoInfo.h"

namespace stereo {

class GoogleStereoInfoParser : public IParser {

public:

    /**
     * GoogleStereoInfoParser Constructor.
     * @param standardBuffer
     *  use standardMeta to get or set standard XMP info value
     * @param extendedBuffer
     *  use extendedMeta to get or set extended XMP info value
     * @param info
     *  GoogleStereoInfo struct for set or get Google info
     */
    GoogleStereoInfoParser(const StereoBuffer_t &standardBuffer,
        const StereoBuffer_t &extendedBuffer, GoogleStereoInfo *info);
    /**
     * GoogleStereoInfoParser Constructor.
     * @param standardMetaOperator
     *  use standardMeta to get or set standard XMP info value
     * @param extendedMetaOperator
     *  use extendedMeta to get or set extended XMP info value
     * @param info
     *  GoogleStereoInfo struct for set or get Google info
     */
    GoogleStereoInfoParser(IMetaOperator *standardMetaOperator,
        IMetaOperator *extendedMetaOperator, GoogleStereoInfo *info);
    virtual ~GoogleStereoInfoParser();
    virtual void read();
    virtual void write();
    virtual SerializedInfo* serialize();

private:
    const StereoString NS_GFOCUS = "http://ns.google.com/photos/1.0/focus/";
    const StereoString NS_GIMAGE = "http://ns.google.com/photos/1.0/image/";
    const StereoString NS_GDEPTH = "http://ns.google.com/photos/1.0/depthmap/";
    const StereoString PRIFIX_GFOCUS = "GFocus";
    const StereoString PRIFIX_GIMAGE = "GImage";
    const StereoString PRIFIX_GDEPTH = "GDepth";
    const StereoString ATTRIBUTE_GFOCUS_BLUR_INFINITY = "BlurAtInfinity";
    const StereoString ATTRIBUTE_GFOCUS_FOCALDISTANCE = "FocalDistance";
    const StereoString ATTRIBUTE_GFOCUS_FOCALPOINTX = "FocalPointX";
    const StereoString ATTRIBUTE_GFOCUS_FOCALPOINTY = "FocalPointY";
    const StereoString ATTRIBUTE_GIMAGE_MIME = "Mime";
    const StereoString ATTRIBUTE_GDEPTH_FORMAT = "Format";
    const StereoString ATTRIBUTE_GDEPTH_NEAR = "Near";
    const StereoString ATTRIBUTE_GDEPTH_FAR = "Far";
    const StereoString ATTRIBUTE_GDEPTH_MIME = "Mime";
    const StereoString ATTRIBUTE_DEPTH_MAP = "Data";
    const StereoString ATTRIBUTE_CLEAR_IMAGE = "Data";

    IMetaOperator *pStandardMetaOperator = nullptr;
    IMetaOperator *pExtendedMetaOperator = nullptr;

    DataCollections *pStandardDataCollections = nullptr;
    DataCollections *pExtendardDataCollections = nullptr;

    StereoVector<SimpleItem*> *pListOfSimpleValue = nullptr;
    StereoVector<BufferItem*> *pListOfBufferItem = nullptr;

    GoogleStereoInfo *pGoogleStereoInfo = nullptr;

    SimpleItem* getSimpleValueInstance();
    BufferItem* getBufferItemInstance();

    void initSimpleValue();
    void initBufferItem();

    void readSimpleValue();
    void readBufferItem();

    void writeSimpleValue();
    void writeBufferItem();

    void dumpValuesAndBuffers(StereoString suffix);
};

}

#endif