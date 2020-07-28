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

#include <sys/resource.h>
#include "main.h"

// add prefer power and backlight keys in below arrays.
int pwrkeys[] = {KEY_POWER, KEY_END};
sp<ILight> g_light;
sp<IHealth> g_health;

void exit_charger(int reason)
{
    KPOC_LOGI("exit_charger: %d \n", reason);

    switch(reason) {
        case EXIT_ALARM_BOOT:
        case EXIT_POWER_UP:
        case EXIT_REBOOT_UBOOT:
            reboot(RB_AUTOBOOT);
            break;
        case EXIT_CHARGING_MODE:
        case EXIT_ERROR_SHUTDOWN:
        case EXIT_NORMAL_SHUTDOWN:
        default:
            reboot(RB_POWER_OFF);
            break;
    }
}

int main(__attribute__((unused))int argc, __attribute__((unused))char *argv[])
{
    set_draw_anim_mode(1);
    pthread_mutex_init(&lights_mutex, NULL);

    setpriority(PRIO_PROCESS, 0, -20);
    FILE *oom_adj = fopen("/proc/self/oom_score_adj", "w");
    if (oom_adj) {
        fputs("-17", oom_adj);
        fclose(oom_adj);
    }

    health_service_init();
    light_init();
    //stop_backlight();
    KPOC_LOGI("stop_backlight 1\n");
    bootlogo_init();

    charging_control();

    unsigned int i;
    for (i=0; i< ARRAY_SIZE(pwrkeys); i++)
        KPOC_LOGI("pwrkeys[%d]:%d\n",i,pwrkeys[i]);

    key_control(pwrkeys, ARRAY_SIZE(pwrkeys)); //will loop inside

    return 0;
}
