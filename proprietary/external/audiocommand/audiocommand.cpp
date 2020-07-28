/* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * Copyright Statement:
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 */

/*
* Description:
*   This is used to debug audio hal and driver by set audio command.
*/

#define LOG_TAG "audiocommand"

typedef unsigned int UINT32;
typedef unsigned short  UINT16;

#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <binder/IPCThreadState.h>
#include <binder/MemoryBase.h>
#include <media/AudioSystem.h>
#include <media/mediaplayer.h>
#include <media/AudioRecord.h>
#include <media/stagefright/AudioSource.h>
#include <media/AudioSystem.h>
#include <system/audio_policy.h>
#include <hardware/audio_policy.h>
#include <hardware_legacy/AudioPolicyInterface.h>
#include <hardware_legacy/AudioSystemLegacy.h>
#include <utils/List.h>
#include <system/audio.h>
#include <utils/Log.h>
#include <media/AudioTrack.h>
#include <media/AudioRecord.h>
#include <tinyalsa/asoundlib.h>
#include "AudioToolkit.h"
#include <pthread.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

using namespace android;

#define MAX_BUFFER_SIZE (0x4000)
#define FAST_FRAME_CHUNK (4)
#define NORMAL_FRAME_CHUNK (12)


#define TONE_REPEAT_TIME (10)
typedef enum {
    AUDIO_DUMP_OUT_STREAMOUT = 1,
    AUDIO_DUMP_OUT_FLINGER,
    AUDIO_DUMP_IN_STREAMIN,
    AUDIO_DUMP_OUT_TRACK,
    AUDIO_DUMP_MAX,

} Audio_Dump_type_t;

#define STR_AUDIO_DUMP_OUT_STREAMOUT "AUDIO_DUMP_OUT_STREAMOUT"
#define STR_AUDIO_DUMP_OUT_FLINGER "AUDIO_DUMP_OUT_FLINGER"
#define STR_AUDIO_DUMP_IN_STREAMIN "AUDIO_DUMP_IN_STREAMIN"
#define STR_AUDIO_DUMP_OUT_TRACK "AUDIO_DUMP_OUT_TRACK"

#define AUDIO_TONE_SIZE (192)
#define AUDIOTRACK_REPEAT_COUNT (500)
#define AUDIO_PULSE_LEVEL (5000)

using namespace android;
unsigned int gDevice;
audio_policy_dev_state_t gState;
int gAudioMode;
static struct pcm *gPcm = NULL;
static struct pcm_config gConfig;
static struct pcm *gPcmIn;
static struct pcm_config gConfigIn;

struct audiotrackattribute {
    audio_stream_type_t streamtype;
    unsigned int streamrate;
    audio_output_flags_t streamflag;
};

static struct audiotrackattribute trackattribute;

#define calc_time_diff(x,y) ((x.tv_sec - y.tv_sec) * 1000 + (double)(x.tv_nsec - y.tv_nsec) * 1e-6)
static struct timespec gFastRecordEndTime;
static unsigned int gFastRecordCycles;
static bool gIsValidData;

static int getAudioMode() {
    printf("getAudioMode enter audio mode\n"
           "  0:normal\n"
           "  1:ringtone\n"
           "  2:incall\n"
           "  3:commuinication mode\n");
    (void)scanf("%x", &gAudioMode);
    return gAudioMode;
}

static audio_devices_t getAudioDevice() {
    printf("getAudioDevice enter audio device \n"
           "  1: earpiece\n"
           "  2: speaker\n"
           "  4: wired_headset\n"
           "  8: wired_headphone\n");
    (void)scanf("%x", &gDevice);
    return (audio_devices_t)gDevice;
}

