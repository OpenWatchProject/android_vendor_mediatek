/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


/* This HAL simulates triggers from the DSP.
 * To send a trigger from the command line you can type:
 *
 * adb forward tcp:14035 tcp:14035
 *
 * telnet localhost 14035
 *
 * Commands include:
 * ls : Lists all models that have been loaded.
 * trig <uuid> : Sends a recognition event for the model at the given uuid
 * update <uuid> : Sends a model update event for the model at the given uuid.
 * close : Closes the network connection.
 *
 * To enable this file, you can make with command line parameter
 * SOUND_TRIGGER_USE_STUB_MODULE=1
 */

#define LOG_TAG "sound_trigger_hw_default"
#define LOG_NDEBUG 1
#define PARSE_BUF_LEN 1024  // Length of the parsing buffer.S

#define EVENT_RECOGNITION 1
#define EVENT_SOUND_MODEL 2

// The following commands work with the network port:
#define COMMAND_LS "ls"
#define COMMAND_RECOGNITION_TRIGGER "trig"  // Argument: model index.
#define COMMAND_RECOGNITION_ABORT "abort"  // Argument: model index.
#define COMMAND_RECOGNITION_FAILURE "fail"  // Argument: model index.
#define COMMAND_UPDATE "update"  // Argument: model index.
#define COMMAND_CLEAR "clear" // Removes all models from the list.
#define COMMAND_CLOSE "close" // Close just closes the network port, keeps thread running.
#define COMMAND_END "end" // Closes connection and stops the thread.

#define ERROR_BAD_COMMAND "Bad command"

#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <log/log.h>

#include <hardware/hardware.h>
#include <system/sound_trigger.h>
#include <hardware/sound_trigger.h>

// MTK Add
#include <AudioType.h>
#include <audio_lock.h>
#include <dlfcn.h>
#include <cutils/properties.h>
#include <soundtriggerAssert.h>
#include <sys/ioctl.h>
#include <sys/resource.h>
#include <utils/threads.h>
#include <audio_ringbuffer.h>

#include "hardware_legacy/AudioMTKHardwareInterface.h"
#include "soundtrigger.h"

#if defined(MTK_VOW_SUPPORT)
#include "voiceunlock2/include/VowEngine_AP_testing.h"
#endif  // #if MTK_VOW_SUPPORT

#if defined(MTK_VOW_ENABLE_CPU_BOOST)
// Power HAL
#include <vendor/mediatek/hardware/mtkpower/1.0/IMtkPerf.h>
#include "mtkperf_resource.h"
using std::vector;
using ::android::hardware::hidl_vec;
using namespace vendor::mediatek::hardware::mtkpower::V1_0;
#define CPU_BOOST_TIME_OUT (10 * 1000)
#endif

#ifdef HAL_LIBRARY_PATH_SYSTEM
#undef HAL_LIBRARY_PATH_SYSTEM
#endif
#ifdef HAL_LIBRARY_PATH_VENDOR
#undef HAL_LIBRARY_PATH_VENDOR
#endif

#define HAL_LIBRARY_PATH_SYSTEM "/system/lib/hw"
#define HAL_LIBRARY_PATH_VENDOR "/vendor/lib/hw"
#define AUDIO_HAL_PREFIX "audio.primary"
#define PLATFORM_ID "ro.board.platform"
#define BOARD_PLATFORM_ID "ro.board.platform"

static char VOW_PHASE2_CNT[] = "persist.vendor.vow.vow_phase2_cnt";
static char VOW_PHASE3_CNT[] = "persist.vendor.vow.vow_phase3_cnt";

static String8 PARAMETER_MTK_VOW_ENABLE = String8("MTK_VOW_ENABLE=1");
static String8 PARAMETER_MTK_VOW_DISABLE = String8("MTK_VOW_ENABLE=0");
static String8 PARAMETER_MTK_VOW_MIC_TYPE = String8("MTK_VOW_MIC_TYPE=");

#define ALLOCATE_MEMORY_SIZE_MAX (64 * 1024) // the max memory size can be allocated

// TODO: Move to structure for clean code & multiple engines? @{
// Move to stub device structure
int g_fd = -1;
void *audio_hw_handle = NULL;
AudioMTKHardwareInterface *gAudioMTKHardware = NULL;

// Move to recognition_context
unsigned int phase_2p5_entry_count = 0;
unsigned int phase_2p5_recog_ok_count = 0;

char tmp_write_buffer[PARSE_BUF_LEN];

#ifdef VOW_DATA_READ_PCMFILE
unsigned int g_read_file_count = 0;
#endif

#ifdef VOW_RECOG_PCMFILE
unsigned int g_recogition_file_count = 0;
#endif

AudioMTKStreamInInterface *gAudioMTKStreamIn = NULL;
#define SAMPLES1                       (160)  /* 10ms for 16k sample rate and 1 channel */
#define SAMPLES2                       (240)  /* 15ms for 16k sample rate and 1 channel */
#define VOW_VOICE_RECORD_THRESHOLD     (2560) /* 80ms */
#define VOW_VOICE_RECORD_BIG_THRESHOLD (8320) /* 260ms */
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
#define VOW_TIMES_OF_BIG_THRESHOLD     ((74880 * 2) / VOW_VOICE_RECORD_BIG_THRESHOLD)
#define VOW_VOICE_SIZE                 (0x3E800)  //256000 = 8sec, unit:byte
#else
#define VOW_TIMES_OF_BIG_THRESHOLD     (74880 / VOW_VOICE_RECORD_BIG_THRESHOLD)
#define VOW_VOICE_SIZE                 (0x1F400)  //128000 = 4sec, unit:byte
#endif
#define VOW_SAMPLE_RATE                (16000)

/* Callback byte[] data:
* data[0]: is_arrived, 0 - detected, 1 - arrived
* data[1:4]: timestamp - second
* data[5:8]: timestamp - nanosecond
*     timestamp example:
*         187.178097778 (s)
*             187 = 0x000000bb => data[1] = 0xbb, data[2] = 0x0, data[3] = 0x0, data[4] = 0x0
*             178097778 = 0x0a9d8e72 => data[5] = 0x72, data[6] = 0x8e, data[7] = 0x9d, data[8] = 0xa
*/
#define CALLBACK_DATA_IS_ARRIVED_SIZE (1)  // unit: byte
#define CALLBACK_DATA_TIMESTAMP_SEC_SIZE (4)  // unit: byte
#define CALLBACK_DATA_TIMESTAMP_NSEC_SIZE (4)  // unit: byte
#define CALLBACK_DATA_SIZE (CALLBACK_DATA_IS_ARRIVED_SIZE + CALLBACK_DATA_TIMESTAMP_SEC_SIZE + CALLBACK_DATA_TIMESTAMP_NSEC_SIZE)

#define CALLBACK_DATA_IS_ARRIVED_OFFSET (0)
#define CALLBACK_DATA_TIMESTAMP_SEC_OFFSET (CALLBACK_DATA_IS_ARRIVED_OFFSET + CALLBACK_DATA_IS_ARRIVED_SIZE)
#define CALLBACK_DATA_TIMESTAMP_NSEC_OFFSET (CALLBACK_DATA_TIMESTAMP_SEC_OFFSET + CALLBACK_DATA_TIMESTAMP_SEC_SIZE)

typedef enum {
    EVENT_PHASE_DEFAULT = 0,
    EVENT_PHASE_2 = EVENT_PHASE_DEFAULT,
    EVENT_PHASE_2P5 = 1,
} sound_trigger_event_phase_t;  // Indicate the current recognition phase

struct vow_phase_2p5_info {
    pthread_t vow_data_read_thread_id;
    short *pVoiceBuffer;  //unit: word
    struct ring_buffer_information m_rb_info;
    bool data_read_stop;
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
    short *pVoice_L_Sample;  //unit: word
    short *pVoice_R_Sample;  //unit: word
#endif
    bool already_read;
    struct recognition_context *model_context;
};

static const struct sound_trigger_properties hw_properties = {
        "The Soundtrigger HAL Project", // implementor
        "Sound Trigger stub HAL", // description
        1, // version
        { 0xed7a7d60, 0xc65e, 0x11e3, 0x9be4, { 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b } }, // uuid
        2, // max_sound_models
        2, // max_key_phrases
        2, // max_users
        RECOGNITION_MODE_VOICE_TRIGGER, // recognition_modes
        true, // capture_transition
        0, // max_buffer_ms
        false, // concurrent_capture
        false, // trigger_in_event
        0 // power_consumption_mw
};

struct recognition_context {
    // Sound Model information, added in method load_sound_model
    sound_model_handle_t model_handle;
    sound_trigger_uuid_t model_uuid;
    sound_trigger_sound_model_type_t model_type;
    sound_model_callback_t model_callback;
    void *model_cookie;

    // Sound Model information, added in start_recognition
    struct sound_trigger_recognition_config *config;
    recognition_callback_t recognition_callback;
    void *recognition_cookie;

    bool model_started;

    // Next recognition_context in the linked list
    struct recognition_context *next;

    //<MTK add
    sound_trigger_event_phase_t event_phase;
    bool phase_2p5_is_running;
    int level_1_confidence_score;
    int level_2_confidence_score;
    // Must keep phase 2.5 model data until library finishing recognizing and freed when unload model
    // phase 2.0 model data can be freed immediately after it's passed to kernel via ioctl
    char *phase_2p5_model_data;
    alock_t *phase_2p5_read_lock;
    bool is_data_read_thread_running;
    // model_state is recording the sound trigger state. when start recognition, then model_state is true.
    // when receive a wakeup event, then model_state will check is true and change into false. use this flag
    // to avoid re-entry processing.
    bool model_state;
    //>MTK add
};

struct stub_sound_trigger_device {
    struct sound_trigger_hw_device device;
    pthread_mutex_t lock;

    // This thread opens a port that can be used to monitor and inject events
    // into the stub HAL.
    pthread_t control_thread;

    // Recognition contexts are stored as a linked list
    struct recognition_context *root_model_context;

    int next_sound_model_id;
};

static void voice_update_entry_count(char *str, unsigned int entry_count) {
    char int_str[6];

    sprintf(int_str, "%d", entry_count);
    property_set(str, int_str);
}

static bool check_uuid_equality(sound_trigger_uuid_t uuid1, sound_trigger_uuid_t uuid2) {
    if (uuid1.timeLow != uuid2.timeLow ||
        uuid1.timeMid != uuid2.timeMid ||
        uuid1.timeHiAndVersion != uuid2.timeHiAndVersion ||
        uuid1.clockSeq != uuid2.clockSeq) {
        return false;
    }
    for (int i = 0; i < 6; i++) {
        if(uuid1.node[i] != uuid2.node[i]) {
            return false;
        }
    }
    return true;
}

bool str_to_uuid(char* uuid_str, sound_trigger_uuid_t* uuid) {
    if (uuid_str == NULL) {
        ALOGI("Invalid str_to_uuid input.");
        return false;
    }

    int tmp[10];
    if (sscanf(uuid_str, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x",
               tmp, tmp+1, tmp+2, tmp+3, tmp+4, tmp+5, tmp+6, tmp+7, tmp+8, tmp+9) < 10) {
        ALOGI("Invalid UUID, got: %s", uuid_str);
        return false;
    }
    uuid->timeLow = (unsigned int)tmp[0];
    uuid->timeMid = (unsigned short)tmp[1];
    uuid->timeHiAndVersion = (unsigned short)tmp[2];
    uuid->clockSeq = (unsigned short)tmp[3];
    uuid->node[0] = (unsigned char)tmp[4];
    uuid->node[1] = (unsigned char)tmp[5];
    uuid->node[2] = (unsigned char)tmp[6];
    uuid->node[3] = (unsigned char)tmp[7];
    uuid->node[4] = (unsigned char)tmp[8];
    uuid->node[5] = (unsigned char)tmp[9];
    return true;
}

