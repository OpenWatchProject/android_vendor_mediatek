package com.mediatek.mtklogger.c2klogger;

import android.util.Log;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

/**
 * @author MTK81255
 *
 */
public class EtsMsg {

    public static int sSizeLast = 0;

    private static long sSpanMs = 2209017600000L; // 2208988800000l;
    private static long sMsPerDay = 86400000L;

    protected long mTime = 0;
    protected short mId;
    protected byte[] mData = null;

    public short getId() {
        return mId;
    }

    public byte[] getData() {
        return mData;
    }

    /**
     * @return byte[]
     */
    public byte[] getLogEntry() {
        byte[] buf = new byte[mData.length + 14];

        // log tickcount and txrx
        long timeNow = System.currentTimeMillis();

        // old version
        // byte[] time = EtsUtil.long2bytes(time_now);

        // transfer it to MFC's DATE
        timeNow += sSpanMs;
        double mfcDateTimestamp = (double) timeNow / sMsPerDay;

        byte[] time = EtsUtil.doubleToByte(mfcDateTimestamp);

        // put it into buf
        System.arraycopy(time, 0, buf, 0, 8);

        // rx
        buf[8] = 0x00;
        buf[9] = 0x00;

        // length
        byte[] tmp = EtsUtil.short2bytes(getLength());
        System.arraycopy(tmp, 0, buf, 10, 2);

        // id
        tmp = EtsUtil.short2bytes(mId);
        System.arraycopy(tmp, 0, buf, 12, 2);

        // data
        System.arraycopy(mData, 0, buf, 14, mData.length);

        return buf;
    }

    /**
     * @return byte[]
     */
    public byte[] getBuf() {
        if (mData == null) {
            return null;
        }

        byte[] buf = new byte[mData.length + 8];

        // header
        buf[0] = (byte) 0xFE;
        buf[1] = (byte) 0xDC;
        buf[2] = (byte) 0xBA;
        buf[3] = (byte) 0x98;

        // length
        short length = (short) (mData.length + 2);
        buf[4] = (byte) (length & 0xFF);
        buf[5] = (byte) (length >> 8 & 0xFF);

        // id
        buf[6] = (byte) (mId & 0xFF);
        buf[7] = (byte) (mId >> 8 & 0xFF);

        // data
        System.arraycopy(mData, 0, buf, 8, mData.length);

        return buf;
    }

    public short getLength() {
        return (short) (mData.length + 2);
    }

    /**
     * @return int
     */
    public int getProgRspSequence() {
        if (mId != 1202) {
            return -1;
        }

        return EtsUtil.bytes2int(mData);
    }

    /**
     * @return byte
     */
    public byte getProgRspAck() {
        if (mId != 1202) {
            return -1;
        }

        return mData[4];
    }

    /**
     * @param id short
     * @param data byte[]
     */
    public EtsMsg(short id, byte[] data) {
        this("00:00:00.0", id, data);
    }

    /**
     * @param time String
     * @param id short
     * @param data byte[]
     */
    public EtsMsg(String time, short id, byte[] data) {
        String strTime = "2000-01-01 " + time;
        SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
        Date date = null;
        try {
            date = sdf.parse(strTime);
            mTime = date.getTime();
        } catch (ParseException e) {
            e.printStackTrace();
        }
        mId = id;

        if (data != null) {
            mData = new byte[data.length];
            System.arraycopy(data, 0, mData, 0, data.length);

        } else {
            mData = new byte[0];
        }
    }