static const unsigned char k1KToneIn48KHz[AUDIO_TONE_SIZE] = {
    0x00, 0x00, 0xFF, 0xFF, 0xB5, 0x10, 0xB5, 0x10, 0x21, 0x21, 0x21, 0x21,
    0xFC, 0x30, 0xFC, 0x30, 0x00, 0x40, 0x00, 0x40, 0xEB, 0x4D, 0xEB, 0x4D,
    0x81, 0x5A, 0x82, 0x5A, 0x8C, 0x65, 0x8C, 0x65, 0xD9, 0x6E, 0xD9, 0x6E,
    0x40, 0x76, 0x41, 0x76, 0xA3, 0x7B, 0xA2, 0x7B, 0xE7, 0x7E, 0xE6, 0x7E,
    0xFF, 0x7F, 0xFF, 0x7F, 0xE6, 0x7E, 0xE7, 0x7E, 0xA2, 0x7B, 0xA2, 0x7B,
    0x41, 0x76, 0x41, 0x76, 0xD9, 0x6E, 0xD8, 0x6E, 0x8D, 0x65, 0x8C, 0x65,
    0x82, 0x5A, 0x81, 0x5A, 0xEB, 0x4D, 0xEB, 0x4D, 0xFF, 0x3F, 0xFF, 0x3F,
    0xFB, 0x30, 0xFB, 0x30, 0x20, 0x21, 0x21, 0x21, 0xB5, 0x10, 0xB5, 0x10,
    0x00, 0x00, 0xFF, 0xFF, 0x4B, 0xEF, 0x4B, 0xEF, 0xE0, 0xDE, 0xE0, 0xDE,
    0x05, 0xCF, 0x05, 0xCF, 0x00, 0xC0, 0x00, 0xC0, 0x14, 0xB2, 0x14, 0xB2,
    0x7D, 0xA5, 0x7F, 0xA5, 0x74, 0x9A, 0x75, 0x9A, 0x27, 0x91, 0x26, 0x91,
    0xC0, 0x89, 0xBF, 0x89, 0x5E, 0x84, 0x5D, 0x84, 0x19, 0x81, 0x19, 0x81,
    0x02, 0x80, 0x02, 0x80, 0x19, 0x81, 0x19, 0x81, 0x5E, 0x84, 0x5E, 0x84,
    0xBF, 0x89, 0xBF, 0x89, 0x27, 0x91, 0x28, 0x91, 0x75, 0x9A, 0x74, 0x9A,
    0x7E, 0xA5, 0x7E, 0xA5, 0x15, 0xB2, 0x15, 0xB2, 0x00, 0xC0, 0x00, 0xC0,
    0x04, 0xCF, 0x04, 0xCF, 0xDF, 0xDE, 0xDF, 0xDE, 0x4B, 0xEF, 0x4B, 0xEF
};

String8 PrintEncodedString(String8 strKey, size_t len, void *ptr) {
    String8 returnValue = String8("");
    size_t sz_Needed;
    size_t sz_enc;
    char *buf_enc = NULL;
    bool bPrint = false;

    //ALOGD("%s in, len = %d", __FUNCTION__, len);
    sz_Needed = Base64_OutputSize(true, len);
    buf_enc = new char[sz_Needed + 1];
    if (buf_enc == NULL) {
        return returnValue;
    }
    buf_enc[sz_Needed] = 0;

    sz_enc = Base64_Encode((unsigned char *)ptr, buf_enc, len);

    if (sz_enc != sz_Needed) {
        ALOGE("%s(), Encode Error!!!after encode (%s), len(%d), sz_Needed(%d), sz_enc(%d)",
              __FUNCTION__, buf_enc, len, sz_Needed, sz_enc);
    } else {
        bPrint = true;
        //ALOGD("%s(), after encode (%s), len(%d), sz_enc(%d)", __FUNCTION__, buf_enc, len, sz_enc);
    }

    if (bPrint) {
        String8 StrVal = String8(buf_enc, sz_enc);
        returnValue += strKey;
        returnValue += StrVal;
        //returnValue += String8(";");
    }

    if (buf_enc != NULL) {
        delete[] buf_enc;
    }

    return returnValue;
}

status_t SetAudioCommand(int par1, int par2) {
    int iPara[2];
    iPara[0] = par1;
    iPara[1] = par2;

    String8 strPara = PrintEncodedString(String8("SetCmd="), sizeof(iPara), iPara);
    return AudioSystem::setParameters(0, strPara);
}

static void audioRecordCallbackFunction(int event, void *user __unused, void *info __unused) {
    switch (event) {
    case AudioRecord::EVENT_MORE_DATA: {
        ALOGD("audiorecordtest::EVENT_MORE_DATA");
        break;
    }
    case AudioRecord::EVENT_OVERRUN: {
        ALOGD("audiorecordtest::EVENT_OVERRUN");
        break;
    }
    default:
        // does nothing
        break;
    }
}

static void fastAudioRecordCbf(int event, void *user __unused, void *audioBuffer) {
    AudioRecord::Buffer *mAudioBuffer;
    mAudioBuffer = (AudioRecord::Buffer *)audioBuffer;

    if (event == AudioRecord::EVENT_MORE_DATA && !gIsValidData) {
        ++gFastRecordCycles;
        short *raw = (short *)mAudioBuffer->raw;
        for (short i = 0; i < (short)mAudioBuffer->frameCount; i++) {
            //ALOGD("FastRecord cycles[%d], index[%d], raw[%d]", gFastRecordCycles, i, raw[i]);
            if (abs(raw[i]) > AUDIO_PULSE_LEVEL) {
                clock_gettime(CLOCK_REALTIME, &gFastRecordEndTime);
                gIsValidData = true;
                break;
            }
        }
    }
}