void write_bad_command_error(int conn_socket, char* command) {
    int num = snprintf(tmp_write_buffer, PARSE_BUF_LEN, "Bad command received: %s", command);
    tmp_write_buffer[PARSE_BUF_LEN - 1] = '\0';  // Just to be sure.
    tmp_write_buffer[PARSE_BUF_LEN - 2] = '\n';
    write(conn_socket, tmp_write_buffer, num);
}

void write_string(int conn_socket, const char* str) {
    int num = snprintf(tmp_write_buffer, PARSE_BUF_LEN, "%s", str);
    tmp_write_buffer[PARSE_BUF_LEN - 1] = '\0';
    tmp_write_buffer[PARSE_BUF_LEN - 2] = '\n';
    write(conn_socket, tmp_write_buffer, num);
}

void write_vastr(int conn_socket, const char* format, ...) {
    va_list argptr;
    va_start(argptr, format);
    int num = vsnprintf(tmp_write_buffer, PARSE_BUF_LEN, format, argptr);
    va_end(argptr);
    tmp_write_buffer[PARSE_BUF_LEN - 1] = '\0';
    tmp_write_buffer[PARSE_BUF_LEN - 2] = '\n';
    write(conn_socket, tmp_write_buffer, num);
}

static void print_uuid(sound_trigger_uuid_t uuid) {
    ALOGI("%s %08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x", __func__, uuid.timeLow, uuid.timeMid,
          uuid.timeHiAndVersion, uuid.clockSeq, uuid.node[0], uuid.node[1], uuid.node[2],
          uuid.node[3], uuid.node[4], uuid.node[5]);
}

static void write_uuid(int conn_socket, sound_trigger_uuid_t uuid) {
    write_vastr(conn_socket, "%08x-%04x-%04x-%04x-%02x%02x%02x%02x%02x%02x\n", uuid.timeLow, uuid.timeMid,
                uuid.timeHiAndVersion, uuid.clockSeq, uuid.node[0], uuid.node[1], uuid.node[2],
                uuid.node[3], uuid.node[4], uuid.node[5]);
}

// Returns model at the given index, null otherwise (error, doesn't exist, etc).
// Note that here index starts from zero.
struct recognition_context* fetch_model_with_handle(
        struct stub_sound_trigger_device* stdev, sound_model_handle_t* model_handle) {
    ALOGI("%s", __func__);
    struct recognition_context *model_context = NULL;
    struct recognition_context *last_model_context = stdev->root_model_context;
    while(last_model_context) {
        if (last_model_context->model_handle == *model_handle) {
            model_context = last_model_context;
            break;
        }
        last_model_context = last_model_context->next;
    }
    return model_context;
}

// Returns the first model that matches the sound model UUID.
static sound_model_handle_t* get_model_handle_with_uuid(struct stub_sound_trigger_device* stdev,
                                                        sound_trigger_uuid_t uuid) {
    sound_model_handle_t* model_handle_str = NULL;
    struct recognition_context *last_model_context = stdev->root_model_context;
    while(last_model_context) {
        if (check_uuid_equality(last_model_context->model_uuid, uuid)) {
            model_handle_str = &last_model_context->model_handle;
            break;
        }
        last_model_context = last_model_context->next;
    }
    return model_handle_str;
}

/* Will reuse ids when overflow occurs */
static sound_model_handle_t generate_sound_model_handle(const struct sound_trigger_hw_device *dev) {
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    int new_id = stdev->next_sound_model_id;
    ++stdev->next_sound_model_id;
    if (stdev->next_sound_model_id == 0) {
        stdev->next_sound_model_id = 1;
    }
    return (sound_model_handle_t) new_id;
}

bool parse_socket_data(int conn_socket, struct stub_sound_trigger_device* stdev);
static void unload_all_sound_models(struct stub_sound_trigger_device *stdev);

static char *sound_trigger_keyphrase_event_alloc(sound_model_handle_t handle,
                                                 struct sound_trigger_recognition_config *config,
                                                 int recognition_status) {
    char *data;
    struct sound_trigger_phrase_recognition_event *event;
    data = (char *)calloc(1, sizeof(struct sound_trigger_phrase_recognition_event) + CALLBACK_DATA_SIZE);
    if (!data)
        return NULL;
    event = (struct sound_trigger_phrase_recognition_event *)data;
    event->common.status = recognition_status;
    event->common.type = SOUND_MODEL_TYPE_KEYPHRASE;
    event->common.model = handle;

    if (config) {
        unsigned int i;

        event->num_phrases = config->num_phrases;
        if (event->num_phrases > SOUND_TRIGGER_MAX_PHRASES)
            event->num_phrases = SOUND_TRIGGER_MAX_PHRASES;
        for (i=0; i < event->num_phrases; i++)
            memcpy(&event->phrase_extras[i],
                   &config->phrases[i],
                   sizeof(struct sound_trigger_phrase_recognition_extra));
    }

    event->num_phrases = 1;
    event->phrase_extras[0].confidence_level = 100;
    event->phrase_extras[0].num_levels = 1;
    event->phrase_extras[0].levels[0].level = 100;
    event->phrase_extras[0].levels[0].user_id = 0;
    // Signify that all the data is comming through streaming, not through the buffer.
    event->common.capture_available = true;
    event->common.audio_config = AUDIO_CONFIG_INITIALIZER;
    event->common.audio_config.sample_rate = 16000;
    event->common.audio_config.channel_mask = AUDIO_CHANNEL_IN_MONO;
    event->common.audio_config.format = AUDIO_FORMAT_PCM_16_BIT;

    return data;
}

static char *sound_trigger_generic_event_alloc(sound_model_handle_t handle,
                                               struct sound_trigger_recognition_config *config,
                                               int recognition_status) {
    char *data;
    struct sound_trigger_generic_recognition_event *event;
    data = (char *)calloc(1, sizeof(struct sound_trigger_generic_recognition_event));
    if (!data)
        return NULL;
    event = (struct sound_trigger_generic_recognition_event *)data;
    event->common.status = recognition_status;
    event->common.type = SOUND_MODEL_TYPE_GENERIC;
    event->common.model = handle;

    // Signify that all the data is comming through streaming, not through the buffer.
    event->common.capture_available = true;
    event->common.audio_config = AUDIO_CONFIG_INITIALIZER;
    event->common.audio_config.sample_rate = 16000;
    event->common.audio_config.channel_mask = AUDIO_CHANNEL_IN_MONO;
    event->common.audio_config.format = AUDIO_FORMAT_PCM_16_BIT;
    (void)config;
    return data;
}

//<MTK add

static int get_model_vendor_id(struct recognition_context *model_context) {
    int vendor_id = VENDOR_ID_NONE;
    if ((model_context->model_uuid.node[0]=='M') &&
            (model_context->model_uuid.node[1]=='T') &&
            (model_context->model_uuid.node[2]=='K') &&
            (model_context->model_uuid.node[3]=='I') &&
            (model_context->model_uuid.node[4]=='N') &&
            (model_context->model_uuid.node[5]=='C')) {
        vendor_id = VENDOR_ID_MTK;
    } else if ((model_context->model_uuid.node[0]=='A') &&
            (model_context->model_uuid.node[1]=='M') &&
            (model_context->model_uuid.node[2]=='Z') &&
            (model_context->model_uuid.node[3]=='I') &&
            (model_context->model_uuid.node[4]=='N') &&
            (model_context->model_uuid.node[5]=='C')) {
        vendor_id = VENDOR_ID_AMAZON;
    } else {
        vendor_id = VENDOR_ID_OTHERS;
    }
    ALOGI("%s(), vendor_id = %d", __func__, vendor_id);
    return vendor_id;
}

static void set_mtk_event(struct recognition_context *model_context,
                          struct sound_trigger_phrase_recognition_event *event) {
    int vendor_id = get_model_vendor_id(model_context);
    int data_offset = sizeof(struct sound_trigger_phrase_recognition_event);
    char *callback_data = (char *)event + data_offset;

    event->common.data_size = CALLBACK_DATA_SIZE;
    event->common.data_offset = data_offset;

    // set more information for Amazon
    if (vendor_id == VENDOR_ID_AMAZON) {
        bool is_arrived = false;
        struct timespec ts;

        memset((void *)&ts, 0, sizeof(timespec));

        if (model_context->event_phase == EVENT_PHASE_2) {
            is_arrived = true;
        } else {
            is_arrived = false;
        }
        (void) clock_gettime(CLOCK_MONOTONIC, &ts);
        ALOGI("%s(), is_arrived = %d, ts.tv_sec = %d = 0x%x, ts.tv_nsec = %d = 0x%x",
                __func__, is_arrived, (int)ts.tv_sec, (int)ts.tv_sec, (int)ts.tv_nsec, (int)ts.tv_nsec);

        memcpy(callback_data + CALLBACK_DATA_IS_ARRIVED_OFFSET,
               &(is_arrived), CALLBACK_DATA_IS_ARRIVED_SIZE);
        memcpy(callback_data + CALLBACK_DATA_TIMESTAMP_SEC_OFFSET,
               &(ts.tv_sec), CALLBACK_DATA_TIMESTAMP_SEC_SIZE);
        memcpy(callback_data + CALLBACK_DATA_TIMESTAMP_NSEC_OFFSET,
               &(ts.tv_nsec), CALLBACK_DATA_TIMESTAMP_NSEC_SIZE);

        /* update confidence level */
        event->phrase_extras[0].confidence_level =
                model_context->level_1_confidence_score;
        event->phrase_extras[0].levels[0].level =
                model_context->level_2_confidence_score;
    }
    ALOGI("%s(), event->common.model = %d, vendor_id = %d, event = %p, data_offset = %d, callback_data = %p, 1st_lv=%d, 2nd_lv=%d",
          __func__, event->common.model, vendor_id,
          event, data_offset, callback_data,
          model_context->level_1_confidence_score,
          model_context->level_2_confidence_score);
}
//>MTK add

