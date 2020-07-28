#ifndef __MT_FDE_AES_H__
#define __MT_FDE_AES_H__

#include "typedefs.h"
//#include "reg_base.H"
#define PERI_CFG_BASE (0x11010000)
#define INFRA_CFG_CKSYS_BASE (0x10210000)
#define FDE_AES_CORE_BASE (0x11B40000)

#define FDE_AES_DBG			0

//#define CONFIG_MT_FDE_FPGA_ENABLE

#define SCP_CLK_CG_CTRL	(SCP_CLK_CTRL_BASE + 0x30)
#define FDE_MCLK_CG_BIT	(1 << 3)
#define FDE_BCLK_CG_BIT	(1 << 4)

#define FDE_OK                              0x0000
#define EAGAIN_FDE                          11  /* Try again */
#define EINVAL_FDE                          22  /* Invalid argument */
#define EOPNOTSUPP_FDE                      95  /* Operation not supported on transport endpoint */
#define ETIMEDOUT_FDE                       110 /* Connection timed out */
#define EREMOTEIO_FDE                       121 /* Remote I/O error */
#define ENOTSUPP_FDE                        524 /* Remote I/O error */
#define FDE_WRITE_FAIL_HS_NACKERR           0xA013
#define FDE_WRITE_FAIL_ACKERR               0xA014
#define FDE_WRITE_FAIL_TIMEOUT              0xA015

//#define FDE_CLK_WRAPPER_RATE    36000   /* kHz for wrapper FDE work frequency */

/******************************************register operation***********************************/
// REGISTER
#define	CONTEXT_SEL			(FDE_AES_CORE_BASE + 0x000)
#define	CONTEXT_WORD0		(FDE_AES_CORE_BASE + 0x004)
#define	CONTEXT_WORD1		(FDE_AES_CORE_BASE + 0x008)
#define	CONTEXT_WORD3		(FDE_AES_CORE_BASE + 0x00C)
#define	REG_COM_A			(FDE_AES_CORE_BASE + 0x020)
#define	REG_COM_B			(FDE_AES_CORE_BASE + 0x024)
#define	REG_COM_C			(FDE_AES_CORE_BASE + 0x028)
#define	REG_COM_D			(FDE_AES_CORE_BASE + 0x100)
#define	REG_COM_E			(FDE_AES_CORE_BASE + 0x104)
#define	REG_COM_F			(FDE_AES_CORE_BASE + 0x108)
#define	REG_COM_G			(FDE_AES_CORE_BASE + 0x10C)
#define	REG_COM_H			(FDE_AES_CORE_BASE + 0x110)
#define	REG_COM_I			(FDE_AES_CORE_BASE + 0x114)
#define	REG_COM_J			(FDE_AES_CORE_BASE + 0x118)
#define	REG_COM_K			(FDE_AES_CORE_BASE + 0x11C)
#define	REG_COM_L			(FDE_AES_CORE_BASE + 0x200)
#define	REG_COM_M			(FDE_AES_CORE_BASE + 0x204)
#define	REG_COM_N			(FDE_AES_CORE_BASE + 0x208)
#define	REG_COM_O			(FDE_AES_CORE_BASE + 0x20C)
#define	REG_COM_P			(FDE_AES_CORE_BASE + 0x210)
#define	REG_COM_Q			(FDE_AES_CORE_BASE + 0x214)
#define	REG_COM_R			(FDE_AES_CORE_BASE + 0x218)
#define	REG_COM_S			(FDE_AES_CORE_BASE + 0x21C)
#define	REG_COM_T			(FDE_AES_CORE_BASE + 0x300)
#define	REG_COM_U			(FDE_AES_CORE_BASE + 0x400)

// PERI
#define	DXFDE_COREK_CG_SET		(PERI_CFG_BASE + 0x2B0)
#define	DXFDE_COREK_CG_CLR		(PERI_CFG_BASE + 0x2B4)
#define	DXFDE_COREK_CG_STA		(PERI_CFG_BASE + 0x2B8)
#define DXFDE_COREK_CG			(1 << 24)

#define	DXFDE_CLOCK_AES_SEL    (INFRA_CFG_CKSYS_BASE+0x180)
#define DXFDE_CLOCK_AES_416     (0x4 << 24)
#define DXFDE_CLOCK_AES_364     (0x3 << 24)
#define DXFDE_CLOCK_AES_546     (0x1 << 24)

// WRAPPER
#define	INFRA_FDE_AES_SI_CTL	(PERI_CFG_BASE + 0x11C)
#define	INFRA_FDE_AES_SI_CTL2	(PERI_CFG_BASE + 0x120)

#define FDE_READ32(addr)        *((volatile u32*)(addr))
#define FDE_WRITE32(addr, val) (*((volatile u32*)(addr)) = (val))

#define GP_WRITE32(addr, val) (*((volatile u32*)(addr)) |= (val))

#define FDE_READ16(addr)        *((volatile u16*)(addr))
#define FDE_WRITE16(addr, val) (*((volatile u16*)(addr)) = (val))

#define FDE_READ8(addr)         *((volatile u8*)(addr))
#define FDE_WRITE8(addr, val)  (*((volatile u8*)(addr)) = (val))
/***********************************end of register operation****************************************/

/***********************************FDE Param********************************************************/
typedef struct mt_fde_aes_t {
	/* CONTEXT_SEL 0x0 */
	U8		context_id;
	U8		bypass; // 1:enable/ 0:disable (Sansa)
	U8		key_slot_id; // Max 32 set key slot.
	U8		sector_size; // 1: 512 Bytes/ 2: 1024 Bytes/ 3: 2048 Bytes/ 4: 4096 Bytes
	/* CONTEXT_WORD1 0x8 */
	U32		sector_offset_L;
	U32		sector_offset_H;
}mt_fde_aes_context;
//==============================================================================
// FDE_AES Exported Function
//==============================================================================
s32 fde_aes_dump_reg(void);
s32 fde_aes_enable_fde(U8 benable);
s32 fde_aes_action(unsigned int enable, unsigned int dump);

#endif /* __MT_FDE_AES_H__ */
