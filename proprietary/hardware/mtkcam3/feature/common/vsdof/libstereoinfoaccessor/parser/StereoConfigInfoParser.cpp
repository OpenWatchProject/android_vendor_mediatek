//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/StereoConfigInfoParser"

#include "StereoConfigInfoParser.h"
#include <stdlib.h>
#include "StereoLog.h"
#include "MetaOperatorFactory.h"
#include "Utils.h"

using namespace stereo;

// assign a initial size when define vector,
// it will reduce the count of resize and buffer copy
#define VECTOR_SIZE_SIMPLE_VALUE 23
#define VECTOR_SIZE_BUFFLE_ITEM 2
#define VECTOR_SIZE_CUST_DATA_ITEM 2

StereoConfigInfoParser::StereoConfigInfoParser(
        const StereoBuffer_t &standardBuffer, const StereoBuffer_t &extendedBuffer,
        BufferMapPtr customizedBuffer, StereoConfigInfo *info) {
    StereoLogD("<StereoConfigInfoParser> by buffer");

    instantiationWay = INSTANTIATION_BY_BUFFER;

    // new data collections
    pStandardDataCollections = new DataCollections();
    pStandardDataCollections->dest = DEST_TYPE_STANDARD_XMP;
    pExtendardDataCollections = new DataCollections();
    pExtendardDataCollections->dest = DEST_TYPE_EXTENDED_XMP;
    pCustomizedDataCollections = new DataCollections();

    pStereoConfigInfo = info;

    initSimpleValue();
    initStructItem();
    initBufferItem();
    initCustDataItem();

    pStandardDataCollections->listOfSimpleValue = pListOfSimpleValue;
    pStandardDataCollections->listOfStructItem = pListOfStructItem;
    pStandardMetaOperator = MetaOperatorFactory::getOperatorInstance(
        XMP_META_OPERATOR, standardBuffer, nullptr);
    pStandardMetaOperator->setData(pStandardDataCollections);

    pExtendardDataCollections->listOfBufferItem = pListOfBufferItem;
    pExtendedMetaOperator = MetaOperatorFactory::getOperatorInstance(
        XMP_META_OPERATOR, extendedBuffer, nullptr);
    pExtendedMetaOperator->setData(pExtendardDataCollections);

    pCustomizedDataCollections->listOfCustomDataItem = pListOfCustomDataItem;
    pCustomizedMetaOperator = MetaOperatorFactory::getOperatorInstance(
        CUSTOMIZED_META_OPERATOR, INVALID_BUFFER, customizedBuffer);
    pCustomizedMetaOperator->setData(pCustomizedDataCollections);
}

StereoConfigInfoParser::StereoConfigInfoParser(IMetaOperator *standardMetaOperator,
        IMetaOperator *extendedMetaOperator, IMetaOperator *customizedMetaOperator,
        StereoConfigInfo *info) {
    StereoLogD("<StereoConfigInfoParser> by operator");

    instantiationWay = INSTANTIATION_BY_OPERATOR;

    // new data collections
    pStandardDataCollections = new DataCollections();
    pStandardDataCollections->dest = DEST_TYPE_STANDARD_XMP;
    pExtendardDataCollections = new DataCollections();
    pExtendardDataCollections->dest = DEST_TYPE_EXTENDED_XMP;
    pCustomizedDataCollections = new DataCollections();

    pStereoConfigInfo = info;

    initSimpleValue();
    initStructItem();
    initBufferItem();
    initCustDataItem();

    pStandardDataCollections->listOfSimpleValue = pListOfSimpleValue;
    pStandardDataCollections->listOfStructItem = pListOfStructItem;
    pStandardMetaOperator = standardMetaOperator;
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->setData(pStandardDataCollections);
    }

    pExtendardDataCollections->listOfBufferItem = pListOfBufferItem;
    pExtendedMetaOperator = extendedMetaOperator;
    if (pExtendedMetaOperator != nullptr) {
        pExtendedMetaOperator->setData(pExtendardDataCollections);
    }

    pCustomizedDataCollections->listOfCustomDataItem = pListOfCustomDataItem;
    pCustomizedMetaOperator = customizedMetaOperator;
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->setData(pCustomizedDataCollections);
    }
}

