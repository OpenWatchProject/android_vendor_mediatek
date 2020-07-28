/*
 * ====================================================================
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * ====================================================================
 *
 * This software consists of voluntary contributions made by many
 * individuals on behalf of the Apache Software Foundation.  For more
 * information on the Apache Software Foundation, please see
 * <http://www.apache.org/>.
 *
 */

package com.mediatek.powerhalservice;

import java.util.regex.Pattern;
import android.util.Log;
import android.os.SystemProperties;

/**
 * A collection of utilities relating to InetAddresses.
 *
 * @since 4.0
 */
public class PowerHalAddressUtils {

    private static final String TAG = "PowerHalAddressUitls";

    private static final int IP_FORMAT_UNKONWN = 0;
    private static final int IP_FORMAT_IPV4    = 1;
    private static final int IP_FORMAT_IPV6    = 2;

    private PowerHalAddressUtils() {
    }

    private static final Pattern IPV4_PATTERN = Pattern
        .compile("^(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\."+
        "(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}$");

    private static final Pattern IPV6_STD_PATTERN = Pattern
        .compile("^(?:[0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}$");

    private static final Pattern IPV6_HEX_COMPRESSED_PATTERN = Pattern
        .compile("^((?:[0-9A-Fa-f]{1,4}(?::[0-9A-Fa-f]{1,4})*)?)::"+
        "((?:[0-9A-Fa-f]{1,4}(?::[0-9A-Fa-f]{1,4})*)?)$");

    private static final Pattern IPV6_IPV4_COMPATIBLE_PATTERN = Pattern
        .compile("^::[fF]{4}:(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)(\\."+
        "(25[0-5]|2[0-4]\\d|[0-1]?\\d?\\d)){3}$");

    public static boolean isIPv4Address(final String input) {
        return IPV4_PATTERN.matcher(input).matches();
    }

    public static boolean isIPv4SourceAddress(final String input) {
        return (input.equals("0.0.0.0/0") == true ||
            (isIPv4Address(input) && input.equals("0.0.0.0") == false));
    }

    public static boolean isIPv4MulticastAddress(final String input) {
        String[] items = input.split("\\.");

        if (items.length > 0) {
            int ipStartValue = Integer.parseInt(items[0]);

            /* 224.0.0.0 ~ 239.255.255.255 */
            if (ipStartValue >= 224 && ipStartValue <= 239) {
                return true;
            }
        }
        return false;
    }

    public static boolean isIPv6StdAddress(final String input) {
        return IPV6_STD_PATTERN.matcher(input).matches();
    }

    public static boolean isIPv6HexCompressedAddress(final String input) {
        return IPV6_HEX_COMPRESSED_PATTERN.matcher(input).matches();
    }

    public static boolean isIPv6IPv4CompatibleAddress(final String input) {
        return IPV6_IPV4_COMPATIBLE_PATTERN.matcher(input).matches();
    }

    public static boolean isIPv6Address(final String input) {
        return (isIPv6StdAddress(input) || isIPv6HexCompressedAddress(input) ||
                isIPv6IPv4CompatibleAddress(input));
    }

    public static boolean isIPv6SourceAddress(final String input) {
        return (input.equals("::/0") == true || isIPv6Address(input));
    }

    public static boolean isIpPairValid(final String src_ip,
        final String dst_ip, int src_port, int dst_port) {
        int src_format = 0, dst_format = 0;

        /* port */
        /* src_port -1 means not assigned */
        if (src_port < -1 || src_port > 65535 || dst_port < -1 || dst_port > 65535) {
            logd("invalid port:" + src_port + "," + dst_port);
            return false; /* unknown format */
        }

        if (isIPv4SourceAddress(src_ip)) {
            src_format = IP_FORMAT_IPV4;
        } else if(isIPv6SourceAddress(src_ip)) {
            src_format = IP_FORMAT_IPV6;
        } else {
            logd("src unknown:" + src_ip);
            return false; /* unknown format */
        }

        if (isIPv4Address(dst_ip)) {
            dst_format = IP_FORMAT_IPV4;
        } else if(isIPv6Address(dst_ip)) {
            dst_format = IP_FORMAT_IPV6;
        } else {
            logd("dst unknown:" + dst_ip);
            return false; /* unknown format */
        }

        /* source and destination are not matched */
        if (src_format != dst_format) {
            logd("not match:" + src_ip + "," + dst_ip);
            return false;
        }

        /* destination check */
        if (dst_format == IP_FORMAT_IPV4) {

            /* loopback address */
            if (dst_ip.startsWith("127")) {
                logd("violate: loopback address:" + dst_ip);
                return false;
            }

            /* broadcast */
            if (dst_ip.equals("255.255.255.255") == true) {
                logd("violate: broadcast:" + dst_ip);
                return false;
            }

            /* multicasting */
            if (isIPv4MulticastAddress(dst_ip)) {
                logd("violate: multicasting:" + dst_ip);
                return false;
            }
        }

        return true;
    }

    private static void log(String info) {
        Log.i(TAG, info + " ");
    }

    private static void logd(String info) {
        Log.d(TAG, info + " ");
    }

    private static void loge(String info) {
        Log.e(TAG, "ERR: " + info + " ");
    }
}

