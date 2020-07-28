#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>

#include <pd_s5k2p7mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_s5k2p7mipiraw"
#define BINING_UIMGYSZ  1744

#define PDBUF_SIZE_X 288    // (72*4) /* pd info */
#define PDBUF_SIZE_Y 432    // (54*4*2)
#define PDBUF_SIZE   124416 // (PDBUF_SIZE_X * PDBUF_SIZE_Y)

#define VCBUF_STRIDE    0xB8 /* VC Info */
#define VCBUF_SIZE_X    0xB4
#define VCBUF_SIZE_Y    0x360
#define VCBUF_SIZE      (VCBUF_STRIDE * VCBUF_SIZE_Y) /* RAW10 format */

PDBufMgr*
PD_S5K2P7MIPIRAW::getInstance()
{
    static PD_S5K2P7MIPIRAW singleton;
    return &singleton;
}


PD_S5K2P7MIPIRAW::PD_S5K2P7MIPIRAW() :
    m_PDBufXSz(0),
    m_PDBufYSz(0),
    m_PDBufSz(0),
    m_PDBuf(nullptr)
{
    AAA_LOGD("[PD Mgr] S5K2P7\n");
    m_bDebugEnable = MFALSE;
}

PD_S5K2P7MIPIRAW::~PD_S5K2P7MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_S5K2P7MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    //all-pixel mode is supported. + 4640*2680 (16:9) supported + binning mode(4:3)
    if ( iPdProfile.u4IsZSD !=0 || iPdProfile.uImgYsz == BINING_UIMGYSZ)
    {
        if( m_PDBuf)
        {
            delete m_PDBuf;
            m_PDBufSz = 0;
            m_PDBuf = NULL;
        }

        m_PDBufXSz = PDBUF_SIZE_X;
        m_PDBufYSz = PDBUF_SIZE_Y;
        m_PDBufSz  = m_PDBufXSz * m_PDBufYSz;
        m_PDBuf    = new MUINT16 [m_PDBufSz];

        ret = MTRUE;
        AAA_LOGD("PDAF Mode is Supported with (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    return ret;

}

MBOOL PD_S5K2P7MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz   = m_PDBufXSz;
    PDYsz   = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

MUINT16* PD_S5K2P7MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 /*u4BitDepth*/, PD_AREA_T* /*ptrPDRegion*/)
{
    MUINT8 *ptr = ptrBufAddr;
    MUINT32 stride  = i4Stride; //180 bytes

    AAA_LOGD("m_PDBufSz(%d) = m_PDBufXSz(%d) * m_PDBufXSz(%d) ,stride(%d)\n", m_PDBufSz, m_PDBufXSz, m_PDBufYSz, stride);

    MUINT16 *ptrL = m_PDBuf;
    MUINT16 *ptrR = m_PDBuf + m_PDBufSz / 2;

    MUINT8 *pRaw[16]; /* Readout 16 lines*/
    MUINT8 RawIdxL[8] = {0, 2, 5, 7,   9,  8, 15, 14}; /* {row number index of the first element , row number index of the next element} */
    MUINT8 RawIdxR[8] = {1, 3, 4, 6,  11, 10, 13, 12}; /* {row number index of the first element , row number index of the next element} */
    MUINT8 *pRawF_L, *pRawN_L;
    MUINT8 *pRawF_R, *pRawN_R;
    MUINT8 Fval0, Fval1, Fval2, Fval3, Fval4; /* First element */
    MUINT8 Nval0, Nval1, Nval2, Nval3, Nval4; /* Next element */

    MUINT32 x = 0, y = 0, z = 0;

    for (y = 0; y < VCBUF_SIZE_Y; y += 16)
    {
        for (z = 0; z < 16; z++)
        {
            pRaw[z] = ptr;
            ptr += stride;
        }

        for (int z = 0; z < 8; z += 2)
        {
            pRawF_L = pRaw[RawIdxL[z]];
            pRawN_L = pRaw[RawIdxL[z+1]];
            pRawF_R = pRaw[RawIdxR[z]];
            pRawN_R = pRaw[RawIdxR[z+1]];

            for (x = 0; x < VCBUF_SIZE_X; x += 5) /* Raw 10*/
            {
                Fval0 = *(pRawF_L++);
                Fval1 = *(pRawF_L++);
                Fval2 = *(pRawF_L++);
                Fval3 = *(pRawF_L++);
                Fval4 = *(pRawF_L++);

                Nval0 = *(pRawN_L++);
                Nval1 = *(pRawN_L++);
                Nval2 = *(pRawN_L++);
                Nval3 = *(pRawN_L++);
                Nval4 = *(pRawN_L++);

                *(ptrL++) = ((Fval0 << 2) & 0x3FC) | ((Fval4 >> 0) & 0x3);
                *(ptrL++) = ((Nval0 << 2) & 0x3FC) | ((Nval4 >> 0) & 0x3);
                *(ptrL++) = ((Nval1 << 2) & 0x3FC) | ((Nval4 >> 2) & 0x3);
                *(ptrL++) = ((Fval1 << 2) & 0x3FC) | ((Fval4 >> 2) & 0x3);
                *(ptrL++) = ((Fval2 << 2) & 0x3FC) | ((Fval4 >> 4) & 0x3);
                *(ptrL++) = ((Nval2 << 2) & 0x3FC) | ((Nval4 >> 4) & 0x3);
                *(ptrL++) = ((Nval3 << 2) & 0x3FC) | ((Nval4 >> 6) & 0x3);
                *(ptrL++) = ((Fval3 << 2) & 0x3FC) | ((Fval4 >> 6) & 0x3);

                Fval0 = *(pRawF_R++);
                Fval1 = *(pRawF_R++);
                Fval2 = *(pRawF_R++);
                Fval3 = *(pRawF_R++);
                Fval4 = *(pRawF_R++);

                Nval0 = *(pRawN_R++);
                Nval1 = *(pRawN_R++);
                Nval2 = *(pRawN_R++);
                Nval3 = *(pRawN_R++);
                Nval4 = *(pRawN_R++);

                *(ptrR++) = ((Fval0 << 2) & 0x3FC) | ((Fval4 >> 0) & 0x3);
                *(ptrR++) = ((Nval0 << 2) & 0x3FC) | ((Nval4 >> 0) & 0x3);
                *(ptrR++) = ((Nval1 << 2) & 0x3FC) | ((Nval4 >> 2) & 0x3);
                *(ptrR++) = ((Fval1 << 2) & 0x3FC) | ((Fval4 >> 2) & 0x3);
                *(ptrR++) = ((Fval2 << 2) & 0x3FC) | ((Fval4 >> 4) & 0x3);
                *(ptrR++) = ((Nval2 << 2) & 0x3FC) | ((Nval4 >> 4) & 0x3);
                *(ptrR++) = ((Nval3 << 2) & 0x3FC) | ((Nval4 >> 6) & 0x3);
                *(ptrR++) = ((Fval3 << 2) & 0x3FC) | ((Fval4 >> 6) & 0x3);
            }
        }
    }

    return m_PDBuf;
}

