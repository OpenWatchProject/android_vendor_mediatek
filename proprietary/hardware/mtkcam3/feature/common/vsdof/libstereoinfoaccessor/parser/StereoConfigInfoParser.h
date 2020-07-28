#ifndef STEREO_CONFIG_INFO_PARSER_H
#define STEREO_CONFIG_INFO_PARSER_H

#include "IParser.h"
#include "PackUtils.h"
#include "MetaOperator.h"
#include "DataItem.h"
#include "StereoConfigInfo.h"

namespace stereo {

class StereoConfigInfoParser : public IParser {

public:

    /**
     * StereoConfigInfoParser Constructor.
     * @param standardBuffer
     *  use standardMeta to get or set standard XMP info value
     * @param extendedBuffer
     *  use extendedMeta to get or set extended XMP info value
     * @param info
     *  StereoConfigInfo struct for set or get stereo config info
     */
    StereoConfigInfoParser(
        const StereoBuffer_t &standardBuffer, const StereoBuffer_t &extendedBuffer,
        BufferMapPtr customizedBuffer, StereoConfigInfo *info);
    /**
     * StereoConfigInfoParser Constructor.
     * @param standardMetaOperator
     *  use standardMeta to get or set standard XMP info value
     * @param extendedMetaOperator
     *  use extendedMeta to get or set extended XMP info value
     * @param info
     *  StereoConfigInfoParser struct for set or get config info
     */
    StereoConfigInfoParser(IMetaOperator *standardMetaOperator,
        IMetaOperator *extendedMetaOperator, IMetaOperator *customizedMetaOperator,
        StereoConfigInfo *info);
    virtual ~StereoConfigInfoParser();
    virtual void read();
    virtual void write();
    virtual SerializedInfo* serialize();

private:
    const int SUPPORT_FACE_COUNT = 3;
    const StereoString NS_GIMAGE = "http://ns.google.com/photos/1.0/image/";
    const StereoString NS_STEREO = "http://ns.mediatek.com/refocus/jpsconfig/";
    const StereoString NS_FACE_FIELD = "FD";
    const StereoString NS_FOCUS_FIELD = "FOC";
    const StereoString PRIFIX_GIMAGE = "GImage";
    const StereoString PRIFIX_STEREO = "MRefocus";
    const StereoString PRIFIX_FACE = "FD";
    const StereoString PRIFIX_FOCUS = "FOC";
    const StereoString ATTRIBUTE_JPS_WIDTH = "JpsWidth";
    const StereoString ATTRIBUTE_JPS_HEIGHT = "JpsHeight";
    const StereoString ATTRIBUTE_MASK_WIDTH = "MaskWidth";
    const StereoString ATTRIBUTE_MASK_HEIGHT = "MaskHeight";
    const StereoString ATTRIBUTE_POS_X = "PosX";
    const StereoString ATTRIBUTE_POS_Y = "PosY";
    const StereoString ATTRIBUTE_VIEW_WIDTH = "ViewWidth";
    const StereoString ATTRIBUTE_VIEW_HEIGHT = "ViewHeight";
    const StereoString ATTRIBUTE_ORIENTATION = "Orientation";
    const StereoString ATTRIBUTE_DEPTH_ROTATION = "DepthRotation";
    const StereoString ATTRIBUTE_MAIN_CAM_POS = "MainCamPos";
    const StereoString ATTRIBUTE_TOUCH_COORDX_1ST = "TouchCoordX1st";
    const StereoString ATTRIBUTE_TOUCH_COORDY_1ST = "TouchCoordY1st";
    const StereoString ATTRIBUTE_FACE_COUNT = "FaceCount";
    const StereoString ATTRIBUTE_FOCUSINFO_STRUCT_NAME = "FocusInfo";
    const StereoString ATTRIBUTE_FOCUSINFO_LEFT = "FocusLeft";
    const StereoString ATTRIBUTE_FOCUSINFO_TOP = "FocusTop";
    const StereoString ATTRIBUTE_FOCUSINFO_RIGHT = "FocusRight";
    const StereoString ATTRIBUTE_FOCUSINFO_BOTTOM = "FocusBottom";
    const StereoString ATTRIBUTE_FOCUSINFO_TYPE = "FocusType";

    const StereoString ATTRIBUTE_DOF_LEVEL = "DOF";
    const StereoString ATTRIBUTE_CONV_OFFSET = "ConvOffset";
    const StereoString ATTRIBUTE_LDC_WIDTH = "LdcWidth";
    const StereoString ATTRIBUTE_LDC_HEIGHT = "LdcHeight";
    const StereoString ATTRIBUTE_LDC_BUFFER_IN_APP15 = PackUtils::TYPE_LDC_DATA;
    const StereoString ATTRIBUTE_CLEAR_IMAGE_IN_APP15 = PackUtils::TYPE_CLEAR_IMAGE;
    const StereoString ATTRIBUTE_LDC_BUFFER_IN_APP1 = "LDC";
    const StereoString ATTRIBUTE_CLEAR_IMAGE_IN_APP1 = "Data";
    const StereoString ATTRIBUTE_FACE_STRUCT_NAME = "FDInfo";
    const StereoString ATTRIBUTE_FACE_LEFT = "FaceLeft";
    const StereoString ATTRIBUTE_FACE_TOP = "FaceTop";
    const StereoString ATTRIBUTE_FACE_RIGHT = "FaceRight";
    const StereoString ATTRIBUTE_FACE_BOTTOM = "FaceBottom";
    const StereoString ATTRIBUTE_FACE_RIP = "FaceRip";

    const StereoString ATTRIBUTE_FACE_FLAG = "IsFace";
    const StereoString ATTRIBUTE_FACE_RATIO = "FaceRatio";
    const StereoString ATTRIBUTE_CUR_DAC = "CurDac";
    const StereoString ATTRIBUTE_MIN_DAC = "MinDac";
    const StereoString ATTRIBUTE_MAX_DAC = "MacDac";

    IMetaOperator *pStandardMetaOperator = nullptr;
    IMetaOperator *pExtendedMetaOperator = nullptr;
    IMetaOperator *pCustomizedMetaOperator = nullptr;

    DataCollections *pStandardDataCollections = nullptr;
    DataCollections *pExtendardDataCollections = nullptr;
    DataCollections *pCustomizedDataCollections = nullptr;

    StereoVector<SimpleItem*> *pListOfSimpleValue = nullptr;
    StereoVector<BufferItem*> *pListOfBufferItem = nullptr;
    StereoVector<StructItem*> *pListOfStructItem = nullptr;
    StereoVector<BufferItem*> *pListOfCustomDataItem = nullptr;

    StereoConfigInfo *pStereoConfigInfo = nullptr;

    SimpleItem* getSimpleValueInstance();
    StructItem* getStructItemInstance(const StereoString &fieldNS, const StereoString &fieldPrefix);
    BufferItem* getBufferItemInstance();

    void initSimpleValue();
    void initBufferItem();
    void initStructItem();
    void initCustDataItem();

    void readSimpleValue();
    void readBufferItem();
    void readStructItem();
    void readCustDataItem();

    void writeSimpleValue();
    void writeStructItem();
    void writeCustDataItem();

    void dumpValuesAndBuffers(StereoString suffix);
};

}

#endif
