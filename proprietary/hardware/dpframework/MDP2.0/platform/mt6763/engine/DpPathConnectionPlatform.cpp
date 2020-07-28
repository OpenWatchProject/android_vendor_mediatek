#include "DpPathConnection.h"
#include "DpPathBase.h"
#include "DpWrapper_ISP.h"
#include "mmsys_config.h"

const DpPathConnection::mout_t DpPathConnection::s_moutMap[MOUT_MAP_SIZE] =
{
    {  tCAMIN, {       tSCL0,       tSCL1, tPATH1_SOUT,    tNone,   tNone},       ISP_MOUT_EN},
    {  tRDMA0, {       tSCL0,       tSCL1, tPATH0_SOUT,    tNone,   tNone}, MDP_RDMA0_MOUT_EN},
    {   tSCL0, { tPATH0_SOUT,     tTDSHP0,       tSCL1,    tNone,   tNone},  MDP_PRZ0_MOUT_EN},
    {   tSCL1, { tPATH0_SOUT,     tTDSHP0, tPATH1_SOUT, tWDMA_EX, tCOLOR0},  MDP_PRZ1_MOUT_EN},
    { tCOLOR0, { tPATH0_SOUT, tPATH1_SOUT,    tWDMA_EX,    tSCL1,   tNone}, MDP_COLOR_MOUT_EN},
};

const DpPathConnection::sel_t DpPathConnection::s_selInMap[SEL_IN_SIZE] =
{
    {       tSCL0, {       tCAMIN,        tRDMA0,   tNone,    tNone,   tNone,        tNone,   tNone,   tNone},   MDP_PRZ0_SEL_IN},
    {       tSCL1, {       tCAMIN,        tRDMA0,   tSCL0,    tNone, tTO_RSZ,      tTDSHP0, tCOLOR0,   tNone},   MDP_PRZ1_SEL_IN},
    {     tTDSHP0, {        tSCL0,         tSCL1,   tNone,    tNone,   tNone,        tNone,   tNone,   tNone},  MDP_TDSHP_SEL_IN},
    {    tWDMA_EX, {      tCOLOR0,         tSCL1,   tNone, tTO_WDMA,   tNone,        tNone,   tNone,   tNone}, DISP_WDMA0_SEL_IN},
    {      tWROT0, {  tPATH0_SOUT, tTO_WROT_SOUT,   tNone,    tNone,   tNone,        tNone,   tNone,   tNone},  MDP_WROT0_SEL_IN},
    {       tWDMA, {  tPATH1_SOUT, tTO_WROT_SOUT,   tNone,    tNone,   tNone,        tNone,   tNone,   tNone},   MDP_WDMA_SEL_IN},
    {     tCOLOR0, {      tTDSHP0,         tSCL1,   tNone,    tNone,   tNone,        tNone,   tNone,   tNone},  MDP_COLOR_SEL_IN},
    { tPATH0_SOUT, {        tSCL0,         tSCL1, tCOLOR0,   tRDMA0,   tNone,        tNone,   tNone,   tNone},  MDP_PATH0_SEL_IN},
    { tPATH1_SOUT, {        tSCL1,       tCOLOR0,   tNone,   tCAMIN,   tNone,        tNone,   tNone,   tNone},  MDP_PATH1_SEL_IN},
};

const DpPathConnection::sout_t DpPathConnection::s_selOutMap[SOUT_MAP_SIZE] =
{
    //{  tTO_WROT_SOUT, {  tWROT0,     tWDMA}, DISP_TO_WROT_SOUT_SEL},
    {    tPATH0_SOUT, {  tWROT0, tTO_DISP0},    MDP_PATH0_SOUT_SEL},
    {    tPATH1_SOUT, {   tWDMA, tTO_DISP1},    MDP_PATH1_SOUT_SEL},
    {   tTDSHP0,      { tCOLOR0,     tSCL1},    MDP_TDSHP_SOUT_SEL},
};


