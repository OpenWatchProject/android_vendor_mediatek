#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Play2Way"
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include <utils/threads.h>

#include "SpeechPcm2way.h"
#include "SpeechType.h"

/*****************************************************************************
*                          C O N S T A N T S
******************************************************************************
*/
#define AUDIO_INPUT_BUFFER_SIZE  (16384) // 16k
#define AUDIO_OUTPUT_BUFFER_SIZE (16384) // 16k

namespace android {
/*==============================================================================
 *                     Property keys
 *============================================================================*/
const char* PROPERTY_KEY_P2W_DUMP_ON = "persist.vendor.audiohal.p2w_dump_on";

/*==============================================================================
 *                     Constant
 *============================================================================*/
static const char kFileNamePlay2Way[] = "/data/vendor/audiohal/audio_dump/Play2Way";
static const char kFileNameRecord2Way[] = "/data/vendor/audiohal/audio_dump/Record2Way";
static const uint32_t kMaxSizeOfFileName = 128;
static const uint32_t kSizeOfPrefixFileNamePlay2Way = sizeof(kFileNamePlay2Way) - 1;
static const uint32_t kSizeOfPrefixFileNameRecord2Way = sizeof(kFileNameRecord2Way) - 1;

/***********************************************************
*
*   PCM2WAY Interface -  Play2Way
*
***********************************************************/

#ifdef PLAY2WAY_USE_SINE_WAVE
static const uint16_t table_1k_tone_8000_hz[] = {
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,

    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F,
    0x0000, 0x5A81, 0x7FFF, 0x5A81,
    0x0000, 0xA57E, 0x8001, 0xA57F
};
static const uint32_t kSizeSinewaveTable = sizeof(table_1k_tone_8000_hz);
#endif

static bool IsPcm2WayDumpEnable() {
    // P2W Log system property
    char property_value[PROPERTY_VALUE_MAX];
    property_get(PROPERTY_KEY_P2W_DUMP_ON, property_value, "0"); //"0": default off
    if (property_value[0] == '1') {
        return true;
    } else {
        return false;
    }
}

Play2Way *Play2Way::mPlay2Way = NULL;
Play2Way *Play2Way::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mPlay2Way == NULL) {
        mPlay2Way = new Play2Way();
    }
    ASSERT(mPlay2Way != NULL);
    return mPlay2Way;
}

Play2Way::Play2Way() {
    // Internal Output Buffer Initialization
    memset((void *)&m_OutputBuf, 0, sizeof(m_OutputBuf));
    m_OutputBuf.pBufBase = new char[AUDIO_OUTPUT_BUFFER_SIZE];
    m_OutputBuf.bufLen   = AUDIO_OUTPUT_BUFFER_SIZE;
    m_OutputBuf.pRead    = m_OutputBuf.pBufBase;
    m_OutputBuf.pWrite   = m_OutputBuf.pBufBase;

    ASSERT(m_OutputBuf.pBufBase != NULL);
    memset(m_OutputBuf.pBufBase, 0, m_OutputBuf.bufLen);

    mPlay2WayStarted = false;
    mIsPlay2WayDumpEnable = false;
    pPlay2WayDumpFile = NULL;
}

Play2Way::~Play2Way() {
    //if (pLad != NULL) pLad->pCCCI->Play2WayLock();

    AL_LOCK(mPlay2WayLock);
    if (m_OutputBuf.pBufBase != NULL) {
        delete[] m_OutputBuf.pBufBase;
        m_OutputBuf.pBufBase = NULL;
        m_OutputBuf.bufLen   = 0;
        m_OutputBuf.pRead    = NULL;
        m_OutputBuf.pWrite   = NULL;
    }
    AL_UNLOCK(mPlay2WayLock);

    //if (pLad != NULL) pLad->pCCCI->Play2WayUnLock();
}


