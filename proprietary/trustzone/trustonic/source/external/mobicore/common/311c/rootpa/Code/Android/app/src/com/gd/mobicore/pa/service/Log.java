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

package com.gd.mobicore.pa.service;

import java.lang.String;
import java.lang.Throwable;

public class Log {

    private static int level_=android.util.Log.INFO;

    /**
    Set level of logging
    */
    public static void setLoggingLevel(int level){
        if( level == android.util.Log.VERBOSE ||
            level == android.util.Log.DEBUG ||
            level == android.util.Log.INFO ||
            level == android.util.Log.WARN ||
            level == android.util.Log.ERROR ||
            level == android.util.Log.ASSERT ){
            level_=level;
        }
    }

    /**
    Send a DEBUG log message.
    */
    public static int d(String tag, String msg){
        if(level_<=android.util.Log.DEBUG) return android.util.Log.d(tag, msg);
        return 0;
    }

    /**
    Send a DEBUG log message and log the exception.
    */
    public static int d(String tag, String msg, Throwable tr){
        if(level_<=android.util.Log.DEBUG) return android.util.Log.d(tag, msg, tr);
        return 0;
    }

    /**
    Send an ERROR log message.
    */
    public static int e(String tag, String msg){
        if(level_<=android.util.Log.ERROR) return android.util.Log.e(tag, msg);
        return 0;
    }

    /**
    Send a ERROR log message and log the exception.
    */
    public static int e(String tag, String msg, Throwable tr){
        if(level_<=android.util.Log.ERROR) return android.util.Log.e(tag, msg, tr);
        return 0;
    }

    /**
    Send an INFO log message.
    */
    public static int i(String tag, String msg){
        if(level_<=android.util.Log.INFO) return android.util.Log.i(tag, msg);
        return 0;
    }

    /**
    Send a INFO log message and log the exception.
    */
    public static int i(String tag, String msg, Throwable tr){
        if(level_<=android.util.Log.INFO) return android.util.Log.i(tag, msg, tr);
        return 0;
    }

    /**
    Handy function to get a loggable stack trace from a Throwable
    */
    public static String getStackTraceString(Throwable tr){
        return android.util.Log.getStackTraceString(tr);
    }

    /**
    Checks to see whether or not a log for the specified tag is loggable at the specified level.
    */
    public static boolean isLoggable(String tag, int level){
        return android.util.Log.isLoggable(tag, level);
    }

    /**
    Low-level logging call.
    */
    public static int println(int priority, String tag, String msg){
        return android.util.Log.println(priority, tag, msg);
    }

    /**
    Send a VERBOSE log message.
    */
    public static int v(String tag, String msg){
        if(level_<=android.util.Log.VERBOSE) return android.util.Log.v(tag, msg);
        return 0;
    }

    /**
    Send a VERBOSE log message and log the exception.
    */
    public static int v(String tag, String msg, Throwable tr){
        if(level_<=android.util.Log.VERBOSE) return android.util.Log.v(tag, msg, tr);
        return 0;
    }

    /**
    Log the exception at WARN level
    */

    public static int w(String tag, Throwable tr){
        if(level_>=android.util.Log.WARN) return android.util.Log.w(tag, tr);
        return 0;
    }

    /**
    Send a WARN log message and log the exception.
    */
    public static int w(String tag, String msg, Throwable tr){
        if(level_<=android.util.Log.WARN) return android.util.Log.w(tag, msg, tr);
        return 0;
    }

    /**
    Send a WARN log message.
    */
    public static int w(String tag, String msg){
        if(level_<=android.util.Log.WARN) return android.util.Log.w(tag, msg);
        return 0;
    }

    /**
    What a Terrible Failure: Report an exception that should never happen.
    */
    public static int wtf(String tag, Throwable tr){
        return android.util.Log.wtf(tag, tr);
    }

    /**
    What a Terrible Failure: Report a condition that should never happen.
    */
    public static int wtf(String tag, String msg){
        return android.util.Log.wtf(tag, msg);
    }

    /**
    What a Terrible Failure: Report an exception that should never happen.
    */
    public static int wtf(String tag, String msg, Throwable tr){
        return android.util.Log.wtf(tag, msg, tr);
    }
}