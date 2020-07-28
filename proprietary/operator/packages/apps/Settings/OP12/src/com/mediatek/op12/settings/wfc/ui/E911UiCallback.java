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


package com.mediatek.op12.settings.wfc.ui;

import com.mediatek.optin.parser.RequestResponse;


/**
 * Callbacks implemented by activity.
 */
public interface E911UiCallback {
    /**
     * Method to intimate user choice of saving E911 address.
     * @param userDetails userDetails entered
     * @return
     */
    public void saveE911Adress(RequestResponse.Address userDetails);

    /**
     * Method to intimate activity to perform partial validation of address.
     * @param userDetails userDetails entered
     * @return
     */
    public void doPartialValidationE911Add(RequestResponse.Address userDetails);


    /**
     * Method to intimate Activity to show SaveAnyway dialog.
     * @param userDetails userDetails entered
     * @return
     */
    public void showSaveAnywayE911Dialog(RequestResponse.Address userDetails);

    /**
     * Method to intimate Activity to show E911 user interaction fragment.
     * @param userDetails userDetails entered
     * @return
     */
    public void showE911AddressFragment(RequestResponse.Address userDetails);

    /**
     * Method to intimate Activity to show E911 user interaction fragment.
     * @return
     */
    public void finishFragment();

    /**
     * Method to intimate Activity to show E911 user interaction fragment.
     * @return
     */
    public void finishActivity();

    /**
     * Method to intimate Activity about dismissing of dialog interacting with user.
     * @return
     */
    public void onDialogDismiss();

    /**
     * Method to intimate Activity about user's cancel selection.
     * @return
     */
    public void handleUserCancel();

}