StereoConfigInfoParser::~StereoConfigInfoParser() {
    StereoLogD("<~StereoConfigInfoParser>");
    // delete pListOfSimpleValue
    if (pListOfSimpleValue != nullptr) {
        for (auto iter = pListOfSimpleValue->begin();
                iter != pListOfSimpleValue->end(); iter++) {
            SimpleItem *pSimpleValue = *iter;
            if (pSimpleValue != nullptr) {
                delete pSimpleValue;
            }
        }
        pListOfSimpleValue->clear();
        delete pListOfSimpleValue;
    }

    // delete pListOfStructItem
    if (pListOfStructItem != nullptr) {
        for (auto iter = pListOfStructItem->begin();
                iter != pListOfStructItem->end(); iter++) {
            StructItem *pStructItem = *iter;
            if (pStructItem != nullptr) {
                delete pStructItem;
            }
        }
        pListOfStructItem->clear();
        delete pListOfStructItem;
    }

    // delete pStandardDataCollections
    if (pStandardDataCollections != nullptr) {
        delete pStandardDataCollections;
    }

    // delete pStandardMetaOperator
    if (instantiationWay == INSTANTIATION_BY_BUFFER && pStandardMetaOperator != nullptr) {
        delete pStandardMetaOperator;
    }

    // delete pListOfBufferItem
    if (pListOfBufferItem != nullptr) {
        for (auto iter = pListOfBufferItem->begin();
                iter != pListOfBufferItem->end(); iter++) {
            BufferItem *pBufferItem = *iter;
            if (pBufferItem != nullptr) {
                delete pBufferItem;
            }
        }
        pListOfBufferItem->clear();
        delete pListOfBufferItem;
    }

    // delete pExtendardDataCollections
    if (pExtendardDataCollections != nullptr) {
        delete pExtendardDataCollections;
    }

    // delete pExtendedMetaOperator
    if (instantiationWay == INSTANTIATION_BY_BUFFER && pExtendedMetaOperator != nullptr) {
        delete pExtendedMetaOperator;
    }

    // delete pListOfCustomDataItem
    if (pListOfCustomDataItem != nullptr) {
        for (auto iter = pListOfCustomDataItem->begin();
                iter != pListOfCustomDataItem->end(); iter++) {
            BufferItem *pBufferItem = *iter;
            if (pBufferItem != nullptr) {
                delete pBufferItem;
            }
        }
        pListOfCustomDataItem->clear();
        delete pListOfCustomDataItem;
    }

    // delete pCustomizedDataCollections
    if (pCustomizedDataCollections != nullptr) {
        delete pCustomizedDataCollections;
    }

    // delete pCustomizedMetaOperator
    if (instantiationWay == INSTANTIATION_BY_BUFFER && pCustomizedMetaOperator != nullptr) {
        delete pCustomizedMetaOperator;
    }
}

void StereoConfigInfoParser::read() {
    StereoLogD("<read>");
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->read();
    }
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->read();
    }
    if (pStereoConfigInfo == nullptr) {
        StereoLogW("<read> pStereoConfigInfo is null!");
        return;
    }
    readSimpleValue();
    readStructItem();
    readCustDataItem();

    // For backward compatibility, read clear image and ldc buffer
    // from APP1 if cannot read from APP15
    if ((!pStereoConfigInfo->clearImage.isValid() || !pStereoConfigInfo->ldcBuffer.isValid())
            && pExtendedMetaOperator != nullptr) {
        StereoLogD("read clear image from APP1");
        pExtendedMetaOperator->read();
        readBufferItem();
    }

    StereoLogD("<read> %s", pStereoConfigInfo->toString().c_str());
    dumpValuesAndBuffers("read");
}

void StereoConfigInfoParser::write() {
    StereoLogD("<write>");
    if (pStereoConfigInfo == nullptr) {
        StereoLogW("<write> pStereoConfigInfo is null!");
        return;
    }
    dumpValuesAndBuffers("write");
    writeSimpleValue();
    writeStructItem();
    writeCustDataItem();
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->write();
    }
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->write();
    }
}

SerializedInfo* StereoConfigInfoParser::serialize() {
    StereoLogD("<serialize>");
    SerializedInfo *info = new SerializedInfo();

    if (pStandardMetaOperator != nullptr) {
        BufferMapPtr standardData = pStandardMetaOperator->serialize();
        auto iter = standardData->find(XMP_KEY);
        if (iter != standardData->end()) {
            info->standardXmpBuf = iter->second;
        } else {
            StereoLogW("can not find property from standard data");
        }
    }

    if (pCustomizedMetaOperator != nullptr) {
        info->customizedBufMap = pCustomizedMetaOperator->serialize();
    }
    return info;
}

