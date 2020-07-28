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

#include "main.h"
#include <linux/fb.h>
#include <suspend/autosuspend.h>

#define UEVENT_BATTERY_CHANGE	"power_supply/batt"
#define BOOTMODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"

static int bc = 0;
static int firstTime = 0;
static int inDraw = 0;
static int nCurrentState = LIGHTS_STATE_UNKNOWN;
static int nChgAnimDuration_msec = 6000;
static int nCbInterval_msec = 200;
static int backlight_on = 1;

int VBAT_POWER_ON = VBAT_POWER_ON_DEFAULT;
int VBAT_POWER_OFF = VBAT_POWER_OFF_DEFAULT;
int lcd_backlight_level = LCD_BL_LEVEL;
pthread_mutex_t mutex, mutexlstate;
pthread_cond_t cond;

static int request_suspend(bool enable)
{
	if (enable)
		return autosuspend_enable();
	else
		return autosuspend_disable();
}

void health_service_init(void)
{
    g_health = get_health_service();
    if (g_health == nullptr) {
        KPOC_LOGI("Could not retrieve health service\n");
        return;
    }
}

void start_backlight()
{
    int val_bootmode = get_int_value(BOOTMODE_PATH);
    int vbat_microvolt;
    bool ac_online;
    int ret;
    HealthInfo info = {};

    ret = get_health_info(&info);
    if (ret < 0) {
        KPOC_LOGI("%s: cannot get HealthInfo\n", __func__);
        return;
    }

    ac_online = info.legacy.chargerAcOnline;
    vbat_microvolt = info.legacy.batteryVoltage * 1000;
    KPOC_LOGI("val_bootmode = %d, val_ac = %d\n", val_bootmode, ac_online);

    if ((val_bootmode == 9) && (ac_online == 0))
    {
#ifdef MTK_BATLOWV_NO_PANEL_ON_EARLY
        lcd_backlight_level = 0;
        if (vbat_microvolt >= VBAT_POWER_ON)
        {
            lcd_backlight_level = LCD_LOW_BAT_BL_LEVEL;
        }
#else
        lcd_backlight_level = LCD_LOW_BAT_BL_LEVEL;
#endif
    }

    LightState brightness = {
        .color = 0xff000000 | (lcd_backlight_level << 16)
                 | (lcd_backlight_level << 8) | lcd_backlight_level,
        .flashMode = Flash::NONE, .brightnessMode = Brightness::USER,
    };
    set_light_brightness(Type::BACKLIGHT, brightness);
}

void stop_backlight()
{
    LightState brightness = {
        .color = 0u, .flashMode = Flash::NONE, .brightnessMode = Brightness::USER,
    };
    set_light_brightness(Type::BACKLIGHT, brightness);

    backlight_on = 0;
}

static const char *chg_volt_path[] = {
    "/sys/devices/platform/charger/ADC_Charger_Voltage",
    "/sys/devices/platform/battery/ADC_Charger_Voltage",
};

int is_charging_source_available()
{
    bool usb_online, ac_online, wireless_online;
    int i = 0, vchr = 0;
    int ret = 0;
    HealthInfo info = {};

    ret = get_health_info(&info);
    if (ret < 0) {
        KPOC_LOGI("%s: cannot get HealthInfo\n", __func__);
        return 0;
    }

    usb_online = info.legacy.chargerUsbOnline;
    ac_online = info.legacy.chargerAcOnline;
    wireless_online = info.legacy.chargerWirelessOnline;

    /* Find path for charger voltage */
    for (i = 0; i < ARRAY_SIZE(chg_volt_path); i++) {
        vchr = get_int_value(chg_volt_path[i]);
        if (vchr != -1)
            break;
    }

    KPOC_LOGI("in %s(), usb:%d ac:%d wireless:%d vchr:%d\n", __func__,
            usb_online, ac_online, wireless_online, vchr);

    return (usb_online || ac_online || wireless_online || vchr >= 2500);
}

static unsigned int key_trigger_suspend = 0;
void trigger_anim()
{
	if(inDraw) {
		key_trigger_suspend = 1;
		return;
	}
	if (!is_charging_source_available()) {
		KPOC_LOGI("no charging source, skip drawing anim\n");
		return;
	}
	key_trigger_suspend = 0;
	pthread_cond_signal(&cond);
}

void start_charging_anim(int reason)
{
	KPOC_LOGI("%s: inDraw:%d, reason:%d\n",__FUNCTION__, inDraw, reason);
	trigger_anim();
}

