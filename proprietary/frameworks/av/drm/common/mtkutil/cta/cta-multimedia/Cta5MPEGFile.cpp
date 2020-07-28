#define LOG_NDEBUG 0
#define LOG_TAG "Cta5MPEGFile"
#include <utils/Log.h>

#include <Cta5MPEGFile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace android;

Cta5MPEGFile::Cta5MPEGFile(int fd, String8 key) : Cta5CommonMultimediaFile(fd, key)
{
    ALOGD("Cta5MPEGFile(fd, key)");
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5MPEGFile::Cta5MPEGFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)
{
    ALOGD("Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)");
}

Cta5MPEGFile::Cta5MPEGFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, datatLen, key)
{
    ALOGD("Cta5MPEGFile(mimeType, datatLen, key)");
}

bool Cta5MPEGFile::parseHeaders(int fd) {
    ALOGD("parseHeaders()");
    off64_t moov_offset = 0LL;
    off64_t moov_size = 0LL;
    off64_t kMaxScanOffset = 128LL;
    off64_t offset = 0LL;
    bool foundGoodFileType = false;
    off64_t moovAtomStartOffset = -1LL;
    off64_t moovAtomEndOffset = -1LL;
    bool done = false;

    uint8_t header[12];
    // If type is not ftyp,mdata,moov or free, return false directly. Or else, it may be mpeg4 file.
    if (DrmCtaMultiMediaUtil::readAt(fd, 0, header, 12) != 12
            || (memcmp("ftyp", &header[4], 4) && memcmp("mdat", &header[4], 4)
                && memcmp("moov", &header[4], 4) && memcmp("free", &header[4], 4)
                            && memcmp("wide", &header[4], 4))) {
        ALOGE("[ERROR][CTA5]return false, type=0x%8.8x", *((uint32_t *)&header[4]));
        return false;
    }

    while (!done && offset < kMaxScanOffset) {
        uint32_t hdr[2];
        if (DrmCtaMultiMediaUtil::readAt(fd, offset, hdr, 8) < 8) {
            return false;
        }

        uint64_t chunkSize = ntohl(hdr[0]);
        uint32_t chunkType = ntohl(hdr[1]);
        off64_t chunkDataOffset = offset + 8;

        if (chunkSize == 1) {
            if (DrmCtaMultiMediaUtil::readAt(fd, offset + 8, &chunkSize, 8) < 8) {
                return false;
            }

            chunkSize = DrmCtaMultiMediaUtil::ntoh64(chunkSize);
            chunkDataOffset += 8;

            if (chunkSize < 16) {
                // The smallest valid chunk is 16 bytes long in this case.
                return false;
            }
        } else if (chunkSize < 8) {
            // The smallest valid chunk is 8 bytes long.
            return false;
        }

        off64_t chunkDataSize = offset + chunkSize - chunkDataOffset;

        char chunkstring[5];
        DrmCtaMultiMediaUtil::MakeFourCCString(chunkType, chunkstring);
        ALOGD("saw chunk type %s, size %ld @ %ld", chunkstring, (long) chunkSize, (long) offset);
        switch (chunkType) {
            case FOURCC('f', 't', 'y', 'p'):
            {
                if (chunkDataSize < 8) {
                    return false;
                }

                uint32_t numCompatibleBrands = (chunkDataSize - 8) / 4;
                for (size_t i = 0; i < numCompatibleBrands + 2; ++i) {
                    if (i == 1) {
                        // Skip this index, it refers to the minorVersion,
                        // not a brand.
                        continue;
                    }

                    uint32_t brand;
                    if (DrmCtaMultiMediaUtil::readAt(fd, chunkDataOffset + 4 * i, &brand, 4) < 4) {
                        return false;
                    }

                    brand = ntohl(brand);

                    if (DrmCtaMultiMediaUtil::isCompatibleBrand(brand)) {
                        foundGoodFileType = true;
                        break;
                    }
                }

                if (!foundGoodFileType) {
                    ALOGW("[ERROR][CTA5]Warning:ftyp brands is not isCompatibleBrand 1");
                    return false;
                }

                break;
            }

            case FOURCC('m', 'o', 'o', 'v'):
            {
                moovAtomStartOffset = offset;
                moovAtomEndOffset = offset + chunkSize;

                moov_offset = offset;
                moov_size = chunkSize;
                done = true;

                ALOGD("found moov: moov_offset: %ld, moov_size: %ld", (long) moov_offset, (long) moov_size);
                break;
            }

            case FOURCC('f', 'r', 'e', 'e'):
            case FOURCC('m', 'd', 'a', 't'):
            case FOURCC('w', 'i', 'd', 'e'):
            case FOURCC('t', 'r', 'a', 'f'):
            case FOURCC('m', 'o', 'o', 'f'):
            case FOURCC('t', 'f', 'h', 'd'):
            case FOURCC('t', 'r', 'u', 'n'):
            case FOURCC('s', 'a', 'i', 'z'):
            case FOURCC('s', 'a', 'i', 'o'):
            case FOURCC('u', 'u', 'i', 'd'):
            {
                char chunk[5];
                DrmCtaMultiMediaUtil::MakeFourCCString(chunkType, chunk);
                ALOGD("chunk: %s @ %ld, chunkSize:%ld", chunk, (long) offset, (long) chunkSize);
                kMaxScanOffset += chunkSize;
                break;
            }
            default:
                break;
        }

        offset += chunkSize;
    }

    // stored header information into Vector<header>
    Header * moov_header = new Header();
    moov_header->clear_header_offset = moov_offset;
    moov_header->clear_header_size = moov_size;

    mHeaders.push_back(moov_header);

    return true;
}


