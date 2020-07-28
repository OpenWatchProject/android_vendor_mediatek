/*
 * Copyright (c) 2014 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>
#include <tbase_private.h>

extern void plat_tbase_fiqforward_init(void);

// SPSR for tbase setup
#define TBASE_INIT_SPSR 0x1d3

// SPSR for entry to tbase
#define TBASE_ENTRY_SPSR 0x1d1

// Maximum pages for monitor - tbase communication
#define TBASE_INTERFACE_PAGES 20
// MMU table entries for area where registers are stored.
uint64_t registerFileL2[TBASE_INTERFACE_PAGES];

// Page aligned start addresses for memory windows between tbase and monitor
uint64_t registerFileStart[REGISTER_FILE_COUNT];
uint64_t registerFileEnd[REGISTER_FILE_COUNT];

// Initilization parameters for tbase.
static bootCfg_t tbaseBootCfg;


// Entry vector start address in tbase
uint64_t tbaseEntryBase;
// Tbase SPSR for SMC and FIQ handling.
uint32_t tbaseEntrySpsr;

// SP bit width; For time begin 64bit not supported
uint64_t swd32Bit = 1; 

// Current status of initialization
uint64_t tbaseInitStatus = TBASE_INIT_NONE;

// Current status of execution
uint64_t tbaseExecutionStatus = TBASE_STATUS_UNINIT;

// mpidr for initial booting core.
// relevant only until tbase first time issues normal SMC to continue with NWd
static uint64_t tbaseBootCoreMpidr;

// ************************************************************************************
// Mask 32 bit software registers

uint64_t maskSWdRegister(uint64_t x)
{
  if (swd32Bit)
    return (uint32_t)x;
  return x;
}

//*******************************************************************************
// Create a secure context ready for programming an entry into the secure 
// payload.
// This does not modify actual EL1 or EL3 system registers, so no need to 
// save / restore them.
 
static int32_t tbase_init_secure_context(tbase_context *tbase_ctx)
{
  uint32_t sctlr = read_sctlr_el3();
  el1_sys_regs_t *el1_state;
  uint64_t mpidr = read_mpidr();

  /* Passing a NULL context is a critical programming error */
  assert(tbase_ctx);
  
  DBG_PRINTF("tbase_init_secure_context\n\r");

  memset(tbase_ctx, 0, sizeof(*tbase_ctx));

  /* Get a pointer to the S-EL1 context memory */
  el1_state = get_sysregs_ctx(&tbase_ctx->cpu_ctx);

  // Program the sctlr for S-EL1 execution with caches and mmu off
  sctlr &= SCTLR_EE_BIT;
  sctlr |= SCTLR_EL1_RES1;
  write_ctx_reg(el1_state, CTX_SCTLR_EL1, sctlr);

  /* Set this context as ready to be initialised i.e OFF */
  tbase_ctx->state = TBASE_STATE_OFF;

  /* Associate this context with the cpu specified */
  tbase_ctx->mpidr = mpidr;

  // Set up cm context for this core
  cm_set_context(mpidr, &tbase_ctx->cpu_ctx, SECURE); 
  // cm_init_exception_stack(mpidr, SECURE);

  return 0;
}

//*******************************************************************************
// Configure tbase and EL3 registers for intial entry 
static void tbase_init_eret( uint64_t entrypoint, uint32_t rw ) {
  uint32_t scr = read_scr();
  uint32_t spsr = TBASE_INIT_SPSR;
  
  assert(rw == TBASE_AARCH32);

  // Set the right security state and register width for the SP
  scr &= ~SCR_NS_BIT; 
  scr &= ~SCR_RW_BIT; 
  // Also IRQ and FIQ handled in secure state
  scr &= ~(SCR_FIQ_BIT|SCR_IRQ_BIT); 
  // No execution from Non-secure memory
  scr |= SCR_SIF_BIT; 
  
  cm_set_el3_eret_context(SECURE, entrypoint, spsr, scr);
  entry_point_info_t *image_info = bl31_plat_get_next_image_ep_info(SECURE);
  assert(image_info);
  image_info->spsr = spsr;

}

// ************************************************************************************
// Initilize pages

static void tbase_init_register_file( int area, int startPage, int pageCount )
{
  uint64_t MAPPING_FLAGS = TBASE_REGISTER_FILE_MMU_FLAGS;
  for (int pageNro=0; pageNro<pageCount; pageNro++) {
      registerFileL2[pageNro+startPage] = (registerFileStart[area] +PAGE_SIZE*pageNro)
              | MAPPING_FLAGS;
  }
}


// ************************************************************************************
// Initialize tbase system for first entry to tbase
// This and initial entry should be done only once in cold boot.

