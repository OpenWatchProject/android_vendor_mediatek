//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/XmpPacker"

#include "XmpPacker.h"
#include "StereoLog.h"
#include "StereoInfo.h"
#include "PackUtils.h"
#include "BufferManager.h"
#include <utils/Trace.h>

using namespace stereo;

#define ATRACE_TAG ATRACE_TAG_ALWAYS

XmpPacker::XmpPacker(PackInfo *packInfo) {
    if (packInfo == nullptr) {
        throw std::invalid_argument("pack info can not be null");
    }
    pPackInfo = packInfo;
}

XmpPacker::~XmpPacker() {
    StereoLogD("~XmpPacker");
}

void XmpPacker::pack() {
    StereoLogD("<pack> begin");
    if (pPackInfo == nullptr) {
        StereoLogW("<pack> pPackInfo is null!");
        return;
    }

    ATRACE_NAME(">>>>XmpPacker-pack");
    if (pPackInfo->unpackedStandardXmpBuf.isValid()) {
        // packed buffer = xmp header + unpacker buffer
        int xmpHeaderStartLen = PackUtils::XMP_HEADER_START.length();
        int unpackedStandardXmpBufSize = pPackInfo->unpackedStandardXmpBuf.size;
        int packedStandardXmpBufSize = unpackedStandardXmpBufSize + xmpHeaderStartLen
            + PackUtils::STR_TERMINATOR_LENGTH;
        // new packed buffer
        BufferManager::createBuffer(packedStandardXmpBufSize, pPackInfo->packedStandardXmpBuf);
        // copy xmp header
        memcpy(pPackInfo->packedStandardXmpBuf.data,
                &PackUtils::XMP_HEADER_START[0], xmpHeaderStartLen);
        (pPackInfo->packedStandardXmpBuf.data)[xmpHeaderStartLen] = PackUtils::STR_TERMINATOR;
        // copy unpacked buffer
        memcpy(pPackInfo->packedStandardXmpBuf.data
                + xmpHeaderStartLen + PackUtils::STR_TERMINATOR_LENGTH,
                pPackInfo->unpackedStandardXmpBuf.data, unpackedStandardXmpBufSize);
    }

    if (pPackInfo->unpackedExtendedXmpBuf.isValid()) {
        pPackInfo->packedExtendedXmpBufArray
            = makeExtXmpData(pPackInfo->unpackedExtendedXmpBuf);
    }
    StereoLogD("<pack> end");
}

void XmpPacker::unpack() {
    StereoLogD("<unpack> begin");
    if (pPackInfo == nullptr) {
        StereoLogW("<unpack> pPackInfo is null!");
        return;
    }

    ATRACE_NAME(">>>>XmpPacker-unpack");
    if (pPackInfo->packedStandardXmpBuf.isValid()) {
        int packedStandardXmpBufferSize = pPackInfo->packedStandardXmpBuf.size;
        int xmpHeaderStartLen = PackUtils::XMP_HEADER_START.length()
                    + PackUtils::STR_TERMINATOR_LENGTH;
        int unpackedStandardXmpBufferSize
            = packedStandardXmpBufferSize - xmpHeaderStartLen;
        BufferManager::createBuffer(
                unpackedStandardXmpBufferSize, pPackInfo->unpackedStandardXmpBuf);
        // skip xmp header, and copy data to unpacked buffer
        memcpy(pPackInfo->unpackedStandardXmpBuf.data,
                pPackInfo->packedStandardXmpBuf.data + xmpHeaderStartLen,
                unpackedStandardXmpBufferSize);
    }

    if (pPackInfo->packedExtendedXmpBufArray != nullptr) {
        int totalFormatLength =
            PackUtils::XMP_EXT_HEADER.length()
            + PackUtils::STR_TERMINATOR_LENGTH
            + PackUtils::MD5_BYTE_COUNT
            + PackUtils::TOTAL_LENGTH_BYTE_COUNT
            + PackUtils::PARTITION_OFFSET_BYTE_COUNT;
        int sectionCount = pPackInfo->packedExtendedXmpBufArray->size();
        StereoLogD("<unpack> section count = %d", sectionCount);

        // calculate unpacked ext xmp buffer size
        int unpackedExtXmpBufferSize = 0;
        for (int i = 0; i < sectionCount; i++) {
            unpackedExtXmpBufferSize +=
                (*pPackInfo->packedExtendedXmpBufArray)[i].size - totalFormatLength;
        }
        StereoLogD("<unpack> unpacked ext xmp buffer size = %d", unpackedExtXmpBufferSize);

        BufferManager::createBuffer(unpackedExtXmpBufferSize, pPackInfo->unpackedExtendedXmpBuf);
        int currentPos = 0;
        for (int i = 0; i < sectionCount; i++) {
            StereoBuffer_t &section = (*pPackInfo->packedExtendedXmpBufArray)[i];
            memcpy(pPackInfo->unpackedExtendedXmpBuf.data + currentPos,
                    section.data + totalFormatLength, section.size - totalFormatLength);
            currentPos += section.size - totalFormatLength;
        }
    }
    StereoLogD("<unpack> end");
}

