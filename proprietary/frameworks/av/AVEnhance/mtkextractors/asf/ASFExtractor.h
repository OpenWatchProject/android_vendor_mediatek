/* SPDX-License-Identifier: LGPL-2.1+ */
/*
* Copyright (C) 2010 MediaTek Inc.
*/

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   ASFExtractor.h
 *
 * Project:
 * --------
 *   MT6573
 *
 * Description:
 * ------------
 *   ASF Extractor interface
 *
 * Author:
 * -------
 *   Morris Yang (mtk03147)
 *
 ****************************************************************************/
#ifndef ASF_EXTRACTOR_H_
#define ASF_EXTRACTOR_H_

#include <media/MediaExtractorPluginApi.h>
#include <media/MediaExtractorPluginHelper.h>
#include <media/NdkMediaFormat.h>

#include <media/stagefright/MediaBufferBase.h>
#include <media/stagefright/MetaDataBase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <utils/Vector.h>
#include <utils/threads.h>
#include "asfparser.h"

namespace android {

#define ASF_EVERYTHING_FINE 0
#define ASF_END_OF_TRACK 1
#define ASF_INSUFFICIENT_DATA 2
#define ASF_INSUFFICIENT_BUFFER_SIZE 3
#define ASF_READ_FAILED 4

#define MAX_VIDEO_WIDTH 1920
#define MAX_VIDEO_HEIGHT 1080
#ifdef  MAX_VIDEO_INPUT_SIZE
#undef  MAX_VIDEO_INPUT_SIZE
#endif
#define MAX_VIDEO_INPUT_SIZE (MAX_VIDEO_WIDTH*MAX_VIDEO_HEIGHT*3 >> 1)
#define MAX_AUDIO_INPUT_SIZE (1024*20)

#define ASFFF_SHOW_TIMESTAMP        (1 << 0)
#define ASFFF_IGNORE_AUDIO_TRACK    (1 << 1)
#define ASFFF_IGNORE_VIDEO_TRACK    (1 << 2)

#define ASF_THUMBNAIL_SCAN_SIZE 10

struct AMessage;
class DataSource;
class String8;

#define GECKO_VERSION                 ((1L<<24)|(0L<<16)|(0L<<8)|(3L))

enum AsfStreamType {
    ASF_VIDEO,
    ASF_AUDIO,
    ASF_OTHER
};

enum {
    IVOP,
    PVOP,
    BVOP,
    BIVOP,
    SKIPPED
};


struct  VC1SeqData {
    uint32_t profile;
    uint32_t level;
    uint32_t rangered;
    uint32_t maxbframes;
    uint32_t finterpflag;
    uint32_t multires;
    uint32_t fps100;
    uint64_t us_time_per_frame;
};

struct AsfTrackInfo {
    uint32_t mTrackNum;
    AMediaFormat *mMeta;
    void * mCodecSpecificData;
    uint32_t mCodecSpecificSize;
    asf_packet_t * mNextPacket;
    uint32_t mCurPayloadIdx;
};

struct  VC1PicData {
    uint32_t interpfrm;
    uint32_t rangeredfrm;
    uint32_t frmcnt;
    uint32_t ptype;
};

enum NAL_Parser_Type {
    ASF_SIZE_NAL_TYPE,
    ASF_START_CODE_TYPE,
    ASF_OTHER_TYPE,
};

enum VC1_FourCC {
    WMV1_FOURCC,
    WMV2_FOURCC,
    WMV3_FOURCC,
    WVC1_FOURCC,
    WMVA_FOURCC
};

class ASFExtractor : public MediaExtractorPluginHelper {
public:
    ASFExtractor(DataSourceHelper *source);

    virtual size_t countTracks();
    virtual MediaTrackHelper *getTrack(size_t index);
    virtual media_status_t getTrackMetaData(AMediaFormat *meta, size_t index, uint32_t flags);

    virtual media_status_t getMetaData(AMediaFormat *meta);
    virtual const char * name() { return "ASFExtractor"; }
    virtual ~ASFExtractor();

protected:
    bool ParseASF();
    int64_t ASFSeekTo(uint32_t seekTimeMs);

    bool RetrieveWmvCodecSpecificData(asf_stream_t *pStreamProp, AMediaFormat *meta,
            VC1SeqData *pCodecSpecificData);
    bool RetrieveWmaCodecSpecificData(asf_stream_t *pStreamProp, AMediaFormat *meta);
    void findThumbnail();

public:
    virtual uint32_t flags() const;
    friend struct ASFSource;

    friend int32_t asf_io_read_func(void *aSource, void *aBuffer, int32_t aSize);
    friend int32_t asf_io_write_func(void *aSource, void *aBuffer, int32_t aSize);
    friend int64_t asf_io_seek_func(void *aSource, int64_t aOffset);

    bool IsValidAsfFile() { return mIsValidAsfFile; }
    bool ParserASF();
    ASFErrorType GetNextMediaPayload(uint8_t* aBuffer, uint32_t& arSize, uint32_t& arTimeStamp,
            uint32_t& arRepDataSize, bool& bIsKeyFrame,uint32_t CurTrackIndex);
    ASFErrorType GetNextMediaFrame(MediaBufferHelper **out, bool& bIsKeyFrame, AsfStreamType strmType,
            bool *isSeeking,uint32_t CurTrackIndex);
    bool ParserVC1CodecPrivateData(uint8_t*input , uint32_t inputlen, VC1SeqData* pSeqData);
    status_t addMPEG4CodecSpecificData(AMediaFormat *meta);
    status_t addAVCCodecSpecificData(AMediaFormat *mMeta);
    NAL_Parser_Type getNALParserType();
    uint8_t getNALSizeLength();
    bool isNALStartCodeType(const sp<ABuffer> &buffer);
    bool isNALSizeNalType(const sp<ABuffer> &buffer);
    uint8_t getLengthSizeMinusOne(const sp<ABuffer> &buffer);
    uint32_t parseNALSize(uint8_t *data);
    status_t asfMakeAVCCodecSpecificData(const sp<ABuffer> &buffer, AMediaFormat *meta);
    VC1_FourCC getVC1FourCC();
    void MakeFourCCString(uint32_t x, char *s);
    uint32_t MakeStringToIntFourCC(char *s);

private:
    AMediaFormat *mFileMeta;

    bool mIgnoreVideo;
    uint64_t mDurationMs;
    bool mSeekable;

    bool mExtractedThumbnails;

    uint64_t mPrerollTimeUs;
    NAL_Parser_Type mNalParType;
    VC1_FourCC mVC1FourCC;
    uint8_t mSizeLength;

    ASFExtractor(const ASFExtractor &);
    ASFExtractor &operator=(const ASFExtractor &);

protected:
    DataSourceHelper *mDataSource;
    Vector<AsfTrackInfo> mTracks;
    android::Mutex mCacheLock;

    ASFParser* mAsfParser;
    uint32_t mAsfParserReadOffset;
    bool mIsValidAsfFile;
    bool mIsAsfParsed;
    bool mHasVideo;
    bool mHasVideoTrack;
    uint64_t mFileSize;
};

int switchAACSampleRateToIndex_asf(int sample_rate);

bool SniffASF(DataSourceHelper *source, float *confidence);
}  // namespace android

#endif  // RM_EXTRACTOR_H_
