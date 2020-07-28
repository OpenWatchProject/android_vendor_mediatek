/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.trustonic.util;

import android.util.Log;

public class tLog {
	/* Enable log with the command:
	 * adb shell setprop log.tag.YOUR_APP_TAG LEVEL
	 * e.g adb shell setprop log.tag.TuiActivity DEBUG
	 * */
    public static void d(String tag, String msg) {
        if (Log.isLoggable(tag, Log.DEBUG)) {
            Log.d(tag, msg);
        }
    }

    public static void i(String tag, String msg) {
        if (Log.isLoggable(tag, Log.INFO)) {
            Log.i(tag, msg);
        }
    }

    public static void e(String tag, String msg) {
        if (Log.isLoggable(tag, Log.ERROR)) {
            Log.e(tag, msg);
        }
    }

    public static void v(String tag, String msg) {
        if (Log.isLoggable(tag, Log.VERBOSE)) {
            Log.v(tag, msg);
        }
    }

    public static void w(String tag, String msg) {
        if (Log.isLoggable(tag, Log.WARN)) {
            Log.w(tag, msg);
        }
    }
}

