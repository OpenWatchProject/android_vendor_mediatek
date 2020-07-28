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

#include <audio_type.h>
#include <audio_ringbuf.h>
#include "audio_task_factory.h"
#include "audio_log.h"
#include "audio_errno.h"
#include "audio_dsp_hw_hal.h"

#include <audio_hw_type.h>

static struct audio_dsp_hw_hal_ops *s_audio_dsp_hw_hal_ops;
static struct audio_dsp_hw_ops *s_audio_dsp_hw_ops;

/* set ops should only init one time */
int set_audio_dsp_hw_hal_ops(struct audio_dsp_hw_hal_ops *ops)
{
    if (ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_hal_ops == NULL) {
        s_audio_dsp_hw_hal_ops = ops;
    }
    else {
        return -EOPS;
    }
    AUD_LOG_E("%s\n", __func__);

    return 0;
}

/* set ops should only init one time */
int set_audio_dsp_hw_ops(struct audio_dsp_hw_ops *ops)
{
    if (ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_ops == NULL) {
        s_audio_dsp_hw_ops = ops;
    }
    else {
        return -EOPS;
    }

    AUD_LOG_E("%s\n", __func__);
    return 0;
}


/*
 * for User to user audio_dsp hal ops.
 */

int audio_dsp_hw_open(void *user, struct audio_hw_buffer *hw_buf)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_open_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_open_op(user, hw_buf);
    return ret;
}

int audio_dsp_hw_write(int handle, char *buf, unsigned int length)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_write_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_write_op(handle, buf, length);
    return ret;
}

int audio_dsp_hw_read(int handle, char *buf, unsigned int length)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_read_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_read_op(handle, buf, length);
    return ret;
}

int audio_dsp_hw_stop(int handle)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_stop_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_stop_op(handle);
    return ret;
}


int audio_dsp_hw_close(int handle)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_close_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_close_op(handle);
    return ret;
}

unsigned int audio_dsp_hw_getcur(int handle)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_getcur_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_getcur_op(handle);
    return ret;
}

int audio_dsp_hw_getlen(int handle)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }
    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_getlen_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_getlen_op(handle);
    return ret;
}

int audio_dsp_hw_irq(void *user, int handle)
{
    int ret = 0;
    if (s_audio_dsp_hw_ops == NULL) {
        return -EOPS;
    }

    if (s_audio_dsp_hw_ops->audio_dsp_hw_irq_op == NULL) {
        return -EOPS;
    }

    ret = s_audio_dsp_hw_ops->audio_dsp_hw_irq_op(user, handle);
    return ret;
}

int audio_dsp_hw_status(void *user, struct audio_hw_buffer *hw_buf)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }
    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_status_op == NULL) {
        return -EOPS;
    }
    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_status_op(user, hw_buf);
    return ret;
}

int audio_dsp_hw_getavail(int handle)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }
    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_getavail_op == NULL) {
        return -EOPS;
    }
    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_getavail_op(handle);
    return ret;
}

int audio_dsp_hw_trigger(int handle, int cmd)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }
    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_trigger_op == NULL) {
        return -EOPS;
    }
    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_trigger_op(handle, cmd);
    return ret;
}

int audio_dsp_hw_update_ptr(int handle)
{
    int ret = 0;
    if (s_audio_dsp_hw_hal_ops == NULL) {
        return -EOPS;
    }
    if (s_audio_dsp_hw_hal_ops->audio_dsp_hw_update_ptr_op == NULL) {
        return -EOPS;
    }
    ret = s_audio_dsp_hw_hal_ops->audio_dsp_hw_update_ptr_op(handle);
    return ret;
}

static int dummy_write_count = 0;
int audio_dsp_dummy_open(void *user, struct audio_hw_buffer *hw_buf)
{
    dummy_write_count = 0;
    return 0;
}

int audio_dsp_dummy_write(int handle, char *buf, unsigned int length)
{
    if (dummy_write_count > 0)
        aud_task_delay(23);
    dummy_write_count++;
    return length;
}

int audio_dsp_dummy_read(int handle, char *buf, unsigned int length)
{
    aud_task_delay(23);
    return length;
}

int audio_dsp_dummy_stop(int handle)
{
    AUD_LOG_D("%s\n", __func__);
    return 0;
}

int audio_dsp_dummy_close(int handle)
{
    dummy_write_count = 0;
    return 0;
}

int audio_dsp_dummy_getcur(int handle)
{
    return 0;
}

int audio_dsp_dummy_getlen(int handle)
{
    return 0;
}

int audio_dsp_dummy_status(int handle){
    return 0;
}

