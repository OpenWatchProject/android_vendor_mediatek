/*******************************************************************************
 * Software Name : RCS IMS Stack
 *
 * Copyright (C) 2010-2016 Orange.
 * Copyright (C) 2015 Sony Mobile Communications Inc.
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
 *
 * NOTE: This file has been modified by Sony Mobile Communications Inc.
 * Modifications are licensed under the License.
 ******************************************************************************/

package com.mediatek.rcs.utils;

import android.content.Context;
import android.content.pm.PackageManager.NameNotFoundException;
import android.os.Build;

import com.mediatek.rcs.utils.logger.Logger;

/**
 * Terminal information
 *
 * @author JM. Auffret
 * @author Deutsche Telekom AG
 */
public class TerminalInfo {

    //private static final Logger sLogger = Logger.getLogger(TerminalInfo.class.getName());

    /**
     * Product name
     */
    private static final String productName = "RCS-client";

    /**
     * RCS stack package name
     */
    public final static String RCS_STACK_PACKAGENAME = "com.orangelabs.rcs";

    /**
     * Product version
     */
    private static String sProductVersion;

    /**
     * RCS client version. Client Version Value = Platform "-" VersionMajor "." VersionMinor
     * Platform = Alphanumeric (max 9) VersionMajor = Number (2 char max) VersionMinor = Number (2
     * char max)
     */
    private static final String CLIENT_VERSION_PREFIX = "RCSAndr-";

    private static final String UNKNOWN = "unknown";

    private static final char FORWARD_SLASH = '/';

    private static final char HYPHEN = '-';

    private static String sClientVersion;

    private static String sBuildInfo;

    private static String sClientInfo;

    /**
     * Returns the product name
     *
     * @return Name
     */
    public static String getProductName() {
        return productName;
    }

    /**
     * Returns the product version
     *
     * @return Version
     */
    public static String getProductVersion(Context ctx) {
        if (sProductVersion == null) {
            try {
                sProductVersion = ctx.getPackageManager().getPackageInfo(
                        RCS_STACK_PACKAGENAME, 0).versionName;

            } catch (NameNotFoundException e) {
                // if (sLogger.isActivated()) {
                //     sLogger.error("Version Name not defined in Manifest", e);
                // }
                sProductVersion = UNKNOWN;
            }
        }
        return sProductVersion;
    }

    /**
     * Returns the product name + version
     *
     * @return product information
     */
    public static String getProductInfo() {
        return productName + FORWARD_SLASH + sProductVersion;
    }

    /**
     * Returns the client version as mentioned under versionName in AndroidManifest, prefixed with
     * CLIENT_VERSION_PREFIX.
     * <p>
     * In case versionName is not found under AndroidManifest it will default to UNKNOWN.
     * </p>
     *
     * @param ctx the context
     * @return Client version
     */
    public static String getClientVersion(Context ctx) {
        if (sClientVersion == null) {
            sClientVersion = CLIENT_VERSION_PREFIX + getProductVersion(ctx);
        }
        return sClientVersion.substring(0, Math.min(sClientVersion.length(), 15));
    }

    /**
     * Returns the client vendor
     *
     * @return Build.MANUFACTURER
     */
    public static String getClientVendor() {
        String vendor = (Build.MANUFACTURER != null) ? Build.MANUFACTURER : UNKNOWN;
        return vendor.substring(0, Math.min(vendor.length(), 4));
    }

    /**
     * Returns the terminal vendor
     *
     * @return Build.MANUFACTURER
     */
    public static String getTerminalVendor() {
        String vendor = (Build.MANUFACTURER != null) ? Build.MANUFACTURER : UNKNOWN;
        return vendor.substring(0, Math.min(vendor.length(), 4));
    }

    /**
     * Returns the terminal model
     *
     * @return Build.DEVICE
     */
    public static String getTerminalModel() {
        String model = (Build.DEVICE != null) ? Build.DEVICE : UNKNOWN;
        return model.substring(0, Math.min(model.length(), 10));
    }

    /**
     * Returns the terminal software version
     *
     * @return Build.DISPLAY
     */
    public static String getTerminalSoftwareVersion() {
        String swVersion = (Build.DISPLAY != null) ? Build.DISPLAY : UNKNOWN;
        return swVersion.substring(0, Math.min(swVersion.length(), 10));
    }

    /**
     * Get the build info
     *
     * @return build info
     */
    public static String getBuildInfo() {
        if (sBuildInfo == null) {
            final String buildVersion = getTerminalModel() + HYPHEN + getTerminalSoftwareVersion();
            sBuildInfo = getTerminalVendor() + FORWARD_SLASH + buildVersion;
        }
        return sBuildInfo;
    }

    /**
     * Returns the client_vendor '/' client_version
     *
     * @return client information
     */
    public static String getClientInfo(Context ctx) {
        if (sClientInfo == null) {
            sClientInfo = getClientVendor() + FORWARD_SLASH
                    + getClientVersion(ctx);
        }
        return sClientInfo;
    }
}
