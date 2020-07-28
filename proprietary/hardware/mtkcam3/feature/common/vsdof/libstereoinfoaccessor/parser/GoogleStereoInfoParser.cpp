//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/GoogleStereoInfoParser"

#include "GoogleStereoInfoParser.h"
#include "StereoLog.h"
#include <stdlib.h>
#include "MetaOperatorFactory.h"
#include "Utils.h"

using namespace stereo;

// assign a initial size when define vector,
// it will reduce the count of resize and buffer copy
#define VECTOR_SIZE_SIMPLE_VALUE 9
#define VECTOR_SIZE_BUFFLE_ITEM 2

GoogleStereoInfoParser:: GoogleStereoInfoParser(const StereoBuffer_t &standardBuffer,
        const StereoBuffer_t &extendedBuffer, GoogleStereoInfo *info) {
    StereoLogD("<GoogleStereoInfoParser> by buffer");

    instantiationWay = INSTANTIATION_BY_BUFFER;

    // new data collections
    pStandardDataCollections = new DataCollections();
    pStandardDataCollections->dest = DEST_TYPE_STANDARD_XMP;
    pExtendardDataCollections = new DataCollections();
    pExtendardDataCollections->dest = DEST_TYPE_EXTENDED_XMP;

    pGoogleStereoInfo = info;

    initSimpleValue();
    initBufferItem();

    pStandardDataCollections->listOfSimpleValue = pListOfSimpleValue;
    pStandardMetaOperator = MetaOperatorFactory::getOperatorInstance(
        XMP_META_OPERATOR, standardBuffer, nullptr);
    pStandardMetaOperator->setData(pStandardDataCollections);

    pExtendardDataCollections->listOfBufferItem = pListOfBufferItem;
    pExtendedMetaOperator = MetaOperatorFactory::getOperatorInstance(
        XMP_META_OPERATOR, extendedBuffer, nullptr);
    pExtendedMetaOperator->setData(pExtendardDataCollections);
}

GoogleStereoInfoParser::GoogleStereoInfoParser(IMetaOperator *standardMetaOperator,
        IMetaOperator *extendedMetaOperator, GoogleStereoInfo *info) {
    StereoLogD("<GoogleStereoInfoParser> by operator");

    instantiationWay = INSTANTIATION_BY_OPERATOR;

    // new data collections
    pStandardDataCollections = new DataCollections();
    pStandardDataCollections->dest = DEST_TYPE_STANDARD_XMP;
    pExtendardDataCollections = new DataCollections();
    pExtendardDataCollections->dest = DEST_TYPE_EXTENDED_XMP;

    pGoogleStereoInfo = info;

    initSimpleValue();
    initBufferItem();

    pStandardDataCollections->listOfSimpleValue = pListOfSimpleValue;
    pStandardMetaOperator = standardMetaOperator;
    pStandardMetaOperator->setData(pStandardDataCollections);

    pExtendardDataCollections->listOfBufferItem = pListOfBufferItem;
    pExtendedMetaOperator = extendedMetaOperator;
    pExtendedMetaOperator->setData(pExtendardDataCollections);
}

GoogleStereoInfoParser::~GoogleStereoInfoParser() {
    StereoLogD("<~GoogleStereoInfoParser>");
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
}

void GoogleStereoInfoParser::read() {
    StereoLogD("<read>");
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->read();
    }
    if (pExtendedMetaOperator != nullptr) {
        pExtendedMetaOperator->read();
    }
    if (pGoogleStereoInfo == nullptr) {
        StereoLogW("<read> pGoogleStereoInfo is null!");
        return;
    }
    readSimpleValue();
    readBufferItem();
    StereoLogD("<read> %s", pGoogleStereoInfo->toString().c_str());
    dumpValuesAndBuffers("read");
}

void GoogleStereoInfoParser::write() {
    StereoLogD("<write>");
    if (pGoogleStereoInfo == nullptr) {
        StereoLogW("<write> pGoogleStereoInfo is null!");
        return;
    }
    dumpValuesAndBuffers("write");
    writeSimpleValue();
    writeBufferItem();
    if (pStandardMetaOperator != nullptr) {
        pStandardMetaOperator->write();
    }
    if (pExtendedMetaOperator != nullptr) {
        pExtendedMetaOperator->write();
    }
}

SerializedInfo* GoogleStereoInfoParser::serialize() {
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
    return info;
}

SimpleItem* GoogleStereoInfoParser::getSimpleValueInstance() {
    SimpleItem *simpleValue = new SimpleItem();
    simpleValue->dest = DEST_TYPE_STANDARD_XMP;
    simpleValue->nameSpaceItem.dest = DEST_TYPE_STANDARD_XMP;
    return simpleValue;
}

