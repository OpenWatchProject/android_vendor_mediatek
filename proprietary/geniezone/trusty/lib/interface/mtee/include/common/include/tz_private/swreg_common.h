/** GZREG_BASE macro must be synchronized  with kernel-space **/
#define GZREG_BASE 0x7E1FE000
//#define GZREG_BASE 0x7FFFE000 /* 2MB GZ kernel*/
//#define GZREG_BASE 0x65FFE000 /* X30 */
//#define GZREG_BASE 0x0012E000 /* SRAM */
#define GZREG_TOTAL_SIZE 0x1000 /* control register size */
#define GZREG_WIDTH 4
#define SYSTEM_GZREG_NUM 16
#define GZREG_MAX_NUM ((GZREG_TOTAL_SIZE) / GZREG_WIDTH - SYSTEM_GZREG_NUM)

#define DREG_BASE (GZREG_BASE + GZREG_TOTAL_SIZE)
#define DREG_TOTAL_SIZE (GZ_REG_SIZE - GZREG_TOTAL_SIZE)
