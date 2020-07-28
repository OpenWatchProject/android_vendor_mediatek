//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/StereoDepthInfoParser"

#include "StereoDepthInfoParser.h"
#include <stdlib.h>
#include "StereoLog.h"
#include "MetaOperatorFactory.h"
#include "Utils.h"

using namespace stereo;

// assign a initial size when define vector,
// it will reduce the count of resize and buffer copy
#define VECTOR_SIZE_SIMPLE_VALUE 9
#define VECTOR_SIZE_BUFFLE_ITEM 1
#define VECTOR_SIZE_CUST_DATA_ITEM 2

StereoDepthInfoParser:: StereoDepthInfoParser(
        const StereoBuffer_t &standardBuffer, const StereoBuffer_t &extendedBuffer,
        BufferMapPtr customizedBuffer, StereoDepthInfo *info) {
    StereoLogD("<StereoDepthInfoParser> by buffer");

    instantiationWay = INSTANTIATION_BY_BUFFER;

    // new data collections
    pStandardDataCollections = new DataCollections();
    pStandardDataCollections->dest = DEST_TYPE_STANDARD_XMP;
    pExtendardDataCollections = new DataCollections();
    pExtendardDataCollections->dest = DEST_TYPE_EXTENDED_XMP;
    pCustomizedDataCollections = new DataCollections();

    pStereoDepthInfo = info;

    initSimpleValue();
    initBufferItem();
    initCustDataItem();

    pStandardDataCollections->listOfSimpleValue = pListOfSimpleValue;
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

StereoDepthInfoParser::StereoDepthInfoParser(IMetaOperator *standardMetaOperator,
        IMetaOperator *extendedMetaOperator, IMetaOperator *customizedMetaOperator,
        StereoDepthInfo *info) {
    StereoLogD("<StereoDepthInfoParser> by operator");

    instantiationWay = INSTANTIATION_BY_OPERATOR;

    // new data collections
    pStandardDataCollections = new DataCollections();
    pStandardDataCollections->dest = DEST_TYPE_STANDARD_XMP;
    pExtendardDataCollections = new DataCollections();
    pExtendardDataCollections->dest = DEST_TYPE_EXTENDED_XMP;
    pCustomizedDataCollections = new DataCollections();

    pStereoDepthInfo = info;

    initSimpleValue();
    initBufferItem();
    initCustDataItem();

    pStandardDataCollections->listOfSimpleValue = pListOfSimpleValue;
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

StereoDepthInfoParser::~StereoDepthInfoParser() {
    StereoLogD("<~StereoDepthInfoParser>");
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

void StereoDepthInfoParser::read() {
    StereoLogD("<read>");
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->read();
    }
    if (pExtendedMetaOperator != nullptr) {
        pExtendedMetaOperator->read();
    }
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->read();
    }
    if (pStereoDepthInfo == nullptr) {
        StereoLogW("<read> pStereoDepthInfo is null!");
        return;
    }
    readSimpleValue();
    readBufferItem();
    readCustDataItem();
    StereoLogD("<read> %s", pStereoDepthInfo->toString().c_str());
    dumpValuesAndBuffers("read");
}

void StereoDepthInfoParser::write() {
    StereoLogD("<write>");
    if (pStereoDepthInfo == nullptr) {
        StereoLogW("<write> pStereoDepthInfo is null!");
        return;
    }
    dumpValuesAndBuffers("write");
    writeSimpleValue();
    writeBufferItem();
    writeCustDataItem();
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->write();
    }
    if (pExtendedMetaOperator != nullptr) {
        pExtendedMetaOperator->write();
    }
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->write();
    }
}

SerializedInfo* StereoDepthInfoParser::serialize() {
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

    if (pExtendedMetaOperator != nullptr) {
        BufferMapPtr extendData = pExtendedMetaOperator->serialize();
        auto iter = extendData->find(XMP_KEY);
        if (iter != extendData->end()) {
            info->extendedXmpBuf = iter->second;
        } else {
            StereoLogW("can not find property from extend data");
        }
    }

    if (pCustomizedMetaOperator != nullptr) {
        info->customizedBufMap = pCustomizedMetaOperator->serialize();
    }
    return info;
}

