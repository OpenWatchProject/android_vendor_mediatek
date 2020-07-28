//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/XmpMetaOperator"
#include "StereoLog.h"

#include "MetaOperator.h"
#include "BufferManager.h"

using namespace stereo;

XmpMetaOperator::XmpMetaOperator(const StereoBuffer_t &xmpBuffer) :
        mDest(0), pListOfSimpleValue(nullptr), pListOfBufferItem(nullptr),
        pListOfArrayItem(nullptr), pListOfStructItem(nullptr) {
    StereoLogD("XmpMetaOperator");
    pXmpMeta = new XmpMeta(xmpBuffer);
}

XmpMetaOperator::~XmpMetaOperator() {
    StereoLogD("~XmpMetaOperator");
    if (pXmpMeta != nullptr) {
        delete pXmpMeta;
    }
}

void XmpMetaOperator::read() {
    StereoLogD("read");
    // read simple value
    if (pListOfSimpleValue != nullptr) {
        for (auto iter = pListOfSimpleValue->begin();
                iter != pListOfSimpleValue->end(); iter++) {
            SimpleItem *pSimpleItem = *iter;
            if (pSimpleItem != nullptr && pSimpleItem->dest == mDest) {
                pXmpMeta->getPropertyString(
                    pSimpleItem->nameSpaceItem.nameSpace,
                    pSimpleItem->name, &(pSimpleItem->value));
                StereoLogD("readed simple value, namespace = %s, name = %s, value = %s",
                    pSimpleItem->nameSpaceItem.nameSpace.c_str(),
                    pSimpleItem->name.c_str(), pSimpleItem->value.c_str());
            }
        }
    }

    // read buffer item
    if (pListOfBufferItem != nullptr) {
        for (auto iter = pListOfBufferItem->begin();
                iter != pListOfBufferItem->end(); iter++) {
            BufferItem *pBufferItem = *iter;
            if (pBufferItem != nullptr && pBufferItem->dest == mDest) {
                pXmpMeta->getPropertyBase64(
                    pBufferItem->nameSpaceItem.nameSpace, pBufferItem->name, pBufferItem->value);
                StereoLogD("readed buffer item, namespace = %s, name = %s",
                    pBufferItem->nameSpaceItem.nameSpace.c_str(),
                    pBufferItem->name.c_str());
            }
        }
    }

    // read array item
    if (pListOfArrayItem != nullptr) {
        for (auto iter = pListOfArrayItem->begin();
                iter != pListOfArrayItem->end(); iter++) {
            ArrayItem *pArrayItem = *iter;
            if (pArrayItem != nullptr && pArrayItem->dest == mDest) {
                pXmpMeta->getArrayItem(
                    pArrayItem->nameSpaceItem.nameSpace, pArrayItem->name,
                    pArrayItem->index, &(pArrayItem->value));
                StereoLogD("readed array item, namespace = %s, name = %s, index = %d, value = %s",
                    pArrayItem->nameSpaceItem.nameSpace.c_str(), pArrayItem->name.c_str(),
                    pArrayItem->index, pArrayItem->value.c_str());
            }
        }
    }

    // read struct item
    if (pListOfStructItem != nullptr) {
        for (auto iter = pListOfStructItem->begin();
                iter != pListOfStructItem->end(); iter++) {
            StructItem *pStructItem = *iter;
            if (pStructItem != nullptr && pStructItem->dest == mDest) {
                // whether need register struct and field namespace at here,
                pXmpMeta->registerNamespace(pStructItem->structNameSpaceItem.nameSpace,
                    pStructItem->structNameSpaceItem.nameSpacePrefix);
                pXmpMeta->registerNamespace(pStructItem->fieldNameSpaceItem.nameSpace,
                    pStructItem->fieldNameSpaceItem.nameSpacePrefix);
                pXmpMeta->getStructField(
                    pStructItem->structNameSpaceItem.nameSpace, pStructItem->structName,
                    pStructItem->fieldNameSpaceItem.nameSpace, pStructItem->fieldName,
                    &(pStructItem->fieldValue));
                StereoLogD("readed struct item, struct namespace = %s, struct name = %s,\
field namesapce = %s, field name = %s, field value = %s",
                    pStructItem->structNameSpaceItem.nameSpace.c_str(),
                    pStructItem->structName.c_str(),
                    pStructItem->fieldNameSpaceItem.nameSpace.c_str(),
                    pStructItem->fieldName.c_str(),
                    pStructItem->fieldValue.c_str());
            }
        }
    }
}

