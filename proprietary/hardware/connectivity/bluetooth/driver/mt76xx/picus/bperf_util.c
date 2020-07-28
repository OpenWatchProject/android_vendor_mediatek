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
#include <sys/time.h>
#include <pthread.h>
#include "bperf_util.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define BPERF_LIBRARY_VERSION "700.0.17071101"
#define LOG_TAG "bperf"
#define MAX_BPERF_EVENTS_IN_A_SECOND 500
#define MAX_BPERF_LE_SCAN_ENTRY 64
#define MAX_BPERF_BT_SCAN_ENTRY 64
#define MAX_PROP_BT_SCAN_ENTRY 16
#define MAX_PROP_NAME 32

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

static unsigned int bperf_average_accumulate;
static unsigned int bperf_average_total_time;

static int event_len_summary_rc_fw_upgrade = 0;
static int event_len_summary_voice = 0;
static int event_len_summary_voice_drop = 0;
static int event_len_summary_a2dp = 0;
static int event_len_summary_a2dp_glitch_warning = 0;
static int event_counter_summary_hid = 0;
static int event_counter_summary_hid_cursor = 0;
static int event_counter_summary_hogp = 0;
static int event_counter_summary_hogp_cursor = 0;
static int event_counter_summary_hid_delta_time_max = 0;
static int event_counter_summary_hid_cursor_delta_time_max = 0;
static int event_counter_summary_hogp_delta_time_max = 0;
static int event_counter_summary_hogp_cursor_delta_time_max = 0;
static int event_counter_summary_ble_adv = 0;

static struct bperf_event *bperf_event_voice, *bperf_event_voice_analysis;
static struct bperf_event *bperf_event_rc_fw_upgrade, *bperf_event_rc_fw_upgrade_analysis;
static struct bperf_event *bperf_event_hid, *bperf_event_hid_analysis;
static struct bperf_event *bperf_event_hid_cursor, *bperf_event_hid_cursor_analysis;
static struct bperf_event *bperf_event_hogp, *bperf_event_hogp_analysis;
static struct bperf_event *bperf_event_hogp_cursor, *bperf_event_hogp_cursor_analysis;
static struct bperf_event *bperf_event_a2dp, *bperf_event_a2dp_analysis;
static struct bperf_event *bperf_event_ble_adv, *bperf_event_ble_adv_analysis;

static unsigned int _bperf_get_microseconds(void)
{
    struct timeval now;
    gettimeofday(&now, NULL);
    return (now.tv_sec * 1000000 + now.tv_usec);
}

static void _bperf_mem_record_event(struct bperf_event* event, const uint8_t *buf, const unsigned int buf_len)
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

            /* HT RC Voice Search (2640)(BLE Data Length Extension) */
            if ( buf_len == 111 && buf[2] == 0x6b && buf[3] == 0x00 && buf[8] == 0x1b && buf[9] == 0x3f && buf[10] == 0x00 )
            {
                event[i].extra_info = ((buf[11] >>3 )& 0x1F);
            }
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
    if ( bperf_event_ble_adv )
        memset(bperf_event_ble_adv, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
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
    if ( bperf_event_ble_adv_analysis )
        memset(bperf_event_ble_adv_analysis, 0, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
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
    if ( bperf_event_ble_adv == NULL )
        bperf_event_ble_adv = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);

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
    if ( bperf_event_ble_adv_analysis == NULL )
        bperf_event_ble_adv_analysis = malloc(sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);

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
    if ( bperf_event_ble_adv )
        free(bperf_event_ble_adv);

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
    if ( bperf_event_ble_adv_analysis )
        free(bperf_event_ble_adv_analysis);
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
    memcpy(bperf_event_ble_adv_analysis, bperf_event_ble_adv, sizeof(struct bperf_event)*MAX_BPERF_EVENTS_IN_A_SECOND);
}

static void _bperf_analysis_rc_fw_upgrade(struct bperf_event *bperf_event)
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
            if ( bperf_average_accumulate == 1 )
            {
                event_len_summary_rc_fw_upgrade += event_len_total;
            }

            printf("[bperf](%d) RC FW Upgrade Num(%d), Throughput (%d bps)\n", bperf_global_counter, event_counter, (event_len_total<<3));
        }
    }
}

