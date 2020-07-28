/*
 * Copyright (c) 2013-2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

package com.trustonic.samplepinpad;

import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;

import com.trustonic.samplepinpad.Screen.Type;
import com.trustonic.util.tLog;

import android.content.Context;

public class PinpadTUI {
    private static final String TAG            = PinpadTUI.class
                                                       .getSimpleName();

    public static final int     IDX_PINPAD          = 0;
    public static final int     IDX_STAR            = 1;
    public static final int     IDX_BLANK           = 2;
    public static final int     IDX_BUT0            = 3;
    public static final int     IDX_BUT1            = 4;
    public static final int     IDX_BUT2            = 5;
    public static final int     IDX_BUT3            = 6;
    public static final int     IDX_BUT4            = 7;
    public static final int     IDX_BUT5            = 8;
    public static final int     IDX_BUT6            = 9;
    public static final int     IDX_BUT7            = 10;
    public static final int     IDX_BUT8            = 11;
    public static final int     IDX_BUT9            = 12;
    public static final int     IDX_CORRECT         = 13;
    public static final int     IDX_CANCEL          = 14;
    public static final int     IDX_VALID           = 15;
    public static final int     IDX_BUT0_PRESSED    = 16;
    public static final int     IDX_BUT1_PRESSED    = 17;
    public static final int     IDX_BUT2_PRESSED    = 18;
    public static final int     IDX_BUT3_PRESSED    = 19;
    public static final int     IDX_BUT4_PRESSED    = 20;
    public static final int     IDX_BUT5_PRESSED    = 21;
    public static final int     IDX_BUT6_PRESSED    = 21;
    public static final int     IDX_BUT7_PRESSED    = 22;
    public static final int     IDX_BUT8_PRESSED    = 23;
    public static final int     IDX_BUT9_PRESSED    = 24;
    public static final int     IDX_CORRECT_PRESSED = 26;
    public static final int     IDX_CANCEL_PRESSED  = 25;
    public static final int     IDX_VALID_PRESSED   = 28;
    public static final int     IDX_LAYOUT          = 29;

    private Context             appContext     = null;
    private ArrayList<byte[]>   listOfImage    = null;

    private byte[]              layout         = null;

    public static String byteToHexString(byte value) {
        StringBuffer hexString = new StringBuffer();
        int intVal = value & 0xff;
        if (intVal < 0x10)
            hexString.append("0");
        hexString.append(Integer.toHexString(intVal));
        return hexString.toString();
    }

    PinpadTUI(Context ctxt, Type resType) {
        appContext = ctxt;

        /* Retrieve picture raw data from resources */
        listOfImage = new ArrayList<byte[]>();

        switch (resType) {
        case LOW:
            tLog.d(TAG, "Low resolution screen.");
            listOfImage.add(getRawRessource(R.raw.pinpad_quarthd));
            listOfImage.add(getRawRessource(R.raw.star_quarthd));
            listOfImage.add(getRawRessource(R.raw.blank_quarthd));
            listOfImage.add(getRawRessource(R.raw.button0_quarthd));
            listOfImage.add(getRawRessource(R.raw.button1_quarthd));
            listOfImage.add(getRawRessource(R.raw.button2_quarthd));
            listOfImage.add(getRawRessource(R.raw.button3_quarthd));
            listOfImage.add(getRawRessource(R.raw.button4_quarthd));
            listOfImage.add(getRawRessource(R.raw.button5_quarthd));
            listOfImage.add(getRawRessource(R.raw.button6_quarthd));
            listOfImage.add(getRawRessource(R.raw.button7_quarthd));
            listOfImage.add(getRawRessource(R.raw.button8_quarthd));
            listOfImage.add(getRawRessource(R.raw.button9_quarthd));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection_quarthd));
            listOfImage.add(getRawRessource(R.raw.buttoncancel_quarthd));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate_quarthd));
            listOfImage.add(getRawRessource(R.raw.button0_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button1_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button2_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button3_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button4_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button5_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button6_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button7_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button8_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.button9_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.buttoncancel_pressed_quarthd));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate_pressed_quarthd));

            /* Retrieve layout raw data from TLC */
            try {
                layout = PinpadTLCWrapper.tlcPinpadGetNativeLayoutQuartHD();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            break;

        case MEDIUM:
            tLog.d(TAG, "Medium resolution screen.");
            listOfImage.add(getRawRessource(R.raw.pinpad));
            listOfImage.add(getRawRessource(R.raw.star));
            listOfImage.add(getRawRessource(R.raw.blank));
            listOfImage.add(getRawRessource(R.raw.button0));
            listOfImage.add(getRawRessource(R.raw.button1));
            listOfImage.add(getRawRessource(R.raw.button2));
            listOfImage.add(getRawRessource(R.raw.button3));
            listOfImage.add(getRawRessource(R.raw.button4));
            listOfImage.add(getRawRessource(R.raw.button5));
            listOfImage.add(getRawRessource(R.raw.button6));
            listOfImage.add(getRawRessource(R.raw.button7));
            listOfImage.add(getRawRessource(R.raw.button8));
            listOfImage.add(getRawRessource(R.raw.button9));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection));
            listOfImage.add(getRawRessource(R.raw.buttoncancel));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate));
            listOfImage.add(getRawRessource(R.raw.button0_pressed));
            listOfImage.add(getRawRessource(R.raw.button1_pressed));
            listOfImage.add(getRawRessource(R.raw.button2_pressed));
            listOfImage.add(getRawRessource(R.raw.button3_pressed));
            listOfImage.add(getRawRessource(R.raw.button4_pressed));
            listOfImage.add(getRawRessource(R.raw.button5_pressed));
            listOfImage.add(getRawRessource(R.raw.button6_pressed));
            listOfImage.add(getRawRessource(R.raw.button7_pressed));
            listOfImage.add(getRawRessource(R.raw.button8_pressed));
            listOfImage.add(getRawRessource(R.raw.button9_pressed));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection_pressed));
            listOfImage.add(getRawRessource(R.raw.buttoncancel_pressed));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate_pressed));

            /* Retrieve layout raw data from TLC */
            try {
                layout = PinpadTLCWrapper.tlcPinpadGetNativeLayout();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }

            break;
        case HIGH:
            tLog.d(TAG, "High resolution screen.");
            listOfImage.add(getRawRessource(R.raw.pinpad_hd));
            listOfImage.add(getRawRessource(R.raw.star_hd));
            listOfImage.add(getRawRessource(R.raw.blank_hd));
            listOfImage.add(getRawRessource(R.raw.button0_hd));
            listOfImage.add(getRawRessource(R.raw.button1_hd));
            listOfImage.add(getRawRessource(R.raw.button2_hd));
            listOfImage.add(getRawRessource(R.raw.button3_hd));
            listOfImage.add(getRawRessource(R.raw.button4_hd));
            listOfImage.add(getRawRessource(R.raw.button5_hd));
            listOfImage.add(getRawRessource(R.raw.button6_hd));
            listOfImage.add(getRawRessource(R.raw.button7_hd));
            listOfImage.add(getRawRessource(R.raw.button8_hd));
            listOfImage.add(getRawRessource(R.raw.button9_hd));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection_hd));
            listOfImage.add(getRawRessource(R.raw.buttoncancel_hd));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate_hd));
            listOfImage.add(getRawRessource(R.raw.button0_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button1_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button2_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button3_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button4_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button5_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button6_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button7_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button8_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.button9_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.buttoncancel_pressed_hd));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate_pressed_hd));

            /* Retrieve HD layout raw data from TLC */
            try {
                layout = PinpadTLCWrapper.tlcPinpadGetNativeLayoutHD();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            break;

        case EXTRA_HIGH:
            tLog.d(TAG, "Extra High resolution screen.");
            listOfImage.add(getRawRessource(R.raw.pinpad_qhd));
            listOfImage.add(getRawRessource(R.raw.star_qhd));
            listOfImage.add(getRawRessource(R.raw.blank_qhd));
            listOfImage.add(getRawRessource(R.raw.button0_qhd));
            listOfImage.add(getRawRessource(R.raw.button1_qhd));
            listOfImage.add(getRawRessource(R.raw.button2_qhd));
            listOfImage.add(getRawRessource(R.raw.button3_qhd));
            listOfImage.add(getRawRessource(R.raw.button4_qhd));
            listOfImage.add(getRawRessource(R.raw.button5_qhd));
            listOfImage.add(getRawRessource(R.raw.button6_qhd));
            listOfImage.add(getRawRessource(R.raw.button7_qhd));
            listOfImage.add(getRawRessource(R.raw.button8_qhd));
            listOfImage.add(getRawRessource(R.raw.button9_qhd));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection_qhd));
            listOfImage.add(getRawRessource(R.raw.buttoncancel_qhd));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate_qhd));
            listOfImage.add(getRawRessource(R.raw.button0_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button1_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button2_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button3_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button4_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button5_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button6_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button7_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button8_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.button9_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.buttoncorrection_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.buttoncancel_pressed_qhd));
            listOfImage.add(getRawRessource(R.raw.buttonvalidate_pressed_qhd));

            /* Retrieve HD layout raw data from TLC */
            try {
                layout = PinpadTLCWrapper.tlcPinpadGetNativeLayoutQHD();
            } catch (Exception e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
            break;

        default:
            tLog.e(TAG, "ERROR: no ressources available for this screen resolution = " + resType);
            break;
        }

        tLog.d(TAG, "listOfImage contains " + listOfImage.size() + " images");
        for (int i = 0; i < listOfImage.size(); i++) {
            tLog.d(TAG, "image " + i + " size = " + listOfImage.get(i).length
                    + " bytes");
        }

        if (layout != null) {
            tLog.d(TAG, "layout size = " + layout.length + " bytes");
        } else {
            tLog.d(TAG, "layout is null!");
        }
    }

    private byte[] getRawRessource(int id) {
        InputStream inSteam = appContext.getResources().openRawResource(id);
        int imageLen = 0;
        byte[] tempBuffer = null;
        try {
            imageLen = inSteam.available();
            tempBuffer = new byte[imageLen];
            inSteam.read(tempBuffer);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
        try {
            inSteam.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }

        return tempBuffer;
    }

    public byte[] getRawImage(int select) {
        byte[] res = null;
        if (select < listOfImage.size()) {
            res = listOfImage.get(select);
        }
        return res;
    }

    public int getRawImageSize(int select) {
        if (select > listOfImage.size()) {
            return 0;
        }
        return listOfImage.get(select).length;
    }

    public byte[] getRawLayout() {
        return layout;
    }
}
