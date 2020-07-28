//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/CustomizedMeta"

#include "CustomizedMeta.h"
#include "StereoLog.h"

using namespace stereo;

CustomizedMeta::CustomizedMeta(BufferMapPtr custData) {
    StereoLogD("CustomizedMeta");
    if (custData != nullptr && !custData->empty()) {
        pCustData = custData;
    } else {
        pCustData = new BufferMap();
    }
}

CustomizedMeta::~CustomizedMeta() {
    StereoLogD("~CustomizedMeta");
    /*
     * The buffer will deleted in PackInfo's destructor
    if (pCustData != nullptr) {
        delete pCustData;
    }*/
}

bool CustomizedMeta::getPropertyBuffer(const StereoString & name, StereoBuffer_t &buffer) {
    StereoLogD("getPropertyBuffer, name = %s", name.c_str());
    if (pCustData == nullptr) {
        StereoLogE("pCustData is nullptr, can not get property buffer");
        return false;
    }
    auto iter = pCustData->find(name);
    if (iter != pCustData->end()) {
        buffer = iter->second;
        return true;
    } else {
        StereoLogW("can not find property");
        return false;
    }
}

void CustomizedMeta::setPropertyBuffer(const StereoString & name, StereoBuffer_t &buffer) {
    StereoLogD("setPropertyBuffer, name = %s", name.c_str());
    if (pCustData == nullptr) {
        StereoLogE("pCustData is nullptr, can not set property buffer");
        return;
    }
    auto iter = pCustData->find(name);
    if (iter == pCustData->end()) {
        pCustData->insert(BufferMap::value_type(name, buffer));
    } else {
        iter->second = buffer;
    }
}

BufferMapPtr CustomizedMeta::serialize() {
    StereoLogD("serialize");
    return pCustData;
}