SimpleItem* StereoDepthInfoParser::getSimpleValueInstance() {
    SimpleItem* simpleValue = new SimpleItem();
    simpleValue->dest = DEST_TYPE_STANDARD_XMP;
    simpleValue->nameSpaceItem.dest = DEST_TYPE_STANDARD_XMP;
    simpleValue->nameSpaceItem.nameSpace = NS_STEREO;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_STEREO;
    return simpleValue;
}

void StereoDepthInfoParser::initSimpleValue() {
    StereoLogD("<initSimpleValue>");
    if (pListOfSimpleValue == nullptr) {
        pListOfSimpleValue = new StereoVector<SimpleItem*>();
        pListOfSimpleValue->reserve(VECTOR_SIZE_SIMPLE_VALUE);
    }

    SimpleItem *simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_META_BUFFER_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_META_BUFFER_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_TOUCH_COORDX_LAST;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_TOUCH_COORDY_LAST;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_DEPTH_OF_FIELD_LAST;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_DEPTH_BUFFER_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_DEPTH_BUFFER_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_DEPTH_MAP_WIDTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_DEPTH_MAP_HEIGHT;
    pListOfSimpleValue->push_back(simpleValue);
}

void StereoDepthInfoParser::initBufferItem() {
    StereoLogD("<initSimpleValue>");
    if (pListOfBufferItem == nullptr) {
        pListOfBufferItem = new StereoVector<BufferItem*>();
        pListOfBufferItem->reserve(VECTOR_SIZE_BUFFLE_ITEM);
    }
    BufferItem *depthMapItem = new BufferItem();
    depthMapItem->dest = DEST_TYPE_EXTENDED_XMP;
    depthMapItem->nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    depthMapItem->nameSpaceItem.nameSpace = NS_GDEPTH;
    depthMapItem->nameSpaceItem.nameSpacePrefix = PRIFIX_GDEPTH;
    depthMapItem->name = ATTRIBUTE_DEPTH_MAP;
    pListOfBufferItem->push_back(depthMapItem);
}

void StereoDepthInfoParser::initCustDataItem() {
    StereoLogD("<initCustDataItem>");
    if (pListOfCustomDataItem == nullptr) {
        pListOfCustomDataItem = new StereoVector<BufferItem*>();
        pListOfCustomDataItem->reserve(VECTOR_SIZE_CUST_DATA_ITEM);
    }
    BufferItem *depthBufferItem = new BufferItem();
    depthBufferItem->name = ATTRIBUTE_DEPTH_BUFFER;
    pListOfCustomDataItem->push_back(depthBufferItem);
    BufferItem *debugBufferItem = new BufferItem();
    debugBufferItem->name = ATTRIBUTE_DEBUG_BUFFER;
    pListOfCustomDataItem->push_back(debugBufferItem);
}

void StereoDepthInfoParser::readSimpleValue() {
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
        if (ATTRIBUTE_META_BUFFER_WIDTH == pSimpleValue->name) {
            pStereoDepthInfo->metaBufferWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_META_BUFFER_HEIGHT == pSimpleValue->name) {
            pStereoDepthInfo->metaBufferHeight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_TOUCH_COORDX_LAST == pSimpleValue->name) {
            pStereoDepthInfo->touchCoordXLast = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_TOUCH_COORDY_LAST == pSimpleValue->name) {
            pStereoDepthInfo->touchCoordYLast = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_DEPTH_OF_FIELD_LAST == pSimpleValue->name) {
            pStereoDepthInfo->depthOfFieldLast = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_DEPTH_BUFFER_WIDTH == pSimpleValue->name) {
            pStereoDepthInfo->depthBufferWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_DEPTH_BUFFER_HEIGHT == pSimpleValue->name) {
            pStereoDepthInfo->depthBufferHeight = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_DEPTH_MAP_WIDTH == pSimpleValue->name) {
            pStereoDepthInfo->depthMapWidth = atoi(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_DEPTH_MAP_HEIGHT == pSimpleValue->name) {
            pStereoDepthInfo->depthMapHeight = atoi(pSimpleValue->value.c_str());
        }
    }
}

void StereoDepthInfoParser::readBufferItem() {
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
        if (ATTRIBUTE_DEPTH_MAP == pBufferItem->name) {
            pStereoDepthInfo->depthMap = pBufferItem->value;
        }
    }
}

