/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2011. All rights reserved.
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

package com.mediatek.incallui;

import android.os.Handler;
import android.os.SystemClock;

/**
 * Helper class used to keep track of events requiring regular intervals.
 */
public class CallTimer extends Handler {
  private Runnable internalCallback;
  private Runnable callback;
  private long lastReportedTime;
  private long interval;
  private boolean running;

  public CallTimer(Runnable runnable) {
    interval = 0;
    lastReportedTime = 0;
    running = false;
    callback = runnable;
    internalCallback = new CallTimerCallback();
  }

  public boolean start(long time) {
    if (time <= 0) {
      return false;
    }
    // cancel any previous timer
    cancel();
    interval = time;
    lastReportedTime = SystemClock.uptimeMillis();

    running = true;
    periodicUpdateTimer();
    return true;
  }

  public void cancel() {
    removeCallbacks(internalCallback);
    running = false;
  }

  private void periodicUpdateTimer() {
    if (!running) {
      return;
    }
    if (callback == null) {
      return;
    }

    final long now = SystemClock.uptimeMillis();
    long nextReport = lastReportedTime + interval;
    while (now >= nextReport) {
      nextReport += interval;
    }

    postAtTime(internalCallback, nextReport);
    lastReportedTime = nextReport;

    // Run the callback
    callback.run();
  }

  private class CallTimerCallback implements Runnable {
    @Override
    public void run() {
      periodicUpdateTimer();
    }
  }
}
