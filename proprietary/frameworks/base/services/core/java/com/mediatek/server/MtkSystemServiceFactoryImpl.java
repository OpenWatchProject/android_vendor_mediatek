/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2017. All rights reserved.
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
package com.mediatek.server;

import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.ServiceManager;
import android.util.Log;
import com.android.server.power.ShutdownThread;

import com.mediatek.server.am.AmsExt;
import com.mediatek.server.am.AmsExtImpl;
import com.mediatek.server.anr.AnrManager;
import com.mediatek.server.anr.AnrManagerImpl;
//import com.mediatek.server.display.MtkDisplayManagerService;
//import com.mediatek.server.display.MtkDisplayManagerServiceImpl;
import com.mediatek.server.pm.PmsExt;
import com.mediatek.server.pm.PmsExtImpl;
import com.mediatek.server.powerhal.PowerHalManager;
import com.mediatek.server.powerhal.PowerHalManagerImpl;
import com.mediatek.server.ppl.MtkPplManager;
import com.mediatek.server.ppl.MtkPplManagerImpl;
import com.mediatek.server.wm.WindowManagerDebugger;
import com.mediatek.server.wm.WindowManagerDebuggerImpl;
import com.mediatek.server.wm.WmsExt;
import com.mediatek.server.wm.WmsExtImpl;

public class MtkSystemServiceFactoryImpl extends MtkSystemServiceFactory {
    private static final String TAG = "MtkSystemServiceFactoryImpl";

    private AmsExt mAmsExt = new AmsExtImpl();

/**
     * Starts makeMtkShutdownThread if existed
    */
    @Override
    public ShutdownThread makeMtkShutdownThread() {
        Log.i(TAG, "Start : MTK Shutdown Thread");
        return new MtkShutdownThread();
    }

    @Override
    public AnrManager makeAnrManager() {
        return new AnrManagerImpl();
    }

    @Override
    public PmsExt makePmsExt() {
        return new PmsExtImpl();
    }

    @Override
    public PowerHalManager makePowerHalManager() {
        return new PowerHalManagerImpl();
    }

    @Override
    public MtkPplManager makeMtkPplManager() {
        return new MtkPplManagerImpl();
    }

    @Override
    public AmsExt makeAmsExt() {
        return mAmsExt;
    }

    @Override
    public WmsExt makeWmsExt() {
        return new WmsExtImpl();
    }

    @Override
    public WindowManagerDebugger makeWindowManagerDebugger() {
        return new WindowManagerDebuggerImpl();
    }
/**
    public MtkDisplayManagerService makeMtkDisplayManagerService() {
        return new MtkDisplayManagerServiceImpl();
    }
**/
}