static int32_t tbase_init_entry()
{
  DBG_PRINTF("tbase_init\n\r");

  // Save el1 registers in case non-secure world has already been set up.
  cm_el1_sysregs_context_save(NON_SECURE);

  uint64_t mpidr = read_mpidr();
  uint32_t linear_id = platform_get_core_pos(mpidr);
  tbase_context *tbase_ctx = &secure_context[linear_id];
  
  
  // Note: mapping is 1:1, so physical and virtual addresses are here the same.
  cpu_context_t *ns_entry_context = (cpu_context_t *) cm_get_context(mpidr, NON_SECURE);  
  
  // ************************************************************************************
  // Configure parameter passing to tbase
  
  // Calculate page start addresses for register areas.
  registerFileStart[REGISTER_FILE_NWD] = page_align((uint64_t)&ns_entry_context[0], DOWN);
  registerFileStart[REGISTER_FILE_MONITOR] = page_align((uint64_t)&msm_area, DOWN);

  // Calculate page end addresses for register areas.
  registerFileEnd[REGISTER_FILE_NWD] = (uint64_t)(&ns_entry_context[TBASE_CORE_COUNT]);
  registerFileEnd[REGISTER_FILE_MONITOR] = ((uint64_t)&msm_area) +sizeof(msm_area);

  int32_t totalPages = 0;
  for (int area=0; area<REGISTER_FILE_COUNT; area++) {
    int32_t pages = page_align(registerFileEnd[area] - registerFileStart[area], UP) / PAGE_SIZE;
    assert( pages +totalPages <= TBASE_INTERFACE_PAGES );
    tbase_init_register_file(area, totalPages, pages);
    totalPages += pages;
  }

  // ************************************************************************************
  // Create boot structure
  tbaseBootCfg.magic       = TBASE_BOOTCFG_MAGIC;
  tbaseBootCfg.length      = sizeof(bootCfg_t);
  tbaseBootCfg.version     = TBASE_MONITOR_INTERFACE_VERSION;
  
  tbaseBootCfg.dRamBase    = TBASE_NWD_DRAM_BASE;
  tbaseBootCfg.dRamSize    = TBASE_NWD_DRAM_SIZE;
  tbaseBootCfg.secDRamBase = TBASE_SWD_DRAM_BASE;
  tbaseBootCfg.secDRamSize = TBASE_SWD_DRAM_SIZE;
  tbaseBootCfg.secIRamBase = TBASE_SWD_IMEM_BASE;
  tbaseBootCfg.secIRamSize = TBASE_SWD_IMEM_SIZE;
  
  tbaseBootCfg.conf_mair_el3 = read_mair_el3();
  tbaseBootCfg.MSMPteCount = totalPages;
  tbaseBootCfg.MSMBase = (uint64_t)registerFileL2;
  
  tbaseBootCfg.gic_distributor_base = TBASE_GIC_DIST_BASE;
  tbaseBootCfg.gic_cpuinterface_base = TBASE_GIC_CPU_BASE;
  tbaseBootCfg.gic_version = TBASE_GIC_VERSION;
  
  tbaseBootCfg.total_number_spi = TBASE_SPI_COUNT;
  tbaseBootCfg.ssiq_number = TBASE_SSIQ_NRO;
  
  tbaseBootCfg.flags       = TBASE_MONITOR_FLAGS;


        DBG_PRINTF("*** tbase boot cfg ***\n\r");
        DBG_PRINTF("* magic                 : 0x%.X\n\r",tbaseBootCfg.magic);
        DBG_PRINTF("* length                : 0x%.X\n\r",tbaseBootCfg.length);
        DBG_PRINTF("* version               : 0x%.X\n\r",tbaseBootCfg.version);
        DBG_PRINTF("* dRamBase              : 0x%.X\n\r",tbaseBootCfg.dRamBase);
        DBG_PRINTF("* dRamSize              : 0x%.X\n\r",tbaseBootCfg.dRamSize);
        DBG_PRINTF("* secDRamBase           : 0x%.X\n\r",tbaseBootCfg.secDRamBase);
        DBG_PRINTF("* secDRamSize           : 0x%.X\n\r",tbaseBootCfg.secDRamSize);
        DBG_PRINTF("* secIRamBase           : 0x%.X\n\r",tbaseBootCfg.secIRamBase);
        DBG_PRINTF("* secIRamSize           : 0x%.X\n\r",tbaseBootCfg.secIRamSize);
        DBG_PRINTF("* conf_mair_el3         : 0x%.X\n\r",tbaseBootCfg.conf_mair_el3);
        DBG_PRINTF("* MSMPteCount           : 0x%.X\n\r",tbaseBootCfg.MSMPteCount);
        DBG_PRINTF("* MSMBase               : 0x%.X\n\r",tbaseBootCfg.MSMBase);
        DBG_PRINTF("* gic_distributor_base  : 0x%.X\n\r",tbaseBootCfg.gic_distributor_base);
        DBG_PRINTF("* gic_cpuinterface_base : 0x%.X\n\r",tbaseBootCfg.gic_cpuinterface_base);
        DBG_PRINTF("* gic_version           : 0x%.X\n\r",tbaseBootCfg.gic_version);
        DBG_PRINTF("* total_number_spi      : 0x%.X\n\r",tbaseBootCfg.total_number_spi);
        DBG_PRINTF("* ssiq_number           : 0x%.X\n\r",tbaseBootCfg.ssiq_number);
        DBG_PRINTF("* flags                 : 0x%.X\n\r",tbaseBootCfg.flags);

  // ************************************************************************************
  // tbaseBootCfg and l2 entries may be accesses uncached, so must flush those.
  flush_dcache_range((unsigned long)&tbaseBootCfg, sizeof(bootCfg_t));
  flush_dcache_range((unsigned long)&registerFileL2, sizeof(registerFileL2));
  
  // ************************************************************************************
  // Set registers for tbase initialization entry
  cpu_context_t *s_entry_context = &tbase_ctx->cpu_ctx;
  gp_regs_t *s_entry_gpregs = get_gpregs_ctx(s_entry_context);
  write_ctx_reg(s_entry_gpregs, CTX_GPREG_X1, 0);
  write_ctx_reg(s_entry_gpregs, CTX_GPREG_X1, (int64_t)&tbaseBootCfg);

  
  // SPSR for SMC handling (FIQ mode)
  tbaseEntrySpsr = TBASE_ENTRY_SPSR;
  
  DBG_PRINTF("tbase init SPSR 0x%x\n\r", read_ctx_reg(get_el3state_ctx(&tbase_ctx->cpu_ctx), 
             CTX_SPSR_EL3) );
  DBG_PRINTF("tbase SMC SPSR %x\nr\r", tbaseEntrySpsr );

  // ************************************************************************************
  // Start tbase

  tbase_synchronous_sp_entry(tbase_ctx);
  tbase_ctx->state = TBASE_STATE_ON;
  
  // Initialize forwarded FIQs from tbase to the ATF
  plat_tbase_fiqforward_init();

#if TBASE_PM_ENABLE
  // Register power managemnt hooks with PSCI
  psci_register_spd_pm_hook(&tbase_pm);
#endif

  cm_el1_sysregs_context_restore(NON_SECURE);
  cm_set_next_eret_context(NON_SECURE);

  return 1;
}