SimpleItem* StereoConfigInfoParser::getSimpleValueInstance() {
    SimpleItem* simpleValue = new SimpleItem();
    simpleValue->dest = DEST_TYPE_STANDARD_XMP;
    simpleValue->nameSpaceItem.dest = DEST_TYPE_STANDARD_XMP;
    simpleValue->nameSpaceItem.nameSpace = NS_STEREO;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_STEREO;
    return simpleValue;
}

StructItem* StereoConfigInfoParser::getStructItemInstance(
        const StereoString &fieldNS, const StereoString &fieldPrefix) {
    StructItem *structItem = new StructItem();
    structItem->dest = DEST_TYPE_STANDARD_XMP;
    structItem->structNameSpaceItem.nameSpace = NS_STEREO;
    structItem->structNameSpaceItem.nameSpacePrefix = PRIFIX_STEREO;
    structItem->fieldNameSpaceItem.nameSpace = fieldNS;
    structItem->fieldNameSpaceItem.nameSpacePrefix = fieldPrefix;
    return structItem;
}

BufferItem* StereoConfigInfoParser::getBufferItemInstance() {
    BufferItem *bufferItem = new BufferItem();
    bufferItem->dest = DEST_TYPE_EXTENDED_XMP;
    bufferItem->nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    return bufferItem;
}

void StereoConfigInfoParser::initSimpleValue() {
    StereoLogD("<initSimpleValue>");
    if (pListOfSimpleValue == nullptr) {
        pListOfSimpleValue = new StereoVector<SimpleItem*>();
        pListOfSimpleValue->reserve(VECTOR_SIZE_SIMPLE_VALUE);
    }

    SimpleItem *simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_JPS_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_JPS_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_MASK_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_MASK_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_POS_X;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_POS_Y;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_VIEW_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_VIEW_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_ORIENTATION;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_DEPTH_ROTATION;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_MAIN_CAM_POS;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_TOUCH_COORDX_1ST;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_TOUCH_COORDY_1ST;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_FACE_COUNT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_DOF_LEVEL;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_LDC_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_LDC_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_FACE_FLAG;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_FACE_RATIO;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_CUR_DAC;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_MIN_DAC;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_MAX_DAC;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_CONV_OFFSET;
    pListOfSimpleValue->push_back(simpleValue);
}

void StereoConfigInfoParser::initBufferItem() {
    StereoLogD("<initSimpleValue>");
    if (pListOfBufferItem == nullptr) {
        pListOfBufferItem = new StereoVector<BufferItem*>();
        pListOfBufferItem->reserve(VECTOR_SIZE_BUFFLE_ITEM);
    }

    BufferItem *bufferItem = getBufferItemInstance();
    bufferItem->nameSpaceItem.nameSpace = NS_STEREO;
    bufferItem->nameSpaceItem.nameSpacePrefix = PRIFIX_STEREO;
    bufferItem->name = ATTRIBUTE_LDC_BUFFER_IN_APP1;
    pListOfBufferItem->push_back(bufferItem);

    bufferItem = getBufferItemInstance();
    bufferItem->nameSpaceItem.nameSpace = NS_GIMAGE;
    bufferItem->nameSpaceItem.nameSpacePrefix = PRIFIX_GIMAGE;
    bufferItem->name = ATTRIBUTE_CLEAR_IMAGE_IN_APP1;
    pListOfBufferItem->push_back(bufferItem);
}

