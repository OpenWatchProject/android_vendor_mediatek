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
 * MediaTek Inc. (C) 2019. All rights reserved.
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

#include <stdio.h>
#include <stdlib.h>
//#include <malloc.h>

//#define CLAMP(value) (value>255.0 ? 255 : value <0.0 ? 0 : (int)(value + 0.5))
#define CLAMP(value) ((int)(value)>255 ? 255 : ((int)(value) <0 ? 0 : (int)(value)))
#define ROUND_CLAMP(value) (value>255.0 ? 255 : value <0.0 ? 0 : (int)(value + 0.5))

typedef struct tagBITMAP {
    int offbits;
    int width;
    int height;
    int is_444;
    unsigned char *r;
    unsigned char *g;
    unsigned char *b;
} BITMAP;
typedef struct tagMATRIX {
    int length;
    double v[5];
} MATRIX;

void bmp_init(BITMAP *bmp, int width, int height, int is_444);
void bmp_eq(BITMAP *des, BITMAP *source, int conv);  // copy header and allocate memory of the same size
void bmp_copy(BITMAP *des, BITMAP *source);  // copy all elements
int bmp_getpos(BITMAP *bmp, int col, int row);
void bmp_conv(BITMAP *bmp, MATRIX *filt, int channel);
void bmp_transpose(BITMAP *bmp);
void bmp_444_422_conv(BITMAP *bmp);
void bmp_444_422_444(BITMAP *bmp);
void bmp_read(char *file, BITMAP* bmp);
void bmp_write(char *file, const BITMAP* bmp);
void bmp_crop(BITMAP *des, BITMAP *src, int x1,int x2, int y1,int y2, int x_sample, int y_sample);
void bmp_free(BITMAP *src);
void bmp_csc(BITMAP *bmp, int MODE); // 0:RGB2YCbCr 1:YCbCr2RGB

void hex_read(char *file, BITMAP* bmp);
void hex_write(char *file, BITMAP* bmp);
void hex_yuv_padx16_write(char *file, BITMAP* bmp, int yuv422);
void hex_rgb565_write(char *file, BITMAP* bmp);
void bin_yuv444_read (char *file1, BITMAP* bmp);
void bmp_plusrand(BITMAP *bmp, int seed, int strength);
void bmp_iir(BITMAP* bmp, double par);
int lfsr20(int cnt);

