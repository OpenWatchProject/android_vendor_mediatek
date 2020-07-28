/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2018. All rights reserved.
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
 */

package com.mediatek.incallui.ext;

import android.os.Bundle;
import android.os.Handler;
import android.telecom.DisconnectCause;
import android.util.Log;

import java.util.List;

/**
 * Default implementation for IInCallExt.
 */
public class DefaultInCallExt implements IInCallExt {

  /**
   * Called to show toast and beep when handover
   * from Wifi to LTE.
   * @param handler
   * @param stage handover started or over
   * @param ratTYpe wifi or LTE
   */
  @Override
  public void showHandoverNotification(Handler handler, int stage, int ratType) {
    // do nothing
  }

  /**
   * Called when incallpresenter setup.
   * @param statusbarNotifier
   * @param state
   * @param callList
   */
  @Override
  public void onInCallPresenterSetUp(Object statusbarNotifier,
      Object state, Object callList) {
    // do nothing
  }

  /**
   * Called when incallpresenter teardown.
   */
  @Override
  public void onInCallPresenterTearDown() {
    // do nothing
  }

  /**
   * Called to show battery dialog.
   * @param newstate
   * @param oldState
   */
  @Override
  public void maybeShowBatteryDialog(Object newstate, Object oldState){
    // do nothing
  }

  /**
   * Called to dismiss battery dialog.
   */
  @Override
  public void maybeDismissBatteryDialog(){
    // do nothing
  }

  /**
   * customize SelectPhoneAccountDialogFragment.
   * @param disconnectCause DisconnectCause
   */
  @Override
  public boolean maybeShowErrorDialog(DisconnectCause disconnectCause){
    Log.d("DefaultInCallExt", "maybeShowErrorDialog disconnectCause = " + disconnectCause);
    return false;
  }

  /**
   * remove congrats alert.
   * @param disconnectCause DisconnectCause
   * @return true if popup is shown
   */
  @Override
  public boolean showCongratsPopup(DisconnectCause disconnectCause) {
    Log.d("DefaultInCallExt", "maybeShowErrorDialog disconnectCause = " + disconnectCause);
    return false;
  }

  /**
   * Connection event received from telecomm.
   * @param call object
   * @param event which is received from telecomm
   * @param extras received with the event
   */
  @Override
  public void onConnectionEvent(Object call, String event, Bundle extras) {
    // do nothing
  }

  /**
   * Add intent list to process intent.
   * @param intentBundle the intentExtraBundle key list
   * @param extraBundle the extraBundle key list
   */
  @Override
  public void addFilterBundle(List<String> intentBundle, List<String> extraBundle) {
    // do nothing
  }

  /**
   * Called when receve upgrade during incoming call.
   * @param obj Object
   * @return true if need to decline the video upgrade during incoming or waiting call
   */
  @Override
  public boolean declineUpgradeDuringIncoming(Object obj) {
    return true;
  }

  /**
   * Called when answer the incoming call.
   * @param isUpgrade if it is incoming upgrade request
   * @param obj the CallList instance used for plugin
   */
  @Override
  public void onAnswer(boolean isUpgrade, Object obj) {
    // do nothing
  }

  /**
   * Called when details change.
   * @param obj the DialerCall Object
   * @param oldDetails the old call detail
   * @param newDetails the new call detail
   */
  @Override
  public void onCallDetailChanged(Object obj, android.telecom.Call.Details oldDetails,
      android.telecom.Call.Details newDetails) {
    // do nothing
  }
}
