/*
 * viatelutils.c
 *
 * VIA CBP funtion for Linux
 *
 * Copyright (C) 2012 VIA TELECOM Corporation, Inc.
 * Author: qli@via-telecom.com
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/**************************/
/* INCLUDES               */
/**************************/
#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <cutils/properties.h>
#include <sys/sysinfo.h>

#define LOG_TAG "ViatelUtils"
#include <utils/Log.h>
#include <c2kutils.h>

#include <c2k_log.h>

#define PATH_NAME_LEN  PROPERTY_VALUE_MAX
#define SYSFS_UART_DRIVER       "/sys/class/tty/"
#define SYSFS_USB_DRIVER_OLD    "/sys/bus/usb/drivers/option/"
#define SYSFS_USB_DRIVER_NEW    "/sys/bus/usb/drivers/via-cbp/"
#define SYSFS_SDIO_DRIVER    "/sys/bus/sdio/drivers/modem_sdio/"

#define SYSFS_MODEM_POWER		"/sys/viatel/modem/power"
#define SYSFS_MODEM_RESET		"/sys/viatel/modem/reset"
#define VMODEM_DEVICE_PATH		"/dev/vmodem"
typedef struct device_type_property{
    int id;
    char *prop;
}device_type_property;

static device_type_property prop_list[VIATEL_CHANNEL_NUM] = {
    {VIATEL_CHANNEL_AT,   "vendor.viatel.device.at"},
    {VIATEL_CHANNEL_DATA, "vendor.viatel.device.data"},
    {VIATEL_CHANNEL_ETS,  "vendor.viatel.device.ets"},
    {VIATEL_CHANNEL_GPS,  "vendor.viatel.device.gps"},
    {VIATEL_CHANNEL_PCV,  "vendor.viatel.device.pcv"},
    {VIATEL_CHANNEL_ASCI, "vendor.viatel.device.asci"},
    {VIATEL_CHANNEL_FLS,  "vendor.viatel.device.fls"},
    {VIATEL_CHANNEL_MUX,  "vendor.viatel.device.mux"},
    {VIATEL_CHANNEL_AT2,  "vendor.viatel.device.at2"},
    {VIATEL_CHANNEL_AT3,  "vendor.viatel.device.at3"},
    {VIATEL_CHANNEL_MDLOG_DATA,  "vendor.viatel.device.mdlog.data"},
    {VIATEL_CHANNEL_EXCP_MSG,	"vendor.viatel.device.excp.msg"},
    {VIATEL_CHANNEL_EXCP_DATA,	"vendor.viatel.device.excp.data"},
    {VIATEL_CHANNEL_AT4,  "vendor.viatel.device.at4"},
    {VIATEL_CHANNEL_AT5,  "vendor.viatel.device.at5"},
    {VIATEL_CHANNEL_AT6,  "vendor.viatel.device.at6"},
    {VIATEL_CHANNEL_AT7,  "vendor.viatel.device.at7"},
    {VIATEL_CHANNEL_AT8,  "vendor.viatel.device.at8"},
};

typedef struct device_type_adjust {
    char *type;
    char *(*adjust)(int num, char *dev);    
}device_type_adjust;

static void print_version_info(void)
{
    LOGD("Viatel Utils suffix Version: %s\n", VIA_SUFFIX_VERSION);
    return;
}