void StereoDepthInfoParser::readCustDataItem() {
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
        if (ATTRIBUTE_DEPTH_BUFFER == pBufferItem->name) {
            pStereoDepthInfo->depthBuffer = pBufferItem->value;
        } else if (ATTRIBUTE_DEBUG_BUFFER == pBufferItem->name) {
            pStereoDepthInfo->debugBuffer = pBufferItem->value;
        }
    }
}

void StereoDepthInfoParser::writeSimpleValue() {
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
        if (ATTRIBUTE_META_BUFFER_WIDTH == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->metaBufferWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_META_BUFFER_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->metaBufferHeight, pSimpleValue->value);
        } else if (ATTRIBUTE_TOUCH_COORDX_LAST == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->touchCoordXLast, pSimpleValue->value);
        } else if (ATTRIBUTE_TOUCH_COORDY_LAST == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->touchCoordYLast, pSimpleValue->value);
        } else if (ATTRIBUTE_DEPTH_OF_FIELD_LAST == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->depthOfFieldLast, pSimpleValue->value);
        } else if (ATTRIBUTE_DEPTH_BUFFER_WIDTH == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->depthBufferWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_DEPTH_BUFFER_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->depthBufferHeight, pSimpleValue->value);
        } else if (ATTRIBUTE_DEPTH_MAP_WIDTH == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->depthMapWidth, pSimpleValue->value);
        } else if (ATTRIBUTE_DEPTH_MAP_HEIGHT == pSimpleValue->name) {
            Utils::int2str(pStereoDepthInfo->depthMapHeight, pSimpleValue->value);
        }
    }
}

void StereoDepthInfoParser::writeBufferItem() {
    StereoLogD("<writeBufferItem>");
    if (pListOfBufferItem == nullptr || pListOfBufferItem->empty()) {
        StereoLogW("<writeBufferItem> pListOfBufferItem is empty, cannot write");
        return;
    }
    for (auto iter = pListOfBufferItem->begin();
            iter != pListOfBufferItem->end(); iter++) {
        BufferItem *pBufferItem = *iter;
        if (pBufferItem == nullptr) {
            continue;
        }
        if (ATTRIBUTE_DEPTH_MAP == pBufferItem->name
                && pStereoDepthInfo->depthMap.isValid()) {
            pBufferItem->value = pStereoDepthInfo->depthMap;
        }
    }
}

void StereoDepthInfoParser::writeCustDataItem() {
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
        if (ATTRIBUTE_DEPTH_BUFFER == pBufferItem->name
                && pStereoDepthInfo->depthBuffer.isValid()) {
            pBufferItem->value = pStereoDepthInfo->depthBuffer;
        } else if (ATTRIBUTE_DEBUG_BUFFER == pBufferItem->name
            && pStereoDepthInfo->debugBuffer.isValid()) {
            pBufferItem->value = pStereoDepthInfo->debugBuffer;
        }
    }
}

void StereoDepthInfoParser::dumpValuesAndBuffers(StereoString suffix) {
    if (!Utils::ENABLE_BUFFER_DUMP) {
        return;
    }
    StereoString dumpFolder;
    StereoString dumpPath;
    dumpFolder.append(DUMP_FILE_FOLDER).append("/")
        .append(pStereoDepthInfo->debugDir).append("/");

    if (pStereoDepthInfo->depthBuffer.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoDepthInfo_depthBuffer_")
            .append(suffix).append(".raw"),
            pStereoDepthInfo->depthBuffer);
    } else {
        StereoLogD("<dumpValuesAndBuffers> depthBuffer is null!");
    }

    if (pStereoDepthInfo->depthMap.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoDepthInfo_depthMap_")
            .append(suffix).append(".raw"),
            pStereoDepthInfo->depthMap);
    } else {
        StereoLogD("<dumpValuesAndBuffers> depthMap is null!");
    }

    if (pStereoDepthInfo->debugBuffer.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoDepthInfo_debugBuffer_")
            .append(suffix).append(".raw"),
            pStereoDepthInfo->debugBuffer);
    } else {
        StereoLogD("<dumpValuesAndBuffers> debugBuffer is null!");
    }

    dumpPath.clear();
    Utils::writeStringToFile(
        dumpPath.append(dumpFolder).append("StereoDepthInfo_").append(suffix).append(".txt"),
        pStereoDepthInfo->toString());
}