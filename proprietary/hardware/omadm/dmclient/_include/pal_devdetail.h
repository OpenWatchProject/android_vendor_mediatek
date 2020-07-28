/**
 * @file pal_devdetail.h
 * @brief File containing PAL interface for DevDetail plugin.
 *
 * Device Details.
 * General device information. Mandatory plugin.
 * @see http://openmobilealliance.org/
 */

#ifndef PAL_DEVDETAIL_H
#define PAL_DEVDETAIL_H
#include "pal_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 *
 * @param[out] buff allocated buff for return value.
 * Manufacturer name of current device
 * For Android device it is a brand of this device.
 * For example: Sony Xperia L (C2103) -> Sony
 * Manufacturer name comes from firmware build data
 * or can be get by parcing result of shell command
 * "pm dump package | grep \"versionCode\""
 * or
 * using getPackageManager().getPackageInfo(getPackageName(), 0).versionName
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_oem_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Firmware Version of current device
 * For Android device it is a version of application
 * Get by parcing result of shell command "getProp"
 * or
 * using android.os.Build
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_fwv_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Software Version of current device
 * For Android device it is a version of android & build
 * Software Version comes from soft build data or
 * can be get by parcing result of shell command "getProp"
 * or
 * using android.os.Build
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_swv_get(data_buffer_t *buff);
/**
 *
 * @param[out] buff allocated buff for return value.
 * Hardware Version of current device
 * For Android device it is a version of android & build
 * Hardware Version comes from firmware build data
 * or
 * can be get by parcing result of shell command "getProp"
 * or
 * using android.os.Build
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hwv_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Support of Large Object("true" or "false")
 * Hardcoded
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_support_lrgobj_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Date the Software Update was successfully completed. The format
shall be "MM:DD:YYYY"
 * from java.util.Date or in c using time.h (library)
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_date_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * UTC time the Software Update was successfully completed. The UTC
format shall be "HH:MM"
 * from java.util.Date or in c using time.h (library)
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_timeutc_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * manufacturer of host device
 * can be get using shell command "lsusb" / "lsusb -v"
 * or
 * from file on host device
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_manu_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Model Name of host device
 * can be get using shell command "lsusb" / "lsusb -v"
 * or
 * from file on host device
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_model_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Software Version of host device
 *from file on host device
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_swv_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Firmware Version of host device
 * from file on host device
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_fwv_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Hardware Version of host device
 * from file on host device
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_hwv_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Date of the latest successful Host Device Software Update.
 * hardcoded or
 * if host device works under linux we need to get date of "/var/log/dpkg.log
 * " file
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_datestamp_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * ID of host device
 * It can be IMEI, MAC address or ID of usb device
 * if hostdevice works under android we can read using shell "getprop"
 * from android.provider.Settings.Secure
 * from telephonyManager
 * using shell "service call iphonesubinfo"
 * if host device works under linux we need to parce result of
 * "ifconfig command on device"
 * " file
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS  *
 *
 */
int pal_system_hostdevice_deviceid_get(data_buffer_t *buff);

/**
 *
 * @param[out] buff allocated buff for return value.
 * Type of current device
 * It can be "Smart Device" or "Feature Phone"
 *
 * @return
 * if length of buff <  length of value, then returns BUFFER_OVERFLOW
 * if buff == NULL or data of buff == NULL, then returnes BUFFER_NOT_DEFINED,
 * else returnes  SUCCESS
 *
 */
int pal_system_devtype_get(data_buffer_t *buff);

#ifdef __cplusplus
}
#endif

#endif // PAL_DEVDETAIL_H
