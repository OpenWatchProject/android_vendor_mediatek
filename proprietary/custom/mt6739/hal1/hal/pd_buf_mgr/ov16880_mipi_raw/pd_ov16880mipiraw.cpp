#include <log/log.h>
#include <fcntl.h>
#include <math.h>
#include <pd_ov16880mipiraw.h>
#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_log.h>

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_ov16880mipiraw"


PDBufMgr*
PD_OV16880MIPIRAW::getInstance()
{
    static PD_OV16880MIPIRAW singleton;
    return &singleton;
}

PD_OV16880MIPIRAW::PD_OV16880MIPIRAW()
{
    ALOGD("OV16880");
    m_PDBufSz  = 0;
    m_PDBuf    = NULL;
    m_PDXSz    = 0;
    m_PDYSz    = 0;
    m_eBufType = EPDBUF_NOTDEF;
}

PD_OV16880MIPIRAW::~PD_OV16880MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf   = NULL;
    m_PDXSz   = 0;
    m_PDYSz   = 0;
}

MBOOL PD_OV16880MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
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


void PD_OV16880MIPIRAW::seprate( int stride, unsigned char *ptr, int pd_x_num, int pd_y_num, int LROutSz, unsigned short *ptrLROut, int iShift)
{
    unsigned int pdW = pd_x_num;
    unsigned int pdH = pd_y_num;

    //separate L and R pd data
    unsigned short *ptrbuf = (unsigned short *)ptr;
    unsigned short **ptrtmp = NULL;
    unsigned short *ptrL = ptrLROut;
    unsigned short *ptrR = &ptrLROut[LROutSz/2];

    for ( unsigned int i=0; i < pdH; i++ )
    {
        //RLLR
        if(i%4==0 || i%4==3)
            ptrtmp = &ptrR;
        else
            ptrtmp = &ptrL;


        for ( unsigned int j=0; j < pdW; j++ )
        {
            unsigned short val = ptrbuf[i*stride/2+j];
            (*ptrtmp)[j] = val>>iShift;
        }
        (*ptrtmp) += pdW;
    }
}


MUINT16* PD_OV16880MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL b14BitData)
{
    if( m_eBufType==EPDBUF_PDO)
    {
        if( m_PDBuf==NULL)
        {
            //first in allocate local PD buffer directly.
            //vaild pd data size
            m_PDXSz   = (m_bpciInfo.pdo_xsize+1)/2;
            m_PDYSz   = (m_bpciInfo.pdo_ysize+1);
            m_PDBufSz = m_PDXSz*m_PDYSz;
            m_PDBuf = new MUINT16 [m_PDBufSz];
        }
        seprate( i4Stride, ptrBufAddr, m_PDXSz, m_PDYSz, m_PDBufSz, m_PDBuf, (b14BitData ? 4 : 2));
    }
    else if( m_eBufType==EPDBUF_VC)
    {
        if( m_PDBuf==NULL)
        {
            //first in allocate local PD buffer directly.
            //vaild pd data size
            m_PDXSz   = 280; // pixel
            m_PDYSz   = 832; // line
            m_PDBufSz = m_PDXSz*m_PDYSz;
            m_PDBuf   = new MUINT16 [m_PDBufSz];
        }

        //convert format from DMA buffer format(Raw10) to pixel format
        //convert format to PD core algorithm input
        MUINT16 **ptr = NULL;
        MUINT16 *ptrL =   m_PDBuf;
        MUINT16 *ptrR = &(m_PDBuf[(m_PDYSz/2)*m_PDXSz]);

        MUINT32 xSzInByte = m_PDXSz * 10 / 8;
        MUINT32 i, j, k;
        for( j=0, k=0; j<m_PDYSz; j++)
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

MBOOL PD_OV16880MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDXSz;
    PDYsz = m_PDYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

