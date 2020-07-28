/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <cutils/log.h>

#include "bperf_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "bperf"
#define MAX_BPERF_EVENTS_IN_A_SECOND 500

static pthread_mutex_t event_data_lock;
static pthread_t bperf_thread_main;
static unsigned int bperf_main_thread_status;
static unsigned int bperf_main_thread_should_stop;
static unsigned int bperf_global_counter;
static uint8_t bperf_global_bitpool;
static uint8_t bperf_global_voble_codec; /* 0: ADPCM, 1:OPUS */
static unsigned char bperf_a2dp_string_interval[64];
static unsigned char bperf_a2dp_string_throughput[64];
static unsigned char bperf_a2dp_string_bitpool[64];

static struct bperf_event *bperf_event_voice, *bperf_event_voice_analysis;
static struct bperf_event *bperf_event_rc_fw_upgrade, *bperf_event_rc_fw_upgrade_analysis;
static struct bperf_event *bperf_event_hid, *bperf_event_hid_analysis;
static struct bperf_event *bperf_event_hid_cursor, *bperf_event_hid_cursor_analysis;
static struct bperf_event *bperf_event_hogp, *bperf_event_hogp_analysis;
static struct bperf_event *bperf_event_hogp_cursor, *bperf_event_hogp_cursor_analysis;
static struct bperf_event *bperf_event_a2dp, *bperf_event_a2dp_analysis;

static unsigned int _bperf_get_microseconds(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000000 + now.tv_usec);
}

static void _bperf_record_event(struct bperf_event* event, const uint8_t *buf, const unsigned int buf_len)
{
    int i;
    (void)buf;

    for ( i = 0 ; i < MAX_BPERF_EVENTS_IN_A_SECOND ; i++ )
    {
        if ( event[i].id == 0 && event[i].time == 0 )
        {
            pthread_mutex_lock(&event_data_lock);
            event[i].id = 1+i;
            event[i].time = _bperf_get_microseconds();
            event[i].buf_len = buf_len;
            pthread_mutex_unlock(&event_data_lock);
            break;
        }
    }
}

static void _bperf_mem_reset()
{
    if ( bperf_event_voice )
        memset(bperf_event_voice, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_rc_fw_upgrade )
        memset(bperf_event_rc_fw_upgrade, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid )
        memset(bperf_event_hid, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid_cursor )
        memset(bperf_event_hid_cursor, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp )
        memset(bperf_event_hogp, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp_cursor )
        memset(bperf_event_hogp_cursor, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_a2dp )
        memset(bperf_event_a2dp, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
}

static void _bperf_mem_reset_analysis()
{
    if ( bperf_event_voice_analysis )
        memset(bperf_event_voice_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_rc_fw_upgrade_analysis )
        memset(bperf_event_rc_fw_upgrade_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid_analysis )
        memset(bperf_event_hid_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid_cursor_analysis )
        memset(bperf_event_hid_cursor_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp_analysis )
        memset(bperf_event_hogp_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp_cursor_analysis )
        memset(bperf_event_hogp_cursor_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_a2dp_analysis )
        memset(bperf_event_a2dp_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
}

static void _bperf_mem_init()
{
    if ( bperf_event_voice == NULL )
        bperf_event_voice = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_rc_fw_upgrade == NULL )
        bperf_event_rc_fw_upgrade = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid == NULL )
        bperf_event_hid = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid_cursor == NULL )
        bperf_event_hid_cursor = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp == NULL )
        bperf_event_hogp = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp_cursor == NULL )
        bperf_event_hogp_cursor= malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_a2dp == NULL )
        bperf_event_a2dp = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);

    if ( bperf_event_voice_analysis == NULL )
        bperf_event_voice_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_rc_fw_upgrade_analysis == NULL )
        bperf_event_rc_fw_upgrade_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid_analysis == NULL )
        bperf_event_hid_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hid_cursor_analysis == NULL )
        bperf_event_hid_cursor_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp_analysis == NULL )
        bperf_event_hogp_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_hogp_cursor_analysis == NULL )
        bperf_event_hogp_cursor_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    if ( bperf_event_a2dp_analysis == NULL )
        bperf_event_a2dp_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);

    _bperf_mem_reset();
    _bperf_mem_reset_analysis();
}

