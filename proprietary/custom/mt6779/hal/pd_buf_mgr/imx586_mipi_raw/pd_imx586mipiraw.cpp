#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>
#include <string.h>


#include <pd_imx586mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <cinttypes>


#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define NEON_OPT 1

#if (NEON_OPT)
#if defined(ANDROID) || defined(__arm__)
#include "arm_neon.h"
#else
#error NEON_OPT is not supported.
#endif
#endif

#define LOG_TAG "pd_buf_mgr_imx586mipiraw_type2"

typedef union _DATA_PARSEER
{
    struct
    {
        unsigned short DATA_R;
        unsigned short DATA_L;
    } DATA;
    unsigned int DATA_ALL;
} DATA_PARSEER;


/*
Currently, sensor driver  outputs Byte2 data format.
For turnkey solution, set 260x384 PD data to PD algorithm for both binning type and hdr type IMX586.
*/

PDBufMgr*
PD_IMX586MIPIRAW_TYPE2::getInstance()
{
    static PD_IMX586MIPIRAW_TYPE2 singleton;
    return &singleton;

}


PD_IMX586MIPIRAW_TYPE2::PD_IMX586MIPIRAW_TYPE2()
{
    AAA_LOGD("%s +", __FUNCTION__);
    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = NULL;
    m_bDebugEnable = MFALSE;
    AAA_LOGD("%s -", __FUNCTION__);
}

PD_IMX586MIPIRAW_TYPE2::~PD_IMX586MIPIRAW_TYPE2()
{
    AAA_LOGD("%s +", __FUNCTION__);
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = NULL;
    AAA_LOGD("%s -", __FUNCTION__);
}


MBOOL PD_IMX586MIPIRAW_TYPE2::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    if( iPdProfile.i4SensorMode==0 || iPdProfile.i4SensorMode==1 || iPdProfile.i4SensorMode==5) {
        /*PREVIEW/CAPTURE/CUSTOM1  12M*/
        m_PDBufXSz = 496;
        m_PDBufYSz = 1488 * 2;

        if(m_PDBuf)
        {
            delete m_PDBuf;
            m_PDBuf = nullptr;
        }
        m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
        m_PDBuf    = new MUINT16 [m_PDBufSz];

        ret = MTRUE;
    } else if (iPdProfile.i4SensorMode==2) {
        if (iPdProfile.uImgXsz == 3840 && iPdProfile.uImgYsz == 2160)
        {
            #define PDBUF_SIZE_X_RAW3840X2160  480
            #define PDBUF_SIZE_Y_RAW3840X2160 (1072 * 2)

            m_PDBufXSz = PDBUF_SIZE_X_RAW3840X2160;
            m_PDBufYSz = PDBUF_SIZE_Y_RAW3840X2160;
        }
        else if (iPdProfile.uImgXsz == 4000 && iPdProfile.uImgYsz == 2600)
        {
            /* VIDEO  4000*2600*/
            m_PDBufXSz = 496;
            m_PDBufYSz = 1296 * 2;
        }
        else
        {
            m_PDBufXSz = 496;
            m_PDBufYSz = 1120 * 2;
        }

        if(m_PDBuf)
        {
            delete m_PDBuf;
            m_PDBuf = nullptr;
        }
        m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
        m_PDBuf    = new MUINT16 [m_PDBufSz];

        ret = MTRUE;
    } else if (iPdProfile.i4SensorMode==4) {
        /* SLIM_VIDEO  4000*2256*/
        m_PDBufXSz = 496;
        m_PDBufYSz = 1128 * 2;

        if(m_PDBuf)
        {
            delete m_PDBuf;
            m_PDBuf = nullptr;
    }
        m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
        m_PDBuf    = new MUINT16 [m_PDBufSz];

        ret = MTRUE;
    } else if (iPdProfile.i4SensorMode==6) {
        /*SENSOR_SCENARIO_ID_CUSTOM2 4K*/
        m_PDBufXSz = 480;
        m_PDBufYSz = 1072 * 2;

        if(m_PDBuf)
    {
            delete m_PDBuf;
            m_PDBuf = nullptr;
        }
        m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
        m_PDBuf    = new MUINT16 [m_PDBufSz];

        ret = MTRUE;
    } else {
        AAA_LOGD("ConvertPDBufFormat is not Supported. ImgSz(%d, %d), SensorMode(%d)", iPdProfile.uImgXsz, iPdProfile.uImgYsz, iPdProfile.i4SensorMode);
    }

    return ret;

}

