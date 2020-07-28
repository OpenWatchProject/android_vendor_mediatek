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
 * MediaTek Inc. (C) 2015. All rights reserved.
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

package com.mediatek.camera.v2.services.storage;
/**
 * The interface IStorageService.
 * used for those who want know about storage info
 */
public interface IStorageService {
    public static int STORAGE_IS_OUT_OF_SERVICES = 0;
    public static int STORAGE_IS_IN_SERVICES = 1;
    public static int STORAGE_IS_PREPARING = 2;
    /**
     * The interface IStorageStateListener.
     * use for those who want monitor storage
     * state and do some thing for it.
     */
    public interface IStorageStateListener {
        /**
         * Use for response storage state change.
         * @param storageState its values are
         *  STORAGE_IS_OUT_OF_SERVICES/STORAGE_IS_IN_SERVICES/STORAGE_IS_PREPARING
         *  means storage three different states.
         */
        public void onStorageStateChanged(int storageState);
    }
    /**
     * Get the storage state.
     * @return true means can use ,false means can't use
     */
    public boolean isStorageReady();
    /**
     * Get the storage space for record.
     * the space has subtract storage threshold to
     * make sure video can save success.
     * @return left storage space for record.
     */
    public long getRecordStorageSpace();
    /**
     * Get the storage space for capture.
     * the space has subtract storage threshold to
     * make sure capture can save success.
     * @return left storage space for capture.
     */
    public long getCaptureStorageSpace();
    /**
     * Get file storage directory.
     * @return the storage directory.
     */
    public String getFileDirectory();
    /**
     * Get storage hint info.
     * user can use this to show storage state string or
     * do some thing they want do
     * @return string id
     */
    public int getStorageHintInfo();
    /**
     * Register listener to monitor the storage state change.
     * @param listener the use listener.
     */
    public void registerStorageStateListener(IStorageStateListener listener);

    /**
     * Remove the listener to monitor the storage state change.
     * @param listener the use listener.
     */
    public void unRegisterStorageStateListener(IStorageStateListener listener);

}