// ************************************************************************************
// Copy sysres from core to core

static void save_sysregs_core(uint64_t fromCoreNro, uint32_t toCoreNro) {
  if (fromCoreNro != toCoreNro) {
    cpu_context_t *cpu_ctx = &secure_context[fromCoreNro].cpu_ctx;
    memcpy(&secure_context[toCoreNro].cpu_ctx, cpu_ctx, sizeof(cpu_context_t) );
  }
}

// ************************************************************************************
// Initialize el1sysregs for non-primary cores

void save_sysregs_allcore() {
  uint64_t mpidr = read_mpidr();
  uint32_t linear_id = platform_get_core_pos(mpidr);
  for (int coreNro = 0; coreNro < TBASE_CORE_COUNT; coreNro++) {
    save_sysregs_core(linear_id, coreNro);
  }
  tbaseBootCoreMpidr = mpidr;
}

// ************************************************************************************
// Core-specific context initialization for non-booting cores

void tbase_init_core(uint64_t mpidr) {
  uint32_t linear_id = platform_get_core_pos(mpidr);
  tbase_context *tbase_ctx = &secure_context[linear_id];
  
  if (mpidr == tbaseBootCoreMpidr) {
    return;
  }

  tbase_init_secure_context(tbase_ctx);
  
  uint32_t boot_core_nro = platform_get_core_pos(tbaseBootCoreMpidr);
  save_sysregs_core(boot_core_nro, linear_id);
}


// ************************************************************************************
// Configure tbase parameters in monitor
// Before this SMC only entry to tbase is return from fastcall.

void configure_tbase(uint64_t x1, uint64_t x2) 
{
  uint32_t w1 = maskSWdRegister(x1);
  DBG_PRINTF( "tbase_fastcall_handler TBASE_SMC_FASTCALL_CONFIG_OK\n\r");
  
  if (TBASE_SMC_FASTCALL_CONFIG_VECTOR==w1) {
    tbaseEntryBase = maskSWdRegister(x2);
    tbaseInitStatus = TBASE_INIT_CONFIG_OK;
    DBG_PRINTF("tbase config ok %llx %x\n\r", tbaseEntryBase, tbaseInitStatus);
    // Register an FIQ handler when executing in the non-secure state.
    uint32_t flags = 0;
    set_interrupt_rm_flag(flags, NON_SECURE);
    uint32_t rc = register_interrupt_type_handler(INTR_TYPE_S_EL1,
                tbase_fiq_handler,
                flags);
    if (rc!=0) {
      DBG_PRINTF( "tbase_fastcall_setup FIQ register failed.\n\r");
    }
  } else { // Just to keep compatibility for a minute
    tbaseEntryBase = w1;
    tbaseInitStatus = TBASE_INIT_CONFIG_OK;
  }
}


// ************************************************************************************
// Setup tbase SPD 

int32_t tbase_fastcall_setup(void)
{
  entry_point_info_t *image_info;
  
  image_info = bl31_plat_get_next_image_ep_info(SECURE);
  assert(image_info);

  uint32_t linear_id = platform_get_core_pos(read_mpidr());

  // TODO: We do not need this anymore, if PM is in use
  tbase_init_secure_context(&secure_context[linear_id]);

  tbase_init_eret(image_info->pc,TBASE_AARCH32);
  bl31_register_bl32_init(&tbase_init_entry);

  return 0;
}