static char * uart_adjust(int num __attribute__((unused)), char *dev __attribute__((unused)))
{
	return "/dev/ttyMT4";
} 
static char * usb_adjust(int num, char *dev)
{
    int n;
    int len = 0;
    char *path = NULL;
    char *spath = NULL;
    DIR *dirp = NULL;
    struct dirent *dirent;

    if(!dev){
        dev = "ttyUSB";
    }
    len = strlen(dev) + PATH_NAME_LEN;
    path = malloc(len);
    if(!path){
        goto end_adjust;
    }
    memset(path, 0, len);

    if((dirp = opendir(SYSFS_USB_DRIVER_OLD)) != 0) {
        spath = SYSFS_USB_DRIVER_OLD;
        closedir(dirp);
    }else if ((dirp = opendir(SYSFS_USB_DRIVER_NEW)) != 0){
        spath = SYSFS_USB_DRIVER_NEW;
        closedir(dirp);
    }else{
        goto end_adjust;
    }

    
    dirp = opendir(spath);
    if(!dirp) {
        goto end_adjust;
    }

    do {
        n = 0;
        dirent = readdir(dirp);
        if(!dirent) {
            goto end_adjust;
        }
        if(!isdigit(dirent->d_name[n++]))
            continue;
        if(dirent->d_name[n++] != '-')
            continue;
        if(!isdigit(dirent->d_name[n++]))
            continue;
        if(dirent->d_name[n++] != ':')
            continue;
        if(!isdigit(dirent->d_name[n++]))
            continue;
        if(dirent->d_name[n++] != '.')
            continue;
        if(dirent->d_name[n++] != ('0' + num) )
            continue;
        
        //form new path
        memset(path, 0, len);
        snprintf(path, len, "%s%s",spath, dirent->d_name);
        closedir(dirp);
        //open this path
        dirp = opendir(path);
        if(!dirp){
            goto end_adjust;
        }

        do {
            n = 0;
            dirent = readdir(dirp);
            if(!dirent) {
                goto end_adjust;
            }
            if(strncmp(dirent->d_name, dev, strlen(dev)))
                continue;
            //we got it, finally
            memset(path, 0, len);
            snprintf(path, len, "/dev/%s",dirent->d_name);
            closedir(dirp);
            break;
        } while(1);
        
        break;
    } while(1);

    return path;

end_adjust:
    if(dirp){
        closedir(dirp);
    }
    if(path){
        free(path);
    }
    return NULL;
}

static char * sdio_adjust(int num, char *dev)
{
	int len = 0;
	char *path = NULL;
	if(!dev){
		LOGE("invalid dev\n");
		return NULL;
	}
	len = strlen(dev) + PATH_NAME_LEN;
	snprintf(dev, len, "ttySDIO%d",num);
	path = malloc(len);
	if(!path){
		goto end_adjust;
	}
	memset(path, 0, len);
	snprintf(path, len, "/dev/%s",dev);
	return path;

end_adjust:
	return NULL;
}

static device_type_adjust adjust_list[] = {
    {"uart", uart_adjust},
    {"usb",  usb_adjust},
    {"sdio", sdio_adjust},
    {NULL, NULL}
};

/* Input
 *		type: the type in adjust list
 *		num : the index of the device ,which can be port or interface
 *		dev : the prefix name of the driver entry in /dev/ path.
 * Return 
 *		The path string of the driver file, just like /dev/ttyUSB0.
 *		NULL if no device can be found.
 * Note: 
 *		The memory of the return path string need be freed by the caller
 */
char * viatelAdjustDevicePath(char *type, int num, char *dev)
{
    char *path = NULL;
    device_type_adjust *ad = NULL;

    print_version_info();

    if(num < 0 || type == NULL){
        return NULL;
    }
    
    ad = adjust_list;
    while(ad->type){
        if(!strncmp(ad->type, type, PATH_NAME_LEN)){
            path = ad->adjust(num, dev);
            break;
        }
        ad++;
    }

    LOGD("get path = %s\n", path ? path : "NULL");
    return path;
}

/* Input
 *		channel: the channel index of the device
 * Return 
 *		The path string of the driver file, just like /dev/ttyUSB0.
 *		NULL if no device can be found.
 * Note: 
 *		The memory of the return path string need be freed by the caller
 */
char * viatelAdjustDevicePathFromProperty(int channel)
{
    char buf[PATH_NAME_LEN];
    char *path = NULL, *head, *type, *num, *dev;
    device_type_property *p;
    int ret, i, step;

    print_version_info();
    
    if(channel >= VIATEL_CHANNEL_NUM || channel < 0){
        goto end_adjust;
    }

    p = prop_list + channel;
    memset(buf, 0, PATH_NAME_LEN);
    ret = property_get(p->prop, buf, NULL);
    if(ret <= 0){
		goto end_adjust;
    }
    //[type].[num].[dev]
    path = NULL;
    i = 0;
    type = num = dev = NULL;
    head = buf;
    step = 0;
    while(i < PATH_NAME_LEN){
        if(buf[i] == '.'){
            buf[i] = '\0';
            step++;
        }
        i++;
    }

    switch(step){
        case 0://fix device 
            path = malloc(PATH_NAME_LEN);
            if(path){
                snprintf(path, PATH_NAME_LEN, "/dev/%s", buf);
            }
            break;
        case 1://type.num, but dev is default value
            type = buf;
            num = buf + strlen(type) + 1;
            path = viatelAdjustDevicePath(type, atoi(num), dev);
            break;
        default:////type.num.dev
            type = buf;
            num = buf + strlen(type) + 1;
            dev = num + strlen(num) + 1;
            path = viatelAdjustDevicePath(type, atoi(num), dev);
    }
end_adjust:
    return  path;
}