void send_event_with_handle(sound_model_handle_t* model_handle_str,
                            struct stub_sound_trigger_device* stdev, int event_type,
                            int status) {
    ALOGI("%s", __func__);
    struct recognition_context *model_context = fetch_model_with_handle(stdev, model_handle_str);
    if (model_context) {
        if (event_type == EVENT_RECOGNITION) {
            if (model_context->recognition_callback == NULL) {
                ALOGI("%s No matching callback", __func__);
                return;
            }

            if (model_context->model_type == SOUND_MODEL_TYPE_KEYPHRASE) {
                struct sound_trigger_phrase_recognition_event *event;
                event = (struct sound_trigger_phrase_recognition_event *)
                        sound_trigger_keyphrase_event_alloc(model_context->model_handle,
                                                            model_context->config, status);
                //<MTK add
                set_mtk_event(model_context, event);
                //>MTK add
                if (event) {
                    model_context->recognition_callback(&(event->common), model_context->recognition_cookie);
                    free(event);
                }
            } else if (model_context->model_type == SOUND_MODEL_TYPE_GENERIC) {
                struct sound_trigger_generic_recognition_event *event;
                event = (struct sound_trigger_generic_recognition_event *)
                        sound_trigger_generic_event_alloc(model_context->model_handle,
                                                          model_context->config, status);
                if (event) {
                    model_context->recognition_callback(&(event->common), model_context->recognition_cookie);
                    free(event);
                }
            } else {
                ALOGI("Unknown Sound Model Type, No Event to Send");
            }
        } else if (event_type == EVENT_SOUND_MODEL) {
            char *data;
            data = (char *)calloc(1, sizeof(struct sound_trigger_model_event));
            if (!data) {
                ALOGW("%s Could not allocate event", __func__);
                return;
            }

            struct sound_trigger_model_event *event;
            event = (struct sound_trigger_model_event *)data;
            event->status = SOUND_MODEL_STATUS_UPDATED;
            event->model = model_context->model_handle;
            if (event) {
                model_context->model_callback(event, model_context->model_cookie);
                free(event);
            }
        }
    } else {
        ALOGI("No model for this handle");
    }
}
//<MTK add
static void *voice_data_read_thread_loop(void *context) {
    struct vow_phase_2p5_info *phase_2p5_info = (struct vow_phase_2p5_info *)context;
    struct recognition_context *model_context = NULL;
    unsigned int read_count, need_count = 0;
    unsigned int read_loop = 0;
    unsigned int free_size = 0;    // unit: byte
    unsigned int bottom_size = 0;  // unit: byte
    unsigned int write_size = 0;   // unit: byte

#ifdef VOW_DATA_READ_PCMFILE
    char file_name_full_path[] = "/sdcard/record_audio.pcm";
    char file_name[] = "/sdcard/record_audio";
    char file_name_ext[] = ".pcm";
    char file_name_full[40];
    FILE *record_file;
    char buffer[4];
#endif  // #ifdef VOW_DATA_READ_PCMFILE

    /*
    * Adjust thread priority
    */
    prctl(PR_SET_NAME, (unsigned long)"sound trigger read", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);
    ALOGD("%s(), pid: %d, tid: %d", __FUNCTION__, getpid(), gettid());

    if (phase_2p5_info == 0) {
        ALOGI("%s(), phase_2p5_info == 0", __func__);
        goto exit;
    }
    model_context = phase_2p5_info->model_context;

    if (!model_context->model_started) {
        ALOGI("%s(), model not started!", __func__);
        goto exit;
    }

    model_context->is_data_read_thread_running = true;

    if (gAudioMTKStreamIn == NULL) {
        ALOGI("%s(), openInputStream error", __func__);
        goto exit;
    }
    /* Set parameter with AUDIO_SOURCE_HOTWORD */
    gAudioMTKStreamIn->setParameters(String8("input_source=1999"));
#ifdef VOW_DATA_READ_PCMFILE
    sprintf(buffer, "%d", g_read_file_count);
    strcpy(file_name_full, file_name);
    strcpy(&file_name_full[strlen(file_name_full)], buffer);
    strcpy(&file_name_full[strlen(file_name_full)], file_name_ext);
    ALOGI("%s(), filename = %s, len = %d", __func__, file_name_full, strlen(file_name_full));
    record_file = fopen(file_name_full, "wb+");
    if (NULL == record_file) {
        ALOGI("%s(), fopen error", __func__);
        goto exit;
    }
    g_read_file_count++;
#endif  // #ifdef VOW_DATA_READ_PCMFILE
    ALOGV("%s(), model_context->model_started = %d, data_read_stop = %d",
          __func__,
          model_context->model_started,
          (unsigned int)phase_2p5_info->data_read_stop);
    while ((model_context->model_started) && (!phase_2p5_info->data_read_stop)) {
        free_size = ring_buffer_get_space_byte_count(&phase_2p5_info->m_rb_info) / sizeof(short);
        write_size = (read_loop < VOW_TIMES_OF_BIG_THRESHOLD)?
                     VOW_VOICE_RECORD_BIG_THRESHOLD : VOW_VOICE_RECORD_THRESHOLD;
        read_loop++;
        if (free_size > write_size) {
            short *p_buf = NULL;
            uint32_t buf_size = 0;

            ring_buffer_get_write_information(&phase_2p5_info->m_rb_info, (uint8_t **)&p_buf, &buf_size);
            bottom_size = buf_size;
            read_count = 0;
            if (bottom_size < write_size) {
                read_count = gAudioMTKStreamIn->read(p_buf, bottom_size);
                ring_buffer_write_done(&phase_2p5_info->m_rb_info, bottom_size);
#ifdef VOW_DATA_READ_PCMFILE
                fwrite(p_buf, bottom_size, 1, record_file);
#endif  // #ifdef VOW_DATA_READ_PCMFILE
                ring_buffer_get_write_information(&phase_2p5_info->m_rb_info, (uint8_t **)&p_buf, &buf_size);
                read_count += gAudioMTKStreamIn->read(p_buf, write_size - bottom_size);
                ring_buffer_write_done(&phase_2p5_info->m_rb_info, write_size - bottom_size);
#ifdef VOW_DATA_READ_PCMFILE
                fwrite(p_buf, write_size - bottom_size, 1, record_file);
#endif  // #ifdef VOW_DATA_READ_PCMFILE
            } else {
                read_count = gAudioMTKStreamIn->read(p_buf, write_size);
                ring_buffer_write_done(&phase_2p5_info->m_rb_info, write_size);
#ifdef VOW_DATA_READ_PCMFILE
                fwrite(p_buf, write_size, 1, record_file);
#endif  // #ifdef VOW_DATA_READ_PCMFILE
            }
            if (read_count == 0) {
                ALOGI("%s(), StreamIn Rec error", __func__);
                break;
            }
            need_count += read_count;
            ALOGV("%s(), read_count = %d, need_count = %d, read_loop = %d", __func__, read_count, need_count, read_loop);
            phase_2p5_info->already_read = true;
            SIGNAL_ALOCK(model_context->phase_2p5_read_lock);
        } else {
            ALOGV("%s(), free space not enough", __func__);
        }
    }
    ALOGI("%s(), need_count = %d", __func__, need_count);

#ifdef VOW_DATA_READ_PCMFILE
    fclose(record_file);
#endif  // #ifdef VOW_DATA_READ_PCMFILE

exit:
    model_context->is_data_read_thread_running = false;
    phase_2p5_info->already_read = false;
    ALOGI("()%s, exit", __func__);
    pthread_exit(NULL);
}

#ifdef MTK_VOW_DUAL_MIC_SUPPORT
static void voice_deinterleave(short *out_l_buf,
                               short *out_r_buf,
                               short *in_buf,
                               unsigned int sample_num)
{
    int i;
    for (i = 0; i < sample_num; i++) {
        *out_l_buf++ = *in_buf++;
        *out_r_buf++ = *in_buf++;
    }
}
#endif

int phase_2p5_testing(struct vow_phase_2p5_info *phase_2p5_info) {
    struct recognition_context *model_context = NULL;
    int lock_ret = 0;
    bool is_testing_done = 0;
    int open_input_result = 0;
    unsigned int testing_result = RECOG_FAIL;
    short *temp_buffer;
    short *record_buffer;
    int i;
    unsigned int size = 0;         // unit: byte
    unsigned int reserv_size = 0;  // unit: byte
    unsigned int bottom_size = 0;  // unit: byte
#if MTK_VOW_SUPPORT
    int vow_engine_result = vowe_bad;
    int rtn_L;
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
    int rtn_R;
#endif
#endif  // #if MTK_VOW_SUPPORT
    bool is_first_frame = true;
    unsigned int size_count = 0;

    int input_format = AUDIO_FORMAT_PCM_16_BIT;
    uint32_t input_device = AUDIO_DEVICE_IN_BUILTIN_MIC;
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
    uint32_t input_channel = AUDIO_CHANNEL_IN_STEREO;
#else
    uint32_t input_channel = AUDIO_CHANNEL_IN_MONO;
#endif
    uint32_t input_sample_rate = VOW_SAMPLE_RATE;

#ifdef VOW_RECOG_PCMFILE
    char file_name_full_path[] = "/sdcard/record_recog_audio.pcm";
    char file_name[] = "/sdcard/record_recog_audio";
    char file_name_ext[] = ".pcm";
    char file_name_full[40];
    FILE *record_file;
    char buffer[4];
#endif  // #ifdef VOW_RECOG_PCMFILE

    ALOGI("%s()", __func__);

#if defined(MTK_VOW_ENABLE_CPU_BOOST)
    sp<IMtkPerf> power_service = NULL;
    int power_handle = 0;
    const int PERF_PARAMS_COUNT = 6;
    int perf_lock_opts[PERF_PARAMS_COUNT] = {
            PERF_RES_CPUFREQ_PERF_MODE, 1,  // force all cpu run at the highest freq
            PERF_RES_DRAM_OPP_MIN, 0,  // force DDR run at the highest freq
            PERF_RES_SCHED_BOOST, 1};  // big core first
    std::vector<int32_t> opt_list;
    power_service = IMtkPerf::tryGetService();
    if (power_service != NULL) {
        opt_list.assign(perf_lock_opts, (perf_lock_opts + PERF_PARAMS_COUNT));
        power_handle = power_service->perfLockAcquire(power_handle,
                CPU_BOOST_TIME_OUT,
                opt_list,
                PERF_PARAMS_COUNT);
        ALOGD("%s(), get powerService, power_service: %p, handle %d", __func__, power_service.get(), power_handle);
    } else {
        ALOGE("%s(), failed to get powerService", __func__);
        //goto exit;
    }
#endif

    if (phase_2p5_info == 0) {
        ALOGI("%s(), phase_2p5_info == 0", __func__);
        goto exit;
    }
    model_context = phase_2p5_info->model_context;

    if (!model_context->model_started) {
        ALOGI("%s(), model not started!", __func__);
        goto exit;
    }
    if (gAudioMTKStreamIn != NULL) {
        ALOGI("%s(), openInputStream is not NULL", __func__);
        goto exit;
    }
#ifdef VOW_RECOG_PCMFILE
    sprintf(buffer, "%d", g_recogition_file_count);
    strcpy(file_name_full, file_name);
    strcpy(&file_name_full[strlen(file_name_full)], buffer);
    strcpy(&file_name_full[strlen(file_name_full)], file_name_ext);
    ALOGI("file_name_full = %s, len = %d", file_name_full, strlen(file_name_full));
    record_file = fopen(file_name_full, "wb+");
    if (NULL == record_file) {
        ALOGI("%s(), fopen error", __func__);
        goto exit;
    }
    g_recogition_file_count++;
#endif  // #ifdef VOW_RECOG_PCMFILE

    phase_2p5_info->data_read_stop = false;

    /* Open Input Stream */
    if (gAudioMTKHardware != NULL) {
        gAudioMTKStreamIn = gAudioMTKHardware->openInputStreamWithFlags(input_device,
                                                                        &input_format,
                                                                        &input_channel,
                                                                        &input_sample_rate,
                                                                        &open_input_result,
                                                                        (audio_in_acoustics_t)0,
                                                                        AUDIO_INPUT_FLAG_RAW);
        ALOGI("%s(), openInputStream = %p", __func__, gAudioMTKStreamIn);
    }

    pthread_create(&phase_2p5_info->vow_data_read_thread_id, (const pthread_attr_t *) NULL,
                   voice_data_read_thread_loop, phase_2p5_info);

    ALOGI("%s(), ready to read", __func__);
    size_count = 0;
    i = 0;
#if MTK_VOW_SUPPORT
    ALOGD("%s(), [P2.5] VOWE_AP_testing_reset", __func__);
    if (VOWE_AP_testing_reset() == vowe_bad) {
        ALOGI("%s(), [ASP API] reset error", __func__);
    }
#endif  // #if MTK_VOW_SUPPORT
    temp_buffer = new short[SAMPLES2];  // for combine data use
    memset(temp_buffer, 0, SAMPLES2 * sizeof(short));
    while (model_context->model_started && !is_testing_done) {
        // wait a lock for 1sec to receive voice data from scp
        lock_ret = WAIT_ALOCK_MS(model_context->phase_2p5_read_lock, 1000);
        //ALOGI("lock_ret=%d", lock_ret);
        if (lock_ret == -ETIMEDOUT) {
            ALOGV("%s(), wait signal timeout: %d", __func__, lock_ret);
            ALOGI("%s(), model_context->is_data_read_thread_running = %d, already_read = %d",
                    __func__, model_context->is_data_read_thread_running, phase_2p5_info->already_read);
            break;
        }

        while (1) {
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
            size = is_first_frame ? (SAMPLES2 << 2) : (SAMPLES1 << 2);  // unit: byte
#else
            size = is_first_frame ? (SAMPLES2 << 1) : (SAMPLES1 << 1);  // unit: byte
#endif
            if (phase_2p5_info->already_read) {
                reserv_size = ring_buffer_get_data_byte_count(&phase_2p5_info->m_rb_info);
            } else {
                reserv_size = 0;
            }
            if (reserv_size >= size) {
                uint8_t *p_buf = NULL;
                uint32_t buf_size = 0;

                // make sure there is enough data to input first frame
                if (is_first_frame) {
                    is_first_frame = false;
                }
                ring_buffer_get_read_information(&phase_2p5_info->m_rb_info, &p_buf, &buf_size);
                bottom_size = buf_size;
                if (bottom_size < size) {
                    memcpy(&temp_buffer[0], p_buf, bottom_size);
                    ring_buffer_read_done(&phase_2p5_info->m_rb_info, bottom_size);
                    ring_buffer_get_read_information(&phase_2p5_info->m_rb_info, &p_buf, &buf_size);
                    memcpy(&temp_buffer[bottom_size], p_buf, size - bottom_size);
                    ring_buffer_read_done(&phase_2p5_info->m_rb_info, size - bottom_size);
                } else {
                    memcpy(&temp_buffer[0], p_buf, size);
                    ring_buffer_read_done(&phase_2p5_info->m_rb_info, size);
                }
                record_buffer = &temp_buffer[0];

#ifdef MTK_VOW_DUAL_MIC_SUPPORT
                voice_deinterleave(phase_2p5_info->pVoice_L_Sample,
                                   phase_2p5_info->pVoice_R_Sample,
                                   record_buffer,
                                   (size >> 2));
                // already separate into two parts L and R, so need to div 2
                size = size >> 1;
#endif
                i++;
                size_count += size;
#ifdef VOW_RECOG_PCMFILE
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
                fwrite(&phase_2p5_info->pVoice_L_Sample[0], sizeof(short), (size >> 1), record_file);
#else
                fwrite(record_buffer, size, 1, record_file);
#endif
#endif  // #ifdef VOW_RECOG_PCMFILE

#if MTK_VOW_SUPPORT
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
                rtn_L = VOWE_AP_testing_inputMic(0, phase_2p5_info->pVoice_L_Sample, (size >> 1));
                rtn_R = VOWE_AP_testing_inputMic(1, phase_2p5_info->pVoice_R_Sample, (size >> 1));
#else
                rtn_L = VOWE_AP_testing_inputMic(0, record_buffer, (size >> 1));
#endif
                if (rtn_L == vowe_bad) {
                    ALOGI("%s(), [P2.5] InputMic error: sz = %d", __func__, (size >> 1));
                    is_testing_done = true;
                    break;
                }
                ALOGV("%s(), [P2.5] reg+, sz = %d", __func__, (size >> 1));
                vow_engine_result = VOWE_AP_testing_recognize();

                ALOGV("%s(), ==> read %d", __func__, i);
                if (vow_engine_result == vowe_bad) {
                    ALOGI("%s(), [P2.5] Recog error, check flow", __func__);
                    testing_result = RECOG_BAD;
                    is_testing_done = true;
                    break;
                } else if (vow_engine_result == vowe_reject) {
                    ALOGI("%s(), [P2.5] check keyword fail", __func__);
                    testing_result = RECOG_FAIL;
                    is_testing_done = true;
                    break;
                } else if (vow_engine_result == vowe_accept) {
                    ALOGI("%s(), [P2.5] check keyword success", __func__);
                    testing_result = RECOG_PASS;
                    is_testing_done = true;
                    break;
                }
#endif  // #if MTK_VOW_SUPPORT
            } else {
                /* there is no enough stream in data, need wait for next receive */
                break;
            }
        }
    }
    ALOGI("%s(), size_count = %d, loop = %d", __func__, size_count, i);
#ifdef VOW_RECOG_PCMFILE
    fclose(record_file);
#endif  // #ifdef VOW_RECOG_PCMFILE
    delete[] temp_buffer;

    /* notice data_read thread to stop */
    phase_2p5_info->data_read_stop = true;
    ALOGI("%s(), +pthread_join_read_data", __func__);
    pthread_join(phase_2p5_info->vow_data_read_thread_id, NULL);
    ALOGI("%s(), -pthread_join_read_data", __func__);

    /* Close Input Stream */
    if (gAudioMTKStreamIn != NULL) {
        gAudioMTKStreamIn->standby();
        ALOGI("%s(), [P2.5] 1. standby ok", __func__);
    }

    if (gAudioMTKHardware != NULL) {
        gAudioMTKHardware->closeInputStream(gAudioMTKStreamIn);
        ALOGI("%s(), clear gAudioMTKStreamIn", __func__);
        gAudioMTKStreamIn = NULL;
        ALOGI("%s(), [P2.5] 2. closeInputStream ok", __func__);
    }

exit:
#if defined(MTK_VOW_ENABLE_CPU_BOOST)
    if (power_service != NULL) {
       power_service->perfLockRelease(power_handle, 0);
    }
#endif
    ALOGI("%s(), exit, testing_result = %d", __func__, testing_result);
    return testing_result;
}

