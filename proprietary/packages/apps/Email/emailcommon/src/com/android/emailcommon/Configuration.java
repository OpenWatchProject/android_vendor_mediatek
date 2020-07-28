/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
/*
 * Copyright (C) 2011 The Android Open Source Project
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
 */

package com.android.emailcommon;

public class Configuration {
    // Bundle key for Exchange configuration (boolean value)
    public static final String EXCHANGE_CONFIGURATION_USE_ALTERNATE_STRINGS =
        "com.android.email.EXCHANGE_CONFIGURATION_USE_ALTERNATE_STRINGS";
    /** M: Add for MTK email. @{ */
    // The default port for pop3/imap/smtp/exchange
    public static final int IMAP_DEFAULT_PORT = 143;
    public static final int POP3_DEFAULT_PORT = 110;
    public static final int SMTP_DEFAULT_PORT = 25;
    public static final int EAS_DEFAULT_PORT = 80;

    public static final int IMAP_DEFAULT_SSL_PORT = 993;
    public static final int POP3_DEFAULT_SSL_PORT = 995;
    public static final int SMTP_DEFAULT_SSL_PORT = 465;
    public static final int EAS_DEFAULT_SSL_PORT = 443;

    // M: max view/edit quoted text length, it should depend on platform.
    private static final int CPU_COUNT = Runtime.getRuntime().availableProcessors();
    public static final int MAX_VIEW_QUOTETEXT_LENGTH = (CPU_COUNT / 2 + 1) * 10 * 1024;
    public static final int MAX_EDIT_QUOTETEXT_LENGTH = (CPU_COUNT / 4 + 1) * 5 * 1024;

    /// M: Some constants to show the email's limitation. @{ */
    //Common max input text length, e.g. 'Signature','Quick Response'
    public static final int COMMON_EDITVIEW_MAX_LENGTH = 1000;
    //constants used to limit add recipients at one time.
    public static final int RECIPIENT_MAX_NUMBER = 250;
    // Common max input text length, e.g. 'Your Name','Account Name',
    // 'Subject','User name','Password'
    public static final int EDITVIEW_MAX_LENGTH_1 = 256;
    //Common max input text length, e.g. 'Email Content'
    public static final int EDITVIEW_MAX_LENGTH_2 = 15000;

    // The switch for test @{
    private static boolean sIsTest = false;

    public static void openTest() {
        sIsTest = true;
    }

    public static void shutDownTest() {
        sIsTest = false;
    }

    public static boolean isTest() {
        return sIsTest;
    }
    /** @} */
}