BufferItem* GoogleStereoInfoParser::getBufferItemInstance() {
    BufferItem *bufferItem = new BufferItem();
    bufferItem->dest = DEST_TYPE_EXTENDED_XMP;
    bufferItem->nameSpaceItem.dest = DEST_TYPE_EXTENDED_XMP;
    return bufferItem;
}

void GoogleStereoInfoParser::initSimpleValue() {
    StereoLogD("<initSimpleValue>");
    if (pListOfSimpleValue == nullptr) {
        pListOfSimpleValue = new StereoVector<SimpleItem*>();
        pListOfSimpleValue->reserve(VECTOR_SIZE_SIMPLE_VALUE);
    }

    SimpleItem *simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GFOCUS_BLUR_INFINITY;
    simpleValue->nameSpaceItem.nameSpace = NS_GFOCUS;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GFOCUS;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GFOCUS_FOCALDISTANCE;
    simpleValue->nameSpaceItem.nameSpace = NS_GFOCUS;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GFOCUS;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GFOCUS_FOCALPOINTX;
    simpleValue->nameSpaceItem.nameSpace = NS_GFOCUS;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GFOCUS;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GFOCUS_FOCALPOINTY;
    simpleValue->nameSpaceItem.nameSpace = NS_GFOCUS;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GFOCUS;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GIMAGE_MIME;
    simpleValue->nameSpaceItem.nameSpace = NS_GIMAGE;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GIMAGE;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GDEPTH_FORMAT;
    simpleValue->nameSpaceItem.nameSpace = NS_GDEPTH;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GDEPTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GDEPTH_NEAR;
    simpleValue->nameSpaceItem.nameSpace = NS_GDEPTH;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GDEPTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GDEPTH_FAR;
    simpleValue->nameSpaceItem.nameSpace = NS_GDEPTH;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GDEPTH;
    pListOfSimpleValue->push_back(simpleValue);

    simpleValue = getSimpleValueInstance();
    simpleValue->name = ATTRIBUTE_GDEPTH_MIME;
    simpleValue->nameSpaceItem.nameSpace = NS_GDEPTH;
    simpleValue->nameSpaceItem.nameSpacePrefix = PRIFIX_GDEPTH;
    pListOfSimpleValue->push_back(simpleValue);
}

void GoogleStereoInfoParser::initBufferItem() {
    StereoLogD("<initBufferItem>");
    if (pListOfBufferItem == nullptr) {
        pListOfBufferItem = new StereoVector<BufferItem*>();
        pListOfBufferItem->reserve(VECTOR_SIZE_BUFFLE_ITEM);
    }
    BufferItem *bufferItem = getBufferItemInstance();
    bufferItem->nameSpaceItem.nameSpace = NS_GIMAGE;
    bufferItem->nameSpaceItem.nameSpacePrefix = PRIFIX_GIMAGE;
    bufferItem->name = ATTRIBUTE_CLEAR_IMAGE;
    pListOfBufferItem->push_back(bufferItem);

    bufferItem = getBufferItemInstance();
    bufferItem->nameSpaceItem.nameSpace = NS_GDEPTH;
    bufferItem->nameSpaceItem.nameSpacePrefix = PRIFIX_GDEPTH;
    bufferItem->name = ATTRIBUTE_DEPTH_MAP;
    pListOfBufferItem->push_back(bufferItem);
}

void GoogleStereoInfoParser::readSimpleValue() {
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
        if (ATTRIBUTE_GFOCUS_BLUR_INFINITY == pSimpleValue->name) {
            pGoogleStereoInfo->focusBlurAtInfinity = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_GFOCUS_FOCALDISTANCE == pSimpleValue->name) {
            pGoogleStereoInfo->focusFocalDistance = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_GFOCUS_FOCALPOINTX == pSimpleValue->name) {
            pGoogleStereoInfo->focusFocalPointX = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_GFOCUS_FOCALPOINTY == pSimpleValue->name) {
            pGoogleStereoInfo->focusFocalPointY = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_GIMAGE_MIME == pSimpleValue->name
            && PRIFIX_GIMAGE == pSimpleValue->nameSpaceItem.nameSpacePrefix) {
            pGoogleStereoInfo->imageMime = pSimpleValue->value;
        } else if (ATTRIBUTE_GDEPTH_FORMAT == pSimpleValue->name) {
            pGoogleStereoInfo->depthFormat = pSimpleValue->value;
        } else if (ATTRIBUTE_GDEPTH_NEAR == pSimpleValue->name) {
            pGoogleStereoInfo->depthNear = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_GDEPTH_FAR == pSimpleValue->name) {
            pGoogleStereoInfo->depthFar = atof(pSimpleValue->value.c_str());
        } else if (ATTRIBUTE_GDEPTH_MIME == pSimpleValue->name
            && PRIFIX_GDEPTH == pSimpleValue->nameSpaceItem.nameSpacePrefix) {
            pGoogleStereoInfo->depthMime = pSimpleValue->value;
        }
    }
}

