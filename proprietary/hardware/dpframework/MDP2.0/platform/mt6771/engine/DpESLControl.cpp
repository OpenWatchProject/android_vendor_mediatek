#include "DpESLControl.h"

DP_STATUS_ENUM setESLRDMA(DpCommand &command,
                          uint32_t identifier,
                          DpColorFormat colorFormat)
{
    uint32_t blockMode = DP_COLOR_GET_BLOCK_MODE(colorFormat);
    uint32_t planeCount = DP_COLOR_GET_PLANE_COUNT(colorFormat);
    uint32_t uniqueID = DP_COLOR_GET_UNIQUE_ID(colorFormat);
    uint32_t m_identifier = identifier;

    uint32_t dmabuf_con_0 = 0;
    uint32_t dmaultra_con_0 = 0;
    uint32_t dmabuf_con_1 = 0;
    uint32_t dmaultra_con_1 = 0;
    uint32_t dmabuf_con_2 = 0;
    uint32_t dmaultra_con_2 = 0;

    dmabuf_con_0 |= (3 << 24);

    if (blockMode) {
        dmabuf_con_0 |= (32 << 0);
        dmaultra_con_0 |= (60 << 24) + (40 << 8);
        dmabuf_con_1 |= (1 << 24) + (32 << 0);
        dmaultra_con_1 |= (30 << 24) + (20 << 8);
        dmabuf_con_2 |= (3 << 24);
    }
    else if (planeCount == 3) {
        dmabuf_con_0 |= (20 << 0);
        dmaultra_con_0 |= (60 << 24) + (40 << 8);
        dmabuf_con_1 |= (1 << 24) + (10 << 0);
        dmaultra_con_1 |= (15 << 24) + (10 << 8);
        dmabuf_con_2 |= (1 << 24) + (10 << 0);
        dmaultra_con_2 |= (15 << 24) + (10 << 8);
    }
    else if (planeCount == 2) {
        dmabuf_con_0 |= (20 << 0);
        dmaultra_con_0 |= (60 << 24) + (40 << 8);
        dmabuf_con_1 |= (1 << 24) + (10 << 0);
        dmaultra_con_1 |= (30 << 24) + (20 << 8);
        dmabuf_con_2 |= (3 << 24);
    }
    else if (uniqueID == 0 || uniqueID == 1) { //RGB
        dmabuf_con_0 |= (40 << 0);
        dmaultra_con_0 |= (180 << 24) + (120 << 8);
        dmabuf_con_1 |= (3 << 24);
        dmabuf_con_2 |= (3 << 24);
    }
    else if (uniqueID == 2 || uniqueID == 3) { //ARGB
        dmabuf_con_0 |= (40 << 0);
        dmaultra_con_0 |= (240 << 24) + (160 << 8);
        dmabuf_con_1 |= (3 << 24);
        dmabuf_con_2 |= (3 << 24);
    }
    else if (uniqueID == 4 || uniqueID == 5) { //UYVY
        dmabuf_con_0 |= (40 << 0);
        dmaultra_con_0 |= (120 << 24) + (80 << 8);
        dmabuf_con_1 |= (3 << 24);
        dmabuf_con_2 |= (3 << 24);
    }
    else if (uniqueID == 7) { //Y8
        dmabuf_con_0 |= (20 << 0);
        dmaultra_con_0 |= (60 << 24) + (40 << 8);
        dmabuf_con_1 |= (3 << 24);
        dmabuf_con_2 |= (3 << 24);
    }
    else {
        return DP_STATUS_RETURN_SUCCESS;
    }

    MM_REG_WRITE(command, MDP_RDMA_DMABUF_CON_0, dmabuf_con_0, 0x07FF007F);
    MM_REG_WRITE(command, MDP_RDMA_DMAULTRA_CON_0, dmaultra_con_0, 0xFFFFFFFF);
    MM_REG_WRITE(command, MDP_RDMA_DMABUF_CON_1, dmabuf_con_1, 0x077F003F);
    MM_REG_WRITE(command, MDP_RDMA_DMAULTRA_CON_1, dmaultra_con_1, 0x7F7F7F7F);
    MM_REG_WRITE(command, MDP_RDMA_DMABUF_CON_2, dmabuf_con_2, 0x073F003F);
    MM_REG_WRITE(command, MDP_RDMA_DMAULTRA_CON_2, dmaultra_con_2, 0x3F3F3F3F);
    return DP_STATUS_RETURN_SUCCESS;
}
DP_STATUS_ENUM setESLWROT(DpCommand &command,
                          uint32_t identifier,
                          DpColorFormat colorFormat)
{
    uint32_t planeCount = DP_COLOR_GET_PLANE_COUNT(colorFormat);
    uint32_t uniqueID = DP_COLOR_GET_UNIQUE_ID(colorFormat);
    uint32_t m_identifier = identifier;

    uint32_t vido_dma_preultra = 0;

    if (planeCount == 3 || planeCount == 2 || uniqueID == 7) { //3-plane, 2-plane, Y8
        vido_dma_preultra = (identifier == 0)? ((88 << 12) + (68 << 0)):((216 << 12) + (196 << 0));
    }
    else if (uniqueID == 0 || uniqueID == 1) { //RGB
        vido_dma_preultra = (identifier == 0)? ((8 << 12) + (16 << 0)):((136 << 12) + (76 << 0));
    }
    else if (uniqueID == 2 || uniqueID == 3) { //ARGB
        vido_dma_preultra = (identifier == 0)? ((16 << 12) + (16 << 0)):((96 << 12) + (16 << 0));
    }
    else if (uniqueID == 4 || uniqueID == 5) { //UYVY
        vido_dma_preultra = (identifier == 0)? ((48 << 12) + (8 << 0)):((176 << 12) + (136 << 0));
    }
    else {
        return DP_STATUS_RETURN_SUCCESS;
    }

    MM_REG_WRITE(command, VIDO_DMA_PREULTRA, vido_dma_preultra, 0x0FFFFFF);
    return DP_STATUS_RETURN_SUCCESS;
}

DP_STATUS_ENUM setESLWDMA(DpCommand &command,
                          DpColorFormat colorFormat)
{
    MM_REG_WRITE(command, WDMA_BUF_CON1, 0x40000000, 0x40000000);
    return DP_STATUS_RETURN_SUCCESS;
}