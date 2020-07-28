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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief

#include <vector>
#include <list>
#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include "isp_function_dip.h"
#include "cam_path_dip.h"

#include "PostProcPipe.h"
#include "IPostProcPipe.h"


#undef LOG_TAG
#define LOG_TAG "Test_ImageIo"

using namespace NSImageio;
using namespace NSIspio;

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        printf(": failed to create file [%s]: %s \n", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            printf(": failed to write to file [%s]: %s\n", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}


MINT32 P2A_FG();
MINT32 P2A_RAW();
MINT32 P2B_YUV_FrameMode_1();
MINT32 P2B_YUV_TileMode();
MINT32 P2A_CRZ1();
MINT32 P2A_CRZ2();
MINT32 P2A_CRZ3();
MINT32 P2A_FM();

int postProcDip(MUINT32 _case){
    int ret = 0;

    printf("enter IspDrvDipFunc _case(%d)\n",_case);
    switch(_case){  // 0 for phy, 1 for vir
        case 0: // Test PostProcPipe
            {
                ret = P2A_RAW();
            }
            break;
        case 1: //p2a path with fg format
            {
                ret=P2A_FG();
            }
            break;
        case 2: //CRZ x1
            {
                ret=P2A_CRZ1();
            }
            break;
        case 3: //CRZ x1/2
            {
                ret=P2A_CRZ2();
            }
            break;
        case 4: //CRZ x1/2 + crop
            {
                ret=P2A_CRZ3();
            }
            break;
        case 5:
            {
                ret = P2B_YUV_FrameMode_1();
            }
            break;
        case 6:
            {
                ret = P2B_YUV_TileMode();
            }
            break;
        case 7:
            {
                ret = P2A_FM();
            }
            break;
        default:
            printf("RW path err(0x%x)",_case);
            ret = -1;
            break;
    }

    return ret;

}

static vector<PortInfo const*> g_vPostProcInPorts(1);
static vector<PortInfo const*> g_vPostProcOutPorts(1);

void Tuning_Dump(void)
{
	IspDrvDipPhy* pDrvDipPhy = (IspDrvDipPhy*)IspDrvDipPhy::createInstance(DIP_A);
	IspDrvDip* m_ispDrvDipTh0;
//	char userNameTh0[30]="user_isp_drv_dip_th0";
	MUINT32 regW, regR;
    IPostProcPipe* mpPostProcPipe;
    dip_x_reg_t *pIspPhyReg;
	PipePackageInfo pipePackageInfo;
	IMemDrv* mpImemDrv=NULL;
	IMEM_BUF_INFO pTuningQueBuf;
	ISP_HW_MODULE hwModule = DIP_A;
	E_ISP_DIP_CQ dipTh0 = ISP_DRV_DIP_CQ_THRE0;
	MUINT32 burstQueIdx = 0;
	MUINT32 dupCqIdx = 0;
	char userNameTh0[30]="user_isp_drv_dip_th0";
	PortInfo m_imgi;
	PortInfo m_wroto;

	pTuningQueBuf.size = sizeof(dip_x_reg_t);
    printf("pTuningQueBuf size %d \n",pTuningQueBuf.size);

    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
	pTuningQueBuf.size = sizeof(dip_x_reg_t);
    mpImemDrv->allocVirtBuf(&pTuningQueBuf);
    mpImemDrv->mapPhyAddr(&pTuningQueBuf);
    printf("buffer VA address %lu \n",pTuningQueBuf.virtAddr);

	pipePackageInfo.burstQIdx=0;
	pipePackageInfo.dupCqIdx=0;
	pipePackageInfo.pTuningQue = (MVOID* )pTuningQueBuf.virtAddr;
	pipePackageInfo.drvScen = eDrvScenario_P2A;
	pipePackageInfo.pixIdP2 = 0;
	pipePackageInfo.p2cqIdx = 0;

    m_imgi.eImgFmt     = eImgFmt_FG_BAYER8;//(EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat;  // Image Pixel Format
    m_imgi.u4ImgWidth  = 480;//mAcdkMhalPrvParam.frmParam.w;  // Dispaly Width. concept is assign memory footprint width and height
    m_imgi.u4ImgHeight = 640;//mAcdkMhalPrvParam.frmParam.h;  // Display Height. concept is assign memory footprint width and height
    m_imgi.index       = EPortIndex_IMGI;      // temp use for debug EPortIndex_IMG2O/EPortIndex_WROTO;     //  port index
    g_vPostProcInPorts.resize(1);
    g_vPostProcInPorts.at(0) = &m_imgi;

    m_wroto.eImgFmt     = eImgFmt_YUY2;//EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat;  // Image Pixel Format
    m_wroto.u4ImgWidth  = 480;//mAcdkMhalPrvParam.frmParam.w;  // Dispaly Width. concept is assign memory footprint width and height
    m_wroto.u4ImgHeight = 640;//mAcdkMhalPrvParam.frmParam.h;  // Display Height. concept is assign memory footprint width and height
    m_wroto.type        = EPortType_Memory;     //  EPortType
    m_wroto.index       = EPortIndex_WROTO;      // temp use for debug EPortIndex_IMG2O/EPortIndex_WROTO;     //  port index
    m_wroto.inout       = EPortDirection_Out;   //  0:in/1:out
    m_wroto.pipePass    = EPipePass_Dip_Th0;      //  select pass
    g_vPostProcOutPorts.resize(1);
    g_vPostProcOutPorts.at(0) = &m_wroto;

	printf("QQ create Test_PostProcPipe \n");
    mpPostProcPipe = IPostProcPipe::createInstance();
	printf("QQ init Test_PostProcPipe \n");
    mpPostProcPipe->init("Test_PostProcPipe");

	regW = 0x45122;
	DIP_WRITE_PHY_REG(DIP_A, pDrvDipPhy, DIP_X_CTL_YUV_EN, regW);
	regR = DIP_READ_PHY_REG(DIP_A, pDrvDipPhy,DIP_X_CTL_YUV_EN);
	printf("[Test:DIP_WRITE_PHY_REG/DIP_READ_PHY_REG]regW(0x%x),regR(0x%x)\n",regW,regR);
	if(regW != regR) {
		printf("[Error] value of regW and regR are not the same.\n");
	}

	pDrvDipPhy->init("isp_drv_dip_phy test");
	DIP_WRITE_PHY_REG(DIP_A, pDrvDipPhy, DIP_X_CTL_RGB_EN, 0x12345678);
	DIP_WRITE_PHY_REG(DIP_A, pDrvDipPhy, DIP_X_CTL_YUV_EN, 0x12345678);
	DIP_WRITE_PHY_REG(DIP_A, pDrvDipPhy, DIP_X_CTL_YUV2_EN, 0x12345678);
	DIP_WRITE_PHY_REG(DIP_A, pDrvDipPhy, DIP_X_DBS2_SIGMA, 0x12345678);
	DIP_WRITE_PHY_REG(DIP_A, pDrvDipPhy, DIP_X_DBS2_BSTBL_0, 0x12345678);
	DIP_WRITE_PHY_REG(DIP_A, pDrvDipPhy, DIP_X_DBS2_BSTBL_2, 0x12345678);

	m_ispDrvDipTh0 = IspDrvDip::createInstance(DIP_A, ISP_DRV_DIP_CQ_THRE0, 0, 0, "user_isp_drv_dip_th0");
    m_ispDrvDipTh0->init("isp_drv_dip ");
    m_ispDrvDipTh0->dumpTuningDebugLog();

}


void postProcDip_Tuning(void)
{
    IPostProcPipe* mpPostProcPipe;
    dip_x_reg_t *pIspPhyReg;
	PipePackageInfo pipePackageInfo;
	IMemDrv* mpImemDrv=NULL;
	IMEM_BUF_INFO pTuningQueBuf;
	static IspDrvDip* m_ispDrvDipTh0;
	ISP_HW_MODULE hwModule = DIP_A;
	E_ISP_DIP_CQ dipTh0 = ISP_DRV_DIP_CQ_THRE0;
	MUINT32 burstQueIdx = 0;
	MUINT32 dupCqIdx = 0;
	char userNameTh0[30]="user_isp_drv_dip_th0";
	PortInfo m_imgi;
	PortInfo m_wroto;


	pTuningQueBuf.size = sizeof(dip_x_reg_t);
    printf("pTuningQueBuf size %d \n",pTuningQueBuf.size);

    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
	pTuningQueBuf.size = sizeof(dip_x_reg_t);
    mpImemDrv->allocVirtBuf(&pTuningQueBuf);
    mpImemDrv->mapPhyAddr(&pTuningQueBuf);
    printf("buffer VA address %lu \n",pTuningQueBuf.virtAddr);

//Tuning Value Setting
	pIspPhyReg = (dip_x_reg_t *)pTuningQueBuf.virtAddr;
    pIspPhyReg->DIP_X_CTL_RGB_EN.Raw=0;
    pIspPhyReg->DIP_X_CTL_YUV_EN.Raw=0;
    pIspPhyReg->DIP_X_CTL_YUV2_EN.Raw=0;
    pIspPhyReg->DIP_X_CTL_RGB_EN.Bits.UNP_EN=0;
    pIspPhyReg->DIP_X_CTL_RGB_EN.Bits.PGN_EN=1;
    pIspPhyReg->DIP_X_CTL_RGB_EN.Bits.GGM_EN=0;
    pIspPhyReg->DIP_X_CTL_RGB_EN.Bits.DBS2_EN=1;
//    pIspPhyReg->DIP_X_DBS2_BSTBL_0.Raw=0x11115678;
//    pIspPhyReg->DIP_X_DBS2_BSTBL_1.Raw=0x22225678;
//    pIspPhyReg->DIP_X_DBS2_BSTBL_2.Raw=0x33335678;
//    pIspPhyReg->DIP_X_DBS2_CTL.Raw=0x44445678;
//    pIspPhyReg->DIP_X_DBS2_CTL_2.Raw=0x55555678;
//    pIspPhyReg->DIP_X_DBS2_SIGMA.Raw=0x66665678;

	pipePackageInfo.burstQIdx = 0;
	pipePackageInfo.dupCqIdx = 0;
	pipePackageInfo.pTuningQue = (MVOID* )pTuningQueBuf.virtAddr;
	pipePackageInfo.drvScen = eDrvScenario_P2A;
	pipePackageInfo.pixIdP2 = 0;
	pipePackageInfo.p2cqIdx = 0;

    m_imgi.eImgFmt     = eImgFmt_FG_BAYER8;//(EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat;  // Image Pixel Format
    m_imgi.u4ImgWidth  = 480;//mAcdkMhalPrvParam.frmParam.w;  // Dispaly Width. concept is assign memory footprint width and height
    m_imgi.u4ImgHeight = 640;//mAcdkMhalPrvParam.frmParam.h;  // Display Height. concept is assign memory footprint width and height
    m_imgi.index       = EPortIndex_IMGI;      // temp use for debug EPortIndex_IMG2O/EPortIndex_WROTO;     //  port index
    g_vPostProcInPorts.resize(1);
    g_vPostProcInPorts.at(0) = &m_imgi;

    m_wroto.eImgFmt     = eImgFmt_YUY2;//EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat;  // Image Pixel Format
    m_wroto.u4ImgWidth  = 480;//mAcdkMhalPrvParam.frmParam.w;  // Dispaly Width. concept is assign memory footprint width and height
    m_wroto.u4ImgHeight = 640;//mAcdkMhalPrvParam.frmParam.h;  // Display Height. concept is assign memory footprint width and height
    m_wroto.type        = EPortType_Memory;     //  EPortType
    m_wroto.index       = EPortIndex_WROTO;      // temp use for debug EPortIndex_IMG2O/EPortIndex_WROTO;     //  port index
    m_wroto.inout       = EPortDirection_Out;   //  0:in/1:out
    m_wroto.pipePass    = EPipePass_Dip_Th0;      //  select pass
    g_vPostProcOutPorts.resize(1);
    g_vPostProcOutPorts.at(0) = &m_wroto;

	printf("QQ create Test_PostProcPipe \n");
    mpPostProcPipe = IPostProcPipe::createInstance();
	printf("QQ init Test_PostProcPipe \n");
    mpPostProcPipe->init("Test_PostProcPipe");
	printf("QQ configPipe Test_PostProcPipe \n");
    mpPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts,&pipePackageInfo);
	printf("QQ start Test_PostProcPipe \n");
    mpPostProcPipe->start();

	printf("QQ start Tuning_Dump \n");
	Tuning_Dump();

	printf("QQ uninit Test_PostProcPipe \n");
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();

}