static void _bperf_analysis_voice(struct bperf_event *bperf_event)
{
    if ( bperf_event )
    {
        int i;
        int event_counter = 0;
        int event_len_total = 0;
        int event_len_voice = 0;
        int event_counter_packet_drop = 0;
        int event_counter_packet_not_in_time = 0;
        int event_delta_time = 0;
        int event_max_delta_time = 0;
        static int latest_voice_data_seq = 0;
        static int latest_voice_data_timestamp = 0;

        for ( i = 0 ; i < MAX_BPERF_EVENTS_IN_A_SECOND ; i++ )
        {
            if ( bperf_event[i].id )
            {
                event_counter++;

                /* Voice data packet contains 4 bytes hci header, 4 bytes l2cap header, 3 bytes ATT header, 1 bytes rc header, 19 bytes audio data */
                event_len_total += (bperf_event[i].buf_len - 4);
                event_len_voice += (bperf_event[i].buf_len - 11);

                if ( i > 0 )
                {
                    /* check sequence num */
                    if ( bperf_event[i].extra_info != 0 && bperf_event[i].extra_info - bperf_event[i-1].extra_info != 1 )
                    {
                        event_counter_packet_drop++;
                    }
                    else if ( bperf_event[i].extra_info == 0 && bperf_event[i-1].extra_info != 0 && bperf_event[i-1].extra_info != 31 )
                    {
                        event_counter_packet_drop++;
                    }

                    /* check delta time */
                    event_delta_time = bperf_event[i].time - bperf_event[i-1].time;
                    if ( event_delta_time > 30000 )
                    {
                        event_counter_packet_not_in_time++;
                    }

                    latest_voice_data_seq = bperf_event[i].extra_info;
                    latest_voice_data_timestamp = bperf_event[i].time;
                }
                else if ( i == 0 )
                {
                    /* check sequence num */
                    if ( bperf_event[i].extra_info != 0 && bperf_event[i].extra_info - latest_voice_data_seq != 1 )
                    {
                        event_counter_packet_drop++;
                    }
                    else if ( bperf_event[i].extra_info == 0 && latest_voice_data_seq != 0 && latest_voice_data_seq != 31 )
                    {
                        event_counter_packet_drop++;
                    }

                    /* check delta time */
                    event_delta_time = bperf_event[i].time - latest_voice_data_timestamp;
                    if ( bperf_event[i].time - latest_voice_data_timestamp > 30000 )
                    {
                        event_counter_packet_not_in_time++;
                    }
                    event_max_delta_time = event_delta_time;
                }

                /* check max delta time */
                if ( event_delta_time > event_max_delta_time )
                {
                    event_max_delta_time = event_delta_time;
                }
            }
            else
            {
                break;
            }
        }

        if ( i > 0 )
        {
            if ( bperf_average_accumulate == 1 )
            {
                event_len_summary_voice += event_len_voice;
                event_len_summary_voice_drop += event_counter_packet_drop;
            }

            if (bperf_global_voble_codec == 0)  /* codec : ADPCM */
            {
                /* Normal ADPCM voice data rate is abount 66,400 bps. */
                printf("[bperf](%d) VOICE Num(%d), " LIGHT_BLUE ", Voice Data Rate%s(%d bps), Packet Drop%s(%d), Not in time%s(%d), Max Latency%s(%dms)\n",
                                bperf_global_counter,
                                event_counter,
                                (event_len_voice<<3)<60000?LIGHT_RED:GREEN,
                                (event_len_voice<<3),
                                event_counter_packet_drop>0?LIGHT_RED:GREEN,
                                event_counter_packet_drop,
                                event_counter_packet_not_in_time>0?YELLOW:GREEN,
                                event_counter_packet_not_in_time,
                                event_max_delta_time>30000?YELLOW:GREEN,
                                event_max_delta_time/1000);
            }
            else    /* codec : OPUS */
            {
                /* Normal ADPCM voice data rate is abount 16,800 bps. */
                printf("[bperf](%d) VOICE Num(%d), Voice Data Rate %s(%d bps)\n",
                                bperf_global_counter,
                                event_counter,
                                (event_len_voice<<3)<1500?LIGHT_RED:GREEN,
                                (event_len_voice<<3));
            }
        }
    }
}

static void _bperf_analysis_hid_hogp(struct bperf_event *bperf_event)
{
    if ( bperf_event )
    {
        int i;
        int event_counter=0;
        int delta_time=0;
        int delta_time_max=0;

        for ( i = 0 ; i < MAX_BPERF_EVENTS_IN_A_SECOND ; i++ )
        {
            if ( bperf_event[i].id )
            {
                event_counter++;
                if ( i == 0 )
                {
                    delta_time = 0;
                    delta_time_max = 0;
                }
                else
                {
                    delta_time = bperf_event[i].time - bperf_event[i-1].time;
                    if ( delta_time > delta_time_max )
                        delta_time_max = delta_time;
                }
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
                if ( bperf_average_accumulate == 1 )
                {
                    event_counter_summary_hid += event_counter;
                    if ( delta_time_max > event_counter_summary_hid_delta_time_max )
                        event_counter_summary_hid_delta_time_max = delta_time_max;
                }

                printf("[bperf](%d) HID Num(%d) Max_Delta_Time(%dms)\n"
                                ,bperf_global_counter, event_counter, delta_time_max/1000);
            }
            else if ( bperf_event == bperf_event_hid_cursor_analysis )
            {
                if ( bperf_average_accumulate == 1 )
                {
                    event_counter_summary_hid_cursor += event_counter;
                    if ( delta_time_max > event_counter_summary_hid_cursor_delta_time_max )
                        event_counter_summary_hid_cursor_delta_time_max = delta_time_max;
                }

                printf("[bperf](%d) HID_Cursor Num(%d) Max_Delta_Time(%dms)\n"
                                ,bperf_global_counter, event_counter, delta_time_max/1000);
            }
            else if ( bperf_event == bperf_event_hogp_analysis )
            {
                if ( bperf_average_accumulate == 1 )
                {
                    event_counter_summary_hogp += event_counter;
                    if ( delta_time_max > event_counter_summary_hogp_delta_time_max )
                        event_counter_summary_hogp_delta_time_max = delta_time_max;
                }

                printf("[bperf](%d) HOGP Num(%d) Max_Delta_Time(%dms)\n",
                                bperf_global_counter, event_counter, delta_time_max/1000);
            }
            else if ( bperf_event == bperf_event_hogp_cursor_analysis )
            {
                if ( bperf_average_accumulate == 1 )
                {
                    event_counter_summary_hogp_cursor += event_counter;
                    if ( delta_time_max > event_counter_summary_hogp_cursor_delta_time_max )
                        event_counter_summary_hogp_cursor_delta_time_max = delta_time_max;
                }

                printf("[bperf](%d) HOGP_Cursor Num(%d) Max_Delta_Time(%dms)\n",
                                bperf_global_counter, event_counter, delta_time_max/1000);
            }
        }
    }
}

