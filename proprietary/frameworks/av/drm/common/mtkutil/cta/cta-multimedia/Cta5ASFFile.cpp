#define LOG_NDEBUG 0
#define LOG_TAG "Cta5ASFFile"
#include <utils/Log.h>

#include <Cta5ASFFile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

using namespace android;

static const uint8_t ASF_HEADER_OBJECT_ID[16] = {
    0x30, 0x26, 0xB2, 0x75, 0x8E, 0x66, 0xCF, 0x11,
    0xA6, 0xD9, 0x00, 0xAA, 0x00, 0x62, 0xCE, 0x6C
};

Cta5ASFFile::Cta5ASFFile(int fd, String8 key) : Cta5CommonMultimediaFile(fd, key)
{
    ALOGD("Cta5ASFFile(fd, key)");
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5ASFFile::Cta5ASFFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)
{
    ALOGD("Cta5ASFFile(mimeType, cid, dcfFlHeaders, datatLen, key)");
}

Cta5ASFFile::Cta5ASFFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, datatLen, key)
{
    ALOGD("Cta5ASFFile(mimeType, datatLen, key)");
}

bool Cta5ASFFile::parseHeaders(int fd) {
    ALOGD("parseHeaders()");
    off64_t header_offset = 0LL;
    off64_t header_size = 0LL;
    uint8_t guid[16] = {0};
    if(DrmCtaMultiMediaUtil::readAt(fd, 0, guid, sizeof(guid)) != sizeof(guid)) {
        ALOGD("failed to read header at 0x%08x", 0);
        return false;
    }
    header_offset = 0;
    if(0!= memcmp(guid, ASF_HEADER_OBJECT_ID, 16)) {
        ALOGE("[ERROR][CTA5]parseASFHeader - not asf/wmv file format.");
        return false;
    }

    if(DrmCtaMultiMediaUtil::readAt(fd, 16, (uint8_t*)&header_size, sizeof(header_size)) != sizeof(header_size)) {
        ALOGE("[ERROR][CTA5]failed to read header at 0x%08x", 16);
        return false;
    }
    ALOGD("parseASFHeader - header_offset[%ld], header_size[%ld]", (long) header_offset, (long) header_size);
    // stored header information into Vector<header>
    Header * asf_header = new Header();
    asf_header->clear_header_offset = header_offset;
    asf_header->clear_header_size = header_size;

    mHeaders.push_back(asf_header);
    return true;
}