int Test_ImageIo(int argc, char** argv)
{
    MUINT32 HwModoule;
    MUINT32 RW_Path;
    (void)argc;(void)argv;
    printf("##############################\n");
    printf("case 0: PostProc\n");
    printf("select test hw module\n");
    printf("##############################\n");
//kk test    char s = getchar();
//kk test    HwModoule = atoi((const char*)&s);

    printf("select path\n");
    printf("##############################\n");
//kk test    s = getchar();
//kk test    RW_Path = atoi((const char*)&s);

    HwModoule = atoi(argv[1]);
    RW_Path = atoi(argv[2]);


    printf("test case :HW module:0x%x, RW path:0x%x\n",HwModoule,RW_Path);


    switch(HwModoule){
        case 0:
            return postProcDip(RW_Path);
            break;
        case 1:
            printf("postProcDip_Tuning\n");
            postProcDip_Tuning();
            break;
        default:
            break;
    }
    return 0;
}


#include "DIP_pics/imgi_960x540_yuy2.h"
#include "DIP_pics/imgi_256x256_yv12.h"
#include "DIP_pics/imgi_256x256_yuy2.h"
#include "DIP_pics/ufdi_256x256_byte8.h"
#include "DIP_pics/lcei_256x256_byte8.h"

// imgi: 256x256(yv12)
MINT32 P2B_YUV_FrameMode_1()
{
    int ret = 0;
    IPostProcPipe* mpPostProcPipe;
    static vector<PortInfo const*> vPostProcInPorts(4);
    static vector<PortInfo const*> vPostProcOutPorts(4);
    PipePackageInfo pipePackageInfo;
    IMemDrv* mpImemDrv=NULL;
    IMEM_BUF_INFO buf_imgi,buf_imgbi,buf_imgci, buf_img2o, buf_img3o, buf_wroto, buf_wdmao, buf_vipi, buf_ufdi, buf_lcei;
    NSImageio::NSIspio::PortInfo port_imgi,port_imgbi,port_imgci, port_img2o, port_img3o, port_wroto, port_wdmao, port_vipi, port_ufdi, port_lcei;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    int _imgi_w_=256, _imgi_h_=256;
    int _ufdi_w_=256, _ufdi_h_=256;
    int _lcei_w_=256, _lcei_h_=256;
    int _imgbi_w_=_imgi_w_/2, _imgbi_h_=_imgi_h_/2;
    int _imgci_w_=_imgi_w_/2, _imgci_h_=_imgi_h_/2;
    int _vipi_w_=256, _vipi_h_=256;

    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    char s;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("Test_P2B_YUV");
    //
    //imgi
    buf_imgi.size=sizeof(g_yuv420_3plane_y_256_256_s256);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(g_yuv420_3plane_y_256_256_s256), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
    port_imgi.resize1.tar_w = _imgi_w_;
    port_imgi.resize1.tar_h = _imgi_h_;
    port_imgi.crop1.x = 0;
    port_imgi.crop1.floatX = 0;
    port_imgi.crop1.y = 0;
    port_imgi.crop1.floatY = 0;
    port_imgi.crop1.w = _imgi_w_;
    port_imgi.crop1.h = _imgi_h_;
    port_imgi.eImgFmt     = eImgFmt_I420;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    //imgbi
    buf_imgbi.size=sizeof(g_yuv420_3plane_u_128_128_s256);
    mpImemDrv->allocVirtBuf(&buf_imgbi);
    mpImemDrv->mapPhyAddr(&buf_imgbi);
    memcpy( (MUINT8*)(buf_imgbi.virtAddr), (MUINT8*)(g_yuv420_3plane_u_128_128_s256), buf_imgbi.size);
    port_imgbi.u4ImgWidth  = _imgbi_w_;
    port_imgbi.u4ImgHeight = _imgbi_h_;
    port_imgbi.resize1.tar_w = _imgbi_w_;
    port_imgbi.resize1.tar_h = _imgbi_h_;
    port_imgbi.crop1.x = 0;
    port_imgbi.crop1.floatX = 0;
    port_imgbi.crop1.y = 0;
    port_imgbi.crop1.floatY = 0;
    port_imgbi.crop1.w = _imgbi_w_;
    port_imgbi.crop1.h = _imgbi_h_;
    port_imgbi.eImgFmt     = eImgFmt_I420;
    port_imgbi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgbi.index       = EPortIndex_IMGBI;
    port_imgbi.inout       = 0; //in
    port_imgbi.capbility   = 0;
    port_imgbi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgbi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    //imgci
    buf_imgci.size=sizeof(g_yuv420_3plane_v_128_128_s256);
    mpImemDrv->allocVirtBuf(&buf_imgci);
    mpImemDrv->mapPhyAddr(&buf_imgci);
    memcpy( (MUINT8*)(buf_imgci.virtAddr), (MUINT8*)(g_yuv420_3plane_v_128_128_s256), buf_imgci.size);
    port_imgci.u4ImgWidth  = _imgci_w_;
    port_imgci.u4ImgHeight = _imgci_h_;
    port_imgci.resize1.tar_w = _imgci_w_;
    port_imgci.resize1.tar_h = _imgci_h_;
    port_imgci.crop1.x = 0;
    port_imgci.crop1.floatX = 0;
    port_imgci.crop1.y = 0;
    port_imgci.crop1.floatY = 0;
    port_imgci.crop1.w = _imgci_w_;
    port_imgci.crop1.h = _imgci_h_;
    port_imgci.eImgFmt     = eImgFmt_I420;
    port_imgci.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgci.index       = EPortIndex_IMGCI;
    port_imgci.inout       = 0; //in
    port_imgci.capbility   = 0;
    port_imgci.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgci.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    //
    port_imgi.resize1.tar_w = _imgi_w_;
    port_imgi.resize1.tar_h = _imgi_h_;
    port_imgi.crop1.x = 0;
    port_imgi.crop1.floatX = 0;
    port_imgi.crop1.y = 0;
    port_imgi.crop1.floatY = 0;
    port_imgi.crop1.w = _imgi_w_;
    port_imgi.crop1.h = _imgi_h_;
    //
    port_imgi.resize2.tar_w = _imgi_w_;
    port_imgi.resize2.tar_h = _imgi_h_;
    port_imgi.crop2.x = 0;
    port_imgi.crop2.floatX = 0;
    port_imgi.crop2.y = 0;
    port_imgi.crop2.floatY = 0;
    port_imgi.crop2.w = _imgi_w_;
    port_imgi.crop2.h = _imgi_h_;
    //
    port_imgi.resize3.tar_w = _imgi_w_;
    port_imgi.resize3.tar_h = _imgi_h_;
    port_imgi.crop3.x = 0;
    port_imgi.crop3.floatX = 0;
    port_imgi.crop3.y = 0;
    port_imgi.crop3.floatY = 0;
    port_imgi.crop3.w = _imgi_w_;
    port_imgi.crop3.h = _imgi_h_;
    //
    port_imgi.u4Stride[ESTRIDE_1ST_PLANE] = _imgi_w_;
    port_imgi.u4BufSize[ESTRIDE_1ST_PLANE]= buf_imgi.size;
    port_imgi.u4BufVA[ESTRIDE_1ST_PLANE]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[ESTRIDE_1ST_PLANE]  = buf_imgi.phyAddr;
    port_imgi.memID[ESTRIDE_1ST_PLANE]    = buf_imgi.memID;
    //
    port_imgi.u4Stride[ESTRIDE_2ND_PLANE] = _imgbi_w_;
    port_imgi.u4BufSize[ESTRIDE_2ND_PLANE]= buf_imgbi.size;
    port_imgi.u4BufVA[ESTRIDE_2ND_PLANE]  = buf_imgbi.virtAddr;
    port_imgi.u4BufPA[ESTRIDE_2ND_PLANE]  = buf_imgbi.phyAddr;
    port_imgi.memID[ESTRIDE_2ND_PLANE]    = buf_imgbi.memID;
    //
    port_imgi.u4Stride[ESTRIDE_3RD_PLANE] = _imgi_w_;
    port_imgi.u4BufSize[ESTRIDE_3RD_PLANE]= buf_imgci.size;
    port_imgi.u4BufVA[ESTRIDE_3RD_PLANE]  = buf_imgci.virtAddr;
    port_imgi.u4BufPA[ESTRIDE_3RD_PLANE]  = buf_imgci.phyAddr;
    port_imgi.memID[ESTRIDE_3RD_PLANE]    = buf_imgci.memID;
    vPostProcInPorts.at(0)=&(port_imgi);
    //vipi
    buf_vipi.size=sizeof(g_yuv422_1plane_y_256_256_s512);
    mpImemDrv->allocVirtBuf(&buf_vipi);
    mpImemDrv->mapPhyAddr(&buf_vipi);
    memcpy( (MUINT8*)(buf_vipi.virtAddr), (MUINT8*)(g_yuv422_1plane_y_256_256_s512), buf_vipi.size);
    port_vipi.u4ImgWidth  = _vipi_w_;
    port_vipi.u4ImgHeight = _vipi_h_;
    port_vipi.resize1.tar_w = _vipi_w_;
    port_vipi.resize1.tar_h = _vipi_h_;
    port_vipi.crop1.x = 0;
    port_vipi.crop1.floatX = 0;
    port_vipi.crop1.y = 0;
    port_vipi.crop1.floatY = 0;
    port_vipi.crop1.w = _vipi_w_;
    port_vipi.crop1.h = _vipi_h_;
    port_vipi.eImgFmt     = eImgFmt_YUY2;
    port_vipi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_vipi.index       = EPortIndex_VIPI;
    port_vipi.inout       = 0; //in
    port_vipi.capbility   = 0;
    port_vipi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_vipi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_vipi.u4Stride[ESTRIDE_1ST_PLANE] = _vipi_w_;
    port_vipi.u4BufSize[ESTRIDE_1ST_PLANE]= buf_vipi.size;
    port_vipi.u4BufVA[ESTRIDE_1ST_PLANE]  = buf_vipi.virtAddr;
    port_vipi.u4BufPA[ESTRIDE_1ST_PLANE]  = buf_vipi.phyAddr;
    port_vipi.memID[ESTRIDE_1ST_PLANE]    = buf_vipi.memID;
    vPostProcInPorts.at(1)=&(port_vipi);

    buf_ufdi.size=sizeof(g_ufdi_256_256_byte8);
    mpImemDrv->allocVirtBuf(&buf_ufdi);
    mpImemDrv->mapPhyAddr(&buf_ufdi);
    memcpy( (MUINT8*)(buf_ufdi.virtAddr), (MUINT8*)(g_ufdi_256_256_byte8), buf_ufdi.size);
    port_ufdi.u4ImgWidth  = _ufdi_w_;
    port_ufdi.u4ImgHeight = _ufdi_h_;
    port_ufdi.resize1.tar_w = _ufdi_w_;
    port_ufdi.resize1.tar_h = _ufdi_h_;
    port_ufdi.crop1.x = 0;
    port_ufdi.crop1.floatX = 0;
    port_ufdi.crop1.y = 0;
    port_ufdi.crop1.floatY = 0;
    port_ufdi.crop1.w = _ufdi_w_;
    port_ufdi.crop1.h = _ufdi_h_;
    port_ufdi.eImgFmt     = eImgFmt_STA_BYTE;
    port_ufdi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_ufdi.index       = EPortIndex_UFDI;
    port_ufdi.inout       = 0; //in
    port_ufdi.capbility   = 0;
    port_ufdi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_ufdi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_ufdi.u4Stride[ESTRIDE_1ST_PLANE] = _ufdi_w_;
    port_ufdi.u4BufSize[ESTRIDE_1ST_PLANE]= buf_ufdi.size;
    port_ufdi.u4BufVA[ESTRIDE_1ST_PLANE]  = buf_ufdi.virtAddr;
    port_ufdi.u4BufPA[ESTRIDE_1ST_PLANE]  = buf_ufdi.phyAddr;
    port_ufdi.memID[ESTRIDE_1ST_PLANE]    = buf_ufdi.memID;
    vPostProcInPorts.at(2)=&(port_ufdi);
    buf_lcei.size=sizeof(g_lcei_256_256_byte8);
    mpImemDrv->allocVirtBuf(&buf_lcei);
    mpImemDrv->mapPhyAddr(&buf_lcei);
    memcpy( (MUINT8*)(buf_lcei.virtAddr), (MUINT8*)(g_lcei_256_256_byte8), buf_lcei.size);
    port_lcei.u4ImgWidth  = _lcei_w_;
    port_lcei.u4ImgHeight = _lcei_h_;
    port_lcei.resize1.tar_w = _lcei_w_;
    port_lcei.resize1.tar_h = _lcei_h_;
    port_lcei.crop1.x = 0;
    port_lcei.crop1.floatX = 0;
    port_lcei.crop1.y = 0;
    port_lcei.crop1.floatY = 0;
    port_lcei.crop1.w = _lcei_w_;
    port_lcei.crop1.h = _lcei_h_;
    port_lcei.eImgFmt     = eImgFmt_STA_BYTE;
    port_lcei.type        = NSImageio::NSIspio::EPortType_Memory;
    port_lcei.index       = EPortIndex_LCEI;
    port_lcei.inout       = 0; //in
    port_lcei.capbility   = 0;
    port_lcei.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_lcei.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_lcei.u4Stride[ESTRIDE_1ST_PLANE] = _lcei_w_;
    port_lcei.u4BufSize[ESTRIDE_1ST_PLANE]= buf_lcei.size;
    port_lcei.u4BufVA[ESTRIDE_1ST_PLANE]  = buf_lcei.virtAddr;
    port_lcei.u4BufPA[ESTRIDE_1ST_PLANE]  = buf_lcei.phyAddr;
    port_lcei.memID[ESTRIDE_1ST_PLANE]    = buf_lcei.memID;
    vPostProcInPorts.at(3)=&(port_lcei);
    //img2o
    buf_img2o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = _imgi_w_;
    port_img2o.u4ImgHeight = _imgi_h_;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = _imgi_w_*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);
    //img3o
    buf_img3o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img3o);
    mpImemDrv->mapPhyAddr(&buf_img3o);
    memset((MUINT8*)buf_img3o.virtAddr, 0xffffffff, buf_img3o.size);
    port_img3o.u4ImgWidth  = _imgi_w_;
    port_img3o.u4ImgHeight = _imgi_h_;
    port_img3o.eImgFmt     = eImgFmt_YUY2;
    port_img3o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img3o.index       = EPortIndex_IMG3O;
    port_img3o.inout       = 1; //out
    port_img3o.capbility   = 0;
    port_img3o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img3o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img3o.u4Stride[0] = _imgi_w_*2;
    port_img3o.u4BufSize[0]= buf_img3o.size;
    port_img3o.u4BufVA[0]  = buf_img3o.virtAddr;
    port_img3o.u4BufPA[0]  = buf_img3o.phyAddr;
    port_img3o.memID[0]    = buf_img3o.memID;
    vPostProcOutPorts.at(1)=&(port_img3o);
    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_VFB_FB;
    printf("###...do config \n");

    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);



