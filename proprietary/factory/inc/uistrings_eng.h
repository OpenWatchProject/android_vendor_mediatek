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

#ifndef _UISTRINGS_ENG_H_
#define _UISTRINGS_ENG_H_

/*use define rather than variable. because initializer element must be constant*/
#define uistr_factory_mode 		 "Factory Mode"
#define uistr_full_test 		  "Manual Test"
#define uistr_auto_test 		  "Auto Test"
#define uistr_item_test 		  "Item Test"
#define uistr_test_report		  "Test Report"
#define uistr_debug_test 		  "Debug Test"
#define uistr_clear_flash 	  "Clear Flash"
#define uistr_clear_emmc 		  "Clear eMMC"
#define uistr_version 			  "Version"
#define uistr_reboot 				  "Reboot"
#define uistr_pass 					  "Test Pass"
#define uistr_fail 					  "Test Fail"
#define uistr_retest 					"Retest"
#define uistr_key_back 					 "Back"
#define uistr_key_pass 					 "Pass"
#define uistr_key_fail 					 "Fail"
#define uistr_key_pre 					 "Prev"
#define uistr_key_next 					 "Next"
#define uistr_keys 					  "Keys"
#define uistr_jogball 			  "Jogball"
#define uistr_ofn 					  "OFN"
#define uistr_touch 				  "Touch Panel"
#define uistr_touch_auto 				  "Touch Panel Auto Test"
#define uistr_backlight_level 				  "Backlight Level"
#define uistr_lcm_test 				  "LCM Test"
#define uistr_lcm 				  "LCM"
#define uistr_nand_flash 		  "NAND Flash"
#define uistr_storage 					  "Storage"
#define uistr_memory_card 	  "Memory Card"
#define uistr_sim_card 			  "SIM Card"
#define uistr_sim_detect 		  "SIM Detect"
#define uistr_sig_test 			  "Signaling Test"
#define uistr_vibrator 			  "Vibrator"
#define uistr_led 					  "LED"
#define uistr_rtc 					  "RTC"
#define uistr_system_stability 	"System Stability"
#define uistr_nfc 					 "NFC"
#define uistr_cmmb              "CMMB"
#define uistr_gps               "GPS"
#define uistr_atv                   "MATV Autoscan"
#define uistr_wifi						"Wi-Fi"
#define uistr_bluetooth       "Bluetooth"
#define uistr_idle       "Idle Current"

/* AP AuxADC */
#define uistr_info_ap_auxadc		"AP-AuxADC Test"
#define uistr_info_auxadc_chn	"AP-AuxADC Channel Initial"

/*audio*/
#define uistr_info_audio_yes							                "Yes"
#define uistr_info_audio_no							                  "No"
#define uistr_info_audio_press							              "Press"
#define uistr_info_audio_release							            "Release"
#define uistr_info_audio_ringtone							            "Ringtone"
#define uistr_info_audio_receiver							            "Receiver"
#define uistr_info_audio_loopback							            "Loopback"
#define uistr_info_audio_loopback_phone_mic_headset				"Loopback-PhoneMic_HesdsetLR"
#define uistr_info_audio_loopback_phone_mic_speaker				"Mic-Spk loopback"
#define uistr_info_audio_loopback_headset_mic_speaker			"Loopback-HeadsetMic_SpeakerLR"
#define uistr_info_audio_loopback_waveplayback						"WavePlayback"
#define uistr_info_audio_speaker_rdc_calibration					"Speaker Rdc Calibration"
#define uistr_info_audio_loopback_note							      "If HeadsetMic Loopback, \n Insert Headset... \n\n\n"
#define uistr_info_audio_headset_note                     "Insert Headset... \n\n"
#define uistr_info_audio_headset_avail                    "Headset Avail : "
#define uistr_info_audio_headset_mic_avail                "Headset Mic : "
#define uistr_info_audio_headset_Button                   "Headset Button: "
#define uistr_info_audio_loopback_complete							  "Test Loopback Case In-Complete"
#define uistr_info_audio_loopback_headset_mic							"Test Headset-Mic"
#define uistr_info_audio_loopback_dualmic_mic							"Test Mic loopback"
#define uistr_info_audio_loopback_dualmic_mi1							"Test Mic1 loopback"
#define uistr_info_audio_loopback_dualmic_mi2							"Test Mic2 loopback"
#define uistr_info_audio_acoustic_loopback							  "Dual-Mic Loopback"
#define uistr_info_audio_acoustic_loopback_DMNR           "With DMNR"
#define uistr_info_audio_acoustic_loopback_without_DMNR		"Without DMNR"
#define uistr_info_audio_headset_debug "Headset Debug"
#define uistr_info_audio_receiver_debug "Receiver Debug"
#define uistr_info_audio_micbias                          "Micbias Test"
#define uistr_info_audio_micbias_on                       "Micbias ON"
#define uistr_info_audio_micbias_off                      "Micbias OFF"
#define uistr_info_audio_speaker_monitor_test             "Speaker Monitor"
#define uistr_info_audio_speaker_monitor_set_temperature  "Speaker Set Temperature"
#define uistr_info_audio_speaker_add_temperature          "Speaker Temperature +0.5"
#define uistr_info_audio_speaker_sub_temperature          "Speaker Temperature -0.5"
#define uistr_info_audio_speaker_exit_and_save_temperature  "Exit And Save"
#define uistr_info_audio_speaker_exit_not_save_temperature  "Exit"

