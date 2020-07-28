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

#include <stdlib.h>
#include <stdio.h>
#include<math.h>
#include<time.h>


//#define CLOCK
//#define DEBUG
//#define NR_ENABLE
//#define output_file
//#define output_skin_mask
#define output_wrinkle
//#define output_alpha
#define output_skin_tone
#define local_skin_tone_en 1
#define YUV_input

#define KEEP_Y_PERCENTAGE     90       //The highest (100-KEEP_Y_PERCENTAGE)% luminance value would be kept
// The higher this value is, the weaker the luminance enhancement
// Its range is from 0~100
#define BRIGHTNESS_LEVEL      16       //Decide the luminance gain
// The higher the value is, the higher the luminance gain (however, it is limited by KEEP_Y_PERCENTAGE)
// Its value is from 0 (no brightness enhance) ~ 31 (2x brightness enhance)
#define PCA_MULTIPLIER        7        //Decide the start point to control Y value to prevent over-saturated
// The higher the value is, the lower the start point is, the lower the luminance enhance
// Its range is from 0~15
#define Y_SATURATION_CONTROLL 6        //Decide the strength to prevent over-saturated of Y
// The higher the value is, the stronger the strength, the lower the luminance enhance
// Its range is from 0~15
#define PCA_SAT_TH            100      //0~256, the higher the value is, the stronger the saturation is (active when saturation_gain > 1)
#define PCA_SAT_TH1           64       //0~256, the lower the value is, the stronger the de-saturation is (active when saturation_gain < 1)
#define EDGE_PERCENTAGE       30       //If more than EDGE_PERCENTAGE% pixel within the block is non-skin points, this block would be classified as non-skin block
// The higher of this value is, the larger the skin region
// Its range is from 20~40


#define ROUND(a) ((a)>0 ? (int)((a)+0.5) : (int)((a)-0.5))
#define CLIP(a,min,max) ((a)>(max) ? (max): (a)<(min)? (min) : (a))
#define PUT_2B(array,offset,value)  \
    (array[offset] = (char) ((value) & 0xFF), \
     array[offset+1] = (char) (((value) >> 8) & 0xFF))
#define PUT_4B(array,offset,value)  \
    (array[offset] = (char) ((value) & 0xFF), \
     array[offset+1] = (char) (((value) >> 8) & 0xFF), \
     array[offset+2] = (char) (((value) >> 16) & 0xFF), \
     array[offset+3] = (char) (((value) >> 24) & 0xFF))
#define top_search
#define bottom_search
#define pi 3.14159

typedef struct
{
    int face_count;
    int face_size[20];
    int alpha_constant[20];
    int extend_width[20];
    int extend_height[20];
    int box_width[20];
    int box_height[20];
    int box_size[20];
    int Avg_Y[20];
    int Avg_Cb[20];
    int Avg_Cr[20];
    int max_y_gain[20];
    int Sel_Y[20];
    int angle_range[40];
    int angle[20];
    double saturation[20];
    int Angle_shift[20];
    double Y_gain[20];
    double Y_offset[20];
    double saturation_gain[20];
    double final_Saturation_gain;
    double final_Y_gain;
    double final_Y_offset;
    int final_hue_shift;
}DEBUG_MESSAGE;
int Map_extractor(unsigned char* r_data, unsigned char* g_data, unsigned char* b_data, int width, int height, int FD_y, int FD_x
        , int FD_height, int FD_width, unsigned char* texture_map, unsigned char* skin_map, int face_count
        , int sub_box_size, int sub_box_height, int sub_box_width
        , unsigned char* skin_block, unsigned char* AvgRGB_block
        , int face_boundary_portion[6], int* use_texture_only);
void template_drawing(int center_group, int face_pose, int sub_box_size, int sub_box_width, int sub_box_height
        , int face_boundary_portion[6], unsigned char* new_GroupMask1
        , unsigned char* new_GroupMask, unsigned char* Map_temp
        , int* left, int* right);
void template_matching(int Label_num, int Label_num1,
        int relaible_block_number, int relaible_block_number2, int sub_box_size, int sub_box_width,
        int sub_box_height, unsigned char* GroupMask, unsigned char* GroupMask_o,
        unsigned char* AvgRGB_block, unsigned char* new_GroupMask1,
        int center_group, int top_place, int bottom_place, int avg_Cb, int avg_Cr,
        unsigned char* Map_temp, unsigned char* working_buffer);
void inner_block_filling(int sub_box_size, int sub_box_width, int sub_box_height,
        int bottom_place, unsigned char* SubSkinColorMask, int face_pose,
        unsigned char* Map_temp);
void block_to_pixel(unsigned char* in_block, int sub_box_height, int sub_box_width,
        int block_size, unsigned char* Map_temp);
int Grouping_and_ExtractReliableGroup(unsigned char* SubSkinColorMask,
        unsigned char* GroupMask, int sub_box_height, int sub_box_width,
        unsigned char* AvgRGB_block, int face_boundary_portion[6],int face_pose, int mode,
        unsigned char* new_SubSkinColorMask, int* relaible_block_number, int* top_place,
        int* bottom_place, int* avg_Cb, int* avg_Cr, int* GroupMask_tmp, unsigned char* working_buffer);
void Face_Boundary_Reconstruction(unsigned char* GroupMask, int sub_box_height, int sub_box_width, int center_group,
        int face_boundary_portion[6],
        unsigned char* group_mask, int label_number, int* left, int* right, int face_pose);
void watershed_expansion(unsigned char* edge_power_all, unsigned char* SubSkinColorMask,
        int sub_box_height, int sub_box_width, int sub_box_size, int ori_box_height, int ori_box_width,
        unsigned char* skin_tone_map1, int middle_edge, unsigned char* SubSkinColorMask_boundary,
        int use_texture_only, unsigned char* expansion_working_buffer);
#if(local_skin_tone_en==1)
void alpha_map_func(unsigned char* SkinColorMask, unsigned char* edge_power_all,
         int input_img_height, int input_img_width,unsigned char* SubSkinColorMask_boundary,
         int sub_box_height, int sub_box_width, int sub_box_size,
         int min_edge, int alpha_constant, unsigned char* Skin_block);
#else
void alpha_map_func(unsigned char* SkinColorMask, unsigned char* edge_power_all, int input_img_height,
         int input_img_width,unsigned char* SubSkinColorMask_boundary, int sub_box_height,
         int sub_box_width, int sub_box_size,int min_edge, int alpha_constant);
#endif
void Gaussian_filter(unsigned char* input, unsigned char* output,double theta,int box_size, int input_img_height,
         int input_img_width, int average_only);
void find_boundary_block(int sub_box_size, int sub_box_height, int sub_box_width, unsigned char* SubSkinColorMask_boundary
        , unsigned char* SubSkinColorMask, unsigned char* Map_temp);

