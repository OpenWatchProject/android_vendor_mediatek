/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

package com.trustonic.samplepinpad;

public class PinpadTLCWrapper {

    public static int       screenWidth;
    public static int       screenHeight;
    public static byte[]    item;
    public static int       itemLen;

    public static final int IDX_PINPAD_PINPAD = 0;
    public static final int IDX_PINPAD_STAR   = 1;
    public static final int IDX_PINPAD_BLANK  = 2;
    public static final int IDX_PINPAD_LAYOUT = 3;
    public static final int IDX_PINPAD_PIN    = 4;
    public static final int IDX_PINPAD_END    = 5;

    /* Native functions */
    public static native boolean tlcPinpadConnect();

    public static native void tlcPinpadDisconnect();

    public static native boolean tlcPinpadGetResolution();

    public static native boolean tlcPinpadProvisionItem(int itemID,
            String path, byte[] item, int itemLen);

    public static native String tlcPinpadVerify();

    public static native byte[] tlcPinpadGetNativeLayout();
    public static native byte[] tlcPinpadGetNativeLayoutHD();
    public static native byte[] tlcPinpadGetNativeLayoutQuartHD();
    public static native byte[] tlcPinpadGetNativeLayoutQHD();

    /**
     * this is used to load the library on application startup. The library has
     * already been unpacked to the app specific folder at installation time by
     * the package manager.
     */
    static {
        System.loadLibrary("TlcPinpad");
    }
}