DP_STATUS_ENUM DpPathConnection::initTilePath(struct TILE_PARAM_STRUCT *p_tile_param)
{
    DpPathBase::iterator  iterator;
    DpEngineType          curType;
    int32_t               index;

    /* tile core property */
    TILE_REG_MAP_STRUCT *ptr_tile_reg_map = p_tile_param->ptr_tile_reg_map;

    if (false == m_connected)
    {
        if (false == queryMuxInfo())
        {
            return DP_STATUS_INVALID_PATH;
        }
    }

    for (iterator = m_pPath->begin(); iterator != m_pPath->end(); iterator++)
    {
        if (true == iterator->isOutputDisable())
        {
            continue;
        }

        curType = iterator->getEngineType();
        switch (curType)
        {
        //ISP
        case tIMGI:
            static_cast<DpWrapper_ISP*>(&*iterator)->initTilePath(p_tile_param);
            break;
        //MDP
        case tCAMIN:    ptr_tile_reg_map->CAMIN_EN      = 1; break;
        //case tTO_RSZ:   ptr_tile_reg_map->TO_RSZ_EN     = 1; break;
        case tRDMA0:    ptr_tile_reg_map->RDMA0_EN      = 1; break;
        case tSCL0:     ptr_tile_reg_map->PRZ0_EN       = 1; break;
        case tSCL1:     ptr_tile_reg_map->PRZ1_EN       = 1; break;
        case tTDSHP0:   ptr_tile_reg_map->TDSHP0_EN     = 1; break;
        case tCOLOR0:   ptr_tile_reg_map->COLOR0_EN     = 1; break;
        case tPATH0_SOUT:    ptr_tile_reg_map->PATH0_SOUT_EN  = 1; break;
        case tPATH1_SOUT:    ptr_tile_reg_map->PATH1_SOUT_EN  = 1; break;
        case tWROT0:    ptr_tile_reg_map->WROT0_EN      = 1; break;
        case tWDMA:     ptr_tile_reg_map->WDMA_EN       = 1; break;
        //case tTO_DISP0:   ptr_tile_reg_map->TO_DISP0_EN     = 1; break;
        //case tTO_DISP1:   ptr_tile_reg_map->TO_DISP0_EN     = 1; break;
        //case tTO_WROT:   ptr_tile_reg_map->TO_WROT_EN     = 1; break;
        default:        break;
        }
    }

    for (index = 0; index < MOUT_NUM; index++)
    {
        switch (s_moutMap[index].id)
        {
        case tCAMIN:    ptr_tile_reg_map->CAMIN_OUT     = m_mOutInfo[index]; break;
        case tRDMA0:    ptr_tile_reg_map->RDMA0_OUT     = m_mOutInfo[index]; break;
        case tSCL0:     ptr_tile_reg_map->PRZ0_OUT      = m_mOutInfo[index]; break;
        case tSCL1:     ptr_tile_reg_map->PRZ1_OUT      = m_mOutInfo[index]; break;
        case tCOLOR0:   ptr_tile_reg_map->COLOR0_OUT    = m_mOutInfo[index]; break;
        default:        assert(0);
        }
    }

    for (index = 0; index < SEL_IN_NUM; index++)
    {
        switch (s_selInMap[index].id)
        {
        case tSCL0:     ptr_tile_reg_map->PRZ0_SEL      = m_sInInfo[index]; break;
        case tSCL1:     ptr_tile_reg_map->PRZ1_SEL      = m_sInInfo[index]; break;
        case tTDSHP0:   ptr_tile_reg_map->TDSHP0_SEL    = m_sInInfo[index]; break;
        case tCOLOR0:     ptr_tile_reg_map->COLOR0_SEL     = m_sInInfo[index]; break;
        case tWROT0:    ptr_tile_reg_map->WROT0_SEL     = m_sInInfo[index]; break;
        case tWDMA:     ptr_tile_reg_map->WDMA_SEL      = m_sInInfo[index]; break;
        case tPATH0_SOUT: ptr_tile_reg_map->PATH0_SEL      = m_sInInfo[index]; break;
        case tPATH1_SOUT: ptr_tile_reg_map->PATH1_SEL      = m_sInInfo[index]; break;
        case tWDMA_EX:    ptr_tile_reg_map->MDP_WDMA_SEL = m_sInInfo[index]; break;
        default:        assert(0);
        }
    }

    for (index = 0; index < SEL_OUT_NUM; index++)
    {
        switch (s_selOutMap[index].id)
        {
        //case tBLS:      ptr_tile_reg_map->BLS_SLO       = m_sOutInfo[index]; break;
        //case tTO_WROT_SOUT:    ptr_tile_reg_map->TO_WROT_SOUT     = m_sOutInfo[index]; break;
        case tPATH0_SOUT:      ptr_tile_reg_map->PATH0_SOUT       = m_sOutInfo[index]; break;
        case tPATH1_SOUT:      ptr_tile_reg_map->PATH1_SOUT       = m_sOutInfo[index]; break;
        case tTDSHP0:     ptr_tile_reg_map->TDSHP0_SOUT      = m_sOutInfo[index]; break;
        default:        assert(0);
        }
    }

    return DP_STATUS_RETURN_SUCCESS;
}