//do{}while(1);//kk test


    printf("####################################################\n");
    //
    printf("Press any key to continue\n");
    s = getchar();

    printf("###...do start\n");
    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("[Error]###...P2 Start Fail!, ret(%d)\n",ret);
        return ret;
    }
    //
    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    printf("###...do deque img3o\n");
    portID.index=EPortIndex_IMG3O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img3o Fail!\n");
        return ret;
    }
    //
    printf("###...do deque imgi\n");
    portID.index=EPortIndex_IMGI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque imgi Fail!\n");
        return ret;
    }
    printf("###...do deque lcei\n");
    portID.index=EPortIndex_LCEI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque lcei Fail!\n");
        return ret;
    }
    printf("###...do deque ufdi\n");
    portID.index=EPortIndex_UFDI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque ufdi Fail!\n");
        return ret;
    }
    printf("###...do deque vipi\n");
    portID.index=EPortIndex_VIPI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque vipi Fail!\n");
        return ret;
    }
    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }

    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2B_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename2[256];
    sprintf(filename2, "/data/imageio_P2B_img3o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_img3o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    printf("###...save file done\n");


    //printf("buf_wroto.phyAddr 0x%x \n",buf_wroto.phyAddr);
    //printf("buf_wdmao.phyAddr 0x%x \n",buf_wdmao.phyAddr);

do{}while(1);//kk test




    //
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();
    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //

    return ret;

}