int Play2Way::Start() {
    ALOGD("%s()", __FUNCTION__);

    // Reset read and write pointer of Output buffer
    AL_LOCK(mPlay2WayLock);

    m_OutputBuf.bufLen   = AUDIO_OUTPUT_BUFFER_SIZE;
    m_OutputBuf.pRead    = m_OutputBuf.pBufBase;
    m_OutputBuf.pWrite   = m_OutputBuf.pBufBase;
    mPlay2WayStarted = true;

    AL_UNLOCK(mPlay2WayLock);
    mIsPlay2WayDumpEnable = IsPcm2WayDumpEnable();

    if (mIsPlay2WayDumpEnable) {
        char fileNamePlay2Way[kMaxSizeOfFileName];
        memset((void *)fileNamePlay2Way, 0, kMaxSizeOfFileName);

        time_t rawtime;
        time(&rawtime);
        struct tm *timeinfo = localtime(&rawtime);
        audio_strncpy(fileNamePlay2Way, kFileNamePlay2Way, kMaxSizeOfFileName);
        strftime(fileNamePlay2Way + kSizeOfPrefixFileNamePlay2Way, kMaxSizeOfFileName - kSizeOfPrefixFileNamePlay2Way - 1, "_%Y_%m_%d_%H%M%S.pcm", timeinfo);

        if (pPlay2WayDumpFile == NULL) {
            AudiocheckAndCreateDirectory(fileNamePlay2Way);
            pPlay2WayDumpFile = fopen(fileNamePlay2Way, "wb");
        }
        if (pPlay2WayDumpFile == NULL) {
            ALOGW("%s(), Fail to open %s", __FUNCTION__, fileNamePlay2Way);
        } else {
            ALOGD("%s(), open %s", __FUNCTION__, fileNamePlay2Way);
        }
    }
    return true;
}

int Play2Way::Stop() {
    ALOGD("%s()", __FUNCTION__);
    AL_LOCK(mPlay2WayLock);

    mPlay2WayStarted = false;

    AL_UNLOCK(mPlay2WayLock);

    if (pPlay2WayDumpFile != NULL) {
        fclose(pPlay2WayDumpFile);
        pPlay2WayDumpFile = NULL;
    }
    return true;
}

int Play2Way::Write(void *buffer, int size_bytes) {
    ALOGD("+%s(), size_bytes=%d", __FUNCTION__, size_bytes);
    if (mPlay2WayStarted == false) {
        ALOGE("%s(), mPlay2WayStarted == false, return", __FUNCTION__);
        return 0;
    }

    AL_LOCK(mPlay2WayLock);

    int num_free_space = RingBuf_getFreeSpace(&m_OutputBuf);
    if (size_bytes > num_free_space) {
        ALOGE("%s(), size_bytes(%u) > num_free_space(%u), drop", __FUNCTION__, size_bytes, num_free_space);
        AL_UNLOCK(mPlay2WayLock);
        return 0;
    }

    RingBuf_copyFromLinear(&m_OutputBuf, (char *)buffer, size_bytes);

    AL_UNLOCK(mPlay2WayLock);

    return size_bytes;
}

/** get free space of internal buffer */
int Play2Way::GetFreeBufferCount() {
    int freeSpaceInpBuf = m_OutputBuf.bufLen - RingBuf_getDataCount(&m_OutputBuf) - 8;//left additional 1 byte for ringbuf read/write ptr
    SLOGV("%s(), buf_cnt:%d, free_cnt:%d", __FUNCTION__, RingBuf_getDataCount(&m_OutputBuf), freeSpaceInpBuf);
    return freeSpaceInpBuf;
}


