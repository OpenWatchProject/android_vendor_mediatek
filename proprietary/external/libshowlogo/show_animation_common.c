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

#define MTK_LOG_ENABLE 0
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/klog.h>
#include <cutils/properties.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <fcntl.h>
#include "show_animation_common.h"
#include "fast_charging_common.h"
#include "charging_animation.h"


#ifdef SLOGD
#undef SLOGD
//#define SLOGD(x,...) do { KLOG_ERROR("libshowlogo", x); } while (0)
#define SLOGD(...) do { KLOG_ERROR("libshowlogo", __VA_ARGS__); } while (0)
#endif

#ifdef SLOGE
#undef SLOGE
//#define SLOGE(x,...) do { KLOG_ERROR("libshowlogo", x); } while (0)
#define SLOGE(...) do { KLOG_ERROR("libshowlogo", __VA_ARGS__); } while (0)
#endif

static int charging_low_index = 0;
static int charging_animation_index = 0;
static int version0_charging_index = 0;
static int bits = 0;

#define CHECK_LOGO_BIN_OK  0
#define CHECK_LOGO_BIN_ERROR  -1

struct charging_anim_ui_dimension {
	int cap_left;
	int cap_top;
	int cap_right;
	int cap_bottom;
	int num_left;
	int num_top;
	int num_right;
	int num_bottom;
	int percent_left;
	int percent_top;
	int percent_right;
	int percent_bottom;
	int top_anim_left;
	int top_anim_top;
	int top_anim_right;
	int top_anim_bottom;
	int bar_left;
	int bar_top;
	int bar_right;
	int bar_bottom;
}  ;
struct charging_anim_ui_dimension charg_anim_ui_dimen ;
static int display_fd;
void init_charging_animation_ui_dimension();
int getValue(char* key, char* defValue);

static unsigned short  *number_pic_addr;
static unsigned short  *line_pic_addr;
static unsigned short  *percent_pic_addr;
static unsigned short  *top_animation_addr;

static  int number_pic_size; //size
// line parameter
static  int line_pic_size;

/*
 * Check logo.bin address if valid, and get logo related info
 * pinfo[0] : the number of all pictures in logo.bin
 * pinf0[1] : the whole length of logo.bin
 * pinf0[2] : the index = 0 picture's length
 * pinfo[3+index] - pinfo[2+index] : means index length
 */
int check_logo_index_valid(int index, void * logo_addr, LOGO_PARA_T *logo_info)
{
    unsigned int *pinfo = (unsigned int*)logo_addr;
    logo_info->logonum = pinfo[0];

    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]logonum =%d, index =%d\n", __FUNCTION__,__LINE__,logo_info->logonum, index);
    }
    if (index >= logo_info->logonum)
    {
        if (MTK_LOG_ENABLE == 1) {
            SLOGE("[show_animation_common: %s %d]unsupported logo, index =%d\n", __FUNCTION__,__LINE__, index);
        }
        return CHECK_LOGO_BIN_ERROR;
    }

    if(index < logo_info->logonum - 1) {
        logo_info->logolen = pinfo[3+index] - pinfo[2+index];
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("show_animation_common, pinfo[1]=%d, pinfo[3+index] - pinfo[2+index]= %d\n",
                   pinfo[1], pinfo[3+index] - pinfo[2+index]);
        }
    }
    else {
        logo_info->logolen = pinfo[1] - pinfo[2+index];
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("show_animation_common, pinfo[1]=%d, pinfo[1] - pinfo[2+index] =%d \n",
                   pinfo[1], pinfo[1] - pinfo[2+index]);
        }
    }

    logo_info->inaddr = (unsigned int)logo_addr + pinfo[2+index];
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("show_animation_common, logo_addr=0x%08x , in_addr=0x%08x,  logolen=%d\n",
                    (unsigned int)logo_addr, logo_info->inaddr,  logo_info->logolen);
    }
    return CHECK_LOGO_BIN_OK;
}


int getValue(char* key, char* defValue) {
    char buf[PROPERTY_VALUE_MAX];
    int len = property_get(key,buf,defValue);
    if (MTK_LOG_ENABLE == 1){
        SLOGD("[libshowlogo: %s %d] buf = %s, key = %s\n",__FUNCTION__,__LINE__, buf, key);
    }
    return (atoi(buf));
}


/*
 * Fill resolution structure based on lcd size
 *
 */
