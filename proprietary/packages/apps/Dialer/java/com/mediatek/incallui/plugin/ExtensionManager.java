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

package com.mediatek.incallui.plugin;

import android.annotation.NonNull;
import android.content.Context;

import com.android.dialer.common.Assert;
import com.android.incallui.Log;

import com.mediatek.incallui.ext.IAnswerExt;
import com.mediatek.incallui.ext.ICallCardExt;
import com.mediatek.incallui.ext.IInCallButtonExt;
import com.mediatek.incallui.ext.IInCallExt;
import com.mediatek.incallui.ext.IRCSeCallButtonExt;
import com.mediatek.incallui.ext.IRCSeCallCardExt;
import com.mediatek.incallui.ext.IRCSeInCallExt;
import com.mediatek.incallui.ext.IStatusBarExt;
import com.mediatek.incallui.ext.IVideoCallExt;
import com.mediatek.incallui.ext.IVilteAutoTestHelperExt;
import com.mediatek.incallui.ext.OpInCallUICustomizationFactoryBase;

public final class ExtensionManager {
  private static Context sApplicationContext;

  private ExtensionManager() {
  }

  public static void registerApplicationContext(@NonNull Context context) {
    if (sApplicationContext == null) {
      sApplicationContext = context;
    }
  }

  private static volatile IRCSeCallButtonExt sRCSeCallButtonExt;
  private static volatile IRCSeCallCardExt sRCSeCallCardExt;
  private static volatile IRCSeInCallExt sRCSeInCallExt;
  private static volatile IInCallExt sInCallExt;
  private static volatile IAnswerExt sAnswerExt;
  private static volatile ICallCardExt sCallCardExt;
  private static volatile IVideoCallExt sVideoCallExt;
  private static volatile IInCallButtonExt sInCallButtonExt;
  private static volatile IStatusBarExt sStatusBarExt;
  private static volatile IVilteAutoTestHelperExt sVilteAutoTestHelperExt;

  /**
   * retrieve the instance of IRCSeCallButtonExt.
   * @return the instance of IRCSeCallButtonExt.
   */
  public static IRCSeCallButtonExt getRCSeCallButtonExt() {
    if (sRCSeCallButtonExt == null) {
      synchronized (IRCSeCallButtonExt.class) {
        if (sRCSeCallButtonExt == null) {
          Assert.checkState(sApplicationContext != null);
          sRCSeCallButtonExt = OpInCallUICustomizationFactoryBase
              .getRcsFactory(sApplicationContext).getRCSeCallButtonExt(sApplicationContext);
        }
      }
    }
    return sRCSeCallButtonExt;
  }

  /**
   * retrieve the instance of IRCSeCallCardExt.
   * @return the instance of IRCSeCallCardExt.
   */
  public static IRCSeCallCardExt getRCSeCallCardExt() {
    if (sRCSeCallCardExt == null) {
      synchronized (IRCSeCallCardExt.class) {
        if (sRCSeCallCardExt == null) {
          Assert.checkState(sApplicationContext != null);
          sRCSeCallCardExt = OpInCallUICustomizationFactoryBase
              .getRcsFactory(sApplicationContext).getRCSeCallCardExt(sApplicationContext);
        }
      }
    }
    return sRCSeCallCardExt;
  }

  /**
   * retrieve the instance of IRCSeInCallExt.
   * @return the instance of IRCSeInCallExt.
   */
  public static IRCSeInCallExt getRCSeInCallExt() {
    if (sRCSeInCallExt == null) {
      synchronized (IRCSeInCallExt.class) {
        if (sRCSeInCallExt == null) {
          Assert.checkState(sApplicationContext != null);
          sRCSeInCallExt = OpInCallUICustomizationFactoryBase
              .getRcsFactory(sApplicationContext).getRCSeInCallExt(sApplicationContext);
        }
      }
    }
    return sRCSeInCallExt;
  }

