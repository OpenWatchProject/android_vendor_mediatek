#include <log/log.h>
#include <utils/Errors.h>
#include <math.h>
#include "kd_imgsensor.h"
#include <cutils/properties.h>
#include <stdlib.h>
#include <string.h>
#include <aaa_log.h>

#include <pd_buf_mgr.h>
#include <pd_ov23850mipiraw.h>
#include <pd_ov13870mipiraw.h>
#include <pd_s5k2p8mipiraw.h>
#include <pd_imx258mipiraw.h>
#include <pd_s5k3m2mipiraw.h>
#include <pd_s5k3m3mipiraw.h>
#include <pd_s5k2x8mipiraw.h>
#include <pd_s5k2l7mipiraw.h>
#include <pd_ov16880mipiraw.h>
#include <pd_imx362mipiraw.h>
#include <pd_imx398mipiraw.h>


#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr"

PDBufMgr::PDBufMgr()
{
    memset( &m_PDBlockInfo, 0, sizeof(SET_PD_BLOCK_INFO_T)         );
    memset( &m_bpciInfo,    0, sizeof(PDBuf_Bpci_Info_t)           );
    subBlkX = 0;
    subBlkY = 0;
    rowNum = 0;
    memset( pPdMap,         0, sizeof(PDMAP_T  ) * MAX_PAIR_NUM * 2);
    memset( pdPixels,       0, sizeof(PDPIXEL_T) * MAX_PAIR_NUM * 2);
}

PDBufMgr::~PDBufMgr()
{}


PDBufMgr*
PDBufMgr::createInstance(SPDProfile_t &iPdProfile)
{
    PDBufMgr *instance = NULL;
    PDBufMgr *ret      = NULL;

    switch( iPdProfile.i4CurrSensorId)
    {
#if defined(IMX362_MIPI_RAW)
    case IMX362_SENSOR_ID :
        instance = PD_IMX362MIPIRAW::getInstance();
        break;
#endif
#if defined(OV23850_MIPI_RAW)
    case OV23850_SENSOR_ID :
        instance = PD_OV23850MIPIRAW::getInstance();
        break;
#endif
#if defined(OV13870_MIPI_RAW)
    case OV13870_SENSOR_ID :
        instance = PD_OV13870MIPIRAW::getInstance();
        break;
#endif
#if defined(OV16880_MIPI_RAW)
    case OV16880_SENSOR_ID :
        instance = PD_OV16880MIPIRAW::getInstance();
        break;
#endif
#if defined(S5K2P8_MIPI_RAW)
    case S5K2P8_SENSOR_ID :
        instance = PD_S5K2P8MIPIRAW::getInstance();
        break;
#endif
#if defined(IMX258_MIPI_RAW)
    case IMX258_SENSOR_ID :
        instance = PD_IMX258MIPIRAW::getInstance();
        break;
#endif
#if defined(S5K3M2_MIPI_RAW)
    case S5K3M2_SENSOR_ID :
        instance = PD_S5K3M2MIPIRAW::getInstance();
        break;
#endif
#if defined(S5K2X8_MIPI_RAW)
    case S5K2X8_SENSOR_ID :
        instance = PD_S5K2X8MIPIRAW::getInstance();
        break;
#endif
#if defined(S5K2L7_MIPI_RAW)
    case S5K2L7_SENSOR_ID :
        instance = PD_S5K2L7MIPIRAW::getInstance();
        break;
#endif
#if defined(IMX398_MIPI_RAW)
    case IMX398_SENSOR_ID :
        instance = PD_IMX398MIPIRAW::getInstance();
        break;
#endif
#if defined(S5K3M3_MIPI_RAW)
    case S5K3M3_SENSOR_ID :
        instance = PD_S5K3M3MIPIRAW::getInstance();
        break;
#endif


    default :
        instance = NULL;
        break;
    }

    if( instance)
        ret = instance->IsSupport(iPdProfile) ? instance : NULL;

    AAA_LOGD( "[PD] [SensorId]0x%04x, [%p]",
              iPdProfile.i4CurrSensorId,
              instance);

    return ret;
}

MVOID PDBufMgr::SetBpciInfo(UINT32 pdo_xsize, UINT32 pdo_ysize)
{
    m_bpciInfo.pdo_xsize = pdo_xsize;
    m_bpciInfo.pdo_ysize = pdo_ysize;
}

