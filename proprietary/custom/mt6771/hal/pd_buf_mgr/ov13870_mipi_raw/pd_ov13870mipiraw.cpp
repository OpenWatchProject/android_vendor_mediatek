#include <utils/Log.h>
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

MUINT16* PD_OV13870MIPIRAW::ConvertPDBufFormat( MUINT32 i4Size, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 /*u4BitDepth*/, PD_AREA_T* ptrPDRegion)
{
    const MUINT32  _w = 0x100;
    const MUINT32  _h = 0x300;
    const MUINT32 _sz = _w * _h;

    MUINT32 _xsz = _w*10/8;

#if 0
    //align 4
    if( (_xsz&0x03)!=0)
    {
        _xsz = _xsz+(4-(0x03&_xsz));
    }
#endif

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

    if(ptrPDRegion)
    {

        MUINT16 **ptr = NULL;

        MUINT16 *cvtbuf = m_PDBuf;

        MUINT32 blk_offy = (ptrPDRegion->i4Y - m_PDBlockInfo.i4OffsetY)/m_PDBlockInfo.i4PitchY;
        MUINT32 blk_numy = (ptrPDRegion->i4H+m_PDBlockInfo.i4PitchY)/m_PDBlockInfo.i4PitchY;

        MUINT16 *ptrL = &(cvtbuf[(0+blk_offy*4)*_w]);
        MUINT16 *ptrR = &(cvtbuf[(_h/2+blk_offy*4)*_w]);

        MUINT8 *srcbuf = ptrBufAddr + (blk_offy*8)*(_stride);

        for( MUINT32 j=0; j<blk_numy*8; j++)
        {
            if(j%4==1 || j%4==2)
                ptr = &ptrL;
            else
                ptr = &ptrR;

            for( MUINT32 i=0; i<(_w*10/8); i+=5)
            {
                MUINT32 idx = j*_stride;
                MUINT8 val0 = srcbuf[ idx + (i  )];
                MUINT8 val1 = srcbuf[ idx + (i+1)];
                MUINT8 val2 = srcbuf[ idx + (i+2)];
                MUINT8 val3 = srcbuf[ idx + (i+3)];
                MUINT8 val4 = srcbuf[ idx + (i+4)];

                *(*ptr)++   = ( ((val1&0x3 ) <<8) &0x300) | ((val0>>0)&0xFF);
                *(*ptr)++   = ( ((val2&0xF ) <<6) &0x3C0) | ((val1>>2)&0x3F);
                *(*ptr)++   = ( ((val3&0x3F) <<4) &0x3F0) | ((val2>>4)&0xF );
                *(*ptr)++   = ( ((val4&0xFF) <<2) &0x3FC) | ((val3>>6)&0x3 );
            }
        }

    }
    else
    {
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

    }



    return m_PDBuf;

}
