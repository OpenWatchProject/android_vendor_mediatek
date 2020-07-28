#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_ov23850mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_ov23850mipiraw"

PDBufMgr*
PD_OV23850MIPIRAW::getInstance()
{
    static PD_OV23850MIPIRAW singleton;
    return &singleton;

}


PD_OV23850MIPIRAW::PD_OV23850MIPIRAW()
{
    AAA_LOGD("[PD Mgr] OV23850\n");
    m_PDBufSz = 0;
    m_PDBuf = NULL;
}

PD_OV23850MIPIRAW::~PD_OV23850MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_OV23850MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //all-pixel mode is supported.
    if( iPdProfile.u4IsZSD!=0)
    {
        ret = MTRUE;
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    return ret;

}

MBOOL PD_OV23850MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = 0xA8;
    PDYsz = 0x800;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

MUINT16* PD_OV23850MIPIRAW::ConvertPDBufFormat( MUINT32  /*i4Size*/, MUINT32 /*i4Stride*/, MUINT8 *ptrBufAddr, MBOOL /*b14BitData*/)
{
    const MUINT32 w=(0xA8)<<1, h=((0x800)>>1)-8;
    const MUINT32 sz=w*h;

    //input i4Size is DMA size.
    if( m_PDBuf==NULL)
    {
        m_PDBufSz = sz;
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }

    if( m_PDBufSz!=sz)
    {
        AAA_LOGD("m_PDBufSz!=0xA8*0x800-16\n");
        if( m_PDBuf)
            delete m_PDBuf;
        m_PDBuf=NULL;

        m_PDBufSz = sz;
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }

    //convert format from DMA buffer format(Raw10) to pixel format
    MUINT16 *ptrbuf = new MUINT16 [0xA8*0x800];
    MUINT32 i,j,k;
    for( j=0, k=0; j<0x800; j++)
    {
        for( i=0; i<0xD2; i+=5)
        {
            //*
            ptrbuf[k  ]    = ( ((ptrBufAddr[ j*0xD4 + (i+1)]&0x3 ) <<8) &0x300) | ((ptrBufAddr[ j*0xD4 + (i  )]>>0)&0xFF);
            ptrbuf[k+1]    = ( ((ptrBufAddr[ j*0xD4 + (i+2)]&0xF ) <<6) &0x3C0) | ((ptrBufAddr[ j*0xD4 + (i+1)]>>2)&0x3F);
            ptrbuf[k+2]    = ( ((ptrBufAddr[ j*0xD4 + (i+3)]&0x3F) <<4) &0x3F0) | ((ptrBufAddr[ j*0xD4 + (i+2)]>>4)&0xF );
            ptrbuf[k+3]    = ( ((ptrBufAddr[ j*0xD4 + (i+4)]&0xFF) <<2) &0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+3)]>>6)&0x3 );
            //*/
            /*
            ptrbuf[k  ]    = ((ptrBufAddr[ j*0xD4 + (i  )] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>0) &0x3);
            ptrbuf[k+1]    = ((ptrBufAddr[ j*0xD4 + (i+1)] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>2) &0x3);
            ptrbuf[k+2]    = ((ptrBufAddr[ j*0xD4 + (i+2)] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>4) &0x3);
            ptrbuf[k+3]    = ((ptrBufAddr[ j*0xD4 + (i+3)] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>6) &0x3);
            //*/
            k+=4;
        }
    }
    AAA_LOGD("total %d pixels, sz %d\n", k, sz);



    //convert format to PD core algorithm input
    MUINT16 **ptr=NULL;
    MUINT16 *ptrL =   m_PDBuf;
    MUINT16 *ptrR = &(m_PDBuf[(h/2)*w]);

    for ( i=0; i < h; i++ )
    {
        if(i%4==0 || i%4==3)
            ptr = &ptrR;
        else
            ptr = &ptrL;


        for ( j=0; j < w; j++ )
        {
            (*ptr)[j] = ptrbuf[i*w+j];
        }
        (*ptr) += w;
    }

    delete []ptrbuf;
    ptrbuf=NULL;

    return m_PDBuf;

}
