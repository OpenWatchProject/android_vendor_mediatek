#include <stdio.h>
#include <console.h>
#include <debug.h>
#include <mtk_plat_common.h>
#include <plat_private.h>
#include <platform_def.h>
#include "fde_aes.h"

#define CRYPTO_HW_TZCC
#ifdef CRYPTO_HW_TZCC /* Use TZCC */
    #define sha256 TZCC_SHA256
    extern unsigned int TZCC_SHA256(const unsigned char *in, unsigned int inlen, unsigned char *hash);
#endif

extern struct atf_arg_t gteearg;

s32 fde_aes_dump_reg(void)
{
	tf_printf(" ################################ FDE_AES DUMP START (ATF) ################################\n");
	//tf_printf("0x1036084C   %x = %x\n", (FDE_AES_CORE_BASE + 0x84C), FDE_READ32(FDE_AES_CORE_BASE + 0x84C));

	tf_printf("DXFDE_CLOCK_AES_SEL  %x = %x\n", DXFDE_CLOCK_AES_SEL, FDE_READ32(DXFDE_CLOCK_AES_SEL));

	tf_printf("DXFDE_COREK_CG_CLR  %x = %x\n", DXFDE_COREK_CG_CLR, FDE_READ32(DXFDE_COREK_CG_CLR));
	tf_printf("INFRA_FDE_AES_SI_CTL  %x = %x\n", INFRA_FDE_AES_SI_CTL, FDE_READ32(INFRA_FDE_AES_SI_CTL));
	tf_printf("INFRA_FDE_AES_SI_CTL2 %x = %x\n", INFRA_FDE_AES_SI_CTL2, FDE_READ32(INFRA_FDE_AES_SI_CTL2));

	tf_printf("CONTEXT_SEL   %x = %x\n", CONTEXT_SEL, FDE_READ32(CONTEXT_SEL));
	tf_printf("CONTEXT_WORD0 %x = %x\n", CONTEXT_WORD0, FDE_READ32(CONTEXT_WORD0));
	tf_printf("CONTEXT_WORD1 %x = %x\n", CONTEXT_WORD1, FDE_READ32(CONTEXT_WORD1));
	tf_printf("CONTEXT_WORD3 %x = %x\n", CONTEXT_WORD3, FDE_READ32(CONTEXT_WORD3));

	tf_printf("REG_COM_A  %x = %x\n", REG_COM_A, FDE_READ32(REG_COM_A));
	tf_printf("REG_COM_B  %x = %x\n", REG_COM_B, FDE_READ32(REG_COM_B));
	tf_printf("REG_COM_C  %x = %x\n", REG_COM_C, FDE_READ32(REG_COM_C));

	tf_printf("REG_COM_D %x = %x\n", REG_COM_D, FDE_READ32(REG_COM_D));
	tf_printf("REG_COM_E %x = %x\n", REG_COM_E, FDE_READ32(REG_COM_E));
	tf_printf("REG_COM_F %x = %x\n", REG_COM_F, FDE_READ32(REG_COM_F));
	tf_printf("REG_COM_G %x = %x\n", REG_COM_G, FDE_READ32(REG_COM_G));
	tf_printf("REG_COM_H %x = %x\n", REG_COM_H, FDE_READ32(REG_COM_H));
	tf_printf("REG_COM_I %x = %x\n", REG_COM_I, FDE_READ32(REG_COM_I));
	tf_printf("REG_COM_J %x = %x\n", REG_COM_J, FDE_READ32(REG_COM_J));
	tf_printf("REG_COM_K %x = %x\n", REG_COM_K, FDE_READ32(REG_COM_K));

	tf_printf("REG_COM_L %x = %x\n", REG_COM_L, FDE_READ32(REG_COM_L));
	tf_printf("REG_COM_M %x = %x\n", REG_COM_M, FDE_READ32(REG_COM_M));
	tf_printf("REG_COM_N %x = %x\n", REG_COM_N, FDE_READ32(REG_COM_N));
	tf_printf("REG_COM_O %x = %x\n", REG_COM_O, FDE_READ32(REG_COM_O));
	tf_printf("REG_COM_P %x = %x\n", REG_COM_P, FDE_READ32(REG_COM_P));
	tf_printf("REG_COM_Q %x = %x\n", REG_COM_Q, FDE_READ32(REG_COM_Q));
	tf_printf("REG_COM_R %x = %x\n", REG_COM_R, FDE_READ32(REG_COM_R));
	tf_printf("REG_COM_S %x = %x\n", REG_COM_S, FDE_READ32(REG_COM_S));

	tf_printf("REG_COM_T %x = %x\n", REG_COM_T, FDE_READ32(REG_COM_T));
	tf_printf("REG_COM_U %x = %x\n", REG_COM_U, FDE_READ32(REG_COM_U));

	tf_printf("DXFDE_COREK_CG_STA %x = %x\n", DXFDE_COREK_CG_STA, FDE_READ32(DXFDE_COREK_CG_STA));

	tf_printf(" ################################ FDE_AES DUMP END ################################\n");

	return 0;
}

