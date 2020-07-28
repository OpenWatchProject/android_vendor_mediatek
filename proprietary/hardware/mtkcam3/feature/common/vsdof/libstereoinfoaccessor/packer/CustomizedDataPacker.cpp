//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/CustomizedDataPacker"

#include "CustomizedDataPacker.h"
#include "StereoLog.h"
#include "StereoInfo.h"
#include "PackUtils.h"
#include "BufferManager.h"
#include "Utils.h"
#include <utils/Trace.h>

using namespace stereo;

#define ATRACE_TAG ATRACE_TAG_ALWAYS

CustomizedDataPacker::CustomizedDataPacker(PackInfo *packInfo) {
    if (packInfo == nullptr) {
        throw std::invalid_argument("pack info can not be null");
    }
    pPackInfo = packInfo;
}

CustomizedDataPacker::~CustomizedDataPacker() {
    StereoLogD("~CustomizedDataPacker");
}

void CustomizedDataPacker::pack() {
    StereoLogD("<pack> begin");
    if (pPackInfo == nullptr) {
        StereoLogW("<pack> pPackInfo is null!");
        return;
    }

    if (pPackInfo->unpackedCustomizedBufMap == nullptr) {
        StereoLogW("<pack> unpackedCustomizedBufMap is null!");
        return;
    }

    ATRACE_NAME(">>>>CustomizedDataPacker-pack");
    StereoVector<StereoBuffer_t>* custDst = nullptr;
    StereoVector<StereoBuffer_t>* packedCustomizedBufArray = new StereoVector<StereoBuffer_t>();
    packedCustomizedBufArray->reserve(100);
    for (auto iter = pPackInfo->unpackedCustomizedBufMap->begin();
            iter != pPackInfo->unpackedCustomizedBufMap->end(); iter++) {
        StereoString type = iter->first;
        if (type.empty()) {
            continue;
        }
        custDst = pack(iter->second, type);
        if (!custDst->empty()) {
            packedCustomizedBufArray->insert(packedCustomizedBufArray->end(),
                custDst->begin(), custDst->end());
            // delete cusDst
            custDst->clear();
        }
        delete custDst;
        custDst = nullptr;
    }
    pPackInfo->packedCustomizedBufArray = packedCustomizedBufArray;
    StereoLogD("<unpack> end");
}

void CustomizedDataPacker::unpack() {
    StereoLogD("<unpack> begin");
    if (pPackInfo == nullptr) {
        StereoLogW("<pack> pPackInfo is null!");
        return;
    }

    if (pPackInfo->packedCustomizedBufArray == nullptr) {
        StereoLogW("<pack> packedCustomizedBufArray is null!");
        return;
    }

    ATRACE_NAME(">>>>CustomizedDataPacker-unpack");
    BufferMapPtr unpackedCustomizedBufMap = new BufferMap();
    StereoMap<StereoString, int> typePosMap;
    int bufferCount = pPackInfo->packedCustomizedBufArray->size();
    for (int i = 0; i < bufferCount; i++) {
        StereoBuffer_t &section = (*pPackInfo->packedCustomizedBufArray)[i];
        if (!section.isValid()) {
            continue;
        }
        // get type
        StereoString type = Utils::buffer2Str(
                section, PackUtils::CUSTOMIZED_TOTAL_LENGTH, PackUtils::TYPE_BUFFER_COUNT);
        // get total length
        unsigned int totalLength = PackUtils::byteBufferToInt(
            section, 0, PackUtils::CUSTOMIZED_TOTAL_LENGTH);
        // find unpacked cust buffer by type from unpackedCustomizedBufMap
        auto iter = unpackedCustomizedBufMap->find(type);
        StereoBuffer_t custItemBuffer;
        if (iter != unpackedCustomizedBufMap->end()) {
            custItemBuffer = iter->second;
        } else {
            BufferManager::createBuffer(totalLength, custItemBuffer);
            unpackedCustomizedBufMap->insert(BufferMap::value_type(type, custItemBuffer));
        }
        // find current type position
        int pos = typePosMap[type];
        // get customized buffer
        int bufferSize = section.size - PackUtils::CUSTOMIZED_TOTAL_FORMAT_LENGTH;
        // copy buffer
        memcpy(custItemBuffer.data + pos, section.data + PackUtils::CUSTOMIZED_TOTAL_FORMAT_LENGTH,
                section.size - PackUtils::CUSTOMIZED_TOTAL_FORMAT_LENGTH);
        // update position
        pos += bufferSize;
        typePosMap[type] = pos;
    }
    pPackInfo->unpackedCustomizedBufMap = unpackedCustomizedBufMap;
    StereoLogD("<unpack> end");
}

