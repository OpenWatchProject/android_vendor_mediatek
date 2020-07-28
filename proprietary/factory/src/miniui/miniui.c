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

#include <linux/input.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/reboot.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "common.h"
#include "miniui.h"
#include "graphics.h"
#include "events.h"
#include "resources.h"
#include "ftm.h"

#define TAG     "[MIUI] "
#define LOGD ALOGD

enum { 
LEFT_SIDE, 
CENTER_TILE, 
RIGHT_SIDE, 
NUM_SIDES 
};
int maxrows=0;
#define set_gr_color(c) \
    gr_color(color_r(c), color_g(c), color_b(c), color_a(c))

static pthread_mutex_t gUpdateMutex = PTHREAD_MUTEX_INITIALIZER;

static gr_surface gCurrentIcon = NULL;

// Log text overlay, displayed when a magic key is pressed
static char text[MAX_ROWS][MAX_COLS];
int item_cols = 0, item_rows = 0;
static int text_cols = 0, text_rows = 0;
static int text_col = 0, text_row = 0, text_top = 0;
static int show_text = 1;

static char menu[MAX_ROWS][MAX_COLS];
static int show_menu = 1;
static int menu_top = 0, menu_items = 0, menu_sel = 0;

// Key event input queue
static pthread_mutex_t key_queue_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t key_queue_cond = PTHREAD_COND_INITIALIZER;
static int key_queue[256], key_queue_len = 0;
static volatile char key_pressed[KEY_MAX + 1];
extern int bg_arr[MAX_ROWS];
char result[3][16] = {"testing\r\n", "pass\r\n", "fail\r\n"};

// Clear the screen and draw the currently selected background icon (if any).
// Should only be called with gUpdateMutex locked.
static void draw_background_locked(gr_surface icon)
{
    gr_color(0, 0, 0, 255);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());

    if (icon) {
        int iconWidth = gr_get_width(icon);
        int iconHeight = gr_get_height(icon);
        int iconX = (gr_fb_width() - iconWidth) / 2;
        int iconY = (gr_fb_height() - iconHeight) / 2;
        gr_blit(icon, 0, 0, iconWidth, iconHeight, iconX, iconY);
    }
}

static void draw_menu_text(GRFont *gr_font, int row, const char* t, int char_height) 
{
	  if(t == NULL)
    {
        return;
    }
    if (t[0] != '\0') 
    {
        gr_text(gr_font, 0, (row+1)*char_height-1, t);
    }
}


// Redraw everything on the screen.  Does not flip pages.
// Should only be called with gUpdateMutex locked.
static void draw_screen_locked(void)
{
    draw_background_locked(gCurrentIcon);
    if (show_text) {
        /* background color */
        gr_color(0, 0, 0, 160);
        gr_fill(0, 0, gr_fb_width(), gr_fb_height());

        /* title background color */
        gr_color(51, 52, 255, 160);
        gr_fill(0, 0, gr_fb_width(), CHAR_HEIGHT);

        /* title text */
        set_gr_color(COLOR_YELLOW);
        draw_menu_text(gr_font_small, 0, menu[0],CHAR_HEIGHT);

        int i = 1;
        if (show_menu) {
            gr_color(64, 96, 255, 160);            
            gr_fill(0, (menu_top+menu_sel) * CHAR_HEIGHT,
                    gr_fb_width(), (menu_top+menu_sel+1)*CHAR_HEIGHT+1);

            for (; i < menu_top + menu_items; ++i) {
                if (i == menu_top + menu_sel) {
                    set_gr_color(COLOR_YELLOW);
                    draw_menu_text(gr_font_small, i, menu[i], CHAR_HEIGHT);
                    gr_color(64, 96, 255, 160);
                } else {
                    gr_color(120, 120, 255, 160);
                    draw_menu_text(gr_font_small, i, menu[i], CHAR_HEIGHT);
                    gr_color(64, 96, 255, 160);
                }
                gr_fill(0, i * CHAR_HEIGHT + CHAR_HEIGHT/2-1,
                        gr_fb_width(), 1);
            }
            gr_fill(0, i*CHAR_HEIGHT+CHAR_HEIGHT/2-1,
                    gr_fb_width(), i*CHAR_HEIGHT+CHAR_HEIGHT/2+1);
            ++i;
        }

        /* printed text color */
        set_gr_color(COLOR_YELLOW);

        int j = 0;
        for (; i < item_rows; ++i)
            draw_menu_text(gr_font_small, i, text[j++], CHAR_HEIGHT);
    }
}

// Redraw everything on the screen and flip the screen (make it visible).
// Should only be called with gUpdateMutex locked.
static void update_screen_locked(void)
{
    draw_screen_locked();
    gr_flip();
}

// Reads input events, handles special hot keys, and adds to the key queue.
static void *input_thread(void *cookie)
{
    int rel_sum_y = 0, rel_sum_x = 0;
    int fake_key = 0;
    if (cookie == NULL) // reserved parameter, for resolving warning
    {
        LOGD(TAG "The parameter is NULL\n");
	}
    for (;;) {
        // wait for the next key event
        struct input_event ev;
        memset(&ev, 0, sizeof(ev));
        do {
            ev_get(&ev, 0);

            if (ev.type == EV_SYN) {
                continue;
            } 
            /*else if (ev.type == EV_REL) {
                if (ev.code == REL_Y) {
                    // accumulate the up or down motion reported by
                    // the trackball.  When it exceeds a threshold
                    // (positive or negative), fake an up/down
                    // key event.
                    rel_sum_y += ev.value;
                    if (rel_sum_y > 3) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_DOWN;
                        ev.value = 1;
                        rel_sum_y = 0;
                    } else if (rel_sum_y < -3) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_UP;
                        ev.value = 1;
                        rel_sum_y = 0;
                    }
                }
                if (ev.code == REL_X) {
                    // accumulate the left or right motion reported by
                    // the trackball.  When it exceeds a threshold
                    // (positive or negative), fake an left/right
                    // key event.
                    rel_sum_x += ev.value;
                    if (rel_sum_x > 3) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_RIGHT;
                        ev.value = 1;
                        rel_sum_x = 0;
                    } else if (rel_sum_x < -3) {
                        fake_key = 1;
                        ev.type = EV_KEY;
                        ev.code = KEY_LEFT;
                        ev.value = 1;
                        rel_sum_x = 0;
                    }
                }
            } */
            else {
                rel_sum_x = 0;
                rel_sum_y = 0;
            }
        } while (ev.type != EV_KEY || ev.code > KEY_MAX);

        pthread_mutex_lock(&key_queue_mutex);
        if (!fake_key) {
            // our "fake" keys only report a key-down event (no
            // key-up), so don't record them in the key_pressed
            // table.
            key_pressed[ev.code] = ev.value;
        }
        fake_key = 0;
        const int queue_max = sizeof(key_queue) / sizeof(key_queue[0]);
       if(ev.code == 330)
       {
           if ((ev.value == 0) && key_queue_len < queue_max) 
           {
               key_queue[key_queue_len++] = ev.code;
               pthread_cond_signal(&key_queue_cond);          
           }        	
       }
        else
        {
            if (ev.value > 0 && key_queue_len < queue_max) {
                key_queue[key_queue_len++] = ev.code;
                pthread_cond_signal(&key_queue_cond);
            }
        }
        pthread_mutex_unlock(&key_queue_mutex);
    }
    return NULL;
}