static void _bperf_analysis_a2dp(struct bperf_event *bperf_event)
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
            if ( bperf_average_accumulate == 1 )
            {
                event_len_summary_a2dp += event_len_total;
            }

            delta_time_average = delta_time_average/i;
            memset(bperf_a2dp_string_interval, 0, sizeof(bperf_a2dp_string_interval));
            memset(bperf_a2dp_string_throughput, 0, sizeof(bperf_a2dp_string_throughput));
            memset(bperf_a2dp_string_bitpool, 0, sizeof(bperf_a2dp_string_bitpool));

            if (bperf_global_bitpool==53)
            {
                snprintf((char*)bperf_a2dp_string_bitpool, sizeof(bperf_a2dp_string_bitpool), "%sBitpool%s(%d)%s", LIGHT_PURPLE, GREEN, bperf_global_bitpool, NONECOLOR);

                if (delta_time_max > 150000)
                {
                    snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%dms)%s", LIGHT_PURPLE, LIGHT_RED, delta_time_max/1000, NONECOLOR);
                    event_len_summary_a2dp_glitch_warning++;
                }
                else if (delta_time_max <= 150000 && delta_time_max > 100000)
                    snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%dms)%s", LIGHT_PURPLE, YELLOW, delta_time_max/1000, NONECOLOR);
                else
                    snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%dms)%s", LIGHT_PURPLE, GREEN, delta_time_max/1000, NONECOLOR);

                /* 90% : 328*1024*0.9  = 302285 */
                /* 85% : 328*1024*0.85 = 285491 */
                if ((event_len_total<<3) < 285491)
                {
                    event_len_summary_a2dp_glitch_warning++;
                    snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d kbps)%s", LIGHT_PURPLE, LIGHT_RED, (event_len_total>>7), NONECOLOR);
                }
                else if ((event_len_total<<3) >= 285491 && (event_len_total<<3) < 302285)
                    snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d kbps)%s", LIGHT_PURPLE, YELLOW, (event_len_total>>7), NONECOLOR);
                else
                    snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d kbps)%s", LIGHT_PURPLE, GREEN, (event_len_total>>7), NONECOLOR);
            }
            else
            {
                snprintf((char*)bperf_a2dp_string_bitpool, sizeof(bperf_a2dp_string_bitpool), "%sBitpool%s(%d)%s", LIGHT_PURPLE, YELLOW, bperf_global_bitpool, NONECOLOR);
                snprintf((char*)bperf_a2dp_string_interval, sizeof(bperf_a2dp_string_interval), "%sMaxInterval%s(%dms)%s", LIGHT_PURPLE, LIGHT_WHITE, delta_time_max/1000, NONECOLOR);
                snprintf((char*)bperf_a2dp_string_throughput, sizeof(bperf_a2dp_string_throughput), "%sThroughput%s(%d kbps)%s", LIGHT_PURPLE, LIGHT_WHITE, (event_len_total>>7), NONECOLOR);
            }

            printf("[bperf](%d) A2DP Num(%d), %s, %s, %s\n",
                  bperf_global_counter, event_counter, bperf_a2dp_string_bitpool, bperf_a2dp_string_interval, bperf_a2dp_string_throughput);

        }
    }
}

static void _bperf_analysis_ble_adv(struct bperf_event *bperf_event)
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
            if ( bperf_average_accumulate == 1 )
            {
                event_counter_summary_ble_adv += event_counter;
            }

            printf("[bperf](%d) BLE ADV Num(%d)\n", bperf_global_counter, event_counter);
        }
    }
}
#if 0
static void _bperf_analysis_inquiry(const uint8_t *buf, const unsigned int buf_len)
{
    int i;
    int entry_found = MAX_BPERF_BT_SCAN_ENTRY;
    int entry_empty = MAX_BPERF_BT_SCAN_ENTRY;
    int entry_used = MAX_BPERF_BT_SCAN_ENTRY;
    int save_entry = 0;
    static unsigned int bt_scan_start_time;
    static unsigned int bt_scan_result_time[MAX_BPERF_BT_SCAN_ENTRY][3];
    static uint8_t bt_scan_result_address[MAX_BPERF_BT_SCAN_ENTRY][6];

    /* HCI_INQUIRY */
    if ( buf_len == 8 && buf[0] == 0x01 && buf[1] == 0x04 && buf[2] == 0x05 )
    {
        printf("[bperf] HCI_INQUIRY Started (Inquiry_Length:%.2fs)(0ms)", (buf[6]*1.28));
        bt_scan_start_time = _bperf_get_microseconds();
        for ( i = 0 ; i < MAX_BPERF_LE_SCAN_ENTRY ; i++ )
        {
            bt_scan_result_time[i][0] = 0;
            bt_scan_result_time[i][1] = 0;
            bt_scan_result_time[i][2] = 0;
            bt_scan_result_address[i][0] = 0;
            bt_scan_result_address[i][1] = 0;
            bt_scan_result_address[i][2] = 0;
            bt_scan_result_address[i][3] = 0;
            bt_scan_result_address[i][4] = 0;
            bt_scan_result_address[i][5] = 0;
        }
    }
    /* HCI_EXTEND_INQUIRY_RESULT || HCI_REMOTE_NAME_REQUEST ||HCI_REMOTE_NAME_REQUEST_COMPLETE*/
    else if ( (buf[0] == 0x2f && buf[1] == 0xff && buf[2] == 0x01) ||
              (buf_len == 13 && buf[0] == 0x19 && buf[1] == 0x04 && buf[2] == 0x0a) ||
              (buf[0] == 0x07 && buf[1] == 0xff && buf[2] == 0x00) )
    {
        for ( i = 0 ; i < MAX_BPERF_BT_SCAN_ENTRY ; i++ )
        {
            if ( bt_scan_result_address[i][0] == 0 && bt_scan_result_address[i][1] == 0 && bt_scan_result_address[i][2] == 0 &&
                 bt_scan_result_address[i][3] == 0 && bt_scan_result_address[i][4] == 0 && bt_scan_result_address[i][5] == 0 )
            {
                entry_empty = i;
                break;
            }
        }

        for ( i = 0 ; i < MAX_BPERF_BT_SCAN_ENTRY ; i++ )
        {
            if ( strncmp((char*)(&bt_scan_result_address[i][0]), (char*)(&buf[3]), 6) == 0 )
            {
                entry_found = i;
                break;
            }
        }

        /* Entry found */
        if ( entry_found != MAX_BPERF_BT_SCAN_ENTRY )
        {
            save_entry = 1;
            entry_used = entry_found;
        }
        /* Entry not found, use empty entry */
        else if ( entry_found == MAX_BPERF_BT_SCAN_ENTRY && entry_empty != MAX_BPERF_BT_SCAN_ENTRY )
        {
            save_entry = 1;
            entry_used = entry_empty;
        }
        /* Entry full */
        else
        {
            save_entry = 0;
            printf("[bperf] HCI_EXTEND_INQUIRY_RESULT Entry FULL!! (%d)\n", MAX_BPERF_BT_SCAN_ENTRY);
        }

        if ( save_entry )
        {
            /* HCI_EXTEND_INQUIRY_RESULT */
            if ( buf[0] == 0x2f && buf[1] == 0xff && buf[2] == 0x01 )
            {
                strncpy((char*)(&bt_scan_result_address[entry_used][0]), (char*)(&buf[3]), 6);
                bt_scan_result_time[entry_used][0] = _bperf_get_microseconds();
                printf("[bperf] HCI_EXTEND_INQUIRY_RESULT (%d)(%02x:%02x:%02x:%02x:%02x:%02x)(%dms)\n",
                            entry_used, buf[8], buf[7], buf[6], buf[5], buf[4], buf[3],
                            (int)((bt_scan_result_time[entry_used][0]-bt_scan_start_time)/1000));
            }
            /* HCI_REMOTE_NAME_REQUEST */
            else if ( buf_len == 13 && buf[0] == 0x19 && buf[1] == 0x04 && buf[2] == 0x0a )
            {
                bt_scan_result_time[entry_used][1] = _bperf_get_microseconds();
                printf("[bperf] HCI_REMOTE_NAME_REQUEST (%d)(%02x:%02x:%02x:%02x:%02x:%02x)(%dms)\n",
                            entry_used, buf[8], buf[7], buf[6], buf[5], buf[4], buf[3],
                            (int)((bt_scan_result_time[entry_used][1]-bt_scan_start_time)/1000));
            }
            /* HCI_REMOTE_NAME_REQUEST_COMPLETE */
            else if ( buf[0] == 0x07 && buf[1] == 0xff && buf[2] == 0x00 )
            {
                bt_scan_result_time[entry_used][2] = _bperf_get_microseconds();
                printf("[bperf] HCI_REMOTE_NAME_REQUEST_COMPLETE (%d)(%02x:%02x:%02x:%02x:%02x:%02x)(%dms)(%s)\n",
                            entry_used, buf[8], buf[7], buf[6], buf[5], buf[4], buf[3],
                            (int)((bt_scan_result_time[entry_used][2]-bt_scan_start_time)/1000),
                            (char*)(&buf[9]));
            }
        }
        /* Duplicate, skip it */
        else
        {}
    }
    /* HCI_INQUIRY_COMPLETE */
    else if ( buf_len == 3 && buf[0] == 0x01 && buf[1] == 0x01 && buf[2] == 0x00 )
    {
        printf("[bperf] HCI_INQUIRY_COMPLETE (%dms)\n", ((_bperf_get_microseconds()-bt_scan_start_time)/1000));
    }
}

