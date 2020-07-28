#include <log/log.h>
#include <fcntl.h>
#include <math.h>

#include <pd_s5k2x8mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_s5k2x8mipiraw"

PDBufMgr*
PD_S5K2X8MIPIRAW::getInstance()
{
    static PD_S5K2X8MIPIRAW singleton;
    return &singleton;
}

PD_S5K2X8MIPIRAW::PD_S5K2X8MIPIRAW()
{
    AAA_LOGD("S5K2X8");
    m_PDBufSz = 0;
    m_PDBuf   = NULL;
    m_PDXSz   = 0;
    m_PDYSz   = 0;
}

PD_S5K2X8MIPIRAW::~PD_S5K2X8MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf   = NULL;
    m_PDXSz   = 0;
    m_PDYSz   = 0;
}

MBOOL PD_S5K2X8MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //all-pixel mode is supported.
    if( iPdProfile.u4IsZSD!=0)
    {
        ret = MTRUE;
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d)", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }

    return ret;
}


void PD_S5K2X8MIPIRAW::seprate( unsigned int stride, unsigned char *ptr, int pd_x_num, int pd_y_num, unsigned short *ptrLROut, int iShift)
{
    unsigned int table[4*8]= {                   0,  stride*1/2+1,  stride*1/2+2,             1,
                                                 stride*4/2,  stride*5/2+0,  stride*5/2+1,  stride*4/2+3,
                                                 stride*7/2,  stride*6/2+0,  stride*6/2+1,  stride*7/2+3,
                                                 stride*11/2, stride*10/2+1, stride*10/2+2, stride*11/2+1,

                                                 stride*1/2,  stride*2/2+0,  stride*2/2+1,  stride*1/2+3,
                                                 stride*3/2,  stride*4/2+1,  stride*4/2+2,  stride*3/2+1,
                                                 stride*8/2,  stride*7/2+1,  stride*7/2+2,  stride*8/2+1,
                                                 stride*10/2,  stride*9/2+0,  stride*9/2+1,  stride*10/2+3
                             };

    unsigned int multiple[4*8]= { 2, 4, 4, 2,
                                  4, 2, 2, 4,
                                  4, 2, 2, 4,
                                  2, 4, 4, 2,
                                  4, 2, 2, 4,
                                  2, 4, 4, 2,
                                  2, 4, 4, 2,
                                  4, 2, 2, 4
                                };


    unsigned short *tempMap = (unsigned short *)ptr;
    unsigned short *ConvBuf1_tmpMap = ptrLROut;
    unsigned short *pout5 = ConvBuf1_tmpMap;
    unsigned short *pout6 = ConvBuf1_tmpMap + pd_x_num*1;
    unsigned short *pout7 = ConvBuf1_tmpMap + pd_x_num*2;
    unsigned short *pout8 = ConvBuf1_tmpMap + pd_x_num*3;

    unsigned short *pout1 = ConvBuf1_tmpMap + pd_x_num*(pd_y_num/2);
    unsigned short *pout2 = ConvBuf1_tmpMap + pd_x_num*((pd_y_num/2)+1);
    unsigned short *pout3 = ConvBuf1_tmpMap + pd_x_num*((pd_y_num/2)+2);
    unsigned short *pout4 = ConvBuf1_tmpMap + pd_x_num*((pd_y_num/2)+3);

    int count=0, idx=0;

    for( int i=0; i<pd_y_num/8; i++)
    {
        for( int j=0; j<pd_x_num; j+=4)
        {
            unsigned int *ptable1 = table;
            unsigned int *ptable2 = ptable1+4;
            unsigned int *ptable3 = ptable2+4;
            unsigned int *ptable4 = ptable3+4;
            unsigned int *ptable5 = ptable4+4;
            unsigned int *ptable6 = ptable5+4;
            unsigned int *ptable7 = ptable6+4;
            unsigned int *ptable8 = ptable7+4;

            unsigned int *pmultiple1 = multiple;
            unsigned int *pmultiple2 = pmultiple1+4;
            unsigned int *pmultiple3 = pmultiple2+4;
            unsigned int *pmultiple4 = pmultiple3+4;
            unsigned int *pmultiple5 = pmultiple4+4;
            unsigned int *pmultiple6 = pmultiple5+4;
            unsigned int *pmultiple7 = pmultiple6+4;
            unsigned int *pmultiple8 = pmultiple7+4;

            for( int k=0; k<4; k++)
            {
                *pout1 = tempMap[(*ptable1)+((*pmultiple1)*count)+idx]>>iShift;
                *pout2 = tempMap[(*ptable2)+((*pmultiple2)*count)+idx]>>iShift;
                *pout3 = tempMap[(*ptable3)+((*pmultiple3)*count)+idx]>>iShift;
                *pout4 = tempMap[(*ptable4)+((*pmultiple4)*count)+idx]>>iShift;
                *pout5 = tempMap[(*ptable5)+((*pmultiple5)*count)+idx]>>iShift;
                *pout6 = tempMap[(*ptable6)+((*pmultiple6)*count)+idx]>>iShift;
                *pout7 = tempMap[(*ptable7)+((*pmultiple7)*count)+idx]>>iShift;
                *pout8 = tempMap[(*ptable8)+((*pmultiple8)*count)+idx]>>iShift;
                ptable1++;
                ptable2++;
                ptable3++;
                ptable4++;
                ptable5++;
                ptable6++;
                ptable7++;
                ptable8++;
                pmultiple1++;
                pmultiple2++;
                pmultiple3++;
                pmultiple4++;
                pmultiple5++;
                pmultiple6++;
                pmultiple7++;
                pmultiple8++;
                pout1++;
                pout2++;
                pout3++;
                pout4++;
                pout5++;
                pout6++;
                pout7++;
                pout8++;
            }
            count++;
        }
        pout1 += 3*pd_x_num;
        pout2 += 3*pd_x_num;
        pout3 += 3*pd_x_num;
        pout4 += 3*pd_x_num;
        pout5 += 3*pd_x_num;
        pout6 += 3*pd_x_num;
        pout7 += 3*pd_x_num;
        pout8 += 3*pd_x_num;
        count  = 0;
        idx   += stride/2*12;
    }
}


MUINT16* PD_S5K2X8MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL b14BitData)
{
    //s5k2x8 is EPDBuf_Raw type, no need convert PD buffer format.

    //first in allocate local PD buffer directly.
    if( m_PDBuf==NULL)
    {
        //vaild pd data size
        m_PDXSz   = (m_bpciInfo.pdo_xsize+1)/2;
        m_PDYSz   = (m_bpciInfo.pdo_ysize+1)*2/3;
        m_PDBufSz = m_PDXSz*m_PDYSz;
        m_PDBuf = new MUINT16 [m_PDBufSz];
    }

    seprate( i4Stride, ptrBufAddr, m_PDXSz, m_PDYSz, m_PDBuf, (b14BitData ? 4 : 2));

    return m_PDBuf;
}


MBOOL PD_S5K2X8MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDXSz;
    PDYsz = m_PDYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);
}