MBOOL PD_IMX586MIPIRAW_TYPE2::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    MBOOL ret = MFALSE;

    //
    MUINT32 x_sz = m_PDBlockInfo.i4BlockNumX * m_PDBlockInfo.i4PitchX / m_PDBlockInfo.i4SubBlkW;
    MUINT32 y_sz = m_PDBlockInfo.i4BlockNumY * m_PDBlockInfo.i4PitchY / m_PDBlockInfo.i4SubBlkH * 2;
    MUINT32 sz =  x_sz * y_sz;

    //
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    AAA_LOGD("GetLRBufferInfo x/y_sz:(%d*%d), PDX/Ysz(%d *%d)\n", x_sz, y_sz, PDXsz, PDYsz);
    ret = ( (x_sz==PDXsz) && (y_sz==PDYsz) && (sz==PDBufSz)) ? MTRUE : MFALSE;

    if(!ret)
    {
        AAA_LOGE("pd block information is not correct");
    }

    return ret;
}

#if NEON_OPT
static void ConvertMipiRaw10To16Bits(MUINT32 i4Width, MUINT32 i4Height, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT16* ptrL, MUINT16* ptrR)
{
    MUINT16* ptr_pd_m = ptrL;
    MUINT16* ptr_pd_n = ptrR;
    MINT32 i4DstWidth = i4Width * 4 / 5 / 2;

    MUINT8 ucTbl0[5][8] = { { 4,0,1,2,3,5,6,7 },{ 0,1,6,2,3,4,5,7 },{ 0,1,2,3,4,5,6,7 },{ 1,2,3,4,0,5,6,7 },{ 0,1,3,4,5,6,2,7 } };
    MUINT8 ucTbl1[2][8] = { { 0,9,10,19,0xFF,0xFF,0xFF,0xFF } ,{ 0xFF,0xFF,0xFF,0xFF,4,5,14,15 } };
    MUINT8 ucTblShfit[16] = { 6,4,2,0, 6,4,2,0, 6,4,2,0, 6,4,2,0};

    uint8x8_t vu1x8Tbl0_0 = vld1_u8(ucTbl0[0]);
    uint8x8_t vu1x8Tbl0_1 = vld1_u8(ucTbl0[1]);
    uint8x8_t vu1x8Tbl0_3 = vld1_u8(ucTbl0[3]);
    uint8x8_t vu1x8Tbl0_4 = vld1_u8(ucTbl0[4]);

    uint8x8_t vu1x8Tbl1_0 = vld1_u8(ucTbl1[0]);
    uint8x8_t vu1x8Tbl1_1 = vld1_u8(ucTbl1[1]);

    uint8x16_t vu1x16TblShift = vld1q_u8(ucTblShfit);

    for (int y = 0; y < i4Height; y++, ptrBufAddr += i4Stride)
        {
        /**********************************************************************
        * Binning is performed by the sensor module. Each 4x4 pixels produces
        * (is binned into) four 10-bits data packed in five-byte MIPI format.
        *  --------------------------------------------------------------------------------------
        * | M0(10)>>2 | N0(10)>>2 | M1(10)>>2 | N1(10)>>2 | N1(10)&3_M1(10)&3_N0(10)&3_M0(10)&3 |
        * |           |           |           |           |                                     |
        * |   Byte 0  |  Byte 1   |  Byte 2   |  Byte 3   |               Byte 4                |
        *  --------------------------------------------------------------------------------------
        * M: Channel0
        * N: Channel1
        * M(10)    means the 10-bit data of M.
        * M(10)>>2 means the upper 8-bit data of M.
        * M(10)&3  means the lower 2-bit data of M.
        * D:       means the abbr. of the lower 2-bit data of M and N.
        *********************************************************************/


        MUINT8* ptr_src = ptrBufAddr;

        int x = 0;

        uint8x8x3_t vu1x8x3Buf0;
        uint8x8x2_t vu1x8x2Buf1, vu1x8x2TmpBuf;
        uint8x8_t vu1x8LL, vu1x8LH, vu1x8HL, vu1x8HH, vu1x8LSB;
        uint16x8_t vu2x8x1Buf3;
        uint16x8_t vu2x8x1Buf4;

        uint16x8x2_t vu2x8x2Buf5;

        uint8x16_t vu1x16Low2BitL, vu1x16Low2BitH;

        for (; x <= i4DstWidth - 16; x += 16, ptr_pd_m += 16, ptr_pd_n += 16, ptr_src += 16 / 2 * 5)
            {
            //Step.1 Loading data from memory to register
            vu1x8x3Buf0.val[0] = vld1_u8(ptr_src);       //[M0   0, N0   1, M1     2, N1   3, D01  4, M2     5, N2   6, M3     7]
            vu1x8x3Buf0.val[1] = vld1_u8(ptr_src + 8);   //[N3   8, D23  9, M4    10, N4  11, M5  12, N5    13, D45 14, M6    15]
            vu1x8x3Buf0.val[2] = vld1_u8(ptr_src + 16);  //[N6  16, M7  17, N7    18, D67 19, M8  20, N8    21, M9  22, N9    23]
            vu1x8x2Buf1.val[0] = vld1_u8(ptr_src + 24);  //[D89 24, M10 25, N10   26, M11 27, N11 28, D1011 29, M12 30, N12   31]
            vu1x8x2Buf1.val[1] = vld1_u8(ptr_src + 32);  //[M13 32, N13 33, D1213 34, M14 35, N14 36, M15   37, N15 38, D1415 39]

            vu1x8x3Buf0.val[0] = vtbl1_u8(vu1x8x3Buf0.val[0], vu1x8Tbl0_0); //[D01 4,    0,     1,     2,     3,       5,       6,       7]
            vu1x8x3Buf0.val[1] = vtbl1_u8(vu1x8x3Buf0.val[1], vu1x8Tbl0_1); //[    8,D23 9,D45 14,    10,    11,      12,      13,      15]
            //vu1x8x3Buf0.val[2]                                              [   16,   17,    18,D67 19,    20,      21,      22,      23]
            vu1x8x2Buf1.val[0] = vtbl1_u8(vu1x8x2Buf1.val[0], vu1x8Tbl0_3); //[   25,   26,    27,    28,D89 24,D1011 29,      30,      31]
            vu1x8x2Buf1.val[1] = vtbl1_u8(vu1x8x2Buf1.val[1], vu1x8Tbl0_4); //[   32,   33,    35,    36,    37,      38,D1213 34,D1415 39]


            //Step.2 Separate the upper 8-bit data
            vu1x8LL = vext_u8(vu1x8x3Buf0.val[0], vu1x8x3Buf0.val[1], 1); //[M0   0,N0   1,M1   2,N1   3,M2   5,N2   6,M3   7,N3   8]
            vu1x8LH = vext_u8(vu1x8x3Buf0.val[1], vu1x8x3Buf0.val[2], 3); //[M4  10,N4  11,M5  12,N5  13,M6  15,N6  16,M7  17,N7  18]
            vu1x8HL = vext_u8(vu1x8x3Buf0.val[2], vu1x8x2Buf1.val[0], 4); //[M8  20,N8  21,M9  22,N9  23,M10 25,N10 26,M11 27,N11 28]
            vu1x8HH = vext_u8(vu1x8x2Buf1.val[0], vu1x8x2Buf1.val[1], 6); //[M12 30,N12 31,M13 32,N13 33,M14 35,N14 36,M15 37,N15 38]


            //Step.3 Separate the lower 2-bit data
            //   [4,9,14,19, 24,29,34,39] == [D01,D23,D45,D67,D89,D1011,D1213,D1415]
            vu1x8LSB = vorr_u8(vtbl3_u8(vu1x8x3Buf0, vu1x8Tbl1_0), vtbl2_u8(vu1x8x2Buf1, vu1x8Tbl1_1));
            //   [D01,D23,D45,D67,D89,D1011,D1213,D1415] => val[0]: [D01,D01, D23,D23, D45,D45, D67,D67], val[1]: [D89,D89, D1011,D1011, D1213,D1213, D1415,D1415]
            vu1x8x2Buf1 = vzip_u8(vu1x8LSB, vu1x8LSB);

            //    Low: [D01,D01, D23,D23, D45,D45, D67,D67]
            //    => val[0]: [D01,D01,D01,D01, D23,D23,D23,D23], val[1]: [D45,D45,D45,D45, D67,D67,D67,D67]
            vu1x8x2TmpBuf = vzip_u8(vu1x8x2Buf1.val[0], vu1x8x2Buf1.val[0]);
            //    [(D01<<6)>>6, (D01<<4)>>6, (D01<<2)>>6, (D01<<0)>>6,  (D23<<6)>>6 ,(D23<<4)>>6,(D23<<2)>>6,(D23<<0)>>6,
            //     (D45<<6)>>6, (D45<<4)>>6, (D45<<2)>>6, (D45<<0)>>6,  (D67<<6)>>6 ,(D67<<4)>>6,(D67<<2)>>6,(D67<<0)>>6]
            vu1x16Low2BitL = vshrq_n_u8(vshlq_u8(vcombine_u8(vu1x8x2TmpBuf.val[0], vu1x8x2TmpBuf.val[1]), vu1x16TblShift), 6);

            //    High: [D89,D89, D1011,D1011, D1213,D1213, D1415,D1415]
            //    Same as low
            vu1x8x2TmpBuf = vzip_u8(vu1x8x2Buf1.val[1], vu1x8x2Buf1.val[1]);
            vu1x16Low2BitH = vshrq_n_u8(vshlq_u8(vcombine_u8(vu1x8x2TmpBuf.val[0], vu1x8x2TmpBuf.val[1]), vu1x16TblShift), 6);


            //Step.4 upper 8-bit data + lower 2-bit data
            vu2x8x1Buf3 = vorrq_u16(vshlq_n_u16(vmovl_u8(vu1x8LL), 2), vmovl_u8(vget_low_u8(vu1x16Low2BitL))); //[M0,N0,M1,N1,M2,N2,M3,N3]
            vu2x8x1Buf4 = vorrq_u16(vshlq_n_u16(vmovl_u8(vu1x8LH), 2), vmovl_u8(vget_high_u8(vu1x16Low2BitL))); //[M4,N4,M5,N5,M6,N6,M7,N7]

            //   unzip MNMNMNMN data => val[0]: [M0,M1,M2,M3,M4,M5,M6,M7]  val[1]: [N0,N1,N2,N3,N4,N5,N6,N7]
            vu2x8x2Buf5 = vuzpq_u16(vu2x8x1Buf3, vu2x8x1Buf4);

            vst1q_u16(ptr_pd_m, vu2x8x2Buf5.val[0]);
            vst1q_u16(ptr_pd_n, vu2x8x2Buf5.val[1]);

            vu2x8x1Buf3 = vorrq_u16(vshlq_n_u16(vmovl_u8(vu1x8HL), 2), vmovl_u8(vget_low_u8(vu1x16Low2BitH))); //[M8,N8,M9,N9,M10,N10,M11,N11]
            vu2x8x1Buf4 = vorrq_u16(vshlq_n_u16(vmovl_u8(vu1x8HH), 2), vmovl_u8(vget_high_u8(vu1x16Low2BitH)));//[M12,N12,M13,N13,M14,N14,M15,N15]

            //   unzip MNMNMNMN data => val[0]: [M8,M9,M10,M11,M12,M13,M14,M15]  val[1]: [N8,N9,N10,N11,N12,N13,N14,N15]
            vu2x8x2Buf5 = vuzpq_u16(vu2x8x1Buf3, vu2x8x1Buf4);

            vst1q_u16(ptr_pd_m + 8, vu2x8x2Buf5.val[0]);
            vst1q_u16(ptr_pd_n + 8, vu2x8x2Buf5.val[1]);
            }

        for (; x <= i4DstWidth - 2; x += 2, ptr_pd_m += 2, ptr_pd_n += 2, ptr_src += 5)
        {
            ptr_pd_m[0] = (ptr_src[0] << 2) | (ptr_src[4] & 0x3);
            ptr_pd_n[0] = (ptr_src[1] << 2) | ((ptr_src[4] >> 2) & 3);
            ptr_pd_m[1] = (ptr_src[2] << 2) | ((ptr_src[4] >> 4) & 3);
            ptr_pd_n[1] = (ptr_src[3] << 2) | ((ptr_src[4] >> 6) & 3);
        }
        }
    }