static void _bperf_analysis_le_scan(const uint8_t *buf, const unsigned int buf_len)
{
    int i;
    int entry_found = MAX_BPERF_LE_SCAN_ENTRY;
    int entry_empty = MAX_BPERF_LE_SCAN_ENTRY;
    int entry_used = MAX_BPERF_LE_SCAN_ENTRY;
    int save_entry = 0;
    static unsigned int le_scan_start_time;
    static unsigned int le_scan_total_time;
    static unsigned int le_scan_result_time[MAX_BPERF_LE_SCAN_ENTRY][3];
    static uint8_t le_scan_result_address[MAX_BPERF_LE_SCAN_ENTRY][6];
    static unsigned long le_scan_result_counter = 0;
    char prop_value_char[99];
    char prop_name_char[MAX_PROP_NAME];

    /* HCI_LE_SCAN_ENABLE */
    if ( buf_len == 5 && buf[0] == 0x0c && buf[1] == 0x20 && buf[2] == 0x02 )
    {
        if ( buf[3] == 1 )
        {
            printf("[bperf] HCI_LE_SCAN_ENABLE Started (Duplicate_Filter:%02x)(0ms)\n", buf[4]);
            le_scan_start_time = _bperf_get_microseconds();

            /* Clean LE Scan Result Counter */
            le_scan_result_counter = 0;
            //property_set("persist.bt.le_scan_result_count", "");

            /* Set All LE Scan Result Property to 0 */
            for ( i = 0 ; i < MAX_PROP_BT_SCAN_ENTRY ; i++)
            {
                snprintf(prop_name_char, sizeof(prop_name_char), "persist.bt.le_scan_result%02d", i);
                            printf("[bperf] clear property:%s value:%s\n", prop_name_char, "0" );
                //property_set(prop_name_char, "0");
            }
        }
        else if ( buf[3] == 0 )
        {
            le_scan_total_time = ((_bperf_get_microseconds()-le_scan_start_time)/1000);

            printf("[bperf] HCI_LE_SCAN_ENABLE Stopped (Duplicate_Filter:%02x)(%dms)\n", buf[4],
                            le_scan_total_time);

            printf("[bperf] LE Scan Result Count (Total Count:%lu in %dms, Average:%lu/sec)\n", le_scan_result_counter,
                            le_scan_total_time, ((le_scan_result_counter * 1000)/le_scan_total_time));


            snprintf(prop_value_char, sizeof(prop_value_char), "Total Count:%lu in %dms, Average:%lu/sec", le_scan_result_counter,
                            le_scan_total_time, ((le_scan_result_counter * 1000)/le_scan_total_time));

            //property_set("persist.bt.le_scan_result_count", prop_value_char);

            le_scan_start_time = 0;
            for ( i = 0 ; i < MAX_BPERF_LE_SCAN_ENTRY ; i++ )
            {
                le_scan_result_time[i][0] = 0;
                le_scan_result_time[i][1] = 0;
                le_scan_result_time[i][2] = 0;
                le_scan_result_address[i][0] = 0;
                le_scan_result_address[i][1] = 0;
                le_scan_result_address[i][2] = 0;
                le_scan_result_address[i][3] = 0;
                le_scan_result_address[i][4] = 0;
                le_scan_result_address[i][5] = 0;
            }
        }
    }
    /* HCI_LE_SET_SCAN_PARAMETER */
    else if ( buf_len == 10 && buf[0] == 0x0b && buf[1] == 0x20 && buf[2] == 0x07 )
    {
        float interval = (buf[4] + (buf[5]<<8))*0.625;
        float window = (buf[6] + (buf[7]<<8))*0.625;

        printf("[bperf] HCI_LE_SET_SCAN_PARAMETER scan_interval(%.2fms) scan_window(%.2fms)\n", interval, window);
    }
    /* HCI_LE_ADVERTISING_REPORT */
    else
    {
        le_scan_result_counter++;

        _bperf_mem_record_event(bperf_event_ble_adv, buf, buf_len);

        for ( i = 0 ; i < MAX_BPERF_LE_SCAN_ENTRY ; i++ )
        {
            if ( le_scan_result_address[i][0] == 0 && le_scan_result_address[i][1] == 0 && le_scan_result_address[i][2] == 0 &&
                 le_scan_result_address[i][3] == 0 && le_scan_result_address[i][4] == 0 && le_scan_result_address[i][5] == 0 )
            {
                entry_empty = i;
                break;
            }
        }

        for ( i = 0 ; i < MAX_BPERF_LE_SCAN_ENTRY ; i++ )
        {
            if ( strncmp((char*)(&le_scan_result_address[i][0]), (char*)(&buf[6]), 6) == 0 )
            {
                entry_found = i;
                break;
            }
        }

        /* Entry found */
        if ( entry_found != MAX_BPERF_LE_SCAN_ENTRY )
        {
            save_entry = 1;
            entry_used = entry_found;
        }
        /* Entry not found, use empty entry */
        else if ( entry_found == MAX_BPERF_LE_SCAN_ENTRY && entry_empty != MAX_BPERF_LE_SCAN_ENTRY )
        {
            save_entry = 1;
            entry_used = entry_empty;
            strncpy((char*)(&le_scan_result_address[entry_used][0]), (char*)(&buf[6]), 6);
        }
        /* Entry full */
        else
        {
            save_entry = 0;
            printf("[bperf] HCI_LE_ADV_REPORT Entry FULL!! (%d)\n", MAX_BPERF_LE_SCAN_ENTRY);
        }

        if ( save_entry )
        {
            /* Connectable Undirected */
            if ( buf[4] == 0x00 && le_scan_result_time[entry_used][0] == 0x00 )
            {
                le_scan_result_time[entry_used][0] = _bperf_get_microseconds();
                printf("[bperf] HCI_LE_ADV_REPORT (%d)(%02x:%02x:%02x:%02x:%02x:%02x)(%dms)(Connectable_Undirected)\n",
                            entry_used, buf[11], buf[10], buf[9], buf[8], buf[7], buf[6], (int)((le_scan_result_time[entry_used][0]-le_scan_start_time)/1000));
            }
            /* Scan Response */
            else if ( buf[4] == 0x04 && le_scan_result_time[entry_used][1] == 0x00 )
            {
                le_scan_result_time[entry_used][1] = _bperf_get_microseconds();
                printf("[bperf] HCI_LE_ADV_REPORT (%d)(%02x:%02x:%02x:%02x:%02x:%02x)(%dms)(Scan_Response)\n",
                            entry_used, buf[11], buf[10], buf[9], buf[8], buf[7], buf[6], (int)((le_scan_result_time[entry_used][1]-le_scan_start_time)/1000));

                /* Set Scan Result to Property */
                for ( i = 0 ; i < MAX_PROP_BT_SCAN_ENTRY ; i++)
                {
                    snprintf(prop_name_char, sizeof(prop_name_char), "persist.bt.le_scan_result%02d", i);
                    //property_get(prop_name_char, prop_value_char, "");
                    if ( strncmp(prop_value_char, "0", 1) == 0 )
                    {
                        snprintf(prop_value_char, sizeof(prop_value_char), "time:%05dms  addr: %02x:%02x:%02x:%02x:%02x:%02x", (int)((le_scan_result_time[entry_used][1]-le_scan_start_time)/1000)
                            , buf[11], buf[10], buf[9], buf[8], buf[7], buf[6]);

                        printf("[bperf] LE Scan Result Set Property:%s value:%s \n", prop_name_char, prop_value_char );
                        //property_set(prop_name_char, prop_value_char);
                        return ;
                    }
                }
            }
            /* Non Connectable Undirected */
            else if ( buf[4] == 0x03 && le_scan_result_time[entry_used][2] == 0x00 )
            {
                le_scan_result_time[entry_used][2] = _bperf_get_microseconds();
                printf("[bperf] HCI_LE_ADV_REPORT (%d)(%02x:%02x:%02x:%02x:%02x:%02x)(%dms)(Non_Connectable_Undirected)\n",
                            entry_used, buf[11], buf[10], buf[9], buf[8], buf[7], buf[6], (int)((le_scan_result_time[entry_used][2]-le_scan_start_time)/1000));
            }
            /* Unknown Type */
            else if ( buf[4] != 0x00 && buf[4] != 0x04 && buf[4] != 0x03 )
            {
                printf("[bperf] HCI_LE_ADV_REPORT (%d)(%02x:%02x:%02x:%02x:%02x:%02x)(%dms)(UNKNOWN 0x%02x)\n",
                            entry_used, buf[11], buf[10], buf[9], buf[8], buf[7], buf[6], (int)((_bperf_get_microseconds()-le_scan_start_time)/1000), buf[4]);
            }
        }
        /* Duplicate, skip it */
        else
        {}
    }
}
#endif