int get_health_info(HealthInfo *info)
{
    if (g_health == nullptr) {
        KPOC_LOGI("%s: No health service, cannot get status\n", __func__);
        return -EINVAL;
    }

    auto ret = g_health->getHealthInfo([&](Result r, HealthInfo out) {
        if (r != Result::SUCCESS) {
            KPOC_LOGI("Cannot get HealthInfo, r=%d\n", r);
            return;
        }
        *info = out;
    });
    if (!ret.isOk()) {
        KPOC_LOGI("transaction error\n");
        return -EINVAL;
    }

    return 0;
}

static int get_health_capacity(int *capacity)
{
    if (g_health == nullptr) {
        KPOC_LOGI("%s: No health service, cannot get status\n", __func__);
        return -EINVAL;
    }

    auto ret = g_health->getCapacity([&](Result r, int value) {
        if (r != Result::SUCCESS) {
            KPOC_LOGI("Cannot get capacity, r=%d\n", r);
            return;
        }
        *capacity = value;
    });
    if (!ret.isOk()) {
        KPOC_LOGI("transaction error\n");
        return -EINVAL;
    }

    return 0;
}

static int get_capacity()
{
    int ret = 0;
    int bat_level = 0;

    do {
        ret = get_health_capacity(&bat_level);
        if (ret < 0) {
            KPOC_LOGI("%s: cannot get capacity\n", __func__);
            return 0;
        }

        usleep(100 * 1000);
        KPOC_LOGI("%s: bat_level: %d\n", __func__, bat_level);
    } while (bat_level == -1);

    return bat_level;
}

int get_voltage()
{
    int vbat_microvolt = 0;
    int ret = 0;
    HealthInfo info = {};

    ret = get_health_info(&info);
    if (ret < 0) {
        KPOC_LOGI("%s: cannot get HealthInfo\n", __func__);
        return 0;
    }

    vbat_microvolt = info.legacy.batteryVoltage * 1000;
    KPOC_LOGI("%s: batt_vol: %d\n", __func__, vbat_microvolt);

    return vbat_microvolt;
}

static void set_light_state(int state)
{
	pthread_mutex_lock(&mutexlstate);
	nCurrentState = state;
	pthread_mutex_unlock(&mutexlstate);
}

static int lights_full()
{
	set_light_state(LIGHTS_STATE_CHGFULL);
	lights_chgfull();
	return 0;
}

//return 1: leave, 0: chgon
static int lights_on()
{
	int leave = false;

	pthread_mutex_lock(&mutexlstate);
	if (nCurrentState != LIGHTS_STATE_CHGON)
		leave = true;
	pthread_mutex_unlock(&mutexlstate);

	if (!leave) {
		lights_chgon();
		return 0;
	}
	return 1;
}

static int lights_exit()
{
	set_light_state(LIGHTS_STATE_EXIT);
	lights_chgexit();
	return 0;
}

static int on_uevent(const char *buf, __attribute__((unused))int len_buf)
{
#ifdef VERBOSE_OUTPUT
	KPOC_LOGI("on_uevent, %s\n", buf);
#endif
	if (!strcasestr(buf, UEVENT_BATTERY_CHANGE))
		return 1;

	//if ac or usb online
	if (is_charging_source_available())
	{
		bc = get_capacity();

		if (bc >= 90) {
			lights_full();
		} else {
			if (nCurrentState != LIGHTS_STATE_CHGON)
                set_light_state(LIGHTS_CHGON);
			lights_on();
		}
	}
	else
        exit_charger(EXIT_CHARGING_MODE);

	return 1;
}

static void* uevent_thread_routine(__attribute__((unused))void *arg)
{
	char buf[1024];
	int len;

	if (!uevent_init())
	{
		KPOC_LOGI("uevent_init failed.\n");
		return 0;
	}

	while (1)
	{
		len = uevent_next_event(buf, sizeof(buf) - 1);
		if (len > 0) {
			if (!on_uevent(buf, len))
				break;
		}
	}
	pthread_exit(NULL);
	return NULL;
}

static void __attribute__((unused)) exit_charing_thread()
{
	inDraw = 0;
	pthread_exit(NULL);
}

