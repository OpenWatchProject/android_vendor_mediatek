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
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "common.h"
#include "miniui.h"
#include "ftm.h"

#include <linux/hdmitx.h>

#ifdef FEATURE_FTM_HDMI

#include "ftm_audio_Common.h"

#ifdef MTK_HDMI_HDCP_SUPPORT
#ifdef MTK_HDCP_DRM_KEY_MNG_SUPPORT
#include "tz_cross/keyblock.h"
#include <dlfcn.h>
#endif
#endif

#define TAG                 "[HDMI] "


static const char *devpath = "/dev/hdmitx";

#define mod_to_hdmi(p)	(hdmi_module *)((char *)(p) + sizeof(struct ftm_module))

#define ARY_SIZE(x)     (sizeof((x)) / sizeof((x[0])))
enum {
    ITEM_PASS,
    ITEM_FAIL,
};
static item_t hdmi_item[] = {
    item(ITEM_PASS,   uistr_pass),
	item(ITEM_FAIL,   uistr_fail),
	item(-1,          NULL),
};

typedef struct {
	struct ftm_module *module;
    char info[1024];

    /* item view */
	struct itemview *itm_view;
    text_t title;
    text_t text;

} hdmi_module;



static int hdmi_enable()
{
    int fd = -1;
    int ret = 0;
    int check_res = 0;
    int capability = 0;

    fd = open(devpath, O_RDONLY);
    if (fd == -1)
    {
        LOGE(TAG "Error, Can't open /dev/hdmitx\n");
        return -1;
    }

    ret = ioctl(fd, MTK_HDMI_GET_CAPABILITY, &capability);
    if (ret < 0 || (capability & HDMI_FACTORY_MODE_NEW) == 0)
    {
        ret = ioctl(fd, MTK_HDMI_FACTORY_MODE_ENABLE, 1);
        if(ret < 0)
        {
            check_res = -1;
            goto check_exit;
        }

        check_res = ioctl(fd, MTK_HDMI_AUDIO_VIDEO_ENABLE, 1);
        if(check_res < 0)
        {
            goto check_exit;
        }

    } else if (capability & HDMI_FACTORY_MODE_NEW) {
        ret = ioctl(fd, MTK_HDMI_FACTORY_CHIP_INIT, &check_res);
        if(ret < 0)
        {
            LOGD(TAG "MTK_HDMI_FACTORY_CHIP_INIT Fail\n");
            goto check_exit;
        }
    }    

    LOGD(TAG "ioctl MTK_HDMI_AUDIO_VIDEO_ENABLE \n");

    ioctl(fd, MTK_HDMI_AUDIO_VIDEO_ENABLE, 1);

    check_res = ioctl(fd, MTK_HDMI_AUDIO_CONFIG,( 2 | (HDMI_MAX_SAMPLERATE_44 << 4) | (HDMI_MAX_BITWIDTH_16 << 7)));
    if(check_res < 0)
    {
		LOGD(TAG "ioctl MTK_HDMI_AUDIO_CONFIG fail\n");			
		//goto check_exit;
    }

    LOGD(TAG "ioctl MTK_HDMI_AUDIO_ENABLE \n");
    check_res = ioctl(fd, MTK_HDMI_AUDIO_ENABLE, 1);
    if(check_res < 0)
    {
		LOGD(TAG "ioctl MTK_HDMI_AUDIO_ENABLE fail\n");
		//goto check_exit;
    }

  //  LOGD(TAG "hdmi_enable to start audio \n");
  //  Common_Audio_init();
  //  LOGD(TAG "hdmi_enable to start sinetone \n");
  //  Audio_HDMI_SineTonePlayback(true, 44100);
  //  LOGD(TAG "hdmi_enable to start audio done\n");
    
check_exit:

    close(fd);
    return check_res;
}


static int hdmi_disable()
{
    int fd = -1;
    int ret = 0, check_res = 0;
    int capability = 0;
    fd = open(devpath, O_RDONLY);
    if (fd == -1)
    {
        LOGE(TAG "Error, Can't open /dev/hdmitx\n");
        return -1;
    }

    ret = ioctl(fd, MTK_HDMI_GET_CAPABILITY, &capability);
    if (ret < 0 || ((capability & HDMI_FACTORY_MODE_NEW) == 0))
    {
        ret = ioctl(fd, MTK_HDMI_AUDIO_VIDEO_ENABLE, 0);
        ioctl(fd, MTK_HDMI_FACTORY_MODE_ENABLE, 0);
    }else if (capability & HDMI_FACTORY_MODE_NEW)
        ret = ioctl(fd, MTK_HDMI_FACTORY_DPI_STOP_AND_POWER_OFF, &check_res);
    close(fd);

  //  LOGD(TAG "hdmi_enable to stop audio \n");
  // Audio_HDMI_SineTonePlayback(false, 44100);
  //  LOGD(TAG "hdmi_enable to stop audio sinetone done\n");
  //  Common_Audio_deinit();
  //  LOGD(TAG "hdmi_enable to stop audio done\n");

    return ret;
}


