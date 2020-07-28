package com.mediatek.mtklogger.c2klogger;

import android.util.Log;

/**
 * @author MTK81255
 *
 */
public class EtsFun extends EtsDevice {

    /**
     */
    public EtsFun() {
    }

    /**
     * @param pathDev String
     * @return Boolean
     */
    public Boolean open(String pathDev) {
        if (!create(pathDev)) {
            return false;
        }

        return true;
    }

    /**
     */
    public void close() {
        this.destroy();
    }

    // RF Reg Ctrl
    public static final byte AUTOMATIC = 0;
    public static final byte DISABLE = 1;
    public static final byte MANUAL = 2;

    // Ctrl Unit Type
    public static final byte CSS = 1;
    public static final byte HWD = 2;
    public static final byte BOTH = 3;

    // Rf Mode
    public static final byte CDMA_1XRTT = 0;
    public static final byte CDMA_EVDO = 1;

    // Rf Paths
    public static final byte MAIN = 1;
    public static final byte DIVERSITY = 2;
    public static final byte MAIN_AND_DIVERSITY = 3;

    // Cdma Band
    public static final byte BAND_CLASS_0 = 0; // 800 MHz cellular band
    public static final byte BAND_CLASS_1 = 1; // 1.8 to 2.0 GHz band
    public static final byte BAND_CLASS_2 = 2; // 872 to 960 MHz TACS band
    public static final byte BAND_CLASS_3 = 3; // 832 to 925 MHz JTACS band
    public static final byte BAND_CLASS_4 = 4; // 1.75 to 1.87 GHz Korean PCS
    public static final byte BAND_CLASS_5 = 5; // 450 MHz NMT band
    public static final byte BAND_CLASS_6 = 6; // 2 GHz IMT-2000 band
    public static final byte BAND_CLASS_7 = 7; // 700 MHz band
    public static final byte BAND_CLASS_8 = 8; // 1800 MHz band
    public static final byte BAND_CLASS_9 = 9; // 900 MHz band
    public static final byte BAND_CLASS_10 = 10; // Secondary 800 MHz NMT band
    public static final byte BAND_CLASS_11 = 11; // 400 MHz European PARM band
    public static final byte BAND_CLASS_12 = 12; // 800 MHz PAMR band
    public static final byte BAND_CLASS_13 = 13; // 2.5 GHz IMT-2000
    public static final byte BAND_CLASS_14 = 14; // US PCS 1.9 GHz band
    public static final byte BAND_CLASS_15 = 15; // AWS band
    public static final byte BAND_CLASS_16 = 16; // US 2.5 GHz band

    /**
     * @param ctrlMode byte
     * @param ctrlUnit byte
     * @param rfMode byte
     * @param rfPath byte
     * @param band byte
     * @param channel short
     * @return Boolean
     */
    public Boolean configChannel(byte ctrlMode, byte ctrlUnit, byte rfMode,
            byte rfPath, byte band, short channel) {
        Log.v("via_ets", "pll config channel");

        short id = 0x05ac;
        byte[] dataChannel = EtsUtil.short2bytes(channel);

        byte[] cmd = new byte[] { ctrlMode, ctrlUnit, rfMode, rfPath, band,
                dataChannel[0], dataChannel[1] };
        EtsMsg msg = sendAndWait(new EtsMsg(id, cmd), id, 2000);

        return msg != null;
    }

}
