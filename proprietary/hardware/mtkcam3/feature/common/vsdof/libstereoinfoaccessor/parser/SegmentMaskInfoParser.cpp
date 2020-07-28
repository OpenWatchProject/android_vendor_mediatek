//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/SegmentMaskInfoParser"

#include "SegmentMaskInfoParser.h"
#include "StereoLog.h"
#include <stdlib.h>
#include "MetaOperatorFactory.h"
#include "Utils.h"

using namespace stereo;

// assign a initial size when define vector,
// it will reduce the count of resize and buffer copy
#define VECTOR_SIZE_SIMPLE_VALUE 8
#define VECTOR_SIZE_CUST_DATA_ITEM 1

SegmentMaskInfoParser:: SegmentMaskInfoParser(
        const StereoBuffer_t &standardBuffer, BufferMapPtr customizedBuffer,
        SegmentMaskInfo *info) {
    StereoLogD("<SegmentMaskInfoParser> by buffer");

    instantiationWay = INSTANTIATION_BY_BUFFER;

    // new data collections
    pStandardDataCollections = new DataCollections();
    pStandardDataCollections->dest = DEST_TYPE_STANDARD_XMP;
    pCustomizedDataCollections = new DataCollections();

    pSegmentMaskInfo = info;

    initSimpleValue();
    initCustDataItem();

    pStandardDataCollections->listOfSimpleValue = pListOfSimpleValue;
    pStandardMetaOperator = MetaOperatorFactory::getOperatorInstance(
        XMP_META_OPERATOR, standardBuffer, nullptr);
    pStandardMetaOperator->setData(pStandardDataCollections);

    pCustomizedDataCollections->listOfCustomDataItem = pListOfCustomDataItem;
    pCustomizedMetaOperator = MetaOperatorFactory::getOperatorInstance(
        CUSTOMIZED_META_OPERATOR, INVALID_BUFFER, customizedBuffer);
    pCustomizedMetaOperator->setData(pCustomizedDataCollections);
}

SegmentMaskInfoParser::~SegmentMaskInfoParser() {
    StereoLogD("<~SegmentMaskInfoParser>");
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

    // delete pStandardDataCollections
    if (pStandardDataCollections != nullptr) {
        delete pStandardDataCollections;
    }

    // delete pStandardMetaOperator
    if (instantiationWay == INSTANTIATION_BY_BUFFER && pStandardMetaOperator != nullptr) {
        delete pStandardMetaOperator;
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

void SegmentMaskInfoParser::read() {
    StereoLogD("<read>");
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->read();
    }
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->read();
    }
    if (pSegmentMaskInfo == nullptr) {
        StereoLogW("<read> pSegmentMaskInfo is null!");
        return;
    }
    readSimpleValue();
    readCustDataItem();
    StereoLogD("<read> %s", pSegmentMaskInfo->toString().c_str());
    dumpValuesAndBuffers("read");
}

void SegmentMaskInfoParser::write() {
    StereoLogD("<write>");
    if (pSegmentMaskInfo == nullptr) {
        StereoLogW("<write> pSegmentMaskInfo is null!");
        return;
    }
    dumpValuesAndBuffers("write");
    writeSimpleValue();
    writeCustDataItem();
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->write();
    }
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->write();
    }
}

