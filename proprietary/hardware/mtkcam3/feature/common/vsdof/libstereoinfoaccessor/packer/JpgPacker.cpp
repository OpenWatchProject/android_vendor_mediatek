//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/JpgPacker"

#include "JpgPacker.h"
#include "StereoLog.h"
#include "StereoInfo.h"
#include "PackUtils.h"
#include "Utils.h"
#include <sstream>
#include "BufferManager.h"
#include <utils/Trace.h>

using namespace stereo;

#define ATRACE_TAG ATRACE_TAG_ALWAYS

JpgPacker::JpgPacker(PackInfo *packInfo) {
    if (packInfo == nullptr) {
        throw std::invalid_argument("pack info can not be null");
    }
    pPackInfo = packInfo;
}

JpgPacker::~JpgPacker() {
    StereoLogD("~JpgPacker");
}

void JpgPacker::pack() {
    StereoLogD("<pack> begin");
    if (pPackInfo == nullptr) {
        StereoLogW("<pack> pPackInfo is null!");
        return;
    }
    if (!pPackInfo->unpackedJpgBuf.isValid()) {
        StereoLogW("<pack> unpackedJpgBuf is null!");
        return;
    }

    ATRACE_NAME(">>>>JpgPacker-pack");
    Section *pStandardSection = nullptr;
    StereoVector<Section*> *pExtendedSections = nullptr;
    StereoVector<Section*> *pCustomizedSections = nullptr;

    if (pPackInfo->packedStandardXmpBuf.isValid()) {
        pStandardSection = new Section(
            PackUtils::APP1, 0, pPackInfo->packedStandardXmpBuf.size + 2);
        pStandardSection->buffer = pPackInfo->packedStandardXmpBuf;
        pStandardSection->type = PackUtils::TYPE_STANDARD_XMP;
    }

    if (pPackInfo->packedExtendedXmpBufArray != nullptr) {
        pExtendedSections = makeJpgSections(
            PackUtils::APP1, pPackInfo->packedExtendedXmpBufArray);
    }

    if (pPackInfo->packedCustomizedBufArray != nullptr) {
        pCustomizedSections = makeJpgSections(
            PackUtils::APP15, pPackInfo->packedCustomizedBufArray);
    }

    BufferTypeInputStream is(pPackInfo->unpackedJpgBuf);
    BufferTypeOutputStream os(1024);
    pack(is, os, pStandardSection, pExtendedSections, pCustomizedSections);

    os.toBuffer(pPackInfo->packedJpgBuf);

    // release sections
    if (pStandardSection != nullptr) {
        delete pStandardSection;
    }
    if (pExtendedSections != nullptr) {
        for (auto iter = pExtendedSections->begin();
                iter != pExtendedSections->end(); iter++) {
            if (*iter != nullptr) {
                delete *iter;
            }
        }
        delete pExtendedSections;
    }

    for (auto iter = pCustomizedSections->begin();
            iter != pCustomizedSections->end(); iter++) {
        if (*iter != nullptr) {
            delete *iter;
        }
    }
    delete pCustomizedSections;

    StereoLogD("<pack> end");
}