StereoVector<StereoBuffer_t>* CustomizedDataPacker::pack(
        StereoBuffer_t &bufferData, StereoString type) {
    StereoLogD("<pack> type name is %s", type.c_str());
    int maxBufferContentLength =
                PackUtils::MAX_BYTE_PER_APP1 - PackUtils::CUSTOMIZED_TOTAL_FORMAT_LENGTH;
    int sectionCount = 0;
    if (bufferData.size % maxBufferContentLength == 0) {
        sectionCount = bufferData.size / maxBufferContentLength;
    } else {
        sectionCount = bufferData.size / maxBufferContentLength + 1;
    }
    StereoLogD("<pack> section count = %d", sectionCount);

    StereoVector<StereoBuffer_t>* custDst = new StereoVector<StereoBuffer_t>(sectionCount);

    int sectionLen = 0;
    int sectionCurrentPos = 0;
    int bufferCurrentPos = 0;
    StereoBuffer_t section;
    for (int i = 0; i < sectionCount; i++) {
        // if last section
        if (i == sectionCount - 1 && bufferData.size % maxBufferContentLength != 0) {
            sectionLen = bufferData.size % maxBufferContentLength
                + PackUtils::CUSTOMIZED_TOTAL_FORMAT_LENGTH;
        } else {
            sectionLen = PackUtils::MAX_BYTE_PER_APP1;
        }

        BufferManager::createBuffer(sectionLen, section);

        sectionCurrentPos = 0;

        // 1. copy total length(4bytes)
        StereoBuffer_t totalLengthBuffer;
        BufferManager::createBuffer(PackUtils::BYTE_COUNT_4, totalLengthBuffer);
        PackUtils::intToByteBuffer(bufferData.size, totalLengthBuffer);
        memcpy(section.data + sectionCurrentPos, totalLengthBuffer.data, totalLengthBuffer.size);
        sectionCurrentPos += totalLengthBuffer.size;

        // 2. copy type buffer(7bytes)
        if (type.length() != PackUtils::TYPE_BUFFER_COUNT) {
            StereoLogW("<pack> type length is not %d, type = %s",
                PackUtils::TYPE_BUFFER_COUNT, type.c_str());
        }
        memcpy(section.data + sectionCurrentPos, &type[0], type.length());
        sectionCurrentPos += PackUtils::TYPE_BUFFER_COUNT;

        // 3. copy serial number(1byte)
        StereoBuffer_t serialNumberBuffer;
        BufferManager::createBuffer(PackUtils::CUSTOMIZED_SERIAL_NUMBER_LENGTH, serialNumberBuffer);
        PackUtils::intToByteBuffer(i, serialNumberBuffer);
        memcpy(section.data + sectionCurrentPos, serialNumberBuffer.data, serialNumberBuffer.size);
        sectionCurrentPos += PackUtils::CUSTOMIZED_SERIAL_NUMBER_LENGTH;

        // 4. copy buffer
        int copyDataLength = sectionLen - PackUtils::CUSTOMIZED_TOTAL_FORMAT_LENGTH;
        memcpy(section.data + sectionCurrentPos,
                bufferData.data + bufferCurrentPos, copyDataLength);
        bufferCurrentPos += copyDataLength;

        // assign
        (*custDst)[i] = section;
    }
    return custDst;
}