int start_recognition_phase_2p5(struct recognition_context *model_context) {
    struct vow_phase_2p5_info *phase_2p5_info;
    int phase_2p5_testing_reult = RECOG_FAIL;

    /* Allocate memory for pthread_create() arguments */
    phase_2p5_info = (struct vow_phase_2p5_info *)calloc(1, sizeof(struct vow_phase_2p5_info));
    if (phase_2p5_info == NULL) {
        ALOGE("%s(), calloc error", __func__);
    } else {
        phase_2p5_info->vow_data_read_thread_id = 0;
        phase_2p5_info->already_read = false;
        phase_2p5_info->pVoiceBuffer = new short[(VOW_VOICE_SIZE >> 1)];
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
        phase_2p5_info->pVoice_L_Sample = new short[SAMPLES2];
        phase_2p5_info->pVoice_R_Sample = new short[SAMPLES2];
#endif
        phase_2p5_info->m_rb_info.write_pointer = 0;
        phase_2p5_info->m_rb_info.read_pointer = 0;
        phase_2p5_info->m_rb_info.buffer_byte_count = VOW_VOICE_SIZE;
        phase_2p5_info->m_rb_info.buffer_base_pointer = (uint8_t *)&phase_2p5_info->pVoiceBuffer[0];
        phase_2p5_info->model_context = model_context;

        model_context->phase_2p5_is_running = true;
        ALOGV("1.phase_2p5_is_running = %d", (unsigned int) model_context->phase_2p5_is_running);
        phase_2p5_testing_reult = phase_2p5_testing(phase_2p5_info);
        model_context->phase_2p5_is_running = false;
        ALOGV("2.phase_2p5_is_running = %d", (unsigned int) model_context->phase_2p5_is_running);
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
        delete[] phase_2p5_info->pVoice_L_Sample;
        delete[] phase_2p5_info->pVoice_R_Sample;
#endif
        delete[] phase_2p5_info->pVoiceBuffer;
        free(phase_2p5_info);
    }
    return phase_2p5_testing_reult;
}

//>MTK add
static void send_event(int conn_socket, struct stub_sound_trigger_device* stdev, int event_type,
                       int status) {
    char* model_uuid_str = strtok(NULL, " \r\n");
    sound_trigger_uuid_t model_uuid;
    if (str_to_uuid(model_uuid_str, &model_uuid)) {
        sound_model_handle_t* model_handle_str = get_model_handle_with_uuid(stdev, model_uuid);
        if (model_handle_str == NULL) {
            ALOGI("%s Bad sound model handle.", __func__);
            write_string(conn_socket, "Bad sound model handle.\n");
            return;
        }
        send_event_with_handle(model_handle_str, stdev, event_type, status);
    } else {
        ALOGI("%s Not a valid UUID", __func__);
        write_string(conn_socket, "Not a valid UUID.\n");
    }
}

static bool recognition_callback_exists(struct stub_sound_trigger_device *stdev) {
    bool callback_found = false;
    if (stdev->root_model_context) {
        struct recognition_context *current_model_context = stdev->root_model_context;
        while(current_model_context) {
            if (current_model_context->recognition_callback != NULL) {
                callback_found = true;
                break;
            }
            current_model_context = current_model_context->next;
        }
    }
    return callback_found;
}

static struct recognition_context * get_model_context(struct stub_sound_trigger_device *stdev,
            sound_model_handle_t handle) {
    struct recognition_context *model_context = NULL;
    if (stdev->root_model_context) {
        struct recognition_context *current_model_context = stdev->root_model_context;
        while(current_model_context) {
            if (current_model_context->model_handle == handle) {
                model_context = current_model_context;
                break;
            }
            current_model_context = current_model_context->next;
        }
    }
    return model_context;
}

static void *control_thread_loop(void *context) {
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)context;
    bool exit = false;
    struct vow_eint_data_struct_t int_data;
    int vendor_id;

    ALOGI("%s()", __func__);

    prctl(PR_SET_NAME, (unsigned long)"sound trigger callback", 0, 0, 0);
    setpriority(PRIO_PROCESS, 0, ANDROID_PRIORITY_AUDIO);

    pthread_mutex_lock(&stdev->lock);
    if (g_fd < 0) {
        g_fd = open("/dev/vow", O_RDONLY);
    }
    ALOGI("g_fd = %d", g_fd);
    if (g_fd < 0) {
        ALOGE("%s(), open device fail!%s\n", __func__, strerror(errno));
        exit = true;
    }
    pthread_mutex_unlock(&stdev->lock);

    // initialize the received data from interrupt
    memset((void *)&int_data, 0, sizeof(int_data));
    int_data.eint_status = VOW_EINT_FAIL;

    ALOGI("%s(), 1. VoiceWakeup interrupt status: status = %d",
          __func__, int_data.eint_status);

    while(!exit) {
        if (g_fd >= 0) {
            read(g_fd, &int_data, sizeof(struct vow_eint_data_struct_t));
            if (int_data.eint_status == VOW_EINT_PASS) {
                struct recognition_context *model_context;

                ALOGI("%s(), 2. VoiceWakeup interrupt status: status = %d, id = %d",
                      __func__, int_data.eint_status, int_data.id);
                pthread_mutex_lock(&stdev->lock);
                model_context = fetch_model_with_handle(stdev, &(int_data.id));
                if (!model_context) {
                    ALOGW("%s(), Can't find sound model handle %d in registered list", __func__, int_data.id);
                    pthread_mutex_unlock(&stdev->lock);
                    continue;
                }
                if (model_context->model_state != true) {
                    ALOGI("%s(), previous is no start recog, ignore this event", __func__);
                    pthread_mutex_unlock(&stdev->lock);
                    continue;
                }
                // set recognition phase
                model_context->event_phase = EVENT_PHASE_2;
                vendor_id = get_model_vendor_id(model_context);
                model_context->level_1_confidence_score = int_data.data[0];
                // send recognition pass event after phase 2.5 pass for MTK
                if (vendor_id != VENDOR_ID_MTK) {
                    model_context->model_state = false;
                    send_event_with_handle(&model_context->model_handle,
                                           stdev,
                                           EVENT_RECOGNITION,
                                           RECOGNITION_STATUS_SUCCESS);
                }
                pthread_mutex_unlock(&stdev->lock);

                // start phase 2.5 recognition if need, it blocks control thread until recognition done
                if (model_context->phase_2p5_model_data != NULL) {
                    int phase_2p5_testing_reult = RECOG_FAIL;

                    if (vendor_id == VENDOR_ID_MTK) {
                        phase_2p5_testing_reult = start_recognition_phase_2p5(model_context);
                    } else if (vendor_id == VENDOR_ID_AMAZON) {
                        phase_2p5_testing_reult = start_recognition_phase_2p5(model_context);
                    }
                    phase_2p5_entry_count++;
                    voice_update_entry_count(VOW_PHASE2_CNT, phase_2p5_entry_count);
                    pthread_mutex_lock(&stdev->lock);
                    if (phase_2p5_testing_reult == RECOG_PASS) {
#if MTK_VOW_SUPPORT
                        if ((vendor_id == VENDOR_ID_AMAZON) || (vendor_id == VENDOR_ID_MTK)){
                            float *values;
                            int numValue;

                            if (VOWE_AP_testing_getFloatArrayArgument(
                                    vowe_argid_testSIDLastUtteranceConfidenceScores,
                                    &values, &numValue) == vowe_ok) {
                                ALOGI("%s(), SID confidence score: %f\n",
                                      __func__, values[0]);
                                model_context->level_2_confidence_score = values[0];
                            } else {
                                ALOGE("%s(), SID confidence get err\n", __func__);
                            }
                        }
#endif
                        // callback phase 2.5 pass event if testing pass
                        model_context->event_phase = EVENT_PHASE_2P5;
                        model_context->model_state = false;
                        send_event_with_handle(&model_context->model_handle,
                                               stdev,
                                               EVENT_RECOGNITION,
                                               RECOGNITION_STATUS_SUCCESS);

                        // update phase 2.5 pass counter
                        phase_2p5_recog_ok_count++;
                        voice_update_entry_count(VOW_PHASE3_CNT, phase_2p5_recog_ok_count);
                        ALOGI("%s(), [P2.5] enter_P25_cnt: %d, enter_P3_cnt: %d\n",
                              __func__, phase_2p5_entry_count, phase_2p5_recog_ok_count);
                    } else {
                        ALOGI("%s(), [P2.5] Recog is not accept: %d", __func__, phase_2p5_recog_ok_count);
                    }
                    pthread_mutex_unlock(&stdev->lock);
                }
            }
        } else {
            usleep(300 * 1000); // sleep 300msec
        }
    }

    return NULL;
}

