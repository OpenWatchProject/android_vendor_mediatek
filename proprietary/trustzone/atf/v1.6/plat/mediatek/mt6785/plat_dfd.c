#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <mtk_plat_common.h>
#include <platform_def.h>
#include <console.h>

#define readl(addr)		mmio_read_32((addr))
#define writel(addr, val)	mmio_write_32((addr), (val))
#define sync_writel(addr, val)	do { mmio_write_32((addr), (val)); dsbsy(); } while (0)

#define MCU_BIU_BASE				(0x0c530000)
#define MISC1_CFG_BASE				(0xa040)
#define DFD_INTERNAL_CTL			(MCU_BIU_BASE+MISC1_CFG_BASE+0x00)
#define DFD_INTERNAL_PWR_ON			(MCU_BIU_BASE+MISC1_CFG_BASE+0x08)
#define DFD_CHAIN_LENGTH0			(MCU_BIU_BASE+MISC1_CFG_BASE+0x0c)
#define DFD_INTERNAL_SHIFT_CLK_RATIO		(MCU_BIU_BASE+MISC1_CFG_BASE+0x10)
#define DFD_INTERNAL_TEST_SO_0			(MCU_BIU_BASE+MISC1_CFG_BASE+0x28)
#define DFD_INTERNAL_NUM_OF_TEST_SO_GROUP	(MCU_BIU_BASE+MISC1_CFG_BASE+0x30)
#define DFD_V30_CTL				(MCU_BIU_BASE+MISC1_CFG_BASE+0x48)
#define DFD_V30_BASE_ADDR			(MCU_BIU_BASE+MISC1_CFG_BASE+0x4c)
#define DFD_TEST_SI_0				(MCU_BIU_BASE+MISC1_CFG_BASE+0x58)
#define DFD_TEST_SI_1				(MCU_BIU_BASE+MISC1_CFG_BASE+0x5c)
#define DFD_HW_TRIGGER_MASK                     (MCU_BIU_BASE+MISC1_CFG_BASE+0xbc)

#define DFD_V35_ENALBE                          (MCU_BIU_BASE+0xa0a8)
#define DFD_V35_TAP_NUMBER                      (MCU_BIU_BASE+0xa0ac)
#define DFD_V35_TAP_EN                          (MCU_BIU_BASE+0xa0b0)
#define DFD_V35_SEQ0_0                          (MCU_BIU_BASE+0xa0c0)
#define DFD_V35_SEQ0_1                          (MCU_BIU_BASE+0xa0c4)

#define DFD_CACHE_DUMP_ENABLE   1
#define DFD_PARITY_ERR_TRIGGER  2

static unsigned long dfd_enabled;
static unsigned long dfd_base_addr;
static unsigned long dfd_chain_length;
static unsigned long dfd_cache_dump;

static void dfd_setup(unsigned long base_addr, unsigned long chain_length, unsigned long cache_dump)
{
	/* bit[0] : rg_rw_dfd_internal_dump_en -> 1 */
	/* bit[2] : rg_rw_dfd_clock_stop_en -> 1 */
	sync_writel(DFD_INTERNAL_CTL, 0x5);

	/* bit[13] : xreset_b_update_disable */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 13));

	/*
	 * bit[10:3] : DFD trigger selection mask :
	 * bit[3] : rg_rw_dfd_trigger_sel[0] : 1'b1 -> enable wdt trigger
	 * bit[4] : rg_rw_dfd_trigger_sel[1] : 1'b1 -> enable HW trigger
	 * bit[5] : rg_rw_dfd_trigger_sel[2] : 1'b1 -> enable SW trigger
	 * bit[6] : rg_rw_dfd_trigger_sel[3] : 1'b1 -> enable SW non-security trigger
	 * bit[7] : rg_rw_dfd_trigger_sel[4] : 1'b1 -> enable timer trigger
	 */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 3));

	/* Bit[20:19] : rg_dfd_armpll_div_mux_sel switch to PLL2 for DFD */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x3 << 19));

	/*
	 * bit[0] : rg_rw_dfd_auto_power_on -> 1
	 * bit[2:1] : rg_rw_dfd_auto_power_on_dely -> 1 (10us)
	 * bit[4:2] : rg_rw_dfd_power_on_wait_time -> 1 (20us)
	 */
	writel(DFD_INTERNAL_PWR_ON, 0xb);

	/* longest scan chain length */
	writel(DFD_CHAIN_LENGTH0, chain_length);

	/*
	 * bit[1:0] : rg_rw_dfd_shift_clock_ratio
	 * 2’b00 -> 1:2
	 */
	writel(DFD_INTERNAL_SHIFT_CLK_RATIO, 0x0);

	/* total 59 test_so */
	writel(DFD_INTERNAL_TEST_SO_0, 0x3b);

	/* Only one group of test_so */
	writel(DFD_INTERNAL_NUM_OF_TEST_SO_GROUP, 0x1);

	/* For Big circular buffer read failed after reboot */
	writel(DFD_TEST_SI_0, 0x108);

	/* For Big circular buffer icache entry 255 write err */
	writel(DFD_TEST_SI_1, 0x20200000);

	/* for DFD-3.0 setup */
	sync_writel(DFD_V30_CTL, 0x1);

	/* set base address */
	writel(DFD_V30_BASE_ADDR, (base_addr & 0xfff00000));

	/* setup global variables for suspend/resume */
	dfd_enabled = 1;
	dfd_base_addr = base_addr;
	dfd_chain_length = chain_length;
	dfd_cache_dump = cache_dump;

	if (cache_dump & DFD_CACHE_DUMP_ENABLE) {
		sync_writel(DFD_V35_ENALBE, 0x1);
		sync_writel(DFD_V35_TAP_NUMBER, 0xA);
		sync_writel(DFD_V35_TAP_EN, 0x3FF);
		sync_writel(DFD_V35_SEQ0_0, 0x63668820);

		if (cache_dump & DFD_PARITY_ERR_TRIGGER) {
			sync_writel(DFD_HW_TRIGGER_MASK, 0xc);
			writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 4));
		}
	}

	dsbsy();
}

void dfd_resume(void)
{
	if (dfd_enabled)
		dfd_setup(dfd_base_addr, dfd_chain_length, dfd_cache_dump);
}

int dfd_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
	int ret = 0;

	switch (arg0) {
	case PLAT_MTK_DFD_SETUP_MAGIC:
		dfd_setup(arg1, arg2, arg3);
		break;
	case PLAT_MTK_DFD_READ_MAGIC:
		/* only allow access to DFD ctrl register base + 0x200 */
		if (arg1 <= 0x200)
			ret = readl(MCU_BIU_BASE+MISC1_CFG_BASE+arg1);
		else
			ret = 0;
		break;
	case PLAT_MTK_DFD_WRITE_MAGIC:
		/* only allow access to DFD ctrl register base + 0x200 */
		if (arg1 <= 0x200)
			sync_writel(MCU_BIU_BASE+MISC1_CFG_BASE+arg1, arg2);

		break;
	default:
		ret = -1;
		break;
	}

	return ret;
}


