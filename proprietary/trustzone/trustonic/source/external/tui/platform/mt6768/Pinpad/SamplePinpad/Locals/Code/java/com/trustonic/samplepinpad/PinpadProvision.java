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

import com.trustonic.util.tLog;

import android.app.Activity;
import android.content.Context;

public class PinpadProvision {
    private static final String TAG             = PinpadProvision.class
                                                        .getSimpleName();

    private String              storageLocation = null;
    private Context             appContext      = null;
    private PinpadTUI           pinpadUI   = null;

    public PinpadProvision(Context ctxt, Screen screen) {
        appContext = ctxt;

        pinpadUI = new PinpadTUI(appContext, screen.getType());

        storageLocation = new String(((Activity) appContext).getApplication()
                .getFilesDir().getAbsolutePath()
                + "/");
    }

    public boolean provisionAllImages() {
        boolean result = false;
        for (int i = 0; i < PinpadTUI.IDX_LAYOUT; i++) {
            result = item(i);
            if (!result) {
                tLog.e(TAG, "error to provision image with id = " + i);
                break;
            }
        }
        return result;
    }

    public boolean item(int id) {
        boolean result = false;
        byte[] item = null;
        switch (id) {
        case PinpadTUI.IDX_LAYOUT:
            item = pinpadUI.getRawLayout();
            break;
        default:
            if (id < PinpadTUI.IDX_LAYOUT) {
                item = pinpadUI.getRawImage(id);
            }
            break;
        }
        if (item != null) {
            tLog.d(TAG, "Calling native tlcPinpadProvisionItem");
            result = PinpadTLCWrapper.tlcPinpadProvisionItem(id, storageLocation,
                    item, item.length);
        } else {
            tLog.e(TAG, "Error getting ressource " + id);
        }
        return result;
    }
}