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
 * MediaTek Inc. (C) 2010. All rights reserved.
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

package com.mediatek.ppl;

/**
 * Dump binary data in hex form.
 */
public class HexDumper {

    public static void hexdump(byte[] data) {
        final int ROW_BYTES = 16;
        final int ROW_QTR1 = 3;
        final int ROW_HALF = 7;
        final int ROW_QTR2 = 11;
        int rows, residue, i, j;
        byte[] save_buf = new byte[ROW_BYTES + 2];
        char[] hex_buf = new char[4];
        char[] idx_buf = new char[8];
        char[] hex_chars = new char[20];

        hex_chars[0] = '0';
        hex_chars[1] = '1';
        hex_chars[2] = '2';
        hex_chars[3] = '3';
        hex_chars[4] = '4';
        hex_chars[5] = '5';
        hex_chars[6] = '6';
        hex_chars[7] = '7';
        hex_chars[8] = '8';
        hex_chars[9] = '9';
        hex_chars[10] = 'A';
        hex_chars[11] = 'B';
        hex_chars[12] = 'C';
        hex_chars[13] = 'D';
        hex_chars[14] = 'E';
        hex_chars[15] = 'F';

        rows = data.length >> 4;
        residue = data.length & 0x0000000F;
        for (i = 0; i < rows; i++)
        {
            int hexVal = (i * ROW_BYTES);
            idx_buf[0] = hex_chars[((hexVal >> 12) & 15)];
            idx_buf[1] = hex_chars[((hexVal >> 8) & 15)];
            idx_buf[2] = hex_chars[((hexVal >> 4) & 15)];
            idx_buf[3] = hex_chars[(hexVal & 15)];

            String idxStr = new String(idx_buf, 0, 4);
            System.out.print(idxStr + ": ");

            for (j = 0; j < ROW_BYTES; j++)
            {
                save_buf[j] = data[(i * ROW_BYTES) + j];

                hex_buf[0] = hex_chars[(save_buf[j] >> 4) & 0x0F];
                hex_buf[1] = hex_chars[save_buf[j] & 0x0F];

                System.out.print(hex_buf[0]);
                System.out.print(hex_buf[1]);
                System.out.print(' ');

                if (j == ROW_QTR1 || j == ROW_HALF || j == ROW_QTR2)
                    System.out.print(" ");

                if (save_buf[j] < 0x20 || save_buf[j] > 0x7E)
                    save_buf[j] = (byte) '.';
            }

            String saveStr = new String(save_buf, 0, j);
            System.out.println(" | " + saveStr + " |");
        }

        if (residue > 0)
        {
            int hexVal = (i * ROW_BYTES);
            idx_buf[0] = hex_chars[((hexVal >> 12) & 15)];
            idx_buf[1] = hex_chars[((hexVal >> 8) & 15)];
            idx_buf[2] = hex_chars[((hexVal >> 4) & 15)];
            idx_buf[3] = hex_chars[(hexVal & 15)];

            String idxStr = new String(idx_buf, 0, 4);
            System.out.print(idxStr + ": ");

            for (j = 0; j < residue; j++)
            {
                save_buf[j] = data[(i * ROW_BYTES) + j];

                hex_buf[0] = hex_chars[(save_buf[j] >> 4) & 0x0F];
                hex_buf[1] = hex_chars[save_buf[j] & 0x0F];

                System.out.print((char) hex_buf[0]);
                System.out.print((char) hex_buf[1]);
                System.out.print(' ');

                if (j == ROW_QTR1 || j == ROW_HALF || j == ROW_QTR2)
                    System.out.print(" ");

                if (save_buf[j] < 0x20 || save_buf[j] > 0x7E)
                    save_buf[j] = (byte) '.';
            }

            for ( /* j INHERITED */; j < ROW_BYTES; j++)
            {
                save_buf[j] = (byte) ' ';
                System.out.print("   ");
                if (j == ROW_QTR1 || j == ROW_HALF || j == ROW_QTR2)
                    System.out.print(" ");
            }

            String saveStr = new String(save_buf, 0, j);
            System.out.println(" | " + saveStr + " |");
        }
    }
}
