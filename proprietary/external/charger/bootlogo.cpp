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


#include <string.h>

//#include <cutils/log.h>

#include "bootlogo.h"
#include "main.h"


/*
 * Read charging animation feature option, and set charging animation version
 *
 * version 0: show 4 recatangle growing animation without battery number 
 * version 1: show wave animation with  battery number                   
 */
void sync_anim_version()
{
    KPOC_LOGI("[ChargingAnimation: %s %d]\n",__FUNCTION__,__LINE__);

    if (is_wireless_charging())
    {
        set_anim_version(VERION_WIRELESS_CHARGING_ANIMATION);     
    } else {
#ifdef ANIMATION_NEW
        set_anim_version(VERION_NEW_ANIMATION);       
#else
        set_anim_version(VERION_OLD_ANIMATION);
        KPOC_LOGI("[ChargingAnimation %s %d]not define ANIMATION_NEW:show old animation \n",__FUNCTION__,__LINE__); 
#endif 
    }
}

/*
 *  Set charging animation drawing method according to running mode:
 *  running IPO : using surface flinger , DRAW_ANIM_MODE_SURFACE 
 *  running KPOC: using framebuffer     , DRAW_ANIM_MODE_FB
 */
void set_draw_anim_mode(int running_mode)
{
    int draw_anim_mode = DRAW_ANIM_MODE_SURFACE;
    KPOC_LOGD("[ChargingAnimation %s %d]kpoc flag == 1 ? : running_mode = %d \n ",__FUNCTION__,__LINE__ ,running_mode);
    if(running_mode == 1) {
        draw_anim_mode = DRAW_ANIM_MODE_FB;
        KPOC_LOGD("[ChargingAnimation %s %d]under kernel power off charging mode, use fb to draw animation!",__FUNCTION__,__LINE__ );
    } else {
        draw_anim_mode = DRAW_ANIM_MODE_SURFACE; 
    }        

    set_draw_mode(draw_anim_mode);
//    set_draw_mode(DRAW_ANIM_MODE_SURFACE);
}


/*
 * Init charging animation version , drawing method and other parameters
 *
 */
void bootlogo_init()
{
    KPOC_LOGI("[ChargingAnimation: %s %d]\n",__FUNCTION__,__LINE__);
    sync_anim_version();
    anim_init();
}


/*
 * Deinit charging animation 
 *
 */
void bootlogo_deinit()
{
    KPOC_LOGI("[ChargingAnimation: %s %d]\n",__FUNCTION__,__LINE__);
    anim_deinit();
}


/*
 * Show first boot logo when phone boot up
 *
 */ 
void bootlogo_show_boot()
{
    KPOC_LOGI("[ChargingAnimation: %s %d]\n",__FUNCTION__,__LINE__);
    show_boot_logo();
}

/*
 * Show charging animation with battery capacity
 *
 */
void bootlogo_show_charging(int capacity, int cnt)
{
    KPOC_LOGI("[ChargingAnimation: %s %d]%d, %d\n",__FUNCTION__,__LINE__, capacity, cnt);

    if (get_battnotify_status())
    {
        KPOC_LOGI("[ChargingAnimation: %s %d] show_charger_error_logo, get_battnotify_status()= %d \n",__FUNCTION__,__LINE__, get_battnotify_status());
        show_charger_ov_logo();
        return;
    }
    if (showLowBattLogo)
    {
        KPOC_LOGI("[ChargingAnimation: %s %d] show_low_battery , showLowBattLogo = %d \n",__FUNCTION__,__LINE__,showLowBattLogo);
        show_low_battery();
        return;
    }
    show_battery_capacity(capacity);
}


/*
 * Show kernel logo when phone boot up
 *
 */
void bootlogo_show_kernel()
{
    KPOC_LOGI("[ChargingAnimation: %s %d] show  kernel logo \n",__FUNCTION__,__LINE__);
    show_kernel_logo();
}
