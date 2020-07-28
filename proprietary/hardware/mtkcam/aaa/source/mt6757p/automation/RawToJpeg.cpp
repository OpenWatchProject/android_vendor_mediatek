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

#define LOG_TAG "RawToJpeg"

#include "IRawToJpeg.h"

#if (ISP_AUTOMATION_FUNC_EN)


#include <iostream>
#include <aaa_log.h>
//#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
//#include <mtkcam/iopipe/SImager/IImageTransform.h>
//#include <mtkcam/iopipe/PostProc/INormalStream.h>
//#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <mtkcam/drv/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/drv/iopipe/SImager/IImageTransform.h>
#include <mtkcam/drv/iopipe/PostProc/INormalStream.h>
#include <mtkcam/drv/iopipe/PostProc/IFeatureStream.h>
#include <mtkcam/utils/imgbuf/IIonImageBufferHeap.h>

//#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <drv/include/mt6757/imageio/ispio_pipe_ports.h>
//#include <mtkcam/algorithm/libdngop/MTKDngOp.h>
#include <algorithm/mt6757/libdngop/MTKDngOp.h>
//#include <mtkcam/utils/Format.h>
#include <mtkcam/utils/std/Format.h>
#include <isp_tuning.h>
//#include <isp_function.h>
#include <drv/src/isp/mt6757/imageio/inc/isp_function.h>
#include <tuning_mgr.h>
//#include <drv/include/mt6757/drv/tuning_mgr.h>
#include <isp_mgr.h>
//#include <aaa/source/mt6757/isp_mgr/isp_mgr.h>

//#include <mtkcam/utils/ImageBufferHeap.h> //for PortBufInfo_v1
#include <mtkcam/utils/imgbuf/ImageBufferHeap.h> //for PortBufInfo_v1

#include <imem_drv.h>
//#include <drv/include/mt6757/drv/imem_drv.h>
#include <isp_drv.h>
//#include <drv/include/mt6757/drv/isp_drv.h>
#include <isp_reg.h>
//#include <drv/include/mt6757/drv/isp_reg.h>

//#include <mtkcam/iopipe/SImager/ISImager.h>
#include <mtkcam/drv/iopipe/SImager/ISImager.h>

#include <include/mtkcam/drv/iopipe/PortMap.h>

using namespace NSIoPipe::NSSImager;
using namespace std;
using namespace NSCam;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NSIoPipe::NSSImager;
using namespace NSIspTuning;
using namespace NSCam::Utils::Format;

using namespace NSIspTuningv3;

// 0:BGGR, 1:GBRG, 2GRBG, 3RGGB
#define IsSOI 1
#define Quality 90

#define TUNING_DATA_FILE        ("/data/cct/p2_tuning.data")


// PortID(EPortType const _eType, MUINT32 const _index, MUINT32 const _inout, EPortCapbility const _capbility = EPortCapbility_None, MUINT32 const _group = 0 )

//static const PortID IMGI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
static const PortID WDMAO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);
//static const PortID WROTO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WROTO, 1);
static const PortID IMGO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGO, 1);
static const PortID IMG2O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG2O, 1);
static const PortID IMG3O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3O, 1);
static const PortID VENC( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_VENC_STREAMO, 1);

#define IMGI        (NSCam::NSIoPipe::PORT_IMGI)
#define WROTO       (NSCam::NSIoPipe::PORT_WROTO)
#define DEPI        (NSCam::NSIoPipe::PORT_DEPI)


MUINT32 gGGMLut[144+144] =
{
0x24002400,
0x24092409,
0x24122412,
0x241b241b,
0x28242824,
0x282e282e,
0x28382838,
0x28422842,
0x284c284c,
0x24562456,
0x285f285f,
0x24692469,
0x24722472,
0x207b207b,
0x24832483,
0x208c208c,
0x20942094,
0x209c209c,
0x20a420a4,
0x20ac20ac,
0x20b420b4,
0x1cbc1cbc,
0x1cc31cc3,
0x20ca20ca,
0x1cd21cd2,
0x1cd91cd9,
0x1ce01ce0,
0x1ce71ce7,
0x18ee18ee,
0x1cf41cf4,
0x1cfb1cfb,
0x19021902,
0x19081908,
0x1d0e1d0e,
0x19151915,
0x191b191b,
0x19211921,
0x19271927,
0x192d192d,
0x15331533,
0x19381938,
0x193e193e,
0x15441544,
0x19491949,
0x154f154f,
0x19541954,
0x155a155a,
0x155f155f,
0x15641564,
0x15691569,
0x156e156e,
0x15731573,
0x15781578,
0x157d157d,
0x15821582,
0x15871587,
0x118c118c,
0x15901590,
0x15951595,
0x119a119a,
0x159e159e,
0x11a311a3,
0x15a715a7,
0x11ac11ac,
0x25b025b0,
0x21b921b9,
0x25c125c1,
0x21ca21ca,
0x21d221d2,
0x1dda1dda,
0x21e121e1,
0x1de91de9,
0x1df01df0,
0x1df71df7,
0x19fe19fe,
0x1e041e04,
0x1a0b1a0b,
0x1a111a11,
0x1e171e17,
0x1a1e1a1e,
0x1e241e24,
0x1a2b1a2b,
0x1e311e31,
0x1e381e38,
0x1a3f1a3f,
0x1e451e45,
0x1a4c1a4c,
0x1a521a52,
0x1a581a58,
0x165e165e,
0x16631663,
0x16681668,
0x166d166d,
0x16721672,
0x12771277,
0x167b167b,
0x26802680,
0x2a892a89,
0x26932693,
0x269c269c,
0x2aa52aa5,
0x22af22af,
0x26b726b7,
0x22c022c0,
0x22c822c8,
0x1ed01ed0,
0x1ed71ed7,
0x1ade1ade,
0x1ae41ae4,
0x1aea1aea,
0x1af01af0,
0x1af61af6,
0x1afc1afc,
0x17021702,
0x1b071b07,
0x170d170d,
0x1b121b12,
0x17181718,
0x1b1d1b1d,
0x17231723,
0x17281728,
0x1b2d1b2d,
0x17331733,
0x17381738,
0x173d173d,
0x17421742,
0x17471747,
0x134c134c,
0x3b503b50,
0x3b5e3b5e,
0x436c436c,
0x437c437c,
0x3b8c3b8c,
0x339a339a,
0x2ba62ba6,
0x23b023b0,
0x23b823b8,
0x23c023c0,
0x2bc82bc8,
0x2bd22bd2,
0x1fdc1fdc,
0x27e327e3,
0x33ec33ec,
0x1ff81ff8,
0x00002400,
0x00002409,
0x00002412,
0x0000241b,
0x00002824,
0x0000282e,
0x00002838,
0x00002842,
0x0000284c,
0x00002456,
0x0000285f,
0x00002469,
0x00002472,
0x0000207b,
0x00002483,
0x0000208c,
0x00002094,
0x0000209c,
0x000020a4,
0x000020ac,
0x000020b4,
0x00001cbc,
0x00001cc3,
0x000020ca,
0x00001cd2,
0x00001cd9,
0x00001ce0,
0x00001ce7,
0x000018ee,
0x00001cf4,
0x00001cfb,
0x00001902,
0x00001908,
0x00001d0e,
0x00001915,
0x0000191b,
0x00001921,
0x00001927,
0x0000192d,
0x00001533,
0x00001938,
0x0000193e,
0x00001544,
0x00001949,
0x0000154f,
0x00001954,
0x0000155a,
0x0000155f,
0x00001564,
0x00001569,
0x0000156e,
0x00001573,
0x00001578,
0x0000157d,
0x00001582,
0x00001587,
0x0000118c,
0x00001590,
0x00001595,
0x0000119a,
0x0000159e,
0x000011a3,
0x000015a7,
0x000011ac,
0x000025b0,
0x000021b9,
0x000025c1,
0x000021ca,
0x000021d2,
0x00001dda,
0x000021e1,
0x00001de9,
0x00001df0,
0x00001df7,
0x000019fe,
0x00001e04,
0x00001a0b,
0x00001a11,
0x00001e17,
0x00001a1e,
0x00001e24,
0x00001a2b,
0x00001e31,
0x00001e38,
0x00001a3f,
0x00001e45,
0x00001a4c,
0x00001a52,
0x00001a58,
0x0000165e,
0x00001663,
0x00001668,
0x0000166d,
0x00001672,
0x00001277,
0x0000167b,
0x00002680,
0x00002a89,
0x00002693,
0x0000269c,
0x00002aa5,
0x000022af,
0x000026b7,
0x000022c0,
0x000022c8,
0x00001ed0,
0x00001ed7,
0x00001ade,
0x00001ae4,
0x00001aea,
0x00001af0,
0x00001af6,
0x00001afc,
0x00001702,
0x00001b07,
0x0000170d,
0x00001b12,
0x00001718,
0x00001b1d,
0x00001723,
0x00001728,
0x00001b2d,
0x00001733,
0x00001738,
0x0000173d,
0x00001742,
0x00001747,
0x0000134c,
0x00003b50,
0x00003b5e,
0x0000436c,
0x0000437c,
0x00003b8c,
0x0000339a,
0x00002ba6,
0x000023b0,
0x000023b8,
0x000023c0,
0x00002bc8,
0x00002bd2,
0x00001fdc,
0x000027e3,
0x000033ec,
0x00001ff8
};

