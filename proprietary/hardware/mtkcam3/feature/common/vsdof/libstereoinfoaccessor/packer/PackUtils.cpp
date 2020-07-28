//#define LOG_NDEBUG 0
#define LOG_TAG "StereoInfoAccessor/PackUtils"

#include "PackUtils.h"
#include "StereoLog.h"
#include "MD5.h"
#include "Utils.h"

using namespace stereo;

const int PackUtils::SOI = 0xFFD8;
const int PackUtils::SOS = 0xFFDA;
const int PackUtils::APP1 = 0xFFE1;
const int PackUtils::APP15 = 0xFFEF;
const int PackUtils::DQT = 0xFFDB;
const int PackUtils::DHT = 0xFFC4;
const int PackUtils::APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT = 4;
const int PackUtils::APP15_LENGTHTAG_BYTE_COUNT = 4;
const int PackUtils::TYPE_BUFFER_COUNT = 7;
const int PackUtils::WRITE_XMP_AFTER_SOI = 0;
const int PackUtils::WRITE_XMP_BEFORE_FIRST_APP1 = 1;
const int PackUtils::WRITE_XMP_AFTER_FIRST_APP1 = 2;
const int PackUtils::FIXED_BUFFER_SIZE = 1024 * 10;

const StereoString PackUtils::XMP_EXT_HEADER = "http://ns.adobe.com/xmp/extension/";
const int PackUtils::MAX_BYTE_PER_APP1 = 0XFFB2;
const int PackUtils::MD5_BYTE_COUNT = 32;
const int PackUtils::TOTAL_LENGTH_BYTE_COUNT = 4;
const int PackUtils::PARTITION_OFFSET_BYTE_COUNT = 4;
const int PackUtils::XMP_COMMON_HEADER_LEN =
            PackUtils::XMP_EXT_HEADER.length() + 1
            + PackUtils::MD5_BYTE_COUNT
            + PackUtils::TOTAL_LENGTH_BYTE_COUNT
            + PackUtils::PARTITION_OFFSET_BYTE_COUNT;
const int PackUtils::MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1 =
            PackUtils::MAX_BYTE_PER_APP1
            - PackUtils::XMP_COMMON_HEADER_LEN;

const StereoString PackUtils::APP_SECTION_MAX_LENGTH = "0xffb2";
const StereoString PackUtils::TYPE_JPS_DATA = "JPSDATA";
const StereoString PackUtils::TYPE_JPS_MASK = "JPSMASK";
const StereoString PackUtils::TYPE_DEPTH_DATA = "DEPTHBF";
const StereoString PackUtils::TYPE_DEBUG_BUFFER = "DEBUGBF";
const StereoString PackUtils::TYPE_XMP_DEPTH = "XMPDEPT";
const StereoString PackUtils::TYPE_SEGMENT_MASK = "SEGMASK";
const StereoString PackUtils::TYPE_CLEAR_IMAGE = "CLRIMAG";
const StereoString PackUtils::TYPE_LDC_DATA = "LDCDATA";

const StereoString PackUtils::TYPE_STANDARD_XMP = "standardXmp";
const StereoString PackUtils::TYPE_EXTENDED_XMP = "extendedXmp";
const StereoString PackUtils::TYPE_UNKNOW_APP15 = "unknownApp15";
const StereoString PackUtils::TYPE_EXIF = "exif";
const StereoString PackUtils::EXIF_HEADER = "Exif";
const StereoString PackUtils::XMP_HEADER_START = "http://ns.adobe.com/xap/1.0/";
const char PackUtils::STR_TERMINATOR = '\0';
const char PackUtils::STR_TERMINATOR_LENGTH = 1;

const int PackUtils::CUSTOMIZED_TOTAL_LENGTH = 4;
const int PackUtils::CUSTOMIZED_SERIAL_NUMBER_LENGTH = 1;
const int PackUtils::CUSTOMIZED_TOTAL_FORMAT_LENGTH =
            PackUtils::CUSTOMIZED_TOTAL_LENGTH
            + PackUtils::TYPE_BUFFER_COUNT
            + PackUtils::CUSTOMIZED_SERIAL_NUMBER_LENGTH;

const StereoString PackUtils::kHexDigits = "0123456789ABCDEF";
const int PackUtils::BYTE_COUNT_4 = 4;

StereoString PackUtils::getMD5(const StereoBuffer_t &in) {
    MD5_CTX context;
    XMP_Uns8 digest [16] = {0};
    MD5Init(&context);
    MD5Update(&context, (XMP_Uns8*)(in.data), (XMP_Uns32)in.size);
    MD5Final(digest, &context );

    StereoString digestStr;
    digestStr.reserve (32);
    for (size_t i = 0; i < 16; ++i) {
        XMP_Uns8 byte = digest[i];
        digestStr.push_back(kHexDigits[byte >> 4]);
        digestStr.push_back(PackUtils::kHexDigits[byte & 0xF]);
    }
    return digestStr;
}

void PackUtils::intToByteBuffer(
        int in, StereoBuffer_t & outBuffer) {
    int byteCount = outBuffer.size;
    for (int i = 0; i < byteCount; i++) {
        (outBuffer.data)[byteCount - 1 - i] = (in >> (i * 8));
    }
}

unsigned int PackUtils::byteBufferToInt(
    const StereoBuffer_t &inBuffer, int start, int size) {
    unsigned int ret = 0;
    for (int i = 0; i < size; i++) {
        ret += ((inBuffer.data)[start + i] & 0xFF) << (8 * (size - 1 - i));
    }
    return ret;
}

StereoString PackUtils::getCustomTypeName(StereoBuffer_t &buffer) {
    StereoString type = Utils::buffer2Str(buffer, CUSTOMIZED_TOTAL_LENGTH, TYPE_BUFFER_COUNT);
    StereoLogD("<getCustomTypeName> type is %s", type.c_str());
    return type;
}