MBOOL PDBufMgr::SetPDBlockInfo( SET_PD_BLOCK_INFO_T &iPDBlockInfo)
{
    memcpy( &m_PDBlockInfo, &iPDBlockInfo, sizeof(SET_PD_BLOCK_INFO_T));

    AAA_LOGD( "[PD] set block information : OffsetX(%d) OffsetY(%d) PitchX(%d) PitchY(%d) SubBlkW(%d) SubBlkH(%d) PairNum(%d)",
              m_PDBlockInfo.i4OffsetX,
              m_PDBlockInfo.i4OffsetY,
              m_PDBlockInfo.i4PitchX,
              m_PDBlockInfo.i4PitchY,
              m_PDBlockInfo.i4SubBlkW,
              m_PDBlockInfo.i4SubBlkH,
              m_PDBlockInfo.i4PairNum);

    return MTRUE;
}

MBOOL PDBufMgr::GetLRBufferInfo(MUINT32 &PDXsz, MUINT32 &PDYsz, MUINT32 &PDBufSz)
{
    PDXsz = 0;
    PDYsz = 0;
    PDBufSz = 0;
    return MFALSE;
}

MBOOL PDBufMgr::GetDualPDVCInfo( MINT32 /*i4CurSensorMode*/, SDUALPDVCINFO_T &oDualPDVChwInfo, MINT32 /*i4AETargetMode*/)
{
    oDualPDVChwInfo.u4VCBinningX = 0;
    oDualPDVChwInfo.u4VCBinningY = 0;
    oDualPDVChwInfo.u4VCBufFmt   = 0;
    return MFALSE;
}


//for pdo general separate method
MBOOL PDBufMgr::pdoPixelPos(MUINT32 pPdoPixPos[][2], int pdNum)
{
    if (pdNum > MAX_PAIR_NUM * 2)
    {
        AAA_LOGE("pdo pixel num > MAX_PD_PAIR_NUM * 2, could lead memory access violation!!!");
        return MFALSE;
    }
    if (pdNum == 0)
    {
        AAA_LOGE("pdNum = 0!!!");
        return MFALSE;
    }
    if (pdNum % 2)
    {
        AAA_LOGE("pdNum is not even!!!");
        return MFALSE;
    }

    for (int i=0; i<pdNum; i++)
    {
        pdPixels[i].cx = pPdoPixPos[i][0];
        pdPixels[i].cy = pPdoPixPos[i][1];
        pdPixels[i].bx = -1;
        pdPixels[i].by = -1;
        pdPixels[i].lr = (i<pdNum/2) ? R : L;
    }
    if (m_PDBlockInfo.i4SubBlkH == 0)
    {
        AAA_LOGE("must call SetPDBlockInfo() to get m_PDBlockInfo.i4SubBlkH before parsePdPixels()");
        return MFALSE;
    }
    parsePdPixels(pdPixels, pdNum, m_PDBlockInfo.i4SubBlkH);
    getPdMapping( pdPixels, pdNum);

    return MTRUE;
}