void viatelModemPower(int sw)
{
	int fd = -1;
	ssize_t ret;
#if 0
	char cmd[32] = {0};

	fd = open(SYSFS_MODEM_POWER, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open modem power sysfs %s.\n", SYSFS_MODEM_POWER);
		return ;
	}
	if(sw){
		ret = snprintf(cmd, sizeof(cmd), "%s","on");
	}else{
		ret = snprintf(cmd, sizeof(cmd), "%s","off");
	}

	ret = write(fd, cmd, len);
	if(ret <= 0){
		LOGE("Fail to write modem power %s command.\n", cmd);
	}
	close(fd);
#else
	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_POWER, &sw); 
	if(ret < 0){
		LOGE("Fail to power %s vmodem.\n", sw?"on":"off");
	}
	close(fd);
#endif
}

void C2KEnterFlightMode()
{
	int fd = -1;
	ssize_t ret;
	
	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_ENTER_FLIGHT_MODE); 
	if(ret < 0){
		LOGE("Fail to enter flight mode.\n");
	}
	close(fd);
}

void C2KLeaveFlightMode()
{
	int fd = -1;
	ssize_t ret;
	
	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_LEAVE_FLIGHT_MODE); 
	if(ret < 0){
		LOGE("Fail to leave flight mode.\n");
	}
	close(fd);
}


void C2KForceAssert()
{
	int fd = -1;
	ssize_t ret;
	
	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_FORCE_ASSERT); 
	if(ret < 0){
		LOGE("Fail to leave flight mode.\n");
	}
	close(fd);
}

void C2KReset(void)
{	
	int fd = -1;
	ssize_t ret;
	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_RESET_FROM_RIL); 
	if(ret < 0){
		LOGE("Fail to reset vmodem.\n");
	}
	close(fd);
}

void viatelModemReset(void)
{	
	int fd = -1;
	ssize_t ret;
#if 0
	char cmd[32] = {0};

	fd = open(SYSFS_MODEM_RESET, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open modem reset sysfs %s\n", SYSFS_MODEM_RESET);
		return ;
	}
	
	ret = snprintf(cmd, sizeof(cmd), "%s","reset");
	ret = write(fd, cmd, len);
	if(ret <= 0){
		LOGE("Fail to write modem %s command.\n", cmd);
	}
	close(fd);
#else
	LOGD("Reset C2K now...\n");
	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_RESET); 
	if(ret < 0){
		LOGE("Fail to reset vmodem.\n");
	}
	close(fd);
#endif
}

void viatelModemReady(void)
{	
	int fd = -1;
	ssize_t ret;

	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_READY); 
	if(ret < 0){
		LOGE("Fail to notify vmodem ready.\n");
	}
	close(fd);
}

void viatelModemResetPCCIF(void)
{
	int fd = -1;
	ssize_t ret;

	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_RESET_PCCIF); 
	if(ret < 0){
		LOGE("Fail to notify vmodem ready.\n");
	}
	close(fd);
}

void viatelModemDie(void)
{	
	int fd = -1;
	ssize_t ret;

	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	ret = ioctl(fd, CMDM_IOCTL_DIE); 
	if(ret < 0){
		LOGE("Fail to die vmodem.\n");
	}
	close(fd);
}

void viatelModemWakeLock(int wake)
{
	int fd = -1;
	ssize_t ret;

	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	
	wake = !!wake;
	ret = ioctl(fd, CMDM_IOCTL_WAKE,&wake); 
	if(ret < 0){
		LOGE("Fail to die vmodem.\n");
	}
	close(fd);

}


