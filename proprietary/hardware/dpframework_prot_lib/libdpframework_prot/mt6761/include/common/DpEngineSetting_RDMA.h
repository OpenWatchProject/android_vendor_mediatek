#ifndef __DP_ENGINE_SETTING_RDMA_H__
#define __DP_ENGINE_SETTING_RDMA_H__

#include "DpColorFormat.h"

struct RdmaInput {
    uint32_t RDMA0_lb_2b_mode;
    uint32_t RDMA0_buffer_mode;
    uint32_t identifier;
    DpColorFormat colorFormat;
    int32_t inTileXRight;
    int32_t inTileXLeft;
};

struct RdmaOutput {
    uint32_t RDMA0_mb_depth;
    uint32_t RDMA0_mf_sb;
    uint32_t RDMA0_mf_jump;
    uint32_t RDMA0_mb_lp;
    uint32_t RDMA0_mb_pps;
    uint32_t RDMA0_sb_depth;
    uint32_t RDMA0_sf_sb;
    uint32_t RDMA0_sf_jump;
    uint32_t RDMA0_sb_lp;
    uint32_t RDMA0_sb_pps;
};

void calRDMASetting(RdmaInput *inParam, RdmaOutput *outParam);

#endif