/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#ifndef _PD_BUF_MGR_H_
#define _PD_BUF_MGR_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1
#include <log/log.h>
#include "pd_buf_common.h"
#include "kd_imgsensor_define.h"

//for pdo general path
typedef enum
{
    R = 0,
    L
} LR_T;

typedef struct
{
    int cx; //coordinate
    int cy;
    int bx; //block
    int by;
    LR_T lr;
} PDPIXEL_T;

typedef struct
{
    int cx;
    int cy;
    int row; //pdo buffer row
    LR_T lr;
} PDMAP_T;

#define MAX_PDO_BUF_ROW_NUM 16
#define MAX_LR_ROW 8
#define MAX_PAIR_NUM 40

typedef enum
{
    /* Add command before this line */
    PDBUFMGR_CMD_NUM

} PDBUFMGR_CMD_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  PD buffer manager I/F : using MTK pd algo.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class PDBufMgr
{
private:

protected:

    SET_PD_BLOCK_INFO_T m_PDBlockInfo;
    PDBuf_Bpci_Info_t m_bpciInfo;

    //for pdo general separate method
    unsigned int subBlkX, subBlkY, rowNum;
    PDMAP_T pPdMap[MAX_PAIR_NUM * 2];
    PDPIXEL_T pdPixels[MAX_PAIR_NUM * 2];

    /**
    * @brief checking current setting is suport PDAF or not.
    */
    virtual MBOOL IsSupport( SPDProfile_t &iPdProfile) = 0;
    /**
    * @for pdo general separate method: parse pd pixels for separate LR
    */
    MVOID parsePdPixels(PDPIXEL_T* pPdPix, int pdNum, int subBlkH);

    /**
    * @for pdo general separate method: get pd mapping for separate LR
    */
    MVOID getPdMapping(PDPIXEL_T* pPdPix, int pdNum);

public:
    PDBufMgr();
    virtual ~PDBufMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//                                      Interface
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    /**
    * @brief create pd buffer manager instance.
    */
    static PDBufMgr* createInstance( SPDProfile_t &iPdProfile);

    MVOID SetBpciInfo(UINT32 pdo_xsize, UINT32 pdo_ysize);

    /* Input */
    /**
    * @brief set pd block information
    */
    virtual MBOOL SetPDBlockInfo( SET_PD_BLOCK_INFO_T &iPDBlockInfo);
    /**
    * @brief send command.
    */
    MBOOL sendCommand( MUINT32  i4Cmd, MVOID* arg=NULL);

    /**
    * @brief convert PD data buffer format.
    */
    virtual MUINT16* ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL b14BitData=MFALSE) = 0;

    /**
    * @brief get m_PDXSz, m_PDYSz, and m_PDBufSz.
    * must called after ConvertPDBufFormat() calculated
    */
    virtual MBOOL GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz);

    /**
    * @brief output DualPD VC information
    */
    virtual MBOOL GetDualPDVCInfo( MINT32 i4CurSensorMode, SDUALPDVCINFO_T &oDualPDVChwInfo, MINT32 i4AETargetMode);

    /**
    * @for pdo general separate method: get pdo pixels' position
    */
    MBOOL pdoPixelPos(MUINT32 pPdoPixPos[][2], int pdNum);

    /**
    * @for pdo general separate method: separate LR by pd mapping
    */
    virtual MVOID separateLR( unsigned int stride, unsigned char *ptr, unsigned int pd_x_num, unsigned int pd_y_num, unsigned short *outBuf, unsigned int iShift);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

};
#endif
