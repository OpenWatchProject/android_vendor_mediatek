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
#include "crz.h"
#define BITWIDTH 8
#define TNUM 20 // Coeff Table number

//#define DEBUG_COEFF
//#define DEBUG_CSEL
//#define ADAPTIVE
#define BMPOUT

//#define MT6235 //different output format

//#define PRZ //different algorithm

//#define SAMENAMING //no table indicator on output filename
//#define SRCDIFF_NAMING
//#define BIN_HEX //for SA, different input hex format

#ifdef MT6235
#define SRAM_MAX_ADDR 800
#else
#define SRAM_MAX_ADDR 1024
#endif

void CRZ::crz (BITMAP* bmp_in, BITMAP* bmp_out, int USEL, int DSEL, int ECV) {
    BITMAP bmp_temp;
    int pvr_bypass;
    int pvr_mode;
    int phr_bypass;
    int phr_mode;

    double table_sel_u, table_sel_d;

    int hsize_for_check;

    /*
       double cubic_bspline[20] = {-1.0, -21.0, -41.0, -61.0, -0.75, -20.75, -40.75, -60.75,
       -0.5, -20.5, -40.5, -60.5, -1.5, -21.5, -2.0, -22.0,
       -2.6, -22.6, -3.3, 1.0};
     */
    /*
       double cubic_bspline[20] = {1.0,    -60.5,    -60.75,    -61.0,
       -40.5,    -40.75,    -41.0,    -20.5,
       -20.75,    -21.0,    -0.5,    -0.75,
       -21.5,    -1.0,    -22.0,    -1.5,
       -22.6,    -2.0,    -2.6,    -3.3};
     */

    double cubic_bspline[20] = {99.0,    1.0,
        -60.5,
        -40.5,    -40.75,
        -20.5,    -20.75,    -0.5,    -20.80,
        -0.75,    -0.80,
        -21.0,    -1.0,
        -21.5,    -22.0,
        -1.5,    -22.6,
        -2.0,    -2.6,    -3.3};

    table_sel_u = cubic_bspline[USEL];
    table_sel_d = cubic_bspline[DSEL];

    phr_bypass = bmp_in->width == bmp_out->width;
    if (bmp_out->width >= bmp_in->width)
        phr_mode = 1;
    else
        phr_mode = 0;

    hsize_for_check = phr_mode ? bmp_in->width : bmp_out->width;

    pvr_bypass = bmp_in->height == bmp_out->height;
    if (bmp_out->height == 1)
        pvr_mode = ECV ? 0 : 1;
    else if (bmp_out->height > bmp_in->height)
        pvr_mode = 2;
    else if ((hsize_for_check <= SRAM_MAX_ADDR && ((bmp_in->height-1)/(bmp_out->height -1) <= 64)) && !ECV)
        pvr_mode = 1;
    else
        pvr_mode = 0;

    if (phr_mode==1) { // v => h
        //        bmp_444_422_444(bmp_in);
        bmp_init (&bmp_temp, bmp_in->width, bmp_out->height, 1);

        if (pvr_mode==2) { // vu => hu
            up_cubic_bspline_1d(bmp_in, &bmp_temp, table_sel_u);
            //hex_write ("test.yuv", &bmp_temp);
            if (!phr_bypass)
                up_cubic_bspline_1d(&bmp_temp, bmp_out, table_sel_u);
            else
                bmp_copy(bmp_out, &bmp_temp);
        }
        else {  // vd => hu
            if (!pvr_bypass) {
                if (pvr_mode == 0)
                    down_ec_v(bmp_in, &bmp_temp);
                else // pvr_mode == 1
                    down_y_cbc_uv_tri_1d(bmp_in, &bmp_temp, table_sel_d);
            }
            else
                bmp_copy(&bmp_temp, bmp_in);

            if (!phr_bypass)
                up_cubic_bspline_1d(&bmp_temp, bmp_out, table_sel_u);
            else
                bmp_copy(bmp_out, &bmp_temp);
        }
    }
    else { // h => v
        bmp_init (&bmp_temp, bmp_out->width, bmp_in->height, 1);

        if (pvr_mode==2) { // hd => vu
            down_y_cbc_uv_tri_1d(bmp_in, &bmp_temp, table_sel_d);
            //        bmp_444_422_444(&bmp_temp);
            up_cubic_bspline_1d(&bmp_temp, bmp_out, table_sel_u);
        }
        else { // hd => vd
            down_y_cbc_uv_tri_1d(bmp_in, &bmp_temp, table_sel_d);
            //        bmp_444_422_444(&bmp_temp);
            if (!pvr_bypass) {
                if (pvr_mode == 0)
                    down_ec_v(&bmp_temp, bmp_out);
                else // pvr_mode == 1
                    down_y_cbc_uv_tri_1d(&bmp_temp, bmp_out, table_sel_d);
            }
            else
                bmp_copy(bmp_out, &bmp_temp);

        }
    }

    //bmp_plusrand(bmp_out, 50, 8);

    bmp_free(&bmp_temp);
}

