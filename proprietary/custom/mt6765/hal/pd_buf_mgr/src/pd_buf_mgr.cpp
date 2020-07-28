#include <utils/Log.h>
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
#include <pd_imx499mipiraw.h>
#include <pd_s5k3m2mipiraw.h>
#include <pd_s5k3m3mipiraw.h>
#include <pd_s5k2x8mipiraw.h>
#include <pd_s5k2l7mipiraw.h>
#include <pd_ov16880mipiraw.h>
#include <pd_imx362mipiraw.h>
#include <pd_imx398mipiraw.h>
#include <pd_s5k3p8sxmipiraw.h>
#include <pd_ov13855mipiraw.h>
#include <pd_s5k3l8mipiraw.h>
#include <pd_ov12a10mipiraw.h>
#include <pd_imx486mipiraw.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif

#define LOG_TAG "pd_buf_mgr"

PDBufMgr::PDBufMgr() :
    m_subBlkNumX(0),
    m_subBlkNumY(0),
    m_rowNum(0)
{
    memset( &m_PDBlockInfo, 0, sizeof(SET_PD_BLOCK_INFO_T));
    memset( &m_sPDOHWInfo,  0, sizeof(SPDOHWINFO_T       ));
    memset( &m_sPdProfile,  0, sizeof(SPDProfile_t       ));
    memset( pPdMap,   0, sizeof(PDMAP_T  ) * MAX_PAIR_NUM * 2);
    memset( pdPixels, 0, sizeof(PDPIXEL_T) * MAX_PAIR_NUM * 2);
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
#if defined(S5K3P8SX_MIPI_RAW)
    case S5K3P8SX_SENSOR_ID :
        instance = PD_S5K3P8SXMIPIRAW::getInstance();
        break;
#endif
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
#if defined(IMX499_MIPI_RAW)
    case IMX499_SENSOR_ID :
        instance = PD_IMX499MIPIRAW::getInstance();
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
#if defined(OV13855_MIPI_RAW)
    case OV13855_SENSOR_ID:
        instance = PD_OV13855MIPIRAW::getInstance();
        break;
#endif
#if defined(S5K3L8_MIPI_RAW)
    case S5K3L8_SENSOR_ID:
        instance = PD_S5K3L8MIPIRAW::getInstance();
        break;
#endif
#if defined(OV12A10_MIPI_RAW)
    case OV12A10_SENSOR_ID:
        instance = PD_OV12A10MIPIRAW::getInstance();
        break;
#endif
#if defined(IMX486_MIPI_RAW)
    case IMX486_SENSOR_ID:
        instance = PD_IMX486MIPIRAW::getInstance();
        break;
#endif
    default :
        instance = NULL;
        break;
    }

    if( instance)
        ret = instance->IsSupport(iPdProfile) ? instance : NULL;

    AAA_LOGD( "%s SensorId(0x%x) instance(%p) PDBufMgrType(%d) IsZSD(%d) ImgSz(%d %d) FullSz(%d %d) PDAF_support(%d) IsFrontalBin(%d) IsPBINen(%d) DualPDSeparateMode(%d) SensorMode(%d) AETargetMode(%d) pipeCfg(%d) curSensorModeSupportPD(%d)",
              __FUNCTION__,
              iPdProfile.i4CurrSensorId,
              instance,
              iPdProfile.BufType,
              iPdProfile.u4IsZSD,
              iPdProfile.uImgXsz,
              iPdProfile.uImgYsz,
              iPdProfile.uFullXsz,
              iPdProfile.uFullYsz,
              iPdProfile.u4PDAF_support_type,
              iPdProfile.u4IsFrontalBinning,
              iPdProfile.bEnablePBIN,
              iPdProfile.uPdSeparateMode,
              iPdProfile.i4SensorMode,
              iPdProfile.AETargetMode,
              iPdProfile.u4PDPipeCtrl,
              iPdProfile.bSensorModeSupportPD);

    return ret;
}