static void *startPlayBackthread(void *arg) {
    ALOGD("+AudioPlaybackTest arg = %p\n", arg);
    ALOGD("+%s, flag = %d, rate = %d\n", __func__, trackattribute.streamflag, trackattribute.streamrate);
    size_t audiotrackframeCount;
    AudioTrack::getMinFrameCount(&audiotrackframeCount, trackattribute.streamtype, trackattribute.streamrate);
    ALOGD("+audiotrackframeCount  = %zu", __func__, audiotrackframeCount);

    int count = 20 * AUDIOTRACK_REPEAT_COUNT;
    uint32_t sampleRate = trackattribute.streamrate;
    audio_format_t format = AUDIO_FORMAT_PCM_16_BIT;
    audio_channel_mask_t channelMask = (audio_channel_mask_t)AUDIO_CHANNEL_OUT_STEREO;
    size_t frameCount = audiotrackframeCount * NORMAL_FRAME_CHUNK;
    audio_output_flags_t flags = trackattribute.streamflag;
    int32_t notificationFrames = 0;
    audio_session_t sessionId = AUDIO_SESSION_NONE;
    audio_usage_t usage = AUDIO_USAGE_MEDIA;
    audio_content_type_t contentType = AUDIO_CONTENT_TYPE_MUSIC;
    audio_attributes_t attributes;
    sp<IMemory> sharedBuffer;

    memset(&attributes, 0, sizeof(attributes));
    attributes.content_type = contentType;
    attributes.usage = usage;

    if ((flags & AUDIO_OUTPUT_FLAG_FAST) != 0) {
        frameCount = audiotrackframeCount * FAST_FRAME_CHUNK;
    }

    sp<AudioTrack> audioTrack;
    audioTrack = new AudioTrack();
    audioTrack->set(AUDIO_STREAM_MUSIC, sampleRate, format, channelMask,
                    frameCount, flags, NULL, NULL, notificationFrames,
                    sharedBuffer, true, sessionId, AudioTrack::TRANSFER_SYNC,
                    NULL, -1, -1, &attributes, false, 1.0f, AUDIO_PORT_HANDLE_NONE);

    if (audioTrack == NULL) {
        ALOGD("New AudioTrack fail");
        audioTrack.clear();
        return 0;
    }

    audioTrack->start();
    size_t bufferSize = AUDIO_TONE_SIZE * TONE_REPEAT_TIME;
    ALOGD("%s bufferSize  = %zu", __func__ , bufferSize);
    char *temp = (char *)malloc(bufferSize);
    if (temp == NULL) {
        ALOGD("Malloc bufferSize fail");
        return 0;
    }

    char *tempptr = temp;
    ALOGD("buffersize = %zu  ", bufferSize);
    for (int i = 0; i < TONE_REPEAT_TIME; i++) {
        memcpy((void *)tempptr, (void *)k1KToneIn48KHz, AUDIO_TONE_SIZE);
        tempptr += AUDIO_TONE_SIZE;
    }
    while (count--) {
        // write buffer
        audioTrack->write((void *)temp, bufferSize);
    }

    audioTrack->stop();
    audioTrack.clear();

    if (temp) {
        free(temp);
        temp = NULL;
    }

    return 0;
}

void createplaybackthread() {
    pthread_t playbacktrack;
    trackattribute.streamflag = AUDIO_OUTPUT_FLAG_NONE;
    trackattribute.streamtype = AUDIO_STREAM_MUSIC;
    trackattribute.streamrate = 32000;
    int ret = pthread_create(&playbacktrack, NULL, startPlayBackthread, (void *)&trackattribute);
    if (ret != 0) {
        printf("%s ret = %d\n", __func__, ret);
    }

    printf("%s done\n", __func__);
}

void createplaybackthreadfast() {
    pthread_t playbacktrack;
    trackattribute.streamflag = AUDIO_OUTPUT_FLAG_FAST;
    trackattribute.streamtype = AUDIO_STREAM_MUSIC;
    trackattribute.streamrate = 48000;
    int ret = pthread_create(&playbacktrack, NULL, startPlayBackthread, (void *)&trackattribute);
    if (ret != 0) {
        printf("%s ret = %d\n", __func__, ret);
    }

    printf("%s done\n", __func__);
}
void createplaybackthreaddeepbuffer() {
    pthread_t playbacktrack;
    trackattribute.streamflag = AUDIO_OUTPUT_FLAG_DEEP_BUFFER;
    trackattribute.streamtype = AUDIO_STREAM_MUSIC;
    trackattribute.streamrate = 48000;
    int ret = pthread_create(&playbacktrack, NULL, startPlayBackthread, (void *)&trackattribute);
    if (ret != 0) {
        printf("%s ret = %d\n", __func__, ret);
    }

    printf("%s done\n", __func__);
}

void startPlayBackRing() {
    pthread_t playbacktrack;
    trackattribute.streamflag = AUDIO_OUTPUT_FLAG_NONE;
    trackattribute.streamtype = AUDIO_STREAM_MUSIC;
    trackattribute.streamrate = 32000;
    int ret = pthread_create(&playbacktrack, NULL, startPlayBackthread, (void *)&trackattribute);
    if (ret != 0) {
        printf("%s ret = %d\n", __func__, ret);
    }

    printf("%s done\n", __func__);

}