StereoVector<StereoBuffer_t>* XmpPacker::makeExtXmpData(StereoBuffer_t &extXmpData) {
    StereoLogD("<makeExtXmpData>");
    StereoVector<StereoBuffer_t> *pExtXmpDataArray = new StereoVector<StereoBuffer_t>();
    // calculate md5 of ext xmp data
    StereoString md5 = PackUtils::getMD5(extXmpData);
    StereoLogD("<makeExtXmpData> md5=%s", md5.c_str());

    // get section count
    int sectionCount = 0;
    if (extXmpData.size % PackUtils::MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1 == 0) {
        sectionCount =
            extXmpData.size / PackUtils::MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1;
    } else {
        sectionCount =
            extXmpData.size / PackUtils::MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1 + 1;
    }
    StereoLogD("<makeExtXmpData> section count = %d", sectionCount);
    pExtXmpDataArray->resize(sectionCount);

    StereoBuffer_t section;
    int currentPos = 0;
    for (int i = 0; i < sectionCount; i++) {
        StereoBuffer_t commonHeader;
        getXmpCommonHeader(md5, extXmpData.size, i, commonHeader);
        // if last section
        if (i == sectionCount - 1
                && extXmpData.size % PackUtils::MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1 != 0) {
            int sectionLen = extXmpData.size
                % PackUtils::MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1 + commonHeader.size;
            BufferManager::createBuffer(sectionLen, section);
            // 1. copy header
            memcpy(section.data, commonHeader.data, commonHeader.size);
            // 2. copy data
            memcpy(section.data + commonHeader.size,
                    extXmpData.data + currentPos, extXmpData.size - currentPos);
            currentPos += sectionLen - commonHeader.size;
        } else { // not last section
            BufferManager::createBuffer(PackUtils::MAX_BYTE_PER_APP1, section);
            // 1. copy header
            memcpy(section.data, commonHeader.data, commonHeader.size);
            // 2. copy data
            memcpy(section.data + commonHeader.size, extXmpData.data + currentPos,
                    PackUtils::MAX_BYTE_PER_APP1 - commonHeader.size);
            currentPos += PackUtils::MAX_BYTE_PER_APP1 - commonHeader.size;
        }
        (*pExtXmpDataArray)[i] = section;
    }
    return pExtXmpDataArray;
}

void XmpPacker::getXmpCommonHeader(
        StereoString md5, int totalLength, int sectionNumber, StereoBuffer_t &header) {
    BufferManager::createBuffer(PackUtils::XMP_COMMON_HEADER_LEN, header);
    int offset = PackUtils::MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1 * sectionNumber;
    int currentPos = 0;
    // 1. copy header
    memcpy(header.data, &PackUtils::XMP_EXT_HEADER[0], PackUtils::XMP_EXT_HEADER.length());
    currentPos += PackUtils::XMP_EXT_HEADER.length();
    // 2. copy tail byte
    header.data[currentPos] = PackUtils::STR_TERMINATOR;
    currentPos += PackUtils::STR_TERMINATOR_LENGTH;
    // 3. copy md5
    memcpy(header.data + currentPos, &md5[0], md5.length());
    currentPos += md5.length();
    // 4. copy 4 bytes total length
    StereoBuffer_t totalLengthBuffer;
    BufferManager::createBuffer(PackUtils::BYTE_COUNT_4, totalLengthBuffer);
    PackUtils::intToByteBuffer(totalLength, totalLengthBuffer);
    memcpy(header.data + currentPos, totalLengthBuffer.data, totalLengthBuffer.size);
    currentPos += totalLengthBuffer.size;
    // copy 4 bytes offset
    StereoBuffer_t offsetBuffer;
    BufferManager::createBuffer(PackUtils::BYTE_COUNT_4, offsetBuffer);
    PackUtils::intToByteBuffer(offset, offsetBuffer);
    memcpy(header.data + currentPos, offsetBuffer.data, offsetBuffer.size);
}
