#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>
#include <string.h>


#include <pd_imx398mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx398mipiraw"

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
    AAA_LOGD("%s +", __FUNCTION__);
    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = NULL;
    m_bDebugEnable = MFALSE;
    AAA_LOGD("%s -", __FUNCTION__);
}

PD_IMX398MIPIRAW::~PD_IMX398MIPIRAW()
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


MBOOL PD_IMX398MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    // Checking ConvertPDBufFormat is support or not.
    if( iPdProfile.u4IsZSD!=0 && iPdProfile.i4SensorMode==1 /*SENSOR_SCENARIO_ID_NORMAL_CAPTURE*/ )
    {
        //all-pixel mode is supported.
        ret = MTRUE;
    }
    else if( iPdProfile.i4SensorMode==2 /*SENSOR_SCENARIO_ID_NORMAL_VIDEO*/)
    {
        ret = MTRUE;
    }
    else
    {
        AAA_LOGD("ConvertPDBufFormat is not Supported. ImgSz(%d, %d), SensorMode(%d)", iPdProfile.uImgXsz, iPdProfile.uImgYsz, iPdProfile.i4SensorMode);
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

MUINT16* PD_IMX398MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth, PD_AREA_T* ptrPDRegion)
{
    /*******************************************
     * Input :
     *      i4Size     - DMA buffer size
     *      ptrBufAddr - data buffer from DMA
     *******************************************/

    MUINT32 XSz = (m_sPDOHWInfo.u4Pdo_xsz+1)/2/sizeof(UINT16); // (Pixle)
    MUINT32 YSz = (m_sPDOHWInfo.u4Pdo_ysz+1)*2; // (Line)

    if( m_PDBufSz!=(XSz*YSz))
    {
        if(m_PDBuf)
            delete m_PDBuf;
        m_PDBuf=NULL;
    }

    if( m_PDBuf==NULL)
    {
        //vaild pd data size
        m_PDBufXSz = (m_sPDOHWInfo.u4Pdo_xsz+1)/2/sizeof(UINT16);
        m_PDBufYSz = (m_sPDOHWInfo.u4Pdo_ysz+1)*2;
        m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
        m_PDBuf    = new MUINT16 [m_PDBufSz];
#if 0
        m_PDBuf    = new MUINT16 [m_PDBufSz*2]; //Testing
#endif
    }


    AAA_LOGD_IF(m_bDebugEnable, "%s: va(%p) stride(%d)", __FUNCTION__, ptrBufAddr, i4Stride);

    if(ptrPDRegion)
    {
        PD_AREA_T PDRegion;
        PDRegion.i4X = ptrPDRegion->i4X;
        PDRegion.i4Y = ptrPDRegion->i4Y;
        PDRegion.i4W = ptrPDRegion->i4W;
        PDRegion.i4H = ptrPDRegion->i4H*4;
        separateLR(i4Stride, ptrBufAddr, m_PDBufXSz, m_PDBufYSz, m_PDBuf, u4BitDepth, &PDRegion);
    }
    else
    {
        separateLR(i4Stride, ptrBufAddr, m_PDBufXSz, m_PDBufYSz, m_PDBuf, u4BitDepth, NULL);
    }


#if 0
    {
        MUINT16 *tmpbuf = m_PDBuf + m_PDBufSz;
        unsigned char  *ptrSrc = (unsigned char*) ptrBufAddr;
        unsigned short *ptrL   = &(tmpbuf[0]);
        unsigned short *ptrR   = &(tmpbuf[m_PDBufXSz*m_PDBufYSz/2]);

        for( unsigned int j=0; j<m_PDBufYSz/2; j++)
        {
            DATA_PARSEER *ptrData = (DATA_PARSEER *)ptrSrc;
            for( unsigned int i=0; i<m_PDBufXSz; i++)
            {
                *(ptrL++) = (ptrData[i].DATA.DATA_L)>>2;
                *(ptrR++) = (ptrData[i].DATA.DATA_R)>>2;
            }
            ptrSrc += i4Stride;
        }
        if( !memcmp ( (void *)tmpbuf, (void *)m_PDBuf, m_PDBufSz*sizeof(unsigned short)))
        {
            AAA_LOGD("%s: MATCH!!!", __FUNCTION__);
        }
        else
        {
            AAA_LOGD("%s: NOT MATCH", __FUNCTION__);
        }
    }
#endif


    return m_PDBuf;
}

