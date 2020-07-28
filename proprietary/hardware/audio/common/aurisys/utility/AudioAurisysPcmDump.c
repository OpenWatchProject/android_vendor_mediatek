#include "AudioAurisysPcmDump.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <string.h>

#include <sys/stat.h> /* for mkdir */
#include <sys/prctl.h> /* operations on a process */

#include <audio_log.h>
#include <audio_memory_control.h>
#include <wrapped_audio.h>

#ifdef MTK_AUDIODSP_SUPPORT
#include <audio_lock.h>
#include <audio_messenger_ipi.h>
#include <audio_task.h>
#include <tinyalsa/asoundlib.h>
#endif


#define MIN_PCM_DUMP_SIZE (MIN_PCM_DUMP_CHUNK*4)
#define MIN_PCM_DUMP_CHUNK (8192)
#define PCM_TIME_OUT_SEC (1)

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "AudioAurisysPcmDump"
#define UNUSED(x) ((void)(x))

const char *audio_format_to_dump_string(const uint32_t format) {
    switch (format) {
    case AUDIO_FORMAT_PCM_8_BIT:
        return "8bit";
    case AUDIO_FORMAT_PCM_16_BIT:
        return "16bit";
    case AUDIO_FORMAT_PCM_24_BIT_PACKED:
        return "24bit";
    case AUDIO_FORMAT_PCM_8_24_BIT:
        return "8_24bit";
    case AUDIO_FORMAT_PCM_32_BIT:
        return "32bit";
    case AUDIO_FORMAT_PCM_FLOAT:
        return "float";
    default:
        ALOGE("%s: invalid audio format %#x", __FUNCTION__, format);
        return "unknown";
    }
}

static int AudiocheckAndCreateDirectory(const char *pC) {
    char tmp[128];
    int i = 0;
    while (*pC) {
        tmp[i] = *pC;
        if (*pC == '/' && i) {
            tmp[i] = '\0';
            if (access(tmp, F_OK) != 0) {
                if (mkdir(tmp, 0770) == -1) {
                    AUD_LOG_E("AudioDumpPCM: mkdir error!");
                    return -1;
                }
            }
            tmp[i] = '/';
        }
        i++;
        pC++;
    }
    return 0;
}

static void *PCMDumpThread(void *arg) {
    pthread_detach(pthread_self());
    prctl(PR_SET_NAME, (unsigned long)__FUNCTION__, 0, 0, 0);
    struct PcmDump_t *self = (struct PcmDump_t *)arg;

    //AUD_LOG_D("PCMDumpThread start self->mPthreadEnable = %d", self->mPthreadEnable);

    unsigned int CopySize = 0;
    char *pLinearBuf = (char *)AUDIO_MALLOC(self->mLineaBufSize);
    while (self->mPthreadEnable == true) {
        pthread_mutex_lock(&self->mPCMDataNotifyMutex);
        CopySize = audio_ringbuf_count(&self->mRingbuffer);
        //AUD_LOG_D("%s fwrite CopySize = %d mLineaBufSize = %d", __FUNCTION__, CopySize, self->mLineaBufSize);

        while (CopySize >= self->mLineaBufSize) {
            audio_ringbuf_copy_to_linear(pLinearBuf, &self->mRingbuffer,  self->mLineaBufSize);
            if (self->mFilep) {
                fwrite((void *)pLinearBuf, 1, self->mLineaBufSize, self->mFilep);
                //AUD_LOG_D("%s fwrite CopySize = %d self->mLineaBufSize = %d", __FUNCTION__, CopySize,self->mLineaBufSize);
            }
            CopySize -= self->mLineaBufSize;
        }

        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + 3;
        timeout.tv_nsec = now.tv_usec * 1000;
        pthread_cond_timedwait(&self->mPCMDataNotifyEvent, &self->mPCMDataNotifyMutex, &timeout);
        pthread_mutex_unlock(&self->mPCMDataNotifyMutex);
    }

    //AUD_LOG_D("PCMDumpThread exit hPCMDumpThread=%p", &self->hPCMDumpThread);

    pthread_mutex_lock(&self->mPCMDumpMutex);
    CopySize = audio_ringbuf_count(&self->mRingbuffer);
    if (CopySize >= self->mLineaBufSize) {
        CopySize = self->mLineaBufSize;
    }
    audio_ringbuf_copy_to_linear(pLinearBuf, &self->mRingbuffer, CopySize);
    fwrite((void *)pLinearBuf, 1, CopySize, self->mFilep);
    pthread_mutex_unlock(&self->mPCMDumpMutex);

    if (pLinearBuf) {
        AUDIO_FREE(pLinearBuf);
        pLinearBuf = NULL;
    }

    //AUD_LOG_D("PCMDumpThread  mPCMDataNotifyEvent ");
    pthread_cond_signal(&self->mPCMDataNotifyEvent);  // notify done
    pthread_exit(NULL);
    return 0;
}


