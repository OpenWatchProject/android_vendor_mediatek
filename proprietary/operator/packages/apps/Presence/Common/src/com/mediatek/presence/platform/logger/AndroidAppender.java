/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010 France Telecom S.A.
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
 * limitations under the License.
 ******************************************************************************/

package com.mediatek.presence.platform.logger;

import android.util.Log;

import com.mediatek.presence.utils.logger.Appender;
import com.mediatek.presence.utils.logger.Logger;

/**
 * Android appender 
 * 
 * @author jexa7410
 */
public class AndroidAppender extends Appender {
    /**
     * Constructor
     */
    public AndroidAppender() {
        super();
    }

    /**
     * Print a trace
     *
     * @param logPrefix Classname
     * @param level Trace level
     * @param trace Trace
     */
    public synchronized void printTrace(String logPrefix, int level, String trace) {
        logPrefix = "[PRESENCE][" + logPrefix + "]";
        printLog(logPrefix, level, trace);
    }

    public synchronized void printTrace(int slotId, String logPrefix, int level, String trace) {
        logPrefix = "[PRESENCE][" + logPrefix + "]" + "[slot" + slotId + "]";
        printLog(logPrefix, level, trace);
    }

    private void printLog(String logPrefix, int level, String trace) {
        if (level == Logger.INFO_LEVEL) {
            Log.i(logPrefix, trace);
        } else if (level == Logger.WARN_LEVEL) {
             Log.w(logPrefix, trace);
        } else if (level == Logger.ERROR_LEVEL) {
            Log.e(logPrefix, trace);
        } else if (level == Logger.FATAL_LEVEL) {
            Log.e(logPrefix, trace);
        } else {
            Log.v(logPrefix, trace);
        }
    }
}
