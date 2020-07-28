/*
 * viatelutils.h
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
#ifndef VIATEL_UTILS_H
#define VIATEL_UTILS_H
#include <linux/ioctl.h>

/* ioctl for vomodem, which must be same as power.c in kernel*/
#define CMDM_IOCTL_RESET        _IO( 'c', 0x01)
#define CMDM_IOCTL_POWER        _IOW('c', 0x02, int)
#define CMDM_IOCTL_CRL			_IOW('c', 0x03, int)
#define CMDM_IOCTL_DIE			_IO( 'c', 0x04)
#define CMDM_IOCTL_WAKE			_IO( 'c', 0x05)
#define CMDM_IOCTL_IGNORE		_IOW( 'c', 0x06, int)
#define CMDM_IOCTL_GET_MD_STATUS	_IOR('c', 0x07, int)
#define CMDM_IOCTL_ENTER_FLIGHT_MODE 	_IO('c', 0x08)
#define CMDM_IOCTL_LEAVE_FLIGHT_MODE 	_IO('c', 0x09)
#define CMDM_IOCTL_READY	_IO( 'c', 0x0A)
#define CMDM_IOCTL_RESET_PCCIF	_IO( 'c', 0x0B)
#define CMDM_IOCTL_FORCE_ASSERT			_IO( 'c', 0x0C)
#define CMDM_IOCTL_RESET_FROM_RIL			_IO( 'c', 0x0D)
//reservce some bit
#define CMDM_IOCTL_GET_SDIO_STATUS      _IO( 'c', 0x10)
#define CMDM_IOCTL_DUMP_BOOTUP_STATUS      _IO( 'c', 0x12)

/* event for vmodem, which must be same as power.c in kernel */
enum ASC_USERSPACE_NOTIFIER_CODE{
    ASC_USER_USB_WAKE =  100,
    ASC_USER_USB_SLEEP,
    ASC_USER_UART_WAKE,
    ASC_USER_UART_SLEEP,
    ASC_USER_SDIO_WAKE,
    ASC_USER_SDIO_SLEEP,
    ASC_USER_MDM_POWER_ON = 200,
    ASC_USER_MDM_POWER_OFF,
    ASC_USER_MDM_RESET_ON,
    ASC_USER_MDM_RESET_OFF,
	ASC_USER_MDM_ERR = 300,
	ASC_USER_MDM_ERR_ENHANCE , 
    ASC_USER_MDM_IPOH = 400,
    ASC_USER_MDM_WDT,
    ASC_USER_MDM_EXCEPTION,
    ASC_USER_MDM_RESET_ON_SINGLE,
};

enum VIATEL_CHANNEL_ID{
    VIATEL_CHANNEL_AT,
    VIATEL_CHANNEL_DATA,
    VIATEL_CHANNEL_ETS,
    VIATEL_CHANNEL_GPS,
    VIATEL_CHANNEL_PCV,
    VIATEL_CHANNEL_ASCI,
    VIATEL_CHANNEL_FLS,
    VIATEL_CHANNEL_MUX,
    VIATEL_CHANNEL_AT2,
    VIATEL_CHANNEL_AT3,
    VIATEL_CHANNEL_MDLOG_DATA,
    VIATEL_CHANNEL_EXCP_MSG,
    VIATEL_CHANNEL_EXCP_DATA,
    VIATEL_CHANNEL_AT4,
    VIATEL_CHANNEL_AT5,
    VIATEL_CHANNEL_AT6,
    VIATEL_CHANNEL_AT7,
    VIATEL_CHANNEL_AT8,
    VIATEL_CHANNEL_NUM
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
extern char * viatelAdjustDevicePath(char *type, int num, char *dev);

/* Input
 *		channel: the channel index of the device
 * Return 
 *		The path string of the driver file, just like /dev/ttyUSB0.
 *		NULL if no device can be found.
 * Note: 
 *		The memory of the return path string need be freed by the caller
 */
#ifdef __cplusplus
extern "C" {
#endif

extern char * viatelAdjustDevicePathFromProperty(int channel);
#ifdef __cplusplus
}
#endif

/* Input
 *		sw: 1 to power on, 0 to power off
 * Return 
 *		None
 * Note: 
 *		switch the modem power
 */
extern void viatelModemPower(int sw);

extern void C2KEnterFlightMode();

extern void C2KLeaveFlightMode();
extern void C2KForceAssert();
extern void C2KReset();


/* Input
 *		None	
 * Return
 *		None	
 * Note: 
 *		reset the modem
 */
extern void viatelModemReset(void);

/* Input
 *		None	
 * Return
 *		None	
 * Note: 
 *		tell driver that modem boot up done
 */
extern void viatelModemReady(void);

/* Input
 *		None	
 * Return
 *		None	
 * Note: 
 *		reset PCCIF after MD1 stop and before MD3 reset
 */
extern void viatelModemResetPCCIF(void);

/* Input
 *		None	
 * Return
 *		None	
 * Note: 
 *		die the modem
 */
extern void viatelModemDie(void);

/* Input
 *		None	
 * Return
 *		None	
 * Note: 
 *		hold/release wakelock
 */

extern void viatelModemWakeLock(int wake);



/* Input
 *		sw: 1 to ignore, 0 to receive
 * Return
 *		None	
 * Note: 
 *		receive/ignore notifier from driver
 */

extern void viatelModemNotifierIgnore(int sw);

extern int viatelGetModemStatus();

extern int viatelGetSdioStatus();

extern int dumpBootupStatus();

extern int rfs_access_ok(void);

#endif