//for pdo general separate method
MVOID PDBufMgr::parsePdPixels(PDPIXEL_T* pPdPix, int pdNum, int subBlkH)
{
    //define block x y
    for( int bIdx=0; bIdx<=pdNum/2; bIdx+=pdNum/2)
    {
        int s = bIdx;
        //make the first pixel in the first block
        pPdPix[s].bx = pPdPix[s].by = 0;

        do
        {
            //decide .by by cy less than BlockDis
            for (int i=bIdx; i<(bIdx+pdNum/2); i++)
            {
                if (pPdPix[i].by == -1)   //for those .by not decided
                {
                    if (pPdPix[i].cy == pPdPix[s].cy ||
                            ((pPdPix[i].cy / subBlkH) == (pPdPix[s].cy / subBlkH))
                       )
                    {
                        pPdPix[i].by = pPdPix[s].by;
                    }
                }
            }
            //each the same .by, decide .bx
            for( int i=bIdx; i<(bIdx+pdNum/2); i++)
            {
                if (pPdPix[i].by == pPdPix[s].by &&
                        pPdPix[i].bx == -1)   //for those .by equals to s and .bx not decided
                {
                    int maxPreceding = -1;
                    int minSucceding = 32767;
                    //find preceding & succeding
                    for( int j=bIdx; j<(bIdx+pdNum/2); j++)
                    {
                        if (j != i &&
                                pPdPix[j].bx != -1 &&
                                pPdPix[j].by == pPdPix[s].by)
                        {
                            if( pPdPix[i].cx == pPdPix[j].cx)
                            {
                                AAA_LOGD("error! should have never been here!\n");
                                pPdPix[i].bx = pPdPix[j].bx;
                                break;
                            }
                            else if (
                                pPdPix[i].cx > pPdPix[j].cx &&
                                maxPreceding < pPdPix[j].cx)
                            {
                                maxPreceding = pPdPix[j].cx;
                            }
                            else if (
                                pPdPix[i].cx < pPdPix[j].cx &&
                                minSucceding > pPdPix[j].cx)
                            {
                                minSucceding = pPdPix[j].cx;
                            }
                        }
                    }
                    if( pPdPix[i].bx == -1)
                    {
                        if( maxPreceding == -1)  //no preceding found
                        {
                            pPdPix[i].bx = 0;
                            for( int k=bIdx; k<(bIdx+pdNum/2); k++)
                            {
                                if (k != i &&
                                        pPdPix[k].by == pPdPix[s].by &&
                                        pPdPix[k].bx != -1)
                                    pPdPix[k].bx ++;
                            }
                        }
                        else if( minSucceding == 32767)  //no succeding found
                        {
                            for (int j=bIdx; j<(bIdx+pdNum/2); j++)
                            {
                                if (j != i &&
                                        pPdPix[j].by == pPdPix[s].by &&
                                        pPdPix[j].cx == maxPreceding)
                                {
                                    pPdPix[i].bx = pPdPix[j].bx + 1;
                                    break;
                                }
                            }
                        }
                        else
                        {
                            for (int j=bIdx; j<(bIdx+pdNum/2); j++)
                            {
                                if (j != i &&
                                        pPdPix[j].by == pPdPix[s].by &&
                                        pPdPix[j].cx == maxPreceding)
                                {
                                    pPdPix[i].bx = pPdPix[j].bx + 1;
                                    break;
                                }
                            }
                            for( int j=bIdx; j<(bIdx+pdNum/2); j++)
                            {
                                if (j != i &&
                                        pPdPix[j].by == pPdPix[s].by &&
                                        pPdPix[j].cx == minSucceding &&
                                        pPdPix[i].bx == pPdPix[j].bx)
                                {
                                    for (int k=bIdx; k<(bIdx+pdNum/2); k++)
                                    {
                                        if (k != i &&
                                                pPdPix[k].bx != -1 &&
                                                pPdPix[k].by == pPdPix[s].by &&
                                                pPdPix[k].cx >= minSucceding)
                                            pPdPix[k].bx ++;
                                    }
                                    break;
                                }
                            }
                        }
                    }
                }
            }
            //find next s
            bool bFoundNext = false;
            for( int i=bIdx; i<(bIdx+pdNum/2); i++)
            {
                if( pPdPix[i].by == -1)
                {
                    pPdPix[i].bx = 0;
                    pPdPix[i].by = pPdPix[s].by + 1;
                    s = i;
                    bFoundNext = true;
                    break;
                }
            }
            if( !bFoundNext)
                break;
        }
        while(1);
    }
}

