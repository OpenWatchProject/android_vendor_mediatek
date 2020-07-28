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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_CAMIO_PORTMAP_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_CAMIO_PORTMAP_H_
//
#include <mtkcam/drv/def/ispio_port_index.h>
#include "Port.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
//
static const NSCam::NSIoPipe::PortID    PORT_IMGO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_UFEO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_UFEO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_RRZO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_RRZO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_CAMSV_IMGO (EPortType_Memory, NSImageio::NSIspio::EPortIndex_CAMSV_IMGO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_CAMSV2_IMGO (EPortType_Memory, NSImageio::NSIspio::EPortIndex_CAMSV2_IMGO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_RAWI (EPortType_Memory, NSImageio::NSIspio::EPortIndex_RAWI, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_LCSO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_LCSO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_LCESHO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_LCESHO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_AAO    (EPortType_Memory, NSImageio::NSIspio::EPortIndex_AAO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_AAHO    (EPortType_Memory, NSImageio::NSIspio::EPortIndex_AAHO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_AFO    (EPortType_Memory, NSImageio::NSIspio::EPortIndex_AFO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_PDO    (EPortType_Memory, NSImageio::NSIspio::EPortIndex_PDO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_PSO    (EPortType_Memory, NSImageio::NSIspio::EPortIndex_PSO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_EISO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_EISO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_FLKO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_FLKO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_RSSO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_RSSO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_TSFSO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_TSFSO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_IMGI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_IMGBI  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGBI, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_IMGCI  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGCI, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_VIPI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIPI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_VIP2I  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIP2I, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_VIP3I  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIP3I, 0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_DEPI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_DEPI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_UFDI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_UFDI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_LCEI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_LCEI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_DMGI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_DMGI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_REGI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_REGI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_IMG2O  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG2O, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_IMG3O  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3O, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_IMG3BO (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3BO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_IMG3CO (EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3CO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_MFBO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_MFBO,  1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_PAK2O   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_PAK2O,  1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_FEO    (EPortType_Memory, NSImageio::NSIspio::EPortIndex_FEO,   1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_WROTO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_WROTO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_WDMAO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_JPEGO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_JPEGO/*index*/, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_VENC_STREAMO(EPortType_Memory, NSImageio::NSIspio::EPortIndex_VENC_STREAMO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_WPEO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_WPEO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_MSKO  (EPortType_Memory, NSImageio::NSIspio::EPortIndex_MSKO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_WPEI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_WPEI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_BPCI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_BPCI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_LSCI   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_LSCI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_TIMGO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_TIMGO,  1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_DCESO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_DCESO,  1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);


//Virtual DMA Port
static const NSCam::NSIoPipe::PortID    PORT_YNR_FACEI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_YNR_FACEI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_YNR_LCEI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_YNR_LCEI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_LPCNR_YUVI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_LPCNR_YUVI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_LPCNR_YUVO   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_LPCNR_YUVO,  1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_LPCNR_UVI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_LPCNR_UVI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_LPCNR_UVO   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_LPCNR_UVO,  1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_GOLDENFRMI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_GOLDENFRMI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_WEIGHTMAPI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_WEIGHTMAPI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_CNR_BLURMAPI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_CNR_BLURMAPI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);


static const NSCam::NSIoPipe::PortID    PORT_LFEOI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_LFEOI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_RFEOI   (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_RFEOI,  0/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_FMO  (EPortType_Memory, NSImageio::NSIspio::VirDIPPortIdx_FMO,  1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_YUVO   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_YUVO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_CRZO_R1   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_CRZO, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_CRZO_R2   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_CRZO_R2, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);
static const NSCam::NSIoPipe::PortID    PORT_RSSO_R2   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_RSSO_R2, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);

static const NSCam::NSIoPipe::PortID    PORT_UNKNOWN   (EPortType_Memory, NSImageio::NSIspio::EPortIndex_UNKNOW, 1/*in/out*/,EPortCapbility_None/*capbility*/,0/*frame group*/);


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_POSTPROC_PORTMAP_H_