MINT32 P2B_YUV_TileMode()
{
    int ret = 0;
    IPostProcPipe* mpPostProcPipe;
    static vector<PortInfo const*> vPostProcInPorts(3);
    static vector<PortInfo const*> vPostProcOutPorts(4);
    PipePackageInfo pipePackageInfo;
    IMemDrv* mpImemDrv=NULL;
    IMEM_BUF_INFO buf_imgi,buf_imgbi,buf_imgci, buf_img2o, buf_img3o, buf_wroto, buf_wdmao;
    NSImageio::NSIspio::PortInfo port_imgi,port_imgbi,port_imgci, port_img2o, port_img3o, port_wroto, port_wdmao;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    int _imgi_w_=960, _imgi_h_=540;
    int _imgbi_w_=_imgi_w_/2, _imgbi_h_=_imgi_h_/2;
    int _imgci_w_=_imgi_w_/2, _imgci_h_=_imgi_h_/2;
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    char s;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("Test_P2B_YUV_Tile");
    //
    //imgi
    buf_imgi.size=sizeof(g_imgi_array_960x540_yuy2);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(g_imgi_array_960x540_yuy2), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
    port_imgi.resize1.tar_w = _imgi_w_;
    port_imgi.resize1.tar_h = _imgi_h_;
    port_imgi.crop1.x = 0;
    port_imgi.crop1.floatX = 0;
    port_imgi.crop1.y = 0;
    port_imgi.crop1.floatY = 0;
    port_imgi.crop1.w = _imgi_w_;
    port_imgi.crop1.h = _imgi_h_;
    port_imgi.eImgFmt     = eImgFmt_YUY2;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    //
    port_imgi.resize1.tar_w = _imgi_w_;
    port_imgi.resize1.tar_h = _imgi_h_;
    port_imgi.crop1.x = 0;
    port_imgi.crop1.floatX = 0;
    port_imgi.crop1.y = 0;
    port_imgi.crop1.floatY = 0;
    port_imgi.crop1.w = _imgi_w_;
    port_imgi.crop1.h = _imgi_h_;
    //
    port_imgi.resize2.tar_w = _imgi_w_;
    port_imgi.resize2.tar_h = _imgi_h_;
    port_imgi.crop2.x = 0;
    port_imgi.crop2.floatX = 0;
    port_imgi.crop2.y = 0;
    port_imgi.crop2.floatY = 0;
    port_imgi.crop2.w = _imgi_w_;
    port_imgi.crop2.h = _imgi_h_;
    //
    port_imgi.resize3.tar_w = _imgi_w_;
    port_imgi.resize3.tar_h = _imgi_h_;
    port_imgi.crop3.x = 0;
    port_imgi.crop3.floatX = 0;
    port_imgi.crop3.y = 0;
    port_imgi.crop3.floatY = 0;
    port_imgi.crop3.w = _imgi_w_;
    port_imgi.crop3.h = _imgi_h_;
    //
    port_imgi.u4Stride[ESTRIDE_1ST_PLANE] = _imgi_w_*2;
    port_imgi.u4BufSize[ESTRIDE_1ST_PLANE]= buf_imgi.size;
    port_imgi.u4BufVA[ESTRIDE_1ST_PLANE]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[ESTRIDE_1ST_PLANE]  = buf_imgi.phyAddr;
    port_imgi.memID[ESTRIDE_1ST_PLANE]    = buf_imgi.memID;
    //
    vPostProcInPorts.at(0)=&(port_imgi);
    //img2o
    buf_img2o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = _imgi_w_;
    port_img2o.u4ImgHeight = _imgi_h_;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = _imgi_w_*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);
    //img3o
    buf_img3o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img3o);
    mpImemDrv->mapPhyAddr(&buf_img3o);
    memset((MUINT8*)buf_img3o.virtAddr, 0xffffffff, buf_img3o.size);
    port_img3o.u4ImgWidth  = _imgi_w_;
    port_img3o.u4ImgHeight = _imgi_h_;
    port_img3o.eImgFmt     = eImgFmt_YUY2;
    port_img3o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img3o.index       = EPortIndex_IMG3O;
    port_img3o.inout       = 1; //out
    port_img3o.capbility   = 0;
    port_img3o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img3o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img3o.u4Stride[0] = _imgi_w_*2;
    port_img3o.u4BufSize[0]= buf_img3o.size;
    port_img3o.u4BufVA[0]  = buf_img3o.virtAddr;
    port_img3o.u4BufPA[0]  = buf_img3o.phyAddr;
    port_img3o.memID[0]    = buf_img3o.memID;
    vPostProcOutPorts.at(1)=&(port_img3o);
    //wroto
    buf_wroto.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_wroto);
    mpImemDrv->mapPhyAddr(&buf_wroto);
    memset((MUINT8*)buf_wroto.virtAddr, 0xffffffff, buf_wroto.size);
    port_wroto.u4ImgWidth  = _imgi_w_;
    port_wroto.u4ImgHeight = _imgi_h_;
    port_wroto.eImgFmt     = eImgFmt_YUY2;
    port_wroto.type        = NSImageio::NSIspio::EPortType_Memory;
    port_wroto.index       = EPortIndex_WROTO;
    port_wroto.inout       = 1; //out
    port_wroto.capbility   = 0;
    port_wroto.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_wroto.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_wroto.u4Stride[0] = _imgi_w_*2;
    port_wroto.u4BufSize[0]= buf_wroto.size;
    port_wroto.u4BufVA[0]  = buf_wroto.virtAddr;
    port_wroto.u4BufPA[0]  = buf_wroto.phyAddr;
    port_wroto.memID[0]    = buf_wroto.memID;
    vPostProcOutPorts.at(2)=&(port_wroto);
    //wdmao
    buf_wdmao.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_wdmao);
    mpImemDrv->mapPhyAddr(&buf_wdmao);
    memset((MUINT8*)buf_wdmao.virtAddr, 0xffffffff, buf_wdmao.size);
    port_wdmao.u4ImgWidth  = _imgi_w_;
    port_wdmao.u4ImgHeight = _imgi_h_;
    port_wdmao.eImgFmt     = eImgFmt_YUY2;
    port_wdmao.type        = NSImageio::NSIspio::EPortType_Memory;
    port_wdmao.index       = EPortIndex_WDMAO;
    port_wdmao.inout       = 1; //out
    port_wdmao.capbility   = 0;
    port_wdmao.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_wdmao.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_wdmao.u4Stride[0] = _imgi_w_*2;
    port_wdmao.u4BufSize[0]= buf_wdmao.size;
    port_wdmao.u4BufVA[0]  = buf_wdmao.virtAddr;
    port_wdmao.u4BufPA[0]  = buf_wdmao.phyAddr;
    port_wdmao.memID[0]    = buf_wdmao.memID;
    vPostProcOutPorts.at(3)=&(port_wdmao);
    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_VFB_FB;
    printf("###...do config \n");

    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);

    printf("####################################################\n");
    //
    printf("Press any key to continue\n");
    s = getchar();

    printf("###...do start\n");
    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("[Error]###...P2 Start Fail!, ret(%d)\n",ret);
        return ret;
    }
    //
    printf("###...do deque wdmao\n");
    portID.index=EPortIndex_WDMAO;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque wdmao Fail!\n");
        return ret;
    }
    //
    printf("###...do deque wroto\n");
    portID.index=EPortIndex_WROTO;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque wroto Fail!\n");
        return ret;
    }
    printf("###...do deque img3o\n");
    portID.index=EPortIndex_IMG3O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img3o Fail!\n");
        return ret;
    }
    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    
    printf("###...do deque imgi\n");
    portID.index=EPortIndex_IMGI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque imgi Fail!\n");
        return ret;
    }

    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }

    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2B_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename2[256];
    sprintf(filename2, "/data/imageio_P2B_img3o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_img3o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename3[256];
    sprintf(filename3, "/data/imageio_P2B_wroto_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename3, reinterpret_cast<MUINT8*>(buf_wroto.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename4[256];
    sprintf(filename4, "/data/imageio_P2B_wdmao_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename4, reinterpret_cast<MUINT8*>(buf_wdmao.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    printf("###...save file done\n");


    //printf("buf_wroto.phyAddr 0x%x \n",buf_wroto.phyAddr);
    //printf("buf_wdmao.phyAddr 0x%x \n",buf_wdmao.phyAddr);

    do{}while(1);//kk test



    //
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();
    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //

    return ret;

}


#include "DIP_pics/imgi_640x480_bayer10.h"
#include "DIP_pics/imgi_1280x720_bayer10.h"   // for test that image be separate into multi-tile

MINT32 P2A_RAW()
{
    int ret = 0;
    IPostProcPipe* mpPostProcPipe;
    static vector<PortInfo const*> vPostProcInPorts(3);
    static vector<PortInfo const*> vPostProcOutPorts(4);
    PipePackageInfo pipePackageInfo;
    IMemDrv* mpImemDrv=NULL;
    IMEM_BUF_INFO buf_imgi, buf_img2o, buf_img3o, buf_wroto, buf_wdmao;
    NSImageio::NSIspio::PortInfo port_imgi, port_img2o, port_img3o, port_wroto, port_wdmao;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    int _imgi_w_=1280, _imgi_h_=720;
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    char s;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("Test_PostProcPipe");
    //
    //imgi
    buf_imgi.size=sizeof(g_imgi_array_1280x720_b10);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
	port_imgi.resize1.tar_w = _imgi_w_;
	port_imgi.resize1.tar_h = _imgi_h_;
	port_imgi.crop1.x = 0;
	port_imgi.crop1.floatX = 0;
	port_imgi.crop1.y = 0;
	port_imgi.crop1.floatY = 0;
	port_imgi.crop1.w = _imgi_w_;
	port_imgi.crop1.h = _imgi_h_;
    port_imgi.eImgFmt     = eImgFmt_BAYER10;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    //
    port_imgi.resize2.tar_w = _imgi_w_;
    port_imgi.resize2.tar_h = _imgi_h_;
	port_imgi.crop2.x = 0;
	port_imgi.crop2.floatX = 0;
	port_imgi.crop2.y = 0;
	port_imgi.crop2.floatY = 0;
	port_imgi.crop2.w = _imgi_w_;
	port_imgi.crop2.h = _imgi_h_;
    //
    port_imgi.resize3.tar_w = _imgi_w_;
    port_imgi.resize3.tar_h = _imgi_h_;
	port_imgi.crop3.x = 0;
	port_imgi.crop3.floatX = 0;
	port_imgi.crop3.y = 0;
	port_imgi.crop3.floatY = 0;
	port_imgi.crop3.w = _imgi_w_;
	port_imgi.crop3.h = _imgi_h_;
    //
    port_imgi.u4Stride[0] = _imgi_w_*10/8;
    port_imgi.u4BufSize[0]= buf_imgi.size;
    port_imgi.u4BufVA[0]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[0]  = buf_imgi.phyAddr;
    port_imgi.memID[0]    = buf_imgi.memID;
    vPostProcInPorts.at(0)=&(port_imgi);
    //img2o
    buf_img2o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = _imgi_w_;
    port_img2o.u4ImgHeight = _imgi_h_;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = _imgi_w_*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);
    //img3o
    buf_img3o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img3o);
    mpImemDrv->mapPhyAddr(&buf_img3o);
    memset((MUINT8*)buf_img3o.virtAddr, 0xffffffff, buf_img3o.size);
    port_img3o.u4ImgWidth  = _imgi_w_;
    port_img3o.u4ImgHeight = _imgi_h_;
    port_img3o.eImgFmt     = eImgFmt_YUY2;
    port_img3o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img3o.index       = EPortIndex_IMG3O;
    port_img3o.inout       = 1; //out
    port_img3o.capbility   = 0;
    port_img3o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img3o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img3o.u4Stride[0] = _imgi_w_*2;
    port_img3o.u4BufSize[0]= buf_img3o.size;
    port_img3o.u4BufVA[0]  = buf_img3o.virtAddr;
    port_img3o.u4BufPA[0]  = buf_img3o.phyAddr;
    port_img3o.memID[0]    = buf_img3o.memID;
    vPostProcOutPorts.at(1)=&(port_img3o);
    //wroto
    buf_wroto.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_wroto);
    mpImemDrv->mapPhyAddr(&buf_wroto);
    memset((MUINT8*)buf_wroto.virtAddr, 0xffffffff, buf_wroto.size);
    port_wroto.u4ImgWidth  = _imgi_w_;
    port_wroto.u4ImgHeight = _imgi_h_;
    port_wroto.eImgFmt     = eImgFmt_YUY2;
    port_wroto.type        = NSImageio::NSIspio::EPortType_Memory;
    port_wroto.index       = EPortIndex_WROTO;
    port_wroto.inout       = 1; //out
    port_wroto.capbility   = 0;
    port_wroto.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_wroto.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_wroto.u4Stride[0] = _imgi_w_*2;
    port_wroto.u4BufSize[0]= buf_wroto.size;
    port_wroto.u4BufVA[0]  = buf_wroto.virtAddr;
    port_wroto.u4BufPA[0]  = buf_wroto.phyAddr;
    port_wroto.memID[0]    = buf_wroto.memID;
    vPostProcOutPorts.at(2)=&(port_wroto);
    //wdmao
    buf_wdmao.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_wdmao);
    mpImemDrv->mapPhyAddr(&buf_wdmao);
    memset((MUINT8*)buf_wdmao.virtAddr, 0xffffffff, buf_wdmao.size);
    port_wdmao.u4ImgWidth  = _imgi_w_;
    port_wdmao.u4ImgHeight = _imgi_h_;
    port_wdmao.eImgFmt     = eImgFmt_YUY2;
    port_wdmao.type        = NSImageio::NSIspio::EPortType_Memory;
    port_wdmao.index       = EPortIndex_WDMAO;
    port_wdmao.inout       = 1; //out
    port_wdmao.capbility   = 0;
    port_wdmao.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_wdmao.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_wdmao.u4Stride[0] = _imgi_w_*2;
    port_wdmao.u4BufSize[0]= buf_wdmao.size;
    port_wdmao.u4BufVA[0]  = buf_wdmao.virtAddr;
    port_wdmao.u4BufPA[0]  = buf_wdmao.phyAddr;
    port_wdmao.memID[0]    = buf_wdmao.memID;
    vPostProcOutPorts.at(3)=&(port_wdmao);
    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_P2A;
    printf("###...do config \n");

    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);


    printf("####################################################\n");
    //
    printf("Press any key to continue\n");
    s = getchar();


    printf("###...do start\n");
    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("[Error]###...P2 Start Fail!, ret(%d)\n",ret);
        return ret;
    }
    //
    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    printf("###...do deque img3o\n");
    portID.index=EPortIndex_IMG3O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img3o Fail!\n");
        return ret;
    }
    printf("###...do deque wroto\n");
    portID.index=EPortIndex_WROTO;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque wroto Fail!\n");
        return ret;
    }
    printf("###...do deque wdmao\n");
    portID.index=EPortIndex_WDMAO;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque wdmao Fail!\n");
        return ret;
    }
    printf("###...do deque imgi\n");
    portID.index=EPortIndex_IMGI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque imgi Fail!\n");
        return ret;
    }

    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }

    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2ARAW_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename2[256];
    sprintf(filename2, "/data/imageio_P2ARAW_img3o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_img3o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename3[256];
    sprintf(filename3, "/data/imageio_P2ARAW_wroto_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename3, reinterpret_cast<MUINT8*>(buf_wroto.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename4[256];
    sprintf(filename4, "/data/imageio_P2ARAW_wdmao_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename4, reinterpret_cast<MUINT8*>(buf_wdmao.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    printf("###...save file done\n");

    printf("buf_img2o.phyAddr 0x%lu \n",buf_img2o.phyAddr);
    printf("buf_img3o.phyAddr 0x%lu \n",buf_img3o.phyAddr);
    printf("buf_wroto.phyAddr 0x%lu \n",buf_wroto.phyAddr);
    printf("buf_wdmao.phyAddr 0x%lu \n",buf_wdmao.phyAddr);



do{}while(1);//kk test

    //
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();
    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //

    return ret;
}

#include "DIP_pics/P2A_FG/imgi_640_480_10.h" //640x480, fg 10 bit
MINT32 P2A_FG()
{
    int ret=0;
    IPostProcPipe* mpPostProcPipe;
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("P2A_FG");
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    printf("####################################################\n");
    //data preparation
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    vector<NSImageio::NSIspio::PortInfo const*> vPostProcInPorts;
    vector<NSImageio::NSIspio::PortInfo const*> vPostProcOutPorts;
    vPostProcInPorts.resize(1);
    vPostProcOutPorts.resize(2);
    NSImageio::NSIspio::PipePackageInfo pipePackageInfo;
    //
    int _imgi_w_=640, _imgi_h_=480;
    NSImageio::NSIspio::PortInfo port_imgi, port_img2o, port_img3o;
    IMEM_BUF_INFO buf_imgi, buf_img2o, buf_img3o;
    //
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;

    //imgi
    buf_imgi.size=sizeof(p2a_fg_g_imgi_array_640_480_10);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(p2a_fg_g_imgi_array_640_480_10), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
    port_imgi.resize1.tar_w = _imgi_w_;
    port_imgi.resize1.tar_h = _imgi_h_;
	port_imgi.crop1.x = 0;
	port_imgi.crop1.floatX = 0;
	port_imgi.crop1.y = 0;
	port_imgi.crop1.floatY = 0;
	port_imgi.crop1.w = _imgi_w_;
	port_imgi.crop1.h = _imgi_h_;
    port_imgi.eImgFmt     = eImgFmt_FG_BAYER10;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_imgi.u4Stride[0] = (_imgi_w_*10/8) * 3 / 2;
    port_imgi.u4BufSize[0]= buf_imgi.size;
    port_imgi.u4BufVA[0]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[0]  = buf_imgi.phyAddr;
    port_imgi.memID[0]    = buf_imgi.memID;
    vPostProcInPorts.at(0)=&(port_imgi);
    //img2o
    buf_img2o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = _imgi_w_;
    port_img2o.u4ImgHeight = _imgi_h_;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = _imgi_w_*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);
    //img3o
    buf_img3o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img3o);
    mpImemDrv->mapPhyAddr(&buf_img3o);
    memset((MUINT8*)buf_img3o.virtAddr, 0xffffffff, buf_img3o.size);
    port_img3o.u4ImgWidth  = _imgi_w_;
    port_img3o.u4ImgHeight = _imgi_h_;
    port_img3o.eImgFmt     = eImgFmt_YUY2;
    port_img3o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img3o.index       = EPortIndex_IMG3O;
    port_img3o.inout       = 1; //out
    port_img3o.capbility   = 0;
    port_img3o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img3o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img3o.u4Stride[0] = _imgi_w_*2;
    port_img3o.u4BufSize[0]= buf_img3o.size;
    port_img3o.u4BufVA[0]  = buf_img3o.virtAddr;
    port_img3o.u4BufPA[0]  = buf_img3o.phyAddr;
    port_img3o.memID[0]    = buf_img3o.memID;
    vPostProcOutPorts.at(1)=&(port_img3o);
    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_P2A;
    printf("###...do config \n");
    ret=mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);
    if(!ret)
    {
        printf("###...postprocPipe config fail\n");
        return ret;
    }
    printf("###...do start\n");
    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("###...P2 Start Fail!\n");
        return ret;
    }

    //printf("###...temp enter while to monitor CVD\n");
    //while(1);

    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    printf("###...do deque img3o\n");
    portID.index=EPortIndex_IMG3O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img3o Fail!\n");
        return ret;
    }
    printf("###...do deque imgi\n");
    portID.index=EPortIndex_IMGI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque imgi Fail!\n");
        return ret;
    }

    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }

    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2AFG_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename2[256];
    sprintf(filename2, "/data/imageio_P2AFG_img3o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_img3o.virtAddr), _imgi_w_ *_imgi_h_ * 2);

    printf("###...save file done\n");

    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpPostProcPipe->uninit("P2A_FG");
    mpPostProcPipe->destroyInstance();
    return ret;
}

