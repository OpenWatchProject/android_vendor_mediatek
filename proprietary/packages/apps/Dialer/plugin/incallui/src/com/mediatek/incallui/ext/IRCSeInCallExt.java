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

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;

public interface IRCSeInCallExt {

  /**
   * called when onCreate(), notify plugin to do initialization.
   * @param icicle the Bundle InCallActivity got
   * @param inCallActivity the InCallActivity instance
   * @param IInCallScreenExt the call back interface for UI updating
   * @internal
   */
  void onCreate(Bundle icicle, Activity inCallActivity, IInCallScreenExt iInCallScreenExt);

  /**
   * called when onNewIntent(), notify plugin activity may reenter.
   * @param intent
   */
  public void onNewIntent(Intent intent);

  /**
   * called when InCallActivity Paused, notify plugin about screen in background.
   * @param inCallActivity the InCallActivity instance
   */
  public void onPause(Activity inCallActivity);

 /**
  * called when InCallActivty resumed, notify plugin about screen comes foreground.
  * @param inCallActivity the InCallActivity instance
  */
  public void onResume(Activity inCallActivity);

  /**
   * called when onDestroy().
   * @param inCallActivity the InCallActivity instance
   * @internal
   */
  void onDestroy(Activity inCallActivity);

  /**
   * This is invoked to plugin when get result from request permissions.
   * @param requestCode Request Code of the permission requested
   * @param permissions List of permissions
   * @param grantResults results of all permissions requested
   */
  public void onRCSeRequestPermissionsResult(
      final int requestCode, final String permissions[], final int[] grantResults);
}
