/**
 * Copyright (c) 2012, Google Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.android.mail.utils;

public class LogTag {
    private static String sLogTag = "UnifiedEmail";

    /// M: add some feature tag. @{
    /// 1. Tag for localsearch and remote search.
    public static final String SEARCH_TAG = "SearchMail";
    /// 2. Tag for send mail.
    public static final String SENDMAIL_TAG = "SendMail";
    /// 3. Tag for receive mail.
    public static final String RECEIVEMAIL_TAG = "ReceiveMail";
    public static final String ASYNCTASK_TAG = "BackgroundAsyncTask";
    /// @}

    /**
     * Get the log tag to apply to logging.
     */
    public static String getLogTag() {
        return sLogTag;
    }

    /**
     * Sets the app-wide log tag to be used in most log messages, and for enabling logging
     * verbosity. This should be called at most once, during app start-up.
     */
    public static void setLogTag(final String logTag) {
        sLogTag = logTag;
    }
}