MBOOL PDBufMgr::SetPDInfo( SET_PD_BLOCK_INFO_T &iPDBlockInfo, SPDProfile_t &iPdProfile, SPDOHWINFO_T &iPDOHWInfo)
{
    memcpy( &m_sPDOHWInfo, &iPDOHWInfo, sizeof(SPDOHWINFO_T));

    memcpy( &m_sPdProfile, &iPdProfile, sizeof(SPDProfile_t));
    AAA_LOGD( "%s SensorId(0x%x) PDBufMgrType(0x%x) IsZSD(%d) ImgSz(%d %d) FullSz(%d %d) PDAF_support(%d) IsFrontalBin(%d) IsPBINen(%d) DualPDSeparateMode(%d) SensorMode(%d) AETargetMode(%d) pipeCfg(%d) curSensorModeSupportPD(%d)",
              __FUNCTION__,
              m_sPdProfile.i4CurrSensorId,
              m_sPdProfile.BufType,
              m_sPdProfile.u4IsZSD,
              m_sPdProfile.uImgXsz,
              m_sPdProfile.uImgYsz,
              m_sPdProfile.uFullXsz,
              m_sPdProfile.uFullYsz,
              m_sPdProfile.u4PDAF_support_type,
              m_sPdProfile.u4IsFrontalBinning,
              m_sPdProfile.bEnablePBIN,
              m_sPdProfile.uPdSeparateMode,
              m_sPdProfile.i4SensorMode,
              m_sPdProfile.AETargetMode,
              m_sPdProfile.u4PDPipeCtrl,
              m_sPdProfile.bSensorModeSupportPD);

    // Coordinate in full size
    memcpy( &m_PDBlockInfo, &iPDBlockInfo, sizeof(SET_PD_BLOCK_INFO_T));

    /* Initial general separate function*/
    if( m_sPdProfile.BufType == EPDBUF_PDO)
    {
        unsigned int pitch_x      = m_PDBlockInfo.i4PitchX;
        unsigned int pitch_y      = m_PDBlockInfo.i4PitchY;
        unsigned int nblk_x       = m_PDBlockInfo.i4BlockNumX;
        unsigned int nblk_y       = m_PDBlockInfo.i4BlockNumY;
        unsigned int pairNum      = m_PDBlockInfo.i4PairNum;
        unsigned int full_lOffx   = m_PDBlockInfo.i4OffsetX;
        unsigned int full_lOffy   = m_PDBlockInfo.i4OffsetY;
        unsigned int sub_blk_sz_x = m_PDBlockInfo.i4SubBlkW;
        unsigned int sub_blk_sz_y = m_PDBlockInfo.i4SubBlkH;
        unsigned int pixelNum     = pairNum*2;

        AAA_LOGD( "SensorMode(%d), SensorInfo : OffsetX(%d), OffsetY(%d), PitchX(%d), PitchY(%d), PairNu(%d), SubBlkW(%d), SubBlkH(%d), BlockNumX(%d), BlockNumY(%d), LeFirst(%d)",
                  m_sPdProfile.i4SensorMode,
                  full_lOffx,
                  full_lOffy,
                  pitch_x,
                  pitch_y,
                  pairNum,
                  sub_blk_sz_x,
                  sub_blk_sz_y,
                  nblk_x,
                  nblk_y,
                  m_PDBlockInfo.i4LeFirst);

        /*******************************************************************************
         * basic check pd block information.
         *******************************************************************************/
        if( /* check block number and pitch information */
            ( nblk_x  == 0) ||
            ( nblk_y  == 0) ||
            ( pitch_x == 0) ||
            ( pitch_y == 0) ||
            ( m_sPdProfile.uImgXsz < (nblk_x*pitch_x+full_lOffx)) ||
            ( m_sPdProfile.uImgYsz < (nblk_y*pitch_y+full_lOffy)))
        {
            AAA_LOGE("block number is not correct (%d)<(%d*%d+%d) or (%d)<(%d*%d+%d), checking sensor driver\n",
                     m_sPdProfile.uImgXsz,
                     nblk_x,
                     pitch_x,
                     full_lOffx,
                     m_sPdProfile.uImgYsz,
                     nblk_y,
                     pitch_y,
                     full_lOffy);

            return MFALSE;
        }
        else if( (pairNum*sub_blk_sz_x*sub_blk_sz_y) != (pitch_x*pitch_y)) /* check pairNum information */
        {
            AAA_LOGE("pairNum is not correct (%d)!=(%d/%d)*(%d/%d), checking sensor driver\n",
                     pairNum,
                     pitch_x,
                     sub_blk_sz_x,
                     pitch_y,
                     sub_blk_sz_y);

            return MFALSE;
        }
        else if( pairNum > MAX_PAIR_NUM)
        {
            AAA_LOGE("can not support GSF, pariNum(%d) > buffer size(%d)\n",
                     pairNum,
                     MAX_PAIR_NUM);

            return MFALSE;
        }

        /*******************************************************************************
         * Reference crop region and mirrorflip inction to modify PD block information.
         *******************************************************************************/
        // Current image related full size coordinate
        unsigned int crop_x = m_PDBlockInfo.i4Crop[m_sPdProfile.i4SensorMode][0];
        unsigned int crop_y = m_PDBlockInfo.i4Crop[m_sPdProfile.i4SensorMode][1];

        // Current pd block offset related to full size coordinate
        int shift_x = crop_x-full_lOffx;
        if( shift_x<=0)
        {
            shift_x = -crop_x;
        }
        else if( shift_x%pitch_x)
        {
            shift_x = ( (shift_x+(pitch_x-1)) / pitch_x) * pitch_x - crop_x;
        }
        else
        {
            shift_x = ( shift_x / pitch_x) * pitch_x - crop_x;
        }


        int shift_y = crop_y-full_lOffy;
        if( shift_y<=0)
        {
            shift_y = -crop_y;
        }
        else if( shift_y%pitch_y)
        {
            shift_y = ( (shift_y+(pitch_y-1)) / pitch_y) * pitch_y - crop_y;
        }
        else
        {
            shift_y = ( shift_y / pitch_y) * pitch_y - crop_y;
        }

        AAA_LOGD( "coordinate shift in current sensor mode (%d, %d)\n", shift_x, shift_y);

        /*******************************************************************************
         * calculate pd pixels' position by orientation and crop information for general separate function
         *******************************************************************************/
        unsigned int cur_lOffx = full_lOffx + shift_x;
        unsigned int cur_lOffy = full_lOffy + shift_y;
        unsigned int cur_rOffx = m_sPdProfile.uImgXsz - cur_lOffx - pitch_x * nblk_x;
        unsigned int cur_rOffy = m_sPdProfile.uImgYsz - cur_lOffy - pitch_y * nblk_y;
        unsigned int offx  = (m_PDBlockInfo.iMirrorFlip & 0x1) ? cur_rOffx : cur_lOffx;
        unsigned int offy  = (m_PDBlockInfo.iMirrorFlip & 0x2) ? cur_rOffy : cur_lOffy;

        AAA_LOGD("mirror_flip(%x), block offset : left side(%d, %d) right sied(%d, %d)\n", m_PDBlockInfo.iMirrorFlip, cur_lOffx, cur_lOffy, cur_rOffx, cur_rOffy);

        memset( pdPixels, 0, sizeof(PDPIXEL_T) * MAX_PAIR_NUM * 2);

        for(unsigned int Pidx=0; Pidx<pairNum; Pidx++)
        {
            unsigned int PosL_X = m_PDBlockInfo.i4PosL[Pidx][0];
            unsigned int PosL_Y = m_PDBlockInfo.i4PosL[Pidx][1];
            unsigned int PosR_X = m_PDBlockInfo.i4PosR[Pidx][0];
            unsigned int PosR_Y = m_PDBlockInfo.i4PosR[Pidx][1];
            MUINT32 idx = Pidx;

            if( /* boundary check */
                ((PosL_X-full_lOffx)<pitch_x) && ((PosL_Y-full_lOffy)<pitch_y) &&
                ((PosR_X-full_lOffx)<pitch_x) && ((PosR_Y-full_lOffy)<pitch_y))
            {
                /* shift coordinate by crop information */
                PosL_X += shift_x;
                PosL_Y += shift_y;
                PosR_X += shift_x;
                PosR_Y += shift_y;

                /* mirror*/
                if(m_PDBlockInfo.iMirrorFlip & 0x1)
                {
                    PosL_X = pitch_x - (PosL_X - cur_lOffx) - 1 + cur_rOffx;
                    PosR_X = pitch_x - (PosR_X - cur_lOffx) - 1 + cur_rOffx;
                }

                /* flip*/
                if(m_PDBlockInfo.iMirrorFlip & 0x2)
                {
                    PosL_Y = pitch_y - (PosL_Y - cur_lOffy) - 1 + cur_rOffy;
                    PosR_Y = pitch_y - (PosR_Y - cur_lOffy) - 1 + cur_rOffy;
                    idx    = (pairNum - 1) - Pidx;
                }

                pdPixels[idx].cx = PosR_X;
                pdPixels[idx].cy = PosR_Y;
                pdPixels[idx].bx = (pdPixels[idx].cx - offx)/sub_blk_sz_x;
                pdPixels[idx].by = (pdPixels[idx].cy - offy)/sub_blk_sz_y;
                pdPixels[idx].lr = R;

                pdPixels[idx + pairNum].cx = PosL_X;
                pdPixels[idx + pairNum].cy = PosL_Y;
                pdPixels[idx + pairNum].bx = (pdPixels[idx + pairNum].cx - offx)/sub_blk_sz_x;
                pdPixels[idx + pairNum].by = (pdPixels[idx + pairNum].cy - offy)/sub_blk_sz_y;
                pdPixels[idx + pairNum].lr = L;

                AAA_LOGD( "PDPos : L(%d):block index(%d,%d),coordinate(%3d %3d)->(%3d %3d), R(%d):block index(%d, %d),coordinate(%3d %3d)->(%3d %3d)",
                          pdPixels[idx + pairNum].lr,
                          pdPixels[idx + pairNum].bx,
                          pdPixels[idx + pairNum].by,
                          m_PDBlockInfo.i4PosL[Pidx][0],
                          m_PDBlockInfo.i4PosL[Pidx][1],
                          pdPixels[idx + pairNum].cx,
                          pdPixels[idx + pairNum].cy,
                          pdPixels[idx].lr,
                          pdPixels[idx].bx,
                          pdPixels[idx].by,
                          m_PDBlockInfo.i4PosR[Pidx][0],
                          m_PDBlockInfo.i4PosR[Pidx][1],
                          pdPixels[idx].cx,
                          pdPixels[idx].cy);

            }
            else
            {
                AAA_LOGE("PD coordinate is not sutible at pair index(%d), PDPos [L](%3d %3d) [R](%3d %3d)\n",
                         Pidx,
                         PosL_X,
                         PosL_Y,
                         PosR_X,
                         PosR_Y);

                return MFALSE;
            }
        }

        //
        {
            unsigned int idx = 0;
            PDPIXEL_T* pPdOut = (PDPIXEL_T*)malloc(pixelNum * sizeof(PDPIXEL_T));
            unsigned int cy[MAX_PDO_BUF_ROW_NUM];

            // Find how many rows of one PD block.
            unsigned int tarY = 0;
            m_rowNum = 0;
            while( idx < pixelNum)
            {
                unsigned int minY = 0xFFFFFFFF;

                //find min cy
                for( unsigned int i=0; i<pixelNum; i++)
                {
                    if( /**/
                        (pdPixels[i].cy<minY) &&
                        (tarY<pdPixels[i].cy))
                    {
                        minY = pdPixels[i].cy;
                    }
                }
                tarY = minY;

                //
                if( m_rowNum < MAX_PDO_BUF_ROW_NUM)
                {
                    cy[m_rowNum++] = tarY;
                }
                else
                {
                    AAA_LOGD("Error! Exceed MAX_PDO_BUF_ROW_NUM!\n");
                }

                //
                for( unsigned int i=0; i<pixelNum; i++)
                {
                    if( pdPixels[i].cy==tarY)
                    {
                        memcpy((void*)&pPdOut[idx++], (void*)&pdPixels[i], sizeof(PDPIXEL_T));
                    }
                }
            }

            //sort by pdo buffer row sequence
            bool bSorting = true;
            while( bSorting)
            {
                bSorting = false;
                for( unsigned int i=0; i<pixelNum-1; i++)
                {
                    if( /* sorting by coordinator*/
                        (pPdOut[i].cy == pPdOut[i+1].cy)&&
                        (pPdOut[i].cx > pPdOut[i+1].cx))
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
            unsigned int minX=0xFFFFFFFF;
            for( unsigned int i=0; i<pixelNum; i++)
            {
                AAA_LOGD("(cx,cy)=(%4d, %4d), (bx,by)=(%d, %d), lr=(%d)",
                         pPdOut[i].cx,
                         pPdOut[i].cy,
                         pPdOut[i].bx,
                         pPdOut[i].by,
                         pPdOut[i].lr);
                if( pPdOut[i].cx < minX)
                {
                    minX = pPdOut[i].cx;
                }
            }

            //sort by block postion (0,0)-->(1,0)-->(2,0) ...
            idx=0;
            for( unsigned int bIdx=0; bIdx<=pixelNum/2; bIdx+=pixelNum/2)
            {
                unsigned int blkX=0, blkY=0;
                while( idx < (bIdx + pixelNum/2))
                {
                    bool bMatch = false;
                    for( unsigned int i=bIdx; i<(bIdx+pixelNum/2); i++)
                    {
                        if( pdPixels[i].bx == blkX && pdPixels[i].by == blkY)
                        {
                            memcpy((void*)&pPdOut[idx++], (void*)&pdPixels[i], sizeof(PDPIXEL_T));
                            bMatch = true;
                            break;
                        }
                    }
                    if( bMatch)
                    {
                        blkX ++;
                    }
                    else
                    {
                        blkX = 0;
                        blkY ++;
                    }
                }
            }

            //fill pPdMap
            for(unsigned int i=0; i<pixelNum/2; i++)
            {
                pPdMap[i*2].cx = pPdOut[i].cx;
                pPdMap[i*2].cy = pPdOut[i].cy;
                pPdMap[i*2].lr = pPdOut[i].lr;
                pPdMap[i*2+1].cx = pPdOut[i+pixelNum/2].cx;
                pPdMap[i*2+1].cy = pPdOut[i+pixelNum/2].cy;
                pPdMap[i*2+1].lr = pPdOut[i+pixelNum/2].lr;
            }

            //for each LR pair in the same row, make sure smaller-x reached first
            for(unsigned int i=0; i<pixelNum/2; i++)
            {
                if( /**/
                    (pPdMap[i*2].cy == pPdMap[i*2+1].cy) &&
                    (pPdMap[i*2].cx >  pPdMap[i*2+1].cx))
                {
                    //swap
                    PDMAP_T temp;
                    memcpy((void*)&temp, (void*)&pPdMap[i*2+1], sizeof(PDMAP_T));
                    memcpy((void*)&pPdMap[i*2+1], (void*)&pPdMap[i*2], sizeof(PDMAP_T));
                    memcpy((void*)&pPdMap[i*2  ], (void*)&temp,        sizeof(PDMAP_T));
                }
            }

            //
            somePairedLRLedByL = 0;
            for(unsigned int i=0; i<pixelNum; i++)
            {
                for(unsigned int j=0; j<m_rowNum; j++)
                {
                    if(pPdMap[i].cy == cy[j])
                    {
                        pPdMap[i].row = j;
                    }
                }
                AAA_LOGD("(cx,cy)=(%3d, %3d), row=(%02d), lr=(%d)",
                         pPdMap[i].cx,
                         pPdMap[i].cy,
                         pPdMap[i].row,
                         pPdMap[i].lr);
                if( /**/
                    (i%2) &&
                    (pPdMap[i].lr!=L))
                {
                    somePairedLRLedByL = true;
                }
            }

            //
            m_subBlkNumX = pitch_x/sub_blk_sz_x;
            m_subBlkNumY = pitch_y/sub_blk_sz_y;
            AAA_LOGD("PD sub block number(%d, %d), pdo buffer row num of one PD block(%d) somePairedLRLedByL(%d)\n\n", m_subBlkNumX, m_subBlkNumY, m_rowNum, somePairedLRLedByL);

            //optimize for symmetric pattern
            while(m_subBlkNumY>1)   //optimize Y arrangement
            {
                bool bSym = true;
                int rowDiff = pPdMap[pixelNum/2].row - pPdMap[0].row;

                //
                if(rowDiff)
                {
                    for(unsigned int i=0; i<pixelNum/2; i++)
                    {
                        if( /**/
                            (pPdMap[i+pixelNum/2].row != pPdMap[i].row + rowDiff) ||
                            (pPdMap[i+pixelNum/2].lr  != pPdMap[i].lr           ))
                        {
                            bSym = false;
                            break;
                        }
                    }
                }
                else
                {
                    bSym = false;
                }

                //
                if (bSym)
                {
                    m_subBlkNumY /= 2;
                    m_rowNum     /= 2;
                    pixelNum     /= 2;
                    AAA_LOGD("Y-dir symmetric optimization!\npd blocks(%d, %d), pdo buffer row num(%d)\n\n", m_subBlkNumX, m_subBlkNumY, m_rowNum);
                }
                else
                {
                    break;
                }
            }

            //
            while(m_subBlkNumX > 1)   //optimize X arrangement
            {
                bool bSym = true;
                int rowDiff = pPdMap[0].row - pPdMap[1].row;

                //
                for(unsigned int i=0; i<pixelNum; i+=2)
                {
                    if( /**/
                        (pPdMap[i].row != pPdMap[i+1].row + rowDiff) ||
                        (pPdMap[i+pixelNum/2].lr != pPdMap[i].lr))
                    {
                        bSym = false;
                        break;
                    }
                }
                if (bSym)
                {
                    m_subBlkNumX /= 2;
                    pixelNum     /= 2;
                    AAA_LOGD("X-dir symmetric optimization!\npd blocks(%d, %d), pdo buffer row num(%d)\n\n", m_subBlkNumX, m_subBlkNumY, m_rowNum);
                }
                else
                {
                    break;
                }
            }

            free(pPdOut);

        }

    }

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
MVOID PDBufMgr::separateLR( unsigned int stride, unsigned char *ptr, unsigned int pd_x_num, unsigned int pd_y_num, unsigned short *outBuf, unsigned int bitDepth, PD_AREA_T * /*ptrPDRegion*/)
{
    //
    if( m_rowNum==0)
    {
        AAA_LOGE("separateByMapping() called before parsing!");
        return;
    }

    //
    unsigned short *inBuf = (unsigned short*)ptr;
    unsigned short *ptr16 = inBuf;

    unsigned short *row[MAX_PDO_BUF_ROW_NUM];
    unsigned int LRoffset = pd_x_num*pd_y_num>>1;
    unsigned short *pL = outBuf;
    unsigned short *pR = pL + LRoffset;

    unsigned int shift_bits = 0;// converting data to 10 bits

    //
    if( (10<=bitDepth) && (bitDepth<=16))
    {
        shift_bits = bitDepth - 10;
    }
    else
    {
        AAA_LOGE("%s abort : wrong input bitDepth(%d)", __FUNCTION__, bitDepth);
        return;
    }

    //
    if(!somePairedLRLedByL)
    {
        if(m_subBlkNumX == 1 && m_subBlkNumY == 1)
        {
            for(unsigned int y=0; y<pd_y_num; y+=2)
            {
                row[0] = ptr16;
                for(unsigned int i=1; i<m_rowNum; i++)
                    row[i] = row[i-1] + (stride>>1);
                ptr16 = row[m_rowNum-1] + (stride>>1);

                if(m_rowNum == 1)
                {
                    unsigned short *rowRL = row[0];
                    for (unsigned int x=0; x<pd_x_num; x+=1)
                    {
                        *(pR++) = *(rowRL++) >> shift_bits;
                        *(pL++) = *(rowRL++) >> shift_bits;
                    }
                }
                else   //rowNum == 2
                {
                    unsigned short *rowR = row[pPdMap[0].row], *rowL = row[pPdMap[1].row];
                    for (unsigned int x=0; x<pd_x_num; x+=1)
                    {
                        *(pR++) = *(rowR++) >> shift_bits;
                        *(pL++) = *(rowL++) >> shift_bits;
                    }
                }

            }

        }
        else
        {
            for(unsigned int y=0; y<pd_y_num; y+=(m_subBlkNumY<<1))
            {
                row[0] = ptr16;
                for(unsigned int i=1; i<m_rowNum; i++)
                    row[i] = row[i-1] + (stride>>1); //stride in BYTE
                ptr16 = row[m_rowNum-1] + (stride>>1);

                for( unsigned int ri=0; ri<m_subBlkNumY; ri++)
                {
                    unsigned int mapIdx = ri*(m_subBlkNumX<<1);

                    if( m_subBlkNumX == 4)   //optimize path
                    {
                        int rIdx0 = pPdMap[mapIdx + 0].row, rIdx1 = pPdMap[mapIdx + 1].row;
                        int rIdx2 = pPdMap[mapIdx + 2].row, rIdx3 = pPdMap[mapIdx + 3].row;
                        int rIdx4 = pPdMap[mapIdx + 4].row, rIdx5 = pPdMap[mapIdx + 5].row;
                        int rIdx6 = pPdMap[mapIdx + 6].row, rIdx7 = pPdMap[mapIdx + 7].row;

                        for (unsigned int x=0; x<pd_x_num; x+=m_subBlkNumX)
                        {
                            *(pR++) = *(row[rIdx0]++) >> shift_bits;
                            *(pL++) = *(row[rIdx1]++) >> shift_bits;
                            *(pR++) = *(row[rIdx2]++) >> shift_bits;
                            *(pL++) = *(row[rIdx3]++) >> shift_bits;
                            *(pR++) = *(row[rIdx4]++) >> shift_bits;
                            *(pL++) = *(row[rIdx5]++) >> shift_bits;
                            *(pR++) = *(row[rIdx6]++) >> shift_bits;
                            *(pL++) = *(row[rIdx7]++) >> shift_bits;
                        }
                    }
                    else   //general path
                    {
                        for(unsigned int x=0; x<pd_x_num; x+=m_subBlkNumX)
                        {
                            for(unsigned int idx=0; idx<(m_subBlkNumX<<1);)
                            {
                                *(pR++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> shift_bits;
                                *(pL++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> shift_bits;
                            }
                        }
                    }
                }
            }
        }
    }
    else //somePairedLRLedByL
    {
        if(m_subBlkNumX == 1 && m_subBlkNumY == 1 && m_rowNum == 1)
        {
            for(unsigned int y=0; y<pd_y_num; y+=2)
            {
                row[0] = ptr16;
                for(unsigned int i=1; i<m_rowNum; i++)
                    row[i] = row[i-1] + (stride>>1);
                ptr16 = row[m_rowNum-1] + (stride>>1);

                {
                    unsigned short *rowRL = row[0];
                    for(unsigned int x=0; x<pd_x_num; x+=1)
                    {
                        *(pL++) = *(rowRL++) >> shift_bits;
                        *(pR++) = *(rowRL++) >> shift_bits;
                    }
                }
            }
        }
        else //general path
        {
            for(unsigned int y=0; y<pd_y_num; y+=(m_subBlkNumY<<1))
            {
                row[0] = ptr16;
                for(unsigned int i=1; i<m_rowNum; i++)
                    row[i] = row[i-1] + (stride>>1);
                ptr16 = row[m_rowNum-1] + (stride>>1);

                for(unsigned int ri=0; ri<m_subBlkNumY; ri++)
                {
                    unsigned int mapIdx = ri*(m_subBlkNumX<<1);
                    for(unsigned int x=0; x<pd_x_num; x+=m_subBlkNumX)
                    {
                        for(unsigned int idx=0; idx<(m_subBlkNumX<<1);)
                        {
                            if(pPdMap[mapIdx + idx].lr == R)
                            {
                                *(pR++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> shift_bits;
                                *(pL++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> shift_bits;
                            }
                            else
                            {
                                *(pL++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> shift_bits;
                                *(pR++) = *(row[pPdMap[mapIdx + (idx++)].row]++) >> shift_bits;
                            }
                        }
                    }
                }
            }
        }
    }
}