void StereoConfigInfoParser::initStructItem() {
    StereoLogD("<initStructItem>");
    if (pListOfStructItem == nullptr) {
        pListOfStructItem = new StereoVector<StructItem*>();
        pListOfStructItem->reserve(SUPPORT_FACE_COUNT * 5 + 5);
    }
    StructItem *structItem;
    std::stringstream ssStructName;
    for (int i = 0; i < SUPPORT_FACE_COUNT; i++) {
        structItem = getStructItemInstance(NS_FACE_FIELD, PRIFIX_FACE);
        ssStructName.str("");
        ssStructName << ATTRIBUTE_FACE_STRUCT_NAME << i;
        structItem->structName = ssStructName.str();
        structItem->fieldName = ATTRIBUTE_FACE_LEFT;
        pListOfStructItem->push_back(structItem);

        structItem = getStructItemInstance(NS_FACE_FIELD, PRIFIX_FACE);
        ssStructName.str("");
        ssStructName << ATTRIBUTE_FACE_STRUCT_NAME << i;
        structItem->structName = ssStructName.str();
        structItem->fieldName = ATTRIBUTE_FACE_TOP;
        pListOfStructItem->push_back(structItem);

        structItem = getStructItemInstance(NS_FACE_FIELD, PRIFIX_FACE);
        ssStructName.str("");
        ssStructName << ATTRIBUTE_FACE_STRUCT_NAME << i;
        structItem->structName = ssStructName.str();
        structItem->fieldName = ATTRIBUTE_FACE_RIGHT;
        pListOfStructItem->push_back(structItem);

        structItem = getStructItemInstance(NS_FACE_FIELD, PRIFIX_FACE);
        ssStructName.str("");
        ssStructName << ATTRIBUTE_FACE_STRUCT_NAME << i;
        structItem->structName = ssStructName.str();
        structItem->fieldName = ATTRIBUTE_FACE_BOTTOM;
        pListOfStructItem->push_back(structItem);

        structItem = getStructItemInstance(NS_FACE_FIELD, PRIFIX_FACE);
        ssStructName.str("");
        ssStructName << ATTRIBUTE_FACE_STRUCT_NAME << i;
        structItem->structName = ssStructName.str();
        structItem->fieldName = ATTRIBUTE_FACE_RIP;
        pListOfStructItem->push_back(structItem);
    }

    structItem = getStructItemInstance(NS_FOCUS_FIELD, PRIFIX_FOCUS);
    structItem->structName = ATTRIBUTE_FOCUSINFO_STRUCT_NAME;
    structItem->fieldName = ATTRIBUTE_FOCUSINFO_LEFT;
    pListOfStructItem->push_back(structItem);

    structItem = getStructItemInstance(NS_FOCUS_FIELD, PRIFIX_FOCUS);
    structItem->structName = ATTRIBUTE_FOCUSINFO_STRUCT_NAME;
    structItem->fieldName = ATTRIBUTE_FOCUSINFO_TOP;
    pListOfStructItem->push_back(structItem);

    structItem = getStructItemInstance(NS_FOCUS_FIELD, PRIFIX_FOCUS);
    structItem->structName = ATTRIBUTE_FOCUSINFO_STRUCT_NAME;
    structItem->fieldName = ATTRIBUTE_FOCUSINFO_RIGHT
;
    pListOfStructItem->push_back(structItem);

    structItem = getStructItemInstance(NS_FOCUS_FIELD, PRIFIX_FOCUS);
    structItem->structName = ATTRIBUTE_FOCUSINFO_STRUCT_NAME;
    structItem->fieldName = ATTRIBUTE_FOCUSINFO_BOTTOM;
    pListOfStructItem->push_back(structItem);

    structItem = getStructItemInstance(NS_FOCUS_FIELD, PRIFIX_FOCUS);
    structItem->structName = ATTRIBUTE_FOCUSINFO_STRUCT_NAME;
    structItem->fieldName = ATTRIBUTE_FOCUSINFO_TYPE;
    pListOfStructItem->push_back(structItem);
}

void StereoConfigInfoParser::initCustDataItem() {
    StereoLogD("<initCustDataItem>");
    if (pListOfCustomDataItem == nullptr) {
        pListOfCustomDataItem = new StereoVector<BufferItem*>();
        pListOfCustomDataItem->reserve(VECTOR_SIZE_CUST_DATA_ITEM);
    }
    BufferItem *custDataItem = new BufferItem();
    custDataItem->name = ATTRIBUTE_LDC_BUFFER_IN_APP15;
    pListOfCustomDataItem->push_back(custDataItem);
    custDataItem = new BufferItem();
    custDataItem->name = ATTRIBUTE_CLEAR_IMAGE_IN_APP15;
    pListOfCustomDataItem->push_back(custDataItem);
}