void init_charging_animation_ui_dimension() {
    int lcm_width, lcm_height;
    struct fb_var_screeninfo vinfo;
    display_fd = open("/dev/graphics/fb0", O_RDONLY);
    if (display_fd < 0) {

      SLOGD("[show_animation_common: %s %d]open mtkfb fail...\n",__FUNCTION__,__LINE__);

    }

    if (ioctl(display_fd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
      close(display_fd);
      SLOGD("[show_animation_common: %s %d]ioctl FBIOGET_VSCREENINFO failed\n",__FUNCTION__,__LINE__);
    }
    close(display_fd);

    lcm_width = vinfo.xres;
    lcm_height = vinfo.yres;
    int rotation = getValue(MTK_LCM_PHYSICAL_ROTATION_PROP, "0");
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[libshowlogo: %s %d]rotation = %d\n",__FUNCTION__,__LINE__, rotation);
    }
    if ((3 == rotation)|| (1 == rotation)){
        lcm_width = vinfo.yres;
        lcm_height = vinfo.xres;
    }
    SLOGD("[show_animation_common: %s %d] lcm_width and lcm_height= %d , %d \n",__FUNCTION__,__LINE__,lcm_width,lcm_height);
     
    if((lcm_width == 1080) && (lcm_height == 1920)) { // Special handling for FHD
      
      SLOGD("[show_animation_common: %s %d]Inside 1080*1920 \n",__FUNCTION__,__LINE__);
      
      
       charg_anim_ui_dimen.cap_left =387;
       charg_anim_ui_dimen.cap_right= 691;
       charg_anim_ui_dimen.num_left=395;
       charg_anim_ui_dimen.num_right=395+84;
       charg_anim_ui_dimen.percent_left=395+84+84;
       charg_anim_ui_dimen.percent_right=395+84+84+108;
       charg_anim_ui_dimen.top_anim_left=387;
       charg_anim_ui_dimen.top_anim_right=691;
       charg_anim_ui_dimen.bar_left=470;
       charg_anim_ui_dimen.bar_top=356;
       charg_anim_ui_dimen.bar_right=610;
       charg_anim_ui_dimen.bar_bottom=678;
       charg_anim_ui_dimen.cap_top= 802;
       charg_anim_ui_dimen.cap_bottom= 1292 ;
       charg_anim_ui_dimen.num_top=479;
       charg_anim_ui_dimen.num_bottom=600;
       charg_anim_ui_dimen.percent_top=479;
       charg_anim_ui_dimen.percent_bottom=600;
       charg_anim_ui_dimen.top_anim_top=100;
       charg_anim_ui_dimen.top_anim_bottom=152;
    }
  
   else if((lcm_width == 1080) && (lcm_height == 2160)) { // Special handling for FHD+
      
      SLOGD("[show_animation_common: %s %d]Inside 1080*2160 \n",__FUNCTION__,__LINE__);
       charg_anim_ui_dimen.cap_left =387;
       charg_anim_ui_dimen.cap_right= 691;
       charg_anim_ui_dimen.num_left=395;
       charg_anim_ui_dimen.num_right=395+84;
       charg_anim_ui_dimen.percent_left=395+84+84;
       charg_anim_ui_dimen.percent_right=395+84+84+108;
       charg_anim_ui_dimen.top_anim_left=387;
       charg_anim_ui_dimen.top_anim_right=691;
       charg_anim_ui_dimen.bar_left=470;
       charg_anim_ui_dimen.bar_top=356;
       charg_anim_ui_dimen.bar_right=610;
       charg_anim_ui_dimen.bar_bottom=678;
       charg_anim_ui_dimen.cap_top= 922;
       charg_anim_ui_dimen.cap_bottom= 1413 ;
       charg_anim_ui_dimen.num_top=599;
       charg_anim_ui_dimen.num_bottom=720;
       charg_anim_ui_dimen.percent_top=599;
       charg_anim_ui_dimen.percent_bottom=720;
       charg_anim_ui_dimen.top_anim_top=100;
       charg_anim_ui_dimen.top_anim_bottom=152;
      
    }

    else if(lcm_width==1080) {
        charg_anim_ui_dimen.cap_left =406;
        charg_anim_ui_dimen.cap_right= 669;
        charg_anim_ui_dimen.num_left=445;
        charg_anim_ui_dimen.num_right=445+45;
        charg_anim_ui_dimen.percent_left=445+90;
        charg_anim_ui_dimen.percent_right=445+90+57;
        charg_anim_ui_dimen.top_anim_left=406;
        charg_anim_ui_dimen.top_anim_right=669;
        charg_anim_ui_dimen.bar_left=470;
        charg_anim_ui_dimen.bar_top=356;
        charg_anim_ui_dimen.bar_right=610;
        charg_anim_ui_dimen.bar_bottom=678;
      
        if (lcm_height == 2280) {
           SLOGD("[show_animation_common: %s %d]Inside 1080*2280 \n",__FUNCTION__,__LINE__);
           charg_anim_ui_dimen.cap_top= 982;
           charg_anim_ui_dimen.cap_bottom= 1473 ;
           charg_anim_ui_dimen.num_top=659;
           charg_anim_ui_dimen.num_bottom=780;
           charg_anim_ui_dimen.percent_top=659;
           charg_anim_ui_dimen.percent_bottom=780;
           charg_anim_ui_dimen.top_anim_top=100;
           charg_anim_ui_dimen.top_anim_bottom=152;
       }
       else if (lcm_height == 2340) {
         SLOGD("[show_animation_common: %s %d]Inside 1080*2340 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 1012;
         charg_anim_ui_dimen.cap_bottom= 1503 ;
         charg_anim_ui_dimen.num_top=689;
         charg_anim_ui_dimen.num_bottom=810;
         charg_anim_ui_dimen.percent_top=689;
         charg_anim_ui_dimen.percent_bottom=810;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=152;
       }
       else if (lcm_height == 2520) {
         SLOGD("[show_animation_common: %s %d]Inside 1080*2520 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 1112;
         charg_anim_ui_dimen.cap_bottom= 1593 ;
         charg_anim_ui_dimen.num_top=819;
         charg_anim_ui_dimen.num_bottom=819+64;
         charg_anim_ui_dimen.percent_top=819;
         charg_anim_ui_dimen.percent_bottom=819+64;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=147;
       }
    }
    else if(lcm_width ==720) {
       charg_anim_ui_dimen.cap_left =278;
       charg_anim_ui_dimen.cap_right= 441;
       charg_anim_ui_dimen.num_left=290;
       charg_anim_ui_dimen.num_right=335;
       charg_anim_ui_dimen.percent_left=380;
       charg_anim_ui_dimen.percent_right=437;
       charg_anim_ui_dimen.top_anim_left=278;
       charg_anim_ui_dimen.top_anim_right=441;
       charg_anim_ui_dimen.bar_left=313;
       charg_anim_ui_dimen.bar_top=238;
       charg_anim_ui_dimen.bar_right=406;
       charg_anim_ui_dimen.bar_bottom=453;

       if(lcm_height == 1280) {
         SLOGD("[show_animation_common: %s %d]Inside 720*1280 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 556;
         charg_anim_ui_dimen.cap_bottom= 817 ;
         charg_anim_ui_dimen.num_top=386;
         charg_anim_ui_dimen.num_bottom=450;
         charg_anim_ui_dimen.percent_top=386;
         charg_anim_ui_dimen.percent_bottom=450;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=129;
       }
       else if(lcm_height == 1440) {
         SLOGD("[show_animation_common: %s %d]Inside 720*1440 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 636;
         charg_anim_ui_dimen.cap_bottom= 898 ;
         charg_anim_ui_dimen.num_top=465;
         charg_anim_ui_dimen.num_bottom=529;
         charg_anim_ui_dimen.percent_top=465;
         charg_anim_ui_dimen.percent_bottom=529;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=129;
      }
      else if(lcm_height == 1500) {
         SLOGD("[show_animation_common: %s %d]Inside 720*1500 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 640;
         charg_anim_ui_dimen.cap_bottom= 955 ;
         charg_anim_ui_dimen.num_top=450;
         charg_anim_ui_dimen.num_bottom=515;
         charg_anim_ui_dimen.percent_top=450;
         charg_anim_ui_dimen.percent_bottom=515;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=129;
      }
      else if(lcm_height == 1520) {
         SLOGD("[show_animation_common: %s %d]Inside 720*1520 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 653;
         charg_anim_ui_dimen.cap_bottom= 970 ;
         charg_anim_ui_dimen.num_top=460;
         charg_anim_ui_dimen.num_bottom=535;
         charg_anim_ui_dimen.percent_top=460;
         charg_anim_ui_dimen.percent_bottom=535;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=129;
      }
      else if(lcm_height == 1560) {
         SLOGD("[show_animation_common: %s %d]Inside 720*1560 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 678;
         charg_anim_ui_dimen.cap_bottom= 1000 ;
         charg_anim_ui_dimen.num_top=480;
         charg_anim_ui_dimen.num_bottom=575;
         charg_anim_ui_dimen.percent_top=480;
         charg_anim_ui_dimen.percent_bottom=575;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=129;
      }
      else if(lcm_height == 1600) {
         SLOGD("[show_animation_common: %s %d]Inside 720*1600 \n",__FUNCTION__,__LINE__);
         charg_anim_ui_dimen.cap_top= 703;
         charg_anim_ui_dimen.cap_bottom= 1030 ;
         charg_anim_ui_dimen.num_top=500;
         charg_anim_ui_dimen.num_bottom=615;
         charg_anim_ui_dimen.percent_top=500;
         charg_anim_ui_dimen.percent_bottom=615;
         charg_anim_ui_dimen.top_anim_top=100;
         charg_anim_ui_dimen.top_anim_bottom=129;
      }
    }
   
    else if(lcm_width==1440 && lcm_height==2560) {
      SLOGD("[show_animation_common: %s %d]Inside 1440*2560 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =556;
      charg_anim_ui_dimen.cap_top= 1112;
      charg_anim_ui_dimen.cap_right= 882;
      charg_anim_ui_dimen.cap_bottom= 1630 ;
      charg_anim_ui_dimen.num_left=570;
      charg_anim_ui_dimen.num_top=770;
      charg_anim_ui_dimen.num_right=660;
      charg_anim_ui_dimen.num_bottom=898;
      charg_anim_ui_dimen.percent_left=750;
      charg_anim_ui_dimen.percent_top=770;
      charg_anim_ui_dimen.percent_right=864;
      charg_anim_ui_dimen.percent_bottom=898;
      charg_anim_ui_dimen.top_anim_left=556;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=882;
      charg_anim_ui_dimen.top_anim_bottom=158;
      charg_anim_ui_dimen.bar_left=443;
      charg_anim_ui_dimen.bar_top=337;
      charg_anim_ui_dimen.bar_right=574;
      charg_anim_ui_dimen.bar_bottom=641;
    }
    else if(lcm_width==480 && lcm_height==854) {
      SLOGD("[show_animation_common: %s %d]Inside 480*854 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =172;
      charg_anim_ui_dimen.cap_top= 357;
      charg_anim_ui_dimen.cap_right= 307;
      charg_anim_ui_dimen.cap_bottom= 573 ;
      charg_anim_ui_dimen.num_left=172;
      charg_anim_ui_dimen.num_top=213;
      charg_anim_ui_dimen.num_right=210;
      charg_anim_ui_dimen.num_bottom=267;
      charg_anim_ui_dimen.percent_left=248;
      charg_anim_ui_dimen.percent_top=213;
      charg_anim_ui_dimen.percent_right=296;
      charg_anim_ui_dimen.percent_bottom=267;
      charg_anim_ui_dimen.top_anim_left=172;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=307;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=184;
      charg_anim_ui_dimen.bar_top=227;
      charg_anim_ui_dimen.bar_right=294;
      charg_anim_ui_dimen.bar_bottom=437;
    }
    else if(lcm_width==540 && lcm_height==960) {
      SLOGD("[show_animation_common: %s %d]Inside 540*960 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =202;
      charg_anim_ui_dimen.cap_top= 410;
      charg_anim_ui_dimen.cap_right= 337;
      charg_anim_ui_dimen.cap_bottom= 626 ;
      charg_anim_ui_dimen.num_left=202;
      charg_anim_ui_dimen.num_top=266;
      charg_anim_ui_dimen.num_right=240;
      charg_anim_ui_dimen.num_bottom=320;
      charg_anim_ui_dimen.percent_left=278;
      charg_anim_ui_dimen.percent_top=266;
      charg_anim_ui_dimen.percent_right=326;
      charg_anim_ui_dimen.percent_bottom=320;
      charg_anim_ui_dimen.top_anim_left=202;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=337;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=235;
      charg_anim_ui_dimen.bar_top=179;
      charg_anim_ui_dimen.bar_right=305;
      charg_anim_ui_dimen.bar_bottom=340;
    }
    else if(lcm_width==480 && lcm_height==800) {
      SLOGD("[show_animation_common: %s %d]Inside 480*800 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =172;
      charg_anim_ui_dimen.cap_top= 330;
      charg_anim_ui_dimen.cap_right= 307;
      charg_anim_ui_dimen.cap_bottom= 546 ;
      charg_anim_ui_dimen.num_left=178;
      charg_anim_ui_dimen.num_top=190;
      charg_anim_ui_dimen.num_right=216;
      charg_anim_ui_dimen.num_bottom=244;
      charg_anim_ui_dimen.percent_left=254;
      charg_anim_ui_dimen.percent_top=190;
      charg_anim_ui_dimen.percent_right=302;
      charg_anim_ui_dimen.percent_bottom=244;
      charg_anim_ui_dimen.top_anim_left=172;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=307;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=209;
      charg_anim_ui_dimen.bar_top=149;
      charg_anim_ui_dimen.bar_right=271;
      charg_anim_ui_dimen.bar_bottom=282;
    }
    else if(lcm_width==480 && lcm_height==960) { // default wuvga 480*960
      
      SLOGD("[show_animation_common: %s %d]Inside 480*960 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =172;
      charg_anim_ui_dimen.cap_top= 410;
      charg_anim_ui_dimen.cap_right= 307;
      charg_anim_ui_dimen.cap_bottom= 626 ;
      charg_anim_ui_dimen.num_left=172;
      charg_anim_ui_dimen.num_top=266;
      charg_anim_ui_dimen.num_right=210;
      charg_anim_ui_dimen.num_bottom=320;
      charg_anim_ui_dimen.percent_left=248;
      charg_anim_ui_dimen.percent_top=266;
      charg_anim_ui_dimen.percent_right=296;
      charg_anim_ui_dimen.percent_bottom=320;
      charg_anim_ui_dimen.top_anim_left=172;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=307;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=184;
      charg_anim_ui_dimen.bar_top=227;
      charg_anim_ui_dimen.bar_right=294;
      charg_anim_ui_dimen.bar_bottom=437;
    }
    else if(lcm_width==320 && lcm_height==480) {
      SLOGD("[show_animation_common: %s %d]Inside 320*480 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =109;
      charg_anim_ui_dimen.cap_top= 189;
      charg_anim_ui_dimen.cap_right= 211;
      charg_anim_ui_dimen.cap_bottom= 350 ;
      charg_anim_ui_dimen.num_left=126;
      charg_anim_ui_dimen.num_top=95;
      charg_anim_ui_dimen.num_right=153;
      charg_anim_ui_dimen.num_bottom=131;
      charg_anim_ui_dimen.percent_left=180;
      charg_anim_ui_dimen.percent_top=95;
      charg_anim_ui_dimen.percent_right=212;
      charg_anim_ui_dimen.percent_bottom=131;
      charg_anim_ui_dimen.top_anim_left=109;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=211;
      charg_anim_ui_dimen.top_anim_bottom=118;
      charg_anim_ui_dimen.bar_left=129;
      charg_anim_ui_dimen.bar_top=128;
      charg_anim_ui_dimen.bar_right=190;
      charg_anim_ui_dimen.bar_bottom=245;
    }
    else if(lcm_width==240 && lcm_height==240) {
      SLOGD("[show_animation_common: %s %d]Inside 240*240 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =82;
      charg_anim_ui_dimen.cap_top= 62;
      charg_anim_ui_dimen.cap_right= 158;
      charg_anim_ui_dimen.cap_bottom= 179 ;
      charg_anim_ui_dimen.num_left=92;
      charg_anim_ui_dimen.num_top=8;
      charg_anim_ui_dimen.num_right=108;
      charg_anim_ui_dimen.num_bottom=31;
      charg_anim_ui_dimen.percent_left=124;
      charg_anim_ui_dimen.percent_top=8;
      charg_anim_ui_dimen.percent_right=144;
      charg_anim_ui_dimen.percent_bottom=31;
      charg_anim_ui_dimen.top_anim_left=82;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=158;
      charg_anim_ui_dimen.top_anim_bottom=113;
      charg_anim_ui_dimen.bar_left=97;
      charg_anim_ui_dimen.bar_top=96;
      charg_anim_ui_dimen.bar_right=140;
      charg_anim_ui_dimen.bar_bottom=184;
    }
    else if(lcm_width==600 && lcm_height==1024) {
      SLOGD("[show_animation_common: %s %d]Inside 600*1024 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =232;
      charg_anim_ui_dimen.cap_top= 442;
      charg_anim_ui_dimen.cap_right= 367;
      charg_anim_ui_dimen.cap_bottom= 658 ;
      charg_anim_ui_dimen.num_left=250;
      charg_anim_ui_dimen.num_top=300;
      charg_anim_ui_dimen.num_right=288;
      charg_anim_ui_dimen.num_bottom=354;
      charg_anim_ui_dimen.percent_left=326;
      charg_anim_ui_dimen.percent_top=300;
      charg_anim_ui_dimen.percent_right=374;
      charg_anim_ui_dimen.percent_bottom=354;
      charg_anim_ui_dimen.top_anim_left=232;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=367;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=260;
      charg_anim_ui_dimen.bar_top=190;
      charg_anim_ui_dimen.bar_right=338;
      charg_anim_ui_dimen.bar_bottom=360;
    }
    else if(lcm_width==1024 && lcm_height==600) {
      SLOGD("[show_animation_common: %s %d]Inside 1024*600 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =444;
      charg_anim_ui_dimen.cap_top= 230;
      charg_anim_ui_dimen.cap_right= 579;
      charg_anim_ui_dimen.cap_bottom= 446 ;
      charg_anim_ui_dimen.num_left=466;
      charg_anim_ui_dimen.num_top=90;
      charg_anim_ui_dimen.num_right=504;
      charg_anim_ui_dimen.num_bottom=144;
      charg_anim_ui_dimen.percent_left=542;
      charg_anim_ui_dimen.percent_top=90;
      charg_anim_ui_dimen.percent_right=590;
      charg_anim_ui_dimen.percent_bottom=144;
      charg_anim_ui_dimen.top_anim_left=444;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=579;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=414;
      charg_anim_ui_dimen.bar_top=186;
      charg_anim_ui_dimen.bar_right=608;
      charg_anim_ui_dimen.bar_bottom=477;
    }
    else if(lcm_width==1280 && lcm_height==800) {
      SLOGD("[show_animation_common: %s %d]Inside 1280*800 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =558;
      charg_anim_ui_dimen.cap_top= 265;
      charg_anim_ui_dimen.cap_right= 721;
      charg_anim_ui_dimen.cap_bottom= 525 ;
      charg_anim_ui_dimen.num_left=585;
      charg_anim_ui_dimen.num_top=95;
      charg_anim_ui_dimen.num_right=630;
      charg_anim_ui_dimen.num_bottom=159;
      charg_anim_ui_dimen.percent_left=675;
      charg_anim_ui_dimen.percent_top=95;
      charg_anim_ui_dimen.percent_right=732;
      charg_anim_ui_dimen.percent_bottom=159;
      charg_anim_ui_dimen.top_anim_left=558;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=721;
      charg_anim_ui_dimen.top_anim_bottom=129;
      charg_anim_ui_dimen.bar_left=525;
      charg_anim_ui_dimen.bar_top=250;
      charg_anim_ui_dimen.bar_right=755;
      charg_anim_ui_dimen.bar_bottom=640;
    }
    else if(lcm_width==800 && lcm_height==1280) {
      SLOGD("[show_animation_common: %s %d]Inside 800*1280 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =318;
      charg_anim_ui_dimen.cap_top= 556;
      charg_anim_ui_dimen.cap_right= 481;
      charg_anim_ui_dimen.cap_bottom= 815 ;
      charg_anim_ui_dimen.num_left=345;
      charg_anim_ui_dimen.num_top=385;
      charg_anim_ui_dimen.num_right=390;
      charg_anim_ui_dimen.num_bottom=449;
      charg_anim_ui_dimen.percent_left=435;
      charg_anim_ui_dimen.percent_top=385;
      charg_anim_ui_dimen.percent_right=492;
      charg_anim_ui_dimen.percent_bottom=449;
      charg_anim_ui_dimen.top_anim_left=318;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=481;
      charg_anim_ui_dimen.top_anim_bottom=129;
      charg_anim_ui_dimen.bar_left=348;
      charg_anim_ui_dimen.bar_top=238;
      charg_anim_ui_dimen.bar_right=453;
      charg_anim_ui_dimen.bar_bottom=452;
    }
    else if(lcm_width==1920 && lcm_height==1200) {
      SLOGD("[show_animation_common: %s %d]Inside 600*1024 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =806;
      charg_anim_ui_dimen.cap_top= 443;
      charg_anim_ui_dimen.cap_right= 1110;
      charg_anim_ui_dimen.cap_bottom= 929 ;
      charg_anim_ui_dimen.num_left=855;
      charg_anim_ui_dimen.num_top=124;
      charg_anim_ui_dimen.num_right=939;
      charg_anim_ui_dimen.num_bottom=245;
      charg_anim_ui_dimen.percent_left=1023;
      charg_anim_ui_dimen.percent_top=124;
      charg_anim_ui_dimen.percent_right=1131;
      charg_anim_ui_dimen.percent_bottom=245;
      charg_anim_ui_dimen.top_anim_left=806;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=1110;
      charg_anim_ui_dimen.top_anim_bottom=152;
      charg_anim_ui_dimen.bar_left=890;
      charg_anim_ui_dimen.bar_top=357;
      charg_anim_ui_dimen.bar_right=1030;
      charg_anim_ui_dimen.bar_bottom=678;
    }
    else if(lcm_width==1200 && lcm_height==1920) {
      SLOGD("[show_animation_common: %s %d]Inside 1200*1920 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =447;
      charg_anim_ui_dimen.cap_top= 803;
      charg_anim_ui_dimen.cap_right= 751;
      charg_anim_ui_dimen.cap_bottom= 1289 ;
      charg_anim_ui_dimen.num_left=494;
      charg_anim_ui_dimen.num_top=481;
      charg_anim_ui_dimen.num_right=578;
      charg_anim_ui_dimen.num_bottom=602;
      charg_anim_ui_dimen.percent_left=662;
      charg_anim_ui_dimen.percent_top=481;
      charg_anim_ui_dimen.percent_right=770;
      charg_anim_ui_dimen.percent_bottom=602;
      charg_anim_ui_dimen.top_anim_left=447;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=751;
      charg_anim_ui_dimen.top_anim_bottom=152;
      charg_anim_ui_dimen.bar_left=529;
      charg_anim_ui_dimen.bar_top=357;
      charg_anim_ui_dimen.bar_right=672;
      charg_anim_ui_dimen.bar_bottom=680;
    }
    else if(lcm_width==768 && lcm_height==1024) {
      SLOGD("[show_animation_common: %s %d]Inside 768*1024 \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =316;
      charg_anim_ui_dimen.cap_top= 442;
      charg_anim_ui_dimen.cap_right= 451;
      charg_anim_ui_dimen.cap_bottom= 658 ;
      charg_anim_ui_dimen.num_left=338;
      charg_anim_ui_dimen.num_top=300;
      charg_anim_ui_dimen.num_right=376;
      charg_anim_ui_dimen.num_bottom=354;
      charg_anim_ui_dimen.percent_left=414;
      charg_anim_ui_dimen.percent_top=300;
      charg_anim_ui_dimen.percent_right=462;
      charg_anim_ui_dimen.percent_bottom=354;
      charg_anim_ui_dimen.top_anim_left=316;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=451;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=334;
      charg_anim_ui_dimen.bar_top=243;
      charg_anim_ui_dimen.bar_right=434;
      charg_anim_ui_dimen.bar_bottom=463;
    }
    else { // default wvga 480*800
      SLOGD("[show_animation_common: %s %d]default case final else \n",__FUNCTION__,__LINE__);
      charg_anim_ui_dimen.cap_left =172;
      charg_anim_ui_dimen.cap_top= 330;
      charg_anim_ui_dimen.cap_right= 307;
      charg_anim_ui_dimen.cap_bottom= 546 ;
      charg_anim_ui_dimen.num_left=178;
      charg_anim_ui_dimen.num_top=190;
      charg_anim_ui_dimen.num_right=216;
      charg_anim_ui_dimen.num_bottom=244;
      charg_anim_ui_dimen.percent_left=254;
      charg_anim_ui_dimen.percent_top=290;
      charg_anim_ui_dimen.percent_right=302;
      charg_anim_ui_dimen.percent_bottom=244;
      charg_anim_ui_dimen.top_anim_left=172;
      charg_anim_ui_dimen.top_anim_top=100;
      charg_anim_ui_dimen.top_anim_right=307;
      charg_anim_ui_dimen.top_anim_bottom=124;
      charg_anim_ui_dimen.bar_left=209;
      charg_anim_ui_dimen.bar_top=149;
      charg_anim_ui_dimen.bar_right=271;
      charg_anim_ui_dimen.bar_bottom=282;
   }
 }

/*
 * Fill a screen size buffer with logo content
 *
 */
void fill_animation_logo(int index, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    LOGO_PARA_T logo_info;
    int logo_width;
    int logo_height;
    int raw_data_size;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return;

    raw_data_size = decompress_logo((void*)logo_info.inaddr, dec_logo_addr, logo_info.logolen, phical_screen.fb_size);
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]raw_data_size = %d\n",__FUNCTION__,__LINE__, raw_data_size);
    }
    //RECT_REGION_T rect = {0, 0, phical_screen.width, phical_screen.height};
    logo_width =phical_screen.width;
    logo_height = phical_screen.height;
    if (phical_screen.rotation == 270 || phical_screen.rotation == 90) {
        logo_width = phical_screen.height;
        logo_height = phical_screen.width;
    }
    if (0 == bits) {
        if (raw_data_size == logo_width*logo_height*4) {
            bits = 32;
        } else if (raw_data_size == logo_width*logo_height*2) {
            bits = 16;
        } else {
            if (MTK_LOG_ENABLE == 1) {
                SLOGE("[show_animation_common: %s %d]Logo data error\n",__FUNCTION__,__LINE__);
            }
            return;
        }
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]bits = %d\n",__FUNCTION__,__LINE__, bits);
        }
    }

    RECT_REGION_T rect = {0, 0, logo_width, logo_height};

    fill_rect_with_content(fill_addr, rect, dec_logo_addr, phical_screen, bits);
}

/*
 * Fill a rectangle size address with special color
 *
 */
void fill_animation_prog_bar(RECT_REGION_T rect_bar,
                       unsigned int fgColor,
                       unsigned int start_div, unsigned int occupied_div,
                       void *fill_addr, LCM_SCREEN_T phical_screen)
{
    unsigned int div_size  = (rect_bar.bottom - rect_bar.top) / (ANIM_V0_REGIONS);
    unsigned int draw_size = div_size - (ANIM_V0_SPACE_AFTER_REGION);

    unsigned int i;

    for (i = start_div; i < start_div + occupied_div; ++ i)
    {
        unsigned int draw_bottom = rect_bar.bottom - div_size * i - (ANIM_V0_SPACE_AFTER_REGION);
        unsigned int draw_top    = draw_bottom - draw_size;

        RECT_REGION_T rect = {rect_bar.left, draw_top, rect_bar.right, draw_bottom};

        fill_rect_with_color(fill_addr, rect, fgColor, phical_screen);

    }
}


/*
 * Fill a rectangle with logo content
 *
 */
void fill_animation_dynamic(int index, RECT_REGION_T rect, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    LOGO_PARA_T logo_info;
    int raw_data_size;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return;

    raw_data_size = decompress_logo((void*)logo_info.inaddr, (void*)dec_logo_addr, logo_info.logolen, (rect.right-rect.left)*(rect.bottom-rect.top)*4);
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]raw_data_size = %d\n",__FUNCTION__,__LINE__, raw_data_size);
    }
    if (0 == bits) {
        if (raw_data_size == (rect.right-rect.left)*(rect.bottom-rect.top)*4) {
            bits = 32;
        } else if (raw_data_size == (rect.right-rect.left)*(rect.bottom-rect.top)*2) {
            bits = 16;
        } else {
            if (MTK_LOG_ENABLE == 1) {
                SLOGE("[show_animation_common: %s %d]Logo data error\n",__FUNCTION__,__LINE__);
            }
            return;
        }
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]bits = %d\n",__FUNCTION__,__LINE__, bits);
        }
    }
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]bits = %d\n",__FUNCTION__,__LINE__, bits);
    }
    fill_rect_with_content(fill_addr, rect, dec_logo_addr, phical_screen, bits);
}


/*
 * Fill a rectangle  with number logo content
 *
 * number_position: 0~1st number, 1~2nd number
 */
void fill_animation_number(int index, unsigned int number_position, void *fill_addr,  void * logo_addr, LCM_SCREEN_T phical_screen)
{
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]index= %d, number_position = %d\n",__FUNCTION__,__LINE__, index, number_position);
    }
    if(number_pic_addr==NULL) {
        number_pic_addr= (unsigned short*)malloc(((charg_anim_ui_dimen.num_right - charg_anim_ui_dimen.num_left)*(charg_anim_ui_dimen.num_bottom - charg_anim_ui_dimen.num_top)*4)*sizeof(unsigned short));
        memset(number_pic_addr,0,(((charg_anim_ui_dimen.num_right - charg_anim_ui_dimen.num_left)*(charg_anim_ui_dimen.num_bottom - charg_anim_ui_dimen.num_top)*4)*sizeof(unsigned short)));
    }
    LOGO_PARA_T logo_info;
    int raw_data_size;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return;
    number_pic_size = (charg_anim_ui_dimen.num_right-charg_anim_ui_dimen.num_left)*(charg_anim_ui_dimen.num_bottom-charg_anim_ui_dimen.num_top)*4;
    // draw default number rect,
    raw_data_size = decompress_logo((void*)logo_info.inaddr, (void*)number_pic_addr, logo_info.logolen, number_pic_size);
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]raw_data_size = %d\n",__FUNCTION__,__LINE__, raw_data_size);
    }
    //static RECT_REGION_T number_location_rect = {NUMBER_LEFT,NUMBER_TOP,NUMBER_RIGHT,NUMBER_BOTTOM};

    RECT_REGION_T battery_number_rect = {charg_anim_ui_dimen.num_left + (charg_anim_ui_dimen.num_right - charg_anim_ui_dimen.num_left)*number_position,
                            charg_anim_ui_dimen.num_top,
                            charg_anim_ui_dimen.num_right + (charg_anim_ui_dimen.num_right - charg_anim_ui_dimen.num_left)*number_position,
                            charg_anim_ui_dimen.num_bottom};
    if (0 == bits) {
        if (raw_data_size == (charg_anim_ui_dimen.num_right - charg_anim_ui_dimen.num_left)*(charg_anim_ui_dimen.num_bottom - charg_anim_ui_dimen.num_top)*4) {
            bits = 32;
        } else if (raw_data_size == (charg_anim_ui_dimen.num_right - charg_anim_ui_dimen.num_left)*(charg_anim_ui_dimen.num_bottom - charg_anim_ui_dimen.num_top)*2) {
            bits = 16;
        } else {
            if (MTK_LOG_ENABLE == 1) {
                SLOGE("[show_animation_common: %s %d]Logo data error\n",__FUNCTION__,__LINE__);
            }
            return;
        }
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]bits = %d\n",__FUNCTION__,__LINE__, bits);
        }
    }
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]bits = %d\n",__FUNCTION__,__LINE__, bits);
    }
    fill_rect_with_content(fill_addr, battery_number_rect, number_pic_addr,phical_screen, bits);
}

/*
 * Fill a line with special color
 *
 */
void fill_animation_line(int index, unsigned int capacity_grids, void *fill_addr,  void * logo_addr, LCM_SCREEN_T phical_screen)
{

    if(line_pic_addr==NULL) {
       line_pic_addr=(unsigned short*)malloc(((charg_anim_ui_dimen.top_anim_right - charg_anim_ui_dimen.top_anim_left)*4)*sizeof(unsigned short));
       memset(line_pic_addr,0,(((charg_anim_ui_dimen.top_anim_right - charg_anim_ui_dimen.top_anim_left)*4)*sizeof(unsigned short)));
    }
    LOGO_PARA_T logo_info;
    int raw_data_size;
    if(check_logo_index_valid(index, logo_addr, &logo_info) != CHECK_LOGO_BIN_OK)
        return;
    line_pic_size= (charg_anim_ui_dimen.top_anim_right - charg_anim_ui_dimen.top_anim_left)*4;
    raw_data_size = decompress_logo((void*)logo_info.inaddr, (void*)line_pic_addr, logo_info.logolen, line_pic_size);
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]raw_data_size = %d\n",__FUNCTION__,__LINE__, raw_data_size);
    }
    if (0 == bits) {
        if (raw_data_size == (charg_anim_ui_dimen.top_anim_right - charg_anim_ui_dimen.top_anim_left)*4) {
            bits = 32;
        } else if (raw_data_size == (charg_anim_ui_dimen.top_anim_right  - charg_anim_ui_dimen.top_anim_left)*2) {
            bits = 16;
        } else {
            if (MTK_LOG_ENABLE == 1) {
                SLOGE("[show_animation_common: %s %d]Logo data error\n",__FUNCTION__,__LINE__);
            }
            return;
        }
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]bits = %d\n",__FUNCTION__,__LINE__, bits);
        }
    }
    RECT_REGION_T rect = {charg_anim_ui_dimen.cap_left, charg_anim_ui_dimen.cap_top, charg_anim_ui_dimen.cap_right, charg_anim_ui_dimen.cap_bottom};
    int i = capacity_grids;
    for(; i < charg_anim_ui_dimen.cap_bottom; i++)
    {
        rect.top = i;
        rect.bottom = i+1;
        fill_rect_with_content(fill_addr, rect, line_pic_addr, phical_screen, bits);
    }
}