static int startRecording() {
    AudioRecord *mRecord;
    status_t mInitCheck;
    uint32_t sampleRate = 16000;
    status_t err = NO_ERROR;
    int inputSource = AUDIO_SOURCE_MIC;
    int channels = AUDIO_CHANNEL_IN_MONO; // AUDIO_CHANNEL_IN_MONO or AUDIO_CHANNEL_IN_STEREO
    /*
        uint32_t flags = AudioRecord::RECORD_AGC_ENABLE |
                         AudioRecord::RECORD_NS_ENABLE  |
                         AudioRecord::RECORD_IIR_ENABLE;
    */
#if 1   // AudioRecord API is changed by M, if need it, please update it
    try{
    mRecord = new AudioRecord(
        (audio_source_t)inputSource, sampleRate, AUDIO_FORMAT_PCM_16_BIT, (audio_channel_mask_t)channels, String16(""),
        4 * MAX_BUFFER_SIZE / sizeof(int16_t), /* Enable ping-pong buffers */
        audioRecordCallbackFunction,
        NULL);
    }
    catch(...) {
        printf("AudioRecord exception \n");
    }


    if (mRecord == NULL) {
        ALOGD("New AudioRecord fail");
        return -1;
    }

    mInitCheck = mRecord->initCheck();
    try {
        err = mRecord->start();
    } catch (...) {
        printf("mRecord->start(); fail!! ret = %d\n", err);
    }

    usleep(10 * 1000 * 1000);
    ALOGD("audiorecordtest::sleep for 10 sec");

    mRecord->stop();
#endif
    return err;
}

void audioHardwareCommand() {
    int input = 0;
    while (true) {
        printf("please enter audio command 0:input parameters \n");
        (void)scanf("%x", &input);
#ifndef MTK_BASIC_PACKAGE
        try {
            SetAudioCommand(0x90, input);
        } catch (...) {
            printf("other exception \n");
        }
#endif
    }
}

void audioGetImpedance() {
    printf("audioGetImpedance \n");
    struct mixer *mMixer;
    mMixer = mixer_open(0);
    struct mixer_ctl *ctl;
    enum mixer_ctl_type type;
    unsigned int  mHeadPhoneImpedence = 0;
    unsigned int numValues, i ;
    if (mMixer != NULL) {
        ctl = mixer_get_ctl_by_name(mMixer, "Audio HP Impedance");
        type = mixer_ctl_get_type(ctl);
        numValues = mixer_ctl_get_num_values(ctl);
        printf("audioGetImpedance type = %d numValues = %d \n", type, numValues);
        for (i = 0; i < numValues; i++) {
            mHeadPhoneImpedence = mixer_ctl_get_value(ctl, i);
            printf("GetHeadPhoneImpedance i = %d mHeadPhoneImpedence = %d \n", i , mHeadPhoneImpedence);
        }
    }

    if (mMixer) {
        mixer_close(mMixer);
    }
}

void audioPcmOutOpen() {
    printf("audioPcmOutOpen \n");
    int pcmNum, sampleRate, channels, periodZize, priodCount;
    printf("please enter pcmNum \n");
    (void)scanf("%d", &pcmNum);
    printf("please enter sampleRate \n");
    (void)scanf("%d", &sampleRate);
    printf("please enter channels \n");
    (void)scanf("%d", &channels);
    printf("please enter periodsize \n");
    (void)scanf("%d", &periodZize);
    printf("please enter priodCount \n");
    (void)scanf("%d", &priodCount);
    gConfig.channels = channels;
    gConfig.rate = sampleRate;
    gConfig.period_size = periodZize;
    gConfig.period_count = priodCount;
    gConfig.format = PCM_FORMAT_S16_LE;
    gConfig.start_threshold = 0;
    gConfig.stop_threshold = 0;
    gConfig.silence_threshold = 0;
    gPcm = pcm_open(0, pcmNum, PCM_OUT, &gConfig);
    printf("pcm_open pcmnum = %d gPcm = %p\n", pcmNum, gPcm);
    pcm_start(gPcm);
}

void audioPcmOutClose() {
    if (gPcm != NULL) {
        pcm_stop(gPcm);
        pcm_close(gPcm);
        gPcm = NULL;
    }
    printf("audioPcmOutClose \n");
}

void audioPcmInOpen() {
    printf("audioPcmInOpen \n");
    int pcmNum, sampleRate, channels, periodSize, priodCount;
    printf("please enter pcmNum \n");
    (void)scanf("%d", &pcmNum);
    printf("please enter sampleRate \n");
    (void)scanf("%d", &sampleRate);
    printf("please enter channels \n");
    (void)scanf("%d", &channels);
    printf("please enter periodSize \n");
    (void)scanf("%d", &periodSize);
    printf("please enter priodCount \n");
    (void)scanf("%d", &priodCount);
    gConfigIn.channels = channels;
    gConfigIn.rate = sampleRate;
    gConfigIn.period_size = periodSize;
    gConfigIn.period_count = priodCount;
    gConfigIn.format = PCM_FORMAT_S16_LE;
    gConfigIn.start_threshold = 0;
    gConfigIn.stop_threshold = 0;
    gConfigIn.silence_threshold = 0;
    gPcmIn = pcm_open(0, pcmNum, PCM_IN, &gConfigIn);
    pcm_start(gPcmIn);
}

void audioPcmInClose() {
    pcm_stop(gPcmIn);
    pcm_close(gPcmIn);
    printf("audioPcmInClose \n");
}