void StereoConfigInfoParser::readSimpleValue() {
    StereoLogD("<readSimpleValue>");
    if (pListOfSimpleValue == nullptr || pListOfSimpleValue->empty()) {
        StereoLogW("<readSimpleValue> pListOfSimpleValue is empty, cannot read");
        return;
    }
    for (auto iter = pListOfSimpleValue->begin();
            iter != pListOfSimpleValue->end(); iter++) {
        SimpleItem *pSimpleValue = *iter;
        if (pSimpleValue == nullptr || pSimpleValue->value.empty()) {
            continue;
        }
        if (ATTRIBUTE_JPS_WIDTH == pSimpleValue->name) {
            pStereoConfigInfo->jpsWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_JPS_HEIGHT == pSimpleValue->name) {
            pStereoConfigInfo->jpsHeight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_MASK_WIDTH == pSimpleValue->name) {
            pStereoConfigInfo->maskWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_MASK_HEIGHT == pSimpleValue->name) {
            pStereoConfigInfo->maskHeight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_POS_X == pSimpleValue->name) {
            pStereoConfigInfo->posX = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_POS_Y == pSimpleValue->name) {
            pStereoConfigInfo->posY = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_VIEW_WIDTH == pSimpleValue->name) {
            pStereoConfigInfo->viewWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_VIEW_HEIGHT == pSimpleValue->name) {
            pStereoConfigInfo->viewHeight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_ORIENTATION == pSimpleValue->name) {
            pStereoConfigInfo->imageOrientation = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_DEPTH_ROTATION == pSimpleValue->name) {
            pStereoConfigInfo->depthOrientation = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_MAIN_CAM_POS == pSimpleValue->name) {
            pStereoConfigInfo->mainCamPos = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_TOUCH_COORDX_1ST == pSimpleValue->name) {
            pStereoConfigInfo->touchCoordX1st = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_TOUCH_COORDY_1ST == pSimpleValue->name) {
            pStereoConfigInfo->touchCoordY1st = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_FACE_COUNT == pSimpleValue->name) {
            pStereoConfigInfo->faceCount = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_DOF_LEVEL == pSimpleValue->name) {
            pStereoConfigInfo->dofLevel = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_CONV_OFFSET == pSimpleValue->name) {
            pStereoConfigInfo->convOffset = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_LDC_WIDTH == pSimpleValue->name) {
            pStereoConfigInfo->ldcWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_LDC_HEIGHT == pSimpleValue->name) {
            pStereoConfigInfo->ldcHeight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_FACE_FLAG == pSimpleValue->name) {
            pStereoConfigInfo->isFace = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_FACE_RATIO == pSimpleValue->name) {
            pStereoConfigInfo->faceRatio = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_CUR_DAC == pSimpleValue->name) {
            pStereoConfigInfo->curDac = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_MIN_DAC == pSimpleValue->name) {
            pStereoConfigInfo->minDac = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_MAX_DAC == pSimpleValue->name) {
            pStereoConfigInfo->maxDac = atoi(pSimpleValue->value.c_str());
        }
    }
}

void StereoConfigInfoParser::readBufferItem() {
    StereoLogD("<readBufferItem>");
    if (pListOfBufferItem == nullptr || pListOfBufferItem->empty()) {
        StereoLogW("<readBufferItem> pListOfBufferItem is empty, cannot read");
        return;
    }
    for (auto iter = pListOfBufferItem->begin();
            iter != pListOfBufferItem->end(); iter++) {
        BufferItem *pBufferItem = *iter;
        if (pBufferItem == nullptr || !pBufferItem->value.isValid()) {
            continue;
        }
        // if already read ldc buffer and clear image buffer from APP15,
        // in other words, if ldc buffer or clear image is not null,
        // don't read from APP1 again
        if (ATTRIBUTE_LDC_BUFFER_IN_APP1 == pBufferItem->name
            && !pStereoConfigInfo->ldcBuffer.isValid()) {
            pStereoConfigInfo->ldcBuffer = pBufferItem->value;
            StereoLogD("<readBufferItem> get ldcBuffer value from APP1");
        } else if (ATTRIBUTE_CLEAR_IMAGE_IN_APP1 == pBufferItem->name
            && !pStereoConfigInfo->clearImage.isValid()) {
            pStereoConfigInfo->clearImage = pBufferItem->value;
            StereoLogD("<readBufferItem> get clearImage value from APP1");
        }
    }
}

