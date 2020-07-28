//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/CustomizedMetaOperator"
#include "StereoLog.h"

#include "MetaOperator.h"

using namespace stereo;

CustomizedMetaOperator::CustomizedMetaOperator(
        BufferMapPtr customizedDataBuffer) :
        pListOfCustDataItem(nullptr) {
    StereoLogD("CustomizedMetaOperator");
    pCustMeta = new CustomizedMeta(customizedDataBuffer);
}

CustomizedMetaOperator::~CustomizedMetaOperator() {
    StereoLogD("~CustomizedMetaOperator");
    if (pCustMeta != nullptr) {
        delete pCustMeta;
    }
}

void CustomizedMetaOperator::read() {
    StereoLogD("read");
    // read buffer item
    if (pListOfCustDataItem != nullptr) {
        for (auto iter = pListOfCustDataItem->begin();
                iter != pListOfCustDataItem->end(); iter++) {
            BufferItem *pBufferItem = *iter;
            if (pBufferItem != nullptr) {
                pCustMeta->getPropertyBuffer(pBufferItem->name, pBufferItem->value);
                StereoLogD("readed cust data item, name = %s", pBufferItem->name.c_str());
            }
        }
    }
}

void CustomizedMetaOperator::write() {
    StereoLogD("write");
    // write buffer item
    if (pListOfCustDataItem != nullptr) {
        for (auto iter = pListOfCustDataItem->begin();
                iter != pListOfCustDataItem->end(); iter++) {
            BufferItem *pBufferItem = *iter;
            if (pBufferItem != nullptr && pBufferItem->value.isValid()) {
                pCustMeta->setPropertyBuffer(pBufferItem->name, pBufferItem->value);
                StereoLogD("wrote cust data item, name = %s", pBufferItem->name.c_str());
            }
        }
    }
}

BufferMapPtr CustomizedMetaOperator::serialize() {
    StereoLogD("serialize");
    return pCustMeta->serialize();
}

void CustomizedMetaOperator::setData(DataCollections *dataCollections) {
    StereoLogD("setData");
    if (dataCollections == nullptr) {
        StereoLogE("dataCollections is null");
        return;
    }
    pListOfCustDataItem = dataCollections->listOfCustomDataItem;
}

