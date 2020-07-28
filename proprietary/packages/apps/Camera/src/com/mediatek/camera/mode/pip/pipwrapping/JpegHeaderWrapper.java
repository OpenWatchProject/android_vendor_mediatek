/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2015. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.camera.mode.pip.pipwrapping;

import com.mediatek.camera.util.Log;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.EOFException;
import java.io.FilterInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.nio.charset.Charset;

public class JpegHeaderWrapper {
    private static final String TAG = JpegHeaderWrapper.class.getSimpleName();
    private static final short SOI = (short) 0xFFD8;
    private static final short EOI = (short) 0xFFD9;
    private static final short APP0 = (short) 0xFFE0;
    private static final short APP1 = (short) 0xFFE1;
    private static final short APP2 = (short) 0xFFE2;
    private static final short APP3 = (short) 0xFFE3;
    private static final short APP4 = (short) 0xFFE4;
    private static final short APP5 = (short) 0xFFE5;
    private static final short APP6 = (short) 0xFFE6;
    private static final short APP7 = (short) 0xFFE7;
    private static final short APP8 = (short) 0xFFE8;
    private static final short APP9 = (short) 0xFFE9;
    private static final short APP10 = (short) 0xFFEA;
    private static final short APP11 = (short) 0xFFEB;
    private static final short APP12 = (short) 0xFFEC;
    private static final short APP13 = (short) 0xFFED;
    private static final short APP14 = (short) 0xFFEE;
    private static final short APP15 = (short) 0xFFEF;
    /**
     * SOF (start of frame). All value between SOF0 and SOF15 is SOF marker
     * except for DHT, JPG, and DAC marker.
     */
    private static final short SOF0 = (short) 0xFFC0;
    private static final short SOF15 = (short) 0xFFCF;
    private static final short DHT = (short) 0xFFC4;
    private static final short JPG = (short) 0xFFC8;
    private static final short DAC = (short) 0xFFCC;

    public byte[] readJpegHeader(byte[] jpeg) throws IllegalArgumentException, Exception {
        int jpegHeaderLength = readJpegHeaderLength(jpeg);
        InputStream inStream = new ByteArrayInputStream(jpeg);
        byte[] jpegHeader = new byte[jpegHeaderLength];
        int readLength = inStream.read(jpegHeader, 0, jpegHeaderLength);
        inStream.close();
        Log.d(TAG, "readJpegHeader jpegHeader length = " + jpegHeader.length + ",readLength = "
                + readLength + ",jpegHeaderLength = " + jpegHeaderLength);
        return jpegHeader;
    }

    /**
     * Write header to jpeg buffer.
     *
     * @param jpeg  the jpeg that want to be written.
     * @param header the header that want to write.
     * @return the jpeg with specific header.
     * @throws IOException if write with error, thows IO exception.
     */
    public byte[] writeJpegHeader(byte[] jpeg, byte[] header) throws IOException {
        ByteArrayOutputStream out = new ByteArrayOutputStream();
        InputStream inStream = new ByteArrayInputStream(jpeg);
        try {
            int jpegHeaderLength = readJpegHeaderLength(jpeg);
            int jpegDataLength = jpeg.length - jpegHeaderLength;
            Log.d(TAG, "[writeJpegHeader]jpegHeaderLength = " + jpegHeaderLength
                    + " jpegDataLength = " + jpegDataLength);
            byte[] jpegdata = new byte[jpegDataLength];
            inStream.skip(jpegHeaderLength);
            int readLength = inStream.read(jpegdata);
            Log.d(TAG, "[writeJpegHeader]read raw jpage data length = " + jpegdata.length
                    + ",readLength = " + readLength);
            out.write(header);
            out.write(jpegdata);
            out.flush();
            jpegdata = null;
            System.gc();
        } catch (Exception e) {
            Log.e(TAG, "[writeJpegHeader]exceptioin " + e.toString());
        } finally {
            inStream.close();
            out.close();
        }
        return out.toByteArray();
    }