void StereoConfigInfoParser::readStructItem() {
    StereoLogD("<readStructItem>");
    if (pListOfStructItem == nullptr || pListOfStructItem->empty()) {
        StereoLogW("<readStructItem> pListOfStructItem is empty, cannot read");
        return;
    }

    if (pStereoConfigInfo == nullptr) {
        StereoLogW("<readStructItem> pStereoConfigInfo is empty, cannot write");
        return;
    }

    // read face detection info
    if (pStereoConfigInfo->fdInfoArray == nullptr) {
        pStereoConfigInfo->fdInfoArray = new StereoVector<FaceDetectionInfo*>();
    } else {
        pStereoConfigInfo->fdInfoArray->clear();
    }
    std::stringstream ssStructName;
    for (int i = 0; i < SUPPORT_FACE_COUNT; i++) {
        FaceDetectionInfo* faceDetection = new FaceDetectionInfo();
        pStereoConfigInfo->fdInfoArray->push_back(faceDetection);

        ssStructName.str("");
        ssStructName << ATTRIBUTE_FACE_STRUCT_NAME << i;
        for (auto iter = pListOfStructItem->begin();
            iter != pListOfStructItem->end(); iter++) {
            StructItem *pStructItem = *iter;
            if (pStructItem == nullptr || pStructItem->fieldName.empty()
                    || pStructItem->fieldValue.empty()) {
                continue;
            }
            if (ssStructName.str() == pStructItem->structName) {
                if (ATTRIBUTE_FACE_LEFT == pStructItem->fieldName) {
                    (*pStereoConfigInfo->fdInfoArray)[i]->faceLeft = atoi(pStructItem->fieldValue.c_str());
                } else if (ATTRIBUTE_FACE_TOP == pStructItem->fieldName) {
                    (*pStereoConfigInfo->fdInfoArray)[i]->faceTop = atoi(pStructItem->fieldValue.c_str());
                } else if (ATTRIBUTE_FACE_RIGHT == pStructItem->fieldName) {
                    (*pStereoConfigInfo->fdInfoArray)[i]->faceRight = atoi(pStructItem->fieldValue.c_str());
                } else if (ATTRIBUTE_FACE_BOTTOM == pStructItem->fieldName) {
                    (*pStereoConfigInfo->fdInfoArray)[i]->faceBottom = atoi(pStructItem->fieldValue.c_str());
                } else if (ATTRIBUTE_FACE_RIP == pStructItem->fieldName) {
                    (*pStereoConfigInfo->fdInfoArray)[i]->faceRip = atoi(pStructItem->fieldValue.c_str());
                }
            }
        }
    }

    // read focus info
    if (pStereoConfigInfo->focusInfo == nullptr) {
        pStereoConfigInfo->focusInfo = new FocusInfo();
    }
    for (auto iter = pListOfStructItem->begin();
        iter != pListOfStructItem->end(); iter++) {
        StructItem *pStructItem = *iter;
        if (pStructItem == nullptr || pStructItem->fieldName.empty()
                || pStructItem->fieldValue.empty()) {
            continue;
        }
        if (ATTRIBUTE_FOCUSINFO_STRUCT_NAME == pStructItem->structName) {
            if (ATTRIBUTE_FOCUSINFO_LEFT == pStructItem->fieldName) {
                pStereoConfigInfo->focusInfo->focusLeft = atoi(pStructItem->fieldValue.c_str());
            } else if (ATTRIBUTE_FOCUSINFO_TOP == pStructItem->fieldName) {
                pStereoConfigInfo->focusInfo->focusTop = atoi(pStructItem->fieldValue.c_str());
            } else if (ATTRIBUTE_FOCUSINFO_RIGHT == pStructItem->fieldName) {
                pStereoConfigInfo->focusInfo->focusRight = atoi(pStructItem->fieldValue.c_str());
            } else if (ATTRIBUTE_FOCUSINFO_BOTTOM == pStructItem->fieldName) {
                pStereoConfigInfo->focusInfo->focusBottom = atoi(pStructItem->fieldValue.c_str());
            } else if (ATTRIBUTE_FOCUSINFO_TYPE == pStructItem->fieldName) {
                pStereoConfigInfo->focusInfo->focusType = atoi(pStructItem->fieldValue.c_str());
            }
        }
    }

}

void StereoConfigInfoParser::readCustDataItem() {
    StereoLogD("<readCustDataItem>");
    if (pListOfCustomDataItem == nullptr || pListOfCustomDataItem->empty()) {
        StereoLogW("<readCustDataItem> pListOfCustomDataItem is empty, cannot read");
        return;
    }
    for (auto iter = pListOfCustomDataItem->begin();
            iter != pListOfCustomDataItem->end(); iter++) {
        BufferItem *pBufferItem = *iter;
        if (pBufferItem == nullptr || !pBufferItem->value.isValid()) {
            continue;
        }
        // always read clear image and ldcBuffer from APP15
        if (ATTRIBUTE_CLEAR_IMAGE_IN_APP15 == pBufferItem->name) {
            pStereoConfigInfo->clearImage = pBufferItem->value;
            StereoLogD("<readBufferItem> get clearImage value from APP15");
        } else if (ATTRIBUTE_LDC_BUFFER_IN_APP15 == pBufferItem->name) {
            pStereoConfigInfo->ldcBuffer = pBufferItem->value;
            StereoLogD("<readBufferItem> get ldcBuffer value from APP15");
        }
    }
}