#endif //NEON_OPT

MUINT16* PD_IMX586MIPIRAW_TYPE2::ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth, PD_AREA_T* ptrPDRegion)
    {
    AAA_LOGD("[%s] buf info:va(%p), size(0x%x), stride(0x%x), bit depth(%d)", __FUNCTION__, ptrBufAddr, i4Size,i4Stride, u4BitDepth);

    unsigned short *ptrL = m_PDBuf;
    unsigned short *ptrR = m_PDBuf + m_PDBufSz / 2;

#if NEON_OPT
    ConvertMipiRaw10To16Bits(i4Stride, m_PDBufYSz/2, i4Stride, ptrBufAddr, ptrL, ptrR);
#else
    for (int j = 0; j<(m_PDBufYSz/2); j++)
        {
        for (int i = 0; i<i4Stride; i += 5)
             {
                   int idx = j*i4Stride;
                   char val0 = ptrBufAddr[idx + (i + 0)];
                   char val1 = ptrBufAddr[idx + (i + 1)];
                   char val2 = ptrBufAddr[idx + (i + 2)];
                   char val3 = ptrBufAddr[idx + (i + 3)];
                   char val4 = ptrBufAddr[idx + (i + 4)];

                   *ptrL++ = ((val0 << 2) & 0x3FC) | ((val4 >> 0) & 0x3);
                   *ptrR++ = ((val1 << 2) & 0x3FC) | ((val4 >> 2) & 0x3);
                   *ptrL++ = ((val2 << 2) & 0x3FC) | ((val4 >> 4) & 0x3);
                   *ptrR++ = ((val3 << 2) & 0x3FC) | ((val4 >> 6) & 0x3);
             }
        }
#endif

    return m_PDBuf;
}
