/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License
 */

package com.android.incallui.incall.protocol;

import android.support.annotation.IntDef;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/** Ids for buttons in the in call UI. */
@Retention(RetentionPolicy.SOURCE)
@IntDef({
  InCallButtonIds.BUTTON_AUDIO,
  InCallButtonIds.BUTTON_MUTE,
  InCallButtonIds.BUTTON_DIALPAD,
  InCallButtonIds.BUTTON_HOLD,
  InCallButtonIds.BUTTON_SWAP,
  InCallButtonIds.BUTTON_UPGRADE_TO_VIDEO,
  InCallButtonIds.BUTTON_SWITCH_CAMERA,
  InCallButtonIds.BUTTON_DOWNGRADE_TO_AUDIO,
  InCallButtonIds.BUTTON_ADD_CALL,
  InCallButtonIds.BUTTON_MERGE,
  InCallButtonIds.BUTTON_PAUSE_VIDEO,
  InCallButtonIds.BUTTON_MANAGE_VIDEO_CONFERENCE,
  InCallButtonIds.BUTTON_MANAGE_VOICE_CONFERENCE,
  InCallButtonIds.BUTTON_SWITCH_TO_SECONDARY,
  InCallButtonIds.BUTTON_SWAP_SIM,
  /// M: MediaTek features.
  InCallButtonIds.BUTTON_HIDE_PREVIEW,
  /// M: [Voice Record]
  InCallButtonIds.BUTTON_SWITCH_VOICE_RECORD,
  /// M: [Hang Up All/Hold]
  InCallButtonIds.BUTTON_HANG_UP_ALL,
  InCallButtonIds.BUTTON_HANG_UP_HOLD,
  /// M: [ECT(blind)]
  InCallButtonIds.BUTTON_ECT,
  InCallButtonIds.BUTTON_BLIND_ECT,
  /// M: [One way video]
  InCallButtonIds.BUTTON_ONE_WAY_VIDEO,
  /// M: [Device Switch]
  InCallButtonIds.BUTTON_DEVICE_SWITCH,
  /// M: [Cancel upgrade]
  InCallButtonIds.BUTTON_CANCEL_UPGRADE,
  InCallButtonIds.BUTTON_COUNT,
  InCallButtonIds.BUTTON_UPGRADE_TO_RTT,
  /// M: [RTT]
  InCallButtonIds.BUTTON_DOWNGRADE_RTT
})
public @interface InCallButtonIds {

  int BUTTON_AUDIO = 0;
  int BUTTON_MUTE = 1;
  int BUTTON_DIALPAD = 2;
  int BUTTON_HOLD = 3;
  int BUTTON_SWAP = 4;
  int BUTTON_UPGRADE_TO_VIDEO = 5;
  int BUTTON_SWITCH_CAMERA = 6;
  int BUTTON_DOWNGRADE_TO_AUDIO = 7;
  int BUTTON_ADD_CALL = 8;
  int BUTTON_MERGE = 9;
  int BUTTON_PAUSE_VIDEO = 10;
  int BUTTON_MANAGE_VIDEO_CONFERENCE = 11;
  int BUTTON_MANAGE_VOICE_CONFERENCE = 12;
  int BUTTON_SWITCH_TO_SECONDARY = 13;
  int BUTTON_SWAP_SIM = 14;
  /// M: MediaTek features.
  int BUTTON_HIDE_PREVIEW = 15;
  /// M: [Voice Record]
  int BUTTON_SWITCH_VOICE_RECORD = 16;
  /// M: [Hang Up All/Hold]
  int BUTTON_HANG_UP_ALL = 17;
  int BUTTON_HANG_UP_HOLD = 18;
  /// M: [ECT(blind)]
  int BUTTON_ECT = 19;
  int BUTTON_BLIND_ECT = 20;
  /// M: [One way video]
  int BUTTON_ONE_WAY_VIDEO = 21;
  /// M: [Device Switch]
  int BUTTON_DEVICE_SWITCH = 22;
  /// M: [Cancel upgrade]
  int BUTTON_CANCEL_UPGRADE = 23;
  int BUTTON_COUNT = 27;
  int BUTTON_UPGRADE_TO_RTT = 25;
  /// M: [RTT]
  int BUTTON_DOWNGRADE_RTT = 26;
}