// total_time : ms
// interval : ms
static void draw_with_interval(void (*func)(int, int), int bc, int total_time_msec, int interval_msec)
{
	struct timeval start;
	int resume_started = 0, backlight_started = 0, cnt = 0;
	int fd_fb, err = 0;
	char filename[32] = {0};
	gettimeofday(&start, NULL);

	while(!time_exceed(start, total_time_msec) && !key_trigger_suspend)
	{
        // check if need to draw animation before performing drawing
		if (!is_charging_source_available())
			return;
		if (!resume_started) {
			resume_started = 1;
			request_suspend(false);
			/* make fb unblank */
			snprintf(filename, sizeof(filename), "/dev/graphics/fb0");
			fd_fb = open(filename, O_RDWR);
			if (fd_fb < 0) {
				KPOC_LOGI("Failed to open fb0 device: %s", strerror(errno));
			}
			err = ioctl(fd_fb, FBIOBLANK, FB_BLANK_UNBLANK);
			if (err < 0) {
				KPOC_LOGI("Failed to unblank fb0 device: %s", strerror(errno));
			}
			if (fd_fb >= 0)
				close(fd_fb);
		}

		func(bc, ++cnt);
		if (!backlight_started) {
			backlight_started = 1;
			usleep(1000);
			start_backlight();
		}
		KPOC_LOGI("draw_with_interval... key_trigger_suspend = %d\n",key_trigger_suspend);
		usleep(interval_msec*1000);
	}
}

static int wait_until(int (*func)(void), int total_time_msec, int interval_msec){
	struct timeval start;
	gettimeofday(&start, NULL);

    while(!time_exceed(start, total_time_msec)){
        if(func()){
            return 1;
        }
		usleep(interval_msec*1000);
    }
    return 0;
}

#define charging_source_waiting_duration_ms 3000
#define charging_source_waiting_interval_ms 200

static void* draw_thread_routine(__attribute__((unused))void *arg)
{
	int bc;
	int fd_fb, err =0;
	char filename[32] = {0};

	do {
		KPOC_LOGI("draw thread working2...\n");
        // move here to avoid suspend when syncing with surfaceflinger

        if(firstTime){
            // make sure charging source online when in KPOC mode
            // add 2s tolerance
            if(wait_until(is_charging_source_available,
                        charging_source_waiting_duration_ms,
                        charging_source_waiting_interval_ms))
            {
                KPOC_LOGI("wait until charging source available\n");
            }else{
                KPOC_LOGI("charging source not available for %d ms at KPOC starup\n",
                        charging_source_waiting_duration_ms);
            }
            firstTime = 0;
        }

		inDraw = 1;

		// check the bc offest value
		bc = get_capacity();
		draw_with_interval(bootlogo_show_charging, bc, nChgAnimDuration_msec, nCbInterval_msec);
		stop_backlight();

        // @@@ draw fb again to refresh ddp
        bootlogo_show_charging(bc, 1);

		/* make fb blank */
		snprintf(filename, sizeof(filename), "/dev/graphics/fb0");
		fd_fb = open(filename, O_RDWR);
		if (fd_fb < 0) {
			KPOC_LOGI("Failed to open fb0 device: %s", strerror(errno));
			break;
		}
		err = ioctl(fd_fb, FBIOBLANK, FB_BLANK_POWERDOWN);
		if (err < 0) {
			KPOC_LOGI("Failed to blank fb0 device: %s", strerror(errno));
		}
		if (fd_fb >= 0)
			close(fd_fb);
		request_suspend(true);

		inDraw = 0;

        pthread_mutex_lock(&mutex);
		pthread_cond_wait(&cond, &mutex);
		pthread_mutex_unlock(&mutex);
	} while(1);
	pthread_exit(NULL);
	return NULL;
}

void charging_control()
{
	int ret = 0;
	pthread_attr_t attr, attrd, attrl;
	pthread_t uevent_thread, draw_thread;

	//charging led control
	if (!is_charging_source_available()) {
		lights_exit();
	}

	pthread_mutex_init(&mutexlstate, NULL);

	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&cond, NULL);

	pthread_attr_init(&attr);
	pthread_attr_init(&attrd);
	pthread_attr_init(&attrl);

	inDraw = 0;

	ret = pthread_create(&uevent_thread, &attr, uevent_thread_routine, NULL);
	if (ret != 0)
	{
		KPOC_LOGI("create uevt pthread failed.\n");
		exit_charger(EXIT_ERROR_SHUTDOWN);
	}

	firstTime = 1;
	ret = pthread_create(&draw_thread, &attrd, draw_thread_routine, NULL);
	if (ret != 0)
	{
		KPOC_LOGI("create draw pthread failed.\n");
		exit_charger(EXIT_ERROR_SHUTDOWN);
	}
}
