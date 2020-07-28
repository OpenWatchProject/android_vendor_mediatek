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
        vido_dma_preultra = (88 << 12) + (68 << 0);
    }
    else if (uniqueID == 0 || uniqueID == 1) { //RGB
        vido_dma_preultra = (8 << 12) + (1 << 0);
    }
    else if (uniqueID == 2 || uniqueID == 3) { //ARGB
        vido_dma_preultra = (1 << 12) + (1 << 0);
    }
    else if (uniqueID == 4 || uniqueID == 5) { //UYVY
        vido_dma_preultra = (48 << 12) + (8 << 0);
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
    uint32_t planeCount = DP_COLOR_GET_PLANE_COUNT(colorFormat);
    uint32_t uniqueID = DP_COLOR_GET_UNIQUE_ID(colorFormat);

    uint32_t wdma_smi_con = 0;
    uint32_t wdma_buf_con1 = 0;
    uint32_t wdma_buf_con3 = 0;
    uint32_t wdma_buf_con4 = 0;
    uint32_t wdma_buf_con5 = 0;
    uint32_t wdma_buf_con6 = 0;
    uint32_t wdma_buf_con7 = 0;
    uint32_t wdma_buf_con8 = 0;
    uint32_t wdma_buf_con9 = 0;
    uint32_t wdma_buf_con10 = 0;
    uint32_t wdma_buf_con11 = 0;
    uint32_t wdma_buf_con12 = 0;
    uint32_t wdma_buf_con13 = 0;
    uint32_t wdma_buf_con14 = 0;
    uint32_t wdma_buf_con15 = 0;
    uint32_t wdma_buf_con16 = 0;
    uint32_t wdma_buf_con17 = 0;
    uint32_t wdma_buf_con18 = 0;
    uint32_t wdma_drs_con0 = 0;
    uint32_t wdma_drs_con1 = 0;
    uint32_t wdma_drs_con2 = 0;
    uint32_t wdma_drs_con3 = 0;

    wdma_smi_con |= (2 << 24) + (2 << 20) + (4 << 16) + (0 << 8) + (0 << 5) + (0 << 4) + (7 << 0);
    wdma_buf_con1 |= (0 << 31) + (1 << 30) + (0 << 28) + (116 << 0);

    if (planeCount == 3) { //3-plane
        wdma_buf_con3 |= (16 << 16) + (32 << 0);
        wdma_buf_con4 |= (16 << 0);
        wdma_buf_con5 |= (0 << 16) + (1 << 0);
        wdma_buf_con6 |= (0 << 16) + (18 << 0);
        wdma_buf_con7 |= (0 << 16) + (14 << 0);
        wdma_buf_con8 |= (0 << 16) + (19 << 0);
        wdma_buf_con9 |= (0 << 16) + (14 << 0);
        wdma_buf_con10 |= (0 << 16) + (19 << 0);
    }
    else if (planeCount == 2) { //2-plane
        wdma_buf_con3 |= (16 << 16) + (32 << 0);
        wdma_buf_con5 |= (0 << 16) + (1 << 0);
        wdma_buf_con6 |= (0 << 16) + (18 << 0);
        wdma_buf_con7 |= (0 << 16) + (28 << 0);
        wdma_buf_con8 |= (0 << 16) + (38 << 0);
    }
    else if (uniqueID == 0 || uniqueID == 1) { //RGB
        wdma_buf_con3 |= (0 << 16) + (64 << 0);
        wdma_buf_con5 |= (0 << 16) + (1 << 0);
        wdma_buf_con6 |= (0 << 16) + (1 << 0);
    }
    else if (uniqueID == 2 || uniqueID == 3) { //ARGB
        wdma_buf_con3 |= (0 << 16) + (64 << 0);
        wdma_buf_con5 |= (0 << 16) + (1 << 0);
        wdma_buf_con6 |= (0 << 16) + (1 << 0);
    }
    else if (uniqueID == 4 || uniqueID == 5) { //UYVY
        wdma_buf_con3 |= (0 << 16) + (64 << 0);
        wdma_buf_con5 |= (0 << 16) + (1 << 0);
        wdma_buf_con6 |= (0 << 16) + (36 << 0);
    }
    else if (uniqueID == 7) { //Y8
        wdma_buf_con3 |= (0 << 16) + (64 << 0);
        wdma_buf_con5 |= (0 << 16) + (56 << 0);
        wdma_buf_con6 |= (0 << 16) + (76 << 0);
    }
    else {
        return DP_STATUS_RETURN_SUCCESS;
    }

    MM_REG_WRITE(command, WDMA_SMI_CON, wdma_smi_con, 0x0FFFFFFF);
    MM_REG_WRITE(command, WDMA_BUF_CON1, wdma_buf_con1, 0xF80003FF);
    //MM_REG_WRITE(command, WDMA_BUF_CON3, wdma_buf_con3, 0x01FF01FF); //it may lead to WDMA FIFO full issue
    //MM_REG_WRITE(command, WDMA_BUF_CON4, wdma_buf_con4, 0x01FF); //it may lead to WDMA FIFO full issue
    MM_REG_WRITE(command, WDMA_BUF_CON5, wdma_buf_con5, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON6, wdma_buf_con6, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON7, wdma_buf_con7, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON8, wdma_buf_con8, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON9, wdma_buf_con9, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON10, wdma_buf_con10, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON11, wdma_buf_con11, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON12, wdma_buf_con12, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON13, wdma_buf_con13, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON14, wdma_buf_con14, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON15, wdma_buf_con15, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON16, wdma_buf_con16, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_BUF_CON17, wdma_buf_con17, 0x03FF0001);
    MM_REG_WRITE(command, WDMA_BUF_CON18, wdma_buf_con18, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_DRS_CON0, wdma_drs_con0, 0x03FF0001);
    MM_REG_WRITE(command, WDMA_DRS_CON1, wdma_drs_con1, 0x03FF03FF);
    MM_REG_WRITE(command, WDMA_DRS_CON2, wdma_drs_con2, 0xD1FF0000);
    MM_REG_WRITE(command, WDMA_DRS_CON3, wdma_drs_con3, 0x03FF03FF);

    return DP_STATUS_RETURN_SUCCESS;
}