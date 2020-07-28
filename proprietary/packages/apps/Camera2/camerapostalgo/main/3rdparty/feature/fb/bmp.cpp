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

#include "global.h"
#include "bmp.h"

void bmp_init(BITMAP *bmp, int width, int height, int is_444) {
    bmp->width   = width;
    bmp->height  = height;
    bmp->offbits = 54;
    bmp->is_444  = is_444;

    if (is_444) {
        if ((bmp->r=(unsigned char *)calloc( bmp->height*bmp->width, sizeof(unsigned char) )) == NULL ||
                (bmp->g=(unsigned char *)calloc( bmp->height*bmp->width, sizeof(unsigned char) )) == NULL ||
                (bmp->b=(unsigned char *)calloc( bmp->height*bmp->width, sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
    }
    else {
        if ((bmp->r=(unsigned char *)calloc( bmp->height*bmp->width, sizeof(unsigned char) )) == NULL ||
                (bmp->g=(unsigned char *)calloc( bmp->height*bmp->width/2, sizeof(unsigned char) )) == NULL ||
                (bmp->b=(unsigned char *)calloc( bmp->height*bmp->width/2, sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
    }
}


// copy header and allocate memory of the same size
void bmp_eq(BITMAP *des, BITMAP *source, int convert) {
    des->width   = source->width;
    des->height  = source->height;
    des->offbits = source->offbits;
    des->is_444  = convert ^ source->is_444;

    if(des->is_444) {
        if ((des->r=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
                (des->g=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
                (des->b=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
    }
    else {
        if ((des->r=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
                (des->g=(unsigned char *)calloc( des->height*des->width/2,sizeof(unsigned char) )) == NULL ||
                (des->b=(unsigned char *)calloc( des->height*des->width/2,sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
    }
}

void bmp_copy(BITMAP *des, BITMAP *source) {
    int i;

    des->width   = source->width;
    des->height  = source->height;
    des->offbits = source->offbits;
    des->is_444  = source->is_444;

    if(des->is_444) {
        if ((des->r=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
                (des->g=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
                (des->b=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
        for (i = 0; i<des->width*des->height; i++) {
            des->r[i] = source->r[i];
            des->g[i] = source->g[i];
            des->b[i] = source->b[i];
        }
    }
    else {
        if ((des->r=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
                (des->g=(unsigned char *)calloc( des->height*des->width/2,sizeof(unsigned char) )) == NULL ||
                (des->b=(unsigned char *)calloc( des->height*des->width/2,sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
        for (i = 0; i<des->width*des->height; i++) {
            des->r[i] = source->r[i];
        }
        for (i= 0; i<des->width*des->height/2; i++) {
            des->g[i] = source->g[i];
            des->b[i] = source->b[i];
        }
    }
}

int bmp_getpos(BITMAP *bmp, int col, int row) {
    int pos;
    int col_corr, row_corr;

    col_corr = (col < 0) ? 0 : (col > (bmp->width-1)  ? bmp->width-1  : col);
    row_corr = (row < 0) ? 0 : (row > (bmp->height-1) ? bmp->height-1 : row);

    pos =  row_corr * bmp->width + col_corr;
    return pos;
}

void bmp_conv(BITMAP *bmp, MATRIX *filt, int channel) {

    int i, j, k;
    unsigned char * buffer_r, * buffer_g, * buffer_b;

    int pos;
    int offset;
    double temp_r, temp_g, temp_b;

    offset = (filt->length/2);

    if (channel == 3) {
        if ((buffer_r=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ||
                (buffer_g=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ||
                (buffer_b=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
        for(i = 0; i<bmp->height; i++) {
            for(j = 0; j<bmp->width; j++) {
                temp_r = 0.0;
                temp_g = 0.0;
                temp_b = 0.0;

                for(k = 0; k < filt->length; k++) {
                    pos = bmp_getpos(bmp, j, i - offset + k);
                    temp_r += (double)bmp->r[pos] * filt->v[k];
                    temp_g += (double)bmp->g[pos] * filt->v[k];
                    temp_b += (double)bmp->b[pos] * filt->v[k];
                }

                pos = bmp_getpos(bmp, j, i);
                buffer_r[pos] = CLAMP(temp_r);
                buffer_g[pos] = CLAMP(temp_g);
                buffer_b[pos] = CLAMP(temp_b);
            }
        }

        free(bmp->r);
        free(bmp->g);
        free(bmp->b);

        bmp->r = buffer_r;
        bmp->g = buffer_g;
        bmp->b = buffer_b;
    }
    else if (channel == 1) {
        if ((buffer_r=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL) {
            printf("Error allocating memory \n");
            exit(1);
        }
        for(i = 0; i<bmp->height; i++) {
            for(j = 0; j<bmp->width; j++) {
                temp_r = 0.0;

                for(k = 0; k < filt->length; k++) {
                    pos = bmp_getpos(bmp, j, i - offset + k);
                    temp_r += (double)bmp->r[pos] * filt->v[k];
                }

                pos = bmp_getpos(bmp, j, i);
                buffer_r[pos] = CLAMP(temp_r);
            }
        }

        free(bmp->r);
        bmp->r = buffer_r;
    }
}

void bmp_transpose(BITMAP *bmp) { //not verified yet
    int i, j;
    unsigned char * buffer_r, * buffer_g, * buffer_b;

    int temp_width;

    if(!bmp->is_444) {
        printf("Warning! Transpose only effective with 444 format!\n\n");
        exit(0);
    }
    if ((buffer_r=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ||
            (buffer_g=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ||
            (buffer_b=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ) {
        printf("Error allocating memory \n");
        exit(1);
    }
    for(i = 0; i<bmp->height; i++) {
        for(j = 0; j<bmp->width; j++) {
            buffer_r[j*bmp->height + i] = bmp->r[i*bmp->width + j];
            buffer_g[j*bmp->height + i] = bmp->g[i*bmp->width + j];
            buffer_b[j*bmp->height + i] = bmp->b[i*bmp->width + j];
        }
    }

    temp_width = bmp->width;
    bmp->width = bmp->height;
    bmp->height = temp_width;

    free(bmp->r);
    free(bmp->g);
    free(bmp->b);

    bmp->r = buffer_r;
    bmp->g = buffer_g;
    bmp->b = buffer_b;
}

void bmp_444_422_conv(BITMAP *bmp) { //not verified yet
    int i;
    int temp_g;
    int temp_b;
    unsigned char * buffer_g, * buffer_b;

    bmp->is_444 = bmp->is_444 ? 0 : 1;
    if(bmp->is_444) {
        if ((buffer_g=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ||
                (buffer_b=(unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
    }
    else {
        if ((buffer_g=(unsigned char *)calloc( bmp->height*bmp->width/2,sizeof(unsigned char) )) == NULL ||
                (buffer_b=(unsigned char *)calloc( bmp->height*bmp->width/2,sizeof(unsigned char) )) == NULL ) {
            printf("Error allocating memory \n");
            exit(1);
        }
    }
    //444 to 422
    if(!bmp->is_444) {
        for(i = 0; i<bmp->width*bmp->height; i++) {
            if(i%2 == 0) {
                temp_g = bmp->g[i];
                temp_b = bmp->b[i];
            }
            else {
                buffer_g[i/2] = (unsigned char)((temp_g + bmp->g[i])/2);
                buffer_b[i/2] = (unsigned char)((temp_b + bmp->b[i])/2);
            }
        }
    }
    //422 to 444
    else {
        for(i = 0; i<bmp->width*bmp->height; i++) {
            buffer_g[i] = bmp->g[i/2];
            buffer_b[i] = bmp->b[i/2];
        }
    }

    free(bmp->g);
    free(bmp->b);

    bmp->g = buffer_g;
    bmp->b = buffer_b;
}

void bmp_444_422_444(BITMAP *bmp) {
    int i, j, pos;

    //444 to 422 to 444
    for(i = 0; i< bmp->height; i++) {
        for (j = 0; j < bmp->width; j = j + 2) {
            if(j != (bmp->width -1)) {
                pos = i*bmp->width + j;
                bmp->g[pos] = (unsigned char)((bmp->g[pos] + bmp->g[pos+1])/2);
                bmp->b[pos] = (unsigned char)((bmp->b[pos] + bmp->b[pos+1])/2);
                bmp->g[pos+1] = bmp->g[pos];
                bmp->b[pos+1] = bmp->b[pos];
            }
        }
    }
}

/*
void bmp_read(char *file, BITMAP* bmp) {
    FILE *fp;
    int index;
    int x;
    int pad;

    // open the file
    if ((fp = fopen(file,"rb")) == NULL) {
        printf("Error opening file %s.\n",file);
        exit(1);
    }

    //check bitmap file header
    if (fgetc(fp)!='B'||fgetc(fp)!='M') {
        fclose(fp);
        printf("not a bitmap file\n");
        exit(1);
    }

    //read offset bits, width and height only. ignore others
    fseek(fp, 8, 1);
    fread(&bmp->offbits, sizeof(int), 1, fp);
    fseek(fp, 4, 1);
    fread(&bmp->width, sizeof(int), 1, fp);
    fread(&bmp->height, sizeof(int), 1, fp);

    pad = bmp->width % 4;

    fseek(fp, 28, 1);

    // printf(" offbits %d width %d  height %d\n", bmp->offbits, bmp->width, bmp->height);

    // allocating memory for r g b
    if ((bmp->r = (unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char) )) == NULL) {
        fclose(fp);
        printf("Error allocating memory for r\n");
        exit(1);
    }

    if ((bmp->g = (unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char))) == NULL) {
        fclose(fp);
        printf("Error allocating memory for g\n");
        exit(1);
    }

    if ((bmp->b = (unsigned char *)calloc( bmp->height*bmp->width,sizeof(unsigned char))) == NULL) {
        fclose(fp);
        printf("Error allocating memory for b\n");
        exit(1);
    }

    //read bitmap pixel value
    for(index=((bmp->height-1)* bmp->width); index>=0; index-=bmp->width) {
        for(x=0; x<bmp->width; x++) {
            bmp->b[index+x]=fgetc(fp);
            bmp->g[index+x]=fgetc(fp);
            bmp->r[index+x]=fgetc(fp);b
        }
        for(x=0; x<pad; x++)
            fgetc(fp);
    }

    bmp->is_444 = 1;

    fclose(fp);
}*/

void bmp_write(char *file, const BITMAP* bmp) {
    FILE *fp;
    int index;
    int x;
    int temp;
    int pad = bmp->width %4;;

    /* open the file */
    if ((fp = fopen(file,"wb")) == NULL) {
        printf("Error opening file %s.\n",file);
        exit(1);
    }

    fputc('B',fp);
    fputc('M',fp);
    //total file size = w * h * 3 + offset
    temp=bmp->width*bmp->height*3+pad*bmp->height+bmp->offbits;
    //printf(" offbits %d width %d  height %d temp %ld\n", bmp->offbits, bmp->width, bmp->height, temp);
    fwrite(&temp, sizeof(int), 1, fp);

    fputc(0x00,fp);
    fputc(0x00,fp);
    fputc(0x00,fp);
    fputc(0x00,fp);

    fwrite(&bmp->offbits, sizeof(int), 1, fp);

    //offset - already read bits : 54 - 14 =40
    fputc(0x28,fp);
    fputc(0x00,fp);
    fputc(0x00,fp);
    fputc(0x00,fp);

    //width and height
    fwrite(&bmp->width, sizeof(int), 1, fp);
    fwrite(&bmp->height, sizeof(int), 1, fp);

    //reserved
    fputc(0x01,fp);
    fputc(0x00,fp);

    //24 bit bitmap
    fputc(0x18,fp);
    fputc(0x00,fp);

    fputc(0x00,fp);
    fputc(0x00,fp);
    fputc(0x00,fp);
    fputc(0x00,fp);

    //total pixel value = w * h * 3
    temp=bmp->width * bmp->height *3 +pad * bmp->height;
    fwrite(&temp, sizeof(int), 1, fp);

    for(x=0;x<16;x++)
        fputc(0x00,fp);

    // iput pixel value

    for(index=((bmp->height-1)* bmp->width); index>=0; index-=bmp->width) {
        for(x=0; x<bmp->width; x++) {
            fputc(bmp->b[index+x],fp);
            fputc(bmp->g[index+x],fp);
            fputc(bmp->r[index+x],fp);
        }
        for(x=0; x<pad; x++)
            fputc(0x00,fp);
    }

    fclose(fp);
}

void bmp_crop(BITMAP *des, BITMAP *src, int x1, int x2, int y1, int y2, int x_sample, int y_sample) {
    int i,j,k;

    des->width = (x2-x1+1)/x_sample;
    des->height = (y2-y1+1)/y_sample;
    des->offbits = src->offbits;
    des->is_444 = src->is_444;

    if ((des->r=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
            (des->g=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ||
            (des->b=(unsigned char *)calloc( des->height*des->width,sizeof(unsigned char) )) == NULL ) {
        printf("Cropped Function: Error allocating memory \n");
        exit(1);
    }

    for(i=0,j=src->width*y1;j<=src->width*y2;j+=(y_sample*src->width)) {
        for(k=x1; k<=x2; k += x_sample, i++) {
            des->r[i] = src->r[j+k];
            des->g[i] = src->g[j+k];
            des->b[i] = src->b[j+k];
        }
    }

}

void bmp_csc(BITMAP *bmp, int MODE) { //0:RGB2YCbCr 1:YCbCr2RGB
    int i;
    int R, G, B;
    int Y, Cb, Cr;

    if(MODE == 0)
        for(i=0;i<bmp->width*bmp->height;i++) {
            R  = bmp->r[i];
            G  = bmp->g[i];
            B  = bmp->b[i];
            Y  = ROUND_CLAMP(0.299*R + 0.587*G + 0.114*B);
            Cb = ROUND_CLAMP(-0.169*R - 0.331*G + 0.5*B + 128.0);
            Cr = ROUND_CLAMP(0.5*R - 0.419*G - 0.081*B + 128.0);
            bmp->r[i] = (unsigned char)Y;
            bmp->g[i] = (unsigned char)Cb;
            bmp->b[i] = (unsigned char)Cr;
        }
    else
        for(i=0;i<bmp->width*bmp->height;i++) {
            Y  = bmp->r[i];
            Cb = bmp->g[i];
            Cr = bmp->b[i];
            R  = ROUND_CLAMP((float)Y+1.402*(float)(Cr-128.0));
            G  = ROUND_CLAMP((float)Y-0.344136*(float)(Cb-128.0)-0.714136*(float)(Cr-128.0));
            B  = ROUND_CLAMP((float)Y+1.772*(float)(Cb-128.0));
            bmp->r[i] = (unsigned char)R;
            bmp->g[i] = (unsigned char)G;
            bmp->b[i] = (unsigned char)B;
        }
}

void bmp_free(BITMAP *src) {
    free(src->r);
    free(src->g);
    free(src->b);
}

void bin_yuv444_read (char *file1, BITMAP* bmp) {
    FILE *fp;
    int coeff_index;
    unsigned int value;

    if ((fp = fopen(file1,"rb")) == NULL) {
        printf("Error opening file %s.\n",file1);
        exit(1);
    }

    //VUY0
    for(coeff_index=0; coeff_index < bmp->height * bmp->width; coeff_index++) {
        if (fread(&value, 1, 1, fp) == 1) {
            bmp->b[coeff_index] = value;
        }
        if (fread(&value, 1, 1, fp) == 1) {
            bmp->g[coeff_index] = value;
        }
        if (fread(&value, 1, 1, fp) == 1) {
            bmp->r[coeff_index] = value;
        }
        if (fread(&value, 1, 1, fp) == 1) { // for dummy byte
        }
    }

    fclose(fp);
}

void hex_read(char *file, BITMAP* bmp) {
    FILE *fp;
    int coeff_index;
    int value;

    if ((fp = fopen(file,"rb")) == NULL) {
        printf("Error opening file %s.\n",file);
        exit(1);
    }

    for(coeff_index=0; coeff_index < bmp->height * bmp->width ; coeff_index++) {
        if (EOF != fscanf(fp, "%x", &value)) {
            bmp->r[coeff_index] = (value>>16) & 0xff;
            bmp->g[coeff_index] = (value>> 8) & 0xff;
            bmp->b[coeff_index] = (value>> 0) & 0xff;
        }

    }

    fclose(fp);
}

void hex_write(char *file, BITMAP* bmp) {
    FILE *fp;
    int coeff_index;

    if ((fp = fopen(file,"w")) == NULL) {
        printf("Error opening file %s.\n",file);
        exit(1);
    }

    for(coeff_index=0; coeff_index < bmp->height * bmp->width ; coeff_index++) {
        fprintf(fp, "%02x%02x%02x\n", bmp->r[coeff_index], bmp->g[coeff_index], bmp->b[coeff_index]);
    }

    fclose(fp);
}

// horizontal padding y to 16's multiples, u v to 8's multiples
// yuv422: 1 => 422, 0 => 420, drop pixel, drop line
void hex_yuv_padx16_write(char *file, BITMAP* bmp, int yuv422) {
    FILE *fp;
    int num_pad;
    int i_inc;

    int i, j, k;

    if ((fp = fopen(file,"w")) == NULL) {
        printf("Error opening file %s.\n",file);
        exit(1);
    }

    num_pad = (16 - bmp->width%16) & 0xf; // 0 ~ 15
    i_inc = yuv422 ? 1 : 2;

    for (i=0; i<bmp->height; i++)
        for (j=0; j<bmp->width; j++) {
            fprintf(fp, "%02x\n", bmp->r[i*bmp->width+j]);
            if(j==bmp->width-1)
                for(k=0; k<num_pad; k++)
                    fprintf(fp, "%02x\n", bmp->r[i*bmp->width+j]);
        }

    for (i=0; i<bmp->height; i+=i_inc)
        for (j=0; j<bmp->width; j+=2) {
            fprintf(fp, "%02x\n", bmp->g[i*bmp->width+j]);
            if(j==bmp->width-1 || j==bmp->width-2)
                for(k=0; k<num_pad/2; k++)
                    fprintf(fp, "%02x\n", bmp->g[i*bmp->width+j]);
        }

    for (i=0; i<bmp->height; i+=i_inc)
        for (j=0; j<bmp->width; j+=2) {
            fprintf(fp, "%02x\n", bmp->b[i*bmp->width+j]);
            if(j==bmp->width-1 || j==bmp->width-2)
                for(k=0; k<num_pad/2; k++)
                    fprintf(fp, "%02x\n", bmp->b[i*bmp->width+j]);
        }

    fclose(fp);
}

void hex_rgb565_write(char *file, BITMAP* bmp) {
    FILE *fp;

    int i, j;
    int r, g, b;
    int y, u, v;
    int value;

    if ((fp = fopen(file,"w")) == NULL) {
        printf("Error opening file %s.\n",file);
        exit(1);
    }

    for (i=0; i<bmp->height; i++) {
        for (j=0; j<bmp->width; j++) {
            y = bmp->r[i*bmp->width+j];
            u = bmp->g[i*bmp->width+j]-128;
            v = bmp->b[i*bmp->width+j]-128;

            r = ROUND_CLAMP((32*y + 45*v)/32.0);
            g = ROUND_CLAMP((32*y - 11*u - 23*v)/32.0);
            b = ROUND_CLAMP((32*y + 57*u)/32.0);

            value = ((r>>3)<<11) + ((g>>2)<<5) + (b>>3);
            fprintf(fp, "%04x\n", value);
        }
    }
    fclose(fp);
}

int lfsr20(int cnt)
{
    int flag;
    int res;
    // 20-bit LFSR, tap = [1,8,19]
    flag = (((cnt&(1<<2))>>2) + ((cnt&(1<<19))>>19)) % 2;
    res = ((cnt<<1)&1048575);    // AND with 20'hfffff
    res += flag;
    return res;
}

void bmp_plusrand(BITMAP* bmp, int seed, int strength) {
    int seed1;
    int offset = 0;
    int msb4;
    int coeff_index;

    seed1 = seed;

    for (coeff_index=0; coeff_index < bmp->height * bmp->width ; coeff_index++) {
        seed1 = lfsr20(seed1);
        msb4 = seed1 >> 16;

        switch (msb4) {
            case 0: offset = -4;
                    break;
            case 1: offset = -3;
                    break;
            case 2: offset = -3;
                    break;
            case 3: offset = -2;
                    break;
            case 4: offset = -2;
                    break;
            case 5: offset = -1;
                    break;
            case 6: offset = -1;
                    break;
            case 7: offset = 0;
                    break;
            case 8: offset = 0;
                    break;
            case 9: offset = 1;
                    break;
            case 10: offset = 1;
                     break;
            case 11: offset = 2;
                     break;
            case 12: offset = 2;
                     break;
            case 13: offset = 3;
                     break;
            case 14: offset = 3;
                     break;
            case 15: offset = 4;
                     break;
        }

        offset *= strength / 8;

        bmp->r[coeff_index] += offset;

    }

}

void bmp_iir(BITMAP* bmp, double par) {
    int i, j;
    double temp_r, temp_g, temp_b;

    for (i = 0; i<bmp->height; i++) { // horizontal iir
        temp_r = bmp->r[i*bmp->width];
        temp_g = bmp->g[i*bmp->width];
        temp_b = bmp->b[i*bmp->width];

        for (j = 1; j<bmp->width; j++) {
            temp_r = ((double)bmp->r[i*bmp->width+j] - par*temp_r)/ (1.0 - par);
            temp_g = ((double)bmp->g[i*bmp->width+j] - par*temp_g)/ (1.0 - par);
            temp_b = ((double)bmp->b[i*bmp->width+j] - par*temp_b)/ (1.0 - par);

            bmp->r[i*bmp->width+j] = (unsigned char) ROUND_CLAMP(temp_r);
            bmp->g[i*bmp->width+j] = (unsigned char) ROUND_CLAMP(temp_g);
            bmp->b[i*bmp->width+j] = (unsigned char) ROUND_CLAMP(temp_b);
        }
    }

    /*
       for (j = 0; j<bmp->width; j++) { // vertical iir
       temp_r = bmp->r[j];
       temp_g = bmp->g[j];
       temp_b = bmp->b[j];

       for (i = 1; i<bmp->height; i++) {
       temp_r = ((double)bmp->r[i*bmp->width+j] + par*temp_r)/ (1.0 - par);
       temp_g = ((double)bmp->g[i*bmp->width+j] + par*temp_g)/ (1.0 - par);
       temp_b = ((double)bmp->b[i*bmp->width+j] + par*temp_b)/ (1.0 - par);

       bmp->r[i*bmp->width+j] = (unsigned char) ROUND_CLAMP(temp_r);
       bmp->g[i*bmp->width+j] = (unsigned char) ROUND_CLAMP(temp_g);
       bmp->b[i*bmp->width+j] = (unsigned char) ROUND_CLAMP(temp_b);
       }
       }*/

}
/*
   double matrix_get2(MATRIX *ma, int x, int y) {
   return ma->value[y* ma->width +x];
   }

   PIXEL bmp_get2(BITMAP *bmp,int x, int y) {
   PIXEL temp;

//padding with boundary
if (x>=bmp->width)
x=bmp->width-1;
else if (x<0)
x=0;

if (y>bmp->height)
y=bmp->height-1;
else if (y<0)
y=0;

temp.r=bmp->r[y * bmp->width +x];
temp.g=bmp->g[y * bmp->width +x];
temp.b=bmp->b[y * bmp->width +x];

return temp;
}

BITMAP bmp_conv2(BITMAP *src, MATRIX *filt) {
BITMAP des;
int i, j, k, l;
double r_value, g_value, b_value;
double px_filt;
PIXEL px_src;

bmp_eq(&des, src);

for (i=0; i<src->height; i++) {
for (j=0; j<src->width; j++) {
r_value=0;
g_value=0;
b_value=0;
for (k=(-(filt->height-1) / 2); k<= ((filt->height-1) / 2); k++) {
for (l=(-(filt->width-1) / 2); l<= ((filt->width-1) / 2); l++)  {
px_src = bmp_get2(src, j+l, i+k);
px_filt = matrix_get2(filt, ((filt->width-1)/2 - l), ((filt->height-1)/2 - k));

r_value += (double)px_src.r * px_filt;
g_value += (double)px_src.g * px_filt;
b_value += (double)px_src.b * px_filt;
}
}
des.r[i * src->width + j]=CLAMP(r_value);
des.g[i * src->width + j]=CLAMP(g_value);
des.b[i * src->width + j]=CLAMP(b_value);
}
}
return des;
}
 */
