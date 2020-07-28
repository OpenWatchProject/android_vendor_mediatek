#define LOG_NDEBUG 0
#define LOG_TAG "Cta5APEFile"
#include <utils/Log.h>

#include <Cta5APEFile.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>

using namespace android;

static bool getAPEInfo(
    int fd, off_t *inout_pos, ape_parser_ctx_t *ape_ctx, bool parseall)
{
    unsigned int i;
    unsigned int file_offset = 0;
    bool ret = false;
    off_t ori_pos = *inout_pos;
    ///LOGD("getAPEInfo %d, %d", *inout_pos, parseall);
    memset(ape_ctx, 0, sizeof(ape_parser_ctx_t));
    char *pFile = new char[20480 + 1024];

    if (pFile == NULL)
    {
        ALOGE("[ERROR][CTA5][ERROR][CTA5]getAPEInfo memory error");
        goto GetApeInfo_Exit;
    }

    if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, pFile, 20480 + 1024) <= 0)
    {
        goto GetApeInfo_Exit;
    }

    while (1)
    {
        char *sync;
        ///if (4 != fread(sync, 1, 4, fp))
        ///if((DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, sync, 4)!= 4)
        sync = pFile + (*inout_pos - ori_pos) ;

        if (*inout_pos - ori_pos > 20480)
        {
            ALOGE("[ERROR][CTA5]getAPEInfo not ape %lx", *inout_pos);
            goto GetApeInfo_Exit;
        }

        if (memcmp(sync, "MAC ", 4) == 0)
        {
            ALOGV("getAPEInfo parse ok, %ld!!!!", *inout_pos);
            ///return false;
            break;
        }
        else if (memcmp(sync + 1, "MAC", 3) == 0)
        {
            *inout_pos += 1;
        }
        else if (memcmp(sync + 2, "MA", 2) == 0)
        {
            *inout_pos += 2;
        }
        else if (memcmp(sync + 3, "M", 1) == 0)
        {
            *inout_pos += 3;
        }
        else if ((memcmp("ID3", sync, 3) == 0))
        {
            size_t len =
                ((sync[6] & 0x7f) << 21)
                | ((sync[7] & 0x7f) << 14)
                | ((sync[8] & 0x7f) << 7)
                | (sync[9] & 0x7f);

            len += 10;

            ALOGV("getAPEInfo id3 tag %ld, len %d", *inout_pos, (int) len);
            *inout_pos += len;

            ori_pos = *inout_pos;

            if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, pFile, 20480 + 1024) <= 0)
            {
                goto GetApeInfo_Exit;
            }
        }
        else
        {
            *inout_pos += 4;
        }

    }

    file_offset = *inout_pos;
    memcpy(ape_ctx->magic, "MAC ", 4);
    ape_ctx->junklength = *inout_pos;
    *inout_pos += 4;

    // unsigned char sync4[4];
    // unsigned char sync2[2];

    if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->fileversion, sizeof(ape_ctx->fileversion)) < 0)
    {
        goto GetApeInfo_Exit;
    }

    if((ape_ctx->fileversion > 4200)
        || (ape_ctx->fileversion < 3940) )
    {
        ALOGE("[ERROR][CTA5]getAPEInfo version is not match %d", ape_ctx->fileversion);
        goto GetApeInfo_Exit;
    }

    if (parseall == false)
    {
        ret = true;
        goto GetApeInfo_Exit;
    }

    *inout_pos += 2;

    if (ape_ctx->fileversion >= 3980)
    {
        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->padding1, sizeof(ape_ctx->padding1)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->descriptorlength, sizeof(ape_ctx->descriptorlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->headerlength, sizeof(ape_ctx->headerlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->seektablelength, sizeof(ape_ctx->seektablelength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->wavheaderlength, sizeof(ape_ctx->wavheaderlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->audiodatalength, sizeof(ape_ctx->audiodatalength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->audiodatalength_high, sizeof(ape_ctx->audiodatalength_high)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->wavtaillength, sizeof(ape_ctx->wavtaillength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->md5, 16) != 16)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 16;

        /* Skip any unknown bytes at the end of the descriptor.  This is for future  compatibility */
        if (ape_ctx->descriptorlength > 52)
        {
            *inout_pos += (ape_ctx->descriptorlength - 52);
        }

        /* Read header data */
        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->compressiontype, sizeof(ape_ctx->compressiontype)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->compressiontype > APE_MAX_COMPRESS)
        {
            ALOGE("[ERROR][CTA5]getAPEInfo(Line%d): unsupported compressiontype = %u", __LINE__, ape_ctx->compressiontype);
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->formatflags, sizeof(ape_ctx->formatflags)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->blocksperframe, sizeof(ape_ctx->blocksperframe)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->finalframeblocks, sizeof(ape_ctx->finalframeblocks)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->totalframes, sizeof(ape_ctx->totalframes)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->bps, sizeof(ape_ctx->bps)) < 0)
        {
            goto GetApeInfo_Exit;
        }

#if !defined(ANDROID_DEFAULT_CODE) && defined(MTK_24BIT_AUDIO_SUPPORT)
        ALOGD("support 24bit, bps:%d",ape_ctx->bps);
#else
        if (ape_ctx->bps > 16)
        {
            goto GetApeInfo_Exit;
        }
#endif

        if (parseall == false)
        {
            ret = true;
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->channels, sizeof(ape_ctx->channels)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->samplerate, sizeof(ape_ctx->samplerate)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->blocksperframe <= 0
                || ape_ctx->totalframes <= 0
                || ape_ctx->bps <= 0
                || ape_ctx->seektablelength <= 0
                || ape_ctx->samplerate <= 0
                || ape_ctx->samplerate > 192000)
        {
            ALOGD("getAPEInfo header error: blocksperframe %x,totalframes %x, bps %x,seektablelength %x, samplerate %x ",
                 ape_ctx->blocksperframe,
                 ape_ctx->totalframes,
                 ape_ctx->bps,
                 ape_ctx->seektablelength,
                 ape_ctx->samplerate);
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;
    }
    else
    {
        ape_ctx->descriptorlength = 0;
        ape_ctx->headerlength = 32;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->compressiontype, sizeof(ape_ctx->compressiontype)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->compressiontype > APE_MAX_COMPRESS)
        {
            ALOGE("[ERROR][CTA5]getAPEInfo(Line%d): unsupported compressiontype = %u", __LINE__, ape_ctx->compressiontype);
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->formatflags, sizeof(ape_ctx->formatflags)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->channels, sizeof(ape_ctx->channels)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 2;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->samplerate, sizeof(ape_ctx->samplerate)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->wavheaderlength, sizeof(ape_ctx->wavheaderlength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->wavtaillength, sizeof(ape_ctx->wavtaillength)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->totalframes, sizeof(ape_ctx->totalframes)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->finalframeblocks, sizeof(ape_ctx->finalframeblocks)) < 0)
        {
            goto GetApeInfo_Exit;
        }

        *inout_pos += 4;

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_HAS_PEAK_LEVEL)
        {
            ///fseek(fp, 4, SEEK_CUR);
            *inout_pos += 4;
            ape_ctx->headerlength += 4;
        }

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_HAS_SEEK_ELEMENTS)
        {
            if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &ape_ctx->seektablelength, sizeof(ape_ctx->seektablelength)) < 0)
            {
                goto GetApeInfo_Exit;
            }

            *inout_pos += 4;
            ape_ctx->headerlength += 4;
            ape_ctx->seektablelength *= sizeof(ape_parser_int32_t);
        }
        else
        {
            ape_ctx->seektablelength = ape_ctx->totalframes * sizeof(ape_parser_int32_t);
        }

        if (ape_ctx->formatflags & MAC_FORMAT_FLAG_8_BIT)
        {
            ape_ctx->bps = 8;
        }
        else if (ape_ctx->formatflags & MAC_FORMAT_FLAG_24_BIT)
        {
            ape_ctx->bps = 24;
            goto GetApeInfo_Exit;
        }
        else
        {
            ape_ctx->bps = 16;
        }

        if (parseall == false)
        {
            ret = true;
            goto GetApeInfo_Exit;
        }

        if (ape_ctx->fileversion >= APE_MIN_VERSION)
        {
            ape_ctx->blocksperframe = 73728 * 4;
        }
        else if ((ape_ctx->fileversion >= 3900) || (ape_ctx->fileversion >= 3800 && ape_ctx->compressiontype >= APE_MAX_COMPRESS))
        {
            ape_ctx->blocksperframe = 73728;
        }
        else
        {
            ape_ctx->blocksperframe = 9216;
        }

        /* Skip any stored wav header */
        if (!(ape_ctx->formatflags & MAC_FORMAT_FLAG_CREATE_WAV_HEADER))
        {
            *inout_pos += ape_ctx->wavheaderlength;
        }

        if (ape_ctx->blocksperframe <= 0
                || ape_ctx->totalframes <= 0
                || ape_ctx->bps <= 0
                || ape_ctx->seektablelength <= 0
                || ape_ctx->samplerate <= 0
                || ape_ctx->samplerate > 192000)
        {
            ALOGD("getAPEInfo header error: blocksperframe %x,totalframes %x, bps %x,seektablelength %x, samplerate %x ",
                 ape_ctx->blocksperframe,
                 ape_ctx->totalframes,
                 ape_ctx->bps,
                 ape_ctx->seektablelength,
                 ape_ctx->samplerate);
            goto GetApeInfo_Exit;
        }
    }
    ape_ctx->totalsamples = ape_ctx->finalframeblocks;

    if (ape_ctx->totalframes > 1)
    {
        ape_ctx->totalsamples += ape_ctx->blocksperframe * (ape_ctx->totalframes - 1);
    }

    if (ape_ctx->seektablelength > 0)
    {
        ape_parser_uint32_t seekaddr = 0;
        ape_ctx->seektable = (uint32_t *)malloc(ape_ctx->seektablelength);

        if (ape_ctx->seektable == NULL)
        {
            goto GetApeInfo_Exit;
        }

        for (i = 0; i < ape_ctx->seektablelength / sizeof(ape_parser_uint32_t); i++)
        {
            if (DrmCtaMultiMediaUtil::readAt(fd, *inout_pos, &seekaddr, 4) < 0)
            {
                free(ape_ctx->seektable);
                ape_ctx->seektable = NULL;
                goto GetApeInfo_Exit;
            }

            ape_ctx->seektable[i] = (seekaddr + file_offset);
            *inout_pos += 4;
        }
    }

    ape_ctx->firstframe = ape_ctx->junklength + ape_ctx->descriptorlength +
                          ape_ctx->headerlength + ape_ctx->seektablelength +
                          ape_ctx->wavheaderlength;
    ape_ctx->seektablefilepos = ape_ctx->junklength + ape_ctx->descriptorlength +
                                ape_ctx->headerlength;


    *inout_pos = ape_ctx->firstframe;

    ALOGV("getAPEInfo header info: offset %d, ape_ctx->junklength %x,ape_ctx->firstframe %x, ape_ctx->totalsamples %x,ape_ctx->fileversion %x, ape_ctx->padding1 %x ",
         file_offset,
         ape_ctx->junklength,
         ape_ctx->firstframe,
         ape_ctx->totalsamples,
         ape_ctx->fileversion,
         ape_ctx->padding1);

    ALOGV("ape_ctx->descriptorlength %x,ape_ctx->headerlength %x,ape_ctx->seektablelength %x,ape_ctx->wavheaderlength %x,ape_ctx->audiodatalength %x ",
         ape_ctx->descriptorlength,
         ape_ctx->headerlength,
         ape_ctx->seektablelength,
         ape_ctx->wavheaderlength,
         ape_ctx->audiodatalength);


    ALOGV("ape_ctx->audiodatalength_high %x,ape_ctx->wavtaillength %x,ape_ctx->compressiontype %x, ape_ctx->formatflags %x,ape_ctx->blocksperframe %x",
         ape_ctx->audiodatalength_high,
         ape_ctx->wavtaillength,
         ape_ctx->compressiontype,
         ape_ctx->formatflags,
         ape_ctx->blocksperframe);

    ALOGV("ape_ctx->finalframeblocks %x,ape_ctx->totalframes %x,ape_ctx->bps %x,ape_ctx->channels %x,ape_ctx->samplerate %x",
         ape_ctx->finalframeblocks,
         ape_ctx->totalframes,
         ape_ctx->bps,
         ape_ctx->channels,
         ape_ctx->samplerate);

    ret = true;

