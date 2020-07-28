#include <log/log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>

#include <pd_imx398mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx398mipiraw"

/*
Currently, sensor driver  outputs Byte2 data format.
For turnkey solution, set 260x384 PD data to PD algorithm for both binning type and hdr type IMX398.
*/

PDBufMgr*
PD_IMX398MIPIRAW::getInstance()
{
    static PD_IMX398MIPIRAW singleton;
    return &singleton;

}


PD_IMX398MIPIRAW::PD_IMX398MIPIRAW()
{
    AAA_LOGD("[PD Mgr] IMX398\n");
    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = NULL;
    m_bDebugEnable = MFALSE;
}

PD_IMX398MIPIRAW::~PD_IMX398MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_IMX398MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    //all-pixel mode is supported.
    if( iPdProfile.u4IsZSD!=0 )
    {
        ret = MTRUE;
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }

    return ret;

}

MBOOL PD_IMX398MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

MUINT16* PD_IMX398MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL b14BitData)
{
    //Input :
    // i4Size        - DMA buffer size
    // ptrBufAddr - data buffer from dma
    // i4FrmCnt   - current frame number

    if( m_PDBuf==NULL)
    {
        //vaild pd data size
        m_PDBufXSz = (m_bpciInfo.pdo_xsize+1)/2/sizeof(UINT16);
        m_PDBufYSz = (m_bpciInfo.pdo_ysize+1)*2;
        m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
        m_PDBuf    = new MUINT16 [m_PDBufSz];
    }
    separateLR(i4Stride, ptrBufAddr, m_PDBufXSz, m_PDBufYSz, m_PDBuf, (b14BitData ? 4 : 2));

    return m_PDBuf;
}