void viatelModemNotifierIgnore(int sw)
{
	int fd = -1;
	ssize_t ret;

	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return ;
	}
	
	sw = !!sw;
	ret = ioctl(fd, CMDM_IOCTL_IGNORE,&sw); 
	if(ret < 0){
		LOGE("Fail to %s notifier.\n", sw?"ignore":"receive");
	}
	close(fd);
}

int viatelGetSdioStatus()
{
        int fd = -1;
        ssize_t ret;

        fd = open(VMODEM_DEVICE_PATH, O_RDWR);
        if(fd < 0){
                LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
                return -1;
        }
        ret = ioctl(fd, CMDM_IOCTL_GET_SDIO_STATUS);
        if(ret < 0){
                LOGE("Fail to get sdio status.\n");
        }
        close(fd);
        return 0;
}

int dumpBootupStatus()
{
        int fd = -1;
        ssize_t ret;

        fd = open(VMODEM_DEVICE_PATH, O_RDWR);
        if(fd < 0){
                LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
                return -1;
        }
        ret = ioctl(fd, CMDM_IOCTL_DUMP_BOOTUP_STATUS);
        if(ret < 0){
                LOGE("Fail to dump bootup status.\n");
        }
        close(fd);
        return 0;
}

int viatelGetModemStatus()
{
	int fd = -1;
	ssize_t ret;
	int md_status;
	fd = open(VMODEM_DEVICE_PATH, O_RDWR);
	if(fd < 0){
		LOGE("Fail to open device %s.\n", VMODEM_DEVICE_PATH);
		return 0;
	}
	ret = ioctl(fd, CMDM_IOCTL_GET_MD_STATUS, &md_status); 
	if(ret < 0){
		LOGE("Fail to get md status.\n");
	}
	close(fd);
	return md_status;
}


#define WAIT_FLAG_NVRAM_INIT_READY	(1<<0)
#define WAIT_FLAG_NVRAM_INIT_PRE_READY	(1<<1)

#define SHOW_LOG_DURATION		240
static void wait_nvram_ready(int wait_bitmap)
{
	char property_val[PROPERTY_VALUE_MAX] = {0};
	int curr_bitmap = 0;
	int retry_cnt = 0;

	LOGD("waiting nvram ready! max:%d\n", retry_cnt);
	while(1){
		property_get("vendor.service.nvram_init", property_val, NULL);
		if ((curr_bitmap & WAIT_FLAG_NVRAM_INIT_READY) == 0) {
			if (strcmp(property_val, "Ready") == 0)
				curr_bitmap |= WAIT_FLAG_NVRAM_INIT_READY;
		}

		if ((curr_bitmap & WAIT_FLAG_NVRAM_INIT_PRE_READY) == 0) {
			if (strcmp(property_val, "Pre_Ready") == 0)
				curr_bitmap |= WAIT_FLAG_NVRAM_INIT_PRE_READY;
		}

		if (curr_bitmap & wait_bitmap)
			return;

		retry_cnt++;
		if ((retry_cnt % SHOW_LOG_DURATION) == 0)
			LOGD("wait service.nvram_init=%s... [%d]\n", property_val, retry_cnt);
		usleep(500*1000);
	}
}
static void wait_decrypt_done(void)
{
    int retry=0;
    char crypto_state[PROPERTY_VALUE_MAX];
    char decrypt_state[PROPERTY_VALUE_MAX] = {0};
    bool isEncryptWithPasswd = false;
    bool toCheckIsEncryptWithPasswd = true;
    long becomeEncryptedTimestamp = 0;
    struct sysinfo info;

    /* AOSP adds a new encrypting flow, encrypting with password:
          1. The device doesn't enable "default encryption"
          2. Set the screen lock with pin/password/pattern
          3. Go to Settings and try to encrypt the device manually
          4. The device will reboot
          5. Vold encrypts the device
          6. After finish encrypting, the device will show the UI to let the end use decrypt the device
             (The property, vold.decrypt, is 'trigger_restart_min_framework')
    */

    LOGD("waiting vold.decrypt=trigger_restart_framework or trigger_restart_min_framework");
    property_get("ro.crypto.state", crypto_state, "");
    property_get("vold.decrypt", decrypt_state, NULL);
    while (true) {
        if (!strcmp(crypto_state, "encrypted")) {
            if (becomeEncryptedTimestamp == 0) {
                sysinfo(&info);
                becomeEncryptedTimestamp = info.uptime;
            }
            if (toCheckIsEncryptWithPasswd == true && isEncryptWithPasswd == false) {
                sysinfo(&info);
                if ( info.uptime - becomeEncryptedTimestamp > 10 ) {
                    if (!strcmp(decrypt_state, "trigger_restart_min_framework")) {
                      isEncryptWithPasswd = true;
                      LOGD("Password is NOT default because vold.decrypt is still trigger_restart_min_framework");
                    }
                    toCheckIsEncryptWithPasswd = false;
                }
            }

            if(isEncryptWithPasswd) { /* Password is NOT default */
                if (!strcmp(decrypt_state, "trigger_restart_min_framework")) {
                    break;
                }
            }
            else { /* Password is default */
                if (!strcmp(decrypt_state, "trigger_restart_framework")) {
                    break;
                }
             }
        }
        else if (!strcmp(crypto_state, "unencrypted")) { /* fake encrypting in meta/factory mode */
            break;
        }

        retry++;
        if ((retry % SHOW_LOG_DURATION) == 0)
            LOGD("wait vold.decrypt...,%s\n", decrypt_state);
        usleep(500*1000);
        property_get("ro.crypto.state", crypto_state, "");
        property_get("vold.decrypt", decrypt_state, NULL);
    }
    LOGD("wait vold.decrypt=%s done success!\n", decrypt_state);
}