#define uistr_info_audio_receiver_freq_response           "Receiver Frequency Response Test"
#define uistr_info_audio_speaker_freq_response            "Speaker Frequency Response Test"
#define uistr_info_audio_receiver_thd                     "Receiver THD Test"
#define uistr_info_audio_speaker_thd                      "Speaker THD Test"
#define uistr_info_audio_headset_thd                      "Headset THD Test"

/*phone*/
#define uistr_info_audio_receiver_phone							            "Receiver(Phone)"
#define uistr_info_headset_phone							                  "Headset(Phone)"
#define uistr_info_audio_loopback_phone_mic_speaker_phone				"Loopback-PhoneMic_SpeakerLR(Phone)"
#define uistr_vibrator_phone 			  "Vibrator(Phone)"

/*speaker oc*/
#define uistr_info_speaker_oc							                "Speaker OC Test"
#define uistr_info_speaker_oc_pass							          "Test is passed"
#define uistr_info_speaker_oc_fail							          "Test is failed"
#define uistr_info_speaker_oc_retest							        "SPK OC Test is retesting \n\n"

/*headset*/
#define uistr_info_headset							                  "Headset"
#define uistr_info_avail						                  "Avail"
#define uistr_info_button						                  "button"
#define uistr_info_press						                  "press"
#define uistr_info_release						                  "release"


#define uistr_g_sensor			"G-Sensor"
#define uistr_g_sensor_c		"G-Sensor cali"
#define uistr_m_sensor			"M-Sensor"
#define uistr_als_ps				"ALS/PS"
#define uistr_gyroscope			"Gyroscope"
#define uistr_gyroscope_c		"Gyroscope cali"
#define uistr_barometer			"Barometer"
#define uistr_humidity			"Humidity"
#define uistr_heart_monitor		"Heart Rate Monitor"
#define uistr_bio_sensor_c		"Bio-Sensor"
#define uistr_bio_sensor_c		"Bio-Sensor cali"
#define uistr_rf_test    "RF Test"
#define uistr_rf_c2k_test    "C2K RF test"
#define uistr_ant_test    "Universal ANT Test"
/*emergency_call*/
#define uistr_info_emergency_call							  "Emergency Call"
#define uistr_info_emergency_call_not_start		  "Emergency Call is not start."
#define uistr_info_emergency_call_testing      "signal testing"
#define uistr_info_emergency_call_success			  "dial 112 success!"
#define uistr_info_emergency_call_fail			  "dial 112 failed!"
#define uistr_info_emergency_call_success_in_modem1           "dail 112 success in modem1! "
#define uistr_info_emergency_call_fail_in_modem1           "dail 112 failed in modem1!"
#define uistr_info_emergency_call_success_in_modem2           "dail 112 success in modem2! "
#define uistr_info_emergency_call_fail_in_modem2           "dail 112 failed in modem2!"
#define uistr_info_emergency_call_success_in_modem5           "dail 112 success in modem5! "
#define uistr_info_emergency_call_fail_in_modem5           "dail 112 failed in modem5!"

