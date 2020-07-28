#ifndef _EMI_DRV_H_
#define _EMI_DRV_H_

/* EMI Memory Protect Unit */
#define EMI_MPU_CTRL                (EMI_MPU_BASE + 0x0)
#define EMI_MPU_DBG                 (EMI_MPU_BASE + 0x4)
#define EMI_MPU_SA0                 (EMI_MPU_BASE + 0x100)
#define EMI_MPU_EA0                 (EMI_MPU_BASE + 0x200)
#define EMI_MPU_SA(region)          (EMI_MPU_SA0 + (region*4))
#define EMI_MPU_EA(region)          (EMI_MPU_EA0 + (region*4))
#define EMI_MPU_APC0                (EMI_MPU_BASE + 0x300)
#define EMI_MPU_APC(region,domain)  (EMI_MPU_APC0 + (region*4) +((domain/8)*0x100))
#define EMI_MPU_CTRL_D0             (EMI_MPU_BASE + 0x800)
#define EMI_MPU_CTRL_D(domain)      (EMI_MPU_CTRL_D0 + (domain*4))
#define EMI_RG_MASK_D0              (EMI_MPU_BASE + 0x900)
#define EMI_RG_MASK_D(domain)       (EMI_RG_MASK_D0 + (domain*4))
#define EMI_MPU_START               (0x0)
#define EMI_MPU_END                 (0x91C)

/* EMI memory protection align 64K */
#define EMI_MPU_ALIGNMENT   0x10000

#define NO_PROTECTION       0
#define SEC_RW              1
#define SEC_RW_NSEC_R       2
#define SEC_RW_NSEC_W       3
#define SEC_R_NSEC_R        4
#define FORBIDDEN           5

#define LOCK                1
#define UNLOCK              0
#define EMIMPU_DX_SEC_BIT  30

#define EMIMPU_DOMAIN_NUM   8
#define EMIMPU_REGION_NUM  24

extern uint32_t sip_emimpu_set_region_protection(unsigned long long start, unsigned long long end, unsigned int region_permission);
extern uint64_t sip_emimpu_write(unsigned int offset, unsigned int reg_value);
extern uint32_t sip_emimpu_read(unsigned int offset);
extern void emimpu_setup(void);

#endif
