#include "AudioMixerOut.h"

#include <audio_utils/format.h>
#include <fstream>
#include <limits>
#include <unordered_map>

#include "AudioALSAPlaybackHandlerBase.h"
#include "AudioALSAStreamManager.h"
#include "AudioALSADriverUtility.h"
#include "AudioUtility.h"
#include "MtkAudioComponent.h"
#include "WCNChipController.h"
#include <SpeechUtility.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioMixerOut"

#define MIX_OUT_MIX_FORMAT AUDIO_FORMAT_PCM_FLOAT

// pcm dump
#define MIX_OUT_DUMP_FILE_PREFIX "/data/vendor/audiohal/audio_dump/mix.out.pcm"
#define MIX_OUT_DUMP_NAME "mixed"

// debug
#define MIX_OUT_DEBUG "vendor.mix.out.debug"

enum MIX_OUT_DBG_TYPE {
    MIX_OUT_DBG_ALL = 0xFFFFFFFF,
};

namespace android {

using namespace std;

static AudioLock mGetInstanceLock;
unordered_map<enum MIXER_USAGE, AudioMixerOut *> mInstanceSet;

AudioMixerOut *AudioMixerOut::getInstance(enum MIXER_USAGE usage) {
    AL_AUTOLOCK(mGetInstanceLock);

    AudioMixerOut *instance;

    if (mInstanceSet.find(usage) == mInstanceSet.end()) {
        instance = new AudioMixerOut(usage);
        mInstanceSet.insert({usage, instance});
    } else {
        instance = mInstanceSet.find(usage)->second;
    }

    return instance;
}

AudioMixerOut::AudioMixerOut(enum MIXER_USAGE usage) :
    mUsage(usage),
    mOutThread(0),
    mDebugType(0) {
    ALOGD("%s(), usage %d", __FUNCTION__, usage);
    memset(&mOutInfo, 0, sizeof(struct MixerOutInfo));

    mClients.clear();
    mClientsLock.clear();
}

AudioMixerOut::~AudioMixerOut() {
    AL_AUTOLOCK(mGetInstanceLock);

    if (mClients.size() > 0) {
        for (size_t i = 0; i < mClients.size(); i++) {
            detach(mClients.keyAt(i));
        }
    }

    AL_AUTOLOCK(mLock);

    mClients.clear();

    for (size_t i = 0; i < mClientsLock.size(); i++) {
        delete mClientsLock[i];
    }
    mClientsLock.clear();

    mInstanceSet.erase(mUsage);
}

status_t AudioMixerOut::attach(const void *id, const stream_attribute_t *attribute) {
    status_t status = NO_ERROR;

    ALOGD("+%s(), id %p, flag %d, mClients.size() %zu, mUsage %d",
          __FUNCTION__, id, attribute->mAudioOutputFlags, mClients.size(), mUsage);

    AL_AUTOLOCK(mLock);
    AL_AUTOLOCK(mThreadLock);

    // check if already exist
    ssize_t idx = mClients.indexOfKey(id);
    if (idx >= 0) {
        ALOGE("%s(), id %p already exixt, flag %d, mClients.size() %zu",
              __FUNCTION__, id, attribute->mAudioOutputFlags, mClients.size());
        ASSERT(0);
        return INVALID_OPERATION;
    }

    // create client
    struct MixerOutClient *client = new MixerOutClient();
    if (!client) {
        ALOGE("%s(), new MixerOutClient failed, id %p, flag %d, mClients.size() %zu",
              __FUNCTION__, id, attribute->mAudioOutputFlags, mClients.size());
        ASSERT(0);
        return NO_MEMORY;
    }

    memset(client, 0, sizeof(struct MixerOutClient));

    client->id = id;
    client->attribute = *attribute;
    client->dataBufferLock = new AudioLock();

    // rate, format convert if needed
    if (mClients.size() > 0) {
        initBliSrc(client, &mOutInfo);
    }

    initBitConverter(client, MIX_OUT_MIX_FORMAT);

    // allocate data buffer for client
    size_t sizePerFrame = getSizePerFrame(MIX_OUT_MIX_FORMAT, attribute->num_channels);
    size_t dataBufferSize = (attribute->latency * attribute->sample_rate * sizePerFrame) / 1000;    // TODO:  use latency(int) are not accurate...
    if (attribute->frame_count * 2 * sizePerFrame > dataBufferSize) {
        dataBufferSize = attribute->frame_count * 2 * sizePerFrame; // at least, require two buffersize
    }

    if (client->blisrc) {
        dataBufferSize *= mOutInfo.attribute.sample_rate;
        dataBufferSize /= attribute->sample_rate;
    }

    AL_LOCK(client->dataBufferLock);
    client->dataBuffer.bufLen = dataBufferSize + RING_BUF_SIZE_OFFSET;
    client->dataBuffer.pBufBase = new char[client->dataBuffer.bufLen];
    client->dataBuffer.pRead = client->dataBuffer.pBufBase;
    client->dataBuffer.pWrite = client->dataBuffer.pBufBase;
    client->dataBuffer.pBufEnd = client->dataBuffer.pBufBase + client->dataBuffer.bufLen;
    AL_UNLOCK(client->dataBufferLock);

    if (!client->dataBuffer.pBufBase) {
        ALOGE("%s(), allocate dataBuffer failed, id %p, flag %d, mClients.size() %zu",
              __FUNCTION__, id, attribute->mAudioOutputFlags, mClients.size());
        ASSERT(0);
        delete client;
        return NO_MEMORY;
    }

    ALOGD("%s(), client->dataBuffer.bufLen %d, frame_count %zu, ch %u, rate %u",
          __FUNCTION__,
          client->dataBuffer.bufLen,
          client->attribute.frame_count,
          attribute->num_channels,
          attribute->sample_rate);

    client->writtenBytes = 0;
    client->isDumpLatency = get_uint32_from_property("vendor.audiohal.dump_latency");
    uint32_t unitSleepUs = get_uint32_from_property("vendor.audiohal.sleep_unit");
    client->unitSleepUs = (unitSleepUs != 0) ? unitSleepUs: 1000;
    uint32_t errorThdSleepUs = get_uint32_from_property("vendor.audiohal.sleep_error_thd");
    client->errorThdSleepUs = (errorThdSleepUs != 0) ? errorThdSleepUs: 2000;

    ALOGD("%s(), isDumpLatency = %d, buffer_size = %d, unitSleepUs = %d, thdSleepUs = %d",
          __FUNCTION__,
          client->isDumpLatency,
          client->attribute.buffer_size,
          client->unitSleepUs,
          client->errorThdSleepUs);

    // add client
    mClients.add(client->id, client);

    // open when 1st attach
    if (mClients.size() == 1) {
        status = createOutThread();
    }

    // create client lock if needed, expect id will not change among same stream out(s), e.g. 3 streamout 3 id
    if (mClientsLock.indexOfKey(id) < 0) {
        mClientsLock.add(id, new AudioLock);
    }

    setScreenState_l(&mOutInfo);

    ALOGD("-%s()", __FUNCTION__);
    return status;
}

void AudioMixerOut::detach(const void *id) {
    ALOGD("+%s(), id %p, mClients.size() %zu, mUsage %d",
          __FUNCTION__, id, mClients.size(), mUsage);

    AL_AUTOLOCK(mLock);
    ssize_t idx = mClients.indexOfKey(id);
    if (idx >= 0) {
        AL_LOCK(mThreadLock);

        AL_LOCK(mClientsLock.valueFor(id));

        deleteClient(mClients[idx]);

        // remove client
        mClients.removeItem(id);

        AL_UNLOCK(mClientsLock.valueFor(id));

        // remove client lock
        delete mClientsLock.valueFor(id);
        mClientsLock.removeItem(id);

        // signal out thread a client is detached
        AL_LOCK(mWaitOutThreadLock);
        AL_SIGNAL(mWaitOutThreadLock);
        AL_UNLOCK(mWaitOutThreadLock);

        setScreenState_l(&mOutInfo);

        AL_UNLOCK(mThreadLock);
        // destroy out thread when no client attached
        if (mClients.size() == 0) {
            destroyOutThread();
        }
    } else {
        ALOGE("%s(), client not found, idx %zd, id %p", __FUNCTION__, idx, id);
        ASSERT(0);
    }

    ALOGD("-%s()", __FUNCTION__);
}

size_t AudioMixerOut::write(const void *id, const void *buffer, size_t bytes) {
    ALOGV("%s(), bytes %zu, id %p", __FUNCTION__, bytes, id);

    AL_AUTOLOCK(mClientsLock.valueFor(id));

    ssize_t idx = mClients.indexOfKey(id);
    if (idx < 0) {
        ALOGE("%s(), client not found, idx %zd, id %p", __FUNCTION__, idx, id);
        ASSERT(0);
        return 0;
    }

    struct MixerOutClient *client = mClients[idx];

    // src
    // const -> to non const
    void *pBuffer = const_cast<void *>(buffer);
    void *pBufferAfterBliSrc = NULL;
    uint32_t bytesAfterBliSrc = 0;
    doBliSrc(client,
             pBuffer, bytes,
             &pBufferAfterBliSrc, &bytesAfterBliSrc);

    // bit conversion
    void *pBufferAfterBitConvertion = NULL;
    uint32_t bytesAfterBitConvertion = 0;
    doBitConversion(client,
                    pBufferAfterBliSrc, bytesAfterBliSrc,
                    &pBufferAfterBitConvertion, &bytesAfterBitConvertion);

    char *writeBuffer = (char *)pBufferAfterBitConvertion;
    int writeBytes = bytesAfterBitConvertion;

    AL_LOCK(client->dataBufferLock);

    // check if buffer enough
    RingBuf_dynamicChangeBufSize(&client->dataBuffer, writeBytes);

    if (writeBytes) {
        uint32_t freeSpace = RingBuf_getFreeSpace(&client->dataBuffer);

        if (freeSpace < writeBytes) {
            RingBuf_copyFromLinear(&client->dataBuffer, writeBuffer, freeSpace);
        } else {
            RingBuf_copyFromLinear(&client->dataBuffer, writeBuffer, writeBytes);
        }
    }

    AL_UNLOCK(client->dataBufferLock);

    // signal out thread of data written
    AL_LOCK(mWaitOutThreadLock);
    AL_SIGNAL(mWaitOutThreadLock);
    AL_UNLOCK(mWaitOutThreadLock);

    if (client->writtenBytes == 0) {
        client->startTime = systemTime(0);
    }
    client->writtenBytes += bytes;
    int targetTimeUs = (client->writtenBytes * 1000 * 1000) /
                          getSizePerFrame(client->attribute.audio_format, client->attribute.num_channels) /
                          client->attribute.sample_rate;
    int procTimeUs = (systemTime(0) - client->startTime) / 1000;
    int sleepTimeUs = targetTimeUs - procTimeUs;
    if (sleepTimeUs > 0) {
        if (client->isDumpLatency != 0) {
            ALOGD("%s(), sleepTimeUs = %d, procTimeUs = %d, targetTimeUs = %d, writtenBytes = %ju, bytes = %zu",
                  __FUNCTION__, sleepTimeUs, procTimeUs, targetTimeUs, client->writtenBytes, bytes);
        }
        adaptiveSleepUs(sleepTimeUs, client->unitSleepUs, client->errorThdSleepUs);
    }

    return bytes;
}

status_t AudioMixerOut::setSuspend(const void *id, bool suspend) {
    ALOGD("%s(), id %p, suspend %d", __FUNCTION__, id, suspend);

    AL_AUTOLOCK(mWaitSuspendLock);
    AL_LOCK(mLock);

    ssize_t idx = mClients.indexOfKey(id);
    if (idx < 0) {
        ALOGW("%s(), client not found, idx %zd, id %p", __FUNCTION__, idx, id);
        AL_UNLOCK(mLock);
        return 0;
    }

    if (mClients[idx]->suspend == suspend) {
        ALOGW("%s(), suspend status not changed, suspend %d", __FUNCTION__, suspend);
        ASSERT(0);

        AL_UNLOCK(mLock);
        return 0;
    }

    mClients[idx]->suspend = suspend;

    // wait for close hardware when all client suspend
    mOutInfo.clientAllSuspend = clientAllSuspend(&mClients);

    AL_UNLOCK(mLock);

    // signal out thread to check suspend status
    AL_LOCK(mWaitOutThreadLock);
    AL_SIGNAL(mWaitOutThreadLock);
    AL_UNLOCK(mWaitOutThreadLock);

    if (mOutInfo.clientAllSuspend) {
        ALOGD("%s(), all clients suspend, wait for hardware close", __FUNCTION__);

        CLEANUP_PUSH_ALOCK(mWaitSuspendLock.getAlock());

        int waitResult = AL_WAIT_MS(mWaitSuspendLock, 2800);

        if (waitResult != 0) {
            ALOGW("%s(), waitResult %d", __FUNCTION__, waitResult);
            ASSERT(0);
        }

        CLEANUP_POP_ALOCK(mWaitSuspendLock.getAlock());
    }

    ALOGD("-%s()", __FUNCTION__);
    return 0;
}

status_t AudioMixerOut::getHardwareBufferInfo(const void *id, time_info_struct_t *HWBuffer_Time_Info) {
    AL_AUTOLOCK(mThreadLock);
    AL_AUTOLOCK(mClientsLock.valueFor(id));

    ssize_t idx = mClients.indexOfKey(id);
    if (idx < 0) {
        ALOGE("%s(), client not found, idx %zd, id %p", __FUNCTION__, idx, id);
        ASSERT(0);
        return UNKNOWN_ERROR;
    }

    struct MixerOutClient *client = mClients[idx];

    if (!mOutInfo.playHandler) {
        return UNKNOWN_ERROR;
    }

    if (mOutInfo.playHandler->getHardwareBufferInfo(HWBuffer_Time_Info) != NO_ERROR) {
        return UNKNOWN_ERROR;
    } else {
        size_t outSizePerFrame = getSizePerFrame(MIX_OUT_MIX_FORMAT, mOutInfo.attribute.num_channels);
        AL_LOCK(client->dataBufferLock);
        HWBuffer_Time_Info->halQueuedFrame += RingBuf_getDataCount(&client->dataBuffer) / outSizePerFrame;
        AL_UNLOCK(client->dataBufferLock);
    }
    return NO_ERROR;
}

status_t AudioMixerOut::setScreenState(const void *id, bool mode, size_t bufferSize, size_t reduceInterruptSize, bool force) {
    AL_AUTOLOCK(mThreadLock);
    AL_AUTOLOCK(mClientsLock.valueFor(id));

    ssize_t idx = mClients.indexOfKey(id);
    if (idx < 0) {
        ALOGE("%s(), client not found, idx %zd, id %p", __FUNCTION__, idx, id);
        return UNKNOWN_ERROR;
    }

    struct MixerOutClient *client = mClients[idx];

    client->screenMode = mode;
    client->screenBufferSize = bufferSize;
    client->screenReduceInterruptSize = reduceInterruptSize;
    client->screenForce = force;

    // will decide which client's param to use in setScreenState_l
    return setScreenState_l(&mOutInfo);
}

status_t AudioMixerOut::setScreenState_l(struct MixerOutInfo *info) {
    ASSERT(AL_TRYLOCK(info->threadLock) != 0);

    status_t ret = NO_ERROR;

    // bt don't need set screen
    if (info->usage == MIXER_USAGE_BT) {
        return 0;
    }

    // decide among clinets
    bool useMinBufferSize = true;

    if (info->clients->size() == 1) {
        struct MixerOutClient *client = info->clients->valueAt(0);

        if (client->attribute.mAudioOutputFlags & AUDIO_OUTPUT_FLAG_DEEP_BUFFER) {
            useMinBufferSize = false;
            info->screenMode = client->screenMode;
            info->screenBufferSize = client->screenBufferSize;
            info->screenReduceInterruptSize = client->screenReduceInterruptSize;
            info->screenForce = client->screenForce;
        }
    }

    if (useMinBufferSize) {
        // interrupt period use buffer size
        info->screenMode = true;

        // get minimum buffer size
        info->screenBufferSize = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < info->clients->size(); i++) {
            struct MixerOutClient *client = info->clients->valueAt(i);

            if (client->screenBufferSize < info->screenBufferSize) {
                info->screenBufferSize = client->screenBufferSize;
            }
        }
    }