#if 0
static void *control_thread_loop_socket(void *context) {
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)context;
    struct sockaddr_in incoming_info;
    struct sockaddr_in self_info;
    int self_socket;
    socklen_t sock_size = sizeof(struct sockaddr_in);
    memset(&self_info, 0, sizeof(self_info));
    self_info.sin_family = AF_INET;
    self_info.sin_addr.s_addr = htonl(INADDR_ANY);
    self_info.sin_port = htons(14035);

    bool exit = false;
    while(!exit) {
        int received_count;
        int requested_count = 2;
        char buffer[requested_count];
        ALOGE("Opening socket");
        self_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (self_socket < 0) {
            ALOGE("Error on socket creation: %s", strerror(errno));
            exit = true;
        } else {
            ALOGI("Socket created");
        }

        int reuse = 1;
        if (setsockopt(self_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0) {
            ALOGE("setsockopt(SO_REUSEADDR) failed");
        }

        int bind_result = bind(self_socket, (struct sockaddr *)&self_info, sizeof(struct sockaddr));
        if (bind_result < 0) {
            ALOGE("Error on bind");
            exit = true;
        }

        int listen_result = listen(self_socket, 1);
        if (listen_result < 0) {
            ALOGE("Error on Listen");
            exit = true;
        }

        while(!exit) {
            int con_socket = accept(self_socket, (struct sockaddr *)&incoming_info, &sock_size);
            if (!con_socket) {
                ALOGE("Lost socket, cannot send trigger");
                break;
            }
            ALOGI("Connection from %s", inet_ntoa(incoming_info.sin_addr));
            if (!parse_socket_data(con_socket, stdev)) {
                ALOGI("Done processing commands over network. Stopping thread.");
                exit = true;
            }
            close(con_socket);
        }
        ALOGE("Closing socket");
        close(self_socket);
    }

    return NULL;
}
#endif

void list_models(int conn_socket, char* buffer,
                 struct stub_sound_trigger_device* stdev) {
    ALOGI("%s", __func__);
    struct recognition_context *last_model_context = stdev->root_model_context;
    unsigned int model_index = 0;
    (void)buffer;
    write_string(conn_socket, "-----------------------\n");
    if (!last_model_context) {
        ALOGI("ZERO Models exist.");
        write_string(conn_socket, "Zero models exist.\n");
    }
    while (last_model_context) {
        write_vastr(conn_socket, "Model Index: %d\n", model_index);
        ALOGI("Model Index: %d", model_index);
        write_vastr(conn_socket, "Model handle: %d\n", last_model_context->model_handle);
        ALOGI("Model handle: %d", last_model_context->model_handle);
        write_uuid(conn_socket, last_model_context->model_uuid);
        print_uuid(last_model_context->model_uuid);
        sound_trigger_sound_model_type_t model_type = last_model_context->model_type;

        if (model_type == SOUND_MODEL_TYPE_KEYPHRASE) {
            write_string(conn_socket, "Keyphrase sound Model.\n");
            ALOGI("Keyphrase sound Model.");
        } else if (model_type == SOUND_MODEL_TYPE_GENERIC) {
            write_string(conn_socket, "Generic sound Model.\n");
            ALOGI("Generic sound Model.");
        } else {
            write_vastr(conn_socket, "Unknown sound model type: %d\n", model_type);
            ALOGI("Unknown sound model type: %d", model_type);
        }
        if (last_model_context->model_started) {
            write_string(conn_socket, "Model started.\n");
            ALOGI("Model started.\n");
        } else {
            write_string(conn_socket, "Model stopped.\n");
            ALOGI("Model stopped.\n");
        }
        write_string(conn_socket, "-----------------------\n\n");
        ALOGI("----\n\n");
        last_model_context = last_model_context->next;
        model_index++;
    }
}

// Gets the next word from buffer, replaces '\n' or ' ' with '\0'.
char* get_command(char* buffer) {
    char* command = strtok(buffer, " ");
    char* newline = strchr(command, '\n');
    if (newline != NULL) {
        *newline = '\0';
    }
    return command;
}

// Parses data coming in from the local socket, executes commands. Returns when
// done. Return code indicates whether the server should continue listening or
// abort (true if continue listening).
bool parse_socket_data(int conn_socket, struct stub_sound_trigger_device* stdev) {
    ALOGI("Calling parse_socket_data");
    bool input_done = false;
    char buffer[PARSE_BUF_LEN];
    FILE* input_fp = fdopen(conn_socket, "r");
    bool continue_listening = true;

    // Note: Since we acquire a lock inside this loop, do not use break or other
    // exit methods without releasing this lock.
    write_string(conn_socket, "\n>>> ");
    while(!input_done) {
        if (fgets(buffer, PARSE_BUF_LEN, input_fp) != NULL) {
            pthread_mutex_lock(&stdev->lock);
            char* command = strtok(buffer, " \r\n");
            if (command == NULL) {
                write_bad_command_error(conn_socket, command);
            } else if (strncmp(command, COMMAND_LS, 2) == 0) {
                list_models(conn_socket, buffer, stdev);
            } else if (strcmp(command, COMMAND_RECOGNITION_TRIGGER) == 0) {
                send_event(conn_socket, stdev, EVENT_RECOGNITION, RECOGNITION_STATUS_SUCCESS);
            } else if (strcmp(command, COMMAND_RECOGNITION_ABORT) == 0) {
                send_event(conn_socket, stdev, EVENT_RECOGNITION, RECOGNITION_STATUS_ABORT);
            } else if (strcmp(command, COMMAND_RECOGNITION_FAILURE) == 0) {
                send_event(conn_socket, stdev, EVENT_RECOGNITION, RECOGNITION_STATUS_FAILURE);
            } else if (strcmp(command, COMMAND_UPDATE) == 0) {
                send_event(conn_socket, stdev, EVENT_SOUND_MODEL, SOUND_MODEL_STATUS_UPDATED);
            } else if (strncmp(command, COMMAND_CLEAR, 5) == 0) {
                unload_all_sound_models(stdev);
            } else if (strncmp(command, COMMAND_CLOSE, 5) == 0) {
                ALOGI("Closing this connection.");
                write_string(conn_socket, "Closing this connection.");
                input_done = true;
            } else if (strncmp(command, COMMAND_END, 3) == 0) {
                ALOGI("End command received.");
                write_string(conn_socket, "End command received. Stopping connection.");
                continue_listening = false;
                input_done = true;
            } else {
                write_vastr(conn_socket, "\nBad command %s.\n\n", command);
            }
            pthread_mutex_unlock(&stdev->lock);
        } else {
            ALOGI("parse_socket_data done (got null)");
            input_done = true;  // break.
        }
        write_string(conn_socket, "\n>>> ");
    }
    return continue_listening;
}

#if 0
static void send_loop_kill_signal() {
    ALOGI("Sending loop thread kill signal");
    int self_socket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in remote_info;
    memset(&remote_info, 0, sizeof(remote_info));
    remote_info.sin_family = AF_INET;
    remote_info.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    remote_info.sin_port = htons(14035);
    if (connect(self_socket, (struct sockaddr *)&remote_info, sizeof(struct sockaddr)) == 0) {
        send(self_socket, COMMAND_END, 3, 0);
    } else {
        ALOGI("Could not connect");
    }
    close(self_socket);
    ALOGI("Sent loop thread kill signal");
}
#endif

//<MTK add
static void init_mtk_model_context(struct recognition_context *context) {
    context->event_phase = EVENT_PHASE_DEFAULT;
    context->phase_2p5_model_data = NULL;
    context->phase_2p5_read_lock = NULL;
    context->is_data_read_thread_running = false;
    context->phase_2p5_is_running = false;
}

static int open_audio_hal() {
    if (audio_hw_handle == NULL) {
        char prop[PATH_MAX];
        char path[PATH_MAX];
        do {
            if (property_get(PLATFORM_ID, prop, NULL) == 0) {
                snprintf(path, sizeof(path), "%s/%s.default.so",
                         HAL_LIBRARY_PATH_SYSTEM, prop);
                if (access(path, R_OK) == 0) { break; }


                if (access(path, R_OK) == 0) { break; }
            } else {
                snprintf(path, sizeof(path), "%s/%s.%s.so",
                         HAL_LIBRARY_PATH_SYSTEM, AUDIO_HAL_PREFIX, prop);
                if (access(path, R_OK) == 0) { break; }

                snprintf(path, sizeof(path), "%s/%s.%s.so",
                         HAL_LIBRARY_PATH_VENDOR, AUDIO_HAL_PREFIX, prop);
                if (access(path, R_OK) == 0) { break; }

                if (property_get(BOARD_PLATFORM_ID, prop, NULL) == 0) {
                    snprintf(path, sizeof(path), "%s/%s.default.so",
                             HAL_LIBRARY_PATH_SYSTEM, prop);
                    if (access(path, R_OK) == 0) { break; }

                    snprintf(path, sizeof(path), "%s/%s.default.so",
                             HAL_LIBRARY_PATH_SYSTEM, prop);
                    if (access(path, R_OK) == 0) { break; }
                } else {
                    snprintf(path, sizeof(path), "%s/%s.%s.so",
                             HAL_LIBRARY_PATH_SYSTEM, AUDIO_HAL_PREFIX, prop);
                    if (access(path, R_OK) == 0) { break; }

                    snprintf(path, sizeof(path), "%s/%s.%s.so",
                             HAL_LIBRARY_PATH_VENDOR, AUDIO_HAL_PREFIX, prop);
                    if (access(path, R_OK) == 0) { break; }
                }
            }
        } while (0);

        ALOGD("%s(), Load %s", __func__, path);
        audio_hw_handle = dlopen(path, RTLD_NOW);
        const char *dlsym_error1 = dlerror();
        ALOGE("%s(), dlerror()=%s", __func__, dlsym_error1);
        if (audio_hw_handle == NULL) {
            ALOGE("%s(), -DL open audio_hw_handle path [%s] fail", __func__, path);
            return -ENOSYS;
        } else {
            create_AudioMTKHw *func1 = (create_AudioMTKHw *)dlsym(audio_hw_handle, "createMTKAudioHardware");
            ALOGD("%s(), %d func1 %p", __func__, __LINE__, func1);
            if (func1 == NULL) {
                ALOGE("%s(), -dlsym createMTKAudioHardware fail", __func__);
                /* close dlopen */
                dlclose(audio_hw_handle);
                audio_hw_handle = NULL;
                return -ENOSYS;
            }
            gAudioMTKHardware = func1();
            ALOGD("%s(), dlopen success gAudioMTKHardware", __func__);
        }
    }
    return 0;
}