#define uistr_info_emergency_call_success_in_modem			 "dial 112 success in modem"
#define uistr_info_emergency_call_fail_in_modem			 "dial 112 failed in modem"

#define uistr_info_emergency_call_in_modem1        "modem1 dial 112"
#define uistr_info_emergency_call_in_modem2        "modem2 dial 112"
#define uistr_info_emergency_call_in_modem5        "modem5 dial 112"


/*CMMB*/
#define uistr_info_cmmb_autoscan             "Auto Scan"
#define uistr_info_cmmb_channellist          "Channel List"
#define uistr_info_cmmb_init_ok              "CMMB driver initialization OK."
#define uistr_info_cmmb_init_fail            "CMMB driver initialization failed!"
#define uistr_info_cmmb_scanning             "Scanning channels..."
#define uistr_info_cmmb_scan_ok              "Channels auto scanning OK."
#define uistr_info_cmmb_scan_fail            "Channels auto scanning failed!"
#define uistr_info_cmmb_tune_channel         "Please select a channel to tune,and wait..."
#define uistr_info_cmmb_servicelist          "CMMB Service List"
#define uistr_info_cmmb_selectstream         "Please select a stream to record"
#define uistr_info_cmmb_recording            "CMMB Recording Stream"
#define uistr_info_cmmb_recording_to         "Recording stream to "
#define uistr_info_cmmb_stop                 "Stop"
#define uistr_info_cmmb_stop_to              "Record stopped, file is saved to "


/*eMMC-SD*/
#define uistr_info_emmc			"eMMC"
#define uistr_info_sd			"SD Card"
#define uistr_info_emmc_fat		"eMMC FAT"
#define uistr_info_emmc_format_item		"Format eMMC FAT (Recommend in product line)"
#define uistr_info_emmc_format_stat		"Format Stat"
#define uistr_info_emmc_format_stat_start		"Formatting eMMC FAT..."
#define uistr_info_emmc_format_data_start   "Formatting /data..."
#define uistr_info_reboot		"Rebooting..."
#define uistr_info_emmc_format_stat_success		"Format eMMC FAT Successfully!"
#define uistr_info_emmc_format_stat_fail		"Format eMMC FAT FAILED!"
#define uistr_info_emmc_sd_avail		"Card Avail "
#define uistr_info_emmc_sd_yes		"Yes"
#define uistr_info_emmc_sd_no		"No"
#define uistr_info_emmc_sd_total_size	"Total Size "
#define uistr_info_emmc_sd_free_size	"Free Size  "
#define uistr_info_emmc_sd_checksum		"Checksum   "
#define uistr_info_sd1			"SD Card 1"
#define uistr_info_sd2			"SD Card 2"

/* UFS */
#define uistr_info_ufs			"UFS"

/* MNTL */
#define uistr_info_mntl			"MNTL"

/* Bluetooth */
#define uistr_info_bt_init            "Status: Initializing"
#define uistr_info_bt_init_fail       "Status: INIT FAILED"
#define uistr_info_bt_init_ok         "Status: Initialized"
#define uistr_info_bt_inquiry_start   "Status: Start inquiring...\n"
#define uistr_info_bt_inquiry_1       "Status: Inquiring ----- \n"
#define uistr_info_bt_inquiry_2       "Status: Inquiring +++++ \n"
#define uistr_info_bt_scan_1          "Status: Scanning ----- \n"
#define uistr_info_bt_scan_2          "Status: Scanning +++++ \n"
#define uistr_info_bt_scan_complete   "Status: Scan Completed\n"
#define uistr_info_bt_no_dev          "----End of Device List  No dev found ----\n"
#define uistr_info_bt_dev_list_end    "----End of Device List----\n"
#define uistr_info_bt_scan_list_end   "----End of Scan List----\n"

