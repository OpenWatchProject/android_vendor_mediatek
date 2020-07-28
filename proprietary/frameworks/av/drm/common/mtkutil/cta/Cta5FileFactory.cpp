#define LOG_TAG "Cta5FileFactory"
#include <utils/Log.h>
#include <Cta5FileFactory.h>
#include <Cta5CommonMultimediaFile.h>
#include <Cta5NormalFile.h>
#include <DrmCtaMultiMediaUtil.h>
#include <Cta5CommonMultimediaFile.h>
#include <Cta5MPEGFile.h>
#include <Cta5AVIFile.h>
#include <Cta5ASFFile.h>
#include <Cta5MatroskaFile.h>
#include <Cta5WAVFile.h>
#include <Cta5FLACFile.h>
#include <Cta5FLVFile.h>




using namespace android;

bool Cta5FileFactory::isMultiMedia(int fd, const String8& key __attribute__((unused)))
{
    // first check whether is dcf file
    // the dcf file should return false, and use NormalFile to encrypt/decrypt
    if(Cta5FileUtil::isDcfFile(fd)) {
        return false;
    }

    //Read the header
    if(Cta5File::isCta5MultimediaFile(fd))
    {
        return true;
    }
    ALOGD("it is a normal file");
    return false;
}

bool Cta5FileFactory::isMultiMedia(String8 mime)
{
    return DrmCtaMultiMediaUtil::isExistedInMultiMediaList(mime);
}

Cta5File *Cta5FileFactory::createCta5File(int fd, String8 key)
{

    if (Cta5File::isCta5MultimediaFile(fd) == true)
    {
        // get mime type
        String8 mime = Cta5CommonMultimediaFile::getOriginalMimetype(fd, key);
        // check mime type
        if(0 == strcmp("video/mp4", mime.string())
            || 0 == strcmp("video/3gpp", mime.string())
            || 0 == strcmp("video/3gp", mime.string())
            || 0 == strcmp("video/quicktime", mime.string())) {
            return new Cta5MPEGFile(fd, key);
        } else if(0 == strcmp("video/avi", mime.string())) {
            return new Cta5AVIFile(fd, key);
        } else if(0 == strcmp("video/x-ms-wmv", mime.string())
            || 0 == strcmp("video/asf", mime.string())
            || 0 == strcmp("audio/x-ms-wma", mime.string())) {
            return new Cta5ASFFile(fd, key);
        } else if(0 == strcmp("video/x-matroska", mime.string())) {
            return new Cta5MatroskaFile(fd, key);
        } else if(0 == strcmp("video/x-flv", mime.string())) {
            return new Cta5FLVFile(fd, key);
        } else if(0 == strcmp("audio/x-wav", mime.string())) {
            return new Cta5WAVFile(fd, key);
        } else if(0 == strcmp("audio/flac", mime.string())) {
            return new Cta5FLACFile(fd, key);
        }
        return new Cta5NormalFile(fd, key);
    } else if (Cta5File::isCta5NormalFile(fd))
    {
        return new Cta5NormalFile(fd, key);
    }
    ALOGE("not a cta5 file.");
    return NULL;
}

Cta5File *Cta5FileFactory::createCta5File(String8 mimeType,
                uint64_t datatLen, String8 key)
{
    if (Cta5FileFactory::isMultiMedia(mimeType))
    {
        // check mime type
        if(0 == strcmp("video/mp4", mimeType.string())
            || 0 == strcmp("video/3gp", mimeType.string())
            || 0 == strcmp("video/3gpp", mimeType.string())
            || 0 == strcmp("video/quicktime", mimeType.string())) {
            return new Cta5MPEGFile(mimeType, datatLen, key);
        } else if(0 == strcmp("video/avi", mimeType.string())) {
            return new Cta5AVIFile(mimeType, datatLen, key);
        } else if(0 == strcmp("video/x-ms-wmv", mimeType.string())
            || 0 == strcmp("video/asf", mimeType.string())
            || 0 == strcmp("audio/x-ms-wma", mimeType.string())) {
            return new Cta5ASFFile(mimeType, datatLen, key);
        } else if(0 == strcmp("video/x-matroska", mimeType.string())) {
            return new Cta5MatroskaFile(mimeType, datatLen, key);
        } else if(0 == strcmp("video/x-flv", mimeType.string())) {
            return new Cta5FLVFile(mimeType, datatLen, key);
        } else if(0 == strcmp("audio/x-wav", mimeType.string())) {
            return new Cta5WAVFile(mimeType, datatLen, key);
        } else if(0 == strcmp("audio/flac", mimeType.string())) {
            return new Cta5FLACFile(mimeType, datatLen, key);
        }
        return new Cta5NormalFile(mimeType, datatLen, key);
    } else
    {
        return new Cta5NormalFile(mimeType, datatLen, key);
    }
}

Cta5File *Cta5FileFactory::createCta5File(String8 mimeType,
                uint64_t datatLen, String8 key, bool isDcfFile)
{
    if(isDcfFile) {
        return new Cta5NormalFile(mimeType, datatLen, key);
    }
    return createCta5File(mimeType, datatLen, key);
}
