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
#ifndef _I_PURE_TO_JPEG_H_
#define _I_PURE_TO_JPEG_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <aaa_types.h>
#include <camera_custom_nvram.h>

class IRawToJpeg
{
public:
    typedef enum
    {
        E_PACK_RAW = 0,
        E_PROC_RAW,
        E_YUV,
        E_JPEG
    } E_STAGE_T;

    typedef enum
    {
        ImgFmt_BAYER10 = 0,
        ImgFmt_YUY2
    } E_FORMAT_T;

    typedef struct
    {
        MINT32      i4Width;
        MINT32      i4Height;
        E_FORMAT_T  i4Format;
        MBOOL       bUnpack2PackEn;
        MUINT32     u4Pixel;
        MINT32      i4JpegWidth;
        MINT32      i4JpegHeight;
    }InputInfo;

    static IRawToJpeg* createInstance();
    virtual void destroyInstance() = 0;

    virtual MBOOL       readFile(const char* input_file, const InputInfo& info) = 0;
    virtual MBOOL       saveFile(const char* output_file, E_STAGE_T eStage) = 0;
    virtual MBOOL       saveJPGToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size) = 0;
    virtual void        releaseFile();
    virtual MBOOL       run() = 0;
    virtual void        setIspCtl(MUINT32 RgbEn, MUINT32 YuvEn, MUINT32 Yuv2En) = 0;

	virtual MBOOL		setPGN(ISP_NVRAM_PGN_T& pgn) = 0;
	virtual MBOOL		setUDM(ISP_NVRAM_UDM_T& udm) = 0;
	virtual MBOOL		setCCM(ISP_NVRAM_CCM_T& ccm) = 0;
	virtual MBOOL		setGGM(ISP_NVRAM_GGM_T& ggm) = 0;
	virtual MBOOL		setDBS(ISP_NVRAM_DBS_T& dbs) = 0;
	virtual MBOOL		setOBC(ISP_NVRAM_OBC_T& obc) = 0;
	virtual MBOOL		setBNR_BPC(ISP_NVRAM_BNR_BPC_T& bpc) = 0;
	virtual MBOOL		setBNR_NR1(ISP_NVRAM_BNR_NR1_T& nr1) = 0;
	virtual MBOOL		setBNR_PDC(ISP_NVRAM_BNR_PDC_T& pdc) = 0;
	virtual MBOOL		setLSC(ISP_NVRAM_LSC_T& lsc, MUINT32 *pLscTbl, MUINT32 tblSize) = 0;
	virtual MBOOL		setRNR(ISP_NVRAM_RNR_T& rnr) = 0;
	virtual MBOOL		setSL2(ISP_NVRAM_SL2_T& sl2) = 0;
	virtual MBOOL		setSL2G(ISP_NVRAM_SL2_T& sl2) = 0;
	virtual MBOOL		setG2C(ISP_NVRAM_G2C_T& g2c) = 0;
	virtual MBOOL		setANR(ISP_NVRAM_ANR_T& anr, ISP_NVRAM_ANR_LUT_T& anrLut) = 0;
	virtual MBOOL		setANR2(ISP_NVRAM_ANR2_T& anr2) = 0;
	virtual MBOOL		setCCR(ISP_NVRAM_CCR_T& ccr) = 0;
	virtual MBOOL		setPCA(ISP_NVRAM_PCA_T& pca, MUINT32 *pcaLut) = 0;
	virtual MBOOL		setSEEE(ISP_NVRAM_EE_T& ee, ISP_NVRAM_SE_T& se) = 0;


protected:  //    Ctor/Dtor.
                        IRawToJpeg(){}
    virtual             ~IRawToJpeg(){}

                        IRawToJpeg(const IRawToJpeg&);
                        IRawToJpeg& operator=(const IRawToJpeg&);
};

#endif // _I_PURE_TO_JPEG_H_