/*Wi-Fi*/
#define uistr_info_wifi_test_pass   "Test Pass"
#define uistr_info_wifi_test_fail   "Test Fail"
#define uistr_info_wifi_renew       "Renew"
#define uistr_info_wifi_error       "[ERROR]"
#define uistr_info_wifi_warn        "[WARN]"
#define uistr_info_wifi_status      "Status"
#define uistr_info_wifi_start       "Start"
#define uistr_info_wifi_init_fail   "INIT FAILED"
#define uistr_info_wifi_scanning    "Scanning"
#define uistr_info_wifi_timeout     "Connection timed-out"
#define uistr_info_wifi_disconnect  "Disconnect"
#define uistr_info_wifi_connecting  "Connecting"
#define uistr_info_wifi_connected   "Connected"
#define uistr_info_wifi_unknown     "Unknown"
#define uistr_info_wifi_mode        "Mode"
#define uistr_info_wifi_infra       "Infrastructure"
#define uistr_info_wifi_adhoc       "Ad-Hoc"
#define uistr_info_wifi_channel     "Channel"
#define uistr_info_wifi_rssi        "RSSI"
#define uistr_info_wifi_rate        "RATE"
#define uistr_info_wifi_iface_err   "read interface error"
#define uistr_info_wifi_fail_scan   "failed to scan"
#define uistr_info_wifi_no_scan_res "no APs found"
#define uistr_info_wifi_connect_err "connect AP error"
#define uistr_info_wifi_no_ap       "no AP to connect"


/* camera */
#define uistr_main_sensor 	"Main Camera"
#define uistr_main2_sensor 	"Main2 Camera"
#define uistr_main3_sensor 	"Main3 Camera"
#define uistr_main4_sensor 	"Main4 Camera"
#define uistr_sub_sensor 	"Sub Camera"
#define uistr_sub2_sensor 	"Sub2 Camera"
#define uistr_main_lens 	"Main Lens"
#define uistr_main2_lens 	"Main2 Lens"
#define uistr_sub_lens 	 	"Sub Lens"
#define uistr_camera_prv_cap_strobe "Preview/Capture/Strobe"
#define uistr_camera_prv_strobe 	"Preview/Strobe"
#define uistr_camera_prv_cap 	"Preview/Capture"
#define uistr_camera_back       "Back"
#define uistr_camera_capture    "Capture"
#define uistr_camera_preview    "Preview"

/* Strobe */
#define uistr_strobe 	"Strobe"
#define uistr_strobe_test "Strobe Test"



/* USB */
#define uistr_info_usb_connect 		  "Connect"
#define uistr_info_usb_disconnect 	  "Disonnect"

/* battery&charging */
#define uistr_info_title_battery_charger "Battery & Charger"
#define uistr_info_title_ac_charger "AC Charger"
#define uistr_info_title_usb_charger "USB Charger"
#define uistr_info_title_battery_yes "YES"
#define uistr_info_title_battery_no "NO"
#define uistr_info_title_battery_connect "Connect"
#define uistr_info_title_battery_no_connect "NO connect"
#define uistr_info_title_battery_cal "BAT Cal."
#define uistr_info_title_battery_val "BAT Volt."
#define uistr_info_title_battery_temp "BAT Temp."
#define uistr_info_title_battery_chr "CHGR"
#define uistr_info_title_battery_chr_val "CHGR Volt."
#define uistr_info_title_battery_chr_current "CHGR Curr."
#define uistr_info_title_battery_ad32 "AD32"
#define uistr_info_title_battery_ad42 "AD42"
#define uistr_info_title_battery_curad "CurAD"
#define uistr_info_title_battery_fg_cur "FG_Current"
#define uistr_info_title_battery_pmic_chip "PMIC Chip"
#define uistr_info_title_battery_mv "mV"
#define uistr_info_title_battery_ma "mA"
#define uistr_info_title_battery_c "Celsius"