int hdmi_entry(struct ftm_param *param, void *priv)
{
    bool exit = false;
    hdmi_module *hdmi = (hdmi_module *)priv;
    int check_res = 0;
    int check_cnt = 0;
    int capability = 0;
    int ret = 0;
    struct itemview *iv;
    bool result_auto = true;
    bool result_manual = true;
#ifdef MTK_HDMI_HDCP_SUPPORT
#ifdef MTK_HDCP_DRM_KEY_MNG_SUPPORT
    unsigned int leng=0;
#endif
#endif
    int size = 0;
    LOGD(TAG "hdmi_entry\n");

     /* show text view */
    if (!hdmi->itm_view) {
        iv = ui_new_itemview();
        if (!iv) {
            LOGD(TAG "No memory for item view");
            return -1;
        }
        hdmi->itm_view = iv;
    }

    iv = hdmi->itm_view;

    /*init item view*/
    memset(&hdmi->info[0], 0, sizeof(hdmi->info));
    memset(&hdmi->info[0], '\n', 10);
    init_text(&hdmi->title, param->name, COLOR_YELLOW);
    init_text(&hdmi->text, &hdmi->info[0], COLOR_YELLOW);

    iv->set_title(iv, &hdmi->title);
    iv->set_items(iv, hdmi_item, 0);
    iv->set_text(iv, &hdmi->text);

    /*iv->redraw(iv);*/
	if(get_is_ata() == 1)
		iv->redraw(iv);

	if(hdmi_enable()) {
		LOGD(TAG "hdmi test fail\n");
		/*hdmi->text.color = COLOR_RED;*/
		sprintf(hdmi->info, "%s%s\n",  "HDMI ", uistr_fail);
    } else {
        LOGD(TAG "hdmi basic test pass and to test connect\n");
        sprintf(hdmi->info, "%s", "please insert cable.\n");

		int fd = open(devpath, O_RDONLY);
		if(get_is_ata() == 0){
	        while(1) {
	            check_res = 0;
	            ret = ioctl(fd, MTK_HDMI_GET_CAPABILITY, &capability);
	            if (ret >= 0 && (capability & HDMI_FACTORY_MODE_NEW))
	                ioctl(fd, MTK_HDMI_FACTORY_JUDGE_CALLBACK, &check_res);
	            else
	                ioctl(fd, MTK_HDMI_FACTORY_GET_STATUS, &check_res);


	            if(check_res <= 0 )
	                check_cnt++;
	            else
	                break;

				if(check_cnt > 60) {
					sprintf(hdmi->info, "%s", "please check your cable and retest.\n");
					close(fd);
					goto skip_dpi_exit;
				}
				if (ret >= 0 && (capability & HDMI_FACTORY_MODE_NEW))
		            usleep(1000*500);
	        }

	        if (ret >= 0 && (capability & HDMI_FACTORY_MODE_NEW)) {
	            ret = ioctl(fd, MTK_HDMI_FACTORY_START_DPI_AND_CONFIG, 0x20000);	 //720p30
	            if(ret < 0) {
	                LOGD(TAG "MTK_HDMI_FACTORY_START_DPI_AND_CONFIG Fail\n");
	                close(fd);
	                goto skip_dpi_exit;
	            }
	        } else {
	            ioctl(fd, MTK_HDMI_VIDEO_CONFIG, 2);
	            usleep(1000*30);
	            ioctl(fd, MTK_HDMI_FACTORY_DPI_TEST, &check_res);
	        }
	        /*hdmi->text.color = COLOR_GREEN;*/
	        /*sprintf(hdmi->info, "HDMI Basic "uistr_pass" \n");*/

	        sprintf(hdmi->info, "%s%s%s", "HDMI Basic ", uistr_pass, " please check TV status \n");

	        close(fd);
	        usleep(1000*100);
	     } else {
	        int loopTimes;
	        char logStr[128];
			char mainlog[128];

			/*Check If HDCP KEY Installed */
#ifdef MTK_HDMI_HDCP_SUPPORT
#ifdef MTK_HDCP_DRM_KEY_MNG_SUPPORT
			int (*get_encrypt_drmkey_hidl)(uint32_t);
			void *handle;

			handle = dlopen("/vendor/lib/hw/vendor.mediatek.hardware.keymanage@1.0-impl.so", RTLD_LAZY);
			if (NULL == handle)
				LOGD(TAG "dlopen vendor.mediatek.hardware.keymanage@1.0-impl.so Fail\n");
			dlerror();
			get_encrypt_drmkey_hidl = dlsym(handle, "get_encrypt_drmkey_hidl_c");
			size = (*get_encrypt_drmkey_hidl)(HDCP_1X_TX_ID);
#else
			size = -1;
#endif
#else
			size = -1;
#endif

			if (size == -1) {

				sprintf (logStr, "%d:HDCP KEY NOT FOUND\r\n", ITEM_HDMI);
				write_data_to_pc (logStr, strlen(logStr));
				LOGE(TAG "HDCP KEY NOT FOUND\n");
				sprintf(hdmi->info, "%s", "HDCP KEY NOT FOUND, Test fail\n");

				result_auto = false;
				goto ata_exit;
			} else {
				LOGE(TAG "HDCP key found\n");
				sprintf (logStr, "%d:HDCP KEY FOUND\r\n", ITEM_HDMI);
				write_data_to_pc (logStr, strlen(logStr));
			}

	        /* Wait cable plug in */
	        for(loopTimes = 0; loopTimes < 2; loopTimes++) {
	            ioctl(fd, MTK_HDMI_FACTORY_GET_STATUS, &check_res);

	            if(check_res >= 0) {
					LOGE(TAG "hdmi plug in\r\n");
					sprintf (logStr, "%d:plug in\r\n", ITEM_HDMI);
					write_data_to_pc (logStr, strlen(logStr));
	                /* sprintf(hdmi->info, "please check your cable and retest.\n"); */
	                break;
				} else if(check_res == -3) {
					LOGE(TAG "hdmi EDID check fail\r\n");
					sprintf (logStr, "%d:EDID check fail\r\n", ITEM_HDMI);
					write_data_to_pc (logStr, strlen(logStr));
					sprintf(hdmi->info, "%s", "EDID check fail, Test fail\n");
					result_auto = false;
					goto ata_exit;
					/* sprintf(hdmi->info, "please check your cable and retest.\n"); */
					break;
				} else {
					LOGE(TAG "didn't plug in\n");
					sprintf(hdmi->info, "%s", "Cable didn't plug in, Test fail\n");
					result_auto = false;
					goto ata_exit;
					break;
				}
					sleep(1);
	        }

			if(check_res == 1) {
				ioctl(fd, MTK_HDMI_VIDEO_CONFIG, 2);
	            usleep(1000*30);
				ioctl(fd, MTK_HDMI_FACTORY_DPI_TEST, &check_res);
				sleep(2);

				if (result_auto == true)
					sprintf(hdmi->info, "%s%s%s", "HDMI Basic ", uistr_pass, " TV show color bar before plug out?\n");
				else
					sprintf(hdmi->info, "%s%s%s", "HDMI Basic ", uistr_fail, " fail before video config\n");
	         }

			LOGE(TAG "waiting plug out\n");
	        /*Wait cable plug out*/
	        if (result_auto)  {
	            loopTimes = 0;
	            do {
	                ioctl(fd, MTK_HDMI_FACTORY_GET_STATUS, &check_res);
	                if(check_res < 0) {
	                    sprintf (logStr, "%d:plug out\r\n", ITEM_HDMI);
						LOGE(TAG "plug out\n");
	                    write_data_to_pc (logStr, strlen(logStr));
	                    break;
	                }

	                loopTimes ++;
	                if (loopTimes > 15) {
	                    result_manual = false;
						sprintf (logStr, "%d:fail\r\n", ITEM_HDMI);
						LOGE(TAG "plug out fail\n");
						write_data_to_pc (logStr, strlen(logStr));
						sprintf(hdmi->info, "%s", "Time of plug out more then 15s, Test fail\n");
	                    break;
	                }

	                sleep (1);
	            } while (1);
			}
			LOGE(TAG "plug out done\n");
ata_exit:
	        if (result_auto && result_manual)
	            hdmi->module->test_result = FTM_TEST_PASS;
	        else {
	            hdmi->module->test_result = FTM_TEST_FAIL;
				sprintf (logStr, "%d:fail\r\n", ITEM_HDMI);
				LOGE(TAG "test fail\n");
				write_data_to_pc (logStr, strlen(logStr));
				exit = true;
	        }
		}
    }

skip_dpi_exit:

    while(!exit) {
        switch(iv->run(iv, NULL)) {

        case ITEM_PASS:
            hdmi->module->test_result = FTM_TEST_PASS;
            exit = true;
            break;
        case ITEM_FAIL:
            hdmi->module->test_result = FTM_TEST_FAIL;
            exit = true;
            break;
        case -1:
            exit = true;
            break;
        default:
            break;
        }
    }

    hdmi_disable();
    return 0;
}

int hdmi_init(void)
{
    int ret = 0;

    struct ftm_module *mod;
	hdmi_module *hdmi;

    LOGD(TAG "hdmi_init\n");

    mod = ftm_alloc(ITEM_HDMI, sizeof(hdmi_module));
    if (!mod)
    {
        LOGD(TAG "hdmi_init failed\n");
        return -ENOMEM;
    }

	hdmi = mod_to_hdmi(mod);
    hdmi->module = mod;

    ret = ftm_register(mod, hdmi_entry, (void*)hdmi);
    if (ret)
    {
		LOGE(TAG "register HDMI failed (%d)\n", ret);
    }

    return ret;
}
#endif

