#define LOG_NDEBUG 0
#define LOG_TAG "Cta5FLVFile"
#include <utils/Log.h>

#include <Cta5FLVFile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

#include "Cta5FLVFile.h"

using namespace android;

#define MAX_PARSE_FRAME 6

int flv_get_keyregion(int fd, int* start_pos, int* size)
{
    int new_fd = dup(fd);
    FILE* fp;
    int tmp_offset = 0;
    int tag_size = 0;
    int i = 0;
    unsigned char buf[10];
    fp = fdopen(new_fd, "rb");
    if (fp == NULL) {
        ALOGE("[ERROR][CTA5]open file failed!\n");
        return -1;
    }
    tmp_offset += 13;
    for (i = 0; i < MAX_PARSE_FRAME; ++i) {
        int ret = 0;
        fseek(fp, tmp_offset, 0);
        ret = fread(buf, sizeof(unsigned char), 10, fp);
        tag_size = (buf[1] << 16 | buf[2] << 8) | buf[3];
        ALOGE("[ERROR][CTA5]buf1(%x), buf2(%x), buf3(%d), tag_size(%d)\n", buf[1], buf[2], buf[3], tag_size);
        tmp_offset += (tag_size + 15);
    }
    fclose(fp);

    *start_pos = 0;
    *size = tmp_offset;

    return 0;
}


Cta5FLVFile::Cta5FLVFile(int fd, String8 key) : Cta5CommonMultimediaFile(fd, key)
{
    ALOGD("Cta5FLVFile(fd, key)");
}


//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5FLVFile::Cta5FLVFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)
{
    ALOGD("Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)");
}

Cta5FLVFile::Cta5FLVFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, datatLen, key)
{
    ALOGD("Cta5FLVFile(mimeType, datatLen, key)");
}

bool Cta5FLVFile::parseHeaders(int fd) {
    ALOGD("parseHeaders()");

    int ret = 0;
    int start_pos = 0;
    int size = 0;
    ret = flv_get_keyregion(fd, &start_pos, &size);
    ALOGD("found flv header, offset[%d], size[%d]\n", start_pos, size);
    if(ret != 0) {
        return false;
    }

    // stored header information into Vector<header>
    Header * flv_header = new Header();
    flv_header->clear_header_offset = start_pos;
    flv_header->clear_header_size = size;

    mHeaders.push_back(flv_header);

    return true;
}


