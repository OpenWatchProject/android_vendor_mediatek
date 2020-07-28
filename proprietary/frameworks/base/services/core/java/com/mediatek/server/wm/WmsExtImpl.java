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

package com.mediatek.server.wm;

import android.os.SystemProperties;
import android.util.Slog;
import android.view.DisplayInfo;
import android.view.WindowManager.LayoutParams;

import com.mediatek.appresolutiontuner.ResolutionTunerAppList;

public class WmsExtImpl extends WmsExt {
    private static final String TAG = "WmsExtImpl";
    private static final String TAG_ART = "AppResolutionTuner";

    /// M: add for App Resolution Tuner feature @{
    @Override
    public boolean isAppResolutionTunerSupport() {
        return "1".equals(SystemProperties.get("ro.vendor.app_resolution_tuner"))
              && 0 == SystemProperties.getInt("persist.vendor.dbg.disable.art",0);
    }

    @Override
    public void loadResolutionTunerAppList() {
        getTunerList().loadTunerAppList();
    }

    @Override
    public void setWindowScaleByWL(com.android.server.wm.WindowState win,DisplayInfo displayInfo,
              LayoutParams attrs, int requestedWidth,int requestedHeight) {
        float scale = 1.f;
        int width = displayInfo.logicalWidth;
        int height = displayInfo.logicalHeight;
        String packageName = attrs != null ? attrs.packageName : null;
        String windowName = attrs != null && attrs.getTitle() != null ?
            attrs.getTitle().toString() : null;
        if (packageName != null && windowName != null && !windowName.contains("FastStarting")
               /** splash screen is transient **/
               && (!windowName.contains("Splash Screen"))
               /** PopupWindow is transient **/
               && (!windowName.contains("PopupWindow"))
               /** full screen window **/
               && ((height == requestedHeight && width == requestedWidth)
               || (attrs.width == -1 && attrs.height == -1 && attrs.x == 0 && attrs.y == 0))
               /** app contains in white list **/
               && getTunerList().isScaledByWMS(packageName,windowName)) {
            scale = getTunerList().getScaleValue(packageName);
        }

        if (scale != 1.f) {
            win.mHWScale = scale;
            win.mNeedHWResizer = true;
            Slog.v(TAG_ART,"setWindowScaleByWL - new scale = " + scale
                + " ,set mEnforceSizeCompat/mNeedHWResizer = true" + " , win : " + win
                + " ,attrs=" + attrs.getTitle().toString());
         }
    }

    private ResolutionTunerAppList getTunerList() {
        return ResolutionTunerAppList.getInstance();
    }
    /// @}
}
