package com.mediatek.mtklogger.c2klogger;

/**
 * @author MTK81255
 *
 */
public class EtsMsgProgCmd extends EtsMsg {
    public static short sID = 1202;

    public static byte sEarse = 0;
    public static byte sWaite = 1;

    /**
     * for command.
     * @param sequence int
     * @param type byte
     * @param section byte
     * @param data byte[]
     */
    public EtsMsgProgCmd(int sequence, byte type, byte section, byte[] data) {
        super(sID, new byte[(data == null ? 0 : data.length) + 10]);

        byte[] temp = EtsUtil.int2bytes(sequence);
        System.arraycopy(temp, 0, mData, 0, 4);

        mData[4] = type;
        mData[5] = section;

        if (data != null) {
            // checksum
            short checksum = EtsUtil.checkSum(data);
            temp = EtsUtil.short2bytes(checksum);
            System.arraycopy(temp, 0, mData, 6, 2);

            // size
            temp = EtsUtil.short2bytes((short) (data.length & 0xFFFF));
            System.arraycopy(temp, 0, mData, 8, 2);

            // data
            System.arraycopy(data, 0, mData, 10, data.length);

        } else {
            mData[6] = 0x00;
            mData[7] = 0x00;
            mData[8] = 0x00;
            mData[9] = 0x00;
        }

    }
}