void GoogleStereoInfoParser::readBufferItem() {
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
        if (ATTRIBUTE_CLEAR_IMAGE == pBufferItem->name
            && PRIFIX_GIMAGE == pBufferItem->nameSpaceItem.nameSpacePrefix) {
            pGoogleStereoInfo->clearImage = pBufferItem->value;
        } else if (ATTRIBUTE_DEPTH_MAP == pBufferItem->name
            && PRIFIX_GDEPTH == pBufferItem->nameSpaceItem.nameSpacePrefix) {
            pGoogleStereoInfo->depthMap = pBufferItem->value;
        }
    }
}

void GoogleStereoInfoParser::writeSimpleValue() {
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
        if (ATTRIBUTE_GFOCUS_BLUR_INFINITY == pSimpleValue->name) {
            Utils::d2str(pGoogleStereoInfo->focusBlurAtInfinity, pSimpleValue->value);
        } else if (ATTRIBUTE_GFOCUS_FOCALDISTANCE == pSimpleValue->name) {
            Utils::d2str(pGoogleStereoInfo->focusFocalDistance, pSimpleValue->value);
        } else if (ATTRIBUTE_GFOCUS_FOCALPOINTX == pSimpleValue->name) {
            Utils::d2str(pGoogleStereoInfo->focusFocalPointX, pSimpleValue->value);
        } else if (ATTRIBUTE_GFOCUS_FOCALPOINTY == pSimpleValue->name) {
            Utils::d2str(pGoogleStereoInfo->focusFocalPointY, pSimpleValue->value);
        } else if (ATTRIBUTE_GIMAGE_MIME == pSimpleValue->name
            && PRIFIX_GIMAGE == pSimpleValue->nameSpaceItem.nameSpacePrefix) {
            pSimpleValue->value = pGoogleStereoInfo->imageMime;
        } else if (ATTRIBUTE_GDEPTH_FORMAT == pSimpleValue->name) {
            pSimpleValue->value = pGoogleStereoInfo->depthFormat;
        } else if (ATTRIBUTE_GDEPTH_NEAR == pSimpleValue->name) {
            Utils::d2str(pGoogleStereoInfo->depthNear, pSimpleValue->value);
        } else if (ATTRIBUTE_GDEPTH_FAR == pSimpleValue->name) {
            Utils::d2str(pGoogleStereoInfo->depthFar, pSimpleValue->value);
        } else if (ATTRIBUTE_GDEPTH_MIME == pSimpleValue->name
            && PRIFIX_GDEPTH == pSimpleValue->nameSpaceItem.nameSpacePrefix) {
            pSimpleValue->value = pGoogleStereoInfo->depthMime;
        }
    }
}

void GoogleStereoInfoParser::writeBufferItem() {
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
        if (ATTRIBUTE_CLEAR_IMAGE == pBufferItem->name
            && PRIFIX_GIMAGE == pBufferItem->nameSpaceItem.nameSpacePrefix
            && pGoogleStereoInfo->clearImage.isValid()) {
            pBufferItem->value = pGoogleStereoInfo->clearImage;
        } else if (ATTRIBUTE_DEPTH_MAP == pBufferItem->name
            && PRIFIX_GDEPTH == pBufferItem->nameSpaceItem.nameSpacePrefix
            && pGoogleStereoInfo->depthMap.isValid()) {
            pBufferItem->value = pGoogleStereoInfo->depthMap;
        }
    }
}

void GoogleStereoInfoParser::dumpValuesAndBuffers(StereoString suffix) {
    if (!Utils::ENABLE_BUFFER_DUMP) {
        return;
    }
    StereoString dumpFolder;
    StereoString dumpPath;
    dumpFolder.append(DUMP_FILE_FOLDER).append("/")
        .append(pGoogleStereoInfo->debugDir).append("/");

    if (pGoogleStereoInfo->clearImage.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("GoogleStereoInfo_clearImage_")
            .append(suffix).append(".raw"),
            pGoogleStereoInfo->clearImage);
    } else {
        StereoLogD("<dumpValuesAndBuffers> clearImage is null!");
    }

    if (pGoogleStereoInfo->depthMap.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("GoogleStereoInfo_depthMap_")
            .append(suffix).append(".raw"),
            pGoogleStereoInfo->depthMap);
    } else {
        StereoLogD("<dumpValuesAndBuffers> depthMap is null!");
    }

    dumpPath.clear();
    Utils::writeStringToFile(
        dumpPath.append(dumpFolder).append("GoogleStereoInfo_").append(suffix).append(".txt"),
        pGoogleStereoInfo->toString());
}