void JpgPacker::unpack() {
    StereoLogD("<unpack> begin");
    if (pPackInfo == nullptr) {
        StereoLogW("<unpack> pPackInfo is null!");
        return;
    }
    if (!pPackInfo->packedJpgBuf.isValid()) {
        StereoLogW("<unpack> packedJpgBuf is null!");
        return;
    }

    ATRACE_NAME(">>>>JpgPacker-unpack");
    BufferTypeInputStream is(pPackInfo->packedJpgBuf);
    StereoVector<Section*>* srcJpgSections = parseAppInfoFromStream(is);
    StereoBuffer_t standardXmp;
    StereoVector<StereoBuffer_t> *pExtendedXmp = new StereoVector<StereoBuffer_t>();
    StereoVector<StereoBuffer_t> *pCustDataBuffer = new StereoVector<StereoBuffer_t>();
    int srcJpgSectionsSize = srcJpgSections->size();
    for (int i = 0; i < srcJpgSectionsSize; i++) {
        Section* sec = (*srcJpgSections)[i];
        if (PackUtils::TYPE_STANDARD_XMP == sec->type) {
            is.seek(sec->offset + PackUtils::APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT);
            BufferManager::createBuffer(sec->length - 2, standardXmp);
            is.read(standardXmp);
        }
        if (PackUtils::TYPE_EXTENDED_XMP == sec->type) {
            is.seek(sec->offset + PackUtils::APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT);
            StereoBuffer_t extendedXmpItem;
            BufferManager::createBuffer(sec->length - 2, extendedXmpItem);
            is.read(extendedXmpItem);
            pExtendedXmp->push_back(extendedXmpItem);
        }
        if (PackUtils::TYPE_DEPTH_DATA == sec->type
            || PackUtils::TYPE_JPS_DATA == sec->type
            || PackUtils::TYPE_JPS_MASK == sec->type
            || PackUtils::TYPE_SEGMENT_MASK == sec->type
            || PackUtils::TYPE_CLEAR_IMAGE == sec->type
            || PackUtils::TYPE_LDC_DATA == sec->type
            || PackUtils::TYPE_DEBUG_BUFFER == sec->type) {
            is.seek(sec->offset + PackUtils::APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT);
            StereoBuffer_t custDataItem;
            BufferManager::createBuffer(sec->length - 2, custDataItem);
            is.read(custDataItem);
            pCustDataBuffer->push_back(custDataItem);
        }
    }

    // these buffer will released when packinfo destroyed and jni invoke ended
    pPackInfo->packedStandardXmpBuf = standardXmp;
    pPackInfo->packedExtendedXmpBufArray = pExtendedXmp;
    pPackInfo->packedCustomizedBufArray = pCustDataBuffer;

    for (auto iter = srcJpgSections->begin();
            iter != srcJpgSections->end(); iter++) {
        if (*iter != nullptr) {
            delete *iter;
        }
    }
    delete srcJpgSections;

    StereoLogD("<unpack> end");
}

StereoVector<Section*>* JpgPacker::makeJpgSections(
        int marker, StereoVector<StereoBuffer_t>* buffers) {
    StereoLogD("<makeJpgSection>");
    int bufferCount = buffers->size();
    StereoVector<Section*> *jpgSections = new StereoVector<Section*>(bufferCount);
    Section *section = nullptr;
    for (int i = 0; i < bufferCount; i++) {
        StereoBuffer_t &buffer = (*buffers)[i];
        if (!buffer.isValid()) {
            continue;
        }
        section = new Section(marker, 0, buffer.size + 2);
        if (marker == PackUtils::APP1) {
            section->type = PackUtils::TYPE_EXTENDED_XMP;
        } else {
            section->type = PackUtils::getCustomTypeName(buffer);
        }
        section->buffer = buffer;
        (*jpgSections)[i] = section;
    }
    return jpgSections;
}

StereoVector<Section*>* JpgPacker::parseAppInfoFromStream(BufferTypeInputStream &is) {
    // reset position at the file start
    is.seek(0);
    int value = is.readUnsignedShort();
    if (value != PackUtils::SOI) {
        StereoLogE("<parseAppInfoFromStream> error, find no SOI");
        return new StereoVector<Section*>();
    }
    StereoLogD("<parseAppInfoFromStream> parse begin!!!");

    StereoVector<Section*> *sections = new StereoVector<Section*>();
    sections->reserve(50);

    int marker = -1;
    long offset = -1;
    int length = -1;
    while ((value = is.readUnsignedShort()) != -1 && value != PackUtils::SOS) {
        marker = value;
        offset = is.getFilePointer() - 2;
        length = is.readUnsignedShort();
        Section *sec = new Section(marker, offset, length);
        sections->push_back(sec);
        is.skip(length - 2);
    }

    for (int i = 0; i < sections->size(); i++) {
        checkAppSectionTypeInStream(is, (*sections)[i]);
    }

    // reset position at the file start
    is.seek(0);
    StereoLogD("<parseAppInfoFromStream> parse end!!!");
    return sections;
}

