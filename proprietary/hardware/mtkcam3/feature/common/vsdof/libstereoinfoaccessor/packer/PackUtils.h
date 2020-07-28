#ifndef PACK_UTILS_H
#define PACK_UTILS_H

#include "types.h"
#include "BufferManager.h"

namespace stereo {

class PackUtils {

public:
    static const int SOI;
    static const int SOS;
    static const int APP1;
    static const int APP15;
    static const int DQT;
    static const int DHT;
    static const int APPXTAG_PLUS_LENGTHTAG_BYTE_COUNT;
    static const int APP15_LENGTHTAG_BYTE_COUNT;
    static const int TYPE_BUFFER_COUNT;
    static const int WRITE_XMP_AFTER_SOI;
    static const int WRITE_XMP_BEFORE_FIRST_APP1;
    static const int WRITE_XMP_AFTER_FIRST_APP1;
    static const int FIXED_BUFFER_SIZE;

    static const StereoString XMP_EXT_HEADER;
    static const int MAX_BYTE_PER_APP1;
    static const int MD5_BYTE_COUNT;
    static const int TOTAL_LENGTH_BYTE_COUNT;
    static const int PARTITION_OFFSET_BYTE_COUNT;
    static const int XMP_COMMON_HEADER_LEN;
    static const int MAX_LEN_FOR_REAL_XMP_DATA_PER_APP1;

    static const StereoString APP_SECTION_MAX_LENGTH;
    static const StereoString TYPE_JPS_DATA;
    static const StereoString TYPE_JPS_MASK;
    static const StereoString TYPE_DEPTH_DATA;
    static const StereoString TYPE_DEBUG_BUFFER;
    static const StereoString TYPE_XMP_DEPTH;
    static const StereoString TYPE_SEGMENT_MASK;
    static const StereoString TYPE_CLEAR_IMAGE;
    static const StereoString TYPE_LDC_DATA;

    static const StereoString TYPE_STANDARD_XMP;
    static const StereoString TYPE_EXTENDED_XMP;
    static const StereoString TYPE_UNKNOW_APP15;
    static const StereoString TYPE_EXIF;
    static const StereoString EXIF_HEADER;
    static const StereoString XMP_HEADER_START;
    static const char STR_TERMINATOR;
    static const char STR_TERMINATOR_LENGTH;

    static const int CUSTOMIZED_TOTAL_LENGTH;
    static const int CUSTOMIZED_SERIAL_NUMBER_LENGTH;
    static const int CUSTOMIZED_TOTAL_FORMAT_LENGTH;

    static const StereoString kHexDigits;
    static const int BYTE_COUNT_4;

    static StereoString getMD5(const StereoBuffer_t &in);
    /**
     * Convert integer to byte buffer.
     * @param in
     *  Integer
     * @param outBuffer
     *  [out]vector<char>
     */
    static void intToByteBuffer(
        int in, StereoBuffer_t & outBuffer);

    /**
     * Convert byte buffer to integer
     */
    static unsigned int byteBufferToInt(
        const StereoBuffer_t &inBuffer, int start, int size);

    /**
     * Get customized data buffer type name.
     * @param buffer
     *            data buffer
     * @return type name
     */
    static StereoString getCustomTypeName(StereoBuffer_t &buffer);
private:

};

}


#endif