void StereoConfigInfoParser::writeSimpleValue() {
    StereoLogD("<writeSimpleValue>");
    if (pListOfSimpleValue == nullptr || pListOfSimpleValue->empty()) {
        StereoLogW("<writeSimpleValue> pListOfSimpleValue is empty, cannot write");
        return;
    }
    for (auto iter = pListOfSimpleValue->begin();
            iter != pListOfSimpleValue->end(); iter++) {
        SimpleItem *pSimpleValue = *iter;
        if (pSimpleValue == nullptr) {
            continue;
        }
        if (ATTRIBUTE_JPS_WIDTH == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->jpsWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_JPS_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->jpsHeight, pSimpleValue->value);
        } else if (ATTRIBUTE_MASK_WIDTH == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->maskWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_MASK_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->maskHeight, pSimpleValue->value);
        } else if (ATTRIBUTE_POS_X == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->posX, pSimpleValue->value);
        } else if (ATTRIBUTE_POS_Y == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->posY, pSimpleValue->value);
        } else if (ATTRIBUTE_VIEW_WIDTH == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->viewWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_VIEW_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->viewHeight, pSimpleValue->value);
        } else if (ATTRIBUTE_ORIENTATION == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->imageOrientation, pSimpleValue->value);
        } else if (ATTRIBUTE_DEPTH_ROTATION == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->depthOrientation, pSimpleValue->value);
        } else if (ATTRIBUTE_MAIN_CAM_POS == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->mainCamPos, pSimpleValue->value);
        } else if (ATTRIBUTE_TOUCH_COORDX_1ST == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->touchCoordX1st, pSimpleValue->value);
        } else if (ATTRIBUTE_TOUCH_COORDY_1ST == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->touchCoordY1st, pSimpleValue->value);
        } else if (ATTRIBUTE_FACE_COUNT == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->faceCount, pSimpleValue->value);
        } else if (ATTRIBUTE_DOF_LEVEL == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->dofLevel, pSimpleValue->value);
        } else if (ATTRIBUTE_CONV_OFFSET == pSimpleValue->name) {
            Utils::d2str(pStereoConfigInfo->convOffset, pSimpleValue->value);
        } else if (ATTRIBUTE_LDC_WIDTH == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->ldcWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_LDC_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->ldcHeight, pSimpleValue->value);
        } else if (ATTRIBUTE_FACE_FLAG == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->isFace, pSimpleValue->value);
        } else if (ATTRIBUTE_FACE_RATIO == pSimpleValue->name) {
            Utils::d2str(pStereoConfigInfo->faceRatio, pSimpleValue->value);
        } else if (ATTRIBUTE_CUR_DAC == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->curDac, pSimpleValue->value);
        } else if (ATTRIBUTE_MIN_DAC == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->minDac, pSimpleValue->value);
        } else if (ATTRIBUTE_MAX_DAC == pSimpleValue->name) {
            Utils::int2str(pStereoConfigInfo->maxDac, pSimpleValue->value);
        }
    }
}