#include "DIP_pics/FM/case1/depi.h"
#include "DIP_pics/FM/case1/dmgi.h"
#include "DIP_pics/FM/case1/mfbo_a.h"
MINT32 P2A_FM()
{
    int ret = 0;
    IPostProcPipe* mpPostProcPipe;
    static vector<PortInfo const*> vPostProcInPorts(2);
    static vector<PortInfo const*> vPostProcOutPorts(1);
    vPostProcInPorts.resize(2);
    vPostProcOutPorts.resize(1);
    PipePackageInfo pipePackageInfo;
    dip_x_reg_t *pIspPhyReg;
    IMEM_BUF_INFO pTuningQueBuf;
    IMemDrv* mpImemDrv=NULL;
    IMEM_BUF_INFO buf_imgi, buf_img2o, buf_img3o, buf_wroto, buf_wdmao, buf_depi, buf_dmgi, buf_mfbo;
    NSImageio::NSIspio::PortInfo port_imgi, port_img2o, port_img3o, port_wroto, port_wdmao, port_depi, port_dmgi, port_mfbo;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    int _imgi_w_=1280, _imgi_h_=720;
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    char s;
    
    int _depi_w_=4000, _depi_h_=56;
    int _dmgi_w_=4000, _dmgi_h_=56;
    int _mfbo_w_=200, _mfbo_h_=56;
    int _depi_stride_ = 4000, _dmgi_stride_ = 4000, _mfbo_stride_ = 200;

    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
	pTuningQueBuf.size = sizeof(dip_x_reg_t);
    mpImemDrv->allocVirtBuf(&pTuningQueBuf);
    mpImemDrv->mapPhyAddr(&pTuningQueBuf);
    printf("Tuning buffer VA address %lu \n",pTuningQueBuf.virtAddr);
    
    //
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("P2A_FM");
    //
#if 0
    //imgi
    buf_imgi.size=sizeof(g_imgi_array_1280x720_b10);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
	port_imgi.resize1.tar_w = _imgi_w_;
	port_imgi.resize1.tar_h = _imgi_h_;
	port_imgi.crop1.x = 0;
	port_imgi.crop1.floatX = 0;
	port_imgi.crop1.y = 0;
	port_imgi.crop1.floatY = 0;
	port_imgi.crop1.w = _imgi_w_;
	port_imgi.crop1.h = _imgi_h_;
    port_imgi.eImgFmt     = eImgFmt_BAYER10;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    //
    port_imgi.resize2.tar_w = _imgi_w_;
    port_imgi.resize2.tar_h = _imgi_h_;
	port_imgi.crop2.x = 0;
	port_imgi.crop2.floatX = 0;
	port_imgi.crop2.y = 0;
	port_imgi.crop2.floatY = 0;
	port_imgi.crop2.w = _imgi_w_;
	port_imgi.crop2.h = _imgi_h_;
    //
    port_imgi.resize3.tar_w = _imgi_w_;
    port_imgi.resize3.tar_h = _imgi_h_;
	port_imgi.crop3.x = 0;
	port_imgi.crop3.floatX = 0;
	port_imgi.crop3.y = 0;
	port_imgi.crop3.floatY = 0;
	port_imgi.crop3.w = _imgi_w_;
	port_imgi.crop3.h = _imgi_h_;
    //
    port_imgi.u4Stride[0] = _imgi_w_*10/8;
    port_imgi.u4BufSize[0]= buf_imgi.size;
    port_imgi.u4BufVA[0]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[0]  = buf_imgi.phyAddr;
    port_imgi.memID[0]    = buf_imgi.memID;
    vPostProcInPorts.at(0)=&(port_imgi);
#endif
    //depi
    buf_depi.size=sizeof(g_depi_array);
    mpImemDrv->allocVirtBuf(&buf_depi);
    mpImemDrv->mapPhyAddr(&buf_depi);
    memcpy( (MUINT8*)(buf_depi.virtAddr), (MUINT8*)(g_depi_array), buf_depi.size);
    port_depi.u4ImgWidth  = _depi_w_;
    port_depi.u4ImgHeight = _depi_h_;
    port_depi.eImgFmt     = eImgFmt_STA_BYTE;
    port_depi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_depi.index       = EPortIndex_DEPI;
    port_depi.inout       = 0; //in
    port_depi.capbility   = 0;
    port_depi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_depi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_depi.u4Stride[0] = _depi_stride_;
    port_depi.u4BufSize[0]= buf_depi.size;
    port_depi.u4BufVA[0]  = buf_depi.virtAddr;
    port_depi.u4BufPA[0]  = buf_depi.phyAddr;
    port_depi.memID[0]    = buf_depi.memID;
    vPostProcInPorts.at(0)=&(port_depi);
    //dmgi
    buf_dmgi.size=sizeof(g_dmgi_array);
    mpImemDrv->allocVirtBuf(&buf_dmgi);
    mpImemDrv->mapPhyAddr(&buf_dmgi);
    memcpy( (MUINT8*)(buf_dmgi.virtAddr), (MUINT8*)(g_dmgi_array), buf_dmgi.size);
    port_dmgi.u4ImgWidth  = _dmgi_w_;
    port_dmgi.u4ImgHeight = _dmgi_h_;
    port_dmgi.eImgFmt     = eImgFmt_STA_BYTE;
    port_dmgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_dmgi.index       = EPortIndex_DMGI;
    port_dmgi.inout       = 0; //in
    port_dmgi.capbility   = 0;
    port_dmgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_dmgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_dmgi.u4Stride[0] = _dmgi_stride_;
    port_dmgi.u4BufSize[0]= buf_dmgi.size;
    port_dmgi.u4BufVA[0]  = buf_dmgi.virtAddr;
    port_dmgi.u4BufPA[0]  = buf_dmgi.phyAddr;
    port_dmgi.memID[0]    = buf_dmgi.memID;
    vPostProcInPorts.at(1)=&(port_dmgi);
#if 0
    //img2o
    buf_img2o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = _imgi_w_;
    port_img2o.u4ImgHeight = _imgi_h_;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = _imgi_w_*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);
    //img3o
    buf_img3o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img3o);
    mpImemDrv->mapPhyAddr(&buf_img3o);
    memset((MUINT8*)buf_img3o.virtAddr, 0xffffffff, buf_img3o.size);
    port_img3o.u4ImgWidth  = _imgi_w_;
    port_img3o.u4ImgHeight = _imgi_h_;
    port_img3o.eImgFmt     = eImgFmt_YUY2;
    port_img3o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img3o.index       = EPortIndex_IMG3O;
    port_img3o.inout       = 1; //out
    port_img3o.capbility   = 0;
    port_img3o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img3o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img3o.u4Stride[0] = _imgi_w_*2;
    port_img3o.u4BufSize[0]= buf_img3o.size;
    port_img3o.u4BufVA[0]  = buf_img3o.virtAddr;
    port_img3o.u4BufPA[0]  = buf_img3o.phyAddr;
    port_img3o.memID[0]    = buf_img3o.memID;
    vPostProcOutPorts.at(1)=&(port_img3o);
    //wroto
    buf_wroto.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_wroto);
    mpImemDrv->mapPhyAddr(&buf_wroto);
    memset((MUINT8*)buf_wroto.virtAddr, 0xffffffff, buf_wroto.size);
    port_wroto.u4ImgWidth  = _imgi_w_;
    port_wroto.u4ImgHeight = _imgi_h_;
    port_wroto.eImgFmt     = eImgFmt_YUY2;
    port_wroto.type        = NSImageio::NSIspio::EPortType_Memory;
    port_wroto.index       = EPortIndex_WROTO;
    port_wroto.inout       = 1; //out
    port_wroto.capbility   = 0;
    port_wroto.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_wroto.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_wroto.u4Stride[0] = _imgi_w_*2;
    port_wroto.u4BufSize[0]= buf_wroto.size;
    port_wroto.u4BufVA[0]  = buf_wroto.virtAddr;
    port_wroto.u4BufPA[0]  = buf_wroto.phyAddr;
    port_wroto.memID[0]    = buf_wroto.memID;
    vPostProcOutPorts.at(2)=&(port_wroto);
    //wdmao
    buf_wdmao.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_wdmao);
    mpImemDrv->mapPhyAddr(&buf_wdmao);
    memset((MUINT8*)buf_wdmao.virtAddr, 0xffffffff, buf_wdmao.size);
    port_wdmao.u4ImgWidth  = _imgi_w_;
    port_wdmao.u4ImgHeight = _imgi_h_;
    port_wdmao.eImgFmt     = eImgFmt_YUY2;
    port_wdmao.type        = NSImageio::NSIspio::EPortType_Memory;
    port_wdmao.index       = EPortIndex_WDMAO;
    port_wdmao.inout       = 1; //out
    port_wdmao.capbility   = 0;
    port_wdmao.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_wdmao.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_wdmao.u4Stride[0] = _imgi_w_*2;
    port_wdmao.u4BufSize[0]= buf_wdmao.size;
    port_wdmao.u4BufVA[0]  = buf_wdmao.virtAddr;
    port_wdmao.u4BufPA[0]  = buf_wdmao.phyAddr;
    port_wdmao.memID[0]    = buf_wdmao.memID;
    vPostProcOutPorts.at(3)=&(port_wdmao);