s32 fde_aes_enable_fde(U8 bEnable)
{
	static s32 bStatus = 0;
	static u32 fde_iv[8] = {0}; // 256-bit
	u32 i;

	if(bEnable & 0xfe)
		return -1;

	tf_printf("%s set %s current %s(ATF)\n", __FUNCTION__, bEnable?"Enable":"Disable", bStatus?"Enable":"Disable");



	if(bEnable && (!bStatus))	// Enable
	{
        // Wrapper
		FDE_WRITE32(DXFDE_COREK_CG_CLR, DXFDE_COREK_CG);

	    // FREQ
	    //GP_WRITE32(DXFDE_CLOCK_AES_SEL, DXFDE_CLOCK_AES_546);
	    GP_WRITE32(DXFDE_CLOCK_AES_SEL, DXFDE_CLOCK_AES_364);

		FDE_WRITE32(INFRA_FDE_AES_SI_CTL,  0x7FFFE7DB);
		FDE_WRITE32(INFRA_FDE_AES_SI_CTL2, 0xFFFFFFFF);

		// ESSIV
		sha256((unsigned char*)gteearg.msg_fde_key, 16, (unsigned char*)fde_iv);
		FDE_WRITE32(REG_COM_U, 0x5); // Check to set this register

		#if FDE_AES_DBG /* FDE KEY and IV DUMP */
		tf_printf("FDEKEY[128] : 0x%x 0x%x 0x%x 0x%x\n",
                gteearg.msg_fde_key[0], gteearg.msg_fde_key[1], gteearg.msg_fde_key[2], gteearg.msg_fde_key[3]);
		tf_printf("FDEIV[256] : 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                fde_iv[0], fde_iv[1], fde_iv[2], fde_iv[3],
                fde_iv[4], fde_iv[5], fde_iv[6], fde_iv[7]);
		#endif /* FDE KEY and IV DUMP */

		// Slot
		for(i = 0; i < 2; i++)
		{
			FDE_WRITE32(REG_COM_A, i);
			FDE_WRITE32(REG_COM_D, gteearg.msg_fde_key[0]);
			FDE_WRITE32(REG_COM_E, gteearg.msg_fde_key[1]);
			FDE_WRITE32(REG_COM_F, gteearg.msg_fde_key[2]);
			FDE_WRITE32(REG_COM_G, gteearg.msg_fde_key[3]);
			FDE_WRITE32(REG_COM_L, fde_iv[0]);
			FDE_WRITE32(REG_COM_M, fde_iv[1]);
			FDE_WRITE32(REG_COM_N, fde_iv[2]);
			FDE_WRITE32(REG_COM_O, fde_iv[3]);
			FDE_WRITE32(REG_COM_P, fde_iv[4]);
			FDE_WRITE32(REG_COM_Q, fde_iv[5]);
			FDE_WRITE32(REG_COM_R, fde_iv[6]);
			FDE_WRITE32(REG_COM_S, fde_iv[7]);
			FDE_WRITE32(REG_COM_T, 0x00000004);
		}
		bStatus = 1;
		#if FDE_AES_DBG
		fde_aes_dump_reg();
		#endif
	}

	if((!bEnable) && bStatus)		// Disable
	{
		FDE_WRITE32(INFRA_FDE_AES_SI_CTL,  0x0);
		FDE_WRITE32(INFRA_FDE_AES_SI_CTL2, 0x0);
		#if FDE_AES_DBG
		fde_aes_dump_reg();
		#endif
		FDE_WRITE32(DXFDE_COREK_CG_SET, DXFDE_COREK_CG);
		bStatus = 0;
	}

	return 0;
}


s32 fde_aes_action(unsigned int enable, unsigned int dump)
{
	s32 ret = -1;

	console_init(gteearg.atf_log_port, UART_CLOCK, UART_BAUDRATE);

	if (enable == 0xb){
		ret = fde_aes_enable_fde(0);
	}
	else if (enable == 0xa){
		ret = fde_aes_enable_fde(1);
	}

	if(dump == 0xa) {
	    ret = fde_aes_dump_reg();
	}

	console_uninit();

	return ret;
}

