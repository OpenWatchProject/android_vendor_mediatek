#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_ov23850mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#define LOG_TAG "pd_buf_mgr_ov23850mipiraw"

PDBufMgr*
PD_OV23850MIPIRAW::getInstance()
{
    static PD_OV23850MIPIRAW singleton;
    return &singleton;

}


PD_OV23850MIPIRAW::PD_OV23850MIPIRAW()
{
    MY_LOG("[PD Mgr] OV23850\n");
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
        MY_LOG("PDAF Mode is not Supported (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    return ret;

}

MINT32 PD_OV23850MIPIRAW::GetPDCalSz()
{
    return 0x55c;
}


MUINT16* PD_OV23850MIPIRAW::ConvertPDBufFormat( MUINT32  i4Size, MUINT8 *ptrBufAddr, MUINT32 i4FrmCnt)
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
        MY_LOG("m_PDBufSz!=0xA8*0x800-16\n");
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
            ptrbuf[k  ]	= ( ((ptrBufAddr[ j*0xD4 + (i+1)]&0x3 ) <<8) &0x300) | ((ptrBufAddr[ j*0xD4 + (i  )]>>0)&0xFF);
            ptrbuf[k+1]	= ( ((ptrBufAddr[ j*0xD4 + (i+2)]&0xF ) <<6) &0x3C0) | ((ptrBufAddr[ j*0xD4 + (i+1)]>>2)&0x3F);
            ptrbuf[k+2]	= ( ((ptrBufAddr[ j*0xD4 + (i+3)]&0x3F) <<4) &0x3F0) | ((ptrBufAddr[ j*0xD4 + (i+2)]>>4)&0xF );
            ptrbuf[k+3]	= ( ((ptrBufAddr[ j*0xD4 + (i+4)]&0xFF) <<2) &0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+3)]>>6)&0x3 );
            //*/
            /*
            ptrbuf[k  ]	= ((ptrBufAddr[ j*0xD4 + (i  )] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>0) &0x3);
            ptrbuf[k+1]	= ((ptrBufAddr[ j*0xD4 + (i+1)] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>2) &0x3);
            ptrbuf[k+2]	= ((ptrBufAddr[ j*0xD4 + (i+2)] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>4) &0x3);
            ptrbuf[k+3]	= ((ptrBufAddr[ j*0xD4 + (i+3)] << 2)&0x3FC) | ((ptrBufAddr[ j*0xD4 + (i+4)]>>6) &0x3);
            //*/
            k+=4;
        }
    }
    MY_LOG("total %d pixels, sz %d\n", k, sz);



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


        for ( int j=0; j < w; j++ )
        {
            (*ptr)[j] = ptrbuf[i*w+j];
        }
        (*ptr) += w;
    }





    char value[200] = {'\0'};
    property_get("vendor.vc.dump.enable", value, "0");
    MBOOL bEnable = atoi(value);
    if (bEnable)
    {
        char fileName[64];
        sprintf(fileName, "/sdcard/vc/%d_pd.raw", i4FrmCnt);
        FILE *fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            delete[] ptrbuf;
            return MFALSE;
        }
        fwrite(reinterpret_cast<void *>(m_PDBuf), 1, sz*2, fp);
        fclose(fp);


        sprintf(fileName, "/sdcard/vc/%d_in.raw", i4FrmCnt);
        fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            delete[] ptrbuf;
            return MFALSE;
        }
        fwrite(reinterpret_cast<void *>(ptrBufAddr), 1, i4Size, fp);
        fclose(fp);


        sprintf(fileName, "/sdcard/vc/%d_convert.raw", i4FrmCnt);
        fp = fopen(fileName, "w");
        if (NULL == fp)
        {
            delete[] ptrbuf;
            return MFALSE;
        }
        fwrite(reinterpret_cast<void *>(ptrbuf), 1, 0xA8*0x800*2, fp);
        fclose(fp);
    }



    delete[] ptrbuf;
    ptrbuf=NULL;

    return m_PDBuf;

}
