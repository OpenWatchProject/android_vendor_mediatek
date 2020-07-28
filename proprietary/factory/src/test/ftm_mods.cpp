/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */


#include <stdio.h>
#include "common.h"
#include "ftm.h"

#define _cplusplus
#ifdef  _cplusplus
extern "C"{
#endif
extern int mcard_init(void);
extern int battery_init(void);
extern int gsensor_init(void);
extern int gs_cali_init(void);
extern int msensor_init(void);
extern int flash_init(void);
extern int clear_flash_init(void);
extern int camera_main_init(void);
extern int camera_main2_init(void);
extern int camera_main3_init(void);
extern int camera_main4_init(void);
extern int camera_sub_init(void);
extern int camera_sub2_init(void);
/* fix build error
extern int lens_main_init(void);
extern int lens_main2_init(void);
extern int lens_sub_init(void)
*/
extern int strobe_init(void);
extern int audio_init(void);
extern int spk_init(void);
extern int keys_init(void);
extern int lcd_init(void);
extern int backlight_init(void);
extern int lcm_init(void);
extern int headset_init(void);
extern int bluetooth_init(void);
extern int rtc_init(void);
extern int touch_init(void);
extern int touch_auto_init(void);
extern int gps_init(void);
extern int idle_init(void);
extern int led_init(void);
extern int vibrator_init(void);
extern int wifi_init(void);
extern int jogball_init(void);
extern int ofn_init(void);
extern int fm_init();
extern int fmtx_init();
extern int MATV_init(void);
extern int alsps_init(void);
extern int tvout_init(void);
extern int gyro_init(void);
extern int gyro_cali_init(void);
extern int sim_init(void);
extern int barometer_init(void);
extern int humidity_init(void);
extern int otg_init(void);
extern int usb_typec_init(void);
extern int cmmb_init(void);
extern int usb_init(void);
extern int nfc_init(void);
extern int emi_init(void);
extern int hdmi_init(void);
extern int audio_debug_init(void);
extern int rf_test_init(void);
extern int universal_ant_test_init(void);
//extern int headset_debug_init(void);
extern int bts_init(void);
extern int ext_buck_init(void);
extern int ext_vbat_boost_init(void);
extern int slave_chg_init(void);
extern int rf_c2k_test_init(void);
extern int irtx_led_init(void);
extern int hrm_init(void);
extern int efuse_init(void);
/* goodix begin */
extern int goodix_fingerprint_init(void);
/* goodix end */
extern int ap_auxadc_init(void);
extern int bio_cali_init(void);
extern int biosensor_init(void);
#ifdef  _cplusplus
}
#endif
extern int sigtest_init(void);
extern int bootdev_init(void);
extern int clear_bootdev_init(void);

ftm_init_fn ftm_init_debug_funcs[] = {
/*
#ifdef FEATURE_FTM_HEADSET
    headset_debug_init,
#endif
*/
#ifdef FEATURE_FTM_AUDIO
#ifdef MTK_AUDIO_HW_SUPPORT
    audio_debug_init,
#endif
#endif

	NULL,

};