uint16_t Play2Way::PutDataToSpeaker(char *target_ptr, uint16_t num_data_request) {
    SLOGV("%s(), pcm_dataRequest=%d", __FUNCTION__, num_data_request);

    AL_LOCK(mPlay2WayLock);

#ifndef PLAY2WAY_USE_SINE_WAVE
    // check the output buffer data count
    int OutputBufDataCount = RingBuf_getDataCount(&m_OutputBuf);
    SLOGV("%s(), OutputBufDataCount=%d", __FUNCTION__, OutputBufDataCount);

    // if output buffer's data is not enough, fill it with zero to PCMdataToModemSize (ex: 320 bytes)
    if (OutputBufDataCount < num_data_request) {
        RingBuf_writeDataValue(&m_OutputBuf, 0, num_data_request - OutputBufDataCount);
        ALOGW("%s(), underflow OutBufSize:%d", __FUNCTION__, OutputBufDataCount);
    }

    // fill downlink data to share buffer
    RingBuf_copyToLinear(target_ptr, &m_OutputBuf, num_data_request);

    SLOGV("OutputBuf B:0x%p, R:%d, W:%d, L:%d", m_OutputBuf.pBufBase, (int)(m_OutputBuf.pRead - m_OutputBuf.pBufBase), (int)(m_OutputBuf.pWrite - m_OutputBuf.pBufBase), m_OutputBuf.bufLen);
#else
    static uint32_t i4Count = 0;
    uint32_t current_count = 0, remain_count = 0;
    char *tmp_ptr = NULL;

    remain_count = num_data_request;
    tmp_ptr = target_ptr;

    if (remain_count > (kSizeSinewaveTable - i4Count)) {
        memcpy(tmp_ptr, table_1k_tone_8000_hz + (i4Count >> 1), kSizeSinewaveTable - i4Count);
        tmp_ptr += (kSizeSinewaveTable - i4Count);
        remain_count -= (kSizeSinewaveTable - i4Count);
        i4Count = 0;
    }
    //while (remain_count > kSizeSinewaveTable)
    if (remain_count > kSizeSinewaveTable) {
        memcpy(tmp_ptr, table_1k_tone_8000_hz, kSizeSinewaveTable);
        tmp_ptr += kSizeSinewaveTable;
        remain_count -= kSizeSinewaveTable;
    }
    if (remain_count > 0) {
        memcpy(tmp_ptr, table_1k_tone_8000_hz, remain_count);
        i4Count = remain_count;
    }
#endif
    if (mIsPlay2WayDumpEnable) {
        if (pPlay2WayDumpFile != NULL) {
            fwrite(target_ptr, sizeof(char), num_data_request, pPlay2WayDumpFile);
        } else {
            ALOGW("%s(), pPlay2WayDumpFile == NULL!!!!!", __FUNCTION__);
        }
    }
    AL_UNLOCK(mPlay2WayLock);
    return num_data_request;
}


/***********************************************************
*
*   PCM2WAY Interface -  Record2Way
*
***********************************************************/
#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "Record2Way"

Record2Way *Record2Way::mRecord2Way = NULL;

Record2Way *Record2Way::GetInstance() {
    static Mutex mGetInstanceLock;
    Mutex::Autolock _l(mGetInstanceLock);

    if (mRecord2Way == NULL) {
        mRecord2Way = new Record2Way();
    }
    return mRecord2Way;
}

Record2Way::Record2Way() {
    // Internal Input Buffer Initialization
    memset((void *)&m_InputBuf, 0, sizeof(RingBuf));
    m_InputBuf.pBufBase = new char[AUDIO_INPUT_BUFFER_SIZE];
    m_InputBuf.bufLen   = AUDIO_INPUT_BUFFER_SIZE;
    m_InputBuf.pRead    = m_InputBuf.pBufBase;
    m_InputBuf.pWrite   = m_InputBuf.pBufBase;

    ASSERT(m_InputBuf.pBufBase != NULL);
    memset(m_InputBuf.pBufBase, 0, m_InputBuf.bufLen);

    m_Rec2Way_Started = false;
    mIsRecord2WayDumpEnable = false;
    pRecord2WayDumpFile = NULL;
}

Record2Way::~Record2Way() {
    //if (pLad != NULL) pLad->pCCCI->Record2WayLock();

    AL_LOCK(mRec2WayLock);
    if (m_InputBuf.pBufBase != NULL) {
        delete []m_InputBuf.pBufBase;
        m_InputBuf.pBufBase = NULL;
        m_InputBuf.bufLen   = 0;
        m_InputBuf.pRead    = NULL;
        m_InputBuf.pWrite   = NULL;
    }
    AL_UNLOCK(mRec2WayLock);

    //if (pLad != NULL) pLad->pCCCI->Record2WayUnLock();
}


