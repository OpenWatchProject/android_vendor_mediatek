#include <log/log.h>
#include <fcntl.h>
#include <math.h>
#include <memory.h>

#include <pd_s5k3m3mipiraw.h>
#include <aaa_log.h>
#include <cutils/properties.h>
#include <stdlib.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr_s5k3m3mipiraw"


/*
Currently, sensor driver  outputs Byte2 data format.
For turnkey solution, set 260x384 PD data to PD algorithm for both binning type and hdr type S5K3M3.
*/

PDBufMgr*
PD_S5K3M3MIPIRAW::getInstance()
{
    static PD_S5K3M3MIPIRAW singleton;
    return &singleton;
}


PD_S5K3M3MIPIRAW::PD_S5K3M3MIPIRAW()
{
    AAA_LOGD("[PD Mgr] S5K3M3\n");
    m_PDBufXSz = 252;
    m_PDBufYSz = 384;
    m_PDBufSz = m_PDBufXSz * m_PDBufYSz;
    m_PDBuf = new MUINT16 [m_PDBufSz];
    m_bDebugEnable = MFALSE;
}

PD_S5K3M3MIPIRAW::~PD_S5K3M3MIPIRAW()
{
    if( m_PDBuf)
        delete m_PDBuf;

    m_PDBuf    = NULL;
}


MBOOL PD_S5K3M3MIPIRAW::IsSupport( SPDProfile_t &iPdProfile)
{
    MBOOL ret = MFALSE;

    //enable/disable debug log
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("vendor.debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

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

MBOOL PD_S5K3M3MIPIRAW::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = m_PDBufXSz;
    PDYsz = m_PDBufYSz;
    PDBufSz = m_PDBufSz;

    return MTRUE;
}

MUINT16* PD_S5K3M3MIPIRAW::ConvertPDBufFormat( MUINT32 /*i4Size*/, MUINT32 i4Stride, MUINT8 *ptrBufAddr, MBOOL /*b14BitData*/)
{
    MUINT8 *ptr = ptrBufAddr;
    int stride = i4Stride; //160 bytes

    MUINT32 x = 0, y = 0;
    while (y < m_PDBufYSz) {
        unsigned short row[16][126]; //at most m_PDBufXSz/2

        for (int j=0; j<16; j++) {
            x = 0;
            for (int bytes = 0; bytes < stride; bytes+=5) {
                unsigned int chRead[5]; //5bytes, 40bits

                for (int i=0; i<5; i++)
	                chRead[i] = *ptr++;

                for (int i=0; i<4; i++) {
                    if (x < m_PDBufXSz/2) {
                        row[j][x++] = (chRead[i] << 2) | ((chRead[4]>>(2*i)) & 0x3);
                    }
                }
            }
        }

        //reconstruct RL from row
        // R L R L
        // L R L R
        // R R L L
        // L L R R
        unsigned short* pR[4];
        unsigned short* pL[4];
        for (int i=0; i<4; i++) {
            pR[i] = m_PDBuf + m_PDBufXSz*(y/2 + i);
            pL[i] = pR[i] + m_PDBufXSz*m_PDBufYSz/2;
        }
        int idx = 0;
        for (x=0; x<m_PDBufXSz/4; x++) {
            *(pR[0]++) = row[0][idx];    *(pR[0]++) = row[2][idx];    *(pR[0]++) = row[2][idx+1];    *(pR[0]++) = row[0][idx+1];
            *(pR[1]++) = row[5][idx];    *(pR[1]++) = row[7][idx];    *(pR[1]++) = row[7][idx+1];    *(pR[1]++) = row[5][idx+1];
            *(pR[2]++) = row[9][idx];    *(pR[2]++) = row[8][idx];    *(pR[2]++) = row[8][idx+1];    *(pR[2]++) = row[9][idx+1];
            *(pR[3]++) =row[15][idx];    *(pR[3]++) =row[14][idx];    *(pR[3]++) =row[14][idx+1];    *(pR[3]++) =row[15][idx+1];

            *(pL[0]++) = row[1][idx];    *(pL[0]++) = row[3][idx];    *(pL[0]++) = row[3][idx+1];    *(pL[0]++) = row[1][idx+1];
            *(pL[1]++) = row[4][idx];    *(pL[1]++) = row[6][idx];    *(pL[1]++) = row[6][idx+1];    *(pL[1]++) = row[4][idx+1];
            *(pL[2]++) =row[11][idx];    *(pL[2]++) =row[10][idx];    *(pL[2]++) =row[10][idx+1];    *(pL[2]++) =row[11][idx+1];
            *(pL[3]++) =row[13][idx];    *(pL[3]++) =row[12][idx];    *(pL[3]++) =row[12][idx+1];    *(pL[3]++) =row[13][idx+1];

            idx+=2;
        }

        y += 8;
    }

    return m_PDBuf;
}

