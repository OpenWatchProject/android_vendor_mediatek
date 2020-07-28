#define LOG_NDEBUG 0
#define LOG_TAG "Cta5MatroskaFile"
#include <utils/Log.h>

#include <Cta5MatroskaFile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include "mkvparser.h"

using namespace android;

struct DataSourceReader : public mkvparser::IMkvReader {
     DataSourceReader(int fd)
         : mFd(fd) {
     }

     virtual int Read(long long position, long length, unsigned char* buffer) {
         if(position < 0 || length < 0) {
            return -1;
         }

         if (length == 0) {
             return 0;
         }

         ssize_t n = DrmCtaMultiMediaUtil::readAt(mFd, position, buffer, length);

         if (n <= 0) {
             ALOGE("[ERROR][CTA5]readAt %ld bytes, Read return -1", (long) n);
             ALOGE("[ERROR][CTA5]position= %lld, length= %ld", position, length);
             return -1;
         }

         return 0;
     }

     virtual int Length(long long* total, long long* available) {
         off64_t size;

         size = lseek64(mFd, 0, SEEK_END);
         if(size == -1) {
             *total = -1;
             *available = (long long)((1ull << 63) - 1);

             return 0;
         }

         if (total) {
             *total = size;
         }

         if (available) {
             *available = size;
         }

         return 0;
     }

 private:
     int mFd;

     DataSourceReader(const DataSourceReader &);
     DataSourceReader &operator=(const DataSourceReader &);
 };



Cta5MatroskaFile::Cta5MatroskaFile(int fd, String8 key) : Cta5CommonMultimediaFile(fd, key)
{
    ALOGD("Cta5MatroskaFile(fd, key)");
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5MatroskaFile::Cta5MatroskaFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)
{
    ALOGD("Cta5MatroskaFile(mimeType, cid, dcfFlHeaders, datatLen, key)");
}

Cta5MatroskaFile::Cta5MatroskaFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, datatLen, key)
{
    ALOGD("Cta5MatroskaFile(mimeType, datatLen, key)");
}

bool Cta5MatroskaFile::parseHeaders(int fd) {
    ALOGD("parseHeaders()");
    DataSourceReader reader(fd);
    mkvparser::EBMLHeader ebmlHeader;
    long long pos;
    if (ebmlHeader.Parse(&reader, pos) < 0) {
        ALOGE("[ERROR][CTA5]parse ebml header fail");
        return false;
    }
    ALOGD("found ebml header: offset: %lld, size: %lld", 0LL, pos);

    // stored header information into Vector<header>
    Header * ebml_header = new Header();
    ebml_header->clear_header_offset = 0LL;
    ebml_header->clear_header_size = pos;

    mHeaders.push_back(ebml_header);
    return true;
}