inline static MINT32 _mapFormat(IRawToJpeg::E_FORMAT_T eFmt)
{
    switch (eFmt)
    {
    case IRawToJpeg::ImgFmt_BAYER10:
        return NSCam::eImgFmt_BAYER10;
    case IRawToJpeg::ImgFmt_YUY2:
        return NSCam::eImgFmt_YUY2;
    default:
        return NSCam::eImgFmt_BAYER10;
    }
}

class RawToJpeg :public IRawToJpeg
{
public:
    //    Ctor/Dtor.
                            RawToJpeg();
    virtual                 ~RawToJpeg();

                            RawToJpeg(const RawToJpeg&);
                            RawToJpeg& operator=(const RawToJpeg&);
    //    overridden functions
    virtual MBOOL           readFile(const char *Filename, const InputInfo& info);//override
    virtual MBOOL           run();
    virtual MBOOL           saveFile(const char *Filename, E_STAGE_T eStage);
    virtual MBOOL           saveJPGToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size);
    virtual void            releaseFile();
    virtual void            destroyInstance();
    virtual void            setIspCtl(MUINT32 RgbEn, MUINT32 YuvEn, MUINT32 Yuv2En);

protected:
    virtual MBOOL           allocBuf(IImageBuffer **pBuf, MSize & imgSize, MINT32 format);
    virtual MBOOL           allocJpgBuf(IImageBuffer **pBuf, MSize & imgSize);
    virtual MBOOL           yuv2jpg();
    virtual MBOOL           unpakeTopack(IImageBuffer *packImgBuf);
    virtual MERROR          convRawToYuv();

    MBOOL                   allocLscBuf(MUINT32 x_size, MUINT32 y_size);
    MBOOL                   freeLscBuf();
    
    MBOOL                   setLscBuf(const MUINT32* pTbl, MUINT32 TblSize);

    MBOOL                   setPGN(ISP_NVRAM_PGN_T& pgn);
    MBOOL                   setUDM(ISP_NVRAM_UDM_T& udm);
    MBOOL                   setCCM(ISP_NVRAM_CCM_T& ccm);
    MBOOL                   setGGM(ISP_NVRAM_GGM_T& ggm);
    MBOOL                   setDBS(ISP_NVRAM_DBS_T& dbs);
    MBOOL                   setOBC(ISP_NVRAM_OBC_T& obc);
    MBOOL                   setBNR_BPC(ISP_NVRAM_BNR_BPC_T& bpc);
    MBOOL                   setBNR_NR1(ISP_NVRAM_BNR_NR1_T& nr1);
    MBOOL                   setBNR_PDC(ISP_NVRAM_BNR_PDC_T& pdc);
    MBOOL                   setLSC(ISP_NVRAM_LSC_T& lsc, MUINT32 *pLscTbl, MUINT32 tblSize);
    MBOOL                   setRNR(ISP_NVRAM_RNR_T& rnr);
    MBOOL                   setSL2(ISP_NVRAM_SL2_T& sl2);
    MBOOL                   setSL2G(ISP_NVRAM_SL2_T& sl2);
    MBOOL                   setG2C(ISP_NVRAM_G2C_T& g2c);
    MBOOL                   setANR(ISP_NVRAM_ANR_T& anr, ISP_NVRAM_ANR_LUT_T& anrLut);
    MBOOL                   setANR2(ISP_NVRAM_ANR2_T& anr2);
    MBOOL                   setCCR(ISP_NVRAM_CCR_T& ccr);
    MBOOL                   setPCA(ISP_NVRAM_PCA_T& pca, MUINT32 *pcaLut);
    MBOOL                   setSEEE(ISP_NVRAM_EE_T& ee, ISP_NVRAM_SE_T& se);


    MBOOL                   applySettingP2();
    MBOOL                   applySettingFileP2();   // for test only


    static  MVOID           enqueCallback(QParams &rParams);
    MVOID                   handleDeque(QParams &rParams);

    ISP_NVRAM_PGN_T         m_sPGN;
    ISP_NVRAM_SL2_T         m_sSL2;
    ISP_NVRAM_UDM_T         m_sUDM;
    ISP_NVRAM_CCM_T         m_sCCM;     //G2G
//    ISP_NVRAM_CCM_CTL_T     m_sCCMCtl;  //G2G
    ISP_NVRAM_GGM_T         m_sGGM;
    ISP_NVRAM_DBS_T         m_sDBS;
    ISP_NVRAM_OBC_T         m_sOBC2;
    ISP_NVRAM_BNR_BPC_T     m_sBNRBpc;
    ISP_NVRAM_BNR_NR1_T     m_sBNRNr1;
    ISP_NVRAM_BNR_PDC_T     m_sBNRPdc;
    ISP_NVRAM_LSC_T         m_sLSC2;
    
    ISP_NVRAM_RNR_T         m_sRNR;
    ISP_NVRAM_SL2_T         m_sSL2G;
    ISP_NVRAM_G2C_T         m_sG2C;
//    ISP_NVRAM_G2C_SHADE_T   m_sG2CShade;
    ISP_NVRAM_ANR_T         m_sANR;     //NBC
    ISP_NVRAM_ANR_LUT_T     m_sANR_Tbl; //NBC ANR LUT
    ISP_NVRAM_ANR2_T        m_sANR2;    //NBC2
    ISP_NVRAM_CCR_T         m_sCCR;     //NBC2

    ISP_NVRAM_PCA_T         m_sPCA;
    MUINT32                 m_u4PCA_Lut[PCA_BIN_NUM*2];
    ISP_NVRAM_EE_T          m_sEE;      //SEEE
    ISP_NVRAM_SE_T          m_sSE;      //SEEE
    

    MUINT32*                m_pUnpackImgBuf;    //m_pSrcImgBuffer; //pure raw
    IImageBuffer*           m_pPackImgBuf;      //m_pDstImgBuffer;//pure raw
    IImageBuffer*           m_pProcBuf;         //pDstBuf;//process raw
//    IFeatureStream*         m_pStream;
//    INormalStream*         m_pStream;
    INormalStream*          m_pNormStream;
    IImageBuffer*           m_pLsciBuf;
    IImageBuffer*           m_pJpgbuf;
    IImageBuffer*           m_pYuvBuf;

    MUINT32                 m_u4MagicNum;
    MSize                   m_rUnpackImgSize;
    MSize                   m_rJpegSize;
    MINT32                  m_i4Format;
    MBOOL                   m_bUnpack2PackEn;
	MUINT32                 m_u4Pixel;

    QParams                 m_rProc2YuvDParams;//Proc2Yuv DequeParams
    MBOOL                   m_bIsV3;    //temp disable tuning path
    volatile MBOOL          m_bEnqueDone;
    void *                  m_pTuning;

    MUINT32 muEnqueCnt;
    MUINT32 muDequeCnt;
    MUINT32 mRgbEn;
    MUINT32 mYuvEn;
    MUINT32 mYuv2En;
};


RawToJpeg::
RawToJpeg()
    : IRawToJpeg()
//    , m_pStream(NULL)
    , m_pUnpackImgBuf(NULL)
    , m_pPackImgBuf(NULL)
    , m_pProcBuf(NULL)
    , m_pLsciBuf(NULL)
    , m_pJpgbuf(NULL)
    , m_pYuvBuf(NULL)
    , m_bIsV3(MFALSE)
    , m_u4MagicNum(0)
    , muEnqueCnt(0)
    , muDequeCnt(0)
    , m_bEnqueDone(MFALSE)
    , m_pTuning(NULL)
    , mRgbEn(0)
    , mYuvEn(0)
    , mYuv2En(0)

