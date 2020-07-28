/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.mediatek.cellbroadcastreceiver;

import android.content.Context;
import android.util.Log;

import com.mediatek.cmas.ext.DefaultCmasDuplicateMessageExt;
import com.mediatek.cmas.ext.DefaultCmasMessageInitiationExt;
import com.mediatek.cmas.ext.DefaultCmasSimSwapExt;
import com.mediatek.cmas.ext.DefaultCmasMainSettingsExt;
import com.mediatek.cmas.ext.ICmasDuplicateMessageExt;
import com.mediatek.cmas.ext.ICmasMessageInitiationExt;
import com.mediatek.cmas.ext.ICmasSimSwapExt;
import com.mediatek.cmas.ext.ICmasMainSettingsExt;
import com.mediatek.cmas.ext.OpCmasCustomizationFactoryBase;


public class CellBroadcastPluginManager {
    private static final String TAG = "CellBroadcastPluginManager";

    public static final int CELLBROADCAST_PLUGIN_TYPE_DUPLICATE_MESSAGE = 0x0001;

    public static final int CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION = 0x0002;

    public static final int CELLBROADCAST_PLUGIN_TYPE_SIM_SWAP = 0x0003;

    public static final int CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS = 0x0004;

    private static ICmasDuplicateMessageExt mCmasDuplicateMessagePlugin = null;

    private static ICmasMessageInitiationExt mCmasMessageInitiationPlugin = null;

    private static ICmasSimSwapExt mCmasSimSwapPlugin = null;

    private static ICmasMainSettingsExt mCmasMainSettingsPlugin = null;
    /**
     * Host can initial Plugin object by this method, Just pass the application
     * context or others`
     *
     * @param context
     */
    public static void initPlugins(Context context) {

        // Duplicate message plugin

        mCmasDuplicateMessagePlugin = OpCmasCustomizationFactoryBase
                                        .getOpFactory(context).makeCmasDuplicateMessage(context);

        // Initiation message plugin

        mCmasMessageInitiationPlugin = OpCmasCustomizationFactoryBase
                                        .getOpFactory(context).makeCmasMessageInitiation(context);

        // Sim swap plugin

        mCmasSimSwapPlugin = OpCmasCustomizationFactoryBase
                                        .getOpFactory(context).makeCmasSimSwap(context);

        // Main Setting plugin

        mCmasMainSettingsPlugin = OpCmasCustomizationFactoryBase
                                        .getOpFactory(context).makeCmasMainSettings(context);

    }

    /**
     * Get the Plugin by Plugin type defined in CellBroadcastPluginManager {
     * CELLBROADCAST_PLUGIN_TYPE_DUPLICATE_MESSAGE 0x0001
     * CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION 0x0002
     * CELLBROADCAST_PLUGIN_TYPE_SIM_SWAP 0x0003 }
     *
     * @param type : plugin type
     * @return: plugin object
     */
    public static Object getCellBroadcastPluginObject(int type) {
        Object obj = null;
        Log.d("@M_" + TAG, "getCellBroadcastPlugin, type = " + type);
        switch (type) {

            case CELLBROADCAST_PLUGIN_TYPE_DUPLICATE_MESSAGE:
                obj = mCmasDuplicateMessagePlugin;
                break;

            case CELLBROADCAST_PLUGIN_TYPE_MESSAGE_INITIATION:
                obj = mCmasMessageInitiationPlugin;
                break;

            case CELLBROADCAST_PLUGIN_TYPE_SIM_SWAP:
                obj = mCmasSimSwapPlugin;
                break;

            case CELLBROADCAST_PLUGIN_TYPE_MAIN_SETTINGS:
                obj = mCmasMainSettingsPlugin;
                break;


            default:
                Log.e("@M_" + TAG, "getCellBroadcastPlugin, type = " + type + " doesn't exist");
        }

        return obj;
    }

    public static void resetPluginInterface() {
        mCmasDuplicateMessagePlugin = null;

        mCmasMessageInitiationPlugin = null;

        mCmasSimSwapPlugin = null;

        mCmasMainSettingsPlugin = null;
    }

}