/* slave charger */
#define uistr_info_title_slave_charger "Slave charger"
#define uistr_info_title_slave_charger_connect "Connect"
#define uistr_info_title_slave_charger_no_connect "NO connect"

/* ext buck */
#define uistr_info_title_ext_buck_item "EXT BUCK"
#define uistr_info_title_ext_buck_chip "EXT BUCK"
#define uistr_info_title_ext_buck2_chip "EXT BUCK2"
#define uistr_info_title_ext_buck3_chip "EXT BUCK3"
#define uistr_info_title_ext_buck_connect "Connect"
#define uistr_info_title_ext_buck_no_connect "NO connect"

/* ext vbat boost */
#define uistr_info_title_ext_vbat_boost_item "EXT VBAT BOOST"
#define uistr_info_title_ext_vbat_boost_chip "EXT VBAT BOOST"
#define uistr_info_title_ext_vbat_boost_connect "Connect"
#define uistr_info_title_ext_vbat_boost_no_connect "NO connect"

/* OTG */
#define uistr_info_otg_status "OTG status"
#define uistr_info_otg_status_device "Device"
#define uistr_info_otg_status_host "Host"

/* LED */
#define uistr_info_nled_test			"NLED test"
#define uistr_info_keypad_led_test		"Keypad LED test"

/* Backlight */
#define uistr_info_show_test_images		"Show Test Images"
#define uistr_info_change_contrast		"Change Contrast"

/* System Stability */
#define uistr_info_stress_test_result		"Stress Test Result"

/* GPS */
#define uistr_info_gps_hot_restart    "Hot Restart"
#define uistr_info_gps_cold_restart   "Cold Restart"
#define uistr_info_gps_error          "GPS failed!"
#define uistr_info_gps_fixed          "GPS fixed TTFF(s)"
#define uistr_info_gps_ttff           "GPS TTFF(s)"
#define uistr_info_gps_svid           "SVid"
#define uistr_info_gps_init           "GPS initializing..."

/*NAND*/
#define uistr_info_nand_clear_flash		"Clear Flash"
#define uistr_info_nand_Manufacturer_ID		"Manufacturer ID"
#define uistr_info_nand_Device_ID		"Device ID"
#define uistr_info_nand_R_W_tests_result		"R/W tests result"
#define uistr_info_nand_erase_info		"\n\nClearing /data(%d).\n\n\nPlease wait !\n\n\nOnce clear is done.\n\nsystem will REBOOT!\n"

/*mATV*/
#define uistr_info_atv_autoscan             "Autoscan"
#define uistr_info_atv_channellist          "Channel List"
#define uistr_info_atv_init_ok              "initilize ok"
#define uistr_info_atv_initizling              "opening, please wait"
#define uistr_info_atv_init_fail            "initilize fail"
#define uistr_info_atv_previewCH                 "Preview Channel"
#define uistr_info_atv_switchCH                 "Switch Channel"
#define uistr_info_atv_refreshCH                 "Refresh Channel"
#define uistr_info_atv_CH 				 "CH"
#define	Country_AFGHANISTAN		"Afghanistan"
#define	Country_ARGENTINA		"Argentina"
#define Country_AUSTRALIA		"Australia"
#define Country_BRAZIL		"Brazil"
#define Country_BURMA		"Burma"
#define Country_CAMBODIA		"Cambodia"
#define Country_CANADA		"Canada"
#define Country_CHILE		"Chile"
#define Country_CHINA		"Mainland China"
#define Country_CHINA_HONGKONG		"Chinese Hong Kong"
#define Country_CHINA_SHENZHEN		"Chinese Shenzhen"
#define Country_EUROPE_EASTERN		"Eastern Europe"
#define Country_EUROPE_WESTERN		"Western Europe"
#define Country_FRANCE			"France"
#define Country_FRENCH_COLONIE		"French Colonie"
#define Country_INDIA		"India"
#define Country_INDONESIA		"Indonesia"
#define Country_IRAN		"Iran"
#define Country_ITALY		"Italy"
#define Country_JAPAN		"Japan"
#define Country_KOREA		"Korea"
#define Country_LAOS		"Laos"
#define Country_MALAYSIA		"Malaysia"
#define Country_MEXICO		"Mexico"
#define Country_NEWZEALAND		"NewZealand"
#define Country_PAKISTAN		"Pakistan"
#define Country_PARAGUAY		"Paraguay"
#define Country_PHILIPPINES		"Philippines"
#define Country_PORTUGAL		"Portugal"
#define Country_RUSSIA			"Russia"
#define Country_SINGAPORE		"Singapore"
#define Country_SOUTHAFRICA		"South Africa"
#define Country_SPAIN				"Spain"
#define Country_TAIWAN			"Taiwan"
#define Country_THAILAND		"Thailand"
#define Country_TURKEY		"Turkey"
#define Country_UNITED_ARAB_EMIRATES	"United Arab Emirates"
#define Country_UNITED_KINGDOM		"United Kingdom"
#define Country_USA		"United State of America"
#define Country_URUGUAY		"Uruguay"
#define Country_VENEZUELA		"Venezuela"
#define Country_VIETNAM		"Vietnam"
#define Country_IRELAND		"Ireland"
#define Country_MOROCCO		"Morocco"
#define Country_BANGLADESH	"Bangladesh"
#define Country_EXIT 				"Exit"