static void _bperf_analysis()
{
    char prop_value_char[99];
    int prop_value_int = 0;
    /*if ( property_get("persist.bluetooth.bperf.average", prop_value_char, "0") > 0 )
    {
        prop_value_int = atoi(prop_value_char);
        if ( prop_value_int > 0 && prop_value_int < 100000 )
        {
            bperf_average_total_time ++;
            bperf_average_accumulate = 1;
        }
    }*/

    if ( bperf_event_ble_adv )
    {
        _bperf_analysis_ble_adv(bperf_event_ble_adv_analysis);
    }

    if ( bperf_event_a2dp_analysis )
    {
        _bperf_analysis_a2dp(bperf_event_a2dp_analysis);
    }

    if ( bperf_event_voice_analysis )
    {
        _bperf_analysis_voice(bperf_event_voice_analysis);
    }

    if ( bperf_event_rc_fw_upgrade_analysis )
    {
        _bperf_analysis_rc_fw_upgrade(bperf_event_rc_fw_upgrade_analysis);
    }

    if ( bperf_event_hid_analysis )
    {
        _bperf_analysis_hid_hogp(bperf_event_hid_analysis);
    }

    if ( bperf_event_hid_cursor_analysis )
    {
        _bperf_analysis_hid_hogp(bperf_event_hid_cursor_analysis);
    }

    if ( bperf_event_hogp_analysis )
    {
        _bperf_analysis_hid_hogp(bperf_event_hogp_analysis);
    }

    if ( bperf_event_hogp_cursor_analysis )
    {
        _bperf_analysis_hid_hogp(bperf_event_hogp_cursor_analysis);
    }

    if ( prop_value_int > 0 )
    {
        prop_value_int--;
        snprintf(prop_value_char, 5, "%d", prop_value_int);
        //property_set("persist.bluetooth.bperf.average", prop_value_char);

        if ( prop_value_int == 0 )
        {
            if ( bperf_average_total_time )
            {
                if ( event_counter_summary_ble_adv )
                {
                    printf("[bperf] BLE ADV Num(average)(%ds) (%d)\n" NONECOLOR, bperf_average_total_time, (event_counter_summary_ble_adv/bperf_average_total_time));
                    event_counter_summary_ble_adv = 0;
                    snprintf(prop_value_char, 5, "%d", event_counter_summary_ble_adv);
                    //property_set("persist.bt.adv_num", prop_value_char);
                }
                if ( event_len_summary_a2dp )
                {
                    printf("[bperf] A2DP(average)(%ds) Throughput (%d kbps) Audio Glitch Warning(%d)\n",
                            bperf_average_total_time, (event_len_summary_a2dp<<3)/bperf_average_total_time, event_len_summary_a2dp_glitch_warning);
                    event_len_summary_a2dp = 0;
                    snprintf(prop_value_char, 8, "%d", (event_len_summary_a2dp>>7)/bperf_average_total_time);
                    //property_set("persist.bt.a2dp_avg_rate", prop_value_char);
                    snprintf(prop_value_char, 8, "%d", event_len_summary_a2dp_glitch_warning);
                    //property_set("persist.bt.a2dp_avg_glitch", prop_value_char);
                    event_len_summary_a2dp_glitch_warning = 0;
                }
                if ( event_counter_summary_hid )
                {
                    printf("[bperf] HID(average)(%ds) Num (%d)  Max_Delta_Time(%dms)\n",
                            bperf_average_total_time,
                            event_counter_summary_hid/bperf_average_total_time,
                            event_counter_summary_hid_delta_time_max/1000);
                    snprintf(prop_value_char, 5, "%d", event_counter_summary_hid);
                    //property_set("persist.bt.hid_num", prop_value_char);
                    event_counter_summary_hid = 0;
                    event_counter_summary_hid_delta_time_max = 0;
                }
                if ( event_counter_summary_hid_cursor )
                {
                    printf("[bperf] HID_Cursor(average)(%ds) Num (%d)  Max_Delta_Time(%dms)\n",
                            bperf_average_total_time,
                            event_counter_summary_hid_cursor/bperf_average_total_time,
                            event_counter_summary_hid_cursor_delta_time_max/1000);
                    snprintf(prop_value_char, 5, "%d", event_counter_summary_hid_cursor);
                    //property_set("persist.bt.hid_cur_num", prop_value_char);
                    event_counter_summary_hid_cursor = 0;
                    event_counter_summary_hid_cursor_delta_time_max = 0;
                }
                if ( event_counter_summary_hogp )
                {
                    printf("[bperf] HOGP(average)(%ds) Num (%d)  Max_Delta_Time(%dms)\n",
                            bperf_average_total_time,
                            event_counter_summary_hogp/bperf_average_total_time,
                            event_counter_summary_hogp_delta_time_max/1000);
                    snprintf(prop_value_char, 5, "%d", event_counter_summary_hogp);
                    //property_set("persist.bt.hopg_num", prop_value_char);
                    event_counter_summary_hogp = 0;
                    event_counter_summary_hogp_delta_time_max = 0;
                }
                if ( event_counter_summary_hogp_cursor )
                {
                    printf("[bperf] HOGP_Cursor(average)(%ds) Num (%d)  Max_Delta_Time(%dms)\n",
                            bperf_average_total_time,
                            event_counter_summary_hogp_cursor/bperf_average_total_time,
                            event_counter_summary_hogp_cursor_delta_time_max/1000);
                    snprintf(prop_value_char, 5, "%d", event_counter_summary_hogp_cursor);
                    //property_set("persist.bt.hopg_cur_num", prop_value_char);
                    event_counter_summary_hogp_cursor = 0;
                    event_counter_summary_hogp_cursor_delta_time_max = 0;
                }
                if ( event_len_summary_voice )
                {
                    printf("[bperf] VOICE(average)(%ds) Voice Data Rate (%d bps)  Total Packet Drop (%d)\n" ,
                            bperf_average_total_time, (event_len_summary_voice<<3)/bperf_average_total_time, event_len_summary_voice_drop);
                    snprintf(prop_value_char, 8, "%d", (event_len_summary_voice<<3)/bperf_average_total_time);
                    //property_set("persist.bt.voble_avg_rate", prop_value_char);
                    snprintf(prop_value_char, 8, "%d", event_len_summary_voice_drop);
                    //property_set("persist.bt.voble_avg_drop", prop_value_char);
                    event_len_summary_voice = 0;
                    event_len_summary_voice_drop = 0;
                }
                if ( event_len_summary_rc_fw_upgrade )
                {
                    printf("[bperf] VOICE(average)(%ds) Voice Data Rate (%d bps)\n", bperf_average_total_time, (event_len_summary_rc_fw_upgrade<<3)/bperf_average_total_time);
                    snprintf(prop_value_char, 5, "%d", (event_len_summary_rc_fw_upgrade<<3)/bperf_average_total_time);
                    //property_set("persist.bt.rc_fw_upg_avg_rate", prop_value_char);
                    event_len_summary_rc_fw_upgrade = 0;
                }
            }

            bperf_average_total_time = 0;
            bperf_average_accumulate = 0;
        }
    }
    /* prop_value_int == 0 */
    else
    {
        //property_get("persist.bluetooth.perf.average", prop_value_char, "0");
        prop_value_int = atoi(prop_value_char);
        if ( prop_value_int > 0 )
        {
            //property_set("persist.bluetooth.bperf.average", prop_value_char);
        }
    }
}