SerializedInfo* SegmentMaskInfoParser::serialize() {
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

SimpleItem* SegmentMaskInfoParser::getSimpleValueInstance() {
    SimpleItem* simpleValue = new SimpleItem();
    simpleValue->dest = DEST_TYPE_STANDARD_XMP;
    simpleValue->nameSpaceItem.dest = DEST_TYPE_STANDARD_XMP;
    simpleValue->nameSpaceItem.nameSpace = NS_STEREO;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_STEREO;
    return simpleValue;
}

void SegmentMaskInfoParser::initSimpleValue() {
    StereoLogD("<initSimpleValue>");
    if (pListOfSimpleValue == nullptr) {
        pListOfSimpleValue = new StereoVector<SimpleItem*>();
        pListOfSimpleValue->reserve(VECTOR_SIZE_SIMPLE_VALUE);
    }

    SimpleItem *simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_MASK_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_MASK_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_SEGMENT_X;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_SEGMENT_Y;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_SEGMENT_LEFT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_SEGMENT_TOP;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_SEGMENT_RIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_SEGMENT_BOTTOM;
    pListOfSimpleValue->push_back(simpleValue);
}

void SegmentMaskInfoParser::initCustDataItem() {
    StereoLogD("<initCustDataItem>");
    if (pListOfCustomDataItem == nullptr) {
        pListOfCustomDataItem = new StereoVector<BufferItem*>();
        pListOfCustomDataItem->reserve(VECTOR_SIZE_CUST_DATA_ITEM);
    }
    BufferItem *custDataItem = new BufferItem();
    custDataItem->name = ATTRIBUTE_SEGMENT_MASK_BUFFER;
    pListOfCustomDataItem->push_back(custDataItem);
}

void SegmentMaskInfoParser::readSimpleValue() {
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
        if (ATTRIBUTE_MASK_WIDTH == pSimpleValue->name) {
            pSegmentMaskInfo->maskWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_MASK_HEIGHT == pSimpleValue->name) {
            pSegmentMaskInfo->maskHeight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_SEGMENT_X == pSimpleValue->name) {
            pSegmentMaskInfo->segmentX = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_SEGMENT_Y == pSimpleValue->name) {
            pSegmentMaskInfo->segmentY = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_SEGMENT_LEFT == pSimpleValue->name) {
            pSegmentMaskInfo->segmentLeft = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_SEGMENT_TOP == pSimpleValue->name) {
            pSegmentMaskInfo->segmentTop = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_SEGMENT_RIGHT == pSimpleValue->name) {
            pSegmentMaskInfo->segmentRight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_SEGMENT_BOTTOM == pSimpleValue->name) {
            pSegmentMaskInfo->segmentBottom = atoi(pSimpleValue->value.c_str());
        }
    }
}

void SegmentMaskInfoParser::readCustDataItem() {
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
        if (ATTRIBUTE_SEGMENT_MASK_BUFFER == pBufferItem->name) {
            pSegmentMaskInfo->maskBuffer = pBufferItem->value;
        }
    }
}

void SegmentMaskInfoParser::writeSimpleValue() {
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
        if (ATTRIBUTE_MASK_WIDTH == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->maskWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_MASK_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->maskHeight, pSimpleValue->value);
        } else if (ATTRIBUTE_SEGMENT_X == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->segmentX, pSimpleValue->value);
        } else if (ATTRIBUTE_SEGMENT_Y == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->segmentY, pSimpleValue->value);
        } else if (ATTRIBUTE_SEGMENT_LEFT == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->segmentLeft, pSimpleValue->value);
        } else if (ATTRIBUTE_SEGMENT_TOP == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->segmentTop, pSimpleValue->value);
        } else if (ATTRIBUTE_SEGMENT_RIGHT == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->segmentRight, pSimpleValue->value);
        } else if (ATTRIBUTE_SEGMENT_BOTTOM == pSimpleValue->name) {
            Utils::int2str(pSegmentMaskInfo->segmentBottom, pSimpleValue->value);
        }
    }
}

void SegmentMaskInfoParser::writeCustDataItem() {
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
        if (ATTRIBUTE_SEGMENT_MASK_BUFFER == pBufferItem->name
                && pSegmentMaskInfo->maskBuffer.isValid()) {
            pBufferItem->value = pSegmentMaskInfo->maskBuffer;
        }
    }
}

void SegmentMaskInfoParser::dumpValuesAndBuffers(StereoString suffix) {
    if (!Utils::ENABLE_BUFFER_DUMP) {
        return;
    }
    StereoString dumpFolder;
    StereoString dumpPath;
    dumpFolder.append(DUMP_FILE_FOLDER).append("/")
        .append(pSegmentMaskInfo->debugDir).append("/");

    if (pSegmentMaskInfo->maskBuffer.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("SegmentMaskInfo_maskBuffer_")
            .append(suffix).append(".raw"),
            pSegmentMaskInfo->maskBuffer);
    } else {
        StereoLogD("<dumpValuesAndBuffers> maskBuffer is null!");
    }

    dumpPath.clear();
    Utils::writeStringToFile(
        dumpPath.append(dumpFolder).append("SegmentMaskInfo_").append(suffix).append(".txt"),
        pSegmentMaskInfo->toString());
}

