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

    if( iPdProfile.i4SensorMode==0 /*SENSOR_SCENARIO_ID_NORMAL_PREVIEW*/ || iPdProfile.i4SensorMode==1 /*SENSOR_SCENARIO_ID_NORMAL_CAPTURE*/)
    {
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
    }
    if( iPdProfile.i4SensorMode==2 /*SENSOR_SCENARIO_ID_NORMAL_VIDEO*/)
    {
        if (iPdProfile.uImgXsz == 3840 && iPdProfile.uImgYsz == 2160)
        {
            #define PDBUF_SIZE_X_RAW3840X2160  480
            #define PDBUF_SIZE_Y_RAW3840X2160 (1072 * 2)

            m_PDBufXSz = PDBUF_SIZE_X_RAW3840X2160;
            m_PDBufYSz = PDBUF_SIZE_Y_RAW3840X2160;
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
    }
    else
    {
        AAA_LOGD("ConvertPDBufFormat is not Supported. ImgSz(%d, %d), SensorMode(%d)", iPdProfile.uImgXsz, iPdProfile.uImgYsz, iPdProfile.i4SensorMode);
    }

    return ret;

}

MBOOL PD_IMX586MIPIRAW_TYPE2::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    MBOOL ret = MFALSE;

    //
    MUINT32 x_sz = m_PDBlockInfo.i4BlockNumX * m_PDBlockInfo.i4PitchX / m_PDBlockInfo.i4SubBlkW;
    MUINT32 y_sz = m_PDBlockInfo.i4BlockNumY * m_PDBlockInfo.i4PitchY / m_PDBlockInfo.i4SubBlkH;
    MUINT32 sz =  x_sz * y_sz;

    //
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    //
    ret = ( (x_sz==PDXsz) && (y_sz==PDYsz) && (sz==PDBufSz)) ? MTRUE : MFALSE;

    if(!ret)
    {
        AAA_LOGE("pd block information is not correct");
    }

    return ret;
}

MUINT16* PD_IMX586MIPIRAW_TYPE2::ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth, PD_AREA_T* ptrPDRegion)
{
    AAA_LOGD("[%s] buf info:va(%p), size(0x%x), stride(0x%x), bit depth(%d)", __FUNCTION__, ptrBufAddr, i4Size,i4Stride, u4BitDepth);

    unsigned short *ptrL = m_PDBuf;
    unsigned short *ptrR = m_PDBuf + m_PDBufSz / 2;

    if (m_sPdProfile.i4SensorMode == 0)
    {
        int YSz = m_PDBufYSz / 2;
        for (int j = 0; j<YSz; j++)
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
    }
    else if ((m_sPdProfile.i4SensorMode == 2) && (m_sPdProfile.uImgXsz == 3840 && m_sPdProfile.uImgYsz == 2160))
    {
        #define VCBUF_SIZE_X_RAW3840X2160 0x4B0
        #define VCBUF_SIZE_Y_RAW3840X2160 0x430

        MUINT8 *ptr  = ptrBufAddr;
        MUINT8 *pRaw;
        MUINT32 x, y;
        for (y = 0; y < VCBUF_SIZE_Y_RAW3840X2160; y++)
        {
            pRaw = ptr;
            for (x = 0; x < VCBUF_SIZE_X_RAW3840X2160; x += 5)
            {
                char val0 = *(pRaw++);
                char val1 = *(pRaw++);
                char val2 = *(pRaw++);
                char val3 = *(pRaw++);
                char val4 = *(pRaw++);

                *ptrL++ = ((val0 << 2) & 0x3FC) | ((val4 >> 0) & 0x3);
                *ptrR++ = ((val1 << 2) & 0x3FC) | ((val4 >> 2) & 0x3);
                *ptrL++ = ((val2 << 2) & 0x3FC) | ((val4 >> 4) & 0x3);
                *ptrR++ = ((val3 << 2) & 0x3FC) | ((val4 >> 6) & 0x3);
            }

            ptr += i4Stride;
        }
    }
    else if (m_sPdProfile.i4SensorMode == 1 || m_sPdProfile.i4SensorMode == 2)
    {
        int YSz = (m_PDBufYSz/2)*2;
        int XSz = m_PDBufXSz/2*2*10/8;
        for (int j = 0; j<YSz; j++)
        {
             for (int i = 0; i<XSz; i += 5)
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
    }

    return m_PDBuf;
}