//for pdo general separate method
MVOID PDBufMgr::getPdMapping(PDPIXEL_T* pPdPix, int pdNum)
{
    int idx = 0;
    PDPIXEL_T* pPdOut = (PDPIXEL_T*)malloc(pdNum * sizeof(PDPIXEL_T));
    int cy[MAX_PDO_BUF_ROW_NUM];

    int tarY = 0;
    rowNum = 0;
    while( idx < pdNum)
    {
        int minY = 32767;
        //find min cy
        for( int i=0; i<pdNum; i++)
        {
            if( pPdPix[i].cy < minY && tarY < pPdPix[i].cy)
                minY = pPdPix[i].cy;
        }
        tarY = minY;
        if( rowNum < MAX_PDO_BUF_ROW_NUM)
            cy[rowNum++] = tarY;
        else
        {
            AAA_LOGD("Error! Exceed MAX_PDO_BUF_ROW_NUM!\n");
        }
        for( int i=0; i<pdNum; i++)
        {
            if (pPdPix[i].cy == tarY)
                memcpy((void*)&pPdOut[idx++], (void*)&pPdPix[i], sizeof(PDPIXEL_T));
        }
    }
    //sort by pdo buffer row sequence
    bool bSorting = true;
    while( bSorting)
    {
        bSorting = false;
        for( int i=0; i<pdNum-1; i++)
        {
            if (pPdOut[i].cy == pPdOut[i+1].cy &&
                    pPdOut[i].cx > pPdOut[i+1].cx)
            {
                //swap
                PDPIXEL_T temp;
                memcpy((void*)&temp, (void*)&pPdOut[i+1], sizeof(PDPIXEL_T));
                memcpy((void*)&pPdOut[i+1], (void*)&pPdOut[i], sizeof(PDPIXEL_T));
                memcpy((void*)&pPdOut[i], (void*)&temp, sizeof(PDPIXEL_T));
                bSorting = true;
            }
        }
    }

    //print sorted layout
    int minX = 32767;
    for( int i=0; i<pdNum; i++)
    {
        AAA_LOGD("(%02d, %02d), (%d, %d), %d\n",
                 pPdOut[i].cx,
                 pPdOut[i].cy,
                 pPdOut[i].bx,
                 pPdOut[i].by,
                 pPdOut[i].lr);
        if( pPdOut[i].cx < minX)
            minX = pPdOut[i].cx;
    }

    //sort by block postion (0,0)-->(1,0)-->(2,0) ...
    idx = 0;
    for( int bIdx=0; bIdx<=pdNum/2; bIdx+=pdNum/2)
    {
        int blkX = 0, blkY = 0;
        while( idx < (bIdx + pdNum/2))
        {
            bool bMatch = false;
            for( int i=bIdx; i<(bIdx+pdNum/2); i++)
            {
                if( pPdPix[i].bx == blkX && pPdPix[i].by == blkY)
                {
                    memcpy((void*)&pPdOut[idx++], (void*)&pPdPix[i], sizeof(PDPIXEL_T));
                    bMatch = true;
                    break;
                }
            }
            if( bMatch)
                blkX ++;
            else
            {
                blkX = 0;
                blkY ++;
            }
        }
    }

    //fill pPdMap
    for( int i=0; i<pdNum/2; i++)
    {
        pPdMap[i*2].cx = pPdOut[i].cx;
        pPdMap[i*2].cy = pPdOut[i].cy;
        pPdMap[i*2].lr = pPdOut[i].lr;
        pPdMap[i*2+1].cx = pPdOut[i+pdNum/2].cx;
        pPdMap[i*2+1].cy = pPdOut[i+pdNum/2].cy;
        pPdMap[i*2+1].lr = pPdOut[i+pdNum/2].lr;
    }

    subBlkX = 0;
    subBlkY = 0;
    for( int i=0; i<pdNum; i++)
    {
        if (pPdOut[i].bx > (int)subBlkX)
            subBlkX = pPdOut[i].bx;
        if (pPdOut[i].by > (int)subBlkY)
            subBlkY = pPdOut[i].by;

        for (unsigned int j=0; j<rowNum; j++)
        {
            if (pPdMap[i].cy == cy[j])
                pPdMap[i].row = j;
        }
        AAA_LOGD("(%02d, %02d), %02d, %d\n",
                 pPdMap[i].cx,
                 pPdMap[i].cy,
                 pPdMap[i].row,
                 pPdMap[i].lr);
    }
    subBlkX++;
    subBlkY++;

    AAA_LOGD("pd blocks(%d, %d), pdo buffer row num(%d)\n\n", subBlkX, subBlkY, rowNum);

    //optimize for symmetric pattern
    while (subBlkY > 1)   //optimize Y arrangement
    {
        bool bSym = true;
        int rowDiff = pPdMap[pdNum/2].row - pPdMap[0].row;
        if (rowDiff)
        {
            for (int i=0; i<pdNum/2; i++)
            {
                if (pPdMap[i+pdNum/2].row != pPdMap[i].row + rowDiff)
                {
                    bSym = false;
                    break;
                }
            }
        }
        else
            bSym = false;
        if (bSym)
        {
            subBlkY /= 2;
            rowNum  /= 2;
            pdNum   /= 2;
            AAA_LOGD("Y-dir symmetric optimization!\npd blocks(%d, %d), pdo buffer row num(%d)\n\n", subBlkX, subBlkY, rowNum);
        }
        else
            break;
    }

    while (subBlkX > 1)   //optimize X arrangement
    {
        bool bSym = true;
        int rowDiff = pPdMap[0].row - pPdMap[1].row;
        for (int i=0; i<pdNum; i+=2)
        {
            if (pPdMap[i].row != pPdMap[i+1].row + rowDiff)
            {
                bSym = false;
                break;
            }
        }
        if (bSym)
        {
            subBlkX /= 2;
            pdNum   /= 2;
            AAA_LOGD("X-dir symmetric optimization!\npd blocks(%d, %d), pdo buffer row num(%d)\n\n", subBlkX, subBlkY, rowNum);
        }
        else
            break;
    }

    free(pPdOut);
}