#endif
    //mfbo
    buf_mfbo.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_mfbo);
    mpImemDrv->mapPhyAddr(&buf_mfbo);
    memset((MUINT8*)buf_mfbo.virtAddr, 0xffffffff, buf_mfbo.size);
    port_mfbo.u4ImgWidth  = _mfbo_w_;
    port_mfbo.u4ImgHeight = _mfbo_h_;
    port_mfbo.eImgFmt     = eImgFmt_STA_BYTE;
    port_mfbo.type        = NSImageio::NSIspio::EPortType_Memory;
    port_mfbo.index       = EPortIndex_MFBO;
    port_mfbo.inout       = 1; //out
    port_mfbo.capbility   = 0;
    port_mfbo.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_mfbo.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_mfbo.u4Stride[0] = _mfbo_stride_;
    port_mfbo.u4BufSize[0]= buf_mfbo.size;
    port_mfbo.u4BufVA[0]  = buf_mfbo.virtAddr;
    port_mfbo.u4BufPA[0]  = buf_mfbo.phyAddr;
    port_mfbo.memID[0]    = buf_mfbo.memID;
    vPostProcOutPorts.at(0)=&(port_mfbo);
    //
    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
	pipePackageInfo.pTuningQue = (MVOID* )pTuningQueBuf.virtAddr;

    //Tuning Value Setting
    pIspPhyReg = (dip_x_reg_t *)pTuningQueBuf.virtAddr;
    pIspPhyReg->DIP_X_CTL_YUV2_EN.Raw=0x00000001;
    pIspPhyReg->DIP_X_FM_SIZE.Raw=0x38640F09;

    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_FM;
    printf("###...do config \n");

    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);


    printf("####################################################\n");
    //
    printf("Press any key to continue\n");
    s = getchar();


    printf("###...do start\n");

    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("[Error]###...P2 Start Fail!, ret(%d)\n",ret);
        return ret;
    }
    //
