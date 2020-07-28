#define LOG_NDEBUG 0
#define LOG_TAG "Cta5AVIFile"
#include <utils/Log.h>

#include <Cta5AVIFile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

using namespace android;

static const int32_t kAVIMaxRIFFSize = 0x40000000LL; // 1G bytes for one riff
static const int32_t kSizeOfListHeader = 12;
static const int32_t kSizeOfSkipHeader = 8;
static const int32_t kSizeOfChunkHeader = 8;

Cta5AVIFile::Cta5AVIFile(int fd, String8 key) : Cta5CommonMultimediaFile(fd, key)
{
    ALOGD("Cta5AVIFile(fd, key)");
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5AVIFile::Cta5AVIFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)
{
    ALOGD("Cta5AVIFile(mimeType, cid, dcfFlHeaders, datatLen, key)");
}

Cta5AVIFile::Cta5AVIFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, datatLen, key)
{
    ALOGD("Cta5AVIFile(mimeType, datatLen, key)");
}

bool Cta5AVIFile::parseHeaders(int fd) {
    ALOGD("parseHeaders()");
    struct riffList list;
    off64_t hdrl_offset = 0LL;
    off64_t hdrl_size = 0LL;

    if(DrmCtaMultiMediaUtil::readAt(fd, 0, (char *)&list, sizeof(list)) != sizeof(list)) {
        ALOGE("[ERROR][CTA5]failed to read header at 0x%08x", 0);
        return false;
    }

    if(list.ID != BFOURCC('R', 'I', 'F', 'F')) {
        ALOGE("[ERROR][CTA5]not riff format");
        return false;
    }

    int32_t size = list.size;
    if(size > kAVIMaxRIFFSize) {
        ALOGE("[ERROR][CTA5]size of RIFF is out of spec %d > %d", size, kAVIMaxRIFFSize);
    }

    off64_t fSize = lseek64(fd, 0, SEEK_END);
    if (fSize <= 0) {
        ALOGE("[ERROR][CTA5]abnormal file size %ld. out of 32 bit? not supported yet", (long) fSize);
        return false;
    }

    if (size + kSizeOfSkipHeader > fSize) {
        // TODO do our best to support chunked file
        ALOGW("file seems to be chunked (%d + %d) > %ld", size, kSizeOfSkipHeader, (long) fSize);
    }

    off_t pos, oldEnd;
    pos = kSizeOfListHeader;
    oldEnd = kSizeOfSkipHeader + size;
    ALOGD("init: offset: %ld, oldEnd: %ld, fSize: %ld", pos, oldEnd, (long) fSize);
    oldEnd = oldEnd > fSize ? fSize : size;

    while(pos < oldEnd) {
        if (pos > fSize) {
            ALOGW("chunked file? %lx > %ld", pos, (long) fSize);
            break;
        }

        int32_t s = DrmCtaMultiMediaUtil::readAt(fd, pos, (char*)&list, sizeof(list));
        if (s != kSizeOfListHeader) {
            if (s != kSizeOfChunkHeader || list.size != 0) {
                ALOGE("[ERROR][CTA5]failed to read header at 0x%08lx", pos);
                return false;
            }
            break;
        }

        ALOGV("ID " FORMATFOURCC " at pos 0x%08lx", PRINTFOURCC(list.ID), pos);

        off_t end = pos + EVEN(list.size) + kSizeOfSkipHeader;

        if (list.ID == BFOURCC('L','I','S','T')) {
            ALOGV("type " FORMATFOURCC, PRINTFOURCC(list.type));
            //pos = pos + kSizeOfListHeader;
            switch (list.type) {
                case BFOURCC('h','d','r','l'):
                    //ALOGD("found hdrl %lx > %lx", pos, end);
                    hdrl_offset = pos;
                    hdrl_size = list.size + kSizeOfSkipHeader;
                    ALOGD("found hdrl: offset: %ld, size: %ld", pos, (long) hdrl_size);

                    // stored header information into Vector<header>
                    Header * hdrl_header = new Header();
                    hdrl_header->clear_header_offset = hdrl_offset;
                    hdrl_header->clear_header_size = hdrl_size;

                    mHeaders.push_back(hdrl_header);

                    return true;
            }
        }
        pos = end;
    }

    return false;
}