void audioSystemCommand() {
    int secondCommand;
    int ret = 0;
    unsigned int strLen = 500;
    char inputParameter[strLen];
    while (true) {
        printf("please enter audio command\n"
               "  0:device connnect\n"
               "  1:device disconnect\n"
               "  2:mode set \n"
               "  3:setparameter \n"
               "  4:getparameter \n");
        (void)scanf("%x", &secondCommand);
        memset(inputParameter, '\0', strLen);
        String8 returnstring;;
        switch (secondCommand) {
        case 0:
            gState = AUDIO_POLICY_DEVICE_STATE_AVAILABLE;
            gDevice = getAudioDevice();
            try {
                ret = AudioSystem::setDeviceConnectionState((audio_devices_t)gDevice, gState, NULL, NULL, AUDIO_FORMAT_DEFAULT);
            } catch (...) {
                printf("setDeviceConnectionState fail!! ret = %d\n", ret);
            }
            break;
        case 1:
            gState = AUDIO_POLICY_DEVICE_STATE_UNAVAILABLE;
            gDevice = getAudioDevice();
            try {
                ret = AudioSystem::setDeviceConnectionState((audio_devices_t)gDevice, gState, NULL, NULL, AUDIO_FORMAT_DEFAULT);
            } catch (...) {
                printf("getAudioDevice fail!! ret = %d\n", ret);
            }
            break;
        case 2:
            gAudioMode = getAudioMode();
            try {
                AudioSystem::setPhoneState((audio_mode_t)gAudioMode);
            } catch (...) {
                printf("other exception \n");
            }
            break;
        case 3:
            (void)scanf("%256s", inputParameter);
            try {
                AudioSystem::setParameters(0, String8(inputParameter));
            } catch (...) {
                printf("other exception \n");
            }
            break;
        case 4:
            (void)scanf("%256s", inputParameter);
            try {
                returnstring = AudioSystem::getParameters(0, String8(inputParameter));
            } catch (...) {
                printf("other exception \n");
            }
            printf("AudioSystem::getParameters %s\n", returnstring.string());
            break;
        default:
            printf("no this command!!!!!!!!!!!!\n");
            break;
        }
    }
}

void dumpAudioDataFunc(bool bEnable) {
    // For replace remove setprop control
    int secondCommand;
    char keyInputFileName[128];
    bool isBreak = false;
    int dumpFlg = 0;
    while (true) {
        if (bEnable) {
            printf("Enter Enable Audio Dump Data\n");
            dumpFlg = 1;
        } else {
            printf("Enter Disable Audio Dump Data\n");
            dumpFlg = 0;
        }

        printf("please enter audio command\n"
               "  1:AUDIO_OUT_STREAMOUT\n"
               "  2:AUDIO_OUT_FLINGER\n"
               "  3:AUDIO_IN_STREAMIN \n"
               "  4:AUDIO_OUT_TRACK \n"
               "  other:Exit loop \n");

        (void)scanf("%x", &secondCommand);
        switch ((Audio_Dump_type_t)secondCommand) {
        case AUDIO_DUMP_OUT_STREAMOUT:
            sprintf(keyInputFileName, "AUDIO_DUMP_OUT_STREAMOUT=%d", dumpFlg);
            break;
        case AUDIO_DUMP_OUT_FLINGER:
            sprintf(keyInputFileName, "AUDIO_DUMP_OUT_FLINGER=%d", dumpFlg);
            break;
        case AUDIO_DUMP_IN_STREAMIN:
            sprintf(keyInputFileName, "AUDIO_DUMP_IN_STREAMIN=%d", dumpFlg);
            break;
        case AUDIO_DUMP_OUT_TRACK:
            sprintf(keyInputFileName, "AUDIO_DUMP_OUT_TRACK=%d", dumpFlg);
            break;
        default:
            isBreak = true;
            printf("Exit\n");
            break;
        }
        if (isBreak) {
            break;
        } else {
            printf("%s\n", keyInputFileName);
            try {
                AudioSystem::setParameters(0, String8(keyInputFileName));
            }     catch (...) {
                printf("other exception \n");
            }
        }
    }
}

void listDumpAudioDataState(void) {
    String8 returnString;
    try {
        returnString = AudioSystem::getParameters(0, String8("AUDIO_DUMP_OUT_STREAMOUT"));
    } catch (...) {
        printf("other exception \n");
    }
    printf("%s\n", returnString.string());
    try {
        returnString = AudioSystem::getParameters(0, String8("AUDIO_DUMP_OUT_FLINGER"));
    } catch (...) {
        printf("other exception \n");
    }
    printf("%s\n", returnString.string());
    try {
        returnString = AudioSystem::getParameters(0, String8("AUDIO_DUMP_IN_STREAMIN"));
    } catch (...) {
        printf("other exception \n");
    }
    printf("%s\n", returnString.string());
    try {
        returnString = AudioSystem::getParameters(0, String8("AUDIO_DUMP_OUT_TRACK"));
    } catch (...) {
        printf("other exception \n");
    }
    printf("%s\n\n\n", returnString.string());
}