#if 0
    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    printf("###...do deque img3o\n");
    portID.index=EPortIndex_IMG3O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img3o Fail!\n");
        return ret;
    }
    printf("###...do deque wroto\n");
    portID.index=EPortIndex_WROTO;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque wroto Fail!\n");
        return ret;
    }
    printf("###...do deque wdmao\n");
    portID.index=EPortIndex_WDMAO;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque wdmao Fail!\n");
        return ret;
    }
    printf("###...do deque imgi\n");
    portID.index=EPortIndex_IMGI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque imgi Fail!\n");
        return ret;
    }
#endif
    printf("###...do deque mfbo\n");
    portID.index=EPortIndex_MFBO;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque mfbo Fail!\n");
        return ret;
    }
    printf("###...do deque depi\n");
    portID.index=EPortIndex_DEPI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque depi Fail!\n");
        return ret;
    }
    printf("###...do deque dmgi\n");
    portID.index=EPortIndex_DMGI;
    ret=mpPostProcPipe->dequeInBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque dmgi Fail!\n");
        return ret;
    }

    // should open after MDP Ready
    /*
    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }
    */
#if 0
    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2ARAW_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename2[256];
    sprintf(filename2, "/data/imageio_P2ARAW_img3o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename2, reinterpret_cast<MUINT8*>(buf_img3o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename3[256];
    sprintf(filename3, "/data/imageio_P2ARAW_wroto_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename3, reinterpret_cast<MUINT8*>(buf_wroto.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    char filename4[256];
    sprintf(filename4, "/data/imageio_P2ARAW_wdmao_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename4, reinterpret_cast<MUINT8*>(buf_wdmao.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    printf("###...save file done\n");

    printf("buf_img2o.phyAddr 0x%x \n",buf_img2o.phyAddr);
    printf("buf_img3o.phyAddr 0x%x \n",buf_img3o.phyAddr);
    printf("buf_wroto.phyAddr 0x%x \n",buf_wroto.phyAddr);
    printf("buf_wdmao.phyAddr 0x%x \n",buf_wdmao.phyAddr);
#endif



//do{}while(1);//kk test

    //
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();
    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //

    return ret;
}

MINT32 P2A_CRZ1()
{
    int ret = 0;
    IPostProcPipe* mpPostProcPipe;
    static vector<PortInfo const*> vPostProcInPorts(3);
    static vector<PortInfo const*> vPostProcOutPorts(3);
    PipePackageInfo pipePackageInfo;
    IMemDrv* mpImemDrv=NULL;
    IMEM_BUF_INFO buf_imgi, buf_img2o, buf_img3o;
    NSImageio::NSIspio::PortInfo port_imgi, port_img2o, port_img3o;
    int _imgi_w_=640, _imgi_h_=480;
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    char s;
    MUINT32 RW_Path;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("Test_PostProcPipe");
    //
    //imgi
    buf_imgi.size=sizeof(g_imgi_array_640x480_b10);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(g_imgi_array_640x480_b10), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
    port_imgi.resize1.tar_w = _imgi_w_;
    port_imgi.resize1.tar_h = _imgi_h_;
	port_imgi.crop1.x = 0;
	port_imgi.crop1.floatX = 0;
	port_imgi.crop1.y = 0;
	port_imgi.crop1.floatY = 0;
	port_imgi.crop1.w = _imgi_w_;
	port_imgi.crop1.h = _imgi_h_;
    port_imgi.eImgFmt     = eImgFmt_BAYER10;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_imgi.u4Stride[0] = _imgi_w_*10/8;
    port_imgi.u4BufSize[0]= buf_imgi.size;
    port_imgi.u4BufVA[0]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[0]  = buf_imgi.phyAddr;
    port_imgi.memID[0]    = buf_imgi.memID;
    vPostProcInPorts.at(0)=&(port_imgi);
    //img2o
    buf_img2o.size=_imgi_w_*_imgi_h_*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = _imgi_w_;
    port_img2o.u4ImgHeight = _imgi_h_;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = _imgi_w_*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);

    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_P2A;
    printf("###...do config \n");

    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);


    printf("####################################################\n");
    //
    printf("Press any key to continue\n");
    s = getchar();


    printf("###...do start\n");
    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("###...P2 Start Fail!\n");
        return ret;
    }

    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }

    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2A_CRZ1_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
    printf("###...save file done\n");


    printf("Press any key to continue\n");
    s = getchar();

    //
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();
    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //

    return ret;
}


MINT32 P2A_CRZ2()
{
    int ret = 0;
    IPostProcPipe* mpPostProcPipe;
    static vector<PortInfo const*> vPostProcInPorts(3);
    static vector<PortInfo const*> vPostProcOutPorts(3);
    PipePackageInfo pipePackageInfo;
    IMemDrv* mpImemDrv=NULL;
    IMEM_BUF_INFO buf_imgi, buf_img2o, buf_img3o;
    NSImageio::NSIspio::PortInfo port_imgi, port_img2o, port_img3o;
    int _imgi_w_=640, _imgi_h_=480;
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    char s;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("Test_PostProcPipe");
    //
    //imgi
    buf_imgi.size=sizeof(g_imgi_array_640x480_b10);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(g_imgi_array_640x480_b10), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
    port_imgi.resize1.tar_w = 320;
    port_imgi.resize1.tar_h = 240;
	port_imgi.crop1.x = 0;
	port_imgi.crop1.floatX = 0;
	port_imgi.crop1.y = 0;
	port_imgi.crop1.floatY = 0;
	port_imgi.crop1.w = _imgi_w_;
	port_imgi.crop1.h = _imgi_h_;
    port_imgi.eImgFmt     = eImgFmt_BAYER10;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_imgi.u4Stride[0] = _imgi_w_*10/8;
    port_imgi.u4BufSize[0]= buf_imgi.size;
    port_imgi.u4BufVA[0]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[0]  = buf_imgi.phyAddr;
    port_imgi.memID[0]    = buf_imgi.memID;
    vPostProcInPorts.at(0)=&(port_imgi);
    //img2o
    buf_img2o.size=320*240*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = 320;
    port_img2o.u4ImgHeight = 240;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = 320*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);

    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_P2A;
    printf("###...do config \n");

    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);


    printf("####################################################\n");
    //
    printf("Press any key to continue\n");
    s = getchar();


    printf("###...do start\n");
    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("###...P2 Start Fail!\n");
        return ret;
    }

    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }

    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2A_CRZ2_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
        printf("###...save file done\n");

    printf("Press any key to continue\n");
    s = getchar();

    //
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();
    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //

    return ret;
}


MINT32 P2A_CRZ3()
{
    int ret = 0;
    IPostProcPipe* mpPostProcPipe;
    static vector<PortInfo const*> vPostProcInPorts(3);
    static vector<PortInfo const*> vPostProcOutPorts(3);
    PipePackageInfo pipePackageInfo;
    IMemDrv* mpImemDrv=NULL;
    IMEM_BUF_INFO buf_imgi, buf_img2o, buf_img3o;
    NSImageio::NSIspio::PortInfo port_imgi, port_img2o, port_img3o;
    int _imgi_w_=640, _imgi_h_=480;
    MUINT32 p2CQ=0x0, p2dupCQ=0x0;
    char s;
    NSImageio::NSIspio::PortID portID;
    NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
    //
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //
    mpPostProcPipe = IPostProcPipe::createInstance();
    mpPostProcPipe->init("Test_PostProcPipe");
    //
    //imgi
    buf_imgi.size=sizeof(g_imgi_array_640x480_b10);
    mpImemDrv->allocVirtBuf(&buf_imgi);
    mpImemDrv->mapPhyAddr(&buf_imgi);
    memcpy( (MUINT8*)(buf_imgi.virtAddr), (MUINT8*)(g_imgi_array_640x480_b10), buf_imgi.size);
    port_imgi.u4ImgWidth  = _imgi_w_;
    port_imgi.u4ImgHeight = _imgi_h_;
    port_imgi.resize1.tar_w = 320;
    port_imgi.resize1.tar_h = 240;
	port_imgi.crop1.x = 120;
	port_imgi.crop1.floatX = 0;
	port_imgi.crop1.y = 140;
	port_imgi.crop1.floatY = 0;
	port_imgi.crop1.w = 400;
	port_imgi.crop1.h = 200;
    port_imgi.eImgFmt     = eImgFmt_BAYER10;
    port_imgi.type        = NSImageio::NSIspio::EPortType_Memory;
    port_imgi.index       = EPortIndex_IMGI;
    port_imgi.inout       = 0; //in
    port_imgi.capbility   = 0;
    port_imgi.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_imgi.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_imgi.u4Stride[0] = _imgi_w_*10/8;
    port_imgi.u4BufSize[0]= buf_imgi.size;
    port_imgi.u4BufVA[0]  = buf_imgi.virtAddr;
    port_imgi.u4BufPA[0]  = buf_imgi.phyAddr;
    port_imgi.memID[0]    = buf_imgi.memID;
    vPostProcInPorts.at(0)=&(port_imgi);
    //img2o
    buf_img2o.size=320*240*2;
    mpImemDrv->allocVirtBuf(&buf_img2o);
    mpImemDrv->mapPhyAddr(&buf_img2o);
    memset((MUINT8*)buf_img2o.virtAddr, 0xffffffff, buf_img2o.size);
    port_img2o.u4ImgWidth  = 320;
    port_img2o.u4ImgHeight = 240;
    port_img2o.eImgFmt     = eImgFmt_YUY2;
    port_img2o.type        = NSImageio::NSIspio::EPortType_Memory;
    port_img2o.index       = EPortIndex_IMG2O;
    port_img2o.inout       = 1; //out
    port_img2o.capbility   = 0;
    port_img2o.eImgRot     = NSImageio::NSIspio::eImgRot_0;
    port_img2o.eImgFlip    = NSImageio::NSIspio::eImgFlip_OFF;
    port_img2o.u4Stride[0] = 320*2;
    port_img2o.u4BufSize[0]= buf_img2o.size;
    port_img2o.u4BufVA[0]  = buf_img2o.virtAddr;
    port_img2o.u4BufPA[0]  = buf_img2o.phyAddr;
    port_img2o.memID[0]    = buf_img2o.memID;
    vPostProcOutPorts.at(0)=&(port_img2o);

    //
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=p2dupCQ;
    pipePackageInfo.p2cqIdx=p2CQ;
    //pipePackageInfo.vModuleParams=mvModule;
    //pipePackageInfo.pTuningQue = framePack.rParams.mvTuningData[q];
    pipePackageInfo.pixIdP2 = 0;
    pipePackageInfo.drvScen=NSImageio::NSIspio::eDrvScenario_P2A;
    printf("###...do config \n");

    mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts,&pipePackageInfo);


    printf("####################################################\n");
    //
    printf("Press any key to continue\n");
    s = getchar();


    printf("###...do start\n");
    ret=mpPostProcPipe->start();
    if(!ret)
    {
        printf("###...P2 Start Fail!\n");
        return ret;
    }

    printf("###...do deque img2o\n");
    portID.index=EPortIndex_IMG2O;
    ret=mpPostProcPipe->dequeOutBuf(portID, rQTSBufInfo, pipePackageInfo.drvScen, pipePackageInfo.p2cqIdx,pipePackageInfo.burstQIdx, pipePackageInfo.dupCqIdx);
    if(ret<0)
    {
        printf("###...deque img2o Fail!\n");
        return ret;
    }
    ret=mpPostProcPipe->dequeMdpFrameEnd(pipePackageInfo.drvScen);
    if(!ret)
    {
         printf("###...dequeMdpFrameEnd fail!\n");
        return ret;
    }

    //dump image
    char filename[256];
    sprintf(filename, "/data/imageio_P2A_CRZ3_img2o_%dx%d.yuv", _imgi_w_,_imgi_h_);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(buf_img2o.virtAddr), _imgi_w_ *_imgi_h_ * 2);
        printf("###...save file done\n");

    printf("Press any key to continue\n");
    s = getchar();

    //
    mpPostProcPipe->uninit("Test_PostProcPipe");
    mpPostProcPipe->destroyInstance();
    //
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    //

    return ret;
}