int get_avail_textline()
{
#define NR_LINE_MARGIN 2
#define NR_LINE_BLANK  2
    return ((gr_fb_height()-UNAVAIL_ROW*CHAR_HEIGHT_ITEM)/CHAR_HEIGHT) - 2 * NR_LINE_MARGIN - NR_LINE_BLANK;
}

void ui_init(void)
{
    gr_init();
    ev_init();

    text_col = text_row = 0;
    item_rows = gr_fb_height() / CHAR_HEIGHT_ITEM;
    --item_rows;
    if (item_rows > MAX_ROWS) item_rows = MAX_ROWS;
    text_top = 1;

    item_cols = gr_fb_width() / CHAR_WIDTH_ITEM;
    
    if (item_cols > MAX_COLS - 1) item_cols = MAX_COLS - 1;

    text_rows = gr_fb_height()/CHAR_HEIGHT;
    text_cols = gr_fb_width() /CHAR_WIDTH;   

    maxrows = item_rows - UNAVAIL_ROW;
    //LOGD(TAG "CUST_LCD_AVAIL_HEIGHT=%d, CUST_LCD_AVAIL_WIDTH=%d\n",
    //    gr_fb_height(), gr_fb_width());
    //LOGD(TAG "CUST_KEY_CONFIRM=%d, CUST_KEY_BACK=%d\n", 
    //    CUST_KEY_CONFIRM, CUST_KEY_BACK);
    //LOGD(TAG "ITEM_ROWS=%d, ITEM_COLS=%d\n", item_rows, item_cols);

    pthread_t t;
    pthread_create(&t, NULL, input_thread, NULL);
}
  
static void show_slash_draw_screen_locked(const char * msg)
{
    draw_background_locked(gCurrentIcon);
   
    // background color
    gr_color(0, 0, 0, 160);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());
  
    // printed text color 
    set_gr_color(COLOR_YELLOW);
 

    if (msg[0] != '\0') {
        //gr_text_scale(1, (1)*CHAR_HEIGHT-1, msg, 2); //will use this in future
        gr_text(gr_font_big, 1, (1)*CHAR_HEIGHT_ITEM-1, msg);
    }
    
}

static void show_slash_update_screen_locked(const char * msg)
{
    show_slash_draw_screen_locked(msg);
    gr_flip();
}

void show_slash_screen(const char * msg)
{

    pthread_mutex_lock(&gUpdateMutex);
    
    show_slash_update_screen_locked(msg);
    
    pthread_mutex_unlock(&gUpdateMutex);
 
}

void ui_print(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, 256, fmt, ap);
    va_end(ap);

    fputs(buf, stdout);
    
    // This can get called before ui_init(), so be careful.
    pthread_mutex_lock(&gUpdateMutex);
    if (text_rows > 0 && text_cols > 0) {
        char *ptr;
        for (ptr = buf; *ptr != '\0'; ++ptr) {
            if (*ptr == '\n' || text_col >= text_cols) {
                text[text_row][text_col] = '\0';
                text_col = 0;
                text_row = (text_row + 1) % text_rows;
                if (text_row == text_top) text_top = (text_top + 1) % text_rows;
            }
            if (*ptr != '\n') text[text_row][text_col++] = *ptr;
        }
        text[text_row][text_col] = '\0';
        update_screen_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
}


void ui_printf(const char *fmt, ...)
{
    char buf[256];
    va_list ap;
	static int col = 0, row = 0, top = 0;

	memset(text,'\0',sizeof(text));
    va_start(ap, fmt);
    vsnprintf(buf, 256, fmt, ap);
    va_end(ap);
	int temp_text_cols = 0;

    fputs(buf, stdout);
    #if defined MTK_FACTORY_MODE_IN_GB2312
    temp_text_cols = text_cols *2 ;
    #else
    temp_text_cols = text_cols ;
    #endif   
    // This can get called before ui_init(), so be careful.
    pthread_mutex_lock(&gUpdateMutex);
    if (text_rows > 0 && text_cols > 0) {
        char *ptr;
        for (ptr = buf; *ptr != '\0'; ++ptr) {
            if ((*ptr == '\n') || (col >= temp_text_cols) 
                ||((col == temp_text_cols-1) && ((*ptr & 0x80) != 0) )
                ) 
            {
                text[row][col] = '\0';
                col = 0;
                row = (row + 1) % text_rows;
                if (row == top) top = (top + 1) % text_rows;
            }
            if (*ptr != '\n') text[row][col++] = *ptr;
        }
        text[row][col] = '\0';
        update_screen_locked();
    }
    pthread_mutex_unlock(&gUpdateMutex);
}


int ui_wait_key()
{
    pthread_mutex_lock(&key_queue_mutex);
    while (key_queue_len == 0) {
        pthread_cond_wait(&key_queue_cond, &key_queue_mutex);
    }

    int key = key_queue[0];
    memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
    pthread_mutex_unlock(&key_queue_mutex);
    
    return key;
}

int ui_wait_phisical_key()
{
    pthread_mutex_lock(&key_queue_mutex);
    while (key_queue_len == 0) {
        pthread_cond_wait(&key_queue_cond, &key_queue_mutex);
    }

    while((key_queue[0] != 114) && (key_queue[0] != 115) && (key_queue[0] != 116))
    {
        memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);
        while (key_queue_len == 0) {
            pthread_cond_wait(&key_queue_cond, &key_queue_mutex);
        }
    }

    int key = key_queue[0];
    memcpy(&key_queue[0], &key_queue[1], sizeof(int) * --key_queue_len);

    pthread_mutex_unlock(&key_queue_mutex);

    return key;
}

int ui_key_pressed(int key)
{
    // This is a volatile static array, don't bother locking
    return key_pressed[key];
}

void ui_clear_key_queue() {
    pthread_mutex_lock(&key_queue_mutex);
    key_queue_len = 0;
    pthread_mutex_unlock(&key_queue_mutex);
}

