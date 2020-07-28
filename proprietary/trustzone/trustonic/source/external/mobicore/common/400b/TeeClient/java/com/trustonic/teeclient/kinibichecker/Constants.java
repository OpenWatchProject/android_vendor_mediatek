/*
 * Copyright (c) 2016-2018 TRUSTONIC LIMITED
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
package com.trustonic.teeclient.kinibichecker;

/**
 * Class to store constants
 *
 */
public class Constants {

    public static final String MC_SYSTEM_REGISTRY_PATH = "/system/app/mcRegistry";
    public static final String MC_REGISTRY_PATH = "/data/app/mcRegistry";  //Normal
    public static final String MC_REGISTRY_PATH_2 = "/data/misc/mcRegistry"; //for Android N
    public static final String MC_REGISTRY_PATH_3 = "/vendor/app/mcRegistry"; //for MediaTek

    public static final String MC_SOCKETS_MOBICORE_PATH = "/dev/mobicore";
    public static final String MC_SOCKETS_MOBICORE_USER_PATH = "/dev/mobicore-user";
    public static final String ROOTPA_COMMONWRAPPER = "/system/lib/libcommonpawrapper.so";

    public static final String ROOTPA_CURL = "/system/lib/libcurl.so";
    public static final String ROOTPA_CURL_64 = "/system/lib64/libcurl.so";

    public static final String TUI_DRIVER_PATH = "/dev/t-base-tui";

    public static final String TUI_SERVICE_PKG_NAME = "com.trustonic.tuiservice";
    public static final String TDP_SERVICE_PKG_NAME ="com.trustonic.teeproxyservice";

}