static void _bperf_mem_free()
{
    if ( bperf_event_voice )
        free(bperf_event_voice);
    if ( bperf_event_rc_fw_upgrade )
        free(bperf_event_rc_fw_upgrade);
    if ( bperf_event_hid )
        free(bperf_event_hid);
    if ( bperf_event_hid_cursor )
        free(bperf_event_hid_cursor);
    if ( bperf_event_hogp )
        free(bperf_event_hogp);
    if ( bperf_event_hogp_cursor )
        free(bperf_event_hogp_cursor);
    if ( bperf_event_a2dp )
        free(bperf_event_a2dp);

    if ( bperf_event_voice_analysis )
        free(bperf_event_voice_analysis);
    if ( bperf_event_rc_fw_upgrade_analysis )
        free(bperf_event_rc_fw_upgrade_analysis);
    if ( bperf_event_hid_analysis )
        free(bperf_event_hid_analysis);
    if ( bperf_event_hid_cursor_analysis )
        free(bperf_event_hid_cursor_analysis);
    if ( bperf_event_hogp_analysis )
        free(bperf_event_hogp_analysis);
    if ( bperf_event_hogp_cursor_analysis )
        free(bperf_event_hogp_cursor_analysis);
    if ( bperf_event_a2dp_analysis )
        free(bperf_event_a2dp_analysis);
}