static void *_bperf_thread_main(void *arg)
{
    unsigned int time_begin;
    unsigned int time_end;
    unsigned int time_sleep;
    (void)arg;
    printf("Thread Started\n");

    if ( !bperf_event_voice || !bperf_event_voice_analysis ||
         !bperf_event_rc_fw_upgrade || !bperf_event_rc_fw_upgrade_analysis ||
         !bperf_event_hid || !bperf_event_hid_analysis ||
         !bperf_event_hid_cursor || !bperf_event_hid_cursor_analysis ||
         !bperf_event_hogp || !bperf_event_hogp_analysis ||
         !bperf_event_hogp_cursor || !bperf_event_hogp_cursor_analysis ||
         !bperf_event_a2dp || !bperf_event_a2dp_analysis ||
         !bperf_event_ble_adv || !bperf_event_ble_adv_analysis )
    {
        printf("Allocate memory failed!\n");
        printf("Thread Stopped\n");
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

        _bperf_analysis();
        _bperf_mem_reset_analysis();
        time_end = _bperf_get_microseconds();
        time_sleep = 1000000-(time_end-time_begin);
        usleep(time_sleep);
        bperf_global_counter++;
    }

    bperf_main_thread_should_stop = 0;
    bperf_main_thread_status = BPERF_STATE_THREAD_STOPPED;
    printf("Thread Stopped\n");
    return 0;
}

