#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>

#include <pd_imx499mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx499mipiraw"

/*
Currently, sensor driver  outputs Byte2 data format.
For turnkey solution, set 260x384 PD data to PD algorithm for both binning type and hdr type IMX499.
*/

PDBufMgr*
PD_IMX499MIPIRAW::getInstance()
{
    static PD_IMX499MIPIRAW singleton;
    return &singleton;

}


PD_IMX499MIPIRAW::PD_IMX499MIPIRAW()
{
    AAA_LOGD("[PD Mgr] IMX499\n");
    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = nullptr;
    m_eBufType = EPDBUF_NOTDEF;
}

PD_IMX499MIPIRAW::~PD_IMX499MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = nullptr;
}


MBOOL PD_IMX499MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    // getting sensor type.
    m_SensorType = iPdProfile.u4PDAF_support_type;

    //
    m_eBufType = iPdProfile.BufType;

    //all-pixel mode is supported.
    if( iPdProfile.u4IsZSD!=0  || (iPdProfile.uImgXsz==4656 && iPdProfile.uImgYsz==3496))
    {
        if( m_eBufType==EPDBUF_VC)
        {
            //allocate resource
            m_PDBufXSz = 560;
            m_PDBufYSz = 832;

            if(m_PDBuf)
            {
                delete m_PDBuf;
                m_PDBuf = nullptr;
            }
            m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
            m_PDBuf    = new MUINT16 [m_PDBufSz];
        }

        ret = MTRUE;
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d), type %d\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz, m_SensorType);
    }


    return ret;

}

MBOOL PD_IMX499MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

MUINT16* PD_IMX499MIPIRAW::ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth, PD_AREA_T * /*ptrPDRegion*/)
{
    AAA_LOGD("%s buffer info : size(%d), stride(%d) addr(%p) bitdepth(%d)",
             __FUNCTION__,
             i4Size,
             i4Stride,
             ptrBufAddr,
             u4BitDepth);

    //Input :
    // i4Size        - DMA buffer size
    // ptrBufAddr - data buffer from dma
    // i4FrmCnt   - current frame number

    if(m_eBufType==EPDBUF_RAW_LEGACY)
    {
#if 0
        if( m_PDBuf==NULL)
        {
            //vaild pd data size
            m_PDBufXSz = (m_sPDOHWInfo.u4Pdo_xsz+1)/sizeof(UINT16);
            m_PDBufYSz = (m_sPDOHWInfo.u4Pdo_ysz+1);
            m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
            m_PDBuf    = new MUINT16 [m_PDBufSz];
        }
        separateLR(i4Stride, ptrBufAddr, m_PDBufXSz, m_PDBufYSz, m_PDBuf, u4BitDepth);
#endif
    }
    else if( m_eBufType==EPDBUF_RAW_LEGACY)
    {}
    else if( m_eBufType==EPDBUF_VC)
    {
        if( m_PDBuf && m_PDBufSz)
        {
            char *buf0 = (char *)ptrBufAddr;
            unsigned int stride = 0x02bC;
            unsigned short **ptr = nullptr;
            unsigned short *ptrR = m_PDBuf;
            unsigned short *ptrL = ptrR + m_PDBufSz/2;

            int pdW=m_PDBufXSz, pdH=m_PDBufYSz;
            for( int j=0; j<pdH; j++)
            {
                if(j%4==0 || j%4==2)
                    ptr = &ptrL;
                else
                    ptr = &ptrR;

                for( int i=0; i<(pdW*10/8); i+=5)
                {
                    int idx = j*stride;
                    char val0 = buf0[ idx + (i  )];
                    char val1 = buf0[ idx + (i+1)];
                    char val2 = buf0[ idx + (i+2)];
                    char val3 = buf0[ idx + (i+3)];
                    char val4 = buf0[ idx + (i+4)];

                    *(*ptr)++   = ( (val0<<2) &0x3FC) | ((val4>>0)&0x3);
                    *(*ptr)++   = ( (val1<<2) &0x3FC) | ((val4>>2)&0x3);
                    *(*ptr)++   = ( (val2<<2) &0x3FC) | ((val4>>4)&0x3);
                    *(*ptr)++   = ( (val3<<2) &0x3FC) | ((val4>>6)&0x3);
                }
            }
        }
    }
    else
    {
        if(m_PDBuf)
        {
            delete m_PDBuf;
            m_PDBuf = nullptr;
            m_PDBufSz = 0;
        }

    }

    return m_PDBuf;
}