/*
*Return value:
* 0: unencrypted, unsupported
* 1: auto encrypt and decrpty on first boot
* 2: vold trigger_restart_framework after decrpty, fsis norma
* 3: vold trigger_restart_min_framework and wait decrpty, fs is tmpfs
* error value: <0
*/
int rfs_access_ok(void)
{
	int ret;
	char property_val[PROPERTY_VALUE_MAX] = {0};

	// Check whether is at decrypt state
	property_get("ro.crypto.state", property_val, NULL);
	LOGD("ro.crypto.state=%s\n",property_val);
	if (strcmp(property_val, "") == 0) {
		LOGD("auto encrypt & decrypt\n");
		wait_decrypt_done();
		wait_nvram_ready(WAIT_FLAG_NVRAM_INIT_READY|WAIT_FLAG_NVRAM_INIT_PRE_READY);
		return 1;
	} else if (strcmp(property_val, "unencrypted") == 0) {
		wait_nvram_ready(WAIT_FLAG_NVRAM_INIT_READY);
		LOGD("unencrypted!!\n");
		return 0;
	} else if (strcmp(property_val, "unsupported") == 0) {
		wait_nvram_ready(WAIT_FLAG_NVRAM_INIT_READY);
		LOGD("unsupported!!\n");
		return 0;
	} else if (strcmp(property_val, "encrypted") == 0) {
		property_get("ro.crypto.type", property_val, NULL);
		if (strcmp(property_val, "file") == 0) {
			wait_nvram_ready(WAIT_FLAG_NVRAM_INIT_READY);
			LOGD("file/FBE!!\n");
			return 0;
		}
		while(1) {
			property_get("vold.decrypt", property_val, NULL);
			if (strcmp(property_val, "trigger_restart_framework") == 0) {
				LOGD("vold.decrypt:trigger_restart_framework\n");
				wait_nvram_ready(WAIT_FLAG_NVRAM_INIT_READY);
				return 2;
			} else if (strcmp(property_val, "trigger_restart_min_framework") == 0) {
				LOGD("vold.decrypt:trigger_restart_min_framework!!\n");
				wait_nvram_ready(WAIT_FLAG_NVRAM_INIT_READY|WAIT_FLAG_NVRAM_INIT_PRE_READY);
				return 3;
			}
			usleep(100*1000);
		}
	} else {
		LOGE("crypto state error %s!!\n", property_val);
		ret = -1;
	}
	return ret;
}