void JpgPacker::checkAppSectionTypeInStream(BufferTypeInputStream &is, Section *section) {
    if (section == nullptr) {
        StereoLogW("<checkAppSectionTypeInStream> input stream or section is null!!!");
        return;
    }

    if (section->marker == PackUtils::APP15) {
        is.seek(section->offset + PackUtils::APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT
            + PackUtils::CUSTOMIZED_TOTAL_LENGTH);
        StereoBuffer_t buffer;
        BufferManager::createBuffer(PackUtils::TYPE_BUFFER_COUNT, buffer);
        is.read(buffer);
        StereoString type = Utils::buffer2Str(buffer, buffer.size);
        if (PackUtils::TYPE_JPS_DATA == type
                || PackUtils::TYPE_JPS_MASK == type
                || PackUtils::TYPE_DEPTH_DATA == type
                || PackUtils::TYPE_SEGMENT_MASK == type
                || PackUtils::TYPE_CLEAR_IMAGE == type
                || PackUtils::TYPE_LDC_DATA == type
                || PackUtils::TYPE_DEBUG_BUFFER == type) {
            section->type = type;
            return;
        }
        section->type = PackUtils::TYPE_UNKNOW_APP15;
        return;
    } else if (section->marker == PackUtils::APP1) {
        is.seek(section->offset + PackUtils::APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT);
        StereoBuffer_t buffer;
        BufferManager::createBuffer(PackUtils::XMP_EXT_HEADER.length(), buffer);
        is.read(buffer);
        StereoString type = Utils::buffer2Str(buffer, buffer.size);
        if (PackUtils::XMP_EXT_HEADER == type) {
            // ext main header is same as ext slave header
            section->type = PackUtils::TYPE_EXTENDED_XMP;
            return;
        }
        type = Utils::buffer2Str(buffer, PackUtils::XMP_HEADER_START.length());
        if (PackUtils::XMP_HEADER_START == type) {
            section->type = PackUtils::TYPE_STANDARD_XMP;
            return;
        }
        type = Utils::buffer2Str(buffer, PackUtils::EXIF_HEADER.length());
        if (PackUtils::EXIF_HEADER == type) {
            section->type = PackUtils::TYPE_EXIF;
            return;
        }
    }
}

int JpgPacker::findProperLocationForXmp(StereoVector<Section*> *sections) {
    for (int i = 0; i < sections->size(); i++) {
        Section *sec = (*sections)[i];
        if (sec->marker == PackUtils::APP1) {
            if (PackUtils::TYPE_EXIF == sec->type) {
                return PackUtils::WRITE_XMP_AFTER_FIRST_APP1;
            } else {
                return PackUtils::WRITE_XMP_BEFORE_FIRST_APP1;
            }
        }
    }
    // means no app1, write after SOI
    return PackUtils::WRITE_XMP_AFTER_SOI;
}

StereoString JpgPacker::getSectionTag(Section *section) {
    std::stringstream ss;
    ss << "marker " << Utils::intToHexString(section->marker)
        << ", offset " << Utils::intToHexString(section->offset)
        << ", length " << Utils::intToHexString(section->length)
        << ", type " << section->type;
    return ss.str();
}

void JpgPacker::writeSectionToStream(BufferTypeInputStream &is, BufferTypeOutputStream &os,
        Section *sec) {
    if (sec == nullptr) {
        StereoLogD("<writeSectionToStream> section is null, return");
        return;
    }
    StereoLogD("<writeSectionToStream> sec.type = %s", sec->type.c_str());
    os.writeShort(sec->marker);
    os.writeShort(sec->length);
    is.seek(sec->offset + PackUtils::APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT);
    StereoBuffer_t buffer;
    BufferManager::createBuffer(sec->length - 2, buffer);
    is.read(buffer);
    os.write(buffer);
}