/* TV out */

#define uistr_info_tvout_plugin 				 "TV cable is plugged in"
#define uistr_info_tvout_checkifplugin 	 "Check if color bar shown on the TV"
#define uistr_info_tvout_notplugin       "TV cable is NOT plugged in."
#define uistr_info_tvout_item       		 "TV OUT"

/* SIM detect */
#define uistr_info_sim_detect_item_sim_1	"Detect SIM1"
#define uistr_info_sim_detect_item_sim_2	"Detect SIM2"
#define uistr_info_sim_detect_item_pass		"Test Pass"
#define uistr_info_sim_detect_item_fail		"Test Fail"
#define uistr_info_sim_detect_test_result	"Detect SIM%d: %s\n"
#define uistr_info_sim_detect_result_pass	"PASS"
#define uistr_info_sim_detect_result_fail	"FAIL"

/* FM Radio */
#define uistr_info_fmr_title                "FM Radio"
#define uistr_info_fmr_no_headset_warning   "Please Insert Headset.\n"
#define uistr_info_fmr_open_fail            "FM open failed\n"
#define uistr_info_fmr_poweron_fail         "FM power on failed\n"
#define uistr_info_fmr_mute_fail            "FM mute after on failed\n"
#define uistr_info_fmr_poweron_ok           "FM power on successfully\n"
#define uistr_info_fmr_setfreq              "FM set Freq:"
#define uistr_info_fmr_mhz                  "MHz"
#define uistr_info_fmr_fail                 "Failed\n"
#define uistr_info_fmr_success              "Successfully\n"
#define uistr_info_fmr_rssi                 "RSSI: %d(dBm)\n"
#define uistr_info_fmr_freq0                "Test Freq 0"
#define uistr_info_fmr_freq1                "Test Freq 1"
#define uistr_info_fmr_freq2                "Test Freq 2"
#define uistr_info_fmr_freq3                "Test Freq 3"
#define uistr_info_fmr_pass                 "RSSI>=%d(dBm)Test Pass"
#define uistr_info_fmr_failed               "RSSI< %d(dBm)Test Fail,try again"

