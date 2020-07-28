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
 * MediaTek Inc. (C) 2016. All rights reserved.
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

package com.mediatek.soundrecorder.ext;

import android.media.MediaRecorder;
import android.os.Bundle;
import android.os.storage.StorageManager;

import com.android.soundrecorder.RemainingTimeCalculator;
import com.android.soundrecorder.SoundRecorderService;

/**
 * The Interface to support the customization of recording time calculation.
 */
public interface IRecordingTimeCalculationExt {

    /**
     * Called when host app, succeed to init from intent.
     * Host app need to pass the extras of the intent to this method.
     *
     * @param extras the extra of the intent to start SoundRecorder activity.
     */
    public void setExtras(Bundle extras);

    /**
     * Called when host app need to create a MediaRecorder object.
     * The default implementation will just new a MediaRecorder.
     * The plugin implementation may set extra attribute for this MediaRecorder.
     *
     * @return an instance of MediaRecorder object.
     */
    public MediaRecorder getMediaRecorder();

    /**
     * Called when host app need to create a RemainingTimeCalculator object.
     * The default implementation will just new a RemainingTimeCalculator.
     * The plugin implementation may return a subclass of RemainingTimeCalculator
     * to change the behavior of RemainingTimeCalculator
     *
     * @param storageManager the reference of StorageManager.
     * @param service the reference of SoundRecorderService.
     *
     * @return an instance of RemainingTimeCalculator or its subclass object.
     */
    public RemainingTimeCalculator getRemainingTimeCalculator(
                                           StorageManager storageManager,
                                           SoundRecorderService service);
}