/* Copyright Statement:
*
* This software/firmware and related documentation ("MediaTek Software") are
* protected under relevant copyright laws. The information contained herein
* is confidential and proprietary to MediaTek Inc. and/or its licensors.
* Without the prior written permission of MediaTek inc. and/or its licensors,
* any reproduction, modification, use or disclosure of MediaTek Software,
* and information contained herein, in whole or in part, shall be strictly prohibited.
*/

#include <string.h>
#include "common.h"
#include "ftm.h"
#include "miniui.h"
#include "utils.h"

#include "item.h"

#define TAG        "[ITEM] "


item_t ftm_auto_test_items[] =
{
#ifdef MTK_EFUSE_WRITER_SUPPORT
#ifdef FEATURE_FTM_EFUSE
    item(ITEM_EFUSE, uistr_info_efuse_test),
#endif
#endif

/* goodix begin */
#ifdef FEATURE_FTM_GOODIX_FINGERPRINT
    item(ITEM_FINGERPRINT_TEST, uistr_goodix_fingerprint),
#endif
/* goodix end */

#ifdef FEATURE_FTM_TOUCH
	item(ITEM_TOUCH_AUTO,	uistr_touch_auto),
#endif

#ifdef FEATURE_FTM_LCM
    item(ITEM_LCM,     uistr_lcm),
#endif

#if 0
#ifdef FEATURE_FTM_3GDATA_SMS
#elif defined FEATURE_FTM_3GDATA_ONLY
#elif defined FEATURE_FTM_WIFI_ONLY
#else
    item(ITEM_SIGNALTEST, uistr_sig_test),
#endif
#endif

#ifdef FEATURE_FTM_BATTERY
    item(ITEM_CHARGER, uistr_info_title_battery_charger),
#endif
#ifdef MTK_DUAL_CHARGER_SUPPORT
#ifdef FEATURE_FTM_SLAVE_CHARGER
    item(ITEM_SLAVE_CHARGER, uistr_info_title_slave_charger),
#endif
#endif

#ifdef FEATURE_FTM_EXT_BUCK
    item(ITEM_EXT_BUCK, uistr_info_title_ext_buck_item),
#endif

#ifdef FEATURE_FTM_EXT_VBAT_BOOST
    item(ITEM_EXT_VBAT_BOOST, uistr_info_title_ext_vbat_boost_item),
#endif

#ifdef FEATURE_FTM_FLASH
   item(ITEM_FLASH,   uistr_nand_flash),
#endif

#if 0
#ifdef FEATURE_FTM_RTC
    item(ITEM_RTC,     uistr_rtc),
#endif
#endif

#ifdef FEATURE_FTM_LCD
    item(ITEM_LCD,     uistr_lcm_test),
    item(ITEM_BACKLIGHT,     uistr_backlight_level),
#endif

#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
    item(ITEM_FM,      uistr_info_fmr_title),
#endif
#endif
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
    item(ITEM_BT, uistr_bluetooth),
#endif
#endif

#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
    item(ITEM_WIFI, uistr_wifi), //no uistr for wifi
#endif
#endif

#if defined(MTK_EMMC_SUPPORT) || defined(MTK_UFS_SUPPORT) || defined(MNTL_SUPPORT)
#if defined(FEATURE_FTM_EMMC) || defined(FEATURE_FTM_UFS) || defined(FEATURE_FTM_MNTL)
    item(ITEM_STORAGE,   uistr_storage),
#endif
#endif

#ifdef FEATURE_FTM_MEMCARD
    item(ITEM_MEMCARD, uistr_memory_card),
#endif

#ifdef FEATURE_FTM_SIM
    item(ITEM_SIM, uistr_sim_detect),
#endif

#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
	item(ITEM_GPS,	   uistr_gps),
#endif
#endif

#ifdef FEATURE_FTM_MAIN_CAMERA
        item(ITEM_MAIN_CAMERA,  uistr_main_sensor),
#endif

#ifdef FEATURE_FTM_MAIN2_CAMERA
        item(ITEM_MAIN2_CAMERA,  uistr_main2_sensor),
#endif

#ifdef FEATURE_FTM_MAIN3_CAMERA
        item(ITEM_MAIN3_CAMERA,  uistr_main3_sensor),
#endif

#ifdef FEATURE_FTM_MAIN4_CAMERA
        item(ITEM_MAIN4_CAMERA,  uistr_main4_sensor),
#endif

#ifdef FEATURE_FTM_SUB_CAMERA
        item(ITEM_SUB_CAMERA, uistr_sub_sensor),
#endif

#ifdef FEATURE_FTM_SUB2_CAMERA
        item(ITEM_SUB2_CAMERA, uistr_sub2_sensor),
#endif

#ifdef FEATURE_FTM_AUDIO
    item(ITEM_LOOPBACK_PHONEMICSPK,uistr_info_audio_loopback_phone_mic_speaker),
#endif
#ifdef RECEIVER_HEADSET_AUTOTEST
#ifdef FEATURE_FTM_AUDIO
    item(ITEM_RECEIVER, uistr_info_audio_receiver),
#endif
#endif

#ifdef FEATURE_FTM_AP_AUXADC
    /* item(ITEM_AP_AUXADC, uistr_info_ap_auxadc), */
#endif

#ifdef FEATURE_FTM_MATV
    //item(ITEM_MATV_NORMAL,  "MATV HW Test"),
    item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
#ifdef FEATURE_FTM_RF
    item(ITEM_RF_TEST,  uistr_rf_test),
#endif
#ifdef FEATURE_FTM_HDMI
    item(ITEM_HDMI, "HDMI"),
#endif

#ifdef FACTORY_C2K_SUPPORT
    item(ITEM_RF_C2K_TEST, uistr_rf_c2k_test),
#endif

#ifdef FEATURE_FTM_IRTX_LED
    item(ITEM_IRTX_LED_TEST, uistr_info_irtx_led_test),
#endif

item(ITEM_MAX_IDS, NULL),

};
item_t pc_control_items[] =
{
    item(ITEM_FM,      "AT+FM"),
    item(ITEM_MEMCARD,      "AT+MEMCARD"),
    item(ITEM_SIM,      "AT+SIM"),
    item(ITEM_GPS,      "AT+GPS"),
/*
 * All booting storage devices shall use the same AT command "AT+EMMC".
 *
 * "item_id" can be different but need to sync with SP ATA Tool.
 */
    item(ITEM_STORAGE,	"AT+EMMC"),

    item(ITEM_WIFI,	"AT+WIFI"),
    item(ITEM_LOOPBACK_PHONEMICSPK,      "AT+RINGTONE"),
#if 0
    item(ITEM_SIGNALTEST,      "AT+SIGNALTEST"),
    item(ITEM_RTC,      "AT+RTC"),
#endif
    item(ITEM_CHARGER,      "AT+CHARGER"),
    item(ITEM_BT,      "AT+BT"),
    item(ITEM_MAIN_CAMERA, "AT+MAINCAMERA"),
    item(ITEM_SUB_CAMERA, "AT+SUBCAMERA"),
/* goodix begin */
#ifdef FEATURE_FTM_GOODIX_FINGERPRINT
    item(ITEM_FINGERPRINT_TEST, "AT+FINGERPRINT"),
#endif
/* goodix end */
    item(ITEM_KEYS, "AT+KEY"),
    item(ITEM_MATV_AUTOSCAN, "AT+MATV"),
   	item(ITEM_TOUCH, "AT+MTOUCH"),
    item(ITEM_TOUCH_AUTO, "AT+TOUCH"),
    #if defined(MTK_EMMC_SUPPORT) || defined(MTK_UFS_SUPPORT)
	#if defined(FEATURE_FTM_EMMC) || defined(FEATURE_FTM_UFS)
	item(ITEM_CLREMMC,"AT+FLASH"),
	#endif
    #else  /* NAND */
    #ifdef FEATURE_FTM_FLASH
    item(ITEM_CLRFLASH, "AT+FLASH"),
	#endif
    #endif /* MTK_EMMC_SUPPORT || MTK_UFS_SUPPORT */
    item(ITEM_VIBRATOR, "AT+VIBRATOR"),
    item(ITEM_LED, "AT+LED"),
#ifdef FEATURE_FTM_RECEIVER
    item(ITEM_RECEIVER, "AT+RECEIVER"),
#endif
    item(ITEM_HEADSET, "AT+HEADSET"),
    item(ITEM_CMMB, "AT+CMMB"),
    item(ITEM_GSENSOR, "AT+GSENSOR"),
    item(ITEM_MSENSOR, "AT+MSENSOR"),
    item(ITEM_ALSPS, "AT+ALSPS"),
    item(ITEM_GYROSCOPE, "AT+GYROSCOPE"),
	item(ITEM_HEART_MONITOR, "AT+HEARTMONITOR"),
    item(ITEM_IDLE, "AT+IDLE"),
    #ifdef FEATURE_FTM_LCM
    item(ITEM_LCM, "AT+LCM"),
    #endif
	  //item(ITEM_VIBRATOR_PHONE, "AT+PVIBRATOR"),
    //item(ITEM_RECEIVER_PHONE, "AT+PRECEIVER"),
    //item(ITEM_HEADSET_PHONE, "AT+PHEADSET"),
    //item(ITEM_LOOPBACK_PHONEMICSPK_PHONE, "AT+PLOOPBACK"),
    item(ITEM_MICBIAS, "AT+MICBIAS"),
#if 0
    item(ITEM_RECEIVER_FREQ_RESPONSE, "AT+RECRESPONSE"),
    item(ITEM_SPEAKER_FREQ_RESPONSE, "AT+SPKRESPONSE"),
    item(ITEM_RECEIVER_THD, "AT+RECTHD"),
    item(ITEM_SPEAKER_THD, "AT+SPKTHD"),
    item(ITEM_HEADSET_THD, "AT+HDSTHD"),
#endif
#ifdef FEATURE_FTM_HDMI
    item(ITEM_HDMI, "AT+HDMI"),
#endif
	item(ITEM_MAX_IDS, NULL),
	#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
    item(ITEM_SPEAKER_MONITOR_SET_TMP,      "AT+SPKSETTMP"),
    item(ITEM_SPEAKER_MONITOR,      "AT+SPKMNTR"),
#endif
};
item_t ftm_debug_test_items[] =
{
#ifdef FEATURE_FTM_AUDIO
		item(ITEM_RECEIVER_DEBUG, uistr_info_audio_receiver_debug),
#endif
	item(ITEM_MAX_IDS, NULL),
};