void dumpAllsetForceUse(void) {
    audio_policy_forced_cfg_t cfg;
    printf("list audio_policy_forced_cfg_t\n"
           "  0: AUDIO_POLICY_FORCE_NONE\n"
           "  1: AUDIO_POLICY_FORCE_SPEAKER\n"
           "  2: AUDIO_POLICY_FORCE_HEADPHONES\n"
           "  3: AUDIO_POLICY_FORCE_BT_SCO\n"
           "  4: AUDIO_POLICY_FORCE_BT_A2DP\n"
           "  5: AUDIO_POLICY_FORCE_WIRED_ACCESSORY\n"
           "  6: AUDIO_POLICY_FORCE_BT_CAR_DOCK\n"
           "  7: AUDIO_POLICY_FORCE_BT_DESK_DOCK\n"
           "  8: AUDIO_POLICY_FORCE_ANALOG_DOCK\n"
           "  9: AUDIO_POLICY_FORCE_DIGITAL_DOCK\n"
           "  10: AUDIO_POLICY_FORCE_NO_BT_A2DP\n"
           "  11: AUDIO_POLICY_FORCE_SYSTEM_ENFORCED\n"
           "  12: AUDIO_POLICY_FORCE_HDMI_SYSTEM_AUDIO_ENFORCED\n"
           "  13: AUDIO_POLICY_FORCE_ENCODED_SURROUND_NEVER\n"
           "  14: AUDIO_POLICY_FORCE_ENCODED_SURROUND_ALWAYS\n"
           "  15: AUDIO_POLICY_FORCE_REMOTE_SUBMIX_IN_WITH_FLAGS (MTK Add)\n"
           "  16: AUDIO_POLICY_FORCE_REMOTE_SUBMIX_IN (MTK Add)\n\n\n"
          );
    try {
        cfg = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_COMMUNICATION);
    } catch (...) {
        printf("other exception \n");
    }
    printf("=> %d AUDIO_POLICY_FORCE_FOR_COMMUNICATION %d \n", AUDIO_POLICY_FORCE_FOR_COMMUNICATION, cfg);
    try {
        cfg = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA);
    } catch (...) {
        printf("other exception \n");
    }
    printf("=> %d AUDIO_POLICY_FORCE_FOR_MEDIA %d \n", AUDIO_POLICY_FORCE_FOR_MEDIA, cfg);
    try {
        cfg = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_RECORD);
    } catch (...) {
        printf("other exception \n");
    }
    printf("=> %d AUDIO_POLICY_FORCE_FOR_RECORD %d \n", AUDIO_POLICY_FORCE_FOR_RECORD, cfg);
    try {
        cfg = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_DOCK);
    } catch (...) {
        printf("other exception \n");
    }
    printf("=> %d AUDIO_POLICY_FORCE_FOR_DOCK %d \n", AUDIO_POLICY_FORCE_FOR_DOCK, cfg);
    try {
        cfg = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_SYSTEM);
    } catch (...) {
        printf("other exception \n");
    }
    printf("=> %d AUDIO_POLICY_FORCE_FOR_SYSTEM %d \n", AUDIO_POLICY_FORCE_FOR_SYSTEM, cfg);
    try {
        cfg = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_HDMI_SYSTEM_AUDIO);
    } catch (...) {
        printf("other exception \n");
    }
    printf("=> %d AUDIO_POLICY_FORCE_FOR_HDMI_SYSTEM_AUDIO %d \n", AUDIO_POLICY_FORCE_FOR_HDMI_SYSTEM_AUDIO, cfg);
    try {
        cfg = AudioSystem::getForceUse(AUDIO_POLICY_FORCE_FOR_ENCODED_SURROUND);
    } catch (...) {
        printf("other exception \n");
    }
    printf("=> %d AUDIO_POLICY_FORCE_FOR_ENCODED_SURROUND %d \n\n\n\n", AUDIO_POLICY_FORCE_FOR_ENCODED_SURROUND, cfg);
}

