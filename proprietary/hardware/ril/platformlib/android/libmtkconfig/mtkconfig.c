/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2018. All rights reserved.
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

#include <stdlib.h>
#include <string.h>
#include <mtkconfig.h>
#include <mtk_log.h>
#include <mtk_properties.h>
#include <cutils/properties.h>

char* msgToConfigs(ConfigDef msg);
int mtkGetFeatureByOs(ConfigDef msg, char *value);
int mtkSetFeatureByOs(ConfigDef msg, char *value);
int mtkGetFeatureByDefault(ConfigDef msg, char *value);

static FeatureConfigInfo settings[] = {
#include "featureconfig.h"
};

int mtkGetFeaturePorting(ConfigDef msg, FeatureValue *fv){

    //Need to decide which is the proper function for your devices.
    return mtkGetFeatureByOs(msg, fv->value);
    //return mtkGetFeatureByDefault(msg, fv->value);
}

int mtkSetFeaturePorting(ConfigDef msg, FeatureValue *fv){
    return mtkSetFeatureByOs(msg, fv->value);
}

//Indicate that the OS needs to control the config settings itself. (e.g., Android)
//In OS Controlled, do NOT call mtkGetFeatureByDefault function.
int mtkGetFeatureByOs(ConfigDef msg, char *value){
    switch(msg){
        case CONFIG_SIM_MODE:
            return property_get("persist.radio.multisim.config", value, "dsds");
        case CONFIG_SS_MODE:
            strncpy(value, "1", 10); // Android platform supports Supplementary Service by default
            return strlen(value);
        case CONFIG_SMS: {
            char sms_config[2] = "1";
            int len = strlen(sms_config);
            memcpy(value, sms_config, len);
            return len;
        }
        case CONFIG_CC:
        {
            char cc_config[2] = "1";
            int len = strlen(cc_config);
            memcpy(value, cc_config, len);
            return len;
        }
        case CONFIG_VOLTE:
            return property_get("persist.vendor.volte_support",value,"0");
        case CONFIG_VILTE:
            return property_get("persist.vendor.vilte_support",value,"0");
        case CONFIG_WFC:
            return property_get("persist.vendor.mtk_wfc_support",value,"0");
        case CONFIG_VIWIFI:
            return property_get("persist.vendor.viwifi_support",value,"0");
        case CONFIG_RCSUA:
            return property_get("persist.vendor.mtk_rcs_ua_support",value,"0");
        default:
            break;
    }
    return 0;
}

//Indicate that the OS needs to control the config settings itself. (e.g., Android)
int mtkSetFeatureByOs(ConfigDef msg, char *value){
    switch(msg){
        case CONFIG_SIM_MODE:
            return property_set("persist.radio.multisim.config", value);
        case CONFIG_VOLTE:
            return property_set("persist.vendor.volte_support",value);
        case CONFIG_VILTE:
            return property_set("persist.vendor.vilte_support",value);
        case CONFIG_WFC:
            return property_set("persist.vendor.mtk_wfc_support",value);
        case CONFIG_VIWIFI:
            return property_set("persist.vendor.viwifi_support",value);
        case CONFIG_RCSUA:
            return property_set("persist.vendor.mtk_rcs_ua_support",value);
        default:
            break;
    }
    return 0;
}

//Indicate that the OS does not need to handle the config settings. (e.g., IPCAM)
//The settings can be read from a header file(featureconfig.h).
int mtkGetFeatureByDefault(ConfigDef msg, char *value){
    char* return_value = msgToConfigs(msg);
    int len = strlen(return_value);
    if(len >= MTK_PROPERTY_VALUE_MAX){
        mtkLogD("mtkGetFeatureByDefault","Length exceed the limit, return");
        return 0;
    }
    memcpy(value, return_value, len);
    return len;
}

char* msgToConfigs(ConfigDef msg){
    int i;
    for (i = 0; i < (int)NUM_OF_ELEMS(settings); i++) {
        if(settings[i].requestNumber == msg){
            return settings[i].value;
        }
    }
    return "";
}