int Record2Way::Start() {
    ALOGD("%s()", __FUNCTION__);
    AL_LOCK(mRec2WayLock);
    mIsRecord2WayDumpEnable = IsPcm2WayDumpEnable();

    if (mIsRecord2WayDumpEnable) {
        char fileNameRecord2Way[kMaxSizeOfFileName];
        memset((void *)fileNameRecord2Way, 0, kMaxSizeOfFileName);

        time_t rawtime;
        time(&rawtime);
        struct tm *timeinfo = localtime(&rawtime);
        audio_strncpy(fileNameRecord2Way, kFileNameRecord2Way, kMaxSizeOfFileName);
        strftime(fileNameRecord2Way + kSizeOfPrefixFileNameRecord2Way, kMaxSizeOfFileName - kSizeOfPrefixFileNameRecord2Way - 1, "_%Y_%m_%d_%H%M%S.pcm", timeinfo);
        if (pRecord2WayDumpFile == NULL) {
            AudiocheckAndCreateDirectory(fileNameRecord2Way);
            pRecord2WayDumpFile = fopen(fileNameRecord2Way, "wb");
        }
        if (pRecord2WayDumpFile == NULL) {
            ALOGW("%s(), Fail to open %s", __FUNCTION__, fileNameRecord2Way);
        } else {
            ALOGD("%s(), open %s", __FUNCTION__, fileNameRecord2Way);
        }
    }
    m_Rec2Way_Started = true;

    // Reset read and write pointer of Input buffer
    m_InputBuf.bufLen = AUDIO_INPUT_BUFFER_SIZE;
    m_InputBuf.pRead  = m_InputBuf.pBufBase;
    m_InputBuf.pWrite = m_InputBuf.pBufBase;

    AL_UNLOCK(mRec2WayLock);
    return true;
}

int Record2Way::Stop() {
    ALOGD("%s()", __FUNCTION__);
    AL_LOCK(mRec2WayLock);

    m_Rec2Way_Started = false;

    AL_UNLOCK(mRec2WayLock);

    if (pRecord2WayDumpFile != NULL) {
        fclose(pRecord2WayDumpFile);
        pRecord2WayDumpFile = NULL;
    }
    return true;
}

#define READ_DATA_FROM_MODEM_FAIL_CNT 10

int Record2Way::Read(void *buffer, int size_bytes) {
    int ret = 0;
    int InputBuf_dataCnt = 0;
    int ReadDataAgain = 0;
    int consume_byte = size_bytes;
    char *buf = (char *)buffer;
    ALOGD("+%s(), size_bytes=%d", __FUNCTION__, size_bytes);

    if (m_Rec2Way_Started == false) {
        ALOGD("Record2Way_Read, m_Rec2Way_Started=false");
        return 0;
    }

    // if internal input buffer has enough data for this read, do it and return
    AL_LOCK(mRec2WayLock);
    InputBuf_dataCnt = RingBuf_getDataCount(&m_InputBuf);
    if (InputBuf_dataCnt >= consume_byte) {
        RingBuf_copyToLinear(buf, &m_InputBuf, consume_byte);
        AL_UNLOCK(mRec2WayLock);
        return consume_byte;
    }
    AL_UNLOCK(mRec2WayLock);


    // if internal input buffer is not enough, keep on trying
    while (ReadDataAgain++ < READ_DATA_FROM_MODEM_FAIL_CNT) {
        if (ReadDataAgain > (READ_DATA_FROM_MODEM_FAIL_CNT - 1)) {
            ALOGW("Record2Way_Read, fail, No data from modem: %d (%d)", ReadDataAgain, InputBuf_dataCnt);
        }
        // Interrupt period of pcm2way driver is 20ms.
        // If wait too long time (150 ms),
        //  -- Modem side has problem, the no interrupt is issued.
        //  -- pcm2way driver is stop. So AP can't read the data from modem.

        //wait some time then get data again from modem.
        usleep(15 * 1000);
        //Read data from modem again
        AL_LOCK(mRec2WayLock);
        InputBuf_dataCnt = RingBuf_getDataCount(&m_InputBuf);
        if (InputBuf_dataCnt >= consume_byte) {
            RingBuf_copyToLinear((char *)buf, &m_InputBuf, consume_byte);
            AL_UNLOCK(mRec2WayLock);
            return consume_byte;
        }
        AL_UNLOCK(mRec2WayLock);
    }

    ALOGD("Record2Way_Read, Modem fail");
    return 0;
}

