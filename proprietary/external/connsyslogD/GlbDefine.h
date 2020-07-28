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

/******************************************************************************
 *
 * Filename:
 * ---------
 *   globeconfig.h
 *
 * Description:
 * ------------
 *   Global macro and constant definitions
 *   All global constants should be defined here so that logger will
 *   use.
 *
 *****************************************************************************/

#ifndef GLBDEFINE_H_
#define GLBDEFINE_H_

namespace consyslogger {
/**
 * Micro define
 */
#define MAX_PATH_LEN            (256)
/**
 * const varible
 */
#define REF_STR(x) #x
#define APPEND_STR(x) REF_STR(x)

/**
 * logger log tag
 */
#define CONSYS_LOG_TAG "connsyslogger"

/**
 * logger name
 */
#define CONSYSLOG_NAME "connsyslog"

/**
 *  logger folder name
 */
#define SD_LOG_ROOT_FORMAT "%s/debuglogger/" CONSYSLOG_NAME "/fw"


/**
 * logger /data path setting:
 * 1. /data/connsyslog/, root folder
 
 */
#define CONSYSLOGGER_DATA_PATH "/data/connsyslog/"

/**
 
 */
#define CONSYSLOGGER_CONFIG_FILE CONSYSLOGGER_DATA_PATH CONSYSLOG_NAME "_config"
/**
 *  Logger default log recycle size.
 * It will read item "com.mediatek.log.connsyslog.maxsize" in file
 * "/system/etc/mtklog-config.prop" to get it value in the first time.
 * see "LOGGER_CUST_FILE"
 */
#define CONSYSLOGGER_RECYCLE_SIZE_FILE CONSYSLOGGER_DATA_PATH CONSYSLOG_NAME "_recycle_size_config"

/**
 * consyslog logger server pipe name
 * Service pipe is used by logger's main thread to read message
 * from client, and execute the pass-in commands in logger.
 */
#define SERVER_FIFO CONSYSLOGGER_DATA_PATH CONSYSLOG_NAME "_serv_fifo"

/**
 * consyslog logger client pipe name
 * Client pipe is used by logger's client or sub thread to get
 * the command executing results.
 */
#define CLIENT_FIFO CONSYSLOGGER_DATA_PATH CONSYSLOG_NAME "_cli_fifo"

/**
 * logger's boot up log root folder path
 */
#define CONSYSLOGGER_BOOTUP_FOLDER CONSYSLOGGER_DATA_PATH "bootupLog"

/**
 * consyslog logger's boot up log folder
 */
#define CONSYSLOGGER_BOOTUP_LOG_FOLDER CONSYSLOGGER_BOOTUP_FOLDER

/**
 * mtklog configure file "/vendor/etc/mtklog-config.prop"
 * When logger starts, it would read and parse this file to get 
 * logger state and logger log recycle size.
 *
 * The file content is like:
 * '''
 *    mtklog_path = internal_sd
 *    com.mediatek.log.mobile.enabled = true
 *    com.mediatek.log.mobile.maxsize = 300
 *    com.mediatek.log.modem.enabled = true
 *    com.mediatek.log.modem.maxsize = 600
 *    com.mediatek.log.net.enabled = true
 *    com.mediatek.log.net.maxsize = 200
 * '''
 */
#define LOGGER_CUST_FILE "/system/etc/mtklog-config.prop"
#define LOGGER_RUNNING_KEY "com.mediatek.log.connsysfw.enabled"
#define LOGGER_LOG_RECYCLE_SIZE "com.mediatek.log.connsysfw.maxsize"

#define LOGGER_LOG_PATH_NEW_KEY "mtklog_path"

/**
 * Logger's logging path
 * This property is set and used by logger.
 * The default value is PROP_VALUE_LOG_PATH_DEFAULT,
 */
#define PROP_VALUE_LOG_PATH_DEFAULT "/mnt/sdcard"

/*
 * Property to keep  Logger running status.
 * 1, Logger is running;
 * 0, Logger is stoped.
 */
#define PROP_CONSYSLOGGER_RUNNING_STATUS "vendor.connsysfw.running"

/**
 * Property to indicate whether device boot complete:
 * 1, boot complete
 * 0, booting
 */
#define PROP_DEVICE_BOOT_COMPLETE "sys.boot_completed"

/*
 * Property to indicate load build type
 * "eng", engineer load
 */
#define PROP_BUILD_TYPE "ro.build.type"


/**
 * System file keeps device booting-up mode
 */
#define BOOTMODE_PATH "/sys/class/BOOT/BOOT/boot/boot_mode"
#define FACTORY_BOOT 4
#define ATE_FACTORY_BOOT 6
#define META_BOOT 1

/**
 * Logger socket server name.
 * This socket server is used to communicate with MTKLogger to accept its command
 */
#define SOCKET_SERVER_NAME "connsysfwlogd"

/**
 * Messages used in logger modules.
 * All these messages are should be handled by logger,
 * and sent to MTKLogger API
 */
#define MSG_FAIL_WRITE_FILE 1 //FAIL_WRITEFILE
#define MSG_SDCARD_NO_EXIST 2   //SDCARD_NOTEXIST
#define MSG_SDCARD_IS_FULL 3  //SDCARD_FULL
#define MSG_SDCARD_NO_LOG_FILE 4 //LOGFILE_NOTEXIST

#define PROP_LOG_SAVE_PATH "vendor.connsysfw.savepath"
#define PROP_WMT_DRIVER_READY "vendor.connsys.driver.ready"

//enable/disabled
#define PROP_ATM_MODE_FLAG "ro.boot.atm"
//0 :do logging
//1: not logging
#define PROP_META_BOOT_FLAG "ro.boot.meta_log_disable"

#define PROP_META_CONN_TYPE "persist.vendor.meta.connecttype"
#define PROP_WIFI_ADDR "persist.vendor.atm.ipaddress"

#define PROP_LOGGER_RUNNING_FOLDER "vendor.connsysfw.run.path"
//1: on ;
#define PROP_FEATURE_OPTION "ro.vendor.connsys.dedicated.log"

}

#endif /* GLOBECONFIG_H_ */
