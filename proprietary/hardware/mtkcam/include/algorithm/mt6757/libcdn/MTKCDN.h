/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */

 /* MediaTek Inc. (C) 2016. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*
**
** Copyright 2008, The Android Open Source Project
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
*/

#ifndef _MTK_CDN_H
#define _MTK_CDN_H

#include "MTKCDNType.h"
#include "MTKCDNErrCode.h"
#include "cdn_cltk_api.h"

#define CDN_MAX_SPLIT      64
#ifdef __ANDROID__
    #define __OCL_GPU__
    //#define PC_UTIL
#else
    //#define __OCL_GPU__
    #define PC_UTIL
#endif

// HW version
#ifdef __OCL_GPU__
//#define _IMG_GPU_
#endif


#define SUPPORT_OUTPUT_YUV422_3P

typedef enum DRVCDNObject_s {
    DRV_CDN_OBJ_NONE = 0,
    DRV_CDN_OBJ_SW,
    DRV_CDN_OBJ_UNKNOWN = 0xFF,
} DrvCDNObject_e;

///////////////////////

typedef struct{
#ifdef __OCL_GPU__
    /* CLTK context */
    cltk_context ctx;

    /* CLTK image/buffers */
    cltk_image uc_Buffer1;
    cltk_image uc_Buffer2;
    cltk_image uc_Buffer3;
    cltk_image f_BufferTrans;
#else
    unsigned char *uc_Buffer1;
    unsigned char *uc_Buffer2;
    unsigned char *uc_Buffer3;
#endif
}CDN_INITPARAMS;

typedef struct {
    //ISP Info
    int isRotate;

    int mode;
    int var;

    float Trans[9];

    int width;
    int height;
    int id;

#if 0 // reserved for future use
    int dwidth;
    int dheight;
    int dPitch;
    int dsH;
    int dsV;

    #ifdef PC_UTIL
    bool unpack;
    #endif
#endif
} CDN_PARAMS;

typedef struct{
    //unpadded width, height
    int width;
    int height;

    // Input Data
    // Width x Height YUV422 8-bit Mono
    unsigned char* MonoYUV;
    // Width x Height YUV422 8-bit Bayer
    unsigned char* BayerYUV;

    // denoise output after CDN
    unsigned char* output[3];            // Width x Height YUYV 8-bit
} CDNBuffers;

typedef struct{
    int width;
    int height;
    int isRotate;
    float Trans[9];
    int CInfo[4];
    int SInfo[15];
    int BOUNDARY_THR;
    int VAR_THR;
    int MACRO_BOUND_THR;
} CDNCheckProcess;

typedef enum
{
    CDN_FEATURE_BEGIN,              // minimum of feature id
    CDN_FEATURE_SET_PARAMS,         // feature id to set images
    CDN_FEATURE_SET_IMAGES,         // feature id to set images
    CDN_FEATURE_GET_WORKBUF_SIZE,   // feature id to query buffer size
    CDN_FEATURE_SET_WORKBUF_ADDR,   // feature id to set working buffer address
    CDN_FEATURE_GET_RESULT,         // feature id to get result
    CDN_FEATURE_GET_LOG,            // feature id to get debugging information
    CDN_FEATURE_CHECK_PROCESS       // feature id to check warp status
} CDN_FEATURE_ENUM;

/*******************************************************************************
*
********************************************************************************/
class MTKCDN {
public:
    static MTKCDN* createInstance(DrvCDNObject_e eobject);
    virtual void   destroyInstance(MTKCDN* obj) = 0;

    virtual ~MTKCDN(){}
    // Process Control
    virtual MRESULT CDNInit(void *InitInData, void *InitOutData);    // Env/Cb setting
    virtual MRESULT CDNMain(void);                                         // START
    virtual MRESULT CDNReset(void);                                        // RESET

    // Feature Control
    virtual MRESULT CDNFeatureCtrl(MUINT32 FeatureID, void* pParaIn, void* pParaOut);
private:

};

class AppCDNTmp : public MTKCDN {
public:
    //
    static MTKCDN* getInstance();
    virtual void destroyInstance(MTKCDN* obj) = 0;
    //
    AppCDNTmp() {};
    virtual ~AppCDNTmp() {};
};

void CDN_PREPROCESS( unsigned char* out_buf, unsigned short* in_buf, int out_w, int out_h, int out_s, int crop_x, int crop_y, int order ) ;

#endif

