#include <utils/Log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>

#include <pd_s5k3p8sxmipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>
//#include "camera_bpci_tbl_s5k3p8sxmipiraw.h"


#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "pd_buf_mgr_s5k3p8sxmipiraw"
#define BINING_UIMGYSZ  1744

PDBufMgr*
PD_S5K3P8SXMIPIRAW::getInstance()
{
    static PD_S5K3P8SXMIPIRAW singleton;
    return &singleton;

}


PD_S5K3P8SXMIPIRAW::PD_S5K3P8SXMIPIRAW() :
    m_PDBufXSz(0),
    m_PDBufYSz(0),
    m_PDBufSz(0),
    m_PDBuf(nullptr)
{
    AAA_LOGD("[PD Mgr] S5K3P8SX\n");
    m_bDebugEnable = MFALSE;
}

PD_S5K3P8SXMIPIRAW::~PD_S5K3P8SXMIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBufSz = 0;
    m_PDBuf = NULL;
}


MBOOL PD_S5K3P8SXMIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    //all-pixel mode is supported. + 4640*2680 (16:9) supported + binning mode(4:3)
    if ( iPdProfile.u4IsZSD !=0 || iPdProfile.uImgYsz == BINING_UIMGYSZ)
    {
        if( m_PDBuf)
        {
            delete m_PDBuf;
            m_PDBufSz = 0;
            m_PDBuf = NULL;
        }

        m_PDBufXSz = 288;
        m_PDBufYSz = 432;
        m_PDBufSz = m_PDBufXSz * m_PDBufYSz;
        m_PDBuf = new MUINT16 [m_PDBufSz];

        ret = MTRUE;
        AAA_LOGD("PDAF Mode is Supported with (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    else if (iPdProfile.uImgYsz==2608)
    {
        if( m_PDBuf)
        {
            delete m_PDBuf;
            m_PDBufSz = 0;
            m_PDBuf = NULL;
        }

        m_PDBufXSz = 288;
        m_PDBufYSz = 320;
        m_PDBufSz = m_PDBufXSz * m_PDBufYSz;
        m_PDBuf = new MUINT16 [m_PDBufSz];

        ret = MTRUE;
        AAA_LOGD("PDAF Mode is Supported with (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    else
    {
        AAA_LOGD("PDAF Mode is not Supported (%d, %d)\n", iPdProfile.uImgXsz, iPdProfile.uImgYsz);
    }
    return ret;

}

MBOOL PD_S5K3P8SXMIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    return (m_PDBufSz ? MTRUE : MFALSE);

}

MUINT16* PD_S5K3P8SXMIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MUINT32 /*u4BitDepth*/, PD_AREA_T* /*ptrPDRegion*/)
{
    MUINT8 *ptr = ptrBufAddr;
    int stride = i4Stride; //180 bytes

    AAA_LOGD("m_PDBufSz(%d) = m_PDBufXSz(%d)*m_PDBufXSz(%d),stride(%d)\n", m_PDBufSz, m_PDBufXSz, m_PDBufYSz,stride);

    MUINT32 x = 0, y = 0;
    while (y < m_PDBufYSz)
    {
        unsigned short row[16][144]; //144 pixel/line

        for (int j=0; j<16; j++)
        {
            x = 0;
            for (int bytes = 0; bytes < 180; bytes+=5)  	//total 180byte pd data
            {
                unsigned char chRead[5]; //5bytes, 40bits

                for (int i=0; i<5; i++)
                    chRead[i] = *ptr++;

                for (int i=0; i<4; i++)
                {
                    if (x < m_PDBufXSz/2)
                    {
                        row[j][x++] = (chRead[i] << 2) | ((chRead[4]>>(2*i)) & 0x3);
                    }
                    //	AAA_LOGD("j(%d),bytes(%d),i(%d),x(%d)\n", j, bytes, i,x);
                }
                //AAA_LOGD("j(%d),bytes(%d),x(%d)\n", j, bytes, x);
            }
            ptr += (stride-180);	//skip padding
        }

        //reconstruct RL from row
        // R L R L
        // L R L R
        // R R L L
        // L L R R
        unsigned short* pR[4];
        unsigned short* pL[4];
        for (int i=0; i<4; i++)
        {
            pL[i] = m_PDBuf + m_PDBufXSz*(y/2 + i);
            pR[i] = pL[i] + m_PDBufXSz*m_PDBufYSz/2;
            //AAA_LOGD("pR[i](%x),pL[i](%x)\n", pR[i], pL[i]);
        }

        int idx = 0;
        for (x=0; x<m_PDBufXSz/4; x++)
        {
            *(pR[0]++) = row[0][idx];
            *(pR[0]++) = row[2][idx];
            *(pR[0]++) = row[2][idx+1];
            *(pR[0]++) = row[0][idx+1];
            *(pR[1]++) = row[5][idx];
            *(pR[1]++) = row[7][idx];
            *(pR[1]++) = row[7][idx+1];
            *(pR[1]++) = row[5][idx+1];
            *(pR[2]++) = row[9][idx];
            *(pR[2]++) = row[8][idx];
            *(pR[2]++) = row[8][idx+1];
            *(pR[2]++) = row[9][idx+1];
            *(pR[3]++) =row[15][idx];
            *(pR[3]++) =row[14][idx];
            *(pR[3]++) =row[14][idx+1];
            *(pR[3]++) =row[15][idx+1];

            *(pL[0]++) = row[1][idx];
            *(pL[0]++) = row[3][idx];
            *(pL[0]++) = row[3][idx+1];
            *(pL[0]++) = row[1][idx+1];
            *(pL[1]++) = row[4][idx];
            *(pL[1]++) = row[6][idx];
            *(pL[1]++) = row[6][idx+1];
            *(pL[1]++) = row[4][idx+1];
            *(pL[2]++) =row[11][idx];
            *(pL[2]++) =row[10][idx];
            *(pL[2]++) =row[10][idx+1];
            *(pL[2]++) =row[11][idx+1];
            *(pL[3]++) =row[13][idx];
            *(pL[3]++) =row[12][idx];
            *(pL[3]++) =row[12][idx+1];
            *(pL[3]++) =row[13][idx+1];

            idx+=2;
            //AAA_LOGD("idx(%d)\n", idx);
        }

        y += 8;
        //AAA_LOGD("y(%d)\n", y);
    }

    return m_PDBuf;
}