//for pdo general separate method
MVOID PDBufMgr::separateLR( unsigned int stride, unsigned char *ptr, unsigned int pd_x_num, unsigned int pd_y_num, unsigned short *outBuf, unsigned int iShift)
{
    if( rowNum == 0)
    {
        AAA_LOGE("separateByMapping() called before parsing!");
        return;
    }
    unsigned short *inBuf = (unsigned short*)ptr;
    unsigned short *ptr16 = inBuf;

    unsigned short *row[MAX_PDO_BUF_ROW_NUM];
    unsigned int LRoffset = pd_x_num*pd_y_num>>1;
    unsigned short *pL = outBuf;
    unsigned short *pR = pL + LRoffset;

    if (subBlkX == 1 && subBlkY == 1)
    {
        for (unsigned int y=0; y<pd_y_num; y+=2)
        {
            row[0] = ptr16;
            for (unsigned int i=1; i<rowNum; i++)
                row[i] = row[i-1] + (stride>>1);
            ptr16 = row[rowNum-1] + (stride>>1);

            if (rowNum == 1)
            {
                unsigned short *rowRL = row[0];
                for (unsigned int x=0; x<pd_x_num; x+=1)
                {
                    *(pR++) = *(rowRL++) >> iShift;
                    *(pL++) = *(rowRL++) >> iShift;
                }
            }
            else   //rowNum == 2
            {
                unsigned short *rowR = row[pPdMap[0].row], *rowL = row[pPdMap[1].row];
                for (unsigned int x=0; x<pd_x_num; x+=1)
                {
                    *(pR++) = *(rowR++) >> iShift;
                    *(pL++) = *(rowL++) >> iShift;
                }
            }
        }
    }
    else
    {
        for (unsigned int y=0; y<pd_y_num; y+=(subBlkY<<1))
        {
            row[0] = ptr16;
            for (unsigned int i=1; i<rowNum; i++)
                row[i] = row[i-1] + (stride>>1);
            ptr16 = row[rowNum-1] + (stride>>1);

            for( unsigned int ri=0; ri<subBlkY; ri++)
            {
                unsigned int mapIdx = ri*(subBlkX<<1);

                if( subBlkX == 4)   //optimize path
                {
                    int rIdx0 = pPdMap[mapIdx + 0].row, rIdx1 = pPdMap[mapIdx + 1].row;
                    int rIdx2 = pPdMap[mapIdx + 2].row, rIdx3 = pPdMap[mapIdx + 3].row;
                    int rIdx4 = pPdMap[mapIdx + 4].row, rIdx5 = pPdMap[mapIdx + 5].row;
                    int rIdx6 = pPdMap[mapIdx + 6].row, rIdx7 = pPdMap[mapIdx + 7].row;

                    for (unsigned int x=0; x<pd_x_num; x+=subBlkX)
                    {
                        *(pR++) = *(row[rIdx0]++) >> iShift;
                        *(pL++) = *(row[rIdx1]++) >> iShift;
                        *(pR++) = *(row[rIdx2]++) >> iShift;
                        *(pL++) = *(row[rIdx3]++) >> iShift;
                        *(pR++) = *(row[rIdx4]++) >> iShift;
                        *(pL++) = *(row[rIdx5]++) >> iShift;
                        *(pR++) = *(row[rIdx6]++) >> iShift;
                        *(pL++) = *(row[rIdx7]++) >> iShift;
                    }
                }
                else   //general path
                {
                    for (unsigned int x=0; x<pd_x_num; x+=subBlkX)
                    {
                        for (unsigned int idx=0; idx<(subBlkX<<1);)
                        {
                            *(pR++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> iShift;
                            *(pL++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> iShift;
                        }
                    }
                }
            }
        }
    }
}