void CRZ::down_ec_v (BITMAP *src_bmp, BITMAP *tar_bmp) {
    int x, y;
    int k = 0;

    int M_Y = src_bmp->height;
    int N_Y = tar_bmp->height;

    int C_Y, C_Y2;
    int C_Y_for_mult, C_Y2_for_mult;
    int C_Acc_Y = 0;
    int C_Acc_Y_for_mult = 0;

    int C_UV_for_mult, C_UV2_for_mult;
    int C_Acc_UV_for_mult = 0;

    int output_en;

    int *MAC_Y;
    int *MAC_U;
    int *MAC_V;

    int in_Y, in_U, in_V;

    int src_width = src_bmp->width;
    int src_height = src_bmp->height;

    // line buffer init
    if ((MAC_Y=(int *)calloc( src_width, sizeof(int) )) == NULL ||
            (MAC_U=(int *)calloc( src_width, sizeof(int) )) == NULL ||
            (MAC_V=(int *)calloc( src_width, sizeof(int) )) == NULL) {
        printf("Error allocating memory \n");
        exit(1);
    }

    for (y = 0; y < src_height; y++) {

        output_en = (M_Y - C_Acc_Y) <= N_Y;
        if (output_en) {
            C_Y  = M_Y - C_Acc_Y;
            C_Y2 = N_Y - C_Y;
        }
        else {
            C_Y  = N_Y;
            C_Y2 = 0;
        }

        // multiplier bitwidth: 9 bits, so C should be limited to 511
        if (N_Y >= 2048) {
            C_Y_for_mult  = C_Y  >> 3;
            C_Y2_for_mult = C_Y2 >> 3;
        }
        else if (N_Y >= 1024) {
            C_Y_for_mult  = C_Y  >> 2;
            C_Y2_for_mult = C_Y2 >> 2;
        }
        else if (N_Y >= 512) {
            C_Y_for_mult  = C_Y  >> 1;
            C_Y2_for_mult = C_Y2 >> 1;
        }
        else {
            C_Y_for_mult  = C_Y;
            C_Y2_for_mult = C_Y2;
        }

        C_UV_for_mult  = C_Y_for_mult;
        C_UV2_for_mult = C_Y2_for_mult;

        C_Acc_Y += C_Y;
        C_Acc_Y_for_mult  += C_Y_for_mult;
        C_Acc_UV_for_mult += C_UV_for_mult;

        for (x = 0; x < src_width; x++) {
            in_Y = src_bmp->r[y * src_width + x];
            in_U = src_bmp->g[y * src_width + x];
            in_V = src_bmp->b[y * src_width + x];

            if (output_en) {
                tar_bmp->r[k]   = ROUND_CLAMP((double)(MAC_Y[x] + in_Y * C_Y_for_mult) / C_Acc_Y_for_mult);
                tar_bmp->g[k]   = ROUND_CLAMP((double)(MAC_U[x] + in_U * C_UV_for_mult) / C_Acc_UV_for_mult);
                tar_bmp->b[k++] = ROUND_CLAMP((double)(MAC_V[x] + in_V * C_UV_for_mult) / C_Acc_UV_for_mult);


                MAC_Y[x] = in_Y * C_Y2_for_mult;
                MAC_U[x] = in_U * C_UV2_for_mult;
                MAC_V[x] = in_V * C_UV2_for_mult;
            }
            else {
                MAC_Y[x] = MAC_Y[x] + in_Y * C_Y_for_mult;
                MAC_U[x] = MAC_U[x] + in_U * C_UV_for_mult;
                MAC_V[x] = MAC_V[x] + in_V * C_UV_for_mult;
            }

        } // horizotal done

        if (output_en) {
            C_Acc_Y = C_Y2;
            C_Acc_Y_for_mult  = C_Y2_for_mult;
            C_Acc_UV_for_mult = C_UV2_for_mult;
        }

    }  // vertical done

    free(MAC_Y);
    free(MAC_U);
    free(MAC_V);
}