void setOneForceUseCfg(void) {
    int secondCommand;
    int thirdCommand;
    audio_policy_force_use_t policy_force_use;
    audio_policy_forced_cfg_t policy_forced_cfg;
    while (true) {
        printf("please enter audio_policy_force_use_t\n"
               "  0:AUDIO_POLICY_FORCE_FOR_COMMUNICATION\n"
               "  1:AUDIO_POLICY_FORCE_FOR_MEDIA\n"
               "  2:AUDIO_POLICY_FORCE_FOR_RECORD \n"
               "  3:AUDIO_POLICY_FORCE_FOR_DOCK \n"
               "  4:AUDIO_POLICY_FORCE_FOR_SYSTEM \n"
               "  5:AUDIO_POLICY_FORCE_FOR_HDMI_SYSTEM_AUDIO \n"
               "  6:AUDIO_POLICY_FORCE_FOR_ENCODED_SURROUND \n"
               "  Other:Exit loop \n");

        (void)scanf("%x", &secondCommand);
        policy_force_use = (audio_policy_force_use_t) secondCommand;
        if (policy_force_use >= AUDIO_POLICY_FORCE_USE_CNT) {
            printf("Exit\n");
            break;
        }
        printf("please enter audio_policy_forced_cfg_t\n"
               "  0: AUDIO_POLICY_FORCE_NONE\n"
               "  1: AUDIO_POLICY_FORCE_SPEAKER\n"
               "  2: AUDIO_POLICY_FORCE_HEADPHONES\n"
               "  3: AUDIO_POLICY_FORCE_BT_SCO\n"
               "  4: AUDIO_POLICY_FORCE_BT_A2DP\n"
               "  5: AUDIO_POLICY_FORCE_WIRED_ACCESSORY\n"
               "  6: AUDIO_POLICY_FORCE_BT_CAR_DOCK\n"
               "  7: AUDIO_POLICY_FORCE_BT_DESK_DOCK\n"
               "  8: AUDIO_POLICY_FORCE_ANALOG_DOCK\n"
               "  9: AUDIO_POLICY_FORCE_DIGITAL_DOCK\n"
               "  10: AUDIO_POLICY_FORCE_NO_BT_A2DP\n"
               "  11: AUDIO_POLICY_FORCE_SYSTEM_ENFORCED\n"
               "  12: AUDIO_POLICY_FORCE_HDMI_SYSTEM_AUDIO_ENFORCED\n"
               "  13: AUDIO_POLICY_FORCE_ENCODED_SURROUND_NEVER\n"
               "  14: AUDIO_POLICY_FORCE_ENCODED_SURROUND_ALWAYS\n"
               "  15: AUDIO_POLICY_FORCE_REMOTE_SUBMIX_IN_WITH_FLAGS (MTK Add)\n"
               "  16: AUDIO_POLICY_FORCE_REMOTE_SUBMIX_IN (MTK Add)\n"
               "  Other: Exit loop\n\n"
              );
        (void)scanf("%x", &thirdCommand);
        policy_forced_cfg = (audio_policy_forced_cfg_t) thirdCommand;
        if (policy_forced_cfg >= AUDIO_POLICY_FORCE_CFG_CNT) {
            printf("Exit\n");
            break;
        }
        try {
            AudioSystem::setForceUse(policy_force_use, policy_forced_cfg);
        } catch (...) {
            printf("other exception \n");
        }
        printf("===================================================================================\n\n");
    }
}

static int sethifidac() {
    char buf[5];
    int retval = 0;
    printf("please set hifi_dac state (on/off) =\n");
    (void)scanf("%4s", buf);
    printf("hifi_dac state => [%s]\n", buf);
    String8 audiocommand;
    audiocommand = String8("hifi_dac=");
    audiocommand.append(buf);
    printf("command = %s\n", audiocommand.string());
    try {
        retval = AudioSystem::setParameters(0, audiocommand);
    } catch (...) {
        printf("other exception \n");
    }
    return retval;
}


static void gethifidac() {
    String8 result;
    try {
        result = AudioSystem::getParameters(0, String8("hifi_dac="));
    } catch (...) {
        printf("other exception \n");
    }
    printf("result = %s\n", result.string());
}

static int fastColdInputLatency() {
    AudioRecord *mRecord;
    status_t err = NO_ERROR;
    struct timespec mStartTime;

    memset((void *)&mStartTime, 0, sizeof(mStartTime));
    memset((void *)&gFastRecordEndTime, 0, sizeof(gFastRecordEndTime));
    gFastRecordCycles = 0;
    gIsValidData = false;

    mRecord = new AudioRecord(AUDIO_SOURCE_VOICE_RECOGNITION, // inputSource
                           48000,                             // sampleRate
                           AUDIO_FORMAT_PCM_16_BIT,           // format
                           AUDIO_CHANNEL_IN_MONO,             // channelMask
                           String16(""),                      // opPackageName
                           0,                                 // frameCount
                           fastAudioRecordCbf,                // cbf
                           NULL,                              // user
                           0,                                 // notificationFrames
                           AUDIO_SESSION_ALLOCATE,            // sessionId
                           AudioRecord::TRANSFER_CALLBACK,    // transferType
                           AUDIO_INPUT_FLAG_FAST);            // flags

    if (mRecord == 0 || mRecord->initCheck() != NO_ERROR) {
        ALOGD("New AudioRecord fail!");
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &mStartTime);
    try {
        err = mRecord->start();
    } catch (...) {
        printf("mRecord->start(); fail!! ret = %d\n", err);
    }
    usleep(300000);
    mRecord->stop();

    if (gFastRecordEndTime.tv_sec == 0 && gFastRecordEndTime.tv_nsec == 0) {
        printf("\nDetect no pulse, plz try again.\n\n");
    } else {
        double latencyTime = calc_time_diff(gFastRecordEndTime, mStartTime);
        printf("\nFast cold input latency: [%1.3f] ms, measureCycles=%d\n\n", latencyTime, gFastRecordCycles);
    }
    return err;
}