/* FM Transmitter */
#define uistr_info_fmt_title                "FM Transmitter"
#define uistr_info_fmt_open_fail            "FM TX open failed\n"
#define uistr_info_fmt_poweron_fail         "FM TX power on failed\n"
#define uistr_info_fmt_poweron_ok           "FM TX power on successfully\n"
#define uistr_info_fmt_audio_out            "1K Hz tone output\n"
#define uistr_info_fmt_setfreq              "FM set Freq: "
#define uistr_info_fmt_mhz                  "MHz"
#define uistr_info_fmt_fail                 "Failed\n"
#define uistr_info_fmt_success              "Successfully\n"
#define uistr_info_fmt_check_rds_fail       "Check RDS TX Failed\n"
#define uistr_info_fmt_enable_rds_fail      "Enable RDS TX Failed\n"
#define uistr_info_fmt_set_rds_fail         "set rds faield\n"
#define uistr_info_fmt_rds                  "RDS TX Data:"
#define uistr_info_fmt_freq0                "Test Freq 0"
#define uistr_info_fmt_freq1                "Test Freq 1"
#define uistr_info_fmt_freq2                "Test Freq 2"
#define uistr_info_fmt_freq3                "Test Freq 3"
#define uistr_info_fmt_freq4                "Test Freq 4"
#define uistr_info_fmt_freq5                "Test Freq 5"
#define uistr_info_fmt_freq6                "Test Freq 6"
#define uistr_info_fmt_freq7                "Test Freq 7"
#define uistr_info_fmt_freq8                "Test Freq 8"
#define uistr_info_fmt_freq9                "Test Freq 9"
#define uistr_info_fmt_pass                 "Test Pass"
#define uistr_info_fmt_failed               "Test Fail"

/* Touchpanel */
#define uistr_info_touch_ctp_main			"CTP Test"
#define uistr_info_touch_calibration			"Calibration"
#define uistr_info_touch_rtp_linearity			"RTP Linearity"
#define uistr_info_touch_ctp_linearity			"CTP Linearity"
#define uistr_info_touch_rtp_accuracy		"RTP Accuracy"
#define uistr_info_touch_ctp_accuracy		"CTP Accuracy"
#define uistr_info_touch_sensitivity			"Sensitivity"
#define uistr_info_touch_deadzone				"Deadzone"
#define uistr_info_touch_zoom					"PinchToZoom"
#define uistr_info_touch_freemode				"FreeMode"
#define uistr_info_touch_start					"touch screen to start!"
#define uistr_info_touch_red_cross			"touch the red cross!"
#define uistr_info_touch_pass_continue		"pass! tap to continue"

/* OFN */
#define uistr_info_ofn_fail							"Fail"
#define uistr_info_ofn_pass						"Pass"
#define uistr_info_ofn_back						"Back"
#define uistr_info_ofn_free_mode_item		"Free Mode"
#define uistr_info_ofn_pass_item				"Test Pass"
#define uistr_info_ofn_fail_item					"Test Fail"
#define uistr_info_ofn_return_item				"Return"

/* G/M-Sensor & ALS/PS & Gyroscope */
#define uistr_info_sensor_back							"back"
#define uistr_info_sensor_pass							"pass"
#define uistr_info_sensor_init_fail						"Initialize fail\n"
#define uistr_info_sensor_initializing					"Initializing...\n"
#define uistr_info_sensor_fail								"fail"

#define uistr_info_g_sensor_testing						"in testing"
#define uistr_info_g_sensor_unknow						"unknown"
#define uistr_info_g_sensor_doing							"Ongoing"
#define uistr_info_g_sensor_done							"done"
#define uistr_info_g_sensor_max							"MAX"
#define uistr_info_g_sensor_min							"MIN"
#define uistr_info_g_sensor_range							"Max-Min should be in the range of 0 ~ 0.2\n"
#define uistr_info_g_sensor_selftest						"selftest:"
#define uistr_info_g_sensor_statistic						"statistic:"
#define uistr_info_g_sensor_notsupport					"not supported"
#define uistr_info_g_sensor_avg							"AVG"
#define uistr_info_g_sensor_std								"STD"

#define uistr_info_m_sensor_self								"shipment test"
#define uistr_info_m_sensor_notsupport							"Not support,please ignore"
#define uistr_info_m_sensor_ok									"OK"
#define uistr_info_m_sensor_testing								"testing"
#define uistr_info_m_sensor_fail								"FAIL"
#define uistr_info_m_sensor_status								"status"
#define uistr_info_m_sensor_data								"Msensor Raw Data"