    if (info->screenBufferSize == 0) {
        return NO_ERROR;
    }

    if (info->playHandler) {
        ret = info->playHandler->setScreenState(info->screenMode,
                                                info->screenBufferSize,
                                                info->screenReduceInterruptSize,
                                                info->screenForce);
        if (ret == NO_ERROR && info->playHandler2) {
            ret = info->playHandler2->setScreenState(info->screenMode,
                                                     info->screenBufferSize,
                                                     info->screenReduceInterruptSize,
                                                     info->screenForce);
        }
    }

    return ret;
}

int AudioMixerOut::getLatency() {
    AL_AUTOLOCK(mThreadLock);

    if (!mOutInfo.playHandler)
        return -EPERM;

    int latency = mOutInfo.playHandler->getLatency();

    if (latency <= 0) {
        const stream_attribute_t *pStreamAttributeTarget = mOutInfo.playHandler->getStreamAttributeTarget();
        latency = getBufferLatencyMs(pStreamAttributeTarget, pStreamAttributeTarget->buffer_size);
    }

    return latency;
}

status_t AudioMixerOut::createOutThread() {
    ALOGD("+%s(), mOutInfo %p", __FUNCTION__, &mOutInfo);

    // initialize mixer out info
    memset(&mOutInfo, 0, sizeof(struct MixerOutInfo));

    mOutInfo.id = &mOutInfo;
    mOutInfo.usage = mUsage;
    mOutInfo.attribute = mClients[0]->attribute;
    mOutInfo.attribute.isMixerOut = true;

    if (mUsage == MIXER_USAGE_BT) {
        mOutInfo.minWriteFrameCnt = 512; // limitation from SWIP
        mOutInfo.attribute.mAudioOutputFlags = (audio_output_flags_t)(mOutInfo.attribute.mAudioOutputFlags & (~AUDIO_OUTPUT_FLAG_FAST));
        mOutInfo.attribute.mAudioOutputFlags = (audio_output_flags_t)(mOutInfo.attribute.mAudioOutputFlags & (~AUDIO_OUTPUT_FLAG_DEEP_BUFFER));
    } else if (mUsage == MIXER_USAGE_DEEP_FAST) {
        mOutInfo.attribute.mAudioOutputFlags = AUDIO_OUTPUT_FLAG_DEEP_BUFFER;
    } else if (mUsage == MIXER_USAGE_SMARTPA) {
        mOutInfo.attribute.mAudioOutputFlags = (audio_output_flags_t)(mOutInfo.attribute.mAudioOutputFlags | AUDIO_OUTPUT_FLAG_FAST);
    }

    mOutInfo.threadLock = &mThreadLock;
    mOutInfo.waitSuspendLock = &mWaitSuspendLock;
    mOutInfo.waitOutThreadLock = &mWaitOutThreadLock;

    mOutInfo.clients = &mClients;
    mOutInfo.clientAllSuspend = false;

    mOutInfo.readBufferTimeUs = mOutInfo.attribute.frame_count * 1000 * 1000 / mOutInfo.attribute.sample_rate;

    initBitConverter(&mOutInfo, MIX_OUT_MIX_FORMAT);

    // set debug info
    char value[PROPERTY_VALUE_MAX];
    property_get(MIX_OUT_DEBUG, value, "0");
    mDebugType = atoi(value);

    // create output thread
    int ret = pthread_create(&mOutThread, NULL, AudioMixerOut::outThread, &mOutInfo);
    if (ret) {
        ALOGE("%s() create outThread fail, ret = %d!!", __FUNCTION__, ret);
        ASSERT(0);
    }
    ret = pthread_setname_np(mOutThread, "audio_mixer_out");
    if (ret) {
        ALOGW("%s(), set mOutThread name fail", __FUNCTION__);
    }

    ALOGD("-%s(), ret %d, output flag %d", __FUNCTION__,
          ret, mOutInfo.attribute.mAudioOutputFlags);
    return ret;
}