static bool started_model_exists(const struct stub_sound_trigger_device *stdev) {
    bool started_model_found = false;
    if (stdev->root_model_context) {
        struct recognition_context *current_model_context = stdev->root_model_context;
        while(current_model_context) {
            if (current_model_context->model_started) {
                started_model_found = true;
                break;
            }
            current_model_context = current_model_context->next;
        }
    }
    return started_model_found;
}

static int enable_vow_hw() {
    int status = 0;
    if (gAudioMTKHardware == NULL) {
        status = open_audio_hal();
        if (status == -ENOSYS) {
            ALOGE("%s(), dlopen audio hal error", __func__);
            return status;
        }
    }
    if (gAudioMTKHardware != NULL) {
        gAudioMTKHardware->setParameters(PARAMETER_MTK_VOW_ENABLE);
    }
    ALOGI("%s(), set enable VoW parameter done", __func__);
    return status;
}

static void disable_vow_hw() {
    if (gAudioMTKHardware != NULL) {
        gAudioMTKHardware->setParameters(PARAMETER_MTK_VOW_DISABLE);
    }

    ALOGI("%s(), start to call VOW_CHECK_STATUS", __func__);
    if (g_fd < 0) {
        g_fd = open("/dev/vow", O_RDONLY);
    }
    if (g_fd < 0) {
        ALOGE("%s(), open device fail!%s", __func__, strerror(errno));
    } else {
        ioctl(g_fd, VOW_CHECK_STATUS, 0);
    }
}

static int set_model_start(bool model_start, struct recognition_context *model_context) {
    int status = 0;

    if (g_fd < 0) {
        g_fd = open("/dev/vow", O_RDONLY);
    }
    if (g_fd < 0) {
        ALOGE("%s(), open device fail!%s", __func__, strerror(errno));
        status = -EBADF;
    } else {
        int ioctl_ret = 0;
        struct vow_model_start_t model_start_info;
        model_start_info.handle = model_context->model_handle;
        model_start_info.confidence_level = model_context->config->phrases[0].confidence_level;
        if (model_start) {
            ioctl_ret = ioctl(g_fd, VOW_MODEL_START, (unsigned long)&model_start_info);
        } else {
            ioctl_ret = ioctl(g_fd, VOW_MODEL_STOP, (unsigned long)&model_start_info);
        }

        if (ioctl_ret < 0) {
            ALOGE("%s(), IOCTL ERR, model_start = %d, ioctl_ret = %d",
                  __func__, model_start, ioctl_ret);
            status = -ECANCELED;
        }
    }

    ALOGI("%s(), model_start = %d, handle = %d, 1st level = %d, 2nd level = %d, status = %d",
          __func__,
          model_start,
          model_context->model_handle,
          model_context->config->phrases[0].confidence_level,
          model_context->config->phrases[0].levels[0].level,
          status);
    return status;
}

static int start_model(struct recognition_context *model_context) {
    return set_model_start(true, model_context);
}

static int stop_model(struct recognition_context *model_context) {
    return set_model_start(false, model_context);
}

static int mtk_load_sound_model(char *model_addr,
                                unsigned int model_size,
                                struct recognition_context *context) {
    ALOGI("%s()+", __func__);

    // init mtk model structure
    init_mtk_model_context(context);

    // init vow kernel control
    if (g_fd < 0) {
        g_fd = open("/dev/vow", O_RDONLY);
    }
    if (g_fd < 0) {
        ALOGE("%s(), open device fail!%s\n", __func__, strerror(errno));
        return -ENODEV;
    } else {
        if (ioctl(g_fd, VOW_SET_CONTROL, (unsigned long)VOWControlCmd_Init) < 0) {
            ALOGE("%s(), IOCTL VOW_SET_CONTROL ERR", __func__);
            return -ENOSYS;
        }
    }

    int vendor_id = get_model_vendor_id(context);
    ALOGV("%s(), Start Load init Model", __func__);
    if (vendor_id == VENDOR_ID_MTK) {
        //load init model
#if defined(MTK_VOW_SUPPORT)
        ParsedDataCollection parsed_model_data;
        ParsedDataSizes vow_rtn_parsed_model_info;
        FileData vow_model_info;
        char *phase_2_model_data = NULL;
        char *phase_2p5_model_data = NULL;

        vow_model_info.length = model_size;
        vow_model_info.beginPtr = model_addr;

        if (VOWE_AP_testing_getModelSize(&vow_rtn_parsed_model_info, vow_model_info) == vowe_bad) {
            ALOGE("%s(), [ASP API] getModelSize error", __func__);
            return -EINVAL;
        }

        ALOGI("%s(), AP ModelSize = %d, DSP ModelSize = %d",
              __func__,
              vow_rtn_parsed_model_info.apSize,
              vow_rtn_parsed_model_info.dspSize);

        if (vow_rtn_parsed_model_info.dspSize > ALLOCATE_MEMORY_SIZE_MAX) {
            ALOGE("%s(), the dsp memory size need to allocate is more than 64K!", __func__);
            return -EINVAL;
        }
        if (vow_rtn_parsed_model_info.dspSize > 0) {
            phase_2_model_data = new char[vow_rtn_parsed_model_info.dspSize];
            if (phase_2_model_data == NULL) {
                ALOGE("%s(), p2 allocate memory fail!", __func__);
                return -EINVAL;
            }
        }
        if (vow_rtn_parsed_model_info.apSize > 0) {
            phase_2p5_model_data = new char[vow_rtn_parsed_model_info.apSize];
            if (phase_2p5_model_data == NULL) {
                ALOGE("%s(), p25 + sid allocate memory fail!", __func__);
                return -EINVAL;
            }
            context->phase_2p5_model_data = phase_2p5_model_data;
        }
        if ((phase_2_model_data != NULL) || (phase_2p5_model_data != NULL)) {
            parsed_model_data.apData.length = vow_rtn_parsed_model_info.apSize;
            parsed_model_data.apData.beginPtr = phase_2p5_model_data;
            parsed_model_data.dspData.length = vow_rtn_parsed_model_info.dspSize;
            parsed_model_data.dspData.beginPtr = phase_2_model_data;
            ALOGV("%s(), AP = [%p]%d, DSP = [%p]%d",
                  __func__,
                  parsed_model_data.apData.beginPtr,
                  parsed_model_data.apData.length,
                  parsed_model_data.dspData.beginPtr,
                  parsed_model_data.dspData.length);
            if (VOWE_AP_testing_parseModel(&parsed_model_data, vow_model_info) == vowe_bad) {
                ALOGE("%s(), [ASP API] parseModel error", __func__);
                return -ENOSYS;
            }
        }
        if (phase_2_model_data != NULL) {
            if (g_fd > 0) {
                struct vow_model_info_t update_model_info;

                update_model_info.uuid = vendor_id;
                update_model_info.id = (long)context->model_handle;
                update_model_info.size = (long)parsed_model_data.dspData.length;
                update_model_info.addr = (long)parsed_model_data.dspData.beginPtr;
                if (ioctl(g_fd, VOW_SET_SPEAKER_MODEL, (unsigned long)&update_model_info) < 0) {
                    ALOGE("%s(), IOCTL VOW_SET_SPEAKER_MODEL ERR", __func__);
                    return -ENOSYS;
                }
            }
            delete[] phase_2_model_data;
        }
        if (phase_2p5_model_data != NULL) {
            ALOGI("%s(), [P2.5]Start Load speaker Model", __func__);
            /* Load Phase2.5 lib */
            VOWE_AP_testing_init_parameters testing_init_info;
            testing_init_info.micNumber   = 1;
            testing_init_info.refNumber   = 0;
            testing_init_info.frameLength = 400;
            testing_init_info.frameShift  = 160;
            testing_init_info.debugFolder = NULL;
            testing_init_info.apParsedModelData.length = parsed_model_data.apData.length;
            testing_init_info.apParsedModelData.beginPtr = parsed_model_data.apData.beginPtr;
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
            testing_init_info.is2MicMode  = 1;
#else
            //testing_init_info.is2MicMode  = 0;
#endif
            if (VOWE_AP_testing_init(&testing_init_info) == vowe_bad) {
                ALOGE("%s(), [ASP API] AP TestInit error", __func__);
                return -ENOSYS;
            }
        }
        /* reset counter */
        phase_2p5_entry_count = 0;
        phase_2p5_recog_ok_count = 0;
        voice_update_entry_count(VOW_PHASE2_CNT, phase_2p5_entry_count);
        voice_update_entry_count(VOW_PHASE3_CNT, phase_2p5_recog_ok_count);

        ALOGI("%s(), Load speaker Model Finish", __func__);
#endif  // #ifdef MTK_VOW_SUPPORT
    } else if (vendor_id == VENDOR_ID_AMAZON) {
        unsigned int first_stage_model_offset = 0;
        unsigned int first_stage_model_size = 0;
        unsigned int second_stage_model_offset = 0;
        unsigned int second_stage_model_size = 0;
        char HeaderArray[16];

        memcpy(HeaderArray, model_addr, 16);
        first_stage_model_offset  = (HeaderArray[0] << 24)  | (HeaderArray[1] << 16)  |
                                    (HeaderArray[2] << 8)   | HeaderArray[3];
        first_stage_model_size    = (HeaderArray[4] << 24)  | (HeaderArray[5] << 16)  |
                                    (HeaderArray[6] << 8)   | HeaderArray[7];
        second_stage_model_offset = (HeaderArray[8] << 24)  | (HeaderArray[9] << 16)  |
                                    (HeaderArray[10] << 8)  | HeaderArray[11];
        second_stage_model_size   = (HeaderArray[12] << 24) | (HeaderArray[13] << 16) |
                                    (HeaderArray[14] << 8)  | HeaderArray[15];
        ALOGI("1st model offset=0x%x, size=0x%x", first_stage_model_offset,
                                                  first_stage_model_size);
        ALOGI("2nd model offset=0x%x, size=0x%x", second_stage_model_offset,
                                                  second_stage_model_size);

        if (first_stage_model_size > 0) {
            if (g_fd > 0) {
                struct vow_model_info_t update_model_info;

                update_model_info.uuid = vendor_id;
                update_model_info.id   = (long)context->model_handle;
                update_model_info.size = (long)first_stage_model_size;
                update_model_info.addr = (long)(model_addr + first_stage_model_offset);
                if (ioctl(g_fd, VOW_SET_SPEAKER_MODEL, (unsigned long)&update_model_info) < 0) {
                    ALOGE("%s(), IOCTL VOW_SET_SPEAKER_MODEL ERR", __func__);
                    return -ENOSYS;
                }
            }
        }
#if defined(MTK_VOW_SUPPORT)
        ParsedDataCollection parsed_model_data;
        ParsedDataSizes vow_rtn_parsed_model_info;
        FileData vow_model_info;
        char *phase_2_model_data = NULL;
        char *phase_2p5_model_data = NULL;

        vow_model_info.length = second_stage_model_size;
        vow_model_info.beginPtr = model_addr + second_stage_model_offset;

        if (VOWE_AP_testing_getModelSize(&vow_rtn_parsed_model_info, vow_model_info) == vowe_bad) {
            ALOGI("%s(), [ASP API] getModelSize error", __func__);
            return -EINVAL;
        }

        ALOGI("%s(), AP ModelSize = %d, DSP ModelSize = %d",
              __func__,
              vow_rtn_parsed_model_info.apSize,
              vow_rtn_parsed_model_info.dspSize);

        if (vow_rtn_parsed_model_info.dspSize > ALLOCATE_MEMORY_SIZE_MAX) {
            ALOGE("%s(), the dsp memory size need to allocate is more than 64K!", __func__);
            return -EINVAL;
        }
        if (vow_rtn_parsed_model_info.dspSize > 0) {
            phase_2_model_data = new char[vow_rtn_parsed_model_info.dspSize];
            if (phase_2_model_data == NULL) {
                ALOGE("%s(), p2 allocate memory fail!", __func__);
                return -EINVAL;
            }
        }
        if (vow_rtn_parsed_model_info.apSize > 0) {
            phase_2p5_model_data = new char[vow_rtn_parsed_model_info.apSize];
            if (phase_2p5_model_data == NULL) {
                ALOGE("%s(), p25 + sid allocate memory fail!", __func__);
                return -EINVAL;
            }
            context->phase_2p5_model_data = phase_2p5_model_data;
        }
        if ((phase_2_model_data != NULL) || (phase_2p5_model_data != NULL)) {
            parsed_model_data.apData.length = vow_rtn_parsed_model_info.apSize;
            parsed_model_data.apData.beginPtr = phase_2p5_model_data;
            parsed_model_data.dspData.length = vow_rtn_parsed_model_info.dspSize;
            parsed_model_data.dspData.beginPtr = phase_2_model_data;
            ALOGV("%s(), AP = [%p]%d, DSP = [%p]%d",
                  __func__,
                  parsed_model_data.apData.beginPtr,
                  parsed_model_data.apData.length,
                  parsed_model_data.dspData.beginPtr,
                  parsed_model_data.dspData.length);
            if (VOWE_AP_testing_parseModel(&parsed_model_data, vow_model_info) == vowe_bad) {
                ALOGE("%s(), [ASP API] parseModel error", __func__);
                return -ENOSYS;
            }
        }
        /* no need to send to scp */
        if (phase_2_model_data != NULL) {
            delete[] phase_2_model_data;
        }
        if (phase_2p5_model_data != NULL) {
            ALOGI("%s(), [P2.5]Start Load speaker Model", __func__);
            /* Load Phase2.5 lib */
            VOWE_AP_testing_init_parameters testing_init_info;
            testing_init_info.micNumber   = 1;
            testing_init_info.refNumber   = 0;
            testing_init_info.frameLength = 400;
            testing_init_info.frameShift  = 160;
            testing_init_info.debugFolder = NULL;
            testing_init_info.apParsedModelData.length = parsed_model_data.apData.length;
            testing_init_info.apParsedModelData.beginPtr = parsed_model_data.apData.beginPtr;
#ifdef MTK_VOW_DUAL_MIC_SUPPORT
            testing_init_info.is2MicMode  = 1;
#else
            //testing_init_info.is2MicMode  = 0;
#endif
            if (VOWE_AP_testing_init(&testing_init_info) == vowe_bad) {
                ALOGE("%s(), [ASP API] AP TestInit error", __func__);
                return -ENOSYS;
            }
        }
        /* reset counter */
        phase_2p5_entry_count = 0;
        phase_2p5_recog_ok_count = 0;
        voice_update_entry_count(VOW_PHASE2_CNT, phase_2p5_entry_count);
        voice_update_entry_count(VOW_PHASE3_CNT, phase_2p5_recog_ok_count);

        ALOGI("%s(), Load speaker Model Finish", __func__);
#endif
    } else {
        // just send model data to DSP for default vendor
        if (g_fd > 0) {
            struct vow_model_info_t update_model_info;

            update_model_info.uuid  = vendor_id;
            update_model_info.id    = (long)context->model_handle;
            update_model_info.size  = (long)model_size;
            update_model_info.addr  = (long)model_addr;
            if (ioctl(g_fd, VOW_SET_SPEAKER_MODEL, (unsigned long)&update_model_info) < 0) {
                ALOGE("%s(), IOCTL VOW_SET_SPEAKER_MODEL ERR", __func__);
                return -ENOSYS;
            }
            ALOGI("%s(), Load 3rd mode Finish", __func__);
        }
    }
    ALOGI("%s()-", __func__);

    return 0;
}