static int  AudioOpendumpPCMFile(struct PcmDump_t *self, const char *filepath) {
#ifdef MTK_AUDIO_HAL_DUMP_DISABLE
    UNUSED(self);
    UNUSED(filepath);
    return 0;
#else
    int ret;
    AUD_LOG_D("%s filepath = %s", __FUNCTION__, filepath);
    ret = AudiocheckAndCreateDirectory(filepath);
    if (ret < 0) {
        AUD_LOG_E("AudioOpendumpPCMFile dumpPCMData checkAndCreateDirectory() fail!!!");
    } else {
        self->mFilep = fopen(filepath, "wb");
        if (self->mFilep  != NULL) {
            pthread_mutex_lock(&self->mPCMDumpMutex);
            if (self->mPthreadEnable == false) {
                //create PCM data dump thread here
                int ret = 0;
                self->mPthreadEnable = true;
                ret = pthread_create(&self->hPCMDumpThread, NULL, PCMDumpThread, self);// with self arg
                if (ret != 0) {
                    AUD_LOG_E("hPCMDumpThread create fail!!!");
                } else {
                    AUD_LOG_D("hPCMDumpThread=%p created", &self->hPCMDumpThread);
                }
            }
            pthread_mutex_unlock(&self->mPCMDumpMutex);
        } else {
            AUD_LOG_D("%s create thread fail", __FUNCTION__);
        }
    }
    return ret;
#endif
}

static int AudioCloseDumpPCMFile(struct PcmDump_t *self) {
#ifdef MTK_AUDIO_HAL_DUMP_DISABLE
    UNUSED(self);
#else
    AUD_LOG_D("%s", __FUNCTION__);
    int ret = 0;
    /* close thread */
    if (self->mPthreadEnable == true) {
        AUD_LOG_D("+%s pthread_mutex_lock", __FUNCTION__);
        pthread_mutex_lock(&self->mPCMDumpMutex);
        AUD_LOG_D("-%s pthread_mutex_lock", __FUNCTION__);
        self->mPthreadEnable = false;

        struct timeval now;
        struct timespec timeout;
        gettimeofday(&now, NULL);
        timeout.tv_sec = now.tv_sec + 1;
        timeout.tv_nsec = now.tv_usec * 1000;
        AUD_LOG_D("+%s pthread_cond_timedwait", __FUNCTION__);
        pthread_cond_signal(&self->mPCMDataNotifyEvent);  // notify done
        pthread_cond_timedwait(&self->mPCMDataNotifyEvent, &self->mPCMDumpMutex, &timeout);
        AUD_LOG_D("-%s pthread_cond_timedwait", __FUNCTION__);
        pthread_mutex_unlock(&self->mPCMDumpMutex);
    }

    /*  destroy */
    ret = pthread_mutex_destroy(&self->mPCMDataNotifyMutex);
    ret = pthread_mutex_destroy(&self->mPCMDumpMutex);
    ret = pthread_cond_destroy(&self->mPCMDataNotifyEvent);

    if (self->mFilep) {
        fclose(self->mFilep);
    }

    /* free rinbuffer*/
    if (self->mRingbuffer.base) {
        AUDIO_FREE(self->mRingbuffer.base);
        self->mRingbuffer.base = NULL;
    }

    self->mRingbuffer.size = 0;
    self->mRingbuffer.read = NULL;
    self->mRingbuffer.write = NULL;
    AUD_LOG_D("%s", __FUNCTION__);
#endif
    return 0;
}

static void AudioDumpPCMData(struct PcmDump_t *self, void *buffer, uint32_t bytes) {
#ifdef MTK_AUDIO_HAL_DUMP_DISABLE
    UNUSED(self);
    UNUSED(buffer);
    UNUSED(bytes);
#else
    unsigned int CopySize = 0;
    if (self->mPthreadEnable) {
        pthread_mutex_lock(&self->mPCMDataNotifyMutex);
        CopySize  = audio_ringbuf_free_space(&self->mRingbuffer);

        if (CopySize < bytes) {
            AUD_LOG_D("warning ... AudioDumpPCMData CopySize = %d bytes = %d", CopySize, bytes);
            bytes = CopySize;
        }
        audio_ringbuf_copy_from_linear_impl(&self->mRingbuffer, (const char *)buffer, bytes);
        pthread_cond_broadcast(&self->mPCMDataNotifyEvent);
        pthread_mutex_unlock(&self->mPCMDataNotifyMutex);
    }
#endif
}

