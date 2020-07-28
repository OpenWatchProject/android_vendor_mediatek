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
 * MediaTek Inc. (C) 2017. All rights reserved.
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

#ifndef _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IPLUGPRARAM_H_
#define _MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IPLUGPRARAM_H_

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

struct PlugStaticInfo
{
    unsigned int version;
    size_t eep_buf_size;

    PlugStaticInfo()
        : version(0)
        , eep_buf_size(0)
    {}

    //void dump() const;
} ;

struct PlugInitParam
{
    int32_t openId;
    int full_w;
    int full_h;
    int img_w;                                   /* width of fcell image */
    int img_h;                                   /* height of fcell image */
    int offset_x;
    int offset_y;
    size_t eep_buf_size;
    void*  eep_buf_addr;
    int    dpc_num;
    void*  dpc_addr;
    int    fd_dpc_num;
    void*  fd_dpc_addr;
    int32_t reser1;
    int32_t reser2;
    int32_t reser3;
    void*   buf1;
    void*   buf2;
    void*   buf3;
    PlugInitParam()
        : openId(-1)
        , full_w(-1)
        , full_h(-1)
        , img_w(-1)
        , img_h(-1)
        , offset_x(-1)
        , offset_y(-1)
        , eep_buf_size(0)
        , eep_buf_addr(nullptr)
        , dpc_num(0)
        , dpc_addr(nullptr)
        , fd_dpc_num(0)
        , fd_dpc_addr(nullptr)
        , reser1(0)
        , reser2(0)
        , reser3(0)
        , buf1(nullptr)
        , buf2(nullptr)
        , buf3(nullptr)
    {}

    //void dump() const;
} ;

struct PlugProcessingParam
{
    int32_t pattern;                            /*cfa pattern + mirror/flip +  DPC*/
    int32_t bayer_order;
    // buffer
    int src_buf_fd;
    int dst_buf_fd;
    int offset_x;
    int offset_y;

    size_t img_w;                                   /* width of fcell image */
    size_t img_h;                                   /* height of fcell image */
    size_t src_buf_size;
    size_t dst_buf_size;
    unsigned short* src_buf;
    unsigned short* dst_buf;

    // tuning
    int32_t gain_camera;                        /*camera gain*/
    int32_t gain_awb;                           /*awb gain*/
    int32_t gain_awb_r;                         /*awb gain:*/
    int32_t gain_awb_gr;                        /*awb gain*/
    int32_t gain_awb_gb;                        /*awb gain*/
    int32_t gain_awb_b;                         /*awb gain*/
    int32_t gain_analog;                        /*capture sensor analog reg gain*/
    int32_t gain_digital;
    int32_t reserveTuning1;
    int32_t reserveTuning2;
    int32_t reserveTuning3;
    int32_t reserveTuning4;
    int32_t reserveTuning5;
    //
    void*   reserveBuf1;
    void*   reserveBuf2;
    void*   reserveBuf3;
    void*   reserveBuf4;
    void*   reserveBuf5;
    //
    int32_t expo_camera;                        /*expo*/
    int32_t luma_sharp_thr_h;                   /*luma sharp threshold high*/
    int32_t luma_sharp_gain;                    /*luma sharp gain*/
    int32_t luma_sharp_strength;                /*luma sharp strength from 1,2,4,8,16,32*/
    int32_t residual_adjust;                    /*residual adjust*/
    int32_t xtk_blc;                            /*xtalk blc*/
    int32_t xtl_ratio_limit;                    /*xtalk ratio limit*/
    int32_t prev_quality;                       /*preview + quality*/
    int32_t cid;
    int32_t reserve_a;
    int32_t reserve_b;
    int32_t reserve_c;
    int32_t reserve_d;
    int32_t reserve_e;

    PlugProcessingParam()
        : pattern(-1)
        , bayer_order(-1)
        , src_buf_fd(-1)
        , dst_buf_fd(-1)
        , offset_x(-1)
        , offset_y(-1)
        , img_w(0)
        , img_h(0)
        , src_buf_size(0)
        , dst_buf_size(0)
        , src_buf(nullptr)
        , dst_buf(nullptr)
        , gain_camera(-1)
        , gain_awb(-1)
        , gain_awb_r(-1)
        , gain_awb_gr(-1)
        , gain_awb_gb(-1)
        , gain_awb_b(-1)
        , gain_analog(-1)
        , gain_digital(-1)
        , reserveTuning1(-1)
        , reserveTuning2(-1)
        , reserveTuning3(-1)
        , reserveTuning4(-1)
        , reserveTuning5(-1)
        , reserveBuf1(nullptr)
        , reserveBuf2(nullptr)
        , reserveBuf3(nullptr)
        , reserveBuf4(nullptr)
        , reserveBuf5(nullptr)
        , expo_camera(-1)
        , luma_sharp_thr_h(-1)
        , luma_sharp_gain(-1)
        , luma_sharp_strength(-1)
        , residual_adjust(-1)
        , xtk_blc(-1)
        , xtl_ratio_limit(-1)
        , prev_quality(-1)
        , cid(-1)
        , reserve_a(-1)
        , reserve_b(-1)
        , reserve_c(-1)
        , reserve_d(-1)
        , reserve_e(-1)
    {}
    //void dump() const;
};

};  //namespace NSCam

#endif  //_MTK_HARDWARE_MTKCAM_INCLUDE_MTKCAM_UTILS_HW_IPLUGPRARAM_H_