    @SuppressWarnings("resource")
    private int readJpegHeaderLength(byte[] jpeg) throws Exception {
        if (jpeg == null) {
            Log.e(TAG, "[readJpegHeaderLength]jpeg is null!");
            throw new IllegalArgumentException("Argument is null");
        }
        InputStream inStream = new ByteArrayInputStream(jpeg);
        CountedDataInputStream dataStream = new CountedDataInputStream(inStream);
        if (dataStream.readShort() != SOI) {
            Log.e(TAG, "[readJpegHeaderLength]Invalid Jpeg Format!");
            throw new Exception("Invalid Jpeg Format");
        }
        short marker = dataStream.readShort();
        int jpegHeaderLength = 2; // 2 bytes SOI
        while (marker != EOI && !isSofMarker(marker)) {
            int markerLength = dataStream.readUnsignedShort();
            if (!(marker == APP0 || marker == APP1 || marker == APP2 || marker == APP3
                    || marker == APP4 || marker == APP5 || marker == APP6 || marker == APP7
                    || marker == APP8 || marker == APP9 || marker == APP10 || marker == APP11
                    || marker == APP12 || marker == APP13 || marker == APP14 || marker == APP15)) {
                break;
            }
            jpegHeaderLength += (markerLength + 2); // 2 bytes marker
            Log.d(TAG, "Read marker = " + Integer.toHexString(marker) + " jpegHeaderLength = "
                    + jpegHeaderLength);
            if (markerLength < 2 || (markerLength - 2) != dataStream.skip(markerLength - 2)) {
                Log.e(TAG, "[readJpegHeaderLength]Invalid Marker Length = " + markerLength);
                throw new Exception("Invalid Marker Length = " + markerLength);
            }
            marker = dataStream.readShort();
        }
        inStream.close();
        dataStream.close();
        return jpegHeaderLength;
    }

    private boolean isSofMarker(short marker) {
        return marker >= SOF0 && marker <= SOF15 && marker != DHT && marker != JPG && marker != DAC;
    }

    private class CountedDataInputStream extends FilterInputStream {
        private int mCount = 0;
        // allocate a byte buffer for a long value;
        private final byte mByteArray[] = new byte[8];
        private final ByteBuffer mByteBuffer = ByteBuffer.wrap(mByteArray);
        public CountedDataInputStream(InputStream in) {
            super(in);
        }

        public int getReadByteCount() {
            return mCount;
        }

        @Override
        public int read(byte[] b) throws IOException {
            int r = in.read(b);
            mCount += (r >= 0) ? r : 0;
            return r;
        }

        @Override
        public int read(byte[] b, int off, int len) throws IOException {
            int r = in.read(b, off, len);
            mCount += (r >= 0) ? r : 0;
            return r;
        }

        @Override
        public int read() throws IOException {
            int r = in.read();
            mCount += (r >= 0) ? 1 : 0;
            return r;
        }

        @Override
        public long skip(long length) throws IOException {
            long skip = in.skip(length);
            mCount += skip;
            return skip;
        }

        public void skipOrThrow(long length) throws IOException {
            if (skip(length) != length)
                throw new EOFException();
        }

        public void skipTo(long target) throws IOException {
            long cur = mCount;
            long diff = target - cur;
            assert (diff >= 0);
            skipOrThrow(diff);
        }

        public void readOrThrow(byte[] b, int off, int len) throws IOException {
            int r = read(b, off, len);
            if (r != len)
                throw new EOFException();
        }

        public void readOrThrow(byte[] b) throws IOException {
            readOrThrow(b, 0, b.length);
        }

        public void setByteOrder(ByteOrder order) {
            mByteBuffer.order(order);
        }

        public ByteOrder getByteOrder() {
            return mByteBuffer.order();
        }

        public short readShort() throws IOException {
            readOrThrow(mByteArray, 0, 2);
            mByteBuffer.rewind();
            return mByteBuffer.getShort();
        }

        public int readUnsignedShort() throws IOException {
            return readShort() & 0xffff;
        }

        public int readInt() throws IOException {
            readOrThrow(mByteArray, 0, 4);
            mByteBuffer.rewind();
            return mByteBuffer.getInt();
        }

        public long readUnsignedInt() throws IOException {
            return readInt() & 0xffffffffL;
        }

        public long readLong() throws IOException {
            readOrThrow(mByteArray, 0, 8);
            mByteBuffer.rewind();
            return mByteBuffer.getLong();
        }

        public String readString(int n) throws IOException {
            byte buf[] = new byte[n];
            readOrThrow(buf);
            return new String(buf, "UTF8");
        }

        public String readString(int n, Charset charset) throws IOException {
            byte buf[] = new byte[n];
            readOrThrow(buf);
            return new String(buf, charset);
        }
    }
}