void JpgPacker::writeSectionToStream(BufferTypeOutputStream &os, Section *sec) {
    if (sec == nullptr) {
        StereoLogD("<writeSectionToStream> section is null, return");
        return;
    }
    StereoLogD("<writeSectionToStream> sec.type = %s", sec->type.c_str());
    os.writeShort(sec->marker);
    os.writeShort(sec->length);
    os.write(sec->buffer);
}

void JpgPacker::writeCust(BufferTypeOutputStream &os,
                       StereoVector<Section*> *pCustomizedSections) {
    if (pCustomizedSections == nullptr) {
        StereoLogW("<writeCust> pCustomizedSections is null, return");
        return;
    }
    int customizedSectionsSize = pCustomizedSections->size();
    StereoLogD("<writeCust> customizedSections size: %d", customizedSectionsSize);
    if (customizedSectionsSize == 0) {
        return;
    }
    for (int i = 0; i < customizedSectionsSize; i++) {
        writeSectionToStream(os, (*pCustomizedSections)[i]);
    }
}

void JpgPacker::writeXmp(BufferTypeOutputStream &os,
                      Section *pStandardSection, StereoVector<Section*> *pExtendedSections) {
    if (pStandardSection != nullptr) {
        StereoLogD("<writeXmp> standardxmp");
        writeSectionToStream(os, pStandardSection);
    }
    if (pExtendedSections != nullptr) {
        int extendedSectionsSize = pExtendedSections->size();
        StereoLogD("<writeXmp> extendedSections size: %d", extendedSectionsSize);
        if (extendedSectionsSize == 0) {
            return;
        }
        for (int i = 0; i < extendedSectionsSize; i++) {
            writeSectionToStream(os, (*pExtendedSections)[i]);
        }
    }
}

void JpgPacker::copyToStreamWithFixBuffer(BufferTypeInputStream &is,
                                    BufferTypeOutputStream &os) {
    StereoBuffer_t readBuffer;
    BufferManager::createBuffer(PackUtils::FIXED_BUFFER_SIZE, readBuffer);
    int readCount = 0;
    while ((readCount = is.read(readBuffer)) != -1) {
        os.write(readBuffer);
    }
}

void JpgPacker::writeImageBuffer(BufferTypeInputStream &blurImageIs,
                                    BufferTypeOutputStream &os) {
    // reset position at the file start
    blurImageIs.seek(0);
    int value = blurImageIs.readUnsignedShort();
    int length = -1;

    // find DQT position
    while ((value = blurImageIs.readUnsignedShort()) != -1 && value != PackUtils::DQT) {
        length = blurImageIs.readUnsignedShort();
        blurImageIs.skip(length - 2);
    }

    // copy from DQT
    blurImageIs.seek(blurImageIs.getFilePointer() - 2);
    int readCount = 0;
    StereoBuffer_t readBuffer;
    BufferManager::createBuffer(PackUtils::FIXED_BUFFER_SIZE, readBuffer);
    while ((readCount = blurImageIs.read(readBuffer)) != -1) {
        os.write(readBuffer);
    }
}