/*
 * Show old charging animation
 *
 */
void fill_animation_battery_old(int capacity,  void *fill_addr, void * dec_logo_addr, void * logo_addr,
                       LCM_SCREEN_T phical_screen)
{
    int capacity_grids = 0;
    if (capacity > 100) capacity = 100;
    capacity_grids = (capacity * (ANIM_V0_REGIONS)) / 100;
    RECT_REGION_T bar_rect = {charg_anim_ui_dimen.bar_left, charg_anim_ui_dimen.bar_top, charg_anim_ui_dimen.bar_right, charg_anim_ui_dimen.bar_bottom};
    if (version0_charging_index < capacity_grids * 2)
        version0_charging_index = capacity_grids * 2;

    if (capacity < 100){
        version0_charging_index > 7? version0_charging_index = capacity_grids * 2 : version0_charging_index++;
    } else {
        version0_charging_index = ANIM_V0_REGIONS * 2;
    }

    fill_animation_logo(ANIM_V0_BACKGROUND_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);

    RECT_REGION_T rect_bar = {bar_rect.left + 1, bar_rect.top + 1, bar_rect.right, bar_rect.bottom};

    fill_animation_prog_bar(rect_bar,
                       (unsigned int)(BAR_OCCUPIED_COLOR),
                       0,  version0_charging_index/2,
                       fill_addr, phical_screen);

    fill_animation_prog_bar(rect_bar,
                      (unsigned int)(BAR_EMPTY_COLOR),
                      version0_charging_index/2, ANIM_V0_REGIONS - version0_charging_index/2,
                      fill_addr, phical_screen);
}

