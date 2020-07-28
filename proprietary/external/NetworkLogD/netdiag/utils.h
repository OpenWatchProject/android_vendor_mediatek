/*
 * utils.h
 *
 *  Created on: Jan 14, 2013
 *      Author: mtk80906
 */

#ifndef UTILS_H_
#define UTILS_H_

//This is the key from customization file of /system/etc/mtklog-config.prop
#define _KEY_CUSTOM_LOG_PATH "mtklog_path"
// This is, /data
#define _VALUE_CONFIG_LOG_PATH_DATA "/data"
// This is, EMMC storage
#define _VALUE_CONFIG_LOG_PATH_EMMC "internal_sd"
// This is, external SD card
#define _VALUE_CONFIG_LOG_PATH_SD "external_sd"

/**
 * These key will be used for Netdiag with Shell UID
 */
#define _KEY_CONFIG_LOG_PATH "persist.vendor.mtklog.log2sd.path"
//Global flag to indicate whether network log is running or not
#define _KEY_FLAG_LOG_RUNNING "vendor.mtklog.netlog.Running"
#define _KEY_FLAG_LOG_INDEX "vendor.mtklog.netlog.index"

//Global flag to indicate whether need to stop log recording loop
#define _KEY_FLAG_LOG_BREAKLOOP "vendor.mtklog.netlog.Breakloop"
//NetworkLog current saving log path, must shorter than 32
#define _KEY_LOG_SAVING_PATH "vendor.netlog.writtingpath"
//NetworkLog stop reason, need to notify java layer
#define _KEY_LOG_STOP_REASON "vendor.netlog.stopreason"

/*
* support Chip mode
*/
#define PROP_CHIPTEST_ENABLE "persist.vendor.chiptest.enable"

#define VZW_VALUE_EXTERNAL_SD_PATH "/mnt/m_external_sd"
#define SD_RESERVED_SPACE  (10 * 1048576)

/**
 * These key will be used for Netdiag with Radio UID
#define _KEY_CONFIG_LOG_PATH "persist.radio.log2sd.path"
//Global flag to indicate whether network log is running or not
#define _KEY_FLAG_LOG_RUNNING "persist.radio.netlog.Running"
//Global flag to indicate whether need to stop log recording loop
#define _KEY_FLAG_LOG_BREAKLOOP "persist.radio.netlog.Breakloop"
//NetworkLog current saving log path, must shorter than 32
#define _KEY_LOG_SAVING_PATH "persist.radio.writtingpath"
//NetworkLog stop reason, need to notify java layer
#define _KEY_LOG_STOP_REASON "persist.radio.netlog.stopreason"
 */

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "Netdiag"
/**
 * These log definition was used for Android version later than JB
 */
#ifndef LOGD
#define LOGD ALOGD
#endif
#ifndef LOGI
#define LOGI ALOGI
#endif
#ifndef LOGE
#define LOGE ALOGE
#endif

#endif /* UTILS_H_ */