void InitPcmDump_t(struct PcmDump_t *self, unsigned int size) {
    AUD_LOG_D("%s size = %d", __FUNCTION__, size);
    int ret = 0;
    self->AudioOpendumpPCMFile  = AudioOpendumpPCMFile;
    self->AudioCloseDumpPCMFile = AudioCloseDumpPCMFile;
    self->AudioDumpPCMData = AudioDumpPCMData;
    self->mFilep = NULL;
    self->hPCMDumpThread = 0;
    self->mPthreadEnable = false;
    ret = pthread_mutex_init(&self->mPCMDataNotifyMutex, NULL);
    ret = pthread_mutex_init(&self->mPCMDumpMutex, NULL);
    ret = pthread_cond_init(&self->mPCMDataNotifyEvent, NULL);

    /* init ring buffer*/
    if (size < 32768) {
        size = 32768;
    }
    self->mRingbuffer.base = (char *)AUDIO_MALLOC(size);
    memset((void *) self->mRingbuffer.base, 0, size);
    self->mRingbuffer.size = size;
    self->mRingbuffer.read = self->mRingbuffer.base;
    self->mRingbuffer.write = self->mRingbuffer.base;
    self->mLineaBufSize = MIN_PCM_DUMP_CHUNK;
    self->isDspDump = false;
}


#ifdef MTK_AUDIODSP_SUPPORT
#define SOUND_CARD_IDX 0
static PcmDump_t *aurisys_dump_array[TASK_SCENE_SIZE];
static struct alock_t *lock[TASK_SCENE_SIZE];

static int set_dsp_dump_wake_lock(bool condition) {
    struct mixer *p_mixer = NULL;
    p_mixer = mixer_open(SOUND_CARD_IDX);

    if (!p_mixer) {
        AUD_LOG_W("%s(), mixer open fail", __FUNCTION__);
        return -1;
    }

    if (mixer_ctl_set_value(mixer_get_ctl_by_name(p_mixer, "audio_dsp_wakelock"), 0, condition)) {
        AUD_LOG_W("%s(), set %d fail", __FUNCTION__, condition);
        mixer_close(p_mixer);
        return -1;
    }

    mixer_close(p_mixer);
    return 0;
}

void init_aurisys_dsp_dump() {
    for (uint32_t i = 0; i < TASK_SCENE_SIZE; i++) {
        if (lock[i] == NULL) {
            NEW_ALOCK(lock[i]);
        }
    }
}

void deinit_aurisys_dsp_dump() {
    for (uint32_t i = 0; i < TASK_SCENE_SIZE; i++) {
        if (lock[i] != NULL) {
            FREE_ALOCK(lock[i]);
            lock[i] = NULL;
        }
    }
}

void process_aurisys_dsp_dump(struct ipi_msg_t *msg, void *buf, uint32_t size) {
    PcmDump_t *aurisys_dump;

    if ((msg == NULL) || (buf == NULL) || (size == 0)) {
        return;
    }

    uint32_t task_scene = msg->param2;

    if (task_scene >= TASK_SCENE_SIZE) {
        ALOGE("%s, msg->param2 = %d >= TASK_SCENE_SIZE", __FUNCTION__, task_scene);
        return;
    }

    LOCK_ALOCK(lock[task_scene]);
    if (aurisys_dump_array[task_scene] != NULL) {
        aurisys_dump = aurisys_dump_array[task_scene];
        aurisys_dump->AudioDumpPCMData(aurisys_dump, buf, size);
    }
    UNLOCK_ALOCK(lock[task_scene]);
}

void add_aurisys_dsp_dump(PcmDump_t *aurisys_dump, uint32_t task_scene) {
    if (aurisys_dump == NULL) {
        return;
    }

    if (task_scene >= TASK_SCENE_SIZE) {
        ALOGE("%s, task_scene = %d >= TASK_SCENE_SIZE", __FUNCTION__, task_scene);
        return;
    }
    set_dsp_dump_wake_lock(true);
    LOCK_ALOCK(lock[task_scene]);
    aurisys_dump_array[task_scene] = aurisys_dump;
    UNLOCK_ALOCK(lock[task_scene]);
}

void delete_aurisys_dsp_dump(PcmDump_t *aurisys_dump __unused, uint32_t task_scene) {
    if (task_scene >= TASK_SCENE_SIZE) {
        ALOGE("%s, task_scene = %d >= TASK_SCENE_SIZE", __FUNCTION__, task_scene);
        return;
    }

    LOCK_ALOCK(lock[task_scene]);
    aurisys_dump_array[task_scene] = NULL;
    UNLOCK_ALOCK(lock[task_scene]);
    set_dsp_dump_wake_lock(false);
}
#endif