/*
 * Show new charging animation
 *
 */
void fill_animation_battery_new(int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]capacity : %d\n",__FUNCTION__,__LINE__, capacity);
    }
    if (percent_pic_addr==NULL) {
        percent_pic_addr=(unsigned short*)malloc(((charg_anim_ui_dimen.percent_right - charg_anim_ui_dimen.percent_left)*(charg_anim_ui_dimen.percent_bottom - charg_anim_ui_dimen.percent_top)*4)*sizeof(unsigned short));
        memset(percent_pic_addr,0,(((charg_anim_ui_dimen.percent_right - charg_anim_ui_dimen.percent_left)*(charg_anim_ui_dimen.percent_bottom - charg_anim_ui_dimen.percent_top)*4)*sizeof(unsigned short)));
    }
    if (top_animation_addr==NULL) {
        top_animation_addr=(unsigned short*)malloc(((charg_anim_ui_dimen.top_anim_right - charg_anim_ui_dimen.top_anim_left)*(charg_anim_ui_dimen.top_anim_bottom - charg_anim_ui_dimen.top_anim_top)*4)*sizeof(unsigned short));
        memset(top_animation_addr,0,(((charg_anim_ui_dimen.top_anim_right - charg_anim_ui_dimen.top_anim_left)*(charg_anim_ui_dimen.top_anim_bottom - charg_anim_ui_dimen.top_anim_top)*4)*sizeof(unsigned short)));
    }
    RECT_REGION_T percent_location_rect = {charg_anim_ui_dimen.percent_left,charg_anim_ui_dimen.percent_top,charg_anim_ui_dimen.percent_right,charg_anim_ui_dimen.percent_bottom};

    if (capacity >= 100) {
        //show_logo(37); // battery 100
        fill_animation_logo(FULL_BATTERY_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);

    } else if (capacity < 10) {
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]charging_low_index = %d\n",__FUNCTION__,__LINE__, charging_low_index);
        }
        charging_low_index ++ ;

        fill_animation_logo(LOW_BAT_ANIM_START_0 + charging_low_index, fill_addr, dec_logo_addr, logo_addr,phical_screen);
        fill_animation_number(NUMBER_PIC_START_0 + capacity, 1, fill_addr, logo_addr, phical_screen);
        fill_animation_dynamic(NUMBER_PIC_PERCENT, percent_location_rect, fill_addr, percent_pic_addr, logo_addr, phical_screen);

        if (charging_low_index >= 9) charging_low_index = 0;

    } else {

        unsigned int capacity_grids = 0;
        //static RECT_REGION_T battery_rect = {CAPACITY_LEFT,CAPACITY_TOP,CAPACITY_RIGHT,CAPACITY_BOTTOM};
        capacity_grids = charg_anim_ui_dimen.cap_bottom - (charg_anim_ui_dimen.cap_bottom - charg_anim_ui_dimen.cap_top) * (capacity - 10) / 90; 
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]capacity_grids : %d,charging_animation_index = %d\n"
                     ,__FUNCTION__,__LINE__, capacity_grids,charging_animation_index);
        }

        //background
        fill_animation_logo(ANIM_V1_BACKGROUND_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);

        fill_animation_line(ANIM_LINE_INDEX, capacity_grids, fill_addr,  logo_addr, phical_screen);
        fill_animation_number(NUMBER_PIC_START_0 + (capacity/10), 0, fill_addr, logo_addr, phical_screen);
        fill_animation_number(NUMBER_PIC_START_0 + (capacity%10), 1, fill_addr, logo_addr, phical_screen);
        fill_animation_dynamic(NUMBER_PIC_PERCENT, percent_location_rect, fill_addr, percent_pic_addr, logo_addr, phical_screen);


         if (capacity <= 90)
         {
            RECT_REGION_T top_animation_rect = {charg_anim_ui_dimen.top_anim_left, capacity_grids - (charg_anim_ui_dimen.top_anim_bottom - charg_anim_ui_dimen.top_anim_top), charg_anim_ui_dimen.top_anim_right, capacity_grids};
            //top_animation_rect.bottom = capacity_grids;
            //top_animation_rect.top = capacity_grids - top_animation_height;
            charging_animation_index++;
            //show_animation_dynamic(15 + charging_animation_index, top_animation_rect, top_animation_addr);
            fill_animation_dynamic(BAT_ANIM_START_0 + charging_animation_index, top_animation_rect, fill_addr,
                            top_animation_addr, logo_addr, phical_screen);

            if (charging_animation_index >= 9) charging_animation_index = 0;
         }
    }

}

