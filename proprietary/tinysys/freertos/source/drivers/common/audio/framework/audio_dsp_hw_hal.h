/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/
/* MediaTek Inc. (C) 2015. All rights reserved.
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
*/
#ifndef AUDIO_DSP_HW_LAYER_H
#define AUDIO_DSP_HW_LAYER_H

#include <stdio.h>
#include <audio_type.h>
#include <audio_task_utility.h>

struct AudioTask;
struct Audio_Task_Msg_t;
struct audio_hw_buffer;

enum {
    STATE_UNINIT = 0,
    STATE_IDLE   = 1,
    STATE_RUN    = 1 << 1,
    STATE_UNDERFLOW = 1 << 2,
    STATE_OVERFLOW  = 1 << 3,
};

enum {
       TRIGGER_STOP = 0,
       TRIGGER_START = 1,
       TRIGGER_PAUSE_PUSH =3,
       TRIGGER_PAUSE_RELEASE = 4,
       TRIGGER_SUSPEND = 5,
       TRIGGER_RESUME = 6,
       TRIGGER_DRAIN = 7,
};

/*
 * for User to use audio_dsp hal ops.
 */

int audio_dsp_hw_open(void *user, struct audio_hw_buffer *hw_buf);
int audio_dsp_hw_close(int handle);
int audio_dsp_hw_write(int handle, char *buf, unsigned int length);
int audio_dsp_hw_read(int handle, char *buf, unsigned int length);
int audio_dsp_hw_stop(int handle);
unsigned int audio_dsp_hw_getcur(int handle);
int audio_dsp_hw_getlen(int handle);
int audio_dsp_hw_irq(void *user, int handle);
int audio_dsp_hw_status(void *user, struct audio_hw_buffer *hw_buf);
int audio_dsp_hw_getavail(int handle);
int audio_dsp_hw_trigger(int handle, int cmd);
int audio_dsp_hw_update_ptr(int handle);

/*
 * for User to use audio_dsp fake ops.
 */

int audio_dsp_dummy_open(void *user, struct audio_hw_buffer *hw_buf);
int audio_dsp_dummy_write(int handle, char *buf, unsigned int length);
int audio_dsp_dummy_read(int handle, char *buf, unsigned int length);
int audio_dsp_dummy_stop(int handle);
int audio_dsp_dummy_close(int handle);
int audio_dsp_dummy_getcur(int handle);
int audio_dsp_dummy_getlen(int handle);
int audio_dsp_dummy_status(int handle);


/*
 * audio hw hal function
 * interface should not related to hw reg.
 */
struct audio_dsp_hw_hal_ops {
    int (*audio_dsp_hw_open_op)(void *user, struct audio_hw_buffer *hw_buf);
    int (*audio_dsp_hw_close_op)(int handle);
    int (*audio_dsp_hw_write_op)(int handle, char *buf, unsigned int length);
    int (*audio_dsp_hw_read_op)(int handle, char *buf, unsigned int length);
    int (*audio_dsp_hw_stop_op)(int handle);
    unsigned int (*audio_dsp_hw_getcur_op)(int handle);
    int (*audio_dsp_hw_getlen_op)(int handle);
    int (*audio_dsp_hw_getavail_op)(int handle);
    int (*audio_dsp_hw_status_op)(void *user, struct audio_hw_buffer *hw_buf);
    int (*audio_dsp_hw_trigger_op)(int handle, int cmd);
    int (*audio_dsp_hw_update_ptr_op)(int handle);
};

/*
 * audio hw related function
 */
struct audio_dsp_hw_ops {
    int (*audio_dsp_hw_irq_op)(void *priv, int id);
};

/* function with audio_dsp_ops */
int set_audio_dsp_hw_hal_ops(struct audio_dsp_hw_hal_ops *ops);
int set_audio_dsp_hw_ops(struct audio_dsp_hw_ops *ops);

#endif // end of AUDIO_DSP_HW_LAYER_H