int Record2Way::GetBufferDataCount() {
    AL_LOCK(mRec2WayLock);
    int InputBuf_dataCnt = RingBuf_getDataCount(&m_InputBuf);
    AL_UNLOCK(mRec2WayLock);

    return InputBuf_dataCnt;
}

void Record2Way::GetDataFromMicrophone(RingBuf ul_ring_buf) {
    size_t InpBuf_freeSpace = 0;
    size_t ShareBuf_dataCnt = 0;

    AL_LOCK(mRec2WayLock);

    // get free space of internal input buffer
    InpBuf_freeSpace = (size_t)RingBuf_getFreeSpace(&m_InputBuf);
    SLOGV("%s(), input_Buf data_cnt:%d, freeSpace:%zu", __FUNCTION__, RingBuf_getDataCount(&m_InputBuf), InpBuf_freeSpace);

    // get data count in share buffer
    ShareBuf_dataCnt = (size_t) RingBuf_getDataCount(&ul_ring_buf);
    SLOGV("%s(), share_Buf data_count:%zu", __FUNCTION__, ShareBuf_dataCnt);

    if (mIsRecord2WayDumpEnable) {
        char linear_buffer[ShareBuf_dataCnt];

        char *pM2AShareBufEnd = ul_ring_buf.pBufBase + ul_ring_buf.bufLen;
        if (ul_ring_buf.pRead + ShareBuf_dataCnt <= pM2AShareBufEnd) {
            memcpy(linear_buffer, ul_ring_buf.pRead, ShareBuf_dataCnt);
        } else {
            uint32 r2e = pM2AShareBufEnd - ul_ring_buf.pRead;
            memcpy(linear_buffer, ul_ring_buf.pRead, r2e);
            memcpy((void *)(linear_buffer + r2e), ul_ring_buf.pBufBase, ShareBuf_dataCnt - r2e);
        }
        if (pRecord2WayDumpFile != NULL) {
            fwrite(linear_buffer, sizeof(char), ShareBuf_dataCnt, pRecord2WayDumpFile);
        } else {
            ALOGW("%s(), pRecord2WayDumpFile == NULL!!!!!", __FUNCTION__);
        }
    }
    // check the data count in share buffer
    if (ShareBuf_dataCnt > 320) {
        SLOGV("%s(), ul_ring_buf size(%zu) > 320", __FUNCTION__, ShareBuf_dataCnt);
    }

    // check free space for internal input buffer
    if (ShareBuf_dataCnt > InpBuf_freeSpace) {
        SLOGV("%s(), uplink buffer full", __FUNCTION__);
        AL_UNLOCK(mRec2WayLock);
        return;
    }

    // copy data from modem share buffer to internal input buffer
    RingBuf_copyEmpty(&m_InputBuf, &ul_ring_buf);

    SLOGV("%s(), InputBuf B:0x%p, R:%d, W:%d, L:%u", __FUNCTION__, m_InputBuf.pBufBase, (int)(m_InputBuf.pRead - m_InputBuf.pBufBase), (int)(m_InputBuf.pWrite - m_InputBuf.pBufBase), m_InputBuf.bufLen);
    SLOGV("%s(), M2A_ShareBuf B:0x%p, R:%d, W:%d, L:%d", __FUNCTION__, ul_ring_buf.pBufBase, (int)(ul_ring_buf.pRead - ul_ring_buf.pBufBase), (int)(ul_ring_buf.pWrite - ul_ring_buf.pBufBase), ul_ring_buf.bufLen);

    AL_UNLOCK(mRec2WayLock);
}

} // end of namespace android