ftm_init_fn ftm_init_funcs[] = {
#ifdef FEATURE_FTM_MEMCARD
    mcard_init,
#endif

#ifdef FEATURE_FTM_BATTERY
    battery_init,
#endif

#ifdef MTK_DUAL_CHARGER_SUPPORT
#ifdef FEATURE_FTM_SLAVE_CHARGER
    slave_chg_init,
#endif
#endif

#ifdef FEATURE_FTM_EXT_BUCK
    ext_buck_init,
#endif

#ifdef FEATURE_FTM_EXT_VBAT_BOOST
    ext_vbat_boost_init,
#endif

#ifdef CUSTOM_KERNEL_ACCELEROMETER
    gsensor_init,
    gs_cali_init,
#endif

#ifdef FEATURE_FTM_IDLE
	idle_init,
#endif

#ifdef CUSTOM_KERNEL_MAGNETOMETER
    msensor_init,
#endif

#ifdef FEATURE_FTM_FLASH
    flash_init,
#endif

#ifdef FEATURE_FTM_CLEARFLASH
    clear_flash_init,
#endif

#ifdef FEATURE_FTM_MAIN_CAMERA
    camera_main_init,
#endif

#ifdef FEATURE_FTM_MAIN2_CAMERA
    camera_main2_init,
#endif

#ifdef FEATURE_FTM_MAIN3_CAMERA
    camera_main3_init,
#endif

#ifdef FEATURE_FTM_MAIN4_CAMERA
    camera_main4_init,
#endif

#ifdef FEATURE_FTM_SUB_CAMERA
    camera_sub_init,
#endif

#ifdef FEATURE_FTM_SUB2_CAMERA
    camera_sub2_init,
#endif

#ifdef FEATURE_FTM_STROBE
    strobe_init,
#endif

#ifdef FEATURE_FTM_AUDIO
#ifdef MTK_AUDIO_HW_SUPPORT
   audio_init,
#endif
#endif

#ifdef FEATURE_FTM_SPK_OC
#ifdef MTK_AUDIO_HW_SUPPORT
    //spk_init,
#endif
#endif

#ifdef FEATURE_FTM_KEYS
    keys_init,
#endif

#ifdef FEATURE_FTM_LCD
    lcd_init,
    backlight_init,
#endif

#ifdef FEATURE_FTM_LCM
    lcm_init,
#endif

#ifdef FEATURE_FTM_AP_AUXADC
	ap_auxadc_init,
#endif

#ifdef FEATURE_FTM_AUDIO
#ifdef FEATURE_FTM_HEADSET
#ifdef MTK_AUDIO_HW_SUPPORT
    headset_init,
#endif
#endif
#endif

#ifdef FEATURE_FTM_USB
    usb_init,
#endif
#ifdef FEATURE_FTM_OTG
    otg_init,
#endif

#ifdef FEATURE_FTM_USB_TYPEC
    usb_typec_init,
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
    bluetooth_init,
#endif
#endif

#ifdef FEATURE_FTM_RTC
    //rtc_init,
#endif

#ifdef FEATURE_FTM_TOUCH
    touch_init,
    touch_auto_init,
#endif

#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
    gps_init,
#endif
#endif


#ifdef MTK_NFC_SUPPORT
    nfc_init,
#endif
#ifdef ST_NFC_SUPPORT_FM
    nfc_init,
#endif

#ifdef FEATURE_FTM_AUDIO
#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FMTX
#ifdef MTK_FM_TX_SUPPORT
#ifdef MTK_AUDIO_HW_SUPPORT
    fmtx_init,
#endif
#endif
#endif

#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
#ifdef MTK_AUDIO_HW_SUPPORT
    fm_init,
#endif
#endif
#endif
#endif
#endif

#ifdef FEATURE_FTM_LED
    led_init,
#endif

#ifdef FEATURE_FTM_SIM
    sim_init,
#endif

#ifdef FEATURE_FTM_AUDIO
#ifdef FEATURE_FTM_VIBRATOR
#ifdef MTK_AUDIO_HW_SUPPORT
    vibrator_init,
#endif
#endif
#endif

#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
	wifi_init,
#endif
#endif

#ifdef FEATURE_FTM_JOGBALL
    jogball_init,
#endif

#ifdef FEATURE_FTM_OFN
    ofn_init,
#endif

#ifdef FEATURE_FTM_MATV
    MATV_init,
#endif
#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_ALS) || defined(CUSTOM_KERNEL_PS)
    alsps_init,
#endif
#ifdef FEATURE_FTM_TVOUT
    tvout_init,
#endif
#ifdef CUSTOM_KERNEL_BAROMETER
    barometer_init,
#endif
#ifdef CUSTOM_KERNEL_HUMIDITY
	humidity_init,
#endif
#ifdef CUSTOM_KERNEL_GYROSCOPE
    gyro_init,
    gyro_cali_init,
#endif
//#ifdef FEATURE_FTM_SIGNALTEST
    sigtest_init,
//#endif

#ifdef FEATURE_FTM_CMMB
    cmmb_init,
#endif
#if defined(MTK_EMMC_SUPPORT) || defined(MTK_UFS_SUPPORT)
#if defined(FEATURE_FTM_EMMC) || defined(FEATURE_FTM_UFS)
    bootdev_init,
#endif

/*
 * eMMC and UFS share the same clear /data & /nvram func.
 * Note the name remains CLEAREMMC.
 */
#ifdef FEATURE_FTM_CLEAREMMC
    clear_bootdev_init,
#endif
#endif /* MTK_EMMC_SUPPORT || MTK_UFS_SUPPORT */

#ifdef MTK_HDMI_SUPPORT
#ifdef FEATURE_FTM_HDMI
    hdmi_init,
#endif
#endif

#ifdef FEATURE_FTM_RF
    rf_test_init,
#endif

#if defined(FEATURE_FTM_UNIVERSAL_ANT)
   universal_ant_test_init,
#endif

#ifdef FEATURE_FTM_BTS
    bts_init,
#endif

#if defined(FACTORY_C2K_SUPPORT) && !defined(EVDO_FTM_DT_VIA_SUPPORT)
   rf_c2k_test_init,
#endif

#ifdef FEATURE_FTM_IRTX_LED
  irtx_led_init,
#endif

#ifdef CUSTOM_KERNEL_HEART
	hrm_init,
#endif

#ifdef FEATURE_FTM_EFUSE
    efuse_init,
#endif

/* fix build error
#ifdef FEATURE_FTM_MAIN_LENS
	lens_main_init,
#endif

#ifdef FEATURE_FTM_MAIN2_LENS
	lens_main2_init,
#endif

#ifdef FEATURE_FTM_SUB_LENS
	lens_sub_init,
#endif
*/
/* goodix begin */
#ifdef FEATURE_FTM_GOODIX_FINGERPRINT
    goodix_fingerprint_init,
#endif
/* goodix end */

#ifdef CUSTOM_KERNEL_BIOMETRIC_SENSOR
	biosensor_init,
	bio_cali_init,
#endif

    NULL,
};