static void _bperf_mem_copy()
{
    memcpy(bperf_event_voice_analysis, bperf_event_voice, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    memcpy(bperf_event_rc_fw_upgrade_analysis, bperf_event_rc_fw_upgrade, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    memcpy(bperf_event_hid_analysis, bperf_event_hid, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    memcpy(bperf_event_hid_cursor_analysis, bperf_event_hid_cursor, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    memcpy(bperf_event_hogp_analysis, bperf_event_hogp, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    memcpy(bperf_event_hogp_cursor_analysis, bperf_event_hogp_cursor, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
    memcpy(bperf_event_a2dp_analysis, bperf_event_a2dp, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
}

static void _bperf_mem_analysis_rc_fw_upgrade(struct bperf_event *bperf_event)
{
    if ( bperf_event )
    {
        int i;
        int event_counter=0;
        int event_len_total=0;
        for ( i = 0 ; i < MAX_BPERF_EVENTS_IN_A_SECOND ; i++ )
        {
            if ( bperf_event[i].id )
            {
                event_counter++;
                event_len_total += (bperf_event[i].buf_len - 4); /* 4 byte hci header should be removed */
            }
            else
            {
                break;
            }
        }

        if ( i > 0 )
        {
            LOG_PER(LIGHT_BLUE "[bperf](%d) RC FW Upgrade Num(%d), Throughput" GREEN "(%d bps)" NONECOLOR,
                                bperf_global_counter, event_counter, (event_len_total<<3));
        }
    }
}

static void _bperf_mem_analysis_voice(struct bperf_event *bperf_event)
{
    if ( bperf_event )
    {
        int i;
        int event_counter=0;
        int event_len_total=0;
        int event_len_voice=0;
        for ( i = 0 ; i < MAX_BPERF_EVENTS_IN_A_SECOND ; i++ )
        {
            if ( bperf_event[i].id )
            {
                event_counter++;
                /* Voice data packet contains 4 bytes hci header, 4 bytes l2cap header, 3 bytes ATT header, 1 bytes rc header, 19 bytes audio data */
                event_len_total += (bperf_event[i].buf_len - 4);
                event_len_voice += (bperf_event[i].buf_len - 11);
            }
            else
            {
                break;
            }
        }

        if ( i > 0 )
        {
            if (bperf_global_voble_codec == 0)  /* codec : ADPCM */
            {
                /* Normal ADPCM voice data rate is abount 66,400 bps. */
                if ( (event_len_voice<<3) < 60000 )
                    LOG_PER(LIGHT_BLUE "[bperf](%d) VOICE Num(%d), Throughput" LIGHT_RED "(%d bps)" LIGHT_BLUE ", Voice Data Rate" LIGHT_RED "(%d bps)" NONECOLOR,
                                    bperf_global_counter, event_counter, (event_len_total<<3), (event_len_voice<<3));
                else
                    LOG_PER(LIGHT_BLUE "[bperf](%d) VOICE Num(%d), Throughput" GREEN "(%d bps)" LIGHT_BLUE ", Voice Data Rate" GREEN "(%d bps)" NONECOLOR,
                                    bperf_global_counter, event_counter, (event_len_total<<3), (event_len_voice<<3));
            }
            else    /* codec : OPUS */
            {
                /* Normal ADPCM voice data rate is abount 16,800 bps. */
                if ( (event_len_voice<<3) < 15000 )
                    LOG_PER(LIGHT_BLUE "[bperf](%d) VOICE Num(%d), Throughput" LIGHT_RED "(%d bps)" LIGHT_BLUE ", Voice Data Rate" LIGHT_RED "(%d bps)" NONECOLOR,
                                    bperf_global_counter, event_counter, (event_len_total<<3), (event_len_voice<<3));
                else
                    LOG_PER(LIGHT_BLUE "[bperf](%d) VOICE Num(%d), Throughput" GREEN "(%d bps)" LIGHT_BLUE ", Voice Data Rate" GREEN "(%d bps)" NONECOLOR,
                                    bperf_global_counter, event_counter, (event_len_total<<3), (event_len_voice<<3));
            }
        }
    }
}

static void _bperf_mem_analysis_hid_hogp(struct bperf_event *bperf_event)
{
    if ( bperf_event )
    {
        int i;
        int event_counter=0;
        for ( i = 0 ; i < MAX_BPERF_EVENTS_IN_A_SECOND ; i++ )
        {
            if ( bperf_event[i].id )
            {
                event_counter++;
            }
            else
            {
                break;
            }
        }

        if ( i > 0 )
        {
            if ( bperf_event == bperf_event_hid_analysis )
            {
                LOG_PER(LIGHT_WHITE "[bperf](%d) HID Num(%d)" NONECOLOR, bperf_global_counter, event_counter);
            }
            else if ( bperf_event == bperf_event_hid_cursor_analysis )
            {
                LOG_PER(LIGHT_CYAN"[bperf](%d) HID_Cursor Num(%d)" NONECOLOR, bperf_global_counter, event_counter);
            }
            else if ( bperf_event == bperf_event_hogp_analysis )
            {
                LOG_PER(LIGHT_WHITE "[bperf](%d) HOGP Num(%d)" NONECOLOR, bperf_global_counter, event_counter);
            }
            else if ( bperf_event == bperf_event_hogp_cursor_analysis )
            {
                LOG_PER(LIGHT_CYAN "[bperf](%d) HOGP_Cursor Num(%d)" NONECOLOR, bperf_global_counter, event_counter);
            }
        }
    }
}

static void _bperf_mem_analysis_a2dp(struct bperf_event *bperf_event)
{
    if ( bperf_event )
    {
        int i;
        int event_counter=0;
        int delta_time_max=0;
        int delta_time_min=0;
        int delta_time_average=0;
        int delta_time=0;
        int event_len_total=0;

        for ( i = 0 ; i < MAX_BPERF_EVENTS_IN_A_SECOND ; i++ )
        {
            if ( bperf_event[i].id )
            {
                event_counter++;
                /* A2DP data packet contains 4byte HCI header, 4 byte L2CAP header, 12 byte AVDTP header*/
                event_len_total += (bperf_event[i].buf_len - 20);
                if ( i == 0 )
                {
                    delta_time_max = delta_time_average = 0;
                    delta_time_min = 999999999;
                }
                else
                {
                    if ( bperf_event[i].time > bperf_event[i-1].time )
                        delta_time = bperf_event[i].time - bperf_event[i-1].time;
                    else
                        delta_time = 0;

                    if ( delta_time > delta_time_max )
                        delta_time_max = delta_time;
                    if ( delta_time < delta_time_min )
                        delta_time_min = delta_time;
                    delta_time_average += delta_time;
                }
            }
            else
            {
                break;
            }
        }

        if ( i > 0 )
        {
            delta_time_average = delta_time_average/i;
            memset(bperf_a2dp_string_interval, 0, sizeof(bperf_a2dp_string_interval));
            memset(bperf_a2dp_string_throughput, 0, sizeof(bperf_a2dp_string_throughput));
            memset(bperf_a2dp_string_bitpool, 0, sizeof(bperf_a2dp_string_bitpool));

            if (bperf_global_bitpool==53)
            {
                snprintf((char*)bperf_a2dp_string_bitpool, sizeof(bperf_a2dp_string_bitpool), "%sBitpool%s(%d)%s", LIGHT_PURPLE, GREEN, bperf_global_bitpool, NONECOLOR);

                if (delta_time_max > 150000)
                    snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%d us)%s", LIGHT_PURPLE, LIGHT_RED, delta_time_max, NONECOLOR);
                else if (delta_time_max <= 150000 && delta_time_max > 100000)
                    snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%d us)%s", LIGHT_PURPLE, YELLOW, delta_time_max, NONECOLOR);
                else
                    snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%d us)%s", LIGHT_PURPLE, GREEN, delta_time_max, NONECOLOR);

                /* 90% : 328*1024*0.9  = 302285 */
                /* 85% : 328*1024*0.85 = 285491 */
                if ((event_len_total<<3) < 285491)
                    snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d bps)%s", LIGHT_PURPLE, LIGHT_RED, (event_len_total<<3), NONECOLOR);
                else if ((event_len_total<<3) >= 285491 && (event_len_total<<3) < 302285)
                    snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d bps)%s", LIGHT_PURPLE, YELLOW, (event_len_total<<3), NONECOLOR);
                else
                    snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d bps)%s", LIGHT_PURPLE, GREEN, (event_len_total<<3), NONECOLOR);
            }
            else
            {
                snprintf((char*)bperf_a2dp_string_bitpool, sizeof(bperf_a2dp_string_bitpool), "%sBitpool%s(%d)%s", LIGHT_PURPLE, YELLOW, bperf_global_bitpool, NONECOLOR);
                snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%d us)%s", LIGHT_PURPLE, LIGHT_WHITE, delta_time_max, NONECOLOR);
                snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d bps)%s", LIGHT_PURPLE, LIGHT_WHITE, (event_len_total<<3), NONECOLOR);
            }

            LOG_PER(LIGHT_PURPLE "[bperf](%d) A2DP Num(%d), %s, %s, %s" NONECOLOR,
                  bperf_global_counter, event_counter, bperf_a2dp_string_bitpool, bperf_a2dp_string_interval, bperf_a2dp_string_throughput);
        }
    }
}
static void _bperf_mem_analysis()
{
    if ( bperf_event_a2dp_analysis )
    {
        _bperf_mem_analysis_a2dp(bperf_event_a2dp_analysis);
    }

    if ( bperf_event_voice_analysis )
    {
        _bperf_mem_analysis_voice(bperf_event_voice_analysis);
    }

    if ( bperf_event_rc_fw_upgrade_analysis )
    {
        _bperf_mem_analysis_rc_fw_upgrade(bperf_event_rc_fw_upgrade_analysis);
    }

    if ( bperf_event_hid_analysis )
    {
        _bperf_mem_analysis_hid_hogp(bperf_event_hid_analysis);
    }

    if ( bperf_event_hid_cursor_analysis )
    {
        _bperf_mem_analysis_hid_hogp(bperf_event_hid_cursor_analysis);
    }

    if ( bperf_event_hogp_analysis )
    {
        _bperf_mem_analysis_hid_hogp(bperf_event_hogp_analysis);
    }

    if ( bperf_event_hogp_cursor_analysis )
    {
        _bperf_mem_analysis_hid_hogp(bperf_event_hogp_cursor_analysis);
    }
}

