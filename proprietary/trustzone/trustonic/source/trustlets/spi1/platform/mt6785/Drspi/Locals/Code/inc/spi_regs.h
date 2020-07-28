#ifndef __SPI_TZ_REG_REG_H__
#define __SPI_TZ_REG_REG_H__

/*
 * secure dirver map region (via drApiMapPhys)
 * 1. t-driver will own virtual address
 * 2. SPI used virtual address range is 0x80000 - 0xFEFFF
 */
extern uint8_t *SpiBaseVA;
extern uint8_t  *SpiTxDmaVA;
extern uint8_t  *SpiRxDmaVA;
extern uint8_t  *SpiPdnBaseVA;
extern uint8_t  *SpiCfgBaseVA;

//#define SPI_REG_VA_BASE      (0x00080000)
//#define SPI_TX_DMA_VA_BASE   (0x00800000)
//#define SPI_RX_DMA_VA_BASE   (0x00A00000)

#define SPI_REG_VA_BASE      (SpiBaseVA)
#define SPI_TX_DMA_VA_BASE   (SpiTxDmaVA)
#define SPI_RX_DMA_VA_BASE   (SpiRxDmaVA)
#define SPI_PDN_VA_BASE      (SpiPdnBaseVA)
#define SPI_CFG_VA_BASE      (SpiCfgBaseVA)

#define SPI_REG_CFG0         (SPI_REG_VA_BASE + 0x00)
#define SPI_REG_CFG1         (SPI_REG_VA_BASE + 0x04)
#define SPI_REG_TX_SRC       (SPI_REG_VA_BASE + 0x08)
#define SPI_REG_RX_DST       (SPI_REG_VA_BASE + 0x0C)
#define SPI_REG_TX_DATA      (SPI_REG_VA_BASE + 0x10)
#define SPI_REG_RX_DATA      (SPI_REG_VA_BASE + 0x14)
#define SPI_REG_CMD          (SPI_REG_VA_BASE + 0x18)
#define SPI_REG_STATUS0      (SPI_REG_VA_BASE + 0x1C)
#define SPI_REG_STATUS1      (SPI_REG_VA_BASE + 0x20)
#define SPI_REG_PAD_SEL      (SPI_REG_VA_BASE + 0x24)
#define SPI_REG_CFG2         (SPI_REG_VA_BASE + 0x28)
#define SPI_REG_TX_SRC_64    (SPI_REG_VA_BASE + 0x2C)
#define SPI_REG_RX_DST_64    (SPI_REG_VA_BASE + 0x30)

#define SPI_REG_VA_BASE_END  (SPI_REG_VA_BASE + 0x028)

#define SPI_REG_PA_BASE       (0x11010000)
#define SPI_PDN_PA_BASE       (0x10003000)
#define SPI_CFG_PA_BASE       (0x10000000)

#define SPI_PDN_SET      (SPI_PDN_VA_BASE + 0x08)
#define SPI_PDN_CLR      (SPI_PDN_VA_BASE + 0x10)
#define SPI_PDN_STA      (SPI_PDN_VA_BASE + 0x18)

#define SPI_CFG_SET      (SPI_CFG_VA_BASE + 0x64)
#define SPI_CFG_CLR      (SPI_CFG_VA_BASE + 0x68)
#define SPI_CFG_STA      (SPI_CFG_VA_BASE + 0x60)

#define SPI_REG_PA_BASE_END  (SPI_REG_PA_BASE + 0x028)

/************************************************************************/
#define SPI_CFG0_SCK_HIGH_OFFSET          0
#define SPI_CFG0_SCK_LOW_OFFSET           16
#define SPI_CFG0_CS_HOLD_OFFSET           0
#define SPI_CFG0_CS_SETUP_OFFSET          16

#define SPI_CFG0_SCK_HIGH_MASK            0xffff
#define SPI_CFG0_SCK_LOW_MASK             0xffff0000
#define SPI_CFG0_CS_HOLD_MASK             0xffff
#define SPI_CFG0_CS_SETUP_MASK            0xffff0000

#define SPI_CFG1_CS_IDLE_OFFSET           0
#define SPI_CFG1_PACKET_LOOP_OFFSET       8
#define SPI_CFG1_PACKET_LENGTH_OFFSET     16
#define SPI_CFG1_GET_TICK_DLY_OFFSET      29

#define SPI_CFG1_CS_IDLE_MASK             0xff
#define SPI_CFG1_PACKET_LOOP_MASK         0xff00
#define SPI_CFG1_PACKET_LENGTH_MASK       0x3fff0000
#define SPI_CFG1_GET_TICK_DLY_MASK        0xe0000000

#define SPI_CMD_ACT_OFFSET                0
#define SPI_CMD_RESUME_OFFSET             1
#define SPI_CMD_RST_OFFSET                2
#define SPI_CMD_PAUSE_EN_OFFSET           4
#define SPI_CMD_DEASSERT_OFFSET           5
#define SPI_CMD_SAMPLE_SEL_OFFSET         6
#define SPI_CMD_CS_POL_OFFSET             7
#define SPI_CMD_CPHA_OFFSET               8
#define SPI_CMD_CPOL_OFFSET               9
#define SPI_CMD_RX_DMA_OFFSET             10
#define SPI_CMD_TX_DMA_OFFSET             11
#define SPI_CMD_TXMSBF_OFFSET             12
#define SPI_CMD_RXMSBF_OFFSET             13
#define SPI_CMD_RX_ENDIAN_OFFSET          14
#define SPI_CMD_TX_ENDIAN_OFFSET          15
#define SPI_CMD_FINISH_IE_OFFSET          16
#define SPI_CMD_PAUSE_IE_OFFSET           17

#define SPI_CMD_ACT_MASK                  0x1
#define SPI_CMD_RESUME_MASK               0x2
#define SPI_CMD_RST_MASK                  0x4
#define SPI_CMD_PAUSE_EN_MASK             0x10
#define SPI_CMD_DEASSERT_MASK             0x20
#define SPI_CMD_CPHA_MASK                 0x100
#define SPI_CMD_CPOL_MASK                 0x200
#define SPI_CMD_RX_DMA_MASK               0x400
#define SPI_CMD_TX_DMA_MASK               0x800
#define SPI_CMD_TXMSBF_MASK               0x1000
#define SPI_CMD_RXMSBF_MASK               0x2000
#define SPI_CMD_RX_ENDIAN_MASK            0x4000
#define SPI_CMD_TX_ENDIAN_MASK            0x8000
#define SPI_CMD_FINISH_IE_MASK            0x10000
#define SPI_CMD_PAUSE_IE_MASK             0x20000

#define SPI_ULTRA_HIGH_EN_OFFSET          0
#define SPI_ULTRA_HIGH_THRESH_OFFSET      16

#define SPI_ULTRA_HIGH_EN_MASK            0x1
#define SPI_ULTRA_HIGH_THRESH_MASK        0xffff0000


// REG API
#define SPI_READ(addr)                  (*(volatile uint32_t*)(addr))
#define SPI_WRITE(addr, val)            (*(volatile uint32_t*)(addr) = val)

#define SPI_READ8(addr)                 (*(volatile uint8_t*)(addr))
#define SPI_WRITE8(addr, val)           (*(volatile uint8_t*)(addr) = val)


#endif // __SPI_TZ_REG_REG_H__

