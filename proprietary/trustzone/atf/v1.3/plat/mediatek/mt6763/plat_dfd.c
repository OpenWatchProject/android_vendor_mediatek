#include <arch_helpers.h>
#include <debug.h>
#include <mmio.h>
#include <platform_def.h>

#define readl(addr)		mmio_read_32((addr))
#define writel(addr, val)	mmio_write_32((addr), (val))
#define sync_writel(addr, val)	do { mmio_write_32((addr), (val)); dsbsy(); } while (0)

#define MCU_BIU_BASE				(0x0c530000)
#define MISC1_CFG_BASE				(0xb00)
#define DFD_INTERNAL_CTL			(MCU_BIU_BASE+MISC1_CFG_BASE+0x00)
#define DFD_INTERNAL_PWR_ON			(MCU_BIU_BASE+MISC1_CFG_BASE+0x08)
#define DFD_CHAIN_LENGTH0			(MCU_BIU_BASE+MISC1_CFG_BASE+0x0c)
#define DFD_INTERNAL_SHIFT_CLK_RATIO		(MCU_BIU_BASE+MISC1_CFG_BASE+0x10)
#define DFD_INTERNAL_TEST_SO_0			(MCU_BIU_BASE+MISC1_CFG_BASE+0x28)
#define DFD_INTERNAL_NUM_OF_TEST_SO_GROUP	(MCU_BIU_BASE+MISC1_CFG_BASE+0x30)
#define DFD_V30_CTL				(MCU_BIU_BASE+MISC1_CFG_BASE+0x100)
#define DFD_V30_BASE_ADDR			(MCU_BIU_BASE+MISC1_CFG_BASE+0x104)

static unsigned long dfd_enabled;
static unsigned long dfd_base_addr;
static unsigned long dfd_chain_length;

static void dfd_setup(unsigned long base_addr, unsigned long chain_length)
{
	/* bit[0] : rg_rw_dfd_internal_dump_en -> 1 */
	/* bit[2] : rg_rw_dfd_clock_stop_en -> 1 */
	sync_writel(DFD_INTERNAL_CTL, 0x5);

	/*
	 * bit[6:3] : rg_rw_dfd_trigger_sel
	 * 4’b0000 -> JTAG trigger
	 * 4’b0001 -> WD trigger
	 * 4’b0011 -> SW trigger
	 */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 3));

	/* bit[14] : early scan mode */
	writel(DFD_INTERNAL_CTL, readl(DFD_INTERNAL_CTL)|(0x1 << 14));

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

	/* total 32 test_so */
	writel(DFD_INTERNAL_TEST_SO_0, 0x20);

	/* Only one group of test_so */
	writel(DFD_INTERNAL_NUM_OF_TEST_SO_GROUP, 0x1);

	/* for DFD-3.0 setup */
	sync_writel(DFD_V30_CTL, 0x1);

	/* set base address */
	writel(DFD_V30_BASE_ADDR, (base_addr & 0xfff00000));

	/* setup global variables for suspend/resume */
	dfd_enabled = 1;
	dfd_base_addr = base_addr;
	dfd_chain_length = chain_length;

	dsbsy();
}

void dfd_resume(void)
{
	if (dfd_enabled)
		dfd_setup(dfd_base_addr, dfd_chain_length);
}

int dfd_smc_dispatcher(unsigned long arg0, unsigned long arg1, unsigned long arg2)
{
	int ret = 0;

	switch (arg0) {
		case PLAT_MTK_DFD_SETUP_MAGIC:
			dfd_setup(arg1, arg2);
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
			if (arg1 <= 0x200) {
				sync_writel(MCU_BIU_BASE+MISC1_CFG_BASE+arg1, arg2);
			}
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}