  /**
   * retrieve the instance of IICallExt.
   * @return the instance of IICallExt.
   */
  public static IInCallExt getInCallExt() {
    if (sInCallExt == null) {
      synchronized (IInCallExt.class) {
        if (sInCallExt == null) {
          Assert.checkState(sApplicationContext != null);
          sInCallExt = OpInCallUICustomizationFactoryBase
              .getOpFactory(sApplicationContext).getInCallExt();
        }
      }
    }
    return sInCallExt;
  }

  /**
   * retrieve the instance of IAnswerExt.
   * @return the instance of IAnswerExt.
   */
  public static IAnswerExt getAnswerExt() {
    if (sAnswerExt == null) {
      synchronized (IAnswerExt.class) {
        if (sAnswerExt == null) {
          Assert.checkState(sApplicationContext != null);
          sAnswerExt = OpInCallUICustomizationFactoryBase
              .getOpFactory(sApplicationContext).getAnswerExt();
        }
      }
    }
    return sAnswerExt;
  }

  /**
   * retrieve the instance of ICallCardExt.
   * @return the instance of ICallCardExt.
   */
  public static ICallCardExt getCallCardExt() {
    if (sCallCardExt == null) {
      synchronized (ICallCardExt.class) {
        if (sCallCardExt == null) {
          Assert.checkState(sApplicationContext != null);
          sCallCardExt = OpInCallUICustomizationFactoryBase
              .getOpFactory(sApplicationContext).getCallCardExt();
        }
      }
    }
    return sCallCardExt;
  }

  /**
   * retrieve the instance of IVideoCallExt.
   * @return the instance of IVideoCallExt.
   */
  public static IVideoCallExt getVideoCallExt() {
    if (sVideoCallExt == null) {
      synchronized (IVideoCallExt.class) {
        if (sVideoCallExt == null) {
          Assert.checkState(sApplicationContext != null);
          sVideoCallExt = OpInCallUICustomizationFactoryBase
              .getOpFactory(sApplicationContext).getVideoCallExt();
        }
      }
    }
    return sVideoCallExt;
  }

  /**
   * retrieve the instance of IInCallButtonExt.
   * @return the instance of IInCallButtonExt.
   */
  public static IInCallButtonExt getInCallButtonExt() {
    if (sInCallButtonExt == null) {
      synchronized (IInCallButtonExt.class) {
        if (sInCallButtonExt == null) {
          Assert.checkState(sApplicationContext != null);
          sInCallButtonExt = OpInCallUICustomizationFactoryBase
              .getOpFactory(sApplicationContext).getInCallButtonExt();
        }
      }
    }
    return sInCallButtonExt;
  }

  /**
   * retrieve the instance of IStatusBarExt.
   * @return the instance of IStatusBarExt.
   */
  public static IStatusBarExt getStatusBarExt() {
    if (sStatusBarExt == null) {
      synchronized (IStatusBarExt.class) {
        if (sStatusBarExt == null) {
          Assert.checkState(sApplicationContext != null);
          sStatusBarExt = OpInCallUICustomizationFactoryBase
              .getOpFactory(sApplicationContext).getStatusBarExt();
        }
      }
    }
    return sStatusBarExt;
  }

  /**
   * retrieve the instance of IVilteAutoTestHelperExt.
   * @return the instance of IVilteAutoTestHelperExt.
   */
  public static IVilteAutoTestHelperExt getVilteAutoTestHelperExt() {
    if (sVilteAutoTestHelperExt == null) {
      synchronized (IVilteAutoTestHelperExt.class) {
        if (sVilteAutoTestHelperExt == null) {
          Assert.checkState(sApplicationContext != null);
          sVilteAutoTestHelperExt = OpInCallUICustomizationFactoryBase
              .getOpFactory(sApplicationContext).getVilteAutoTestHelperExt();
        }
      }
    }
    return sVilteAutoTestHelperExt;
  }
}