static void *_bperf_thread_main(void *arg)
{
    unsigned int time_begin;
    unsigned int time_end;
    unsigned int time_sleep;
    (void)arg;
    LOG_DBG("Thread Started");

    if ( !bperf_event_voice || !bperf_event_voice_analysis ||
         !bperf_event_rc_fw_upgrade || !bperf_event_rc_fw_upgrade_analysis ||
         !bperf_event_hid || !bperf_event_hid_analysis ||
         !bperf_event_hid_cursor || !bperf_event_hid_cursor_analysis ||
         !bperf_event_hogp || !bperf_event_hogp_analysis ||
         !bperf_event_hogp_cursor || !bperf_event_hogp_cursor_analysis ||
         !bperf_event_a2dp || !bperf_event_a2dp_analysis )
    {
        LOG_ERR("Allocate memory failed!");
        LOG_DBG("Thread Stopped");
        return 0;
    }

    bperf_main_thread_status = BPERF_STATE_THREAD_RUNNING;
    while(!bperf_main_thread_should_stop)
    {
        time_begin = _bperf_get_microseconds();

        pthread_mutex_lock(&event_data_lock);
        _bperf_mem_copy();
        _bperf_mem_reset();
        pthread_mutex_unlock(&event_data_lock);

        _bperf_mem_analysis();
        _bperf_mem_reset_analysis();
        time_end = _bperf_get_microseconds();
        time_sleep = 1000000-(time_end-time_begin);
        usleep(time_sleep);
        bperf_global_counter++;
    }

    bperf_main_thread_should_stop = 0;
    bperf_main_thread_status = BPERF_STATE_THREAD_STOPPED;
    LOG_DBG("Thread Stopped");
    return 0;
}