static void _bperf_thread_start()
{
    if ( bperf_main_thread_status != BPERF_STATE_THREAD_RUNNING && !bperf_main_thread_should_stop)
    {
        printf("Create thread\n");
        pthread_create(&bperf_thread_main, NULL, _bperf_thread_main, NULL);
    }
}

static void _bperf_thread_stop()
{
    bperf_main_thread_should_stop = 1;
}

void bperf_notify_cmd(const uint8_t *buf, const unsigned int buf_len)
{
    if ( bperf_main_thread_status == BPERF_STATE_THREAD_RUNNING && !bperf_main_thread_should_stop )
    {
#if 0
        /* HCI_LE_SCAN_ENABLE */
        if ( buf_len == 5 && buf[0] == 0x0c && buf[1] == 0x20 && buf[2] == 0x02 )
        {
            _bperf_analysis_le_scan(buf, buf_len);
        }
        /* HCI_LE_SET_SCAN_PARAMETER */
        else if ( buf_len == 10 && buf[0] == 0x0b && buf[1] == 0x20 && buf[2] == 0x07 )
        {
            _bperf_analysis_le_scan(buf, buf_len);
        }
        /* HCI_INQUIRY */
        else if ( buf_len == 8 && buf[0] == 0x01 && buf[1] == 0x04 && buf[2] == 0x05 )
        {
            _bperf_analysis_inquiry(buf, buf_len);
        }
        /* HCI_REMOTE_NAME_REQUEST */
        else if ( buf_len == 13 && buf[0] == 0x19 && buf[1] == 0x04 && buf[2] == 0x0a )
        {
            _bperf_analysis_inquiry(buf, buf_len);
        }
#endif
    }
}

void bperf_notify_event(const uint8_t *buf, const unsigned int buf_len)
{
    if ( bperf_main_thread_status == BPERF_STATE_THREAD_RUNNING && !bperf_main_thread_should_stop )
    {
#if 0
        /* HCI_LE_ADVERTISING_REPORT */
        if ( (buf_len > 12) && buf[0] == 0x3e && buf[2] == 0x02 && buf[3] == 0x01 )
        {
            _bperf_analysis_le_scan(buf, buf_len);
        }
        /* HCI_INQUIRY_COMPLETE */
        else if ( buf_len == 3 && buf[0] == 0x01 && buf[1] == 0x01 && buf[2] == 0x00 )
        {
            _bperf_analysis_inquiry(buf, buf_len);
        }
        /* HCI_REMOTE_NAME_REQUEST_COMPLETE */
        else if ( buf[0] == 0x07 && buf[1] == 0xff && buf[2] == 0x00 )
        {
            _bperf_analysis_inquiry(buf, buf_len);
        }
        /* HCI_EXTEND_INQUIRY_RESULT */
        else if ( buf[0] == 0x2f && buf[1] == 0xff && buf[2] == 0x01 )
        {
            _bperf_analysis_inquiry(buf, buf_len);
        }
#endif
    }
}