item_t ftm_test_items[] =
{
#ifdef MTK_EFUSE_WRITER_SUPPORT
#ifdef FEATURE_FTM_EFUSE
    item(ITEM_EFUSE, uistr_info_efuse_test),
#endif
#endif
/* goodix begin */
#ifdef FEATURE_FTM_GOODIX_FINGERPRINT
    item(ITEM_FINGERPRINT_TEST, uistr_goodix_fingerprint),
#endif
/* goodix end */

#ifdef FEATURE_FTM_KEYS
    item(ITEM_KEYS,    uistr_keys),
#endif
#ifdef FEATURE_FTM_JOGBALL
    item(ITEM_JOGBALL, uistr_jogball),
#endif
#ifdef FEATURE_FTM_OFN
    item(ITEM_OFN,     uistr_ofn),
#endif
#ifdef FEATURE_FTM_TOUCH
    item(ITEM_TOUCH,   uistr_touch),
    item(ITEM_TOUCH_AUTO,	uistr_touch_auto),
#endif

#ifdef FEATURE_FTM_LCD
    item(ITEM_LCD,     uistr_lcm_test),
    item(ITEM_BACKLIGHT,     uistr_backlight_level),
#endif


#ifdef FEATURE_FTM_LCM
    item(ITEM_LCM,     uistr_lcm),
#endif

#ifdef FEATURE_FTM_FLASH
    item(ITEM_FLASH,   uistr_nand_flash),
#endif
#if defined(FEATURE_FTM_EMMC) || defined(FEATURE_FTM_UFS) || defined(FEATURE_FTM_MNTL)
    item(ITEM_STORAGE,   uistr_storage),
#endif
#ifdef FEATURE_FTM_MEMCARD
    item(ITEM_MEMCARD, uistr_memory_card),
#endif
#ifndef FEATURE_FTM_WIFI_ONLY
#ifdef FEATURE_FTM_SIMCARD
    item(ITEM_SIMCARD, uistr_sim_card),
#endif
#ifdef FEATURE_FTM_SIM
    item(ITEM_SIM, uistr_sim_detect),
#endif
#endif

#if 0
//#ifdef FEATURE_FTM_SIGNALTEST
#ifdef FEATURE_FTM_3GDATA_SMS
#elif defined FEATURE_FTM_3GDATA_ONLY
#elif defined FEATURE_FTM_WIFI_ONLY
#else
    item(ITEM_SIGNALTEST, uistr_sig_test),
#endif
//#endif
#endif

#ifdef FEATURE_FTM_VIBRATOR
    item(ITEM_VIBRATOR, uistr_vibrator),
#endif
#ifdef FEATURE_FTM_LED
    item(ITEM_LED,     uistr_led),
#endif
#if 0
#ifdef FEATURE_FTM_RTC
    item(ITEM_RTC,     uistr_rtc),
#endif
#endif

#ifdef FEATURE_FTM_AP_AUXADC
    item(ITEM_AP_AUXADC, uistr_info_ap_auxadc),
#endif

#ifdef FEATURE_FTM_AUDIO
#if defined(MTK_SPEAKER_MONITOR_SUPPORT)
    item(ITEM_SPEAKER_MONITOR,uistr_info_audio_speaker_monitor_test),
    item(ITEM_SPEAKER_MONITOR_SET_TMP, uistr_info_audio_speaker_monitor_set_temperature),
#endif
    item(ITEM_LOOPBACK_PHONEMICSPK, uistr_info_audio_loopback_phone_mic_speaker),
#ifdef FEATURE_FTM_RECEIVER
    item(ITEM_RECEIVER, uistr_info_audio_receiver),
#endif
#ifdef FEATURE_FTM_LOOPBACK
    item(ITEM_LOOPBACK, uistr_info_audio_loopback),
#endif
#ifdef FEATURE_FTM_ACSLB
    item(ITEM_ACOUSTICLOOPBACK, uistr_info_audio_acoustic_loopback),
#endif
#ifdef FEATURE_FTM_PHONE_MIC_HEADSET_LOOPBACK
    item(ITEM_LOOPBACK1, uistr_info_audio_loopback_phone_mic_headset),
#endif
#ifdef FEATURE_FTM_PHONE_MIC_SPEAKER_LOOPBACK
    item(ITEM_LOOPBACK2, uistr_info_audio_loopback_phone_mic_speaker),
#endif
#ifdef FEATURE_FTM_HEADSET_MIC_SPEAKER_LOOPBACK
    item(ITEM_LOOPBACK3, uistr_info_audio_loopback_headset_mic_speaker),
#endif
#ifdef FEATURE_FTM_WAVE_PLAYBACK
    item(ITEM_WAVEPLAYBACK, uistr_info_audio_loopback_waveplayback),
#endif
    item(ITEM_MICBIAS, uistr_info_audio_micbias),

#ifdef FEATURE_FTM_SPEAKER_CALIBRATION
    item(ITEM_SPEAKER_RDC_CALIBRATION, uistr_info_audio_speaker_rdc_calibration),
#endif
    // Phone Level Test

#if 0
    item(ITEM_RECEIVER_FREQ_RESPONSE, uistr_info_audio_receiver_freq_response),
    item(ITEM_SPEAKER_FREQ_RESPONSE, uistr_info_audio_speaker_freq_response),
    item(ITEM_RECEIVER_THD, uistr_info_audio_receiver_thd),
    item(ITEM_SPEAKER_THD, uistr_info_audio_speaker_thd),
    item(ITEM_HEADSET_THD, uistr_info_audio_headset_thd),
#endif
#endif //FEATURE_FTM_AUDIO

#ifdef FEATURE_FTM_HEADSET
    item(ITEM_HEADSET, uistr_info_headset),
#endif
#ifdef FEATURE_FTM_SPK_OC
    item(ITEM_SPK_OC, uistr_info_speaker_oc),
#endif
#ifdef FEATURE_FTM_OTG
    item(ITEM_OTG, "OTG"),
#endif
#ifdef FEATURE_FTM_USB_TYPEC
    item(ITEM_USB_TYPEC, "USBTYPEC"),
#endif
#ifdef FEATURE_FTM_USB
    item(ITEM_USB, "USB"),
#endif
#ifdef CUSTOM_KERNEL_ACCELEROMETER
    item(ITEM_GSENSOR, uistr_g_sensor),
#endif
#ifdef CUSTOM_KERNEL_ACCELEROMETER
    item(ITEM_GS_CALI, uistr_g_sensor_c),
#endif
#ifdef CUSTOM_KERNEL_MAGNETOMETER
    item(ITEM_MSENSOR, uistr_m_sensor),
#endif
#if defined(CUSTOM_KERNEL_ALSPS) || defined(CUSTOM_KERNEL_ALS) || defined(CUSTOM_KERNEL_PS)
    item(ITEM_ALSPS, uistr_als_ps),
#endif
#ifdef CUSTOM_KERNEL_BAROMETER
    item(ITEM_BAROMETER, uistr_barometer),
#endif
#ifdef CUSTOM_KERNEL_HUMIDITY
    item(ITEM_HUMIDITY, uistr_humidity),
#endif
#ifdef CUSTOM_KERNEL_GYROSCOPE
    item(ITEM_GYROSCOPE, uistr_gyroscope),
    item(ITEM_GYROSCOPE_CALI, uistr_gyroscope_c),
#endif
#ifdef CUSTOM_KERNEL_HEART
    item(ITEM_HEART_MONITOR, uistr_heart_monitor),
#endif

#ifdef FEATURE_FTM_MAIN_CAMERA
    item(ITEM_MAIN_CAMERA,  uistr_main_sensor),
#endif

#ifdef FEATURE_FTM_MAIN2_CAMERA
    item(ITEM_MAIN2_CAMERA,  uistr_main2_sensor),
#endif

#ifdef FEATURE_FTM_MAIN3_CAMERA
item(ITEM_MAIN3_CAMERA,  uistr_main3_sensor),
#endif

#ifdef FEATURE_FTM_MAIN4_CAMERA
item(ITEM_MAIN4_CAMERA,  uistr_main4_sensor),
#endif

#ifdef FEATURE_FTM_SUB_CAMERA
    item(ITEM_SUB_CAMERA, uistr_sub_sensor),
#endif

#ifdef FEATURE_FTM_SUB2_CAMERA
    item(ITEM_SUB2_CAMERA, uistr_sub2_sensor),
#endif

#ifdef FEATURE_FTM_STROBE
    item(ITEM_STROBE, uistr_strobe),
#endif
#ifdef MTK_GPS_SUPPORT
#ifdef FEATURE_FTM_GPS
    item(ITEM_GPS,     uistr_gps),
#endif
#endif

#ifdef MTK_NFC_SUPPORT
    item(ITEM_NFC,    uistr_nfc),
#endif
#ifdef ST_NFC_SUPPORT_FM
    item(ITEM_NFC,    uistr_nfc),
#endif

#ifdef MTK_FM_SUPPORT
#ifdef FEATURE_FTM_FM
#ifdef MTK_FM_RX_SUPPORT
    item(ITEM_FM,      uistr_info_fmr_title),
#endif
#endif
#ifdef FEATURE_FTM_FMTX
#ifdef MTK_FM_TX_SUPPORT
    item(ITEM_FMTX, uistr_info_fmt_title),
#endif
#endif
#endif

#ifdef MTK_BT_SUPPORT
#ifdef FEATURE_FTM_BT
    item(ITEM_BT, uistr_bluetooth),
#endif
#endif

#ifdef MTK_WLAN_SUPPORT
#ifdef FEATURE_FTM_WIFI
    item(ITEM_WIFI, uistr_wifi),
#endif
#endif

#if 1
#ifdef FEATURE_FTM_MATV
    item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif

#if 0
    //item(ITEM_MATV_NORMAL,  "MATV HW Test"),
    item(ITEM_MATV_AUTOSCAN,  uistr_atv),
#endif
#endif

#ifdef FEATURE_FTM_BATTERY
    //item(ITEM_CHARGER, "Battery & Charger"),
   item(ITEM_CHARGER, uistr_info_title_battery_charger),
#endif

#ifdef MTK_DUAL_CHARGER_SUPPORT
#ifdef FEATURE_FTM_SLAVE_CHARGER
    item(ITEM_SLAVE_CHARGER, uistr_info_title_slave_charger),
#endif
#endif

#ifdef FEATURE_FTM_EXT_BUCK
    item(ITEM_EXT_BUCK, uistr_info_title_ext_buck_item),
#endif

#ifdef FEATURE_FTM_EXT_VBAT_BOOST
    item(ITEM_EXT_VBAT_BOOST, uistr_info_title_ext_vbat_boost_item),
#endif

#ifdef FEATURE_FTM_IDLE
    item(ITEM_IDLE,    uistr_idle),
#endif
#ifdef FEATURE_FTM_TVOUT
    item(ITEM_TVOUT,     uistr_info_tvout_item),
#endif
#ifdef FEATURE_FTM_CMMB
    item(ITEM_CMMB, uistr_cmmb),
#endif
#ifdef FEATURE_FTM_HDMI
    item(ITEM_HDMI, "HDMI"),
#endif
#ifdef FEATURE_FTM_TVE
    item(ITEM_CVBS, "CVBS"),
#endif
#ifdef FEATURE_FTM_RF
    item(ITEM_RF_TEST,  uistr_rf_test),
#endif

#ifdef FEATURE_FTM_UNIVERSAL_ANT
    item(ITEM_UNIVERSAL_ANT_TEST,  uistr_ant_test),
#endif

#ifdef FEATURE_FTM_BTS
    item(ITEM_BTS, uistr_bts),
#endif

#ifdef FACTORY_C2K_SUPPORT
#ifndef FEATURE_FTM_UNIVERSAL_ANT
    item(ITEM_RF_C2K_TEST, uistr_rf_c2k_test),
#endif
#endif

#ifdef FEATURE_FTM_IRTX_LED
    item(ITEM_IRTX_LED_TEST, uistr_info_irtx_led_test),
#endif

/* fix build error
#ifdef FEATURE_FTM_MAIN_LENS
    item(ITEM_MAIN_LENS,  uistr_main_lens),
#endif

#ifdef FEATURE_FTM_MAIN2_LENS
    item(ITEM_MAIN2_LENS,  uistr_main2_lens),
#endif

#ifdef FEATURE_FTM_SUB_LENS
    item(ITEM_SUB_LENS,  uistr_sub_lens),
#endif
*/

#ifdef CUSTOM_KERNEL_BIOMETRIC_SENSOR
        item(ITEM_BIOSENSOR, uistr_bio_sensor),
        item(ITEM_BIOSENSOR_CALI, uistr_bio_sensor_c),
#endif

    item(ITEM_MAX_IDS, NULL),
};

