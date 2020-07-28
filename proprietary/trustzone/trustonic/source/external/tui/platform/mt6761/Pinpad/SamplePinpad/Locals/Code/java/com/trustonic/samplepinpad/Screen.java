/*
 * Copyright (c) 2013-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
package com.trustonic.samplepinpad;

import com.trustonic.util.tLog;

public class Screen {
    private static final String TAG             = PinpadProvision.class
            .getSimpleName();

    /** Type of resolution **/
    protected static final int LOW_RES_WIDTH         = 300;
    protected static final int MEDIUM_RES_WIDTH      = 600;
    protected static final int MIDHIGH_RES_WIDTH     = 720;
    protected static final int HIGH_RES_WIDTH        = 1080;
    protected static final int EXTRA_HIGH_RES_WIDTH  = 1400;

    protected enum Type {
        UNSUPPORTED,
        LOW,
        MEDIUM,
        MIDHIGH,
        HIGH,
        EXTRA_HIGH
    }

    private Type type;

    Screen() {
        int width  = PinpadTLCWrapper.screenHeight;
        int height = PinpadTLCWrapper.screenWidth;
        int smallestDim = width < height ? width : height;
        type = Type.UNSUPPORTED;

        // Define what kind of resolution are we facing.
        if (smallestDim < MEDIUM_RES_WIDTH) {
            type = Type.LOW;
        } else if ((smallestDim >= MEDIUM_RES_WIDTH) && (smallestDim < MIDHIGH_RES_WIDTH)) {
            type = Type.MEDIUM;
        } else if ((smallestDim >= MIDHIGH_RES_WIDTH) && (smallestDim < HIGH_RES_WIDTH)) {
            type = Type.MIDHIGH;
        } else if ((smallestDim >= HIGH_RES_WIDTH) && (smallestDim < EXTRA_HIGH_RES_WIDTH)) {
            type = Type.HIGH;
        } else if (smallestDim > EXTRA_HIGH_RES_WIDTH) {
            type = Type.EXTRA_HIGH;
        } else {
            tLog.e(TAG, "Error wrong resolution!");
            type = Type.UNSUPPORTED;
        }
    }

    public Type getType() {
        return type;
    }

    public boolean IsSupported () {
        return type != Type.UNSUPPORTED;
    }
}