/*
 * Show wireless charging animation
 * total 29 logo:from 39 ~ 68
 * less(0<10): 50-53 , low(<30):54-57 ,middle(<60):58-61 , high():62-75 , o:66, full:67,num (0-9):39-48, %:49
 *
 */

 void fill_animation_battery_wireless_charging(int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen)
{
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]capacity : %d\n",__FUNCTION__,__LINE__, capacity);
    }
//    RECT_REGION_T wireless_bgd_rect = {0, 0, phical_screen.width, phical_screen.height};

    charging_low_index >= 3? charging_low_index = 0:charging_low_index++;
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]charging_low_index = %d\n",__FUNCTION__,__LINE__, charging_low_index);
    }

    if (capacity >= 100) {
         // battery 100
        fill_animation_logo(V2_BAT_100_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);
    } else if (capacity <= 0) {
        fill_animation_logo(V2_BAT_0_INDEX, fill_addr, dec_logo_addr, logo_addr,phical_screen);
    } else {
        int bg_index = V2_BAT_0_10_START_INDEX; //capacity > 0 && capacity < 10
        if (capacity >= 10 && capacity < 40) {
            bg_index = V2_BAT_10_40_START_INDEX;
        } else if (capacity >= 40 && capacity < 80) {
            bg_index = V2_BAT_40_80_START_INDEX;
        } else if (capacity >= 80 && capacity < 100) {
            bg_index = V2_BAT_80_100_START_NDEX;
        }
        fill_animation_logo(bg_index + charging_low_index, fill_addr, dec_logo_addr, logo_addr,phical_screen);
        RECT_REGION_T tmp_rect = {(int)phical_screen.width * 4/10,
                        (int) phical_screen.height * 1/6,
                        (int)phical_screen.width* 5/10,
                        (int)phical_screen.height*16/60};
        unsigned short tmp_num_addr[(int)phical_screen.width * phical_screen.height/100]; //addr

        if (capacity >= 10) {
            if (MTK_LOG_ENABLE == 1) {
                SLOGD("[show_animation_common: %s %d]tmp_rect left = %d, right = %d,top = %d,bottom = %d,\n",__FUNCTION__,__LINE__,
                            tmp_rect.left,tmp_rect.right,tmp_rect.top,tmp_rect.bottom);
            }

            fill_animation_dynamic(V2_NUM_START_0_INDEX + (capacity/10), tmp_rect, fill_addr, tmp_num_addr, logo_addr, phical_screen);
            tmp_rect.left += (int)phical_screen.width /10;
            tmp_rect.right += (int)phical_screen.width /10;
        }

        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]tmp_rect left = %d, right = %d,top = %d,bottom = %d,\n",__FUNCTION__,__LINE__,
                    tmp_rect.left,tmp_rect.right,tmp_rect.top,tmp_rect.bottom);
        }

        fill_animation_dynamic(V2_NUM_START_0_INDEX + (capacity%10), tmp_rect, fill_addr, tmp_num_addr, logo_addr, phical_screen);

        tmp_rect.left += (int)phical_screen.width /10;
        tmp_rect.right += (int)phical_screen.width /10;
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]tmp_rect left = %d, right = %d,top = %d,bottom = %d,\n",__FUNCTION__,__LINE__,
                            tmp_rect.left,tmp_rect.right,tmp_rect.top,tmp_rect.bottom);
        }

        fill_animation_dynamic(V2_NUM_PERCENT_INDEX, tmp_rect, fill_addr, tmp_num_addr, logo_addr, phical_screen);
    }
}

