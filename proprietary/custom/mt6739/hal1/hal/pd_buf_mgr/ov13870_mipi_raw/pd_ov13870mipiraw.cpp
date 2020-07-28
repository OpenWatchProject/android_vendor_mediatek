#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_ov13870mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_ov13870mipiraw"

PDBufMgr*
PD_OV13870MIPIRAW::getInstance()
{
    static PD_OV13870MIPIRAW singleton;
    return &singleton;

}

PD_OV13870MIPIRAW::PD_OV13870MIPIRAW()
{
    AAA_LOGD("[PD Mgr] OV13870\n");
    m_PDBufSz      = 0;
    m_PDBuf        = NULL;
    m_bDebugEnable = MFALSE;
}

PD_OV13870MIPIRAW::~PD_OV13870MIPIRAW()
{
    if( m_PDBuf)
    {
        delete m_PDBuf;
        m_PDBuf = NULL;
    }

    m_PDBufSz = 0;
}


MBOOL PD_OV13870MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    // enable/disable debug log
    m_bDebugEnable = property_get_int32("vendor.debug.af_mgr.enable", 0);

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

MBOOL PD_OV13870MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = 0x100;
    PDYsz = 0x300;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

MUINT16* PD_OV13870MIPIRAW::ConvertPDBufFormat( MUINT32  i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL /*b14BitData*/)
{
    const MUINT32  _w = 0x100;
    const MUINT32  _h = 0x300;
    const MUINT32 _sz = _w * _h;

    MUINT32 _xsz = _w*10/8;

    //align 4
    if( (_xsz&0x03)!=0)
    {
        _xsz = _xsz+(4-(0x03&_xsz));
    }


    const MUINT32 _stride = _xsz;

    AAA_LOGD_IF( m_bDebugEnable, "buf : sz(%d_%d), stride(%d_%d), va(%p)", i4Size, _stride*_h, i4Stride, _stride, ptrBufAddr);

    //input i4Size is DMA size.
    if( m_PDBuf==NULL)
    {
        m_PDBufSz = _sz;
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }

    if( m_PDBufSz!=_sz)
    {
        AAA_LOGD("m_PDBufSz!=0x100*0x300");
        if( m_PDBuf)
            delete m_PDBuf;
        m_PDBuf=NULL;

        m_PDBufSz = _sz;
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }



    MUINT16 **ptr = NULL;

    MUINT16 *ptrL =  m_PDBuf;
    MUINT16 *ptrR = &m_PDBuf[(_h/2)*_w];

    for( MUINT32 j=0; j<_h; j++)
    {
        if(j%4==1 || j%4==2)
            ptr = &ptrL;
        else
            ptr = &ptrR;

        for( MUINT32 i=0; i<(_w*10/8); i+=5)
        {
            MUINT32 idx = j*_stride;
            MUINT8 val0 = ptrBufAddr[ idx + (i  )];
            MUINT8 val1 = ptrBufAddr[ idx + (i+1)];
            MUINT8 val2 = ptrBufAddr[ idx + (i+2)];
            MUINT8 val3 = ptrBufAddr[ idx + (i+3)];
            MUINT8 val4 = ptrBufAddr[ idx + (i+4)];

            *(*ptr)++   = ( ((val1&0x3 ) <<8) &0x300) | ((val0>>0)&0xFF);
            *(*ptr)++   = ( ((val2&0xF ) <<6) &0x3C0) | ((val1>>2)&0x3F);
            *(*ptr)++   = ( ((val3&0x3F) <<4) &0x3F0) | ((val2>>4)&0xF );
            *(*ptr)++   = ( ((val4&0xFF) <<2) &0x3FC) | ((val3>>6)&0x3 );
        }
    }



    return m_PDBuf;

}