    /**
     * @param buf
     *            : raw bytes received from ETS device
     * @param sizeBuf int
     * @return List<EtsMsg>
     */
    public static List<EtsMsg> parse(byte buf[], int sizeBuf) {
        int index = 0;
        List<EtsMsg> msgs = new ArrayList<EtsMsg>();

        sSizeLast = sizeBuf;
        while (sSizeLast >= 8) {

            // find the begin
            Boolean findHeader = false;
            for (; index <= sizeBuf - 8; ++index) {
                if (buf[index] == (byte) 0xFE && buf[index + 1] == (byte) 0xDC
                        && buf[index + 2] == (byte) 0xBA
                        && buf[index + 3] == (byte) 0x98) {

                    findHeader = true;
                    break;
                }
            }

            if (!findHeader) {
                Log.e("via_ets", "can't find the header(index=" + index + ")");
                sSizeLast = 0;
                break;
            }

            sSizeLast = sizeBuf - index; // the begin of this message
            index += 4;

            // length
            byte[] temp = new byte[2];
            temp[0] = buf[index++];
            temp[1] = buf[index++];

            short length = EtsUtil.bytes2short(temp);
            if (length < 2 || length > 2048) {
                Log.e("via_ets", "invalid length(length=" + length + "),index="
                        + index);
                // size_last = size_buf-index;
                continue;
            }

            if (sizeBuf < index + length) {
                // if(length>size_last-6) {
                Log.w("via_ets", "not enough data for this message(length="
                        + length + ", last buf size=" + (sizeBuf - index));
                break;
            }

            // id
            temp[0] = buf[index++];
            temp[1] = buf[index++];
            short id = EtsUtil.bytes2short(temp);

            // data
            temp = new byte[length - 2];
            System.arraycopy(buf, index, temp, 0, length - 2);
            index += length - 2;

            // new msg
            msgs.add(new EtsMsg(id, temp));

            // refresh the size_last
            sSizeLast = sizeBuf - index;
        }

        return msgs;
    }

    /**
     * @param buf byte[]
     * @param sizeTotal int
     * @return int
     */
    public static int removeErrorBuffer(byte[] buf, int sizeTotal) {
        Log.i("via_ets", "The size_buf = " + sizeTotal);
        int index = 0;
        if (sizeTotal <= 8) {
            return 0;
        }

        Boolean findHeader = false;
        for (; index <= sizeTotal - 8; ++index) {
            if ((buf[index] == (byte) 0xFE) && (buf[index + 1] == (byte) 0xDC)
                    && (buf[index + 2] == (byte) 0xBA)
                    && (buf[index + 3] == (byte) 0x98)) {
                Log.i("via_ets", "buf[" + index + "] is " + buf[index]);
                Log.i("via_ets", "buf[" + (index + 1) + "] is "
                        + buf[index + 1]);
                Log.i("via_ets", "buf[" + (index + 2) + "] is "
                        + buf[index + 2]);
                Log.i("via_ets", "buf[" + (index + 3) + "] is "
                        + buf[index + 3]);
                findHeader = true;
                break;
            }
        }

        Log.e("via_ets", "The index=" + index);
        if (!findHeader) {
            Log.e("via_ets", "can't find the header(index=" + index + ")");
            return 0;
        }

        System.arraycopy(buf, index, buf, 0, sizeTotal - index);
        return sizeTotal - index;
    }

    /**
     * @param line
     *            : like "14:52:19.8[ Raw Tx: Len=5, 0x65 0x00 0x00 0x00 0x00".
     * @return EtsMsg
     */
    public static EtsMsg parse(String line) {

        int index = line.indexOf("Raw Tx"); // or Rx
        if (index < 0) {
            return null;
        }

        index = line.indexOf(',', index);
        if (index < 0) {
            return null;
        }
        index += 2;

        String dataLine = line.substring(index);
        byte[] msgBin = hexStr2bytes(dataLine);

        // id
        short id = EtsUtil.bytes2short(msgBin);

        // data
        byte[] data = new byte[msgBin.length - 2];
        for (int i = 0; i < msgBin.length - 2; i++) {
            data[i] = msgBin[i + 2];
        }

        return new EtsMsg(line.substring(0, 10), id, data);
    }

    /**
     * @param data
     *            : like 0x65 0x00 0x00 0x00 0x00
     * @return byte[]
     */
    private static byte[] hexStr2bytes(String src) {
        String[] datas = src.split(" ");

        byte[] ret = new byte[datas.length];
        for (int i = 0; i < datas.length; ++i) {
            ret[i] = Integer.decode(datas[i]).byteValue();
        }

        return ret;
    }

}