item_t ftm_cust_items[ITEM_MAX_IDS];
item_t ftm_cust_auto_items[ITEM_MAX_IDS];


item_t *get_item_list(void)
{
	item_t *items;

	LOGD(TAG "get_item_list");

	items = ftm_cust_items[0].name ? ftm_cust_items : ftm_test_items;

	return items;
}

item_t *get_debug_item_list(void)
{
	item_t *items;

	LOGD(TAG "get_debug_item_list");

	items = ftm_debug_test_items;

	return items;
}

item_t *get_manual_item_list(void)
{
	item_t *items;
	item_t *items_auto;
	int i = 0;
	int j =0;
	LOGD(TAG "get_manual_item_list");

	items = ftm_cust_items[0].name ? ftm_cust_items : ftm_test_items;

	items_auto = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;

	while (items_auto[i].name != NULL)
	{
		for(j =0;items[j].name != NULL ;j++)
		{
			if(strcmp(items[j].name,items_auto[i].name)==0)
			{
				items[j].mode = FTM_AUTO_ITEM;
				LOGD(TAG "%s",items[j].name);
			}
		}
		i++;
	}

	return items;
}

item_t *get_auto_item_list(void)
{
	item_t *items;

	//items = ftm_cust_auto_items[0].name ? ftm_cust_auto_items : ftm_auto_test_items;
	items = ftm_cust_auto_items;

	return items;
}

const char *get_item_name(item_t *item, int id)
{
    if(item == NULL)
    {
        return NULL;
    }

	while (item->name != NULL)
    {
		if (item->id == id)
			return item->name;
		item++;
	}
	return NULL;
}

int get_item_id(item_t *item, char *name)
{

    if((item == NULL) || (name == NULL))
    {
        return -1;
    }

	while (item->name != NULL)
	{
		if(strlen(item->name)==strlen(name))
		{
			if (!strncasecmp(item->name, name, strlen(item->name)))
				return item->id;
		}
		item++;
	}
	return -1;
}

int get_item_test_type(item_t *item, const char *name)
{
    if((item == NULL) || (name == NULL))
    {
        return -1;
    }

    while (item->name != NULL)
	{
		if(strlen(item->name)==strlen(name))
		{
        	if (!strncasecmp(item->name, name, strlen(item->name)))
            {
                return item->mode;
            }
		}
        item++;
    }
    return -1;
}