static int mtk_unload_sound_model(struct recognition_context *model_context) {
    ALOGI("%s()+", __func__);
    if (g_fd < 0) {
        g_fd = open("/dev/vow", O_RDONLY);
    }
    if (g_fd < 0) {
        ALOGE("%s(), open device fail!%s", __func__, strerror(errno));
        return -ENODEV;
    } else {
        if (ioctl(g_fd, VOW_CLR_SPEAKER_MODEL, model_context->model_handle) < 0) {
            ALOGE("IOCTL VOW_CLR_SPEAKER_MODEL ERR");
            return -ENOSYS;
        }
    }
//#if defined(VOW_PHASE_2P5)
    // TODO: Get uuid by handle to check whether phase 2.5 model needs to be released
#if defined(MTK_VOW_SUPPORT)
    if (model_context->phase_2p5_model_data != NULL) {
        if (VOWE_AP_testing_release() == vowe_bad) {
            ALOGI("%s(), [ASP API] AP release error", __func__);
            return -ENOSYS;
        }
        if (model_context->phase_2p5_model_data != NULL) {
            ALOGI("%s(), Release P25 sid Model", __func__);
            delete[] model_context->phase_2p5_model_data;
            model_context->phase_2p5_model_data = NULL;
        }
    }
#endif  // #ifdef MTK_VOW_SUPPORT
//#endif
    ALOGI("%s()-", __func__);
    return 0;
}

static int mtk_start_recognition(const struct stub_sound_trigger_device *stdev,
                                 struct recognition_context *model_context) {
    ALOGI("%s(), model_context->model_handle = %d", __func__, model_context->model_handle);

    int status = 0;

    if (model_context->phase_2p5_is_running == true) {
        ALOGE("%s(), model_context->phase_2p5_is_running, return", __func__);
        return 0;
    }

    // enable VoW HW when the first recognition comes
    // started flag of each model is set to be true after mtk_start_recognition
    if (!started_model_exists(stdev)) {
        status = enable_vow_hw();
        if (status < 0) {
            ALOGE("%s(), enable_vow_hw fail!, status = %d", __func__, status);
            return status;
        }
    }

    // set model start
    status = start_model(model_context);

//#if defined(VOW_PHASE_2P5)
    // init phase 2.5 recognition
    // TODO: Get uuid by handle to check whether phase 2.5 model needs to be initialized @{
    // new phase 2.5 lock for synchronizion between recognition and data read thread
    // wrap by phase 2.5 option (MTK_VOW_SUPPORT?)?
    if (model_context->phase_2p5_read_lock == NULL) {
        NEW_ALOCK(model_context->phase_2p5_read_lock);
    }

#if defined(MTK_VOW_SUPPORT)
    // initialize recognition library
    if (model_context->phase_2p5_model_data != NULL) {
        int vendor_id;

        if (VOWE_AP_testing_setArgument(vowe_argid_printLog, 0) == vowe_bad) {
            ALOGE("%s(), [P2.5] SetArg printLog error", __func__);
            status = -ECANCELED;
        }
        ALOGD("%s(), [P2.5] Lib_version: %s", __func__, VOWE_AP_testing_version());
        vendor_id = get_model_vendor_id(model_context);
        if (vendor_id == VENDOR_ID_AMAZON) {
            float values[1];

            values[0] = (float)model_context->config->phrases[0].levels[0].level;
            ALOGD("%s(), [P2.5] SID Thd:%f", __func__, values[0]);
            if ((values[0] <= 100) && (values[0] >= 0)) {
                ALOGD("%s(), set SIDConfidenceThresholds", __func__);
                if (VOWE_AP_testing_setFloatArrayArgument(
                        vowe_argid_testSIDConfidenceThresholds, values, 1) == vowe_bad) {
                    ALOGE("%s(), [P2.5] SetArg Threshold error", __func__);
                }
                ALOGD("%s(), [P2.5] VOWE_AP_testing_reset", __func__);
                if (VOWE_AP_testing_reset() == vowe_bad) {  // reset to set SID confidence thd
                    ALOGI("%s(), [ASP API] reset error", __func__);
                }
            }
        }
    }
#endif  // #if MTK_VOW_SUPPORT
    // @}
//#endif
    return status;
}

static int mtk_stop_recognition(struct stub_sound_trigger_device *stdev,
                                sound_model_handle_t handle) {
    ALOGI("%s(), handle = %d", __func__, handle);

    int status = 0;
    int cnt = 0;
//#if defined(VOW_PHASE_2P5)
    struct recognition_context *model_context = fetch_model_with_handle(stdev, &handle);

    if (!model_context) {
        ALOGW("%s(), Can't find sound model handle %d in registered list", __func__, handle);
        return -ENOSYS;
    }

    if (model_context->phase_2p5_model_data != NULL) {
        // waiting for phase 2.5 threads stop done
        pthread_mutex_unlock(&stdev->lock);
        cnt = 0;
        while (model_context->is_data_read_thread_running) {
            usleep(500);
            cnt++;
            ASSERT(cnt < 2000);
        }
        pthread_mutex_lock(&stdev->lock);

        ALOGI("[P2.5] End, phase_2p5_entry_count: %d, phase_2p5_recog_ok_count: %d\n",
              phase_2p5_entry_count, phase_2p5_recog_ok_count);
        phase_2p5_entry_count = 0;
        phase_2p5_recog_ok_count = 0;
        voice_update_entry_count(VOW_PHASE2_CNT, phase_2p5_entry_count);
        voice_update_entry_count(VOW_PHASE3_CNT, phase_2p5_recog_ok_count);
    }
    if (model_context->phase_2p5_read_lock != NULL) {
        FREE_ALOCK(model_context->phase_2p5_read_lock);
        ALOGI("FREE ALOCK model_context->phase_2p5_read_lock = %p", model_context->phase_2p5_read_lock);
    }
//#endif

#ifdef VOW_DATA_READ_PCMFILE
    g_read_file_count = 0;
#endif
#ifdef VOW_RECOG_PCMFILE
    g_recogition_file_count = 0;
#endif

    // set model stop
    status = stop_model(model_context);

    // disnable VoW HW when the last recognition stops
    // started flag of each model is set to be false before mtk_stop_recognition
    if (!started_model_exists(stdev)) {
        disable_vow_hw();
    }

    return status;
}

static void mtk_close() {
    if (audio_hw_handle != NULL) {
        /* close dlopen */
        dlclose(audio_hw_handle);
        audio_hw_handle = NULL;
        gAudioMTKHardware = NULL;
    }
}

static int mtk_init_vow() {
    if (g_fd < 0) {
        g_fd = open("/dev/vow", O_RDONLY);
    }
    if (g_fd < 0) {
        ALOGE("%s(), open device fail!%s\n", __func__, strerror(errno));
        return -ENODEV;
    } else {
        if (ioctl(g_fd, VOW_SET_CONTROL, (unsigned long)VOWControlCmd_Reset) < 0) {
            ALOGE("%s(), IOCTL VOW_SET_CONTROL ERR", __func__);
            return -ENOSYS;
        }
    }
    return 0;
}
//>MTK add

static int stdev_get_properties(const struct sound_trigger_hw_device *dev,
                                struct sound_trigger_properties *properties) {
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    ALOGI("%s", __func__);
    if (properties == NULL)
        return -EINVAL;
    memcpy(properties, &hw_properties, sizeof(struct sound_trigger_properties));
    (void)stdev;
    return 0;
}