#define uistr_info_sensor_cali_clear								"Clear Cali Data"
#define uistr_info_sensor_cali_do									"Do Calibration"
#define uistr_info_sensor_cali_do_20								"Do Calibration(20%)"
#define uistr_info_sensor_cali_do_40								"Do Calibration(40%)"
#define uistr_info_sensor_cali_ok									"Cali done\n"
#define uistr_info_sensor_cali_fail								"Cali fail\n"
#define uistr_info_sensor_cali_ongoing							"Cali ongoing, don't touch!\n"

#define uistr_info_sensor_alsps_thres_high					"PS THRESHOLD HIGH"
#define uistr_info_sensor_alsps_thres_low					"PS THRESHOLD LOW"
#define uistr_info_sensor_alsps_check_command					"commond not support, please ignore"
#define uistr_info_sensor_alsps_result							"TEST RESULT"

#define uistr_info_sensor_pressure_value							"PRESS"
#define uistr_info_sensor_temperature_value							"TEMP"

#define uistr_info_bio_sensor_max							"MAX"
#define uistr_info_bio_sensor_min							"MIN"
#define uistr_info_bio_sensor_thres						"THRESHOLD"
#define uistr_info_bio_sensor_result						"TEST RESULT"
/*test report*/
#define uistr_info_test_report_back                        "Back"

#define uistr_info_detect_sim1                     "Detect SIM1"
#define uistr_info_detect_sim2                     "Detect SIM2"
#define uistr_info_test_pass                     "Test Pass"
#define uistr_info_test_fail                     "Test Fail"
#define uistr_info_detect_sim                     "Detect SIM"
#define uistr_info_yes                     "Yes"
#define uistr_info_no                     "No"
#define uistr_info_fail                     "Fail"
#define uistr_info_pass                     "Pass"

/* BTS */
#define uistr_bts                           "BTS"

#endif

/* NFC */
#define uistr_info_nfc_swp_test       "NFC_SWP_TEST"
#define uistr_info_nfc_tag_dep        "NFC_TAG_DEP"
#define uistr_info_nfc_card_mode      "NFC_NFC_CARD_MODE"
#define uistr_info_nfc_vcard_mode     "NFC_NFC_VIRTUAL_CARD_MODE"
#define uistr_info_nfc_testing      "Test..."
#define uistr_info_nfc_colse2reader      "CLOSE TO READER"
#define uistr_info_nfc_init              "initializing..."
#define uistr_info_nfc_vcard_removedSIM     "Please Remove SIM or uSD"
#define uistr_info_nfc_put_dut2reader_cm     "card mode configure ok. Please put DUT close to reader device"
#define uistr_info_nfc_put_dut2reader_vcm     "virtual card mode configure ok and please put DUT close to reader device"
/*Hotknot*/
#define uistr_hotknot       "Hotknot"
#define uistr_info_hotknot_master				   "master device"
#define uistr_info_hotknot_slave				   "slave device"
#define uistr_info_hotknot_fail				   "Fail"
#define uistr_info_hotknot_pass				   "Pass"
#define uistr_info_hotknot_mode_select         "Select Master or slave mode"
#define uistr_info_hotknot_info						"Do Hotknot again than will exit"

/* IrTx */
#define uistr_info_irtx_led_test "IrTx LED Test"
#define uistr_info_irtx_open_device "Status: Open devcie node"
#define uistr_info_irtx_call_ioctl "Status: Call ioctl"
#define uistr_info_irtx_led_enable "Enable IRTX LED"
#define uistr_info_irtx_led_disable "Disable IRTX LED"

/* Fingerprint */
/* goodix begin */
#define uistr_goodix_fingerprint "Fingerprint"
/* goodix end */

/* efuse */
#define uistr_info_efuse_test   "EFUSE"
#define uistr_info_efuse_result "EFUSE Self-blow result"
#define uistr_info_efuse_success "SUCCESS"
#define uistr_info_efuse_reblow "REBLOW"
#define uistr_info_efuse_broken "BROKEN"
#define uistr_info_efuse_unknown "UNKNOWN"