void JpgPacker::pack(BufferTypeInputStream &is, BufferTypeOutputStream &os,
            Section *pStandardSection, StereoVector<Section*> *pExtendedSections,
            StereoVector<Section*> *pCustomizedSections) {
    StereoLogD("<pack> write begin!!!");

    StereoVector<Section*> *srcJpgSections = parseAppInfoFromStream(is);
    os.writeShort(PackUtils::SOI);
    int writenLocation = findProperLocationForXmp(srcJpgSections);
    bool hasWrittenXmp = false;
    bool hasWrittenCustomizedData = false;
    bool needWriteBlurImage = pPackInfo->unpackedBlurImageBuf.isValid();
    bool hasWrittenBlurImage = false;
    if (writenLocation == PackUtils::WRITE_XMP_AFTER_SOI) {
        // means no APP1
        StereoLogD("<pack> No APP1 information!");
        writeXmp(os, pStandardSection, pExtendedSections);
        hasWrittenXmp = true;
    }
    for (int i = 0; i < srcJpgSections->size(); i++) {
        Section *sec = (*srcJpgSections)[i];
        if (PackUtils::TYPE_EXIF == sec->type) {
            StereoLogD("<pack> write exif, %s", getSectionTag(sec).c_str());
            writeSectionToStream(is, os, sec);
            if (!hasWrittenXmp) {
                writeXmp(os, pStandardSection, pExtendedSections);
                hasWrittenXmp = true;
            }
        } else {
            if (!hasWrittenXmp) {
                StereoLogD("<pack> write xmp, %s", getSectionTag(sec).c_str());
                writeXmp(os, pStandardSection, pExtendedSections);
                hasWrittenXmp = true;
            }
            // APPx must be before DQT/DHT
            if (!hasWrittenCustomizedData
                    && (sec->marker == PackUtils::DQT
                    || sec->marker == PackUtils::DHT)) {
                StereoLogD("<pack> write custom, %s", getSectionTag(sec).c_str());
                writeCust(os, pCustomizedSections);
                hasWrittenCustomizedData = true;
            }
            // write blur image
            if (needWriteBlurImage && !hasWrittenBlurImage
                    && sec->marker == PackUtils::DQT) {
                StereoLogD("<pack> copy blur image to output stream");
                StereoLogD("<pack> write blur, %s", getSectionTag(sec).c_str());
                BufferTypeInputStream blurImageIs(pPackInfo->unpackedBlurImageBuf);
                writeImageBuffer(blurImageIs, os);
                hasWrittenBlurImage = true;

                for (auto iter = srcJpgSections->begin();
                        iter != srcJpgSections->end(); iter++) {
                    if (*iter != nullptr) {
                        delete *iter;
                    }
                }
                delete srcJpgSections;
                StereoLogD("<pack> write end!!!");
                return;
            }
            if (PackUtils::TYPE_DEPTH_DATA == sec->type
                    || PackUtils::TYPE_JPS_DATA == sec->type
                    || PackUtils::TYPE_JPS_MASK == sec->type
                    || PackUtils::TYPE_SEGMENT_MASK == sec->type
                    || PackUtils::TYPE_STANDARD_XMP == sec->type
                    || PackUtils::TYPE_EXTENDED_XMP == sec->type
                    || PackUtils::TYPE_CLEAR_IMAGE == sec->type
                    || PackUtils::TYPE_LDC_DATA == sec->type
                    || PackUtils::TYPE_DEBUG_BUFFER == sec->type) {
                // skip old data
                is.skip(sec->length + 2);
                StereoLogD("<pack> skip old data, type: %s", sec->type.c_str());
            } else {
                StereoLogD("<pack> write other info, %s", getSectionTag(sec).c_str());
                writeSectionToStream(is, os, sec);
            }
        }
    }
    // write jps and mask to app15, before sos
    if (!hasWrittenCustomizedData) {
        writeCust(os, pCustomizedSections);
        hasWrittenCustomizedData = true;
    }
    // write remain whole file (from SOS)
    if (!hasWrittenBlurImage) {
        StereoLogD("<pack> write remain whole file (from SOS)");
        copyToStreamWithFixBuffer(is, os);
    }

    for (auto iter = srcJpgSections->begin();
            iter != srcJpgSections->end(); iter++) {
        if (*iter != nullptr) {
            delete *iter;
        }
    }
    delete srcJpgSections;
    StereoLogD("<pack> write end!!!");
}