{
    MBOOL ret = MFALSE;
    m_rUnpackImgSize.w = 0;
    m_rUnpackImgSize.h = 0;
    m_rJpegSize.w = 0;
    m_rJpegSize.h = 0;

    if (allocLscBuf(0x600,0x10) == MFALSE) {
        MY_LOG("[%s] Alloc LSC buffer failed",__FUNCTION__);
    }

#if 0    // jokery
    m_pStream  = IFeatureStream::createInstance( //IFeatureStream driver
            LOG_TAG,
            EFeatureStreamTag_PRaw_Stream,
            0xFFFF,m_bIsV3);//change to use sensor index, 0xFFFF means pure pass2

    if( m_pStream == NULL )
        MY_LOG("[%s] Create Feature Stream failed",__FUNCTION__);
    else
        MY_LOG("[%s] Create Feature Stream OK",__FUNCTION__);

    if( !m_pStream->init(LOG_TAG) )
        MY_LOG("[%s] Feature Stream init failed",__FUNCTION__);
    else
        MY_LOG("[%s] Feature Stream init OK",__FUNCTION__);
#endif 

//    ret = TuningMgr::getInstance(0)->init(LOG_TAG);
//    if(ret != MTRUE)
//        MY_LOG("[%s] Tuning Mgr init fail",__FUNCTION__);

    //objects
    MY_LOG("[%s] Creat Normal Stream",__FUNCTION__);
//    m_pNormStream = INormalStream::createInstance(LOG_TAG, NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Cap,0xFFFF, m_bIsV3);//ENormalStreamTag_Prv
    m_pNormStream = INormalStream::createInstance(0xFFFF);
    MY_LOG("[%s] Normal Stream init",__FUNCTION__);
    m_pNormStream->init(LOG_TAG);
}

RawToJpeg::
~RawToJpeg()
{
#if 0
    if( !m_pStream->uninit(LOG_TAG) )
        MY_LOG("[%s] Feature Stream uninit failed",__FUNCTION__);
    else
        MY_LOG("[%s] Feature Stream uninit OK",__FUNCTION__);
#endif

//    if( !(TuningMgr::getInstance(0)->uninit(LOG_TAG)) )
//        MY_LOG("[%s] Tuning Mgr uninit failed",__FUNCTION__);
//    else
//        MY_LOG("[%s] Tuning Mgr uninit OK",__FUNCTION__);


    MY_LOG("[%s] Normal Stream uninit",__FUNCTION__);
    if(m_pNormStream)
    {
        m_pNormStream->uninit(LOG_TAG);
        m_pNormStream->destroyInstance();
    }
    
    freeLscBuf();
#if 0
    if(m_pStream)
        m_pStream->destroyInstance();
#endif
    if (m_pTuning!=NULL)
        ::free(m_pTuning);
}


MBOOL
RawToJpeg::
readFile(const char *Filename, const InputInfo& rInfo)
{
    MY_LOG("[%s] Read file +",__FUNCTION__);
    MBOOL ret = MFALSE;

    m_rUnpackImgSize.w = rInfo.i4Width;    //m_rUnpackImgSize.w = 5312;
    m_rUnpackImgSize.h = rInfo.i4Height;   //m_rUnpackImgSize.h = 2976;
    m_rJpegSize.w = rInfo.i4JpegWidth;
    m_rJpegSize.h = rInfo.i4JpegHeight;
    m_i4Format = _mapFormat(rInfo.i4Format);           //eImgFmt_BAYER10;
    m_bUnpack2PackEn = rInfo.bUnpack2PackEn;
	m_u4Pixel = rInfo.u4Pixel;
    MY_LOG("[%s] Unpack.size(%d,%d) Jpeg.size(%d,%d) Format(0x%x) Unpack2PackEn(%d)", 
        __FUNCTION__,
        m_rUnpackImgSize.w,m_rUnpackImgSize.h,
        m_rJpegSize.w,m_rJpegSize.h,
        m_i4Format,m_bUnpack2PackEn);

    FILE* pInput = fopen(Filename,"rb");
    //Get input data size
    fseek(pInput, 0, SEEK_END);
    MUINT32 size = ftell(pInput);
    fseek(pInput, 0, SEEK_SET);

    m_pUnpackImgBuf = (MUINT32*)malloc(size);
    if( m_pUnpackImgBuf == NULL )
    {
        MY_LOG("[%s] Allocat Unpack image buffe failed",__FUNCTION__);
        return MFALSE;
    }

    fread( m_pUnpackImgBuf, 1, size, pInput );

    MY_LOG("[%s] Unpack image buffe Addr(%p)",__FUNCTION__,m_pUnpackImgBuf);


    MY_LOG("[%s] Unpack to Pack start",__FUNCTION__);
    ret = allocBuf(&m_pPackImgBuf, m_rUnpackImgSize, m_i4Format);
    if(ret != MTRUE)
        MY_LOG("[%s] Allocat pack image buffe failed",__FUNCTION__);
    else
        MY_LOG("[%s] Allocat pack image buffe OK",__FUNCTION__);

    //m_bUnpack2PackEn = 1;

    if(m_bUnpack2PackEn == 1)
    {
        MY_LOG("[%s] m_bUnpack2PackEn(%d)",__FUNCTION__,m_bUnpack2PackEn);
        ret = unpakeTopack(m_pPackImgBuf);
        if(ret != MTRUE)
            MY_LOG("[%s] Unpack to Pack failed",__FUNCTION__);
        else
            MY_LOG("[%s] Unpack to Pack OK",__FUNCTION__);

        MY_LOG("[%s] Unpack to Pack end",__FUNCTION__);
    }
    else
    {
        MY_LOG("[%s] m_bUnpack2PackEn(%d)",__FUNCTION__,m_bUnpack2PackEn);
        FILE* pOutput = fopen("/data/cct/pack-check.raw","wb");
        ::memcpy((void*)m_pPackImgBuf->getBufVA(0), m_pUnpackImgBuf, size);
        fwrite ((void*)m_pPackImgBuf->getBufVA(0) ,1 , size, pOutput);
        fclose(pOutput);
    }
    fclose(pInput);
    MY_LOG("[%s] Read file -",__FUNCTION__);
    return MTRUE;
}

MBOOL
RawToJpeg::
allocBuf(IImageBuffer **pBuf, MSize & imgSize, MINT32 format)
{

    MY_LOG("[%s] AllocBuf +",__FUNCTION__);
    MY_LOG("[%s] AllocBuf WxH(%dx%d) format(0x%x)",__FUNCTION__, imgSize.w, imgSize.h, format);

    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    MUINT32 bufStridesInBytes[3] = {0};
    MUINT32 plane = queryPlaneCount(format);
    MY_LOG("[%s] plane(%d) +",__FUNCTION__,plane);


    for (MUINT32 i = 0; i < plane; i++)
    {
        bufStridesInBytes[i] = (NSCam::Utils::Format::queryPlaneWidthInPixels(format,i, imgSize.w) * NSCam::Utils::Format::queryPlaneBitsPerPixel(format,i)) / 8;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};

    IImageBufferAllocator::ImgParam imgParam(
            format, imgSize, bufStridesInBytes, bufBoundaryInBytes, plane
            );

    *pBuf = allocator->alloc_ion(LOG_TAG, imgParam);
    if  ( (*pBuf) == NULL )
    {
        MY_LOG("[%s] Allocat buffer failed",__FUNCTION__);
        return MFALSE;
    }

    MY_LOG("[%s] Allocat buffer lock before",__FUNCTION__);

    (*pBuf)->lockBuf( LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN );

    MY_LOG("[%s] Allocat buffer lock after",__FUNCTION__);
    MY_LOG("[%s] ImgBitsPerPixel(%d) BufSizeInBytes(%d) getBufStridesInBytes(%d) getImgFormat(0x%x) BufAddr(%p) first byte(%x)",__FUNCTION__,(*pBuf)->getImgBitsPerPixel(), (*pBuf)->getBufSizeInBytes(0), (*pBuf)->getBufStridesInBytes(0), (*pBuf)->getImgFormat(),(*pBuf)->getBufVA(0),*((MUINT8*)(*pBuf)->getBufVA(0)));

    MY_LOG("[%s] AllocBuf -",__FUNCTION__);

    return MTRUE;
}