static int stdev_load_sound_model(const struct sound_trigger_hw_device *dev,
                                  struct sound_trigger_sound_model *sound_model,
                                  sound_model_callback_t callback,
                                  void *cookie,
                                  sound_model_handle_t *handle) {
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    ALOGI("%s stdev %p", __func__, stdev);
    int status = 0;
    pthread_mutex_lock(&stdev->lock);

    if (handle == NULL || sound_model == NULL) {
        pthread_mutex_unlock(&stdev->lock);
        return -EINVAL;
    }
    if (sound_model->data_size == 0 ||
            sound_model->data_offset < sizeof(struct sound_trigger_sound_model)) {
        pthread_mutex_unlock(&stdev->lock);
        return -EINVAL;
    }
    //<MTK add
    // for VTS random gerneric model test item
    if (sound_model->type == SOUND_MODEL_TYPE_GENERIC) {
        ALOGI("SOUND_MODEL_TYPE_GENERIC is not allow for soundtrigger HAL, return");
        pthread_mutex_unlock(&stdev->lock);
        return -EINVAL;
    }
    //>MTK add

    struct recognition_context *model_context;
    model_context = (struct recognition_context *)malloc(sizeof(struct recognition_context));
    if(!model_context) {
        ALOGW("Could not allocate recognition_context");
        pthread_mutex_unlock(&stdev->lock);
        return -ENOSYS;
    }

    // Add the new model context to the recognition_context linked list
    if (stdev->root_model_context) {
        // Find the tail
        struct recognition_context *current_model_context = stdev->root_model_context;
        unsigned int model_count = 0;
        while(current_model_context->next) {
            current_model_context = current_model_context->next;
            model_count++;
            if (model_count >= hw_properties.max_sound_models) {
                ALOGW("Can't load model: reached max sound model limit");
                free(model_context);
                pthread_mutex_unlock(&stdev->lock);
                return -ENOSYS;
            }
        }
        current_model_context->next = model_context;
    } else {
        stdev->root_model_context = model_context;
    }

    model_context->model_handle = generate_sound_model_handle(dev);
    *handle = model_context->model_handle;
    model_context->model_type = sound_model->type;

    char *data = (char *)sound_model + sound_model->data_offset;
    ALOGI("%s data size %d data %d - %d", __func__,
          sound_model->data_size, data[0], data[sound_model->data_size - 1]);
    model_context->model_uuid = sound_model->uuid;
    model_context->model_callback = callback;
    model_context->model_cookie = cookie;
    model_context->config = NULL;
    model_context->recognition_callback = NULL;
    model_context->recognition_cookie = NULL;
    model_context->next = NULL;
    model_context->model_started = false;
    model_context->model_state = false;
    ALOGI("Sound model loaded: Handle %d ", *handle);

    //<MTK add
    status = mtk_load_sound_model(data, sound_model->data_size, model_context);
    //>MTK add

    pthread_mutex_unlock(&stdev->lock);
    return status;
}

static void unload_all_sound_models(struct stub_sound_trigger_device *stdev) {
    ALOGI("%s", __func__);
    struct recognition_context *model_context = stdev->root_model_context;
    stdev->root_model_context = NULL;
    pthread_mutex_lock(&stdev->lock);
    while (model_context) {
        ALOGI("Deleting model with handle: %d", model_context->model_handle);
        //<MTK add
        mtk_unload_sound_model(model_context);
        //>MTK add
        struct recognition_context *temp = model_context;
        model_context = model_context->next;
        free(temp->config);
        free(temp);
    }
    pthread_mutex_unlock(&stdev->lock);
}

static int stdev_unload_sound_model(const struct sound_trigger_hw_device *dev,
                                    sound_model_handle_t handle) {
    // If recognizing, stop_recognition must be called for a sound model before unload_sound_model
    ALOGI("%s", __func__);
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    int status = 0;
    ALOGI("unload_sound_model:%d", handle);
    pthread_mutex_lock(&stdev->lock);

    struct recognition_context *model_context = NULL;
    struct recognition_context *previous_model_context = NULL;
    if (stdev->root_model_context) {
        struct recognition_context *current_model_context = stdev->root_model_context;
        while(current_model_context) {
            if (current_model_context->model_handle == handle) {
                model_context = current_model_context;
                break;
            }
            previous_model_context = current_model_context;
            current_model_context = current_model_context->next;
        }
    }
    if (!model_context) {
        ALOGW("Can't find sound model handle %d in registered list", handle);
        pthread_mutex_unlock(&stdev->lock);
        return -ENOSYS;
    }

    //<MTK add
    status = mtk_unload_sound_model(model_context);
    //>MTK add

    if (previous_model_context) {
        previous_model_context->next = model_context->next;
    } else {
        stdev->root_model_context = model_context->next;
    }
    free(model_context->config);
    free(model_context);

    pthread_mutex_unlock(&stdev->lock);
    return status;
}

static int stdev_start_recognition(const struct sound_trigger_hw_device *dev,
                                   sound_model_handle_t handle,
                                   const struct sound_trigger_recognition_config *config,
                                   recognition_callback_t callback,
                                   void *cookie) {
    ALOGI("%s", __func__);
    ALOGI("HAL_SOUND_TRIGGER_VER: %s", HAL_SOUND_TRIGGER_VER);
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    pthread_mutex_lock(&stdev->lock);

    /* If other models running with callbacks, don't start trigger thread */
    bool other_callbacks_found = recognition_callback_exists(stdev);
    if (other_callbacks_found == true) {
        ALOGI("other_callbacks_found exist!");
    }

    struct recognition_context *model_context = get_model_context(stdev, handle);
    if (!model_context) {
        ALOGW("Can't find sound model handle %d in registered list", handle);
        pthread_mutex_unlock(&stdev->lock);
        return -ENOSYS;
    }

    free(model_context->config);
    model_context->config = NULL;
    if (config) {
        model_context->config = (struct sound_trigger_recognition_config *)malloc(sizeof(*config));
        if (!model_context->config) {
            pthread_mutex_unlock(&stdev->lock);
            return -ENOMEM;
        }
        memcpy(model_context->config, config, sizeof(*config));
    }
    model_context->recognition_callback = callback;
    model_context->recognition_cookie = cookie;

    //<MTK add
    // print data
    if (config->data_size != 0) {
        char *data = (char *)config + config->data_offset;

        ALOGV("%s(), data_size = %d, data: %d - %d", __func__,
              config->data_size, data[0], data[config->data_size - 1]);
    }

    int status = mtk_start_recognition(stdev, model_context);
    if (status < 0) {
        ALOGE("%s(), mtk_start_recognition error, status = %d", __func__, status);
        return status;
    }
    //>MTK add

    model_context->model_started = true;
    model_context->model_state = true;

    pthread_mutex_unlock(&stdev->lock);
    ALOGI("%s done for handle %d", __func__, handle);
    return 0;
}

static int stdev_stop_recognition(const struct sound_trigger_hw_device *dev,
            sound_model_handle_t handle) {
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    ALOGI("%s", __func__);
    pthread_mutex_lock(&stdev->lock);

    struct recognition_context *model_context = get_model_context(stdev, handle);
    if (!model_context) {
        ALOGW("Can't find sound model handle %d in registered list", handle);
        pthread_mutex_unlock(&stdev->lock);
        return -ENOSYS;
    }

    if (model_context->config == NULL) {
        ALOGW("%s, double stop, skip this handle %d", __func__, handle);
        pthread_mutex_unlock(&stdev->lock);
        return 0;
    }

    model_context->model_started = false;
    model_context->model_state = false;

    //<MTK add
    mtk_stop_recognition(stdev, handle);
    //>MTK add

    free(model_context->config);
    model_context->config = NULL;
    model_context->recognition_callback = NULL;
    model_context->recognition_cookie = NULL;

    pthread_mutex_unlock(&stdev->lock);
    ALOGI("%s done for handle %d", __func__, handle);

    return 0;
}

#if 0
static int stdev_stop_all_recognitions(const struct sound_trigger_hw_device *dev) {
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    ALOGI("%s", __func__);
    pthread_mutex_lock(&stdev->lock);

    struct recognition_context *model_context = stdev->root_model_context;
    while (model_context) {
        model_context->model_started = false;
        model_context->model_state = false;

        //<MTK add
        // do not stop hardware, otherwise the voice data will not continuous
        //mtk_stop_recognition(stdev, model_context->model_handle);
        //>MTK add

        free(model_context->config);
        model_context->config = NULL;
        model_context->recognition_callback = NULL;
        model_context->recognition_cookie = NULL;

        ALOGI("%s stopped handle %d", __func__, model_context->model_handle);

        model_context = model_context->next;
    }

    pthread_mutex_unlock(&stdev->lock);

    return 0;
}
#endif

static int stdev_get_model_state(const struct sound_trigger_hw_device *dev,
                                 sound_model_handle_t handle) {
    int ret = 0;
    struct stub_sound_trigger_device *stdev = (struct stub_sound_trigger_device *)dev;
    ALOGI("%s", __func__);
    pthread_mutex_lock(&stdev->lock);

    struct recognition_context *model_context = get_model_context(stdev, handle);
    if (!model_context) {
        ALOGW("Can't find sound model handle %d in registered list", handle);
        ret = -ENOSYS;
        goto exit;
    }

    if (!model_context->model_started) {
        ALOGW("Sound model %d not started", handle);
        ret = -ENOSYS;
        goto exit;
    }

    if (model_context->recognition_callback == NULL) {
        ALOGW("Sound model %d not initialized", handle);
        ret = -ENOSYS;
        goto exit;
    }

    // trigger recognition event, set data if need
    send_event_with_handle(&model_context->model_handle,
            stdev,
            EVENT_RECOGNITION,
            RECOGNITION_STATUS_GET_STATE_RESPONSE);

exit:
    pthread_mutex_unlock(&stdev->lock);
    ALOGI("%s done for handle %d", __func__, handle);

    return ret;
}

__attribute__ ((visibility ("default")))
int sound_trigger_open_for_streaming() {
    int ret = 0;
    return ret;
}

__attribute__ ((visibility ("default")))
size_t sound_trigger_read_samples(int audio_handle, void *buffer, size_t  buffer_len) {
    (void)audio_handle;
    (void)buffer;
    (void)buffer_len;
    size_t ret = 0;
    return ret;
}

__attribute__ ((visibility ("default")))
int sound_trigger_close_for_streaming(int audio_handle __unused) {
    return 0;
}

static int stdev_close(hw_device_t *device) {
    // TODO: Implement the ability to stop the control thread. Since this is a
    // test hal, we have skipped implementing this for now. A possible method
    // would register a signal handler for the control thread so that any
    // blocking socket calls can be interrupted. We would send that signal here
    // to interrupt and quit the thread.
    free(device);
    mtk_close();
    return 0;
}

static int stdev_open(const hw_module_t* module, const char* name,
                     hw_device_t** device) {
    struct stub_sound_trigger_device *stdev;

    if (strcmp(name, SOUND_TRIGGER_HARDWARE_INTERFACE) != 0)
        return -EINVAL;

    stdev = (struct stub_sound_trigger_device *)calloc(1, sizeof(struct stub_sound_trigger_device));
    if (!stdev)
        return -ENOMEM;

    stdev->next_sound_model_id = 1;
    stdev->root_model_context = NULL;

    stdev->device.common.tag = HARDWARE_DEVICE_TAG;
    stdev->device.common.version = SOUND_TRIGGER_DEVICE_API_VERSION_1_1;
    stdev->device.common.module = (struct hw_module_t *) module;
    stdev->device.common.close = stdev_close;
    stdev->device.get_properties = stdev_get_properties;
    stdev->device.load_sound_model = stdev_load_sound_model;
    stdev->device.unload_sound_model = stdev_unload_sound_model;
    stdev->device.start_recognition = stdev_start_recognition;
    stdev->device.stop_recognition = stdev_stop_recognition;
    stdev->device.stop_all_recognitions = NULL;//stdev_stop_all_recognitions;
    stdev->device.get_model_state = stdev_get_model_state;

    pthread_mutex_init(&stdev->lock, (const pthread_mutexattr_t *) NULL);

    *device = &stdev->device.common;

    pthread_create(&stdev->control_thread, (const pthread_attr_t *) NULL,
                control_thread_loop, stdev);
    ALOGI("Starting control thread for the stub hal.");

    //<MTK Add
    mtk_init_vow();
    //>MTK Add
    return 0;
}

static struct hw_module_methods_t hal_module_methods = {
    .open = stdev_open,
};

struct sound_trigger_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = SOUND_TRIGGER_MODULE_API_VERSION_1_0,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = SOUND_TRIGGER_HARDWARE_MODULE_ID,
        .name = "Default sound trigger HAL",
        .author = "The Android Open Source Project",
        .methods = &hal_module_methods,
    },
};