void ui_line(int x1, int y1, int x2, int y2, int width) {
    pthread_mutex_lock(&gUpdateMutex);
    gr_line(x1,y1,x2,y2,width);
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_color(int r, int g, int b, int a) {
    pthread_mutex_lock(&gUpdateMutex);
    gr_color(r,g,b,a);
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_text(int x, int y, char *str) {
    pthread_mutex_lock(&gUpdateMutex);
    gr_text(gr_font_small,x,y,str);
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_fill(int x, int y, int w, int h) {
    pthread_mutex_lock(&gUpdateMutex);
    gr_fill(x,y,w,h);
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_circle(int x, int y, int r) {
    pthread_mutex_lock(&gUpdateMutex);
    gr_circle(x,y,r);
    pthread_mutex_unlock(&gUpdateMutex);
}

void ui_flip() {
    pthread_mutex_lock(&gUpdateMutex);
    gr_flip();
    pthread_mutex_unlock(&gUpdateMutex);
}

int ui_fb_width() {
    int width = 0;
    pthread_mutex_lock(&gUpdateMutex);
    width = gr_fb_width();
    pthread_mutex_unlock(&gUpdateMutex);
    return width;
}

int ui_fb_height() {
    int height = 0;
    pthread_mutex_lock(&gUpdateMutex);
    height = gr_fb_height();
    pthread_mutex_unlock(&gUpdateMutex);
    return height;
}

static int textview_redraw(struct textview *tv)
{
    int rc = 0;
    //char buf[512];    
    //char *ptr;
    const char *string = NULL ;
    int row, len;
    int x, y, s;
    int line;
    //unsigned char r,g,b,a;

    pthread_mutex_lock(&gUpdateMutex);
    
    draw_background_locked(gCurrentIcon);


    /* background color */
    gr_color(0, 0, 0, 160);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());

    /* title text */
    if (tv->m_title) {
        /* title background color */
        gr_color(51, 52, 255, 160);
        gr_fill(0, 0, gr_fb_width(), CHAR_HEIGHT_ITEM);
        set_gr_color(tv->m_title->color);
        gr_text(gr_font_big, 0, CHAR_HEIGHT_ITEM, tv->m_title->string);
    }
    
    /* normal text */
    if (tv->m_text) {
        set_gr_color(tv->m_text->color);
        //string = ptr = tv->m_text->string;
        string = tv->m_text->string;
        len = strlen(string) + 1;
        row = 3;
        for(line = tv->m_start; line <tv->m_end; line++){
            gr_text(gr_font_small, 0, CHAR_HEIGHT_ITEM+5+(line+1)*CHAR_HEIGHT, tv->m_pstr[line]);	         
        }
    }

    /* coordinate text */
    if (tv->m_nr_ctexts) {
        int nr = tv->m_nr_ctexts;
        ctext_t *ctext = &tv->m_ctexts[0];
        while (nr-- > 0) {
            set_gr_color(ctext->color);
            gr_text(gr_font_small, ctext->x, ctext->y +CHAR_HEIGHT_ITEM, ctext->string);
            ctext++; 
        }
    }
    
    if (tv->m_left_btn || tv->m_center_btn || tv->m_right_btn) {
        /* bottom bar background color */    
        gr_color(51, 52, 255, 160);
        gr_fill(0, gr_fb_height() - 2 * CHAR_HEIGHT_ITEM, gr_fb_width(), gr_fb_height());

        /* bottom text */
        y = gr_fb_height() - (CHAR_HEIGHT_ITEM >> 1) - 1;
        s = gr_fb_width() / 6;

        if (tv->m_left_btn) {
            set_gr_color(tv->m_left_btn->color);
            x = 0;
            gr_text(gr_font_big,x, y, tv->m_left_btn->string);
        }
        if (tv->m_center_btn) {
            set_gr_color(tv->m_center_btn->color);
            //x = (gr_fb_width()>>1)-CHAR_WIDTH_ITEM ;
            //gr_text(gr_font_big, x, y, tv->m_center_btn->string);
            #if defined(SUPPORT_GB2312)
            x = (gr_fb_width()>>1)-(((strlen(tv->m_right_btn->string)>>1)*CHAR_WIDTH_ITEM)>>1);
            #else
            x = (gr_fb_width()>>1)-((strlen(tv->m_right_btn->string)*CHAR_WIDTH_ITEM)>>1);
            #endif
            gr_text(gr_font_big, x, y, tv->m_center_btn->string);
        }
        if (tv->m_right_btn) {
            set_gr_color(tv->m_right_btn->color);
            #if defined(SUPPORT_GB2312)
            x = gr_fb_width()-((strlen(tv->m_right_btn->string)>>1)*CHAR_WIDTH_ITEM) ;
            #else
            x = gr_fb_width()-(strlen(tv->m_right_btn->string)*CHAR_WIDTH_ITEM) ;
            #endif
            gr_text(gr_font_big, x, y, tv->m_right_btn->string);
        }
    }
    gr_flip();

    pthread_mutex_unlock(&gUpdateMutex);

    tv->m_redraw = 0;
    return rc;
}

static int textview_set_titile(struct textview *tv, text_t *title)
{
    LOGD(TAG "%s: \n", __FUNCTION__);
    tv->m_title = title;
    return 0;
}

static int textview_set_text(struct textview *tv, text_t *text)
{
    tv->m_text  = text;
    tv->m_redraw = 1;
    return 0;
}

static int textview_set_ctext(struct textview *tv, ctext_t *ctexts, int nr_ctexts)
{
    tv->m_ctexts = ctexts;
    tv->m_nr_ctexts = nr_ctexts;
    tv->m_redraw = 1;    
    return 0;
}

static int textview_set_btn(struct textview *tv, text_t *left, text_t *center, text_t *right)
{
    tv->m_left_btn   = left;
    tv->m_center_btn = center;
    tv->m_right_btn  = right;
    tv->m_redraw     = 1;
    return 0;
}

static int textview_exit(struct textview *tv)
{
    tv->m_exit = 1;
    return 0;
}

static int textview_run(struct textview *tv)
{
    int key, handled, rc = 0;

    ui_clear_key_queue();

    tv->m_redraw = 1;
    do {
        if (tv->m_redraw)
            tv->redraw(tv);

        key = ui_wait_key();

        //LOGD(TAG "KEY: %d\n", key);
        
        if (tv->m_khandler)
            handled = tv->m_khandler(key, tv->m_priv);
        if (tv->m_exit)
            break;
    } while (1);

    tv->m_exit = 0;
    
    return rc;    
}

static void itemview_update_locked(struct itemview *iv)
{
    int i = 0;
    int j = 0;
    int x = 0, y = 0;
    if(iv == NULL)
    {
        return;
    }
    i = iv->menu_from;


    draw_background_locked(gCurrentIcon);

    /* background color of view */
    gr_color(0, 0, 0, 160);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());

    /* background color of title bar */
    gr_color(51, 52, 255, 160);
    gr_fill(0, 0, gr_fb_width(), CHAR_HEIGHT_ITEM);

    /* title text */
    set_gr_color(COLOR_YELLOW);
    draw_menu_text(gr_font_big, 0, iv->menu[0], CHAR_HEIGHT_ITEM);

    if (iv->menu_items) {
        /* background color of selected bar */
        x = 0;
        y = CHAR_HEIGHT_ITEM * (iv->menu_top - iv->menu_from + iv->menu_sel + 1);
        gr_color(64, 96, 255, 160);
        gr_fill(x, y, gr_fb_width(), y + CHAR_HEIGHT_ITEM + 1);
        for (j = 1; i < iv->menu_top + iv->menu_items; ++i, ++j) {
            if (i == iv->menu_top + iv->menu_sel) {
                /* text color for selected item */
                set_gr_color(COLOR_YELLOW);
                draw_menu_text(gr_font_big,j, iv->menu[i], CHAR_HEIGHT_ITEM);                	
                gr_color(64, 96, 255, 160);
            } else {
                /* background color for other items */
                x = 0;
                y = (iv->menu_top - iv->menu_from + i) * CHAR_HEIGHT_ITEM;
                set_gr_color(iv->m_items[i - iv->menu_top].background);
                if(y < (gr_fb_height()-(UNAVAIL_ROW+1)*CHAR_HEIGHT_ITEM))
                {
                    gr_fill(x, y, gr_fb_width(), y + CHAR_HEIGHT_ITEM + 1);	
                }
                /* text for other items */
                gr_color(120, 120, 255, 160);
                if(j < ((gr_fb_height()/CHAR_HEIGHT_ITEM)-UNAVAIL_ROW))
                {
                    draw_menu_text(gr_font_big, j, iv->menu[i], CHAR_HEIGHT_ITEM);
                }
                
                gr_color(64, 96, 255, 160);
            }
            gr_fill(0, j * CHAR_HEIGHT_ITEM + CHAR_HEIGHT_ITEM / 2 - 1,
                    gr_fb_width(), 1);
        }       
        if(iv->menu_items-iv->menu_from>= maxrows)
        {
            j = maxrows+1;
        }
        else
        {
           j = iv->menu_items-iv->menu_from+2; 	
        }

        gr_fill(0, j * CHAR_HEIGHT_ITEM+CHAR_HEIGHT / 2 - 1,
                gr_fb_width(), j * CHAR_HEIGHT_ITEM + CHAR_HEIGHT / 2 + 1);
        ++i;
    }

    /* printed text color */
    set_gr_color(COLOR_YELLOW);
		int base_item = i;
		j = 0;
    for (i = 0; i < text_rows; i++)
    {
    	  if(iv->text[i][0] != '\0')
    	  {
    	      j++;
    	      gr_text(gr_font_small, 0, base_item*CHAR_HEIGHT_ITEM+j*CHAR_HEIGHT, iv->text[i]);
    	  }
		}
	
    if (iv->m_left_btn ||iv->m_center_btn|| iv->m_right_btn ) 
    {
        /* bottom bar background color */    
        gr_color(51, 52, 255, 160);
        gr_fill(0, gr_fb_height() - 2 * CHAR_HEIGHT_ITEM, gr_fb_width(), gr_fb_height());

        /* bottom text */
        y = gr_fb_height() - (CHAR_HEIGHT_ITEM >> 1) - 1;

        if (iv->m_left_btn) 
        {
            set_gr_color(iv->m_left_btn->color);
            x = 0;
            gr_text(gr_font_big,x, y, iv->m_left_btn->string);
        }
        if (iv->m_center_btn) 
        {
            set_gr_color(iv->m_center_btn->color);
            #if defined(SUPPORT_GB2312)
            x = (gr_fb_width()>>1)-(((strlen(iv->m_right_btn->string)>>1)*CHAR_WIDTH_ITEM)>>1);
            #else
            x = (gr_fb_width()>>1)-((strlen(iv->m_right_btn->string)*CHAR_WIDTH_ITEM)>>1);
            #endif
            gr_text(gr_font_big, x, y, iv->m_center_btn->string);
        }
        if (iv->m_right_btn) 
        {
            set_gr_color(iv->m_right_btn->color);
            #if defined(SUPPORT_GB2312)
            x = gr_fb_width()-((strlen(iv->m_right_btn->string)>>1)*CHAR_WIDTH_ITEM) ;
            #else
            x = gr_fb_width()-(strlen(iv->m_right_btn->string)*CHAR_WIDTH_ITEM) ;
            #endif
            gr_text(gr_font_big, x, y, iv->m_right_btn->string);
        }
    }
    
    gr_flip();
}

static void itemview_start_menu(struct itemview *iv, int item_sel)
{
    int i = 0;
    int top = 0;
    
    if(iv == NULL)
    {
        return;
    }

    text_t *title = iv->m_title;
    item_t *items = iv->m_items;

    pthread_mutex_lock(&gUpdateMutex);

    /* title text */
    if (title) {
		if(text_cols >= 1){
        strncpy(iv->menu[i], title->string, (text_cols-1)*2);
        iv->menu[i][(text_cols-1)*2] = '\0';
        i++;
    }
    }
    top = i;
    
    /* menu items text */
    for (; i < MAX_ROWS; ++i) {
        if (items[i - top].name == NULL) break;
        strncpy(iv->menu[i], items[i - top].name, MAX_COLS*2-1/*text_cols-1*/);
        iv->menu[i][MAX_COLS*2-1/*text_cols-1*/] = '\0';
    }
    
    iv->menu_sel     = item_sel;
    iv->menu_top     = top;
    iv->menu_from    = iv->menu_from == 0 ? top : iv->menu_from;
    iv->menu_items   = i - top;

    pthread_mutex_unlock(&gUpdateMutex);
}

int itemview_menu_select(struct itemview *iv, int sel) 
{
    int old_sel = 0;

    int top     = 0;
    if(iv == NULL)
    {
        return -1;
    }
    old_sel = iv->menu_sel;
    int maxrows = item_rows - iv->menu_top -UNAVAIL_ROW;
    top = iv->menu_top;


    pthread_mutex_lock(&gUpdateMutex);

    if (sel < 0) {
        iv->menu_sel  = iv->menu_items + sel;
        if (iv->menu_items >= maxrows) {
            //LOGD(TAG "[1]update menu_from: %d --> %d\n", 
//                iv->menu_from, top + iv->menu_sel - maxrows);
            iv->menu_from = top + iv->menu_sel - maxrows + ((iv->page_index)*(maxrows+1));
        }
    } else if (sel >= iv->menu_items) {
        iv->menu_sel  = top + sel - (iv->menu_items) - 1;
        iv->menu_from = top+((iv->page_index)*(maxrows+1));
        iv->round_flag = 1;
        iv->page_index = 0 ;

    } else {
        iv->menu_sel = sel;
        if (top + sel >= iv->menu_from) {
            if (top + sel - iv->menu_from  >= maxrows) {
                //LOGD(TAG "[2]update menu_from: %d --> %d\n", 
//                    iv->menu_from, top + sel - maxrows);
                iv->menu_from = top + sel - maxrows;
            }
        } else {
            iv->menu_from = top + sel+ ((iv->page_index)*(maxrows+1));
        }
    }
    //LOGD(TAG "sel=%d, maxrows=%d, menu_from=%d, menu_sel=%d\n", 
//        sel, maxrows, iv->menu_from, iv->menu_sel);
    
    if (iv->menu_sel != old_sel){
    	  if(1 == iv->round_flag)
    	 {
    	     iv->menu_from = top+((iv->page_index)*(maxrows+1));
    	     iv->round_flag = 0; 
    	 }
         itemview_update_locked(iv);
     }
    
    pthread_mutex_unlock(&gUpdateMutex);
    return iv->menu_sel;
}

static void itemview_end_menu(struct itemview *iv) 
{
    pthread_mutex_lock(&gUpdateMutex);
    itemview_update_locked(iv);
    pthread_mutex_unlock(&gUpdateMutex);
}

static int itemview_set_title(struct itemview *iv, text_t *title)
{
    //LOGD(TAG "%s\n", __FUNCTION__);
    iv->m_title = title;
    return 0;
}

static int itemview_set_items(struct itemview *iv, item_t *items, int selected)
{
    //LOGD(TAG "%s\n", __FUNCTION__);
    iv->m_items  = items;
    iv->menu_sel = selected;
    return 0;
}

static int itemview_set_text(struct itemview *iv, text_t *text)
{
    //LOGD(TAG "%s\n", __FUNCTION__);
    iv->m_text = text;
    return 0;
}

static int itemview_redraw(struct itemview *iv)
{
    const char *ptr = NULL ;
    int text_row = 0;
    int text_col = iv->text_col;
    text_t *ptext = iv->m_text;
    int word_cnt = 0;
    //LOGD(TAG "%s\n", __FUNCTION__);

    int temp_text_cols = 0;
    #if defined MTK_FACTORY_MODE_IN_GB2312
    temp_text_cols = text_cols *2 ;
    #else
    temp_text_cols = text_cols ;
    #endif

    pthread_mutex_lock(&gUpdateMutex);
    memset(iv->text, 0, sizeof(iv->text));
    if (ptext) {
        for (ptr = ptext->string; *ptr != '\0'; ++ptr)
        {
            if (*ptr == '\n' || word_cnt  >= temp_text_cols)
            {

                iv->text[text_row][text_col] = '\0';
                text_col = 0;
                word_cnt = 0;
                text_row = (text_row + 1) % text_rows;
                if (text_row == iv->text_top)
                {
                    iv->text_top = (iv->text_top + 1) % text_rows;
                }
            }

            if (*ptr != '\n') {
             if(((*ptr)&0x80) != 0)
             {
               iv->text[text_row][text_col++] = *ptr;
               iv->text[text_row][text_col++] = *(++ptr);

                       word_cnt += 2;
             }
             else
             {
               iv->text[text_row][text_col++] = *ptr;

                     word_cnt += 1;
             }
                // word_cnt += 2;
            }
        }
    }
    iv->text[text_row][text_col] = '\0';
    iv->text_col = text_col;
    iv->text_row = text_row;
    itemview_update_locked(iv);
    pthread_mutex_unlock(&gUpdateMutex);

    return 0;
}


static int itemview_set_btn(struct itemview *iv, text_t *left, text_t *center, text_t *right)
{
    if(iv == NULL)
    {
          return -1;
    }

    iv->m_left_btn   = left;
    iv->m_right_btn  = right;
    iv->m_center_btn = center;
    iv->m_redraw     = 1;
    return 0;	
}

static int itemview_exit(struct itemview *iv)
{
    /* clear text */
    memset(iv->text, 0, sizeof(char) * MAX_COLS * MAX_ROWS);
    iv->text_col = iv->text_row = 0;

    return 0;
}

static int itemview_run(struct itemview *iv, bool *pexit)
{
    int key = KEY_RESERVED;
    int selected = 0;
    int chosen_item = -1 ;
    int exit = 0 ;
    int count = 3000 ;
    int longpressed = 0;

    //LOGD(TAG "%s %p\n", __FUNCTION__, iv);

    if(iv == NULL)
    {
        return -1;
    }

    selected = iv->menu_sel;

    ui_clear_key_queue();
    itemview_start_menu(iv, selected);
    itemview_redraw(iv);

    while (chosen_item < 0 && !exit) {
    	  if(0 == g_touch)
        {
        	  g_touch = -1;
    	      if(1 == g_RBTN)
            {
                g_RBTN = 0;
                return R_BTN_DOWN;   
            }
            if(1 == g_LBTN)
            {
                g_LBTN = 0;
                return L_BTN_DOWN;
            }
            if(1 == g_CBTN)
            {
                g_CBTN = 0;
                return C_BTN_DOWN;
            }
            if((g_y < (iv->menu_items > maxrows ? maxrows+1:iv->menu_items+1)*CHAR_HEIGHT_ITEM)&&(g_y > CHAR_HEIGHT_ITEM))
        	  {
                selected = g_y/CHAR_HEIGHT_ITEM-1;
                selected = selected+iv->menu_from-1;
                if(selected <= iv->menu_items-1)
                {
                    selected = itemview_menu_select(iv,selected);
                    chosen_item = iv->m_items[selected].id ;
                    break;
                }
            }
	
        }
        if (key==BTN_TOUCH || !ui_key_pressed(key) || !longpressed) 
        {
            key = ui_wait_key();
            count = 3000;
            longpressed = 0;
        }
        
        #ifdef FEATURE_FTM_TWO_KEY 
        while (ui_key_pressed(key)) 
        {
            usleep(1000);
            count--;
            if(0 == count)
            {
                longpressed = 1;
                break;
            }
        }
        if(count>0)
        {
            longpressed = 0;	
        }
        count = 3000 ;
        #endif
        
        switch (key) 
        {
            case UI_KEY_BACK:
                //LOGD(TAG "KEY:BACK\n");
                exit = true;
                break;
          
            case UI_KEY_UP:
            case UI_KEY_VOLUP:
                //LOGD(TAG "KEY:UP\n");
                --selected;
                selected = itemview_menu_select(iv, selected);
                break;
            case UI_KEY_DOWN:
            case UI_KEY_VOLDOWN:
                #ifdef FEATURE_FTM_TWO_KEY
            	if(1 == longpressed)
            	{
            	  //LOGD(TAG "KEY:BACK long pressed \n");
                exit = true;	
            	}	
            	else
            	{
            	#endif
            		//LOGD(TAG "KEY:DOWN\n");
                ++selected;
                selected = itemview_menu_select(iv, selected);
                #ifdef FEATURE_FTM_TWO_KEY
            	}
                #endif
                break;        
            case UI_KEY_CONFIRM:
                //LOGD(TAG "KEY:CONFIRM\n");
                chosen_item = iv->m_items[selected].id;
                break;
            default:
                //LOGD(TAG "KEY: %d\n", key);
                break;
        }           
    };

    iv->menu_sel = selected;    
    itemview_end_menu(iv);

    if (exit)
        iv->exit(iv);

    if (pexit)
        *pexit = exit;
    
    return chosen_item;
}

static int paintview_set_title(struct paintview *pv, text_t *title)
{
    pthread_mutex_lock(&gUpdateMutex);
    pv->m_title = title;
    pv->m_redraw = 1;    
    pthread_mutex_unlock(&gUpdateMutex);
    return 0;
}

static int paintview_set_btn(struct paintview *pv, text_t *left, text_t *center, text_t *right)
{
    pthread_mutex_lock(&gUpdateMutex);
    pv->m_left_btn   = left;
    pv->m_center_btn = center;
    pv->m_right_btn  = right;
    pv->m_redraw     = 1;
    pthread_mutex_unlock(&gUpdateMutex);    
    return 0;
}

static int paintview_set_point(struct paintview *pv, point_t *my_point, int nr_points)
{
    pthread_mutex_lock(&gUpdateMutex);
    pv->m_points = my_point;
    pv->m_nr_points = nr_points;
    pv->m_redraw = 1;
    pthread_mutex_unlock(&gUpdateMutex);
    return 0;
}

static int paintview_set_image(struct paintview *pv, image_t *my_img)
{
    pthread_mutex_lock(&gUpdateMutex);    
    pv->m_img = my_img; 
    pv->m_redraw = 1; 
    pthread_mutex_unlock(&gUpdateMutex);
    return 0;
}

static int paintview_set_line(struct paintview *pv, line_t *my_line, int nr_lines)
{
    pthread_mutex_lock(&gUpdateMutex);
    pv->m_lines = my_line;
    pv->m_nr_lines = nr_lines;
    pv->m_redraw = 1;
    pthread_mutex_unlock(&gUpdateMutex);
    return 0;
}

static int paintview_set_circle(struct paintview *pv, circle_t *my_circle, int nr_circles)
{
    pthread_mutex_lock(&gUpdateMutex);
    pv->m_circles = my_circle;
    pv->m_nr_circles = nr_circles;
    pv->m_redraw = 1;
    pthread_mutex_unlock(&gUpdateMutex);
    return 0;
}

static int paintview_drawpoint(struct paintview *pv)
{
    pthread_mutex_lock(&gUpdateMutex);
    if (pv->m_nr_points) 
    {    
        int nr = pv->m_nr_points;
        point_t *ppoint = &pv->m_points[0];
        while(nr--)
        {
            set_gr_color(ppoint->color);
            gr_point(ppoint->x, ppoint->y, ppoint->radius);
            ppoint++;
        }
    }
    pthread_mutex_unlock(&gUpdateMutex);    
    return 0;
}

static int paintview_drawimage(struct paintview *pv)
{
    int w, h;
    pthread_mutex_lock(&gUpdateMutex);
    if (pv->m_img!= NULL) 
    {    
        point_t point; 
        point.radius = 1; 
        unsigned char *pTemp = (unsigned char *)pv->m_img->surface;
        unsigned char r, g, b, a; 
        //color_t *pColor = (color_t*)pv->m_img->surface;
        for (h = 0; h < pv->m_img->y; h++)
        {
            for (w = 0; w < pv->m_img->x; w++)
            {
                 b = *pTemp++;
                 g = *pTemp++; 
                 r = *pTemp++; 
                 a= 0xFF; 
                 //set_gr_color(*pColor); 
                 gr_color(r, g , b, 0xff);
                 gr_point(w, h, 1);
            }
        }
    }
    pthread_mutex_unlock(&gUpdateMutex);    
    return 0;
}

static int paintview_drawline(struct paintview *pv)
{
    //LOGD(TAG "%s start\n", __FUNCTION__);
    //LOGD(TAG "pv->m_nr_lines = %d\n", pv->m_nr_lines);
    pthread_mutex_lock(&gUpdateMutex);
    if (pv->m_nr_lines) 
    {    
        int nr = pv->m_nr_lines;
        line_t *pline = &pv->m_lines[0];
        while(nr--)
        {
            set_gr_color(pline->color);
            gr_line(pline->sx, pline->sy, pline->dx, pline->dy, pline->width);
            pline++;
        }
    }   
    pthread_mutex_unlock(&gUpdateMutex);
    //LOGD(TAG "%s end\n", __FUNCTION__);
    return 0;
}

static int paintview_drawcircle(struct paintview *pv)
{
    pthread_mutex_lock(&gUpdateMutex);
    if (pv->m_nr_circles) 
    {    
        int nr = pv->m_nr_circles;
        circle_t *pcircle = &pv->m_circles[0];
        while(nr--)
        {
            set_gr_color(pcircle->color);
            gr_circle(pcircle->x, pcircle->y, pcircle->radius);
            pcircle++;
        }
    }
    pthread_mutex_unlock(&gUpdateMutex);    
    return 0;
}

static int paintview_run(struct paintview *pv)
{
    int key, handled, rc = 0;

    ui_clear_key_queue();
    
    //LOGD(TAG "%s start\n", __FUNCTION__);

    pv->m_redraw = 1;
    do {        
        if (pv->m_redraw)
            pv->redraw(pv);

        /* Try to paint something */      
        /* pv->drawpoint(pv);
        pv->drawline(pv);
        pv->drawcircle(pv);        
        pv->drawimg(pv);
        
        pv->flip(); */       

        key = ui_wait_key();

        //LOGD(TAG "KEY: %d\n", key);
        
        if (pv->m_khandler)
            handled = pv->m_khandler(key, pv->m_priv);
        if (pv->m_exit)
            break;
    } while (1);

    pv->m_exit = 0;
    
    return rc;    
}

int paintview_redraw(struct paintview *pv)
{
    int rc = 0;  
    int x = 0, y = 0, s = 0;
    if(pv == NULL)
    {
        return -1;
    }


    pthread_mutex_lock(&gUpdateMutex);
    
    draw_background_locked(gCurrentIcon);


    /* background color */
    gr_color(0, 0, 0, 160);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());

    /* title text */
    if (pv->m_title) {
        /* title background color */
        gr_color(51, 52, 255, 160);
        gr_fill(0, 0, gr_fb_width(), 2 * CHAR_HEIGHT_ITEM);
        set_gr_color(pv->m_title->color);
        gr_text(gr_font_small, 0, 1 * ((CHAR_HEIGHT_ITEM >> 1) + CHAR_HEIGHT_ITEM) - 1, pv->m_title->string);
    }  

    /* TODO: Need to be filled */
    
    if (pv->m_left_btn || pv->m_center_btn || pv->m_right_btn) {
        /* bottom bar background color */    
        gr_color(51, 52, 255, 160);
        gr_fill(0, gr_fb_height() - 2 * CHAR_HEIGHT_ITEM, gr_fb_width(), gr_fb_height());

        /* bottom text */
        y = gr_fb_height() - (CHAR_HEIGHT_ITEM >> 1) - 1;        
        s = gr_fb_width() / 6;

        if (pv->m_left_btn) {
            set_gr_color(pv->m_left_btn->color);
            x = 1 * s - (gr_measure(gr_font_small, pv->m_left_btn->string) >> 1);// * CHAR_WIDTH_ITEM;
            gr_text(gr_font_small, x, y, pv->m_left_btn->string);
        }
        if (pv->m_center_btn) {
            set_gr_color(pv->m_center_btn->color);
            x = 3 * s - (gr_measure(gr_font_small, pv->m_center_btn->string) >> 1);// * CHAR_WIDTH_ITEM;
            gr_text(gr_font_small, x, y, pv->m_center_btn->string);
        }
        if (pv->m_right_btn) {
            set_gr_color(pv->m_right_btn->color);
            x = 5 * s - (gr_measure(gr_font_small, pv->m_right_btn->string) >> 1);// * CHAR_WIDTH_ITEM;
            gr_text(gr_font_small, x, y, pv->m_right_btn->string);
        }
    }

    //gr_flip();

    pthread_mutex_unlock(&gUpdateMutex);
    
    pv->m_redraw = 0;
    return rc;

}

static int paintview_flip(void)
{
    pthread_mutex_lock(&gUpdateMutex);
    gr_flip();
    pthread_mutex_unlock(&gUpdateMutex);    
    return 0;
}

static int paintview_exit(struct paintview *pv)
{
    pv->m_exit = 1;

    /* TODO: Need to be filled */

    return 0;
}


static int imageview_set_image(struct imageview *imv, char *filename, int x, int y)
{
    int rc;

    if (imv->m_image.surface) {
        res_free_surface((gr_surface)imv->m_image.surface);
        imv->m_image.surface = NULL;
    }
    
    rc = res_create_surface(filename, (gr_surface*)&imv->m_image.surface);

    if (rc == 0) {
        imv->m_image.x = x;
        imv->m_image.y = y;
    }

    return rc;
}

static int imageview_redraw(struct imageview *imv)
{
	  int rc = 0;
    int x = 0, y = 0, s = 0;

    if(imv == NULL)
    {
        return -1;
    }


    pthread_mutex_lock(&gUpdateMutex);
    
    draw_background_locked(gCurrentIcon);


    /* background color */
    gr_color(0, 0, 0, 160);
    gr_fill(0, 0, gr_fb_width(), gr_fb_height());

    /* title text */
    if (imv->m_title) {
        /* title background color */
        gr_color(51, 52, 255, 160);
        gr_fill(0, 0, gr_fb_width(), 2 * CHAR_HEIGHT_ITEM);
        set_gr_color(imv->m_title->color);
        gr_text(gr_font_small, 0, 1 * ((CHAR_HEIGHT_ITEM >> 1) + CHAR_HEIGHT_ITEM) - 1, imv->m_title->string);
    }
    
    if (imv->m_image.surface) {
        int iconWidth = gr_get_width(imv->m_image.surface);
        int iconHeight = gr_get_height(imv->m_image.surface);
        int iconX = (gr_fb_width() - iconWidth) / 2;
        int iconY = (gr_fb_height() - iconHeight) / 2;

        gr_blit(imv->m_image.surface, imv->m_image.x, imv->m_image.y, 
            iconWidth, iconHeight, iconX, iconY);
    }
    
    if (imv->m_left_btn || imv->m_center_btn || imv->m_right_btn) {
        /* bottom bar background color */    
        gr_color(51, 52, 255, 160);
        gr_fill(0, gr_fb_height() - 2 * CHAR_HEIGHT_ITEM, gr_fb_width(), gr_fb_height());

        /* bottom text */
        y = gr_fb_height() - (CHAR_HEIGHT_ITEM >> 1) - 1;        
        s = gr_fb_width() / 6;

        if (imv->m_left_btn) {
            set_gr_color(imv->m_left_btn->color);
            x = 1 * s - (strlen(imv->m_left_btn->string) >> 1) * CHAR_WIDTH_ITEM;
            gr_text(gr_font_small, x, y, imv->m_left_btn->string);
        }
        if (imv->m_center_btn) {
            set_gr_color(imv->m_center_btn->color);
            x = 3 * s - (strlen(imv->m_center_btn->string) >> 1) * CHAR_WIDTH_ITEM;
            gr_text(gr_font_small, x, y, imv->m_center_btn->string);
        }
        if (imv->m_right_btn) {
            set_gr_color(imv->m_right_btn->color);
            x = 5 * s - (strlen(imv->m_right_btn->string) >> 1) * CHAR_WIDTH_ITEM;
            gr_text(gr_font_small, x, y, imv->m_right_btn->string);
        }
    }
    gr_flip();

    pthread_mutex_unlock(&gUpdateMutex);
    
    imv->m_redraw = 0;
    return rc;

}

static int imageview_set_title(struct imageview *imv, text_t *title)
{
    pthread_mutex_lock(&gUpdateMutex);
    imv->m_title = title;
    imv->m_redraw = 1;    
    pthread_mutex_unlock(&gUpdateMutex);
    return 0;
}

static int imageview_set_btn(struct imageview *imv, text_t *left, text_t *center, text_t *right)
{
    pthread_mutex_lock(&gUpdateMutex);
    imv->m_left_btn   = left;
    imv->m_center_btn = center;
    imv->m_right_btn  = right;
    imv->m_redraw     = 1;
    pthread_mutex_unlock(&gUpdateMutex);    
    return 0;
}

static int imageview_exit(struct imageview *imv)
{
    imv->m_exit = 1;

    if (imv->m_image.surface) {
        res_free_surface((gr_surface)imv->m_image.surface);
        imv->m_image.surface = NULL;
    }

    return 0;
}

static int imageview_run(struct imageview *imv)
{
    int key, handled, rc = 0;

    ui_clear_key_queue();

    imv->m_redraw = 1;
    do {
        if (imv->m_redraw)
            imv->redraw(imv);

        key = ui_wait_key();

        //LOGD(TAG "KEY: %d\n", key);
        
        if (imv->m_khandler)
            handled = imv->m_khandler(key, imv->m_priv);
        if (imv->m_exit)
            break;
    } while (1);

    imv->m_exit = 0;
    
    return rc;    
}

int ui_init_itemview(struct itemview *iv)
{
    if (iv) {
        memset(iv, 0, sizeof(struct itemview));
        iv->set_title = itemview_set_title;
        iv->set_items = itemview_set_items;
        iv->set_text  = itemview_set_text;
        iv->run       = itemview_run;
        iv->redraw    = itemview_redraw;
        iv->exit      = itemview_exit;
        iv->start_menu= itemview_start_menu;
        iv->set_btn   = itemview_set_btn;
    }
    return 0;
}

struct itemview *ui_new_itemview(void)
{
    struct itemview *iv;

    iv = malloc(sizeof(struct itemview));
    ui_init_itemview(iv);

    return iv;
}

void ui_free_itemview(struct itemview **iv)
{
    free(*iv);    
    *iv = NULL;
    return;
}

int ui_init_textview(struct textview *tv, khandler handler, void *priv)
{
    if (tv) {
        memset(tv, 0, sizeof(struct textview));
        tv->m_khandler   = handler;
        tv->m_priv       = priv;
        tv->set_title    = textview_set_titile;
        tv->set_text     = textview_set_text;
        tv->set_ctext    = textview_set_ctext;
        tv->set_btn      = textview_set_btn;
        tv->run          = textview_run;
        tv->redraw       = textview_redraw;
        tv->exit         = textview_exit;
    }
    return 0;
}

int ui_init_imageview(struct imageview *imv, khandler handler, void *priv)
{
    if (imv) {
        memset(imv, 0, sizeof(struct imageview));
        imv->m_khandler   = handler;
        imv->m_priv       = priv;
        imv->set_title    = imageview_set_title;
        imv->set_image    = imageview_set_image;
        imv->set_btn      = imageview_set_btn;
        imv->run          = imageview_run;
        imv->redraw       = imageview_redraw;
        imv->exit         = imageview_exit;
    }
    return 0;

}

int ui_init_paintview(struct paintview *pv, khandler handler, void *priv)
{
    if (pv) {
        memset(pv, 0, sizeof(struct paintview));
        pv->m_khandler   = handler;
        pv->m_priv       = priv;
        pv->set_title    = paintview_set_title;
        pv->set_btn      = paintview_set_btn;
        pv->set_point    = paintview_set_point;
        pv->set_line     = paintview_set_line;
        pv->set_circle   = paintview_set_circle;
        pv->set_img      = paintview_set_image; 
        pv->run          = paintview_run;
        pv->redraw       = paintview_redraw;
        pv->exit         = paintview_exit;
        pv->drawpoint    = paintview_drawpoint;
        pv->drawline     = paintview_drawline;
        pv->drawcircle   = paintview_drawcircle;        
        pv->drawimg      = paintview_drawimage;
        pv->flip         = paintview_flip;
    }
    return 0;
}

/* get the CUST_KEY_CONFIRM from cust_keys.h */
int get_confirm_key(void)
{
	return UI_KEY_CONFIRM;
}

int paintview_key_handler(int key, void *priv)
{
	int handled = 0;
	struct paintview *pv;
    if(priv == NULL)
    {
        return -1;
    }

	pv = (struct paintview *)priv;

	switch (key) 
    {
    	case UI_KEY_BACK:
#ifdef FEATURE_FTM_TWO_KEY
		case UI_KEY_VOLDOWN:
#endif
    		pv->exit(pv);
    		break;
    	default:
    		handled = -1;
    		break;
	}
	return handled;
}
 
int textview_key_handler(int key, void *priv)
{
	int handled = 0;
    #ifdef FEATURE_FTM_TWO_KEY
    int count = 3000;
    int longpressed = 0;
	#endif
    int avail_lines = get_avail_textline();
    struct textview *tv;
    if(priv == NULL)
    {
        return -1;
    }
  
	tv = (struct textview *)priv;
	#ifdef FEATURE_FTM_TOUCH_MODE
    if(g_RBTN == 1)
    {
  	    g_RBTN = -1;
        tv->exit(tv);   	
    }
    #endif
    #ifdef FEATURE_FTM_TWO_KEY
    while (ui_key_pressed(key)) 
    {
        usleep(1000);
        count--;
	    if(count == 0) 
        {
            longpressed = 1;
            break ;
        }
    }
    if(count>0)
    {
        longpressed = 0;
    }
    count = 3000 ;
    #endif
  
	switch (key) 
    {   
    	case UI_KEY_BACK:
    		tv->exit(tv);
    		break;
    	
    	case UI_KEY_UP:
    	case UI_KEY_VOLUP:
    		//LOGD(TAG "textview_key_handler: key up\n");
    		//LOGD(TAG "textview_key_handler: avail_lines=%d, tv->m_nr_lines=%d, tv->m_start=%d, tv->m_end=%d,", \
    			//avail_lines, tv->m_nr_lines, tv->m_start, tv->m_end);
    		if (avail_lines < tv->m_nr_lines && tv->m_start > 0) 
			{
    			tv->m_start--;
    			tv->m_end--;
    			tv->m_redraw = 1;
    			//LOGD(TAG "textview_key_handler: key up redraw\n");
    		}
    		break;
    	case UI_KEY_DOWN:
    	case UI_KEY_VOLDOWN:
            #ifdef FEATURE_FTM_TWO_KEY
    		if(1 == longpressed)
    		{
    		    tv->exit(tv);
    		}
    		else
    		{
    		#endif
    		    //LOGD(TAG "textview_key_handler: key down\n");
    		    //LOGD(TAG "textview_key_handler: avail_lines=%d, tv->m_nr_lines=%d, tv->m_start=%d, tv->m_end=%d,", \
    		    //avail_lines, tv->m_nr_lines, tv->m_start, tv->m_end);
    		    if (avail_lines < tv->m_nr_lines && tv->m_end < tv->m_nr_lines) 
			    {
    		        tv->m_start++;
    		        tv->m_end++;
    		        tv->m_redraw = 1;
    		        //LOGD(TAG "textview_key_handler: key down redraw\n");
                }
            #ifdef FEATURE_FTM_TWO_KEY
            }
            #endif
    		break;
    	default:
    		handled = -1;
    		break;
	}
	return handled;
}

int imageview_key_handler(int key, void *priv)
{
	int handled = 0;
    struct imageview *imv;
    if(priv == NULL)
    {
        return -1;
    }
	imv = (struct imageview *)priv;

	switch (key) 
    {
    	case UI_KEY_BACK:
    		imv->exit(imv);
    		break;
    	default:
    		handled = -1;
    		break;
	}
	return handled;
}