void XmpMetaOperator::write() {
    StereoLogD("write");
    // write simple value
    if (pListOfSimpleValue != nullptr) {
        for (auto iter = pListOfSimpleValue->begin();
                iter != pListOfSimpleValue->end(); iter++) {
            SimpleItem *pSimpleItem = *iter;
            if (pSimpleItem != nullptr && pSimpleItem->dest == mDest
                    && !pSimpleItem->value.empty()) {
                pXmpMeta->registerNamespace(pSimpleItem->nameSpaceItem.nameSpace,
                    pSimpleItem->nameSpaceItem.nameSpacePrefix);
                pXmpMeta->setPropertyString(pSimpleItem->nameSpaceItem.nameSpace,
                    pSimpleItem->name, pSimpleItem->value);
                StereoLogD("wrote simple value, namespace = %s, name = %s, value = %s",
                    pSimpleItem->nameSpaceItem.nameSpace.c_str(),
                    pSimpleItem->name.c_str(), pSimpleItem->value.c_str());
            }
        }
    }

    // write buffer item
    if (pListOfBufferItem != nullptr) {
        for (auto iter = pListOfBufferItem->begin();
                iter != pListOfBufferItem->end(); iter++) {
            BufferItem *pBufferItem = *iter;
            if (pBufferItem != nullptr && pBufferItem->dest == mDest
                    && pBufferItem->value.isValid()) {
                pXmpMeta->registerNamespace(pBufferItem->nameSpaceItem.nameSpace,
                    pBufferItem->nameSpaceItem.nameSpacePrefix);
                pXmpMeta->setPropertyBase64(
                    pBufferItem->nameSpaceItem.nameSpace,
                    pBufferItem->name, pBufferItem->value);
                StereoLogD("wrote buffer item, namespace = %s, name = %s",
                    pBufferItem->nameSpaceItem.nameSpace.c_str(),
                    pBufferItem->name.c_str());
            }
        }
    }

    // write array item
    if (pListOfArrayItem != nullptr) {
        for (auto iter = pListOfArrayItem->begin();
                iter != pListOfArrayItem->end(); iter++) {
            ArrayItem *pArrayItem = *iter;
            if (pArrayItem != nullptr && pArrayItem->dest == mDest
                    && !pArrayItem->value.empty()) {
                pXmpMeta->registerNamespace(pArrayItem->nameSpaceItem.nameSpace,
                    pArrayItem->nameSpaceItem.nameSpacePrefix);
                pXmpMeta->setArrayItem(
                    pArrayItem->nameSpaceItem.nameSpace, pArrayItem->name,
                    pArrayItem->index, pArrayItem->value);
                StereoLogD("wrote array item, namespace = %s, name = %s, index = %d, value = %s",
                    pArrayItem->nameSpaceItem.nameSpace.c_str(), pArrayItem->name.c_str(),
                    pArrayItem->index, pArrayItem->value.c_str());
            }
        }
    }

    // write struct item
    if (pListOfStructItem != nullptr) {
        for (auto iter = pListOfStructItem->begin();
                iter != pListOfStructItem->end(); iter++) {
            StructItem *pStructItem = *iter;
            if (pStructItem != nullptr && pStructItem->dest == mDest
                    && !pStructItem->fieldValue.empty()) {
                pXmpMeta->registerNamespace(pStructItem->structNameSpaceItem.nameSpace,
                    pStructItem->structNameSpaceItem.nameSpacePrefix);
                pXmpMeta->registerNamespace(pStructItem->fieldNameSpaceItem.nameSpace,
                    pStructItem->fieldNameSpaceItem.nameSpacePrefix);
                pXmpMeta->setStructField(
                    pStructItem->structNameSpaceItem.nameSpace, pStructItem->structName,
                    pStructItem->fieldNameSpaceItem.nameSpace, pStructItem->fieldName,
                    pStructItem->fieldValue);
                StereoLogD("wrote struct item, struct namespace = %s, struct name = %s,\
field namesapce = %s, field name = %s, field value = %s",
                    pStructItem->structNameSpaceItem.nameSpace.c_str(),
                    pStructItem->structName.c_str(),
                    pStructItem->fieldNameSpaceItem.nameSpace.c_str(),
                    pStructItem->fieldName.c_str(),
                    pStructItem->fieldValue.c_str());
            }
        }
    }
}

BufferMapPtr XmpMetaOperator::serialize() {
    StereoLogD("serialize");
    BufferMapPtr result = new BufferMap();
    StereoString rdfString;
    pXmpMeta->serialize(&rdfString);
    StereoBuffer_t buffer;
    BufferManager::createBuffer(rdfString.length(), buffer);
    memcpy(buffer.data, &rdfString[0], buffer.size);
    result->insert(BufferMap::value_type(xmpKey, buffer));
    return result;
}

void XmpMetaOperator::setData(DataCollections *dataCollections) {
    StereoLogD("setData");
    if (dataCollections == nullptr) {
        StereoLogE("dataCollections is null");
        return;
    }
    mDest = dataCollections->dest;
    pListOfSimpleValue = dataCollections->listOfSimpleValue;
    pListOfBufferItem = dataCollections->listOfBufferItem;
    pListOfArrayItem = dataCollections->listOfArrayItem;
    pListOfStructItem = dataCollections->listOfStructItem;
}