/*
 * Pump charging aniamtion
 * index 39: 100%, 40~45 : animation logo, 46~55: number logo, 56: % logo
 *
 */
void fill_animation_battery_fast_charging(int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr, LCM_SCREEN_T phical_screen, int draw_anim_mode)
{
    int display_width = phical_screen.width;
    int display_height = phical_screen.height;
    int curr_left = 0;
    int num_width = 0;
    int num_height = 0;
    int top_margin_height = 0;
    int rotation = getValue(MTK_LCM_PHYSICAL_ROTATION_PROP, "0");
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[libshowlogo: %s %d]rotation = %d\n",__FUNCTION__,__LINE__, rotation);
    }
    if(draw_anim_mode == DRAW_ANIM_MODE_FB){
       if ((1 == rotation) || (3 == rotation)) {
             display_width = phical_screen.height;
             display_height = phical_screen.width;
          }
       }
    num_width = LOGO_NUM_WIDTH(display_width);
    num_height = LOGO_NUM_HEIGHT(display_height);
    top_margin_height = LOGO_TOP_MARGIN(display_height);
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]capacity : %d, num_width:%d, num_height:%d, top_margin_height:%d\n", __FUNCTION__,__LINE__, capacity, num_width, num_height, top_margin_height);
    }

    charging_low_index >= 5? charging_low_index = 0:charging_low_index++;
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]charging_low_index = %d\n",__FUNCTION__,__LINE__, charging_low_index);

        SLOGD("[show_animation_common: %s %d]capacity : %d\n",__FUNCTION__,__LINE__, capacity);
    }

    if (capacity >= 100) {
        fill_animation_logo(FAST_CHARGING_BAT_100_INDEX, fill_addr, dec_logo_addr, logo_addr, phical_screen);
    } else {
        fill_animation_logo(FAST_CHARGING_BAT_START_0_INDEX + charging_low_index, fill_addr, dec_logo_addr, logo_addr, phical_screen);

        curr_left = (display_width - num_width * 2) >> 1;
        unsigned short tmp_num_addr[(int)phical_screen.width * phical_screen.height/20];

        if (capacity >= 10) {
            curr_left = (display_width - num_width * 3) >> 1;
            RECT_REGION_T tmp_rect_1 = {curr_left, top_margin_height, curr_left + num_width, top_margin_height + num_height};
            if (MTK_LOG_ENABLE == 1) {
                SLOGD("[show_animation_common: %s %d]capacity = %d, show 1nd num : %d\n",__FUNCTION__,__LINE__, capacity, capacity/10);
            }
            fill_animation_dynamic(FAST_CHARGING_NUM_START_0_INDEX + (capacity/10), tmp_rect_1, fill_addr, tmp_num_addr, logo_addr, phical_screen);
            curr_left += num_width;
        }
        RECT_REGION_T tmp_rect_2 = {curr_left, top_margin_height, curr_left + num_width, top_margin_height + num_height};
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]capacity = %d, show 2st num : %d\n",__FUNCTION__,__LINE__, capacity, capacity%10);
        }
        fill_animation_dynamic(FAST_CHARGING_NUM_START_0_INDEX + (capacity%10), tmp_rect_2, fill_addr, tmp_num_addr, logo_addr, phical_screen);

        curr_left += num_width;
        RECT_REGION_T tmp_rect_3 = {curr_left, top_margin_height, curr_left + num_width, top_margin_height + num_height};
        if (MTK_LOG_ENABLE == 1) {
            SLOGD("[show_animation_common: %s %d]show percent \n",__FUNCTION__,__LINE__);
        }
        fill_animation_dynamic(FAST_CHARGING_NUM_PERCENT_INDEX, tmp_rect_3, fill_addr, tmp_num_addr, logo_addr, phical_screen);
    }
}

/*
 * Show charging animation by version
 *
 */
void fill_animation_battery_by_ver(int capacity, void *fill_addr, void * dec_logo_addr, void * logo_addr,
                        LCM_SCREEN_T phical_screen, int version)
{
    if (MTK_LOG_ENABLE == 1) {
        SLOGD("[show_animation_common: %s %d]version : %d\n",__FUNCTION__,__LINE__, version);
    }
    switch (version)
    {
        case VERION_OLD_ANIMATION:
            fill_animation_battery_old(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);

            break;
        case VERION_NEW_ANIMATION:
            fill_animation_battery_new(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);

            break;
        case VERION_WIRELESS_CHARGING_ANIMATION:
            fill_animation_battery_wireless_charging(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);

            break;
        default:
            fill_animation_battery_old(capacity, fill_addr, dec_logo_addr, logo_addr, phical_screen);

            break;
    }
}