GetApeInfo_Exit:

    if (pFile)
    {
        delete[] pFile;
    }

    pFile = NULL;
    return ret;

}


Cta5APEFile::Cta5APEFile(int fd, String8 key) : Cta5CommonMultimediaFile(fd, key)
{
    ALOGD("Cta5APEFile(fd, key)");
}

//This constructor is useful when you want to get a Cta5 file format
//To convert a normal file to a CTA5 file, you may need this version
Cta5APEFile::Cta5APEFile(String8 mimeType, String8 cid, String8 dcfFlHeaders,
        uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, cid, dcfFlHeaders, datatLen, key)
{
    ALOGD("Cta5APEFile(mimeType, cid, dcfFlHeaders, datatLen, key)");
}

Cta5APEFile::Cta5APEFile(String8 mimeType, uint64_t datatLen, String8 key) :
        Cta5CommonMultimediaFile(mimeType, datatLen, key)
{
    ALOGD("Cta5APEFile(mimeType, datatLen, key)");
}

bool Cta5APEFile::parseHeaders(int fd) {
    ALOGD("parseHeaders()");
    off_t pos = 0;
    // uint32_t header = 0;
    ape_parser_ctx_t ape_ctx;

    if (!getAPEInfo(fd, &pos, &ape_ctx, false))
    {
        return false;
    }
    ALOGD("found APE header, offset[0], size[%ld]", pos);

    // stored header information into Vector<header>
    Header * ape_header = new Header();
    ape_header->clear_header_offset = 0;
    ape_header->clear_header_size = pos;

    mHeaders.push_back(ape_header);

    return true;
}