void StereoConfigInfoParser::writeStructItem() {
    StereoLogD("<writeStructItem>");
    if (pListOfStructItem == nullptr || pListOfStructItem->empty()) {
        StereoLogW("<writeStructItem> pListOfStructItem is empty, cannot write");
        return;
    }

    if (pStereoConfigInfo == nullptr) {
        StereoLogW("<writeStructItem> pStereoConfigInfo is empty, cannot write");
        return;
    }

    // write face detection info
    if (pStereoConfigInfo->fdInfoArray != nullptr
            && !pStereoConfigInfo->fdInfoArray->empty()) {
        int fdInfoCount = pStereoConfigInfo->fdInfoArray->size() > SUPPORT_FACE_COUNT ?
            SUPPORT_FACE_COUNT : pStereoConfigInfo->fdInfoArray->size();
        std::stringstream ssStructName;
        for (int i = 0; i < fdInfoCount; i++) {
            if ((*pStereoConfigInfo->fdInfoArray)[i] == nullptr) {
                StereoLogW("(*pStereoConfigInfo->fdInfoArray)[%d] == nullptr", i);
                continue;
            }
            ssStructName.str("");
            ssStructName << ATTRIBUTE_FACE_STRUCT_NAME << i;
            for (auto iter = pListOfStructItem->begin();
                iter != pListOfStructItem->end(); iter++) {
                StructItem *pStructItem = *iter;
                if (pStructItem == nullptr) {
                    continue;
                }
                if (ssStructName.str() == pStructItem->structName) {
                    if (ATTRIBUTE_FACE_LEFT == pStructItem->fieldName) {
                        Utils::int2str((*pStereoConfigInfo->fdInfoArray)[i]->faceLeft, pStructItem->fieldValue);
                    } else if (ATTRIBUTE_FACE_TOP == pStructItem->fieldName) {
                        Utils::int2str((*pStereoConfigInfo->fdInfoArray)[i]->faceTop, pStructItem->fieldValue);
                    } else if (ATTRIBUTE_FACE_RIGHT == pStructItem->fieldName) {
                        Utils::int2str((*pStereoConfigInfo->fdInfoArray)[i]->faceRight, pStructItem->fieldValue);
                    } else if (ATTRIBUTE_FACE_BOTTOM == pStructItem->fieldName) {
                        Utils::int2str((*pStereoConfigInfo->fdInfoArray)[i]->faceBottom, pStructItem->fieldValue);
                    } else if (ATTRIBUTE_FACE_RIP == pStructItem->fieldName) {
                        Utils::int2str((*pStereoConfigInfo->fdInfoArray)[i]->faceRip, pStructItem->fieldValue);
                    }
                }
            }
        }
    }

    // write focus info
    if (pStereoConfigInfo->focusInfo != nullptr) {
        for (auto iter = pListOfStructItem->begin();
            iter != pListOfStructItem->end(); iter++) {
            StructItem *pStructItem = *iter;
            if (pStructItem == nullptr) {
                continue;
            }
            if (ATTRIBUTE_FOCUSINFO_STRUCT_NAME == pStructItem->structName) {
                if (ATTRIBUTE_FOCUSINFO_LEFT == pStructItem->fieldName) {
                    Utils::int2str(pStereoConfigInfo->focusInfo->focusLeft, pStructItem->fieldValue);
                } else if (ATTRIBUTE_FOCUSINFO_TOP == pStructItem->fieldName) {
                    Utils::int2str(pStereoConfigInfo->focusInfo->focusTop, pStructItem->fieldValue);
                } else if (ATTRIBUTE_FOCUSINFO_RIGHT == pStructItem->fieldName) {
                    Utils::int2str(pStereoConfigInfo->focusInfo->focusRight, pStructItem->fieldValue);
                } else if (ATTRIBUTE_FOCUSINFO_BOTTOM == pStructItem->fieldName) {
                    Utils::int2str(pStereoConfigInfo->focusInfo->focusBottom, pStructItem->fieldValue);
                } else if (ATTRIBUTE_FOCUSINFO_TYPE == pStructItem->fieldName) {
                    Utils::int2str(pStereoConfigInfo->focusInfo->focusType, pStructItem->fieldValue);
                }
            }
        }
    }
}

void StereoConfigInfoParser::writeCustDataItem() {
    StereoLogD("<writeCustDataItem>");
    if (pListOfCustomDataItem == nullptr || pListOfCustomDataItem->empty()) {
        StereoLogW("<writeCustDataItem> pListOfCustomDataItem is empty, cannot write");
        return;
    }
    for (auto iter = pListOfCustomDataItem->begin();
            iter != pListOfCustomDataItem->end(); iter++) {
        BufferItem *pBufferItem = *iter;
        if (pBufferItem == nullptr) {
            continue;
        }
        if (ATTRIBUTE_LDC_BUFFER_IN_APP15 == pBufferItem->name
                && pStereoConfigInfo->ldcBuffer.isValid()) {
            pBufferItem->value = pStereoConfigInfo->ldcBuffer;
        } else if (ATTRIBUTE_CLEAR_IMAGE_IN_APP15 == pBufferItem->name
                && pStereoConfigInfo->clearImage.isValid()) {
            pBufferItem->value = pStereoConfigInfo->clearImage;
        }
    }
}

void StereoConfigInfoParser::dumpValuesAndBuffers(StereoString suffix) {
    if (!Utils::ENABLE_BUFFER_DUMP) {
        return;
    }
    StereoString dumpFolder;
    StereoString dumpPath;
    dumpFolder.append(DUMP_FILE_FOLDER).append("/")
        .append(pStereoConfigInfo->debugDir).append("/");

    if (pStereoConfigInfo->clearImage.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoConfigInfo_clearImage_")
            .append(suffix).append(".raw"),
            pStereoConfigInfo->clearImage);
    } else {
        StereoLogD("<dumpValuesAndBuffers> clearImage is null!");
    }

    if (pStereoConfigInfo->ldcBuffer.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoConfigInfo_ldcBuffer_")
            .append(suffix).append(".raw"),
            pStereoConfigInfo->ldcBuffer);
    } else {
        StereoLogD("<dumpValuesAndBuffers> ldcBuffer is null!");
    }

    dumpPath.clear();
    Utils::writeStringToFile(
        dumpPath.append(dumpFolder).append("StereoConfigInfo_").append(suffix).append(".txt"),
        pStereoConfigInfo->toString());
}