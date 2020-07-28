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

import android.content.Context;
import android.graphics.drawable.RippleDrawable;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import java.util.HashMap;


public interface IRCSeCallButtonExt {

  /**
   * called when CallButtonFragment view created.
   * customize this view
   * @param context host Context
   * @param rootView the CallButtonFragment view
   * @internal
   */
  void onViewCreated(Context context, View rootView);

  /**
   * called when call state changed
   * notify the foreground call to plug-in.
   * @param call current foreground call
   * @param callMap a mapping of callId -> call for all current calls
   * @internal
   */
  void onStateChange(android.telecom.Call call, HashMap<String, android.telecom.Call> callMap);

  /**
   * called when popup menu item in CallButtonFragment clicked.
   * involved popup menus such as audio mode, vt
   * @param menuItem the clicked menu item
   * @return true if this menu event has already handled by plugin
   * @internal
   */
  boolean handleMenuItemClick(MenuItem menuItem);

  /**
   * TODO: [M migration]should find somewhere to add it back.
   * called when configure overflow menu item in CallButtonFragment.
   * @param context the Activity context
   * @param menu the Activity overflow menu
   */
  void configureOverflowMenu(Context context, Menu menu);

  /**
   * called when update backgroung drawable of callbutton in callbutton fragment.
   * Plugin should sync the color of message button with host button color
   * @param rpDrawable the background drawable need to changed to
   */
  void updateNormalBgDrawable(RippleDrawable rpDrawable);
}
