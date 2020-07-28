#ifndef __CMDQ_SEC_DDP_PLATFORM__
#define __CMDQ_SEC_DDP_PLATFORM__

/* PORTING: OVL engine involve in SVP primary/extend
 * Provide base address for later macro calculate
 */
#define DISP_OVL0_BASE			0x14008000
#define DISP_OVL1_BASE			0x14009000
#define DISP_OVL0_2L_BASE		0x14018000
#define DISP_OVL1_2L_BASE		0x14019000
#define DISP_REG_OVL_SRC_CON		0x2c	/* offset to enable/disable layer */
#define DISP_REG_OVL_L0_CON		0x30	/* offset for L0 CON */
#define DISP_REG_OVL_L1_CON		0x50	/* offset for L1 CON */
#define DISP_REG_OVL_2L_SRC_CON		0x2c	/* offset to enable/disable layer */
#define DISP_REG_OVL_2L_L0_CON		0x30	/* offset for L0 CON */
#define DISP_REG_OVL_2L_L1_CON		0x50	/* offset for L1 CON */

/* region to calculate layer from addr */
#define DISP_OVL_LAYER_REGION		DISP_REG_OVL_L1_CON - DISP_REG_OVL_L0_CON
#define DISP_OVL_2L_LAYER_REGION	DISP_REG_OVL_2L_L1_CON - DISP_REG_OVL_2L_L0_CON

/* PORTING: OVL engine used in extend output.
 * These macro called from CMDQ TA/TDRV.
 * Necessary to porting in each platform.
 *
 * in jade,
 * primary is ovl1_2l and extend is ovl1
 */
#define CMDQ_DDP_IS_OVL_EXT(addr)	(addr == DISP_OVL1_BASE)

/* macro to check if address is one of ovl engine or ovl_2l engine */
#define CMDQ_DDP_IS_OVL(addr)		((addr == DISP_OVL0_BASE) || (addr == DISP_OVL1_BASE))
#define CMDQ_DDP_IS_OVL_2L(addr)	((addr == DISP_OVL0_2L_BASE) || (addr == DISP_OVL1_2L_BASE))

/* macro to check if base address is one of OVL/OVL_2L */
#define CMDQ_DDP_IS_OVL_ENG(addr)	(CMDQ_DDP_IS_OVL(addr) || CMDQ_DDP_IS_OVL_2L(addr))

/* macro to calculate layer number from register address */
#define CMDQ_DDP_OVL_LAYER(reg)		(((reg & 0xFF) - DISP_REG_OVL_L0_CON) / DISP_OVL_LAYER_REGION)
#define CMDQ_DDP_OVL_2L_LAYER(reg)	(((reg & 0xFF) - DISP_REG_OVL_2L_L0_CON) / DISP_OVL_2L_LAYER_REGION)

/* PORTING: register use to check dislay path
 * Check if current mode is primary only or decuple mirror mode
 *
 * In Jade, decuple mode uses OVL1->WDMA,
 * thus we check DISP_WDMA_SEL_IN register in MMSYS config.
 * bit 3: from DISP_OVL1
 */
#define MMSYS_BASE_PA			0x14000000
#define DISP_WDMA_SEL_IN_OFF		0x5c
#define DISP_WDMA_SEL_IN		MMSYS_BASE_PA | DISP_WDMA_SEL_IN_OFF
#define DISP_WDMA_SEL_IN_OVL1_MASK	0x4

/* PORTING: macro to return current display is decuple or not */
#define DISPLAY_IS_DECUPLE_MODE()	(CMDQ_REG_GET32(DISP_WDMA_SEL_IN) & DISP_WDMA_SEL_IN_OVL1_MASK)

#endif

