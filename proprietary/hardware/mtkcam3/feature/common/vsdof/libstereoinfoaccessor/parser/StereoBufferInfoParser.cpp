//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/StereoBufferInfoParser"

#include "StereoBufferInfoParser.h"
#include "StereoLog.h"
#include "MetaOperatorFactory.h"
#include "Utils.h"

using namespace stereo;

// assign a initial size when define vector,
// it will reduce the count of resize and buffer copy
#define VECTOR_SIZE_CUST_DATA_ITEM 2

StereoBufferInfoParser:: StereoBufferInfoParser(
    BufferMapPtr customizedBuffer, StereoBufferInfo *info) {
    StereoLogD("<StereoBufferInfoParser> by buffer");

    instantiationWay = INSTANTIATION_BY_BUFFER;

    pStereoBufferInfo = info;
    initCustDataItem();
    pCustomizedDataCollections = new DataCollections();
    pCustomizedDataCollections->listOfCustomDataItem = pListOfCustomDataItem;
    pCustomizedMetaOperator = MetaOperatorFactory::getOperatorInstance(
        CUSTOMIZED_META_OPERATOR, INVALID_BUFFER, customizedBuffer);
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->setData(pCustomizedDataCollections);
    }
}

StereoBufferInfoParser:: StereoBufferInfoParser(
    IMetaOperator *customizedMetaOperator, StereoBufferInfo *info) {
    StereoLogD("<StereoBufferInfoParser> by operator");

    instantiationWay = INSTANTIATION_BY_OPERATOR;

    pStereoBufferInfo = info;
    initCustDataItem();
    pCustomizedDataCollections = new DataCollections();
    pCustomizedDataCollections->listOfCustomDataItem = pListOfCustomDataItem;
    pCustomizedMetaOperator = customizedMetaOperator;
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->setData(pCustomizedDataCollections);
    }
}

StereoBufferInfoParser::~StereoBufferInfoParser() {
    StereoLogD("<~StereoBufferInfoParser>");
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

void StereoBufferInfoParser::read() {
    StereoLogD("<read>");
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->read();
    }
    if (pStereoBufferInfo == nullptr) {
        StereoLogW("<read> pStereoBufferInfo is null!");
        return;
    }
    readCustDataItem();
    StereoLogD("<read> %s", pStereoBufferInfo->toString().c_str());
    dumpValuesAndBuffers("read");
}

void StereoBufferInfoParser::write() {
    StereoLogD("<write>");
    if (pStereoBufferInfo == nullptr) {
        StereoLogW("<write> pStereoBufferInfo is null!");
        return;
    }
    dumpValuesAndBuffers("write");
    writeCustDataItem();
    if (pCustomizedMetaOperator != nullptr) {
        pCustomizedMetaOperator->write();
    }
}

SerializedInfo* StereoBufferInfoParser::serialize() {
    StereoLogD("<serialize>");
    SerializedInfo *info = new SerializedInfo();
    if (pCustomizedMetaOperator != nullptr) {
        info->customizedBufMap = pCustomizedMetaOperator->serialize();
    }
    return info;
}

void StereoBufferInfoParser::initCustDataItem() {
    StereoLogD("<initCustDataItem>");
    // new vector<BufferItem*>
    if (pListOfCustomDataItem == nullptr) {
        pListOfCustomDataItem = new StereoVector<BufferItem*>();
        pListOfCustomDataItem->reserve(VECTOR_SIZE_CUST_DATA_ITEM);
    }
    // add jps buffer item
    BufferItem* custDataItem = new BufferItem();
    custDataItem->name = ATTRIBUTE_JPS_BUFFER;
    pListOfCustomDataItem->push_back(custDataItem);
    // add segment mask buffer item
    custDataItem = new BufferItem();
    custDataItem->name = ATTRIBUTE_SEGMENT_MASK_BUFFER;
    pListOfCustomDataItem->push_back(custDataItem);
}

void StereoBufferInfoParser::readCustDataItem() {
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
        if (ATTRIBUTE_JPS_BUFFER == pBufferItem->name) {
            pStereoBufferInfo->jpsBuffer = pBufferItem->value;
        } else if (ATTRIBUTE_SEGMENT_MASK_BUFFER == pBufferItem->name) {
            pStereoBufferInfo->maskBuffer = pBufferItem->value;
        }
    }
}

void StereoBufferInfoParser::writeCustDataItem() {
    StereoLogD("<writeCustDataItem>");
    if (pListOfCustomDataItem == nullptr || pListOfCustomDataItem->empty()) {
        StereoLogW("<writeCustDataItem> pListOfCustomDataItem is empty, cannot wite");
        return;
    }
    for (auto iter = pListOfCustomDataItem->begin();
            iter != pListOfCustomDataItem->end(); iter++) {
        BufferItem *pBufferItem = *iter;
        if (pBufferItem == nullptr) {
            continue;
        }
        if (ATTRIBUTE_JPS_BUFFER == pBufferItem->name
                && pStereoBufferInfo->jpsBuffer.isValid()) {
            pBufferItem->value = pStereoBufferInfo->jpsBuffer;
        } else if (ATTRIBUTE_SEGMENT_MASK_BUFFER == pBufferItem->name
                && pStereoBufferInfo->maskBuffer.isValid()) {
            pBufferItem->value = pStereoBufferInfo->maskBuffer;
        }
    }
}

void StereoBufferInfoParser::dumpValuesAndBuffers(StereoString suffix) {
    if (!Utils::ENABLE_BUFFER_DUMP) {
        return;
    }
    StereoString dumpFolder;
    StereoString dumpPath;
    dumpFolder.append(DUMP_FILE_FOLDER).append("/")
        .append(pStereoBufferInfo->debugDir).append("/");

    if (pStereoBufferInfo->maskBuffer.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoBufferInfo_maskBuffer_")
            .append(suffix).append(".raw"),
            pStereoBufferInfo->maskBuffer);
    } else {
        StereoLogD("<dumpValuesAndBuffers> maskBuffer is null!");
    }

    if (pStereoBufferInfo->jpsBuffer.isValid()) {
        dumpPath.clear();
        Utils::writeBufferToFile(
            dumpPath.append(dumpFolder).append("StereoBufferInfo_jpsBuffer_")
            .append(suffix).append(".raw"),
            pStereoBufferInfo->jpsBuffer);
    } else {
        StereoLogD("<dumpValuesAndBuffers> maskBuffer is null!");
    }

    dumpPath.clear();
    Utils::writeStringToFile(
        dumpPath.append(dumpFolder).append("StereoBufferInfo_").append(suffix).append(".txt"),
        pStereoBufferInfo->toString());
}