MBOOL
RawToJpeg::
allocJpgBuf(IImageBuffer **pBuf, MSize & imgSize)
{
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    MUINT32 const thumbnailsize = 160 * 128; //FIXME: temp solution
    MUINT32 bufsize = imgSize.w * imgSize.h;

    if( bufsize > thumbnailsize ) // to make sure buffer is large enough for thumbnail
    {
        bufsize = bufsize * 6 / 5; //jpeg compression ratio
    }
    else
    {
        bufsize = bufsize * 2;
    }

    MINT32 bufBoundaryInBytes = 0;
    IImageBufferAllocator::ImgParam imgParam(
            MSize(imgSize.w,imgSize.h),
            bufsize,
            bufBoundaryInBytes);

    (*pBuf) = allocator->alloc_ion(LOG_TAG, imgParam);
    (*pBuf)->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_MASK|eBUFFER_USAGE_SW_MASK);
    return MTRUE;
}

MBOOL
RawToJpeg::
allocLscBuf(MUINT32 x_size, MUINT32 y_size)
{
    // create buffer
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3] = {x_size, 0, 0};
    IImageBufferAllocator::ImgParam imgParam =
        IImageBufferAllocator::ImgParam((NSCam::EImageFormat)NSCam::eImgFmt_STA_BYTE,
            NSCam::MSize(x_size, y_size), bufStridesInBytes, bufBoundaryInBytes, 1);
    sp<IIonImageBufferHeap> pHeap =
        IIonImageBufferHeap::create(LOG_TAG, imgParam);
    if (pHeap == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", LOG_TAG);
        return MFALSE;
    }
//    sp<IImageBuffer> pImgBuf = pHeap->createImageBuffer();
    IImageBuffer* pImgBuf = pHeap->createImageBuffer();
    if (pImgBuf == NULL) {
        CAM_LOGE("[%s] ImageBufferHeap create fail", LOG_TAG);
        return MFALSE;
    }
    // lock buffer
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_SW_WRITE_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    if (!(pImgBuf->lockBuf(LOG_TAG, usage)))
    {
        CAM_LOGE("[%s] Stuff ImageBuffer lock fail",  LOG_TAG);
        return MFALSE;
    }

    m_pLsciBuf = pImgBuf;

    MY_LOG("name(%s), virtAddr(%p), phyAddr(%p), size(%d)",
        LOG_TAG, m_pLsciBuf->getBufVA(0), m_pLsciBuf->getBufPA(0), m_pLsciBuf->getBufSizeInBytes(0));

    return MTRUE;
}

MBOOL
RawToJpeg::
freeLscBuf()
{
    MUINT32 u4PhyAddr = m_pLsciBuf->getBufPA(0);
    const void* pVirAddr = (void *)m_pLsciBuf->getBufVA(0);
    if (m_pLsciBuf == NULL) {
        CAM_LOGE("name(%s): Stuff ImageBuffer not exist", LOG_TAG);
        return MFALSE;
    }

    m_pLsciBuf->unlockBuf(LOG_TAG);
    // destroy buffer
    m_pLsciBuf = NULL;
    //
    MY_LOG("name(%s) ImageBuffer virtAddr(%p), phyAddr(%p)", LOG_TAG, pVirAddr, u4PhyAddr);
    return MTRUE;
}



MBOOL
RawToJpeg::
unpakeTopack(IImageBuffer *packImgBuf)
{
    MY_LOG("[%s] unpake to pack +",__FUNCTION__);
/** Start to simulate unpack... **/
    MTKDngOp *MyDngop = NULL;
    DngOpResultInfo MyDngopResultInfo;
    DngOpImageInfo MyDngopImgInfo;
    /** Initialize **/
    MY_LOG("[%s] Start to simulate unpack...",__FUNCTION__);

    MyDngop = MyDngop->createInstance(DRV_DNGOP_PACK_OBJ_SW);
    MyDngopImgInfo.Width = packImgBuf->getImgSize().w;//5312;
    MyDngopImgInfo.Height = packImgBuf->getImgSize().h;//2976;
    MyDngopImgInfo.Stride_src = packImgBuf->getImgSize().w*2;//5312*2;
    MyDngopImgInfo.Stride_dst = packImgBuf->getBufStridesInBytes(0);//5312*10/8;
    MyDngopImgInfo.BIT_NUM = 10;//10bit unpack
    MyDngopImgInfo.Bit_Depth = 10;//for unpack and pack12
    MyDngopImgInfo.Buff_size = MyDngopImgInfo.Stride_dst * MyDngopImgInfo.Height;

    // set dst buffer address
    MyDngopResultInfo.ResultAddr = reinterpret_cast<void*>(packImgBuf->getBufVA(0));

    // assign src raw data
    MyDngopImgInfo.srcAddr = reinterpret_cast<void*>((MUINT32*)m_pUnpackImgBuf);
    MY_LOG("[%s] unpack image(%dx%d), Stride(%d) Addr(%p)",__FUNCTION__,MyDngopImgInfo.Width, MyDngopImgInfo.Height, MyDngopImgInfo.Stride_src, MyDngopImgInfo.srcAddr);

    MRESULT err = MyDngop->DngOpMain((void*)&MyDngopImgInfo, (void*)&MyDngopResultInfo);
    if (FAILED(err)){
        MY_LOG("[%s] err=%d\n",__FUNCTION__,err);
        MY_LOG("[%s] FAILED(err)=%d\n",__FUNCTION__,FAILED(err));
        return err;
        }
    MY_LOG("[%s] pack image(%dx%d), Stride(%d) Addr(%p)",__FUNCTION__,packImgBuf->getImgSize().w, packImgBuf->getImgSize().h,packImgBuf->getBufStridesInBytes(0),packImgBuf->getBufVA(0));

    MyDngop->destroyInstance(MyDngop);
    MY_LOG("[%s] End to simulate pack...",__FUNCTION__);
    /** End simulate pack... **/

    MY_LOG("[%s] unpake to pack -",__FUNCTION__);
    return MTRUE;
}



MBOOL
RawToJpeg::
setLscBuf(const MUINT32* pTbl, MUINT32 TblSize)
{
    if (m_pLsciBuf != NULL) {
        ::memcpy((void*)m_pLsciBuf->getBufVA(0), pTbl, TblSize);
        return MTRUE;
    } else
        return MFALSE;
}



MBOOL
RawToJpeg::
saveFile(const char *Filename, E_STAGE_T eStage)
{
    MY_LOG("[%s] Save File +",__FUNCTION__);
    MBOOL ret = MFALSE;
    switch (eStage)
    {
        case E_PACK_RAW:
            ret = m_pPackImgBuf->saveToFile(Filename);
            if(ret != MTRUE)
            {
                MY_LOG("[%s] Save Pack ImgBuf failed",__FUNCTION__);
            }
            break;
        case E_PROC_RAW:
            ret = m_pProcBuf->saveToFile(Filename);
            if(ret != MTRUE)
            {
                MY_LOG("[%s] Save ProcBuf failed",__FUNCTION__);
            }
            break;
        case E_YUV:
            ret = saveJPGToFile(Filename, reinterpret_cast<MUINT8*>(m_rProc2YuvDParams.mvOut[0].mBuffer->getBufVA(0)), m_rUnpackImgSize.w *m_rUnpackImgSize.h * 2);
             if(ret != MTRUE)
            {
                MY_LOG("[%s] Save YuvBuf failed",__FUNCTION__);
            }
            break;
        case E_JPEG:
            ret = m_pJpgbuf->saveToFile(Filename);
            if(ret != MTRUE)
            {
                MY_LOG("[%s] Save Jpgbuf failed",__FUNCTION__);
            }
            break;
    }
    MY_LOG("[%s] Save File -",__FUNCTION__);
    return MTRUE;
}



#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

