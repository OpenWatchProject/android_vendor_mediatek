#include <log/log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>

#include <pd_imx258mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx258mipiraw"

#define _IMX258_RAW_TYPE_ 1
#define _IMX258_HDR_TYPE_ 2
#define _IMX258_BINNING_TYPE_ 3
#define _IMX258_PURE_RAW_TYPE_ 6


/*
Currently, sensor driver  outputs Byte2 data format.
For turnkey solution, set 260x384 PD data to PD algorithm for both binning type and hdr type IMX258.
*/

PDBufMgr*
PD_IMX258MIPIRAW::getInstance()
{
    static PD_IMX258MIPIRAW singleton;
    return &singleton;

}


PD_IMX258MIPIRAW::PD_IMX258MIPIRAW()
{
    AAA_LOGD("[PD Mgr] IMX258\n");
    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = NULL;
}

PD_IMX258MIPIRAW::~PD_IMX258MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufXSz = 0;
    m_PDBufYSz = 0;
    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_IMX258MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);


    // getting sensor type.
    m_SensorType = iPdProfile.u4PDAF_support_type;


    //all-pixel mode is supported.
    if( iPdProfile.u4PDAF_support_type==_IMX258_HDR_TYPE_) //HDR type.
    {
        if( iPdProfile.u4IsZSD!=0)
        {
            ret = MTRUE;
        }
    }
    else if( iPdProfile.u4PDAF_support_type==_IMX258_BINNING_TYPE_) //Binning type.
    {
        if( iPdProfile.u4IsZSD!=0  || (iPdProfile.uImgXsz==2100 && iPdProfile.uImgYsz==1560))
        {
            ret = MTRUE;
        }
    }
    else if( iPdProfile.u4PDAF_support_type==_IMX258_RAW_TYPE_) //Raw type.
    {
        if( iPdProfile.u4IsZSD!=0 )
        {
            ret = MTRUE;
        }
    }
    else if( iPdProfile.u4PDAF_support_type==_IMX258_PURE_RAW_TYPE_) //Pure raw type.
    {
        if( iPdProfile.u4IsZSD!=0 )
        {
            ret = MTRUE;
        }
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d), type %d\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz, m_SensorType);
    }

    m_eBufType = iPdProfile.BufType;

    return ret;

}

MBOOL PD_IMX258MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

MUINT16* PD_IMX258MIPIRAW::ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL b14BitData)
{
    //Input :
    // i4Size        - DMA buffer size
    // ptrBufAddr - data buffer from dma
    // i4FrmCnt   - current frame number

    if (m_eBufType==EPDBUF_PDO)
    {
        if( m_PDBuf==NULL)
        {
            //vaild pd data size
            m_PDBufXSz = (m_bpciInfo.pdo_xsize+1)/sizeof(UINT16);
            m_PDBufYSz = (m_bpciInfo.pdo_ysize+1);
            m_PDBufSz  = m_PDBufXSz*m_PDBufYSz;
            m_PDBuf    = new MUINT16 [m_PDBufSz];
        }
        separateLR(i4Stride, ptrBufAddr, m_PDBufXSz, m_PDBufYSz, m_PDBuf, (b14BitData ? 4 : 2));
    }
    else if (m_eBufType==EPDBUF_VC)
    {
        unsigned short *vcData = (unsigned short*)ptrBufAddr;
        unsigned short pixRead;
        unsigned int pixCount = 0;
        static unsigned int padding[3];

        if (m_PDBufXSz == 0) //parse for pd buffer info once
        {
            unsigned len;

            //parse for LR buffer info
            len = 0;
            while ((pixRead = vcData[pixCount]) == 0)   //buffer head padding
            {
                pixCount ++;
                len ++;
            }
            padding[0] = len;

            len = 0;
            while ((pixRead = vcData[pixCount]) != 0)   //R data
            {
                pixCount ++;
                len ++;
            }
            m_PDBufXSz = len;

            len = 0;
            while ((pixRead = vcData[pixCount]) == 0)   //padding between R & L
            {
                pixCount ++;
                len ++;
            }
            padding[1] = len;

            pixCount += m_PDBufXSz; //L data

            len = 0;
            while ((pixRead = vcData[pixCount]) == 0)   //padding to next R & L
            {
                pixCount ++;
                len ++;
            }
            padding[2] = len;

            //decide LR buffer size
            m_PDBufYSz = i4Size / (m_PDBufXSz * 2 + padding[1] + padding[2]);
            m_PDBufSz = m_PDBufXSz*m_PDBufYSz;
            m_PDBuf = new MUINT16 [m_PDBufSz];
        }

        //write to R L
        unsigned short *pL = m_PDBuf;
        unsigned short *pR = pL + m_PDBufSz/2;
        unsigned int idx = 0;

        //read from data start
        pixCount = padding[0];

        for (MUINT32 y=0; y<m_PDBufYSz/2; y++)
        {
            memcpy((void*)&pL[idx], (void*)&vcData[pixCount], m_PDBufXSz * sizeof(unsigned short));
            pixCount += m_PDBufXSz + padding[1];
            memcpy((void*)&pR[idx], (void*)&vcData[pixCount], m_PDBufXSz * sizeof(unsigned short));
            pixCount += m_PDBufXSz + padding[2];

            idx += m_PDBufXSz;
        }
    }
    else
    {
        m_PDBuf = NULL;
    }

    return m_PDBuf;
}

