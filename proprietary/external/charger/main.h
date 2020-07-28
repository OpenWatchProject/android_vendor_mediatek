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

#ifndef _MAIN_H
#define _MAIN_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/inotify.h>
#include <sys/limits.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <linux/input.h>
#include <linux/ioctl.h>
#include <errno.h>
#include <hardware_legacy/uevent.h>
#include <ctype.h>
#include <cutils/properties.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <bootlogo.h>
#include <utils/Log.h>
#include <cutils/klog.h>

#include <dlfcn.h>
#include <sys/reboot.h>
#include <sys/resource.h>

// Use light HAL
#include <android/hardware/light/2.0/ILight.h>
// Use health HAL
#include <android/hardware/health/2.0/IHealth.h>
#include <healthhalutils/HealthHalUtils.h>

using ::android::hardware::light::V2_0::Brightness;
using ::android::hardware::light::V2_0::Flash;
using ::android::hardware::light::V2_0::ILight;
using ::android::hardware::light::V2_0::LightState;
using ::android::hardware::light::V2_0::Status;
using ::android::hardware::light::V2_0::Type;

using ::android::hardware::health::V2_0::get_health_service;
using ::android::hardware::health::V2_0::HealthInfo;
using ::android::hardware::health::V2_0::IHealth;
using ::android::hardware::health::V2_0::Result;
using ::android::sp;

#if 0
#define KPOC_LOGI(x...) ALOGI(x)
#define KPOC_LOGE(x...) ALOGE(x)
#define KPOC_LOGD(x...) ALOGD(x)
#else
#define KPOC_LOGI(x...) do { KLOG_ERROR("charger", x); } while (0)
// HACK: Sprout drops previous console logs below log level 4
#define KPOC_LOGE(x...) do { KLOG_WARNING("charger", x); } while (0)
#define KPOC_LOGD(x...) do { KLOG_DEBUG("charger", x); } while (0)
#endif

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "CHARGER"
#endif

#define ARRAY_SIZE(a) (sizeof (a) / sizeof ((a)[0]))
#define VBAT_POWER_ON_DEFAULT  3450000
#define VBAT_POWER_OFF_DEFAULT 3400000

#define LCD_BL_LEVEL 150
#define LCD_LOW_BAT_BL_LEVEL 20

#define RTC_DEV_PATH "/dev/rtc0"
#define ALARM_DEV_PATH "/dev/alarm"

/* ignore case */
static const char * keypad_device_name[] =
{
	"kpd",
	"keypad",
	"pmic-keys",
};

enum {
    EXIT_POWER_UP         = 0,
    EXIT_REBOOT_UBOOT     = 1,
    EXIT_ERROR_SHUTDOWN   = 2,
    EXIT_ALARM_BOOT		   = 5,
    EXIT_CHARGING_MODE     = 8,
    EXIT_NORMAL_SHUTDOWN   = 10,
    EXIT_AMOUNT            = 11,
};

enum {
    CONTROL_UNKNOWN        = 0,
    CONTROL_ON             = 1,
    CONTROL_OFF            = 2,
};

// do not change the order
enum {
    LIGHTS_INIT        = 0,
    LIGHTS_CHGFULL     = 1,
    LIGHTS_CHGON       = 2,
    LIGHTS_CHGEXIT     = 3,
    LIGHTS_FUNC_NUM    = 4,
};

enum {
    LIGHTS_STATE_UNKNOWN = 0,
    LIGHTS_STATE_CHGFULL = 1,
    LIGHTS_STATE_CHGON   = 2,
    LIGHTS_STATE_EXIT    = 3,
};

enum {
    TRIGGER_ANIM_KEY         = 8,
    TRIGGER_AMOUNT           = 10,
};

extern int showLowBattLogo;
extern pthread_mutex_t lights_mutex;
extern int VBAT_POWER_ON;
extern int VBAT_POWER_OFF;
extern sp<ILight> g_light;
extern sp<IHealth> g_health;

int get_int_value(const char * path);
void set_int_value(const char * path, const int value);
void health_service_init(void);
void light_init(void);
void set_light_brightness(Type light_type, LightState brightness);
int get_battnotify_status();
int is_wireless_charging();
void exit_charger(int reason);
int get_health_info(HealthInfo *info);

void key_control(int * pwrkeys, int pwrkeys_num);
void charging_control();

void start_charging_anim(int capacity);

void alarm_control();
int is_charging_source_available();

bool time_exceed(struct timeval start, int duration_sec);
int get_voltage();

void stop_backlight();
void start_backlight();

int lights_chgfull();
int lights_chgon();
int lights_chgexit();

#endif // _MAIN_H

#define VERBOSE_OUTPUT