void CRZ::down_y_cbc_uv_tri_1d (BITMAP *src_bmp, BITMAP *tar_bmp, double cubic_or_bspline_) {

    int coeff_index;
    int coeff_index_scaled;
    int coeff_index_approx;
    int coeff_index_approx_scaled;
    unsigned int coeff_step;
    float coeff_index_temp;

    int i, j;
    int k = 0;
    int i_max, j_max, k_inc;
    int skip_1st;

    int *coeff0, *coeff1, *coeff2, *coeff3;
    int coeff_rs;
    int c0, c1, c2, c3;
    int uv_c1, uv_c2;

    int mode_down;

    int src_width, src_height;
    int tar_width, tar_height;
    int M, N;
    int M_m1, N_m1;
    int HorV_;
    int pixel_count = 0;

    // parameters
    unsigned int MAX_NM = 64;
    unsigned int PREC = 1<<14;

    src_width  = src_bmp->width;
    src_height = src_bmp->height;
    tar_width  = tar_bmp->width;
    tar_height = tar_bmp->height;

    HorV_ = (src_width == tar_width) ? 0 : (src_height == tar_height) ? 1 : -1;
    if (HorV_ == -1) {
        printf("Only 1D !\n");
        return;
    }

    if (((coeff0=(int *)calloc(MAX_NM, sizeof(int) )) == NULL) ||
            ((coeff1=(int *)calloc(MAX_NM, sizeof(int) )) == NULL) ||
            ((coeff2=(int *)calloc(MAX_NM, sizeof(int) )) == NULL) ||
            ((coeff3=(int *)calloc(MAX_NM, sizeof(int) )) == NULL) ) {
        printf("Error allocating coeff memory \n");
        exit(1);
    }

    skip_1st = 0;

    mode_down = HorV_ ? src_width > tar_width : src_height > tar_height;
    M = HorV_ ? src_width : src_height;
    N = HorV_ ? tar_width : tar_height;
    M_m1 = M-1;
    N_m1 = N-1;

    // rounding by + M_m1/2
    coeff_step = (int)((N_m1*MAX_NM*PREC+(M_m1>>1))/M_m1);
    // test

    if(mode_down) { //downsample
        int mac0, mac1, mac2, mac3;
        int div0, div1, div2, div3;
        int u_mac0, u_mac1, u_mac2;
        int v_mac0, v_mac1, v_mac2;
        int uv_div0, uv_div1, uv_div2;

        coeff_cubic_bspline (coeff0, coeff1, coeff2, coeff3, MAX_NM, cubic_or_bspline_);

        if (HorV_) {
            if (M_m1 <= 64*N_m1)
                coeff_rs = 0;
            else if (M_m1 <= 128*N_m1)
                coeff_rs = 1;
            else if (M_m1 <= 256*N_m1)
                coeff_rs = 2;
            else if (M_m1 <= 512*N_m1)
                coeff_rs = 3;
            else if (M_m1 <= 1024*N_m1)
                coeff_rs = 4;
            else // test
                coeff_rs = 4;
        }
        else
            coeff_rs = 0;


        if (HorV_) {
            i_max = src_height;
            j_max = src_width;
            k_inc = 1;
        }
        else {
            i_max = src_width;
            j_max = src_height;
            k_inc = src_width;
        }

        for(i=0; i<i_max; i++) {
            div0 = 0;
            div1 = 0;
            div2 = 0;
            div3 = 0;

            mac0 = 0;
            mac1 = 0;
            mac2 = 0;
            mac3 = 0;

            uv_div0 = 0;
            uv_div1 = 0;
            uv_div2 = 0;

            u_mac0 = 0;
            u_mac1 = 0;
            u_mac2 = 0;

            v_mac0 = 0;
            v_mac1 = 0;
            v_mac2 = 0;

            coeff_index = M_m1;
            coeff_index_approx = MAX_NM * PREC;

            if (!HorV_)
                k = i;

            skip_1st = 0;

            for(j=0; j<j_max; j++) {
                int in_y, in_u, in_v;

                if (HorV_) {
                    in_y = src_bmp->r[i*src_width+j];
                    in_u = src_bmp->g[i*src_width+j];
                    in_v = src_bmp->b[i*src_width+j];
                }
                else {
                    in_y = src_bmp->r[j*src_width+i];
                    in_u = src_bmp->g[j*src_width+i];
                    in_v = src_bmp->b[j*src_width+i];
                }

                //---------- coefficient selection -------------------------
                coeff_index_temp = (float)(coeff_index * MAX_NM) / (float)(M_m1);
                coeff_index_scaled = (int)(coeff_index_temp + 0.5);
                coeff_index_approx_scaled = (int)(((float)(coeff_index_approx) / (float)(PREC)) + 0.5);

                // debugging only
#if 1
                if (abs(coeff_index_scaled - coeff_index_approx_scaled) > 1) {
                    printf("coeff_index_erorr\n");
                    //exit(0);
                }
#endif

                // use approximating accumulator to prevent using divider for coeff_index
                coeff_index_scaled = coeff_index_approx_scaled;

                // test
                if(coeff_index_scaled ==0) {
                    // if(coeff_index > N_m1 && coeff_index_scaled ==0) {

                    c0 = coeff3[MAX_NM-1] >> coeff_rs;
                    c1 = coeff2[MAX_NM-1] >> coeff_rs;
                    c2 = coeff1[MAX_NM-1] >> coeff_rs;
                    c3 = coeff0[MAX_NM-1] >> coeff_rs;
                }
                else {
                    c0 = coeff0[coeff_index_scaled-1] >> coeff_rs;
                    c1 = coeff1[coeff_index_scaled-1] >> coeff_rs;
                    c2 = coeff2[coeff_index_scaled-1] >> coeff_rs;
                    c3 = coeff3[coeff_index_scaled-1] >> coeff_rs;
                }

                // uv: linear
                uv_c1 = coeff_index_scaled >> coeff_rs;
                uv_c2 = (MAX_NM - coeff_index_scaled) >> coeff_rs;

                //---------- multiply and accumulate -------------------------
                mac0 += c0 * in_y;
                mac1 += c1 * in_y;
                mac2 += c2 * in_y;
                mac3 += c3 * in_y;

                div0 += c0;
                div1 += c1;
                div2 += c2;
                div3 += c3;

                u_mac1 += uv_c1 * in_u;
                u_mac2 += uv_c2 * in_u;
                v_mac1 += uv_c1 * in_v;
                v_mac2 += uv_c2 * in_v;

                uv_div1 += uv_c1;
                uv_div2 += uv_c2;

                //----------- output -----------------------------------------
                if(coeff_index <= N_m1 || j == j_max-1) {

                    if (skip_1st == 1 ||  j == j_max-1) {
                        //test
                        //printf("%4d, div:%3d %3d %3d, c:%d %d\n", j, div0, div1, div2, c1, c2);
                        if(div0 != 0 && uv_div0 != 0 && div1 != 0 && uv_div1 != 0) {
                            if(N != 1) {
                                if (div0 != 0 && uv_div0 != 0) {
                                    tar_bmp->r[k] = ROUND_CLAMP((double)mac0 / div0);
                                    tar_bmp->g[k] = ROUND_CLAMP((double)u_mac0 / uv_div0);
                                    tar_bmp->b[k] = ROUND_CLAMP((double)v_mac0 / uv_div0);
                                }
                                if (div0 == 0 || uv_div0 == 0)
                                    printf("Divide by zero!\n");
                            }
                            else {
                                if (div1 != 0 && uv_div1 != 0) {
                                    tar_bmp->r[k] = ROUND_CLAMP((double)mac1 / div1);
                                    tar_bmp->g[k] = ROUND_CLAMP((double)u_mac1 / uv_div1);
                                    tar_bmp->b[k] = ROUND_CLAMP((double)v_mac1 / uv_div1);
                                }
                                if (div1 == 0 || uv_div1 == 0)
                                    printf("Divide by zero!\n");
                            }
                        }


                        //if(pixel_count == 2301)
                        //    k= k;

                        k = k + k_inc;
                        pixel_count ++;
                    }

                    if (skip_1st == 0) {
                        skip_1st = 1;
                    }


                    mac0 = mac1;
                    div0 = div1;
                    mac1 = mac2;
                    div1 = div2;
                    mac2 = mac3;
                    div2 = div3;
                    mac3 = 0;
                    div3 = 0;

                    u_mac0  = u_mac1;
                    v_mac0  = v_mac1;
                    uv_div0 = uv_div1;
                    u_mac1  = u_mac2;
                    v_mac1  = v_mac2;
                    uv_div1 = uv_div2;
                    u_mac2  = 0;
                    v_mac2  = 0;
                    uv_div2 = 0;

                }

                //----------- Update index -----------------------------------------
                coeff_index_approx -= coeff_step;
                coeff_index -= N_m1;

                if (coeff_index == 0)
                    coeff_index_approx = MAX_NM * PREC;
                else if(coeff_index_approx < 0)
                    coeff_index_approx += MAX_NM * PREC;

                if(coeff_index <= 0)
                    coeff_index += M_m1;

                // test
                // printf("coeff_index_approx = %x\n", coeff_index_approx);


                }  // j loop

                // force output the last pixel
                if (N >= 2) {
                    // test
                    // printf("%d %d\n", j, div0);

                    tar_bmp->r[k] = ROUND_CLAMP((double)mac0 / div0);
                    tar_bmp->g[k] = ROUND_CLAMP((double)u_mac0 / uv_div0);
                    tar_bmp->b[k] = ROUND_CLAMP((double)v_mac0 / uv_div0);

                    k = k + k_inc;
                }

                // checking
                if (HorV_) {
                    if (k % N != 0)
                        printf("Horizontal Error occurs!!!!!!!!!!!\n");
                }
                else {
                    if (k - tar_width*tar_height != i)
                        printf("Vertical Error occurs!!!!!!!!!!!\n");
                }

            }  // i loop

        } //downsample
        free(coeff0);
        free(coeff1);
        free(coeff2);
        free(coeff3);
    }

    void CRZ::up_cubic_bspline_1d (BITMAP *src_bmp, BITMAP *tar_bmp,
            double cubic_or_bspline_) {

        int coeff_index;
        int i, j;
        int k = 0;
        int i_max, j_max, k_inc;
        int residual_max;

        int src_width, src_height;
        int tar_width, tar_height;
        unsigned int src_width_for_ratio, src_height_for_ratio;
        unsigned int tar_width_for_ratio, tar_height_for_ratio;

        int in0_y, in1_y, in2_y, in3_y;
        int in0_u, in1_u, in2_u, in3_u;
        int in0_v, in1_v, in2_v, in3_v;
        int in0_index, in1_index, in2_index, in3_index;
        int mac_y, mac_u, mac_v;

        unsigned int ratio, residual;
        int temp_ratio, temp_residual;
        int tcoeff0[64], tcoeff1[64], tcoeff2[64], tcoeff3[64];
        int ucoeff0[64], ucoeff1[64], ucoeff2[64], ucoeff3[64];
        int c05coeff0[64], c05coeff1[64], c05coeff2[64], c05coeff3[64];
        int c00coeff0[64], c00coeff1[64], c00coeff2[64], c00coeff3[64];

        int y_c0, y_c1, y_c2, y_c3;
        int c_c0, c_c1, c_c2, c_c3;
#ifdef ADAPTIVE
        int diff0, diff1, diff2;
#endif
        int y_c_sel;
#ifdef DEBUG_CSEL
        int y_c_sel_0_count=0, y_c_sel_1_count=0, y_c_sel_2_count=0;
#endif
        int HorV_;
        int pixel_count= 0;

        src_width = src_bmp->width;
        src_height = src_bmp->height;
        tar_width = tar_bmp->width;
        tar_height = tar_bmp->height;

        src_width_for_ratio = src_width - 1;
        src_height_for_ratio = src_height - 1;
        tar_width_for_ratio = tar_width - 1;
        tar_height_for_ratio = tar_height - 1;

        HorV_ = (src_width == tar_width) ? 0 : (src_height == tar_height) ? 1 : -1;
        if (HorV_ == -1) {
            printf("Only 1D !\n");
            return;
        }

        coeff_cubic_bspline (tcoeff3, tcoeff2, tcoeff1, tcoeff0, 64, cubic_or_bspline_);
        coeff_cubic_bspline (c05coeff3, c05coeff2, c05coeff1, c05coeff0, 64, -0.5);
        coeff_cubic_bspline (c00coeff3, c00coeff2, c00coeff1, c00coeff0, 64, 0.0);


        if (HorV_) {
            ratio = (unsigned int) ((src_width_for_ratio <<20) + (tar_width_for_ratio>>1)) / tar_width_for_ratio;
            residual = src_width_for_ratio % tar_width_for_ratio;

            i_max = src_height;
            j_max = tar_width;
            k_inc = 1;
            residual_max = j_max -1;
            coeff_cubic_bspline (ucoeff3, ucoeff2, ucoeff1, ucoeff0, 64, 1.0);
        }
        else {
            ratio = (unsigned int) ((src_height_for_ratio <<20) + (tar_height_for_ratio>>1)) / tar_height_for_ratio;
            residual = src_height_for_ratio % tar_height_for_ratio;

            i_max = src_width;
            j_max = tar_height;
            k_inc = src_width;
            residual_max = j_max -1;
            coeff_cubic_bspline (ucoeff3, ucoeff2, ucoeff1, ucoeff0, 64, 1.0);
        }

        for(i=0; i<i_max; i++) {
            int ref;
            int mantessa;

            if (!HorV_)
                k = i;

            temp_ratio = 0;
            temp_residual = 0;

            for(j=0; j<j_max; j++) {
                // Position and coeff_index calculation
                ref = (temp_ratio & 0xfff00000) >> 20;  // MSB 12 bits
                coeff_index = (temp_ratio & 0x000fc000) >> 14;  // 6 bits

                // determine effect of residual
                mantessa = (temp_ratio & 0xfffff);
                if (temp_residual== residual_max && mantessa !=0)  // need rounding
                {
                    // ref = ref + 1;
                    ref = ref + ((temp_ratio & 0x00080000) >> 19);
                    coeff_index = 0;

                    // update parameters
                    temp_ratio = (ref << 20) + ratio;
                    temp_residual = residual;
                }
                else if (temp_residual==residual_max)  // no need rounding
                {
                    temp_ratio += ratio;
                    temp_residual = residual;
                }
                else
                {
                    // update parameters
                    temp_ratio += ratio;
                    if (residual_max != 0) {
                        if (((temp_residual + residual) % residual_max)==0)
                            // next pixel may need rounding
                            temp_residual += residual;
                        else
                            temp_residual  = (temp_residual+residual) % residual_max;
                    }

                }
                // end: Position and coeff_index calculation

                in0_index = ref-1;
                in1_index = ref;
                in2_index = ref+1;
                in3_index = ref+2;

                if (HorV_) {
                    in0_index = (in0_index < 0) ? 0 : (in0_index >= src_width) ? (src_width - 1) : in0_index;
                    in1_index = (in1_index < 0) ? 0 : (in1_index >= src_width) ? (src_width - 1) : in1_index;
                    in2_index = (in2_index < 0) ? 0 : (in2_index >= src_width) ? (src_width - 1) : in2_index;
                    in3_index = (in3_index < 0) ? 0 : (in3_index >= src_width) ? (src_width - 1) : in3_index;

                    in0_y = src_bmp->r[i*src_width+in0_index];
                    in1_y = src_bmp->r[i*src_width+in1_index];
                    in2_y = src_bmp->r[i*src_width+in2_index];
                    in3_y = src_bmp->r[i*src_width+in3_index];
                    in0_u = src_bmp->g[i*src_width+in0_index];
                    in1_u = src_bmp->g[i*src_width+in1_index];
                    in2_u = src_bmp->g[i*src_width+in2_index];
                    in3_u = src_bmp->g[i*src_width+in3_index];
                    in0_v = src_bmp->b[i*src_width+in0_index];
                    in1_v = src_bmp->b[i*src_width+in1_index];
                    in2_v = src_bmp->b[i*src_width+in2_index];
                    in3_v = src_bmp->b[i*src_width+in3_index];

                }
                else {
                    in0_index = (in0_index < 0) ? 0 : (in0_index >= src_height) ? (src_height - 1) : in0_index;
                    in1_index = (in1_index < 0) ? 0 : (in1_index >= src_height) ? (src_height - 1) : in1_index;
                    in2_index = (in2_index < 0) ? 0 : (in2_index >= src_height) ? (src_height - 1) : in2_index;
                    in3_index = (in3_index < 0) ? 0 : (in3_index >= src_height) ? (src_height - 1) : in3_index;

                    in0_y = src_bmp->r[in0_index*src_width+i];
                    in1_y = src_bmp->r[in1_index*src_width+i];
                    in2_y = src_bmp->r[in2_index*src_width+i];
                    in3_y = src_bmp->r[in3_index*src_width+i];
                    in0_u = src_bmp->g[in0_index*src_width+i];
                    in1_u = src_bmp->g[in1_index*src_width+i];
                    in2_u = src_bmp->g[in2_index*src_width+i];
                    in3_u = src_bmp->g[in3_index*src_width+i];
                    in0_v = src_bmp->b[in0_index*src_width+i];
                    in1_v = src_bmp->b[in1_index*src_width+i];
                    in2_v = src_bmp->b[in2_index*src_width+i];
                    in3_v = src_bmp->b[in3_index*src_width+i];
                }

#ifdef ADAPTIVE
                diff0 = (in1_y - in0_y);
                diff1 = (in2_y - in1_y);
                diff2 = (in3_y - in2_y);
                if ((diff2 > 0 && diff0 > 0) || (diff2 < 0 && diff0 < 0)) {
                    if (abs(diff1) <= 3) {
                        y_c_sel_1_count++;
                        y_c_sel = 1;  // c0
                    }
                    else if ((diff1 > 0 && diff0 > 0) || (diff1 < 0 && diff0 < 0)) {
                        y_c_sel_2_count++;
                        y_c_sel = 2; // c05
                    }
                    else {
                        y_c_sel_0_count ++;
                        y_c_sel = 0; // orig
                    }
                }
                else if ((diff1 > 0 && diff0 > 0) || (diff1 < 0 && diff0 < 0)) {
                    y_c_sel_2_count ++;
                    y_c_sel = 2; // c05
                }
                else {
                    y_c_sel_0_count++;
                    y_c_sel = 0; // orig
                }
#else
                y_c_sel = 0;
#endif

                if(coeff_index ==0) {
                    switch (y_c_sel) {
                        case 0: // orig
                            y_c0 = tcoeff3[64-1];
                            y_c1 = tcoeff2[64-1];
                            y_c2 = tcoeff1[64-1];
                            y_c3 = tcoeff0[64-1];
                            break;
                        case 1: // c0
                            y_c0 = c00coeff3[64-1];
                            y_c1 = c00coeff2[64-1];
                            y_c2 = c00coeff1[64-1];
                            y_c3 = c00coeff0[64-1];
                            break;
                        case 2: // c05
                            y_c0 = c05coeff3[64-1];
                            y_c1 = c05coeff2[64-1];
                            y_c2 = c05coeff1[64-1];
                            y_c3 = c05coeff0[64-1];
                            break;
                    }
                    c_c0 = ucoeff3[64-1];
                    c_c1 = ucoeff2[64-1];
                    c_c2 = ucoeff1[64-1];
                    c_c3 = ucoeff0[64-1];
                }
                else {
                    switch (y_c_sel) {
                        case 0:
                            y_c0 = tcoeff0[coeff_index-1];
                            y_c1 = tcoeff1[coeff_index-1];
                            y_c2 = tcoeff2[coeff_index-1];
                            y_c3 = tcoeff3[coeff_index-1];
                            break;
                        case 1: // c0
                            y_c0 = c00coeff0[coeff_index-1];
                            y_c1 = c00coeff1[coeff_index-1];
                            y_c2 = c00coeff2[coeff_index-1];
                            y_c3 = c00coeff3[coeff_index-1];
                            break;
                        case 2: // c05
                            y_c0 = c05coeff0[coeff_index-1];
                            y_c1 = c05coeff1[coeff_index-1];
                            y_c2 = c05coeff2[coeff_index-1];
                            y_c3 = c05coeff3[coeff_index-1];
                            break;
                    }
                    c_c0 = ucoeff0[coeff_index-1];
                    c_c1 = ucoeff1[coeff_index-1];
                    c_c2 = ucoeff2[coeff_index-1];
                    c_c3 = ucoeff3[coeff_index-1];
                }

                mac_y = y_c0 * in0_y +
                    y_c1 * in1_y +
                    y_c2 * in2_y +
                    y_c3 * in3_y + (1<<(BITWIDTH-1));

                mac_u = c_c0 * in0_u +
                    c_c1 * in1_u +
                    c_c2 * in2_u +
                    c_c3 * in3_u + (1<<(BITWIDTH-1));

                mac_v = c_c0 * in0_v +
                    c_c1 * in1_v +
                    c_c2 * in2_v +
                    c_c3 * in3_v + (1<<(BITWIDTH-1));

                tar_bmp->r[k] = CLAMP((mac_y >> BITWIDTH));
                tar_bmp->g[k] = CLAMP((mac_u >> BITWIDTH));
                tar_bmp->b[k] = CLAMP((mac_v >> BITWIDTH));

                // if(tar_bmp->r[k] == 0xb5 && tar_bmp->g[k] == 0x77 && tar_bmp->b[k] == 0x79) // debug
                // if(pixel_count == 75)
                //    pixel_count = pixel_count;

                k = k + k_inc;
                pixel_count++;

            } // j loop
        } // i loop

#ifdef DEBUG_CSEL
        printf(" %2.2f    %2.2f    %2.2f\n",
                100.0 * y_c_sel_0_count / (y_c_sel_0_count+y_c_sel_1_count+y_c_sel_2_count),
                100.0 * y_c_sel_1_count / (y_c_sel_0_count+y_c_sel_1_count+y_c_sel_2_count),
                100.0 * y_c_sel_2_count / (y_c_sel_0_count+y_c_sel_1_count+y_c_sel_2_count));
#endif

    }

    void CRZ::coeff_cubic_bspline(int * coeff0, int * coeff1, int * coeff2, int * coeff3,
            int MAX_NM, double cubic_or_bspline_) {
        int i;
        double x = 2.0;
        double a;
        double co;
        double co_c, co_s;
        int weight_s_int;
        double weight_s;

        double *f_coeff0, *f_coeff1, *f_coeff3;

        int FACTOR = 1 << BITWIDTH;

        a = cubic_or_bspline_;

        if (((f_coeff0=(double *)calloc(MAX_NM, sizeof(double) )) == NULL) ||
                ((f_coeff1=(double *)calloc(MAX_NM, sizeof(double) )) == NULL) ||
                ((f_coeff3=(double *)calloc(MAX_NM, sizeof(double) )) == NULL) ) {
            printf("Error allocating coeff memory \n");
            exit(1);
        }

        for(i = 0; i < MAX_NM * 2; i++) {
            x -= (double)1.0/MAX_NM;

            if (cubic_or_bspline_ <= -10.0) {  // cubic and b-spline weighted
                weight_s_int = ((int)cubic_or_bspline_/10);
                a = cubic_or_bspline_ - weight_s_int * 10.0;
                weight_s = -(double)weight_s_int / 8.0;


                if (x <= 1) {
                    co_c = (a+2.0)*x*x*x - (a+3.0)*x*x + 1.0;
                    co_s = 0.5*x*x*x - x*x + 2.0/3.0;
                }
                else {
                    co_c = a*x*x*x - 5.0*a*x*x + 8.0*a*x - 4.0*a;
                    co_s = 1.0/6.0*(2.0-x)*(2.0-x)*(2.0-x);
                }
                co = co_s*weight_s + co_c*(1.0-weight_s);
            }
            else if (cubic_or_bspline_ <= 0.0) {  // cubic
                if (x <= 1)
                    co = (a+2.0)*x*x*x - (a+3.0)*x*x + 1.0;
                else
                    co = a*x*x*x - 5.0*a*x*x + 8.0*a*x - 4.0*a;
            }
            else if (cubic_or_bspline_ != 99.0) {  // b-spline
                if (x <= 1)
                    co = 0.5*x*x*x - x*x + 2.0/3.0;
                else
                    co = 1.0/6.0*(2.0-x)*(2.0-x)*(2.0-x);
            }
            else {  // linear
                if (x <= 1)
                    co = 1-x;
                else
                    co = 0;
            }

            switch (i/MAX_NM) {
                case 0:
                    f_coeff0[i%MAX_NM] = co;
                    break;
                case 1:
                    f_coeff1[i%MAX_NM] = co;
                    break;
            }
        }

        coeff3[MAX_NM-1] = 0;
        for (i = 0; i < MAX_NM * 2; i++) {
            if (i < MAX_NM -1) {
                coeff0[i] = ROUND(f_coeff0[i]*FACTOR);
                coeff3[MAX_NM-2-i] = coeff0[i];
            }
            else if (i == MAX_NM -1) {
                coeff0[i] = ROUND(f_coeff0[i]*FACTOR);
                coeff2[i] = coeff0[i];
            }
            else if (i < (MAX_NM+ MAX_NM/2)) {
                coeff1[i-MAX_NM] = ROUND(f_coeff1[i-MAX_NM]*FACTOR);
                coeff2[MAX_NM-2-(i-MAX_NM)] = coeff1[i-MAX_NM];
            }
            else {
                coeff1[i-MAX_NM] = FACTOR - coeff0[i-MAX_NM] - coeff2[i-MAX_NM] - coeff3[i-MAX_NM];
                if (MAX_NM-2-(i-MAX_NM) >=0)
                    coeff2[MAX_NM-2-(i-MAX_NM)] = coeff1[i-MAX_NM];
            }
        }

#ifdef DEBUG_COEFF
        for (i=0; i < MAX_NM; i++) {
            printf("%4d  ", coeff0[i]);
            printf("%4d  ", coeff1[i]);
            printf("%4d  ", coeff2[i]);
            printf("%4d\n", coeff3[i]);
        }
        printf("\n");
#endif
        free(f_coeff0);
        free(f_coeff1);
        free(f_coeff3);
    }

    void CRZ::updown_prz(unsigned char *in_pixel, unsigned char *out_pixel,
            unsigned int tar_width, unsigned int tar_height,
            unsigned int src_width, unsigned int src_height, int is422_Buffer, double shift) {


        unsigned int hratio, vratio;
        unsigned int hresidual, vresidual;

        unsigned int temp_hratio, temp_vratio;
        unsigned int temp_hresidual, temp_vresidual;

        unsigned int uTemp, uTemp1, uX, uY;
        unsigned int uX0, uX1, uY0, uY1, uW0, uW1;
        unsigned int mantessa;

        unsigned int in_pxl_odd;

        unsigned int cpTemp0, cpTemp1;
        int k = 0;
        int prev;
        unsigned char * buffer_pixel;

        hratio = (src_width << 20) / tar_width;
        vratio = (src_height << 20) / tar_height;
        hresidual = src_width % tar_width;
        vresidual = src_height % tar_height;

        if ((buffer_pixel=(unsigned char *)calloc(tar_width * src_height, sizeof(unsigned char))) == NULL) {
            printf("Error allocating memory \n");
            exit(1);
        }


        // horizontal resizing

        for (uY = 0; uY < (unsigned int)src_height; uY++)
        {
            // temp_hratio = 0;
            temp_hratio = (unsigned int)(-shift*(2<<20));
            temp_hresidual = 0;
            in_pxl_odd = 0;
            for (uX = 0; uX < tar_width; uX++)
            {
                // determine coeff_index of two pixels for interpolation
                uX0 = (temp_hratio & 0xfff00000) >> 20;  // MSB 12 bits
                uX1 = uX0 + 1;
                if (uX0 == 0xfff) {
                    uX0 = 0;
                    uX1 = 0;
                }
                // determine weighting : 7-bits
                uW1 = (temp_hratio & 0x000fe000) >> 13;
                uW0 = 0x80 -uW1;

                // determine effect of residual
                mantessa = (temp_hratio & 0xfffff);
                if (temp_hresidual == tar_width && mantessa != 0) {  // need rounding
                    // update parameters
                    temp_hratio = ((uX0+1) << 20) + (unsigned int)(-shift*(2<<20));
                    temp_hresidual = hresidual;

                    uX0 = (temp_hratio & 0xfff00000) >> 20;  // MSB 12 bits
                    uX1 = uX0 + 1;
                    uW1 = (temp_hratio & 0x000fe000) >> 13;
                    uW0 = 0x80 -uW1;

                    temp_hratio += hratio;
                }
                else if (temp_hresidual == tar_width)  { // no need rounding
                    temp_hratio += hratio;
                    temp_hresidual = hresidual;
                }
                else {
                    // update parameters
                    temp_hratio += hratio;

                    if (((temp_hresidual + hresidual) % tar_width) == 0)
                        // next pixel may need rounding
                        temp_hresidual += hresidual;
                    else
                        temp_hresidual  = (temp_hresidual+hresidual) % tar_width;
                }

                // interpolation
                cpTemp0 = (uY * src_width) + uX0;

                if (uX1 == (unsigned int)src_width) { // out of boundary
                    cpTemp1 = cpTemp0;
                    uW1 = 0;
                    uW0 = 0x80;
                }
                else cpTemp1 = cpTemp0 +1;


                uTemp  = in_pixel[cpTemp0];
                uTemp1 = in_pixel[cpTemp1];
                uTemp  = uTemp * uW0 + uTemp1 * uW1;
                // buffer_pixel[k++] = (unsigned char)((uTemp & 0x7f80)>>7);

                uTemp = (uTemp & 0x7f80)>>7;

                if(!is422_Buffer)
                    buffer_pixel[k++] = (unsigned char)uTemp;
                else {
                    if (~in_pxl_odd) {
                        prev = uTemp;
                    }
                    else {
                        buffer_pixel[k++] = (prev + uTemp) >> 1;
                        buffer_pixel[k++] = (prev + uTemp) >> 1;
                    }
                    in_pxl_odd=~in_pxl_odd;
                }
            }
        }


        // vertical resizing
        k=0;
        temp_vratio = (unsigned int)(-shift*(2<<20));
        // temp_vratio = (tar_height >= src_height) ? 0 : 1<<19; //yhc
        temp_vresidual =0;

        for (uY =0; uY < tar_height; uY++) {
            // if(uY == 191)
            //    uY = uY;
            // determine coeff_index of two pixels for interpolation
            uY0 = (temp_vratio & 0xfff00000) >> 20; // MSB 12 bists
            uY1 = uY0 + 1;
            if (uY0 == 0xfff) {
                uY0 = 0;
                uY1 = 0;
            }
            // determine weighting : 7-bits
            uW1 = (temp_vratio & 0x000fe000) >> 13;
            uW0 =0x80 -uW1;
            // determine effect of residual
            mantessa = (temp_vratio & 0xfffff);
            if (temp_vresidual==tar_height && mantessa !=0) // need rounding
            {
                // update parameter
                temp_vratio = ((uY0+1) <<20) + (unsigned int)(-shift*(2<<20));
                temp_vresidual = vresidual;

                uY0 = (temp_vratio & 0xfff00000) >> 20;
                uY1 = uY0 +1;
                uW1 = (temp_vratio & 0x000fe000) >> 13;
                uW0 =0x80 -uW1;

                temp_vratio += vratio;
            }
            else if (temp_vresidual==tar_height) // no need rounding
            {
                temp_vratio += vratio;
                temp_vresidual = vresidual;
            }
            else
            {
                // update parameters
                temp_vratio += vratio;
                if (((temp_vresidual + vresidual) % tar_height) == 0)
                    // next pixel may need rounding
                    temp_vresidual += vresidual;
                else
                    temp_vresidual  = (temp_vresidual + vresidual) % tar_height;
            }

            // resizing whole line
            for (uX = 0; uX < tar_width; uX++)
            {
                // interpolation
                cpTemp0 = (uY0 * tar_width) + uX;
                if (uY1 == (unsigned int)src_height)
                {
                    cpTemp1 = cpTemp0;
                    uW1     = 0;
                    uW0     = 0x80;
                }
                else cpTemp1 = (uY1 * tar_width) + uX;

                uTemp  = buffer_pixel[cpTemp0];
                uTemp1 = buffer_pixel[cpTemp1];
                uTemp = uTemp * uW0 +uTemp1 * uW1;
                out_pixel[k++] = (unsigned char)((uTemp & 0x7f80)>>7);

            }
        }
        free(buffer_pixel);
    }
