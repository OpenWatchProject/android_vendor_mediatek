/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2007 The Android Open Source Project
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/poll.h>
#include <string.h>
#include <unistd.h>
#include <cust.h>
#include <linux/input.h>
#include <pthread.h>
#include <graphics.h>
#include "events.h"
#include "miniui.h"
#define MAX_DEVICES 32
#define MAX_LENGTH  1024

static struct pollfd ev_fds[MAX_DEVICES];
static unsigned ev_count = 0;
static unsigned ev_touch = 0;
static struct input_event ev_queue[2048];
static pthread_mutex_t event_mutex = PTHREAD_MUTEX_INITIALIZER;
static int ev_head_num = 0;
int g_x = 0;
int g_y = 0;
int g_touch = -1;
int g_RBTN = -1;
int g_LBTN = -1;
int g_CBTN = -1;

void dispatch_event(struct input_event *ev) 
{
    static int x = 0 ; 
	static int y = 0 ;
    if(ev->type == EV_ABS && ev->code == ABS_MT_POSITION_X) 
    { 
        x = ev->value; 
    } 
    else if(ev->type == EV_ABS && ev->code==ABS_MT_POSITION_Y) 
    { 
        y=ev->value; 
    }
    else if(ev->type==EV_KEY && ev->code==BTN_TOUCH) 
    { 
    	g_touch = ev->value;    
    } 
    else if((ev->type==EV_SYN)&&(ev->code == SYN_REPORT)) 
    {
        g_x = x;
        g_y = y;
        if((g_x > 0) && (g_x < gr_fb_width()/3) &&(g_y < gr_fb_height()) && (g_y > gr_fb_height()-2*CHAR_HEIGHT_ITEM))
        {
            g_LBTN = 1;
            g_CBTN = 0;
            g_RBTN = 0;
        }
        
        else if((g_x > 2*gr_fb_width()/3) && (g_x<gr_fb_width()) &&  (g_y < gr_fb_height()) && (g_y>gr_fb_height()-2*CHAR_HEIGHT_ITEM))
        {
            g_LBTN = 0;
            g_CBTN = 0;
            g_RBTN = 1;
        }
        
        else if((g_x > gr_fb_width()/3) && (g_x < 2*gr_fb_width()/3) &&  (g_y < gr_fb_height()) && (g_y > gr_fb_height()-2*CHAR_HEIGHT_ITEM))
        {
            g_LBTN = 0;
            g_CBTN = 1;
            g_RBTN = 0;
        }
        else
        {
            g_LBTN = 0;
            g_CBTN = 0;
            g_RBTN = 0; 	
        }
    }
}

void add_event(struct input_event *ev) {
     ev_queue[ev_head_num].type = ev->type; 
     ev_queue[ev_head_num].code = ev->code; 
     ev_queue[ev_head_num].value = ev->value; 
     pthread_mutex_lock(&event_mutex);
     ev_head_num++;
     if (ev_head_num == 2048) ev_head_num = 0;
     pthread_mutex_unlock(&event_mutex);
}

int ev_init(void)
{
    DIR *dir = NULL;
    struct dirent *de;
    char name[MAX_LENGTH];
    int fd;
    
    dir = opendir("/dev/input");
    if(dir != 0) {
        while((de = readdir(dir))) {
//            fprintf(stderr,"/dev/input/%s\n", de->d_name);
            if(strncmp(de->d_name,"event",5)) continue;
            fd = openat(dirfd(dir), de->d_name, O_RDONLY);
            if(fd < 0) continue;

            ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name); 
            
            //LOGD(TAG "NAME = %s\n", name);
            if (!strcmp(name, "mtk-tpd") || !strcmp(name, "mtk-tpd2")) {
                ev_touch = ev_count;
            }
           
            ev_fds[ev_count].fd = fd;
            ev_fds[ev_count].events = POLLIN;
            ev_count++;
            if(ev_count == MAX_DEVICES) break;
        }
    }
	
	closedir(dir);
    return 0;
}

void ev_exit(void)
{
    while (ev_count > 0) {
        close(ev_fds[--ev_count].fd);
    }
}

int ev_get(struct input_event *ev, unsigned dont_wait)
{
    int r;
    unsigned n;
    static unsigned idx = 0;

    do {
        r = poll(ev_fds, ev_count, dont_wait ? 0 : -1);

        if(r > 0) {
            n = idx;
            do {
                if(ev_fds[n].revents & POLLIN) {
                    r = read(ev_fds[n].fd, ev, sizeof(*ev));
                    if(r == sizeof(*ev)) {
                        idx = ((n+1)%ev_count);
                        if (n == ev_touch) {
                            add_event(ev); 
                            #ifdef FEATURE_FTM_TOUCH_MODE
							dispatch_event(ev);
							#endif      
                        }
                        return 0;
                    }
                }
                n = ((n+1)%ev_count);
            } while(n!=idx);
        }
    } while(dont_wait == 0);

    return -1;
}

int get_ev_head_num()
{
    return ev_head_num;
}

void get_ev_arr(struct input_event *ev, int i)
{
	ev->type = ev_queue[i].type;
    ev->code = ev_queue[i].code;
    ev->value = ev_queue[i].value;
}