static int getCommands() {
    int mainCommand;
    int ret = 0;
    while (true) {
        printf("please enter audio command\n"
               "  0: audioplayback(AUDIO_STREAM_MUSIC)\n"
               "  1: audioplayback(AUDIO_STREAM_RING)\n"
               "  2: audiorecord\n"
               "  3: audiosystem command\n"
               "  4: audiohardwarecoomand\n"
               "  7: alsa test\n"
               "  8: pcmout open\n"
               "  9: pcmout close\n"
               "  10: pcmin open\n"
               "  11: pcmin close\n"
               "  12: enable audio data dump\n"
               "  13: disable audio data dump\n"
               "  14: list audio data dump\n"
               "  15: setforce media to loudspeaker\n"
               "  16: setforce media to default\n"
               "  17: dumpAllsetForceUse\n"
               "  18: setOneforceUseCfg\n"
               "  19: createplaybackthread\n"
               "  20: createplaybackthreadfast\n"
               "  21: createplaybackthreaddeepbuffer\n"
               "  22: sethifi_dac_state\n"
               "  23: gethifi_dac_state\n"
               "  24: fastColdInputLatency\n"
              );

        (void)scanf("%d", &mainCommand);
        printf("=> [%d]\n", mainCommand);
        switch (mainCommand) {
        case 0:
            createplaybackthread();
            break;
        case 1:
            startPlayBackRing();
            break;
        case 2:
            startRecording();
            break;
        case 3:
            audioSystemCommand();
            break;
        case 4:
            audioHardwareCommand();
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            audioGetImpedance();
            break;
        case 8:
            audioPcmOutOpen();
            break;
        case 9:
            audioPcmOutClose();
            break;
        case 10:
            audioPcmInOpen();
            break;
        case 11:
            audioPcmInClose();
            break;
        case 12:
            dumpAudioDataFunc(true);
            break;
        case 13:
            dumpAudioDataFunc(false);
            break;
        case 14:
            listDumpAudioDataState();
            break;
        case 15:
            try {
                ret = AudioSystem::setForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA, AUDIO_POLICY_FORCE_SPEAKER);
            } catch (...) {
                printf("other exception \n");
            }
            break;
        case 16:
            try {
                ret = AudioSystem::setForceUse(AUDIO_POLICY_FORCE_FOR_MEDIA, AUDIO_POLICY_FORCE_NONE);
            } catch (...) {
                printf("other exception \n");
            }
            break;
        case 17:
            dumpAllsetForceUse();
            break;
        case 18:
            setOneForceUseCfg();
            break;
        case 19:
            createplaybackthread();
            break;
        case 20:
            createplaybackthreadfast();
            break;
        case 21:
            createplaybackthreaddeepbuffer();
            break;
        case 22:
            ret = sethifidac();
            break;
        case 23:
            gethifidac();
            break;
        case 24:
            fastColdInputLatency();
            break;
        default:
            break;
        }
    }

    return ret;
}


static int runSetParam(const char *cmd) {
    status_t ret = NO_ERROR;
    int retval = 0;

    try {
        ret = AudioSystem::setParameters(0, String8(cmd));
    } catch (...) {
        fprintf(stderr, "cmd \"%s\" setParameters fail!! ret = %d\n", cmd, ret);
        return -1;
    }

    return retval;
}


static int runGetParam(const char *cmd) {
    const char *get_string = NULL;
    int retval = 0;

    try {
        get_string = AudioSystem::getParameters(0, String8(cmd)).string();
    } catch (...) {
        fprintf(stderr, "cmd \"%s\" getParameters fail!!\n", cmd);
        return -1;
    }

    fprintf(stderr, "%s\n", get_string);

    return retval;
}


static int dispatchCommand(char *cmd) {
    int retval = 0;

    // remove '\n'
    char *ptr_newline = strchr(cmd, '\n');
    if (ptr_newline != NULL) {
        *ptr_newline = '\0';
    }

    if (strchr(cmd, '=') != NULL) { // has '=', it's set function
        retval = runSetParam(cmd);
    } else {
        retval = runGetParam(cmd);
    }

    return retval;
}

static int waitCommand() {
    printf("Enter get/set Parameter string you want or 'exit' to leave\n");
    char str[512];
    int retval = 0;

    while (1) {
        (void)scanf("%s", str);

        if (strcmp(str, "exit") == 0) {
            break;
        }

        if (strchr(str, '=') != NULL) { // has '=', it's set function
            retval = runSetParam(str);
        } else {
            retval = runGetParam(str);
        }
    }

    return retval;
}

int main(int argc, char *argv[]) {
    ProcessState::self()->startThreadPool();
    sp<ProcessState> proc(ProcessState::self());
    ALOGD("start audiocommand\n");
    int retval = 0;

    if (argc == 1) { // interaction mode
        retval = getCommands();
    } else if (argc == 2) { // single mode
        if ((strcmp(argv[1], "-w") == 0)) {
            retval = waitCommand();
        } else {
            retval = dispatchCommand(argv[1]);
        }
    } else {
        printf("error: no this option\n");
    }

    ALOGD("end audiocommand\n");
    return retval;
}