void bperf_notify_data(const uint8_t *buf, const unsigned int buf_len)
{
    if ( bperf_main_thread_status == BPERF_STATE_THREAD_RUNNING && !bperf_main_thread_should_stop )
    {
        /* HT RC Voice Search (2541) */
        if ( (buf_len == 12 || buf_len == 31) &&
             (buf[2] == 0x08 || buf[2] == 0x1b) && buf[3] == 0x00 && buf[8] == 0x1b && buf[9] == 0x35 && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_mem_record_event(bperf_event_voice, buf, buf_len);
        }
        /* HT RC Voice Search (2640) */
        else if ( buf_len == 31 && buf[2] == 0x1b && buf[3] == 0x00 && buf[8] == 0x1b && buf[9] == 0x3f && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_mem_record_event(bperf_event_voice, buf, buf_len);
        }
        /* HT RC Voice Search (2640)(BLE Data Length Extension) */
        else if ( buf_len == 111 && buf[2] == 0x6b && buf[3] == 0x00 && buf[8] == 0x1b && buf[9] == 0x3f && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_mem_record_event(bperf_event_voice, buf, buf_len);
        }
        /* Airoha Voice Search */
        else if ( buf_len == 31 && buf[2] == 0x1b && buf[3] == 0x00 && buf[8] == 0x1b && buf[9] == 0x29 && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 0;
            _bperf_mem_record_event(bperf_event_voice, buf, buf_len);
        }
        /* Nordic Voice Search, Turnkey */
        else if ( buf[3] == 0x00 && buf[8] == 0x1b && buf[9] == 0x1d && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 1;
            _bperf_mem_record_event(bperf_event_voice, buf, buf_len);
        }
        /* Nordic Voice Search, Huitong */
        else if ( buf[3] == 0x00 && buf[8] == 0x1b && buf[9] == 0x23 && buf[10] == 0x00 )
        {
            bperf_global_voble_codec = 1;
            _bperf_mem_record_event(bperf_event_voice, buf, buf_len);
        }
        /* HT RC Button */
        else if ( buf_len == 12 && buf[2] == 0x08 && buf[3] == 0x00 && buf[4] == 0x04 && buf[5] == 0x00 && buf[8] == 0x1b )
        {
            _bperf_mem_record_event(bperf_event_hogp, buf, buf_len);
        }
        /* HT RC Button */
        else if ( buf_len == 13 && buf[2] == 0x09 && buf[3] == 0x00 && buf[4] == 0x05 && buf[5] == 0x00 && buf[8] == 0x1b )
        {
            _bperf_mem_record_event(bperf_event_hogp, buf, buf_len);
        }
        /* HID : SNOW RC */
        else if ( buf_len == 13 && buf[2] == 0x09 && buf[3] == 0x00 && buf[4] == 0x05 && buf[5] == 0x00 && buf[8] == 0xa1 )
        {
            _bperf_mem_record_event(bperf_event_hid_cursor, buf, buf_len);
        }
        /* HID : Logitech Keyboard */
        else if ( buf_len == 18 && buf[2] == 0x0e && buf[3] == 0x00 && buf[4] == 0x0a && buf[5] == 0x00 && buf[8] == 0xa1 )
        {
            _bperf_mem_record_event(bperf_event_hid, buf, buf_len);
        }
        /* HID_Curosr : Microsoft Sculpt Comfort Mouse */
        else if ( buf_len == 19 && buf[2] == 0x0f && buf[3] == 0x00 && buf[4] == 0x0b && buf[5] == 0x00 && buf[9] == 0x1a )
        {
            _bperf_mem_record_event(bperf_event_hid_cursor, buf, buf_len);
        }
        /* HID_Cursor : Logitech M557 */
        else if ( buf_len == 16 && buf[2] == 0x0c && buf[3] == 0x00 && buf[4] == 0x08 && buf[5] == 0x00 && buf[8] == 0xa1 )
        {
            _bperf_mem_record_event(bperf_event_hid_cursor, buf, buf_len);
        }
        /* HID_Cursor : Logitech M558 */
        else if ( buf_len == 17 && buf[2] == 0x0d && buf[3] == 0x00 && buf[4] == 0x09 && buf[5] == 0x00 && buf[8] == 0xa1 )
        {
            _bperf_mem_record_event(bperf_event_hid_cursor, buf, buf_len);
        }
        /* HOGP_Cursor : Microsoft Designer BLE Mouse*/
        else if ( buf_len == 20 && buf[2] == 0x10 && buf[3] == 0x00 && buf[4] == 0x0c && buf[5] == 0x00 && buf[8] == 0x1b )
        {
            _bperf_mem_record_event(bperf_event_hogp_cursor, buf, buf_len);
        }
        /* HOGP_Cursor : Elecom BLE Mouse */
        else if ( buf_len == 17 && buf[2] == 0x0d && buf[3] == 0x00 && buf[4] == 0x09 && buf[5] == 0x00 && buf[8] == 0x1b )
        {
            _bperf_mem_record_event(bperf_event_hogp_cursor, buf, buf_len);
        }
        /* A2DP Sink */
        else if ( buf[8] == 0x80 && buf[9] == 0x60 )
        {
            _bperf_mem_record_event(bperf_event_a2dp, buf, buf_len);
            if ( buf[20] == 0x00 ) /* SCMS-T */
                bperf_global_bitpool = buf[24];
            else
                bperf_global_bitpool = buf[23];
        }
        /* A2DP Src */
        else if ( buf_len == 587 && buf[8] == 0x80 && buf[9] == 0x60 && buf[20] == 0x00 )
        {
            _bperf_mem_record_event(bperf_event_a2dp, buf, buf_len);
            bperf_global_bitpool = buf[24];
        }
        /* HT RC FW Upgrade */
        else if ( buf_len == 29 && buf[2] == 0x19 && buf[3] == 0x00 && buf[4] == 0x15 && buf[5] == 0x00 && buf[9] == 0x48 && buf[10] == 0x00 )
        {
            _bperf_mem_record_event(bperf_event_rc_fw_upgrade, buf, buf_len);
        }
#if 0
        /* HID : DS4 */
        else if ( buf_len == 19 && buf[2] == 0x0f && buf[3] == 0x00 && buf[4] == 0x0b && buf[5] == 0x00 && buf[8] == 0xa1 )
        {
            _bperf_mem_record_event(bperf_event_hid, buf, buf_len);
        }
#endif
    }
}

void bperf_init()
{
    printf("[bperf] Version : %s\n", BPERF_LIBRARY_VERSION);
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