static void _bperf_thread_start()
{
    if ( bperf_main_thread_status != BPERF_STATE_THREAD_RUNNING && !bperf_main_thread_should_stop)
    {
        LOG_DBG("Create thread");
        pthread_create(&bperf_thread_main, NULL, _bperf_thread_main, NULL);
    }
}

static void _bperf_thread_stop()
{
    bperf_main_thread_should_stop = 1;
}

void bperf_notify_data(const uint8_t *buf, const unsigned int buf_len)
{
    if ( bperf_main_thread_status == BPERF_STATE_THREAD_RUNNING && !bperf_main_thread_should_stop )
    {
        /* HT RC Voice Search (2541) */
        if ( (buf_len == 12 || buf_len == 31) &&
             (buf[2] == 0x08 || buf[2] == 0x1b) && buf[3] == 0x00 &&
             buf[8] == 0x1b && buf[9] == 0x35 && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_record_event(bperf_event_voice, buf, buf_len);
        }
        /* HT RC Voice Search (2640) */
        else if ( buf_len == 31 &&
                  buf[2] == 0x1b && buf[3] == 0x00 &&
                  buf[8] == 0x1b && buf[9] == 0x3f && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_record_event(bperf_event_voice, buf, buf_len);
        }
        /* HT RC Voice Search (2640)(BLE Data Length Extension) */
        else if ( buf_len == 111 &&
                  buf[2] == 0x6b && buf[3] == 0x00 &&
                  buf[8] == 0x1b && buf[9] == 0x3f && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_record_event(bperf_event_voice, buf, buf_len);
        }
        /* Airoha Voice Search */
        else if ( buf_len == 31 &&
                  buf[2] == 0x1b && buf[3] == 0x00 &&
                  buf[8] == 0x1b && buf[9] == 0x29 && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_record_event(bperf_event_voice, buf, buf_len);
        }
        /* Nordic Voice Search, Turnkey */
        else if ( buf[3] == 0x00 &&
                  buf[8] == 0x1b && buf[9] == 0x1d && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 1;
            _bperf_record_event(bperf_event_voice, buf, buf_len);
        }
        /* Nordic Voice Search, Huitong*/
        else if ( buf[3] == 0x00 &&
                  buf[8] == 0x1b && buf[9] == 0x23 && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 1;
            _bperf_record_event(bperf_event_voice, buf, buf_len);
        }
        /* HT RC Button */
        else if ( buf_len == 12 &&
                  buf[2] == 0x08 && buf[3] == 0x00 &&
                  buf[4] == 0x04 && buf[5] == 0x00 && buf[8] == 0x1b )
        {
            _bperf_record_event(bperf_event_hogp, buf, buf_len);
        }
        /* HID : Logitech Keyboard*/
        else if ( (buf_len == 18) &&
                  (buf[2] == 0x0e && buf[3] == 0x00) &&
                  (buf[4] == 0x0a && buf[5] == 0x00) &&
                  (buf[8] == 0xa1) )
        {
            _bperf_record_event(bperf_event_hid, buf, buf_len);
        }
        /* HID : Microsoft Designer BLE Mouse*/
        else if ( (buf_len == 20) &&
                  (buf[2] == 0x10 && buf[3] == 0x00) &&
                  (buf[4] == 0x0c && buf[5] == 0x00) &&
                  (buf[8] == 0x1b) )
        {
            _bperf_record_event(bperf_event_hogp, buf, buf_len);
        }
        /* HID : Elecom BLE Mouse */
        else if ( (buf_len == 17) &&
                  (buf[2] == 0x0d && buf[3] == 0x00) &&
                  (buf[4] == 0x09 && buf[5] == 0x00) &&
                  (buf[8] == 0x1b) )
        {
            _bperf_record_event(bperf_event_hogp, buf, buf_len);
        }
        /* HID : Logitech M557 */
        else if ( (buf_len == 16) &&
                  (buf[2] == 0x0c && buf[3] == 0x00) &&
                  (buf[4] == 0x08 && buf[5] == 0x00) &&
                  (buf[8] == 0xa1) )
        {
            _bperf_record_event(bperf_event_hid, buf, buf_len);
        }
        /* HID : DS4 */
        else if ( (buf_len == 19) &&
                  (buf[2] == 0x0f && buf[3] == 0x00) &&
                  (buf[4] == 0x0b && buf[5] == 0x00) &&
                  (buf[8] == 0xa1) )
        {
            _bperf_record_event(bperf_event_hid, buf, buf_len);
        }
        /* A2DP Sink */
        else if ( buf[8] == 0x80 && buf[9] == 0x60 )
        {
            _bperf_record_event(bperf_event_a2dp, buf, buf_len);
            if ( buf[20] == 0x00 ) /* SCMS-T */
                bperf_global_bitpool = buf[24];
            else
                bperf_global_bitpool = buf[23];
        }
        /* A2DP Src */
        else if ( buf_len == 587 &&
             buf[8] == 0x80 && buf[9] == 0x60 &&
             buf[20] == 0x00 )
        {
            _bperf_record_event(bperf_event_a2dp, buf, buf_len);
            bperf_global_bitpool = buf[24];
        }
        /* HT RC FW Upgrade */
        else if ( (buf_len == 29) &&
                  (buf[2] == 0x19 && buf[3] == 0x00) &&
                  (buf[4] == 0x15 && buf[5] == 0x00) &&
                  (buf[9] == 0x48 && buf[10] == 0x00) )
        {
            _bperf_record_event(bperf_event_rc_fw_upgrade, buf, buf_len);
        }
    }
}

void bperf_init()
{
    bperf_global_counter = 0;
    pthread_mutex_init(&event_data_lock, NULL);
    _bperf_mem_init();
    _bperf_thread_start();
    return;
}

void bperf_uninit()
{
    _bperf_thread_stop();
    _bperf_mem_free();
    pthread_mutex_destroy(&event_data_lock);
    return;
}
