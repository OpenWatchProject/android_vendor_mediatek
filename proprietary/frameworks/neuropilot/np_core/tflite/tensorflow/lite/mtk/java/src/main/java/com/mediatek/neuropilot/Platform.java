/* MediaTek Inc. (C) 2018. All rights reserved.
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

package com.mediatek.neuropilot;

import android.os.Build;
import android.util.Log;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public final class Platform {
  private static final String LIBNAME = "neuropilot_jni";

  public static int INFERENCE_TYPE_NONE = 0;
  public static int INFERENCE_TYPE_QUANT = 1;
  public static int INFERENCE_TYPE_FLOAT = 2;

  private static final String TAG = "Platform";
  private static final String PROC_CPU_INFO_PATH = "/proc/cpuinfo";

  public static int getPreferredInference() {
    if (!isMtkPlatform()) {
      Log.e(TAG, "Not MTK platform");
      return INFERENCE_TYPE_NONE;
    }
    Log.d(TAG, "Build.VERSION.SDK_INT:" + Build.VERSION.SDK_INT);
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.O_MR1) {
      return INFERENCE_TYPE_NONE;
    }
    if (Build.VERSION.SDK_INT < Build.VERSION_CODES.P) {
      return INFERENCE_TYPE_QUANT;
    }
    try {
      Log.d(TAG, "Load:" + LIBNAME);
      System.loadLibrary(LIBNAME);
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Platform: failed to load native library: " +
              e.getMessage());
    }
    return getInferencePreference();
  }

  private static String getCpuName() {
    String str = null;

    try {
      FileReader fr = new FileReader("/proc/cpuinfo");
      BufferedReader br = new BufferedReader(fr);

      while ((str = br.readLine()) != null) {
        if (str.contains("Hardware")) {
          str =  str.split(":")[1];
          break;
        }
      }

      br.close();
    } catch (IOException e) {
      e.printStackTrace();
    }

    return str;
  }

  private static boolean isMtkPlatform() {
    String cpuName = getCpuName();
    if (cpuName == null) {
      return false;
    }
    Log.d(TAG, "CPU name:" + cpuName);
    Pattern p = Pattern.compile("mt[0-9][0-9][0-9][0-9]*|m[0-9][0-9][0-9][0-9]*");
    Matcher m = p.matcher(cpuName.toLowerCase());
    return m.find();
  }

  private static native int getInferencePreference();

  /**
   * Load the TensorFlowLite runtime C library.
   */
  static boolean init() {
    try {
      System.loadLibrary(LIBNAME);
      return true;
    } catch (UnsatisfiedLinkError e) {
      System.err.println("Platform: failed to load native library: " +
                         e.getMessage());
      return false;
    }
  }
/*
  static {
    init();
  }
*/
}
