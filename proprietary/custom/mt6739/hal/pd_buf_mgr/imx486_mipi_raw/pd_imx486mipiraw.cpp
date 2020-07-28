#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>

#include <pd_imx486mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_imx486mipiraw"

#define _IMX486_RAW_TYPE_ 1
#define _IMX486_HDR_TYPE_ 2
#define _IMX486_BINNING_TYPE_ 3
#define _IMX486_PURE_RAW_TYPE_ 6


/*
Currently, sensor driver  outputs Byte2 data format.
For turnkey solution, set 260x384 PD data to PD algorithm for both binning type and hdr type IMX486.
*/

PDBufMgr*
PD_IMX486MIPIRAW::getInstance()
{
    static PD_IMX486MIPIRAW singleton;
    return &singleton;

}


PD_IMX486MIPIRAW::PD_IMX486MIPIRAW()
{
    AAA_LOGD("[PD Mgr] IMX486\n");
    m_PDBufSz  = 0;
    m_PDBuf    = NULL;
    m_PDBufXSz    = 0;
    m_PDBufYSz    = 0;
    m_eBufType = EPDBUF_NOTDEF;
}

PD_IMX486MIPIRAW::~PD_IMX486MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz  = 0;
    m_PDBuf    = NULL;
    m_PDBufXSz    = 0;
    m_PDBufYSz    = 0;
    m_eBufType = EPDBUF_NOTDEF;
}


MBOOL PD_IMX486MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //all-pixel mode is supported.
    if( iPdProfile.u4IsZSD!=0)
    {
        ret = MTRUE;
    }
    else
    {
        ALOGD("PDAF Mode is not Supported (%d, %d)", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }

    m_eBufType = iPdProfile.BufType;

    return ret;

}

MBOOL PD_IMX486MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

void PD_IMX486MIPIRAW::seprate( int stride, unsigned char *ptr, int pd_x_num, int pd_y_num, int LROutSz, unsigned short *ptrLROut,unsigned int bitDepth)
{
    unsigned int pdW = pd_x_num;
    unsigned int pdH = pd_y_num;

    //separate L and R pd data
    unsigned short *ptrbuf = (unsigned short *)ptr;
    unsigned short **ptrtmp = NULL;
    unsigned short *ptrL = ptrLROut;
    unsigned short *ptrR = &ptrLROut[LROutSz/2];

    unsigned int shift_bits = 0;// converting data to 10 bits

    if( (10<=bitDepth) && (bitDepth<=16))
    {
        shift_bits = bitDepth - 10;
    }
    else
    {
        AAA_LOGE("%s abort : wrong input bitDepth(%d)", __FUNCTION__, bitDepth);
        return;
    }

    for ( unsigned int i=0; i < pdH; i++ )
    {
        //RLLR
        if(i%4==0 || i%4==3)
            ptrtmp = &ptrL;
        else
            ptrtmp = &ptrR;


        for ( unsigned int j=0; j < pdW; j++ )
        {
            unsigned short val = ptrbuf[i*stride/2+j];
            (*ptrtmp)[j] = val>>shift_bits;
        }
        (*ptrtmp) += pdW;
    }
}

MUINT16* PD_IMX486MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 u4BitDepth, PD_AREA_T* /*ptrPDRegion*/)
{
    if( m_eBufType==EPDBUF_PDO)
    {
        if( m_PDBuf==NULL)
        {
            //first in allocate local PD buffer directly.
            //vaild pd data size
            m_PDBufXSz   = (m_sPDOHWInfo.u4Pdo_xsz+1)/2;
            m_PDBufYSz   = (m_sPDOHWInfo.u4Pdo_ysz+1);
            m_PDBufSz = m_PDBufXSz*m_PDBufYSz;
            m_PDBuf = new MUINT16 [m_PDBufSz];
        }
        seprate( i4Stride, ptrBufAddr, m_PDBufXSz, m_PDBufYSz, m_PDBufSz, m_PDBuf, u4BitDepth);
    }
    else if( m_eBufType==EPDBUF_VC)
    {
        if( m_PDBuf==NULL)
        {
            //first in allocate local PD buffer directly.
            //vaild pd data size
            m_PDBufXSz   = 280; // pixel
            m_PDBufYSz   = 832; // line
            m_PDBufSz = m_PDBufXSz*m_PDBufYSz;
            m_PDBuf   = new MUINT16 [m_PDBufSz];
        }

        //convert format from DMA buffer format(Raw10) to pixel format
        //convert format to PD core algorithm input
        MUINT16 **ptr = NULL;
        MUINT16 *ptrL =   m_PDBuf;
        MUINT16 *ptrR = &(m_PDBuf[(m_PDBufYSz/2)*m_PDBufXSz]);

        MUINT32 xSzInByte = m_PDBufXSz * 10 / 8;
        MUINT32 i, j, k;
        for( j=0, k=0; j<m_PDBufYSz; j++)
        {
            if( j%4==1 || j%4==2)
                ptr = &ptrL;
            else
                ptr = &ptrR;

            for( i=0; i<xSzInByte; i+=5)
            {
                int idx = j*i4Stride;
                char val0 = ptrBufAddr[ idx + (i  )];
                char val1 = ptrBufAddr[ idx + (i+1)];
                char val2 = ptrBufAddr[ idx + (i+2)];
                char val3 = ptrBufAddr[ idx + (i+3)];
                char val4 = ptrBufAddr[ idx + (i+4)];

                *(*ptr)++    = ( ((val1&0x3 ) <<8) &0x300) | ((val0>>0)&0xFF);
                *(*ptr)++    = ( ((val2&0xF ) <<6) &0x3C0) | ((val1>>2)&0x3F);
                *(*ptr)++    = ( ((val3&0x3F) <<4) &0x3F0) | ((val2>>4)&0xF );
                *(*ptr)++    = ( ((val4&0xFF) <<2) &0x3FC) | ((val3>>6)&0x3 );
                k+=4;
            }

        }
        ALOGD("total %d pixels, sz %d, Stride %d\n", k, m_PDBufSz, i4Stride);

    }
    else
    {
        m_PDBuf = NULL;
    }

    return m_PDBuf;
}