void AudioMixerOut::destroyOutThread() {
    ALOGD("+%s()", __FUNCTION__);

    int ret;
    void *retval;

    // pthread_join
    ret = pthread_join(mOutThread, &retval);
    if (ret) {
        ALOGE("%s(), mOutThread pthread_join fail, ret = %d", __FUNCTION__, ret);
        ASSERT(0);
    }

    deinitBitConverter(&mOutInfo);

    ALOGD("-%s()", __FUNCTION__);
}

void AudioMixerOut::deleteClient(struct MixerOutClient *client) {
    // clean client
    if (client->dataBuffer.pBufBase != NULL) {
        delete [] client->dataBuffer.pBufBase;
    }

    if (client->dataBufferLock != NULL) {
        delete client->dataBufferLock;
    }

    deinitBitConverter(client);
    deinitBliSrc(client);

    // delete client
    delete client;
}

void *AudioMixerOut::outThread(void *arg) {
    struct MixerOutInfo *info = (struct MixerOutInfo *)arg;
    MixerOutClientVector *clients = info->clients;
    int ret = 0;

    ALOGD("+%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    AudioALSAStreamManager *streamManager = AudioALSAStreamManager::getInstance();
    info->playHandler= NULL;
    info->playHandler2 = NULL;
    struct mixer *mixer = AudioALSADriverUtility::getInstance()->getMixer();

    if (!streamManager) {
        ALOGD("-%s(), streamManager == NULL, pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
        ASSERT(0);
        return NULL;
    }

    // data pending buf
    RingBuf pendingBuf;
    pendingBuf.bufLen = 0x10000; // 64k
    pendingBuf.pBufBase = new char[pendingBuf.bufLen];
    pendingBuf.pRead = pendingBuf.pBufBase;
    pendingBuf.pWrite = pendingBuf.pBufBase;

    if (!pendingBuf.pBufBase) {
        ALOGE("%s(), allocate pendingBuf failed", __FUNCTION__);
        ASSERT(0);
    }

    bool isBtMerge = WCNChipController::GetInstance()->IsBTMergeInterfaceSupported();

    FILE *pcmMixedDumpFile = mixerOutDumpOpen(MIX_OUT_DUMP_NAME, streamout_propty, info);

    char *inDataBuffer = NULL;
    char *outDataBuffer = NULL;
    size_t outDataBufferSize = 0;

    char *tempWriteBuf = NULL;
    size_t tempWriteBufSize = 0;

    do {
        AL_LOCK(info->threadLock);

        if (clients->size() == 0) {
            AL_UNLOCK(info->threadLock);
            break;
        }

        // handle all suspend
        if (info->clientAllSuspend) {
            destroyPlaybackHandler(info->playHandler, streamManager);
            info->playHandler = NULL;
            destroyPlaybackHandler(info->playHandler2, streamManager);
            info->playHandler2 = NULL;

            AL_SIGNAL(info->waitSuspendLock);

            AL_UNLOCK(info->threadLock);

            // sleep until state change signal
            int waitResult = waitSignal(info->waitOutThreadLock, 100 * 1000,
                                        "out, all suspend, wait state change signal");

            if (waitResult != 0) {
                ALOGW("%s(), waitResult %d", __FUNCTION__, waitResult);
            }

            continue;
        }

        // determine read frame count
        size_t readBufferFrameCount = std::numeric_limits<size_t>::max();
        for (size_t i = 0; i < clients->size(); i++) {
            struct MixerOutClient *client = clients->valueAt(i);

            if (client->attribute.frame_count == 0) {
                ALOGE("%s(), client id %p, frame_count == 0", __FUNCTION__, client->id);
                ASSERT(0);
                continue;
            }

            if (client->attribute.frame_count < readBufferFrameCount) {
                readBufferFrameCount = client->attribute.frame_count;
            }
        }

        info->readBufferTimeUs = readBufferFrameCount * 1000 * 1000 / info->attribute.sample_rate;

        size_t outSizePerFrame = getSizePerFrame(MIX_OUT_MIX_FORMAT, info->attribute.num_channels);
        unsigned int readBufferSize = readBufferFrameCount * outSizePerFrame;

        // check if data enough
        unsigned int waitDataTimeUs = 0;
        for (size_t i = 0; i < clients->size(); i++) {
            struct MixerOutClient *client = clients->valueAt(i);
            AL_LOCK(client->dataBufferLock);
            unsigned int availDataSize = RingBuf_getDataCount(&client->dataBuffer);
            AL_UNLOCK(client->dataBufferLock);

            if (availDataSize < readBufferSize && !client->suspend) {
                waitDataTimeUs = (readBufferSize - availDataSize) * 1000 * 1000 / outSizePerFrame / info->attribute.sample_rate;
                //ALOGD("%s(), waitDataTimeUs %u, readBufferSize %u, availDataSize %u, outSizePerFrame %zu, freeSpace %d, dataCount %d",
                //      __FUNCTION__, waitDataTimeUs, readBufferSize, availDataSize, outSizePerFrame,
                //      RingBuf_getFreeSpace(&client->dataBuffer), RingBuf_getDataCount(&client->dataBuffer));
                break;
            }
        }

        // wait for data if needed
        if (waitDataTimeUs) {
            AL_UNLOCK(info->threadLock);

            int waitResult = waitSignal(info->waitOutThreadLock, waitDataTimeUs,
                                        "out, wait for avail data");

            if (waitResult != 0) {
                ALOGW("%s(), waitResult %d, waitDataTimeUs %u", __FUNCTION__, waitResult, waitDataTimeUs);
            }

            continue;
        }

        // prepare out buffer
        if (outDataBufferSize < readBufferSize) {
            if (outDataBuffer) {
                delete[] outDataBuffer;
            }

            outDataBuffer = new char[readBufferSize];
            outDataBufferSize = readBufferSize;

            if (inDataBuffer) {
                delete[] inDataBuffer;
            }

            inDataBuffer = new char[readBufferSize];
        }

        if (!outDataBuffer) {
            ALOGE("%s(), outDataBuffer == NULL, outDataBufferSize %zu, readBufferSize %u",
                  __FUNCTION__, outDataBufferSize, readBufferSize);
            ASSERT(0);
            AL_UNLOCK(info->threadLock);
            continue;
        }

        memset(outDataBuffer, 0, readBufferSize);

        // mix data
        float *mixBuffer = (float *)outDataBuffer;
        unsigned int mixBufferSize = readBufferSize;
        unsigned int mixBufferCount = mixBufferSize / audio_bytes_per_sample(MIX_OUT_MIX_FORMAT);

        for (size_t c = 0; c < clients->size(); c++) {
            struct MixerOutClient *client = clients->valueAt(c);

            if (client->suspend) {
                continue;
            }

            AL_LOCK(client->dataBufferLock);
            RingBuf_copyToLinear(inDataBuffer, &client->dataBuffer, mixBufferSize);
            AL_UNLOCK(client->dataBufferLock);

            float *tempBuffer = (float *)inDataBuffer;

            for (unsigned int i = 0; i < mixBufferCount; i++) {
                mixBuffer[i] += tempBuffer[i];
            }
        }

        // open playback handler
        if (info->playHandler == NULL) {
            info->playHandler = streamManager->createPlaybackHandler(&info->attribute);
            info->playHandler->open();
            info->playHandler->setFirstDataWriteFlag(true);

            if (isBtSpkDevice(info->attribute.output_devices)) {
                info->attribute2 = info->attribute;
                info->attribute2.output_devices = AUDIO_DEVICE_OUT_BLUETOOTH_SCO;
                info->playHandler2 = streamManager->createPlaybackHandler(&info->attribute2);
                info->playHandler2->open();
                info->playHandler2->setFirstDataWriteFlag(true);
            }

            setScreenState_l(info);
        } else {
            info->playHandler->setFirstDataWriteFlag(false);
            if (info->playHandler2) {
                info->playHandler2->setFirstDataWriteFlag(false);
            }
        }

        AL_UNLOCK(info->threadLock);

        // insure minimum frame count
        // minWriteFrameCnt default is zero and set it if needed
        if (pendingBuf.pBufBase != NULL &&
            (readBufferFrameCount < info->minWriteFrameCnt || RingBuf_getDataCount(&pendingBuf) != 0)) {

            RingBuf_copyFromLinear(&pendingBuf, (char *)mixBuffer, mixBufferSize);

            size_t dataSize = RingBuf_getDataCount(&pendingBuf);

            if (dataSize < info->minWriteFrameCnt * outSizePerFrame) {
                continue;
            } else {
                // calculate write size, should be the multiple of min frame count
                size_t remainDataSize = dataSize % (info->minWriteFrameCnt * outSizePerFrame);
                mixBufferSize = remainDataSize == 0 ? dataSize : (dataSize - remainDataSize);

                if (RingBuf_checkDataCrossBoundary(&pendingBuf, mixBufferSize) == 0) {
                    // direct use ring buf buffer
                    mixBuffer = (float *)pendingBuf.pRead;

                    RingBuf_discardData(&pendingBuf, mixBufferSize);
                } else {
                    // copy to new
                    if (tempWriteBufSize < mixBufferSize) {
                        if (tempWriteBuf) {
                            delete[] tempWriteBuf;
                        }

                        tempWriteBuf = new char[mixBufferSize];
                        tempWriteBufSize = mixBufferSize;
                    }
                    RingBuf_copyToLinear(tempWriteBuf, &pendingBuf, mixBufferSize);
                    mixBuffer = (float *)tempWriteBuf;
                }
            }
        }

//        ALOGD("%s(), pRead %p, pbase %p, diff %ld, mixBufferSize %u",
//              __FUNCTION__, pendingBuf.pRead, pendingBuf.pBufBase, (long)(pendingBuf.pRead - pendingBuf.pBufBase), mixBufferSize);

        // write
        void *pBufferAfterBitConvertion = NULL;
        uint32_t bytesAfterBitConvertion = 0;
        doBitConversion(info,
                        mixBuffer, mixBufferSize,
                        &pBufferAfterBitConvertion, &bytesAfterBitConvertion);

        char *writeBuffer = (char *)pBufferAfterBitConvertion;
        int writeBytes = bytesAfterBitConvertion;

        info->playHandler->write(writeBuffer, writeBytes);    // TODO: propagate return value back up?

        if (info->playHandler2) {
#ifdef MTK_SUPPORT_BTCVSD_ALSA
            bool skipWrite = false;
            if (!isBtMerge) {
                // check if timeout during write bt data
                struct mixer_ctl *ctl = mixer_get_ctl_by_name(mixer, "BTCVSD Tx Timeout Switch");
                int index = mixer_ctl_get_value(ctl, 0);
                skipWrite = index != 0;
            }

            if (!skipWrite)
#endif
            {
                info->playHandler2->write(writeBuffer, writeBytes);
            }
        }

        mixerOutDumpWriteData(pcmMixedDumpFile, writeBuffer, writeBytes);

        if (pendingBuf.pBufBase != NULL) {
            pendingBuf.pRead = pendingBuf.pBufBase;
            pendingBuf.pWrite = pendingBuf.pBufBase;
        }
    } while (clients->size() > 0);

    destroyPlaybackHandler(info->playHandler, streamManager);
    info->playHandler = NULL;
    destroyPlaybackHandler(info->playHandler2, streamManager);
    info->playHandler2 = NULL;

    AL_SIGNAL(info->waitSuspendLock);

    if (outDataBuffer) {
        delete[] outDataBuffer;
    }

    if (inDataBuffer) {
        delete[] inDataBuffer;
    }

    mixerOutDumpClose(pcmMixedDumpFile);

    if (pendingBuf.pBufBase != NULL) {
        delete [] pendingBuf.pBufBase;
    }

    ALOGD("-%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());
    return NULL;
}

int AudioMixerOut::destroyPlaybackHandler(AudioALSAPlaybackHandlerBase *playbackHandler,
                                          AudioALSAStreamManager *streamManager) {
    status_t status = 0;

    if (playbackHandler) {
        status = playbackHandler->close();
        if (status != NO_ERROR) {
            ALOGE("%s(), playbackHandler->close() fail!!", __FUNCTION__);
            ASSERT(0);
        }

        // destroy playback handler
        streamManager->destroyPlaybackHandler(playbackHandler);
        playbackHandler = NULL;
    }

    return status;
}

bool AudioMixerOut::clientAllSuspend(const MixerOutClientVector *clients) {
    for (size_t i = 0; i < clients->size(); i++) {
        if (!clients->valueAt(i)->suspend) {
            return false;
        }
    }
    return true;
}

int AudioMixerOut::waitSignal(AudioLock *mWaitLock, unsigned int waitTimeUs, const char *dbgString __unused) {
    int waitResult;

    // sleep until signalled
    CLEANUP_PUSH_ALOCK(mWaitLock->getAlock());
    AL_LOCK(mWaitLock);

    //ALOGD("%s(), %s, waitTimeUs %u", __FUNCTION__, dbgString, waitTimeUs);

    waitTimeUs = (waitTimeUs < 1000) ?  1000 : waitTimeUs;  // TODO: add AL_WAIT_US API?
    waitResult = AL_WAIT_MS(mWaitLock, waitTimeUs / 1000);

    AL_UNLOCK(mWaitLock);
    CLEANUP_POP_ALOCK(mWaitLock->getAlock());

    return waitResult;
}

// dump file
FILE *AudioMixerOut::mixerOutDumpOpen(const char *name, const char *property, const struct MixerOutInfo *info) {
    static unsigned int dumpFileCount = 0;

    ALOGV("%s()", __FUNCTION__);
    FILE *file = NULL;
    char dumpFileName[128];
    sprintf(dumpFileName, "%s.%s.%u.pid%d.tid%d.%d.%s.%d.pcm", MIX_OUT_DUMP_FILE_PREFIX, name, dumpFileCount,
            getpid(), gettid(),
            info->attribute.sample_rate,
            transferAudioFormatToDumpString(info->attribute.audio_format),
            info->attribute.num_channels);

    file = NULL;
    file = AudioOpendumpPCMFile(dumpFileName, property);

    if (file != NULL) {
        ALOGD("%s DumpFileName = %s", __FUNCTION__, dumpFileName);

        dumpFileCount++;
        dumpFileCount %= MAX_DUMP_NUM;
    }

    return file;
}

void AudioMixerOut::mixerOutDumpClose(FILE *file) {
    ALOGV("%s()", __FUNCTION__);
    if (file) {
        AudioCloseDumpPCMFile(file);
        ALOGD("%s(), close it", __FUNCTION__);
    }
}

void AudioMixerOut::mixerOutDumpWriteData(FILE *file, const void *buffer, size_t bytes) {
    if (file) {
        AudioDumpPCMData((void *)buffer, bytes, file);
    }
}

static const uint32_t kBliSrcOutputBufferSize = 0x10000;  // 64k
status_t AudioMixerOut::initBliSrc(struct MixerOutClient *client, const struct MixerOutInfo *outInfo) {
    unsigned int sourceRate, targetRate;
    unsigned int sourceChannels, targetChannels;
    audio_format_t format;

    sourceRate = client->attribute.sample_rate;
    sourceChannels = client->attribute.num_channels;
    format = client->attribute.audio_format;

    targetRate = outInfo->attribute.sample_rate;
    targetChannels = outInfo->attribute.num_channels;

    if (sourceRate != targetRate || sourceChannels != targetChannels) {
        ALOGD("%s(), flag %d, sample_rate: %d => %d, num_channels: %d => %d, mStreamAttributeSource->audio_format: 0x%x",
              __FUNCTION__, client->attribute.mAudioOutputFlags,
              sourceRate,  targetRate, sourceChannels, targetChannels, format);

        SRC_PCM_FORMAT src_pcm_format = SRC_IN_Q1P15_OUT_Q1P15;
        if (format == AUDIO_FORMAT_PCM_32_BIT) {
            src_pcm_format = SRC_IN_Q1P31_OUT_Q1P31;
        } else if (format == AUDIO_FORMAT_PCM_16_BIT) {
            src_pcm_format = SRC_IN_Q1P15_OUT_Q1P15;
        } else {
            ALOGE("%s(), not support mStreamAttributeSource->audio_format(0x%x) SRC!!", __FUNCTION__, format);
            ASSERT(0);
        }

        client->blisrc = newMtkAudioSrc(sourceRate, sourceChannels,
                                        targetRate,  targetChannels,
                                        src_pcm_format);
        ASSERT(client->blisrc != NULL);
        client->blisrc->open();

        client->blisrcOutBuffer = new char[kBliSrcOutputBufferSize];
        ASSERT(client->blisrcOutBuffer != NULL);
    }

    return NO_ERROR;
}

status_t AudioMixerOut::deinitBliSrc(struct MixerOutClient *client) {
    // deinit BLI SRC if need
    if (client->blisrc != NULL) {
        client->blisrc->close();
        delete client->blisrc;
        client->blisrc = NULL;
    }

    if (client->blisrcOutBuffer != NULL) {
        delete[] client->blisrcOutBuffer;
        client->blisrcOutBuffer = NULL;
    }

    return NO_ERROR;
}

status_t AudioMixerOut::doBliSrc(struct MixerOutClient *client, void *pInBuffer, uint32_t inBytes,
                                 void **ppOutBuffer, uint32_t *pOutBytes) {
    if (client->blisrc == NULL) { // No need SRC
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    } else {
        char *p_read = (char *)pInBuffer;
        uint32_t num_raw_data_left = inBytes;
        uint32_t num_converted_data = kBliSrcOutputBufferSize; // max convert num_free_space

        uint32_t consumed = num_raw_data_left;
        client->blisrc->process((int16_t *)p_read, &num_raw_data_left,
                                (int16_t *)client->blisrcOutBuffer, &num_converted_data);
        consumed -= num_raw_data_left;
        p_read += consumed;

        ALOGV("%s(), num_raw_data_left = %u, num_converted_data = %u",
              __FUNCTION__, num_raw_data_left, num_converted_data);

        if (num_raw_data_left > 0) {
            ALOGW("%s(), num_raw_data_left(%u) > 0", __FUNCTION__, num_raw_data_left);
            ASSERT(num_raw_data_left == 0);
        }

        *ppOutBuffer = client->blisrcOutBuffer;
        *pOutBytes = num_converted_data;
    }

    ASSERT(*ppOutBuffer != NULL && *pOutBytes != 0);
    return NO_ERROR;
}

unsigned int AudioMixerOut::getBitConvertDstBufferSize(audio_format_t dstFmt,
                                                       audio_format_t srcFmt,
                                                       unsigned int srcBufSizeByte) {
    size_t dstFmtByte = audio_bytes_per_sample(dstFmt);
    size_t srcFmtByte = audio_bytes_per_sample(srcFmt);

    if (dstFmtByte == 0) {
        ALOGE("%s(), invalid dstFmt %d, dstFmtByte = %zu", __FUNCTION__, dstFmt, dstFmtByte);
        ASSERT(0);
        dstFmtByte = audio_bytes_per_sample(AUDIO_FORMAT_PCM_16_BIT);
    }

    if (srcFmtByte == 0) {
        ALOGE("%s(), invalid srcFmt %d, srcFmtByte = %zu", __FUNCTION__, srcFmt, srcFmtByte);
        ASSERT(0);
        srcFmtByte = audio_bytes_per_sample(AUDIO_FORMAT_PCM_16_BIT);
    }

    return (srcBufSizeByte * dstFmtByte) / srcFmtByte;
}

static const uint32_t kMaxBitConvertBufferSize = 0x10000;  // 64k
status_t AudioMixerOut::initBitConverter(struct MixerOutClient *client, audio_format_t dstFmt) {
    return initBitConverter(client, client->attribute.audio_format, dstFmt);
}

status_t AudioMixerOut::initBitConverter(struct MixerOutInfo *info, audio_format_t srcFmt) {
    return initBitConverter(info, srcFmt, info->attribute.audio_format);
}

template<class T>
status_t AudioMixerOut::initBitConverter(T *client, audio_format_t srcFmt, audio_format_t dstFmt) {
    client->srcFmt = srcFmt;
    client->dstFmt = dstFmt;

    // init bit converter if need
    if (client->srcFmt != client->dstFmt) {
        ALOGD("%s(), id %p, format: 0x%x => 0x%x, size %zu => %zu",
              __FUNCTION__, client->id, client->srcFmt, client->dstFmt,
              audio_bytes_per_sample(client->srcFmt), audio_bytes_per_sample(client->dstFmt));
        client->bitConvertBuffer = new char[kMaxBitConvertBufferSize];
    }

    return NO_ERROR;
}

template<class T>
status_t AudioMixerOut::deinitBitConverter(T *client) {
    // deinit bit converter if need
    if (client->bitConvertBuffer != NULL) {
        delete[] client->bitConvertBuffer;
        client->bitConvertBuffer = NULL;
    }

    return NO_ERROR;
}

template<class T>
status_t AudioMixerOut::doBitConversion(T *client,
                                        void *pInBuffer, uint32_t inBytes,
                                        void **ppOutBuffer, uint32_t *pOutBytes) {
    if (client->bitConvertBuffer != NULL) {
        audio_format_t dstFmt = client->dstFmt;
        audio_format_t srcFmt = client->srcFmt;
        unsigned int srcSizeBytes = inBytes;

        size_t srcFmtByte = audio_bytes_per_sample(srcFmt);
        if (srcFmtByte == 0) {
            ALOGE("%s(), flag %d, invalid srcFmt %d, srcFmtByte = %zu",
                  __FUNCTION__, client->attribute.mAudioOutputFlags, srcFmt, srcFmtByte);
            ASSERT(0);
            srcFmtByte = audio_bytes_per_sample(AUDIO_FORMAT_PCM_16_BIT);
        }
        unsigned int srcNumSample = srcSizeBytes / srcFmtByte;
        void *srcBuffer = pInBuffer;
        unsigned int bitConvertBufferSizeByte = getBitConvertDstBufferSize(dstFmt, srcFmt, srcSizeBytes);

        ALOGV("%s(), bit convert, format: 0x%x => 0x%x, srcNumSample %d, src size %d, dst size %d",
              __FUNCTION__, srcFmt, dstFmt, srcNumSample, srcSizeBytes, bitConvertBufferSizeByte);

        memcpy_by_audio_format(client->bitConvertBuffer,
                               dstFmt,
                               srcBuffer,
                               srcFmt,
                               srcNumSample);

        *pOutBytes = bitConvertBufferSizeByte;
        *ppOutBuffer = client->bitConvertBuffer;
    } else {
        *ppOutBuffer = pInBuffer;
        *pOutBytes = inBytes;
    }

    ASSERT(*ppOutBuffer != NULL && *pOutBytes != 0);
    return NO_ERROR;
}

}