MBOOL
RawToJpeg::
saveJPGToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOG("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOG("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
RawToJpeg::
enqueCallback(QParams &rParams) {
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc +++");
    RawToJpeg *pRawToJpeg = reinterpret_cast<RawToJpeg *>(rParams.mpCookie);
    pRawToJpeg->handleDeque(rParams);
    //MY_LOGD_IF(mbEnableLog, "pass2CbFunc ---");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
RawToJpeg::
handleDeque(QParams &rParams) {
#if 1   // jokery
    MY_LOG("P2:handleDeque");
    if (rParams.mvOut[0].mBuffer <= 0) {
        MY_LOG("[%s] rParams.mvOut[0].mBuffer error",__FUNCTION__);
    } else {
        // ready for reading the Yuv image from rParams.mvOut[0].mBuffer
        m_rProc2YuvDParams = rParams;
    }
    m_bEnqueDone = MTRUE;

#endif
}


MERROR
RawToJpeg::
convRawToYuv()
{
    MY_LOG("[%s] enter",__FUNCTION__);

    MERROR ret = OK;

    String8 strEnqueLog;
    QParams enqueParams;
    //frame tag
    enqueParams.mvStreamTag.push_back(ENormalStreamTag_Normal /*mnStreamTag*/);

    // input
    {
        Input src;
        src.mPortID = IMGI; //params.in.mPortId;
        src.mPortID.group = 0;
        src.mBuffer = m_pPackImgBuf;   //pSrc;

        enqueParams.mvIn.push_back(src);
        strEnqueLog += String8::format("EnQ: Src Port(%d) Fmt(0x%x) Size(%dx%d) => ",
                   src.mPortID.index, src.mBuffer->getImgFormat(),
                   src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h);
    }

    // LSC table
    if (m_pLsciBuf != NULL) {
        //IImageBuffer *pSrc = static_cast<IImageBuffer *>(m_pLsciBuf);
        Input src;
        src.mPortID = DEPI;
        src.mPortID.group = 0;
        src.mBuffer = m_pLsciBuf; //pSrc;

        enqueParams.mvIn.push_back(src);
        MY_LOG("[%s] EnQ Src mPortID.index(%d) Fmt(0x%x) Size(%dx%d)", __FUNCTION__, src.mPortID.index, 
            src.mBuffer->getImgFormat(), src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h);
    } else {
        MY_LOG("[%s] LSC table not available", __FUNCTION__);
    }

    #if 0   // jokery
    // input LCEI
    if(params.in_lcso.mHandle != NULL){
        if( OK != (ret = params.in_lcso.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW(" (%d) Lcso handle not null but waitState failed! ", pRequest->getFrameNo());
            return BAD_VALUE;
        }else {
            IImageBuffer* pSrc = params.in_lcso.mHandle->getBuffer();
            //
            Input src;
            src.mPortID       = params.in_lcso.mPortId;
            src.mPortID.group = 0;
            src.mBuffer       = pSrc;

            //
            enqueParams.mvIn.push_back(src);
            MY_LOGD_IF(mbEnableLog, "EnQ Src mPortID.index(%d) Fmt(0x%x) "
                "Size(%dx%d)", src.mPortID.index, src.mBuffer->getImgFormat(),
                src.mBuffer->getImgSize().w, src.mBuffer->getImgSize().h);
        }
        //

    }
    #endif

    // output
    //output buffer
    allocBuf(&m_pYuvBuf, m_rJpegSize, _mapFormat(IRawToJpeg::ImgFmt_YUY2));//eImgFmt_I420,eImgFmt_YUY2

    Output dst;
    dst.mPortID = WROTO;    //params.vOut[i].mPortId;
    #if 0   // jokery
    dst.mPortID.group = 0;
    MUINT32 const uUsage = params.vOut[i].mUsage;
    dst.mPortID.capbility = (NSIoPipe::EPortCapbility)(
            (uUsage & GRALLOC_USAGE_HW_COMPOSER) ? EPortCapbility_Disp :
            (uUsage & GRALLOC_USAGE_HW_VIDEO_ENCODER) ? EPortCapbility_Rcrd :
            EPortCapbility_None);
    #endif
    dst.mBuffer = m_pYuvBuf;    //pDst;
    dst.mTransform = 0;   //params.vOut[i].mTransform;

    enqueParams.mvOut.push_back(dst);

    if (enqueParams.mvOut.size() == 0) {
        //MY_LOGWO("no dst buffer");
        return BAD_VALUE;
    }

    #if 1   // jokery
    {
        //crop
        MINT32 offsetX = (m_rUnpackImgSize.w - min(m_rUnpackImgSize.w, m_rJpegSize.w));
        MINT32 offsetY = (m_rUnpackImgSize.h - min(m_rUnpackImgSize.h, m_rJpegSize.h));
        if(offsetX < 0) offsetX = 0;
        if(offsetY < 0) offsetY = 0;
        MINT32 cropW = m_rUnpackImgSize.w - 2 * offsetX;
        MINT32 cropH = m_rUnpackImgSize.h - 2 * offsetY;
        MCrpRsInfo crop;
        
        crop.mGroupID=3;        //2;
        crop.mCropRect.p_fractional.x=0;
        crop.mCropRect.p_fractional.y=0;
        crop.mCropRect.p_integral.x=offsetX;
        crop.mCropRect.p_integral.y=offsetY;
        crop.mCropRect.s.w=cropW;
        crop.mCropRect.s.h=cropH;
        crop.mResizeDst.w=m_rJpegSize.w;
        crop.mResizeDst.h=m_rJpegSize.h;

        strEnqueLog += String8::format("Dst Grp(%d) Rot(%d) Crop(%d,%d)(%dx%d) Size(%dx%d)",
                crop.mGroupID, dst.mTransform,
                crop.mCropRect.p_integral.x, crop.mCropRect.p_integral.y,
                crop.mCropRect.s.w, crop.mCropRect.s.h,
                crop.mResizeDst.w, crop.mResizeDst.h);
        
        enqueParams.mvCropRsInfo.push_back(crop);
    }
    #endif

    if (m_pTuning != NULL) {
        MY_LOG("[%s] push_back IQ data",__FUNCTION__);
        enqueParams.mvTuningData.push_back(m_pTuning);
    } else {
        MY_LOG("[%s] IQ data not available",__FUNCTION__);
    }
    
    MY_LOG("%s", strEnqueLog.string());

    // callback
    enqueParams.mpfnCallback = enqueCallback;
    enqueParams.mpCookie = this;

#if 0
    // FIXME: need this?
    enqueParams.mvPrivaData.push_back(NULL);
#endif

    MY_LOG("p2 enque count:%d, size[in/out]:%zu/%zu",
               muEnqueCnt, enqueParams.mvIn.size(), enqueParams.mvOut.size());

    muEnqueCnt++;
    MY_LOG("p2 enque +");
    MY_LOG("P2:Driver:enque");
    if (!/*mpPipe*/ m_pNormStream->enque(enqueParams)) {
        MY_LOG("p2 enque failed");
        return UNKNOWN_ERROR;
    }
    MY_LOG("p2 enque -");

    MY_LOG("[%s] leave",__FUNCTION__);
    return OK;
}



MBOOL
RawToJpeg::
yuv2jpg()
{
    MY_LOG("[%s] Yuv to Jpg +",__FUNCTION__);

    ISImager *pISImager = ISImager::createInstance(m_rProc2YuvDParams.mvOut[0].mBuffer);

    if( allocJpgBuf(&m_pJpgbuf, m_rJpegSize) )
        MY_LOG("[%s] Alloc jpg buf success",__FUNCTION__);
    else
        MY_LOG("[%s] Alloc jpg buf failed",__FUNCTION__);

    MY_LOG("[%s] Set target img buffer",__FUNCTION__);
    pISImager->setTargetImgBuffer(m_pJpgbuf);


    MY_LOG("[%s] Set transform",__FUNCTION__);
    pISImager->setTransform(0);

    MY_LOG("[%s] Set encode param",__FUNCTION__);
    pISImager->setEncodeParam(IsSOI, Quality);


    MY_LOG("[%s] Set crop jpg Rect",__FUNCTION__);
    MRect cropjpg = MRect(MPoint(0, 0), m_rProc2YuvDParams.mvOut[0].mBuffer->getImgSize());

    MY_LOG("[%s] Set Crop ROI",__FUNCTION__);
    pISImager->setCropROI(cropjpg);

    MY_LOG("[%s] Execute",__FUNCTION__);
    pISImager->execute();

    MY_LOG("[%s] Destroy pISImager instance",__FUNCTION__);
    pISImager->destroyInstance();
    pISImager = NULL;

    MY_LOG("[%s] Yuv to Jpg -",__FUNCTION__);
    return MTRUE;
}


MBOOL
RawToJpeg::
setPGN(ISP_NVRAM_PGN_T& pgn)
{
    m_sPGN = pgn;
    return MTRUE;
}

MBOOL
RawToJpeg::
setUDM(ISP_NVRAM_UDM_T& udm)
{
    m_sUDM = udm;
    return MTRUE;
}

MBOOL
RawToJpeg::
setCCM(ISP_NVRAM_CCM_T& ccm)
{
    m_sCCM = ccm;
    return MTRUE;
}

MBOOL
RawToJpeg::
setGGM(ISP_NVRAM_GGM_T& ggm)
{
    m_sGGM = ggm;
    return MTRUE;
}

MBOOL
RawToJpeg::
setDBS(ISP_NVRAM_DBS_T& dbs)
{
    m_sDBS = dbs;
    return MTRUE;
}

MBOOL
RawToJpeg::
setOBC(ISP_NVRAM_OBC_T& obc)
{
    m_sOBC2 = obc;
    return MTRUE;
}

MBOOL
RawToJpeg::
setBNR_BPC(ISP_NVRAM_BNR_BPC_T& bpc)
{
    m_sBNRBpc = bpc;
    return MTRUE;
}

MBOOL
RawToJpeg::
setBNR_NR1(ISP_NVRAM_BNR_NR1_T& nr1)
{
    m_sBNRNr1 = nr1;
    return MTRUE;
}

MBOOL
RawToJpeg::
setBNR_PDC(ISP_NVRAM_BNR_PDC_T& pdc)
{
    m_sBNRPdc = pdc;
    return MTRUE;
}

MBOOL
RawToJpeg::
setLSC(ISP_NVRAM_LSC_T& lsc, MUINT32 *pLscTbl, MUINT32 tblSize)
{
    m_sLSC2 = lsc;
    setLscBuf( pLscTbl, tblSize );
    return MTRUE;
}

MBOOL
RawToJpeg::
setRNR(ISP_NVRAM_RNR_T& rnr)
{
    m_sRNR = rnr;
    return MTRUE;
}

MBOOL
RawToJpeg::
setSL2(ISP_NVRAM_SL2_T& sl2)
{
    m_sSL2 = sl2;
    return MTRUE;
}

MBOOL
RawToJpeg::
setSL2G(ISP_NVRAM_SL2_T& sl2)
{
    m_sSL2G = sl2;
    return MTRUE;
}

MBOOL
RawToJpeg::
setG2C(ISP_NVRAM_G2C_T& g2c)
{
    m_sG2C = g2c;
    return MTRUE;
}

MBOOL
RawToJpeg::
setANR(ISP_NVRAM_ANR_T& anr, ISP_NVRAM_ANR_LUT_T& anrLut)
{
    m_sANR = anr;
    m_sANR_Tbl = anrLut;
    return MTRUE;
}

MBOOL
RawToJpeg::
setANR2(ISP_NVRAM_ANR2_T& anr2)
{
    m_sANR2 = anr2;
    return MTRUE;
}

MBOOL
RawToJpeg::
setCCR(ISP_NVRAM_CCR_T& ccr)
{
    m_sCCR = ccr;
    return MTRUE;
}

MBOOL
RawToJpeg::
setPCA(ISP_NVRAM_PCA_T& pca, MUINT32 *pcaLut)
{
    m_sPCA = pca;
    ::memcpy(m_u4PCA_Lut, pcaLut, sizeof(m_u4PCA_Lut));
    return MTRUE;
}

MBOOL
RawToJpeg::
setSEEE(ISP_NVRAM_EE_T& ee, ISP_NVRAM_SE_T& se)
{
    m_sEE = ee;
    m_sSE = se;
    return MTRUE;
}


MBOOL
RawToJpeg::
applySettingP2()
{
    MY_LOG("[%s] Apply Setting P2 +",__FUNCTION__);
    MBOOL fgRet = MTRUE;
    MINT16 i;

    CROP_RZ_INFO_T CropRzInfo;
    RAWIspCamInfo RawCamInfo;

    unsigned int tuningsize = (m_pNormStream != NULL) ? m_pNormStream->getRegTableSize() : 0;//sizeof(dip_x_reg_t);
    //unsigned int tuningsize = sizeof(dip_x_reg_t);
    if (tuningsize == 0) {
        MY_LOG("getRegTableSize is 0 (%p)", m_pNormStream);
    } else {
        MY_LOG("getRegTableSize is %x ", tuningsize);
    }

    if (m_pTuning == NULL )
        m_pTuning = ::malloc(tuningsize);

    if (m_pTuning == NULL) {
        MY_LOG("[%s] Tuning data buffer malloc failed",__FUNCTION__);
        return MFALSE;
    }
        
    dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(m_pTuning);
    ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));

#if 0
    pReg->DIP_X_CTL_RGB_EN.Raw = mRgbEn;
    pReg->DIP_X_CTL_YUV_EN.Raw = mYuvEn;
    pReg->DIP_X_CTL_YUV2_EN.Raw = (mYuv2En & 0xFF);
#endif

#if 1
    CropRzInfo.i4FullW = m_rUnpackImgSize.w;
    CropRzInfo.i4FullH = m_rUnpackImgSize.h;
    CropRzInfo.i4OfstX = 0;
    CropRzInfo.i4OfstY = 0;
    CropRzInfo.i4Width = m_rUnpackImgSize.w;
    CropRzInfo.i4Height = m_rUnpackImgSize.h;
    CropRzInfo.i4RzWidth = m_rUnpackImgSize.w;
    CropRzInfo.i4RzHeight = m_rUnpackImgSize.h;
    CropRzInfo.fgOnOff = MTRUE;

    ISP_MGR_PGN_T& rPGN = ISP_MGR_PGN_T::getInstance(ESensorDev_Main);
    ISP_MGR_SL2_T& rSL2 = ISP_MGR_SL2_T::getInstance(ESensorDev_Main);
    ISP_MGR_UDM_T& rUDM = ISP_MGR_UDM_T::getInstance(ESensorDev_Main);
    ISP_MGR_CCM_T& rCCM = ISP_MGR_CCM_T::getInstance(ESensorDev_Main);
    ISP_MGR_GGM_T& rGGM = ISP_MGR_GGM_T::getInstance(ESensorDev_Main);
    ISP_MGR_DBS2_T& rDBS2 = ISP_MGR_DBS2_T::getInstance(ESensorDev_Main);
    ISP_MGR_OBC2_T& rOBC2 = ISP_MGR_OBC2_T::getInstance(ESensorDev_Main);
    ISP_MGR_BNR2_T& rBNR2 = ISP_MGR_BNR2_T::getInstance(ESensorDev_Main);
    ISP_MGR_LSC2_T& rLSC2 = ISP_MGR_LSC2_T::getInstance(ESensorDev_Main);
    ISP_MGR_RNR_T& rRNR = ISP_MGR_RNR_T::getInstance(ESensorDev_Main);
    ISP_MGR_SL2G_T& rSL2G = ISP_MGR_SL2G_T::getInstance(ESensorDev_Main);

    ISP_MGR_G2C_T& rG2C = ISP_MGR_G2C_T::getInstance(ESensorDev_Main);
    ISP_MGR_G2C_SHADE_T& rG2C_Shad = ISP_MGR_G2C_SHADE_T::getInstance(ESensorDev_Main);
    ISP_MGR_NBC_T& rNBC = ISP_MGR_NBC_T::getInstance(ESensorDev_Main);
    ISP_MGR_NBC2_T& rNBC2 = ISP_MGR_NBC2_T::getInstance(ESensorDev_Main);
    ISP_MGR_PCA_T& rPCA = ISP_MGR_PCA_T::getInstance(ESensorDev_Main);
    ISP_MGR_SEEE_T& rSEEE = ISP_MGR_SEEE_T::getInstance(ESensorDev_Main);
/*
    ISP_MGR_LCE_T& rLCE = ISP_MGR_LCE_T::getInstance(ESensorDev_Main);
    ISP_MGR_MFB_T& rMFB = ISP_MGR_MFB_T::getInstance(ESensorDev_Main);
    ISP_MGR_MIXER3_T& rMIXER3 = ISP_MGR_MIXER3_T::getInstance(ESensorDev_Main);
    ISP_MGR_RMM2_T& rRMM2 = ISP_MGR_RMM2_T::getInstance(ESensorDev_Main);
    ISP_MGR_RMG2_T& rRMG2 = ISP_MGR_RMG2_T::getInstance(ESensorDev_Main);
    ISP_MGR_HFG_T& rHFG = ISP_MGR_HFG_T::getInstance(ESensorDev_Main);
*/
    rPGN.setEnable(MTRUE);
    rSL2.setEnable(MTRUE);
    rUDM.setEnable(MTRUE);
    rCCM.setEnable(MTRUE);
    rGGM.setEnable(MTRUE);
    rDBS2.setEnable(MTRUE);
    rOBC2.setEnable(MTRUE);
    rBNR2.setBPC2Enable(MTRUE);
    rBNR2.setCT2Enable(MTRUE);
    rBNR2.setPDC2Enable(MFALSE);
    rBNR2.setCCTBPC2Enable(MTRUE);
    rBNR2.setCCTCT2Enable(MTRUE);
    rBNR2.setCCTPDC2Enable(MFALSE);
    rLSC2.enableLsc(MTRUE);   //setEnable(MFALSE);
    rRNR.setEnable(MTRUE);
    rSL2G.setEnable(MTRUE);
    
    rG2C.setEnable(MTRUE);
//    rG2C_Shad.setEnable(MFALSE);
    rNBC.setANR1Enable(MTRUE);
    rNBC.setCCTANR1Enable(MTRUE);
    rNBC.setANRTBLEnable(MTRUE);
    rNBC2.setANR2Enable(MTRUE);
    rNBC2.setCCREnable(MTRUE);
    rNBC2.setBOKEnable(MTRUE);
    rNBC2.setCCTANR2Enable(MTRUE);
    rNBC2.setCCTCCREnable(MTRUE);
    rNBC2.setCCTBOKEnable(MTRUE);
    rPCA.setEnable(MTRUE);
    rSEEE.setEnable(MTRUE);
/*
    rHFG.setEnable(MFALSE);
    rMFB.setEnable(MFALSE);
    rMIXER3.setEnable(MFALSE);
    rRMM2.setEnable(MFALSE);
    rRMG2.setEnable(MFALSE);
    rLCE.setEnable(MFALSE);
*/

    rPGN.put(m_sPGN);
    rSL2.put(m_sSL2);
    rUDM.put(m_sUDM);
    rCCM.put(m_sCCM);
    rGGM.put(m_sGGM);
    rDBS2.put(m_sDBS);
    rOBC2.put(m_sOBC2);
    rBNR2.put(m_sBNRBpc);
    rBNR2.put(m_sBNRNr1);
    rBNR2.put(m_sBNRPdc);
    rLSC2.put(m_sLSC2);
    if (m_pLsciBuf != NULL)
        rLSC2.putAddr(m_pLsciBuf->getBufPA(0));
    rRNR.put(m_sRNR);
    rSL2G.put(m_sSL2G);
    rG2C.put(m_sG2C);
    rNBC.put(m_sANR);
    rNBC.putANR_TBL(m_sANR_Tbl);    // ANR LUT
    rNBC2.put(m_sANR2);
    rNBC2.put(m_sCCR);
    rPCA.put(m_sPCA);
    rPCA.loadLut(m_u4PCA_Lut);      // PCA LUT
    rSEEE.put(m_sEE);
    rSEEE.put(m_sSE);


//    rPGN.apply(EIspProfile_Capture, pReg);
    rPGN.apply(RawCamInfo, pReg);
    rSL2.apply(CropRzInfo, CropRzInfo, EIspProfile_Capture, pReg);
    rUDM.apply(EIspProfile_Capture, pReg);
//    rCCM.apply(EIspProfile_Capture, pReg);
    rCCM.apply(RawCamInfo, pReg);
    rGGM.apply(EIspProfile_Capture, pReg);
    rDBS2.apply(EIspProfile_Capture, pReg);  
//    rOBC2.apply(EIspProfile_Capture, pReg);
    rOBC2.apply(RawCamInfo, pReg);
    rBNR2.apply(EIspProfile_Capture, pReg);
    rLSC2.apply(EIspProfile_Capture, pReg);
//    rRNR.apply(EIspProfile_Capture, pReg);
    rRNR.apply(RawCamInfo, pReg);
    rSL2G.apply(CropRzInfo, EIspProfile_Capture, pReg);

    rG2C.apply(EIspProfile_Capture, pReg);
//    rG2C_Shad.apply(EIspProfile_Capture, pReg);
    rNBC.apply(EIspProfile_Capture, pReg);
    rNBC2.apply(EIspProfile_Capture, pReg);
    rPCA.apply(EIspProfile_Capture, pReg);
    rSEEE.apply(EIspProfile_Capture, pReg);
/*
    rMFB.apply(EIspProfile_Capture, pReg);
    rMIXER3.apply(EIspProfile_Capture, pReg);
    rRMM2.apply(RawCamInfo, pReg);
    rRMG2.apply(RawCamInfo, pReg);
    rLCE.apply(CropRzInfo, EIspProfile_Capture, pReg);
    rHFG.apply(CropRzInfo, EIspProfile_Capture, pReg);
*/

    // the protection of HW limitations
    //ispHWConstraintSet(pReg);   // the protection of HW limitations
#endif
#if 0

    FILE *fptr;
    if ((fptr=fopen(TUNING_DATA_FILE,"rb"))!=NULL) {
        MY_LOG("[%s] loading tuning data",__FUNCTION__);
        MY_LOG("[%s] m_pTuning %p  size %d",__FUNCTION__, m_pTuning, tuningsize);
        fread(reinterpret_cast<char *>(m_pTuning), 1, tuningsize, fptr);
        fclose(fptr);
    } else {
        MY_LOG("[%s] tuning data not found",__FUNCTION__);
    }

    pReg->DIP_X_CTL_RGB_EN.Raw = mRgbEn;
    pReg->DIP_X_CTL_YUV_EN.Raw = mYuvEn;
    pReg->DIP_X_CTL_YUV2_EN.Raw = (mYuv2En & 0xFF);
    
#endif

#if 1
    {
        FILE *fptr;
        if ((fptr=fopen("/data/cct/TuningVerify.data","wb"))!=NULL) {
            MY_LOG("[%s] saving tuning verify data",__FUNCTION__);
            MY_LOG("[%s] m_pTuning %p  size %d",__FUNCTION__, m_pTuning, tuningsize);
            fwrite(reinterpret_cast<char *>(m_pTuning), 1, tuningsize, fptr);
            fclose(fptr);
        } else {
            MY_LOG("[%s] saving tuning data failed",__FUNCTION__);
        }
    }
#endif

    MY_LOG("[%s] Apply Setting P2 -",__FUNCTION__);
    return fgRet;
}

MBOOL
RawToJpeg::
applySettingFileP2()
{
    MY_LOG("[%s] Apply Setting File P2 +",__FUNCTION__);
    MBOOL fgRet = MTRUE;
    MINT16 i;

    CROP_RZ_INFO_T CropRzInfo;
    RAWIspCamInfo RawCamInfo;

//   setLscBuf(gLscTable, sizeof(gLscTable));

    unsigned int tuningsize = (m_pNormStream != NULL) ? m_pNormStream->getRegTableSize() : 0;//sizeof(dip_x_reg_t);
    //unsigned int tuningsize = sizeof(dip_x_reg_t);
    if (tuningsize == 0) {
        MY_LOG("getRegTableSize is 0 (%p)", m_pNormStream);
    } else {
        MY_LOG("getRegTableSize is %x ", tuningsize);
    }

    if (m_pTuning == NULL )
        m_pTuning = ::malloc(tuningsize);

    if (m_pTuning == NULL) {
        MY_LOG("[%s] Tuning data buffer malloc failed",__FUNCTION__);
        return MFALSE;
    }
        
    dip_x_reg_t* pReg = reinterpret_cast<dip_x_reg_t*>(m_pTuning);
    ::memset((void*)pReg, 0, sizeof(dip_x_reg_t));

#if 0
    pReg->DIP_X_CTL_RGB_EN.Raw = mRgbEn;
    pReg->DIP_X_CTL_YUV_EN.Raw = mYuvEn;
    pReg->DIP_X_CTL_YUV2_EN.Raw = (mYuv2En & 0xFF);
#endif

    FILE *fptr;
    if ((fptr=fopen(TUNING_DATA_FILE,"rb"))!=NULL) {
        MY_LOG("[%s] loading tuning data",__FUNCTION__);
        MY_LOG("[%s] m_pTuning %p  size %d",__FUNCTION__, m_pTuning, tuningsize);
        fread(reinterpret_cast<char *>(m_pTuning), 1, tuningsize, fptr);
        fclose(fptr);
    } else {
        MY_LOG("[%s] tuning data not found",__FUNCTION__);
        return MFALSE;
    }

    pReg->DIP_X_CTL_RGB_EN.Bits.LSC2_EN = 0;
    

#if 1
// the case is working but color is not so good.
    if ((mYuv2En & 0x100)) {
    	pReg->DIP_X_OBC2_OFFST0.Raw                       =   0x00001F00;
    	pReg->DIP_X_OBC2_OFFST1.Raw                       =   0x00001F00;
    	pReg->DIP_X_OBC2_OFFST2.Raw                       =   0x00001F00;
    	pReg->DIP_X_OBC2_OFFST3.Raw                       =   0x00001F00;
    	pReg->DIP_X_OBC2_GAIN0.Raw                        =   0x00000226;
    	pReg->DIP_X_OBC2_GAIN1.Raw                        =   0x00000226;
    	pReg->DIP_X_OBC2_GAIN2.Raw                        =   0x00000226;
    	pReg->DIP_X_OBC2_GAIN3.Raw                        =   0x00000226;
    }

    if ((mYuv2En & 0x200)) {
        pReg->DIP_X_UDM_INTP_CRS.Raw                      =   0x38303060;
        pReg->DIP_X_UDM_INTP_NAT.Raw                      =   0x1431E53F;
        pReg->DIP_X_UDM_INTP_AUG.Raw                      =   0x00500500;
        pReg->DIP_X_UDM_LUMA_LUT1.Raw                     =   0x07FEFB30;
        pReg->DIP_X_UDM_LUMA_LUT2.Raw                     =   0x0361124F;
    //    pReg->DIP_X_UDM_SL_CTL.Raw                        =   0x0039B5A1;
        pReg->DIP_X_UDM_SL_CTL.Raw                        =   0x0039B5A0;
        pReg->DIP_X_UDM_HFTD_CTL.Raw                      =   0x08421000;
        pReg->DIP_X_UDM_NR_STR.Raw                        =   0x01028000;
        pReg->DIP_X_UDM_NR_ACT.Raw                        =   0x00000050;
        pReg->DIP_X_UDM_HF_STR.Raw                        =   0x80120800;
        pReg->DIP_X_UDM_HF_ACT1.Raw                       =   0x46FF41FB;
        pReg->DIP_X_UDM_HF_ACT2.Raw                       =   0x0046FF55;
        pReg->DIP_X_UDM_CLIP.Raw                          =   0x009E3F64;
        pReg->DIP_X_UDM_DSB.Raw                           =   0x007FA800;
        pReg->DIP_X_UDM_LR_RAT.Raw                        =   0x0000000F;
    }

    if ((mYuv2En & 0x400)) {
        pReg->DIP_X_G2C_CONV_0A.Raw                       =   0x012D0099;
        pReg->DIP_X_G2C_CONV_0B.Raw                       =   0x0000003A;
        pReg->DIP_X_G2C_CONV_1A.Raw                       =   0x075607AA;
        pReg->DIP_X_G2C_CONV_1B.Raw                       =   0x00000100;
        pReg->DIP_X_G2C_CONV_2A.Raw                       =   0x072A0100;
        pReg->DIP_X_G2C_CONV_2B.Raw                       =   0x000007D6;
        
        pReg->DIP_X_G2G_CNV_1.Raw                         =   0x1EF20379;
        pReg->DIP_X_G2G_CNV_2.Raw                         =   0x00001F95;
        pReg->DIP_X_G2G_CNV_3.Raw                         =   0x02A51F97;
        pReg->DIP_X_G2G_CNV_4.Raw                         =   0x00001FC4;
        pReg->DIP_X_G2G_CNV_5.Raw                         =   0x1E3F0002;
        pReg->DIP_X_G2G_CNV_6.Raw                         =   0x000003BF;

        pReg->DIP_X_G2G_CTRL.Raw                          =   0x0000;
        pReg->DIP_X_G2G_CTRL.Bits.G2G_ACC                 =   9;
        pReg->DIP_X_G2G_CTRL.Bits.G2G_CFC_EN              =   1;
        pReg->DIP_X_G2G_CTRL.Bits.G2G_H                   =   4063;
        pReg->DIP_X_G2G_CTRL.Bits.G2G_L                   =   32;
        pReg->DIP_X_G2G_CFC.Raw                           =   0x1CE729CE;
    }
    
    if ((mYuv2En & 0x800)) {
        for (i=0; i<144; i++ ) {
        	pReg->DIP_X_GGM_LUT_RB[i].Raw = gGGMLut[i];
        	pReg->DIP_X_GGM_LUT_G[i].Raw = gGGMLut[i+144];
        }
    	pReg->DIP_X_GGM_CTRL.Raw = 0x00;    //0x01; // linear gamma
    }

    pReg->DIP_X_CTL_YUV2_EN.Raw                        =   0x00000000;

    pReg->DIP_X_CTL_RGB_EN.Raw                        =   0x0000;
    pReg->DIP_X_CTL_RGB_EN.Bits.OBC2_EN               =   1;
    pReg->DIP_X_CTL_RGB_EN.Bits.GGM_EN                =   1;
    pReg->DIP_X_CTL_RGB_EN.Bits.G2G_EN                =   1;
    pReg->DIP_X_CTL_RGB_EN.Bits.UDM_EN                =   1;
    pReg->DIP_X_CTL_YUV_EN.Bits.G2C_EN                =   1;

    pReg->DIP_X_CTL_RGB_EN.Raw = mRgbEn;
    pReg->DIP_X_CTL_YUV_EN.Raw = mYuvEn;
    pReg->DIP_X_CTL_YUV2_EN.Raw = (mYuv2En & 0xFF);

    if ( pReg->DIP_X_CTL_RGB_EN.Bits.LSC2_EN == 1 && m_pLsciBuf != NULL ) {
        pReg->DIP_X_CTL_DMA_EN.Bits.DEPI_EN = 1;

        pReg->DIP_X_LSC2_CTL1.Raw = 0x30400000;
        pReg->DIP_X_LSC2_CTL2.Raw = 0x1e0a7;
        pReg->DIP_X_LSC2_CTL3.Raw = 0x1e07d;
        pReg->DIP_X_LSC2_LBLOCK.Raw = 0xa70085;
        pReg->DIP_X_LSC2_RATIO_0.Raw = 0x20202020;
        pReg->DIP_X_LSC2_RATIO_1.Raw = 0x20202020;
        
        pReg->DIP_X_DEPI_OFST_ADDR.Raw  = 0;
        pReg->DIP_X_DEPI_XSIZE.Raw      = 0x60;
        pReg->DIP_X_DEPI_YSIZE.Raw      = 0x01;
        pReg->DIP_X_DEPI_STRIDE.Raw     = 0x60;
        pReg->DIP_X_LSC2_CTL1.Raw       = 0x30400000;
    }
#endif

    MY_LOG("[%s] Apply Setting File P2 -",__FUNCTION__);
    return fgRet;
}


MBOOL
RawToJpeg::
run()
{
    MY_LOG("[%s] Run +",__FUNCTION__);
    MBOOL ret = MFALSE;
    m_bEnqueDone = MFALSE;
//    applySettingFileP2();
    applySettingP2();
    ret = convRawToYuv();

    while (m_bEnqueDone == MFALSE);

    MY_LOG("[%s] enque done",__FUNCTION__);

    yuv2jpg();
    
    MY_LOG("[%s] Run -",__FUNCTION__);

    return MTRUE;

}

void
RawToJpeg::
releaseFile()
{
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    if(m_pPackImgBuf)
    {
        m_pPackImgBuf->unlockBuf(LOG_TAG);
        allocator->free(m_pPackImgBuf);
        m_pPackImgBuf = NULL;
    }

    if(m_pProcBuf)
    {
        m_pProcBuf->unlockBuf(LOG_TAG);
        allocator->free(m_pProcBuf);
        m_pProcBuf = NULL;
    }

    if(m_pJpgbuf)
    {
        m_pJpgbuf->unlockBuf(LOG_TAG);
        allocator->free(m_pJpgbuf);
        m_pJpgbuf = NULL;
    }

    if(m_pYuvBuf)
    {
        m_pYuvBuf->unlockBuf(LOG_TAG);
        allocator->free(m_pYuvBuf);
        m_pYuvBuf = NULL;
    }

    if(m_pUnpackImgBuf)
    {
        free(m_pUnpackImgBuf);
        m_pUnpackImgBuf = NULL;
    }
}


IRawToJpeg*
IRawToJpeg::
createInstance()
{
    return new RawToJpeg();
}

void
RawToJpeg::
destroyInstance()
{
    delete this;
}

void
RawToJpeg::
setIspCtl(MUINT32 RgbEn, MUINT32 YuvEn, MUINT32 Yuv2En)
{
    mRgbEn = RgbEn;
    mYuvEn = YuvEn;
    mYuv2En = Yuv2En;
}
    
#endif
