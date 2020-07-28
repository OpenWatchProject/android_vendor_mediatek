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
#include <bl_common.h>
#include <assert.h>
#include <arch_helpers.h>
#include <console.h>
#include <platform.h>
#include <context_mgmt.h>
#include <runtime_svc.h>
#include <bl31.h>
#include <tbase_private.h>

#include <gic_v2.h>
#include <gic_v3.h>
#include <interrupt_mgmt.h>
#include <plat_config.h>
#include "plat_private.h"
#include <plat_tbase.h>

/*
 * Notes:
 *   * 32 bit SMC take and return only 32 bit results; remaining bits are undef.
 *     * Never use remaining values for anything.
 *   * PM callbacks are dummy.
 *     * Implement resume and migrate callbacks.
 *   * We update secure system registers at every return. We could optimize this.
 * To be consireded:
 *   * Initialization checks: Check non-null context
 *   * On-demand intialization
 *   * State checking: Chech tbase does not return incorrect way
 *     (fastcall vs normal SMC / interrupt)
 *   * Disable FIQs, if occuring when tbase is not ok to handle them.
 */ 

// MSM areas
struct msm_area_t msm_area;

// Context for each core. gp registers not used by SPD.
tbase_context *secure_context = msm_area.secure_context;

extern uint32_t plat_tbase_dump(void);
extern uint32_t plat_tbase_forward_fiq(uint32_t fiqId);

__attribute__((weak)) uint32_t plat_tbase_handle_fastcall(uint32_t smc_fid,
		uint64_t x1,
		uint64_t x2,
		uint64_t x3,
		uint64_t x4,
		void *handle)
{
	return PLAT_TBASE_INPUT_ERROR;
}

// ************************************************************************************
// Common setup for normal fastcalls and fastcalls to tbase

static void tbase_setup_entry_common( cpu_context_t *s_context, 
              cpu_context_t *ns_context, 
              enum tbase_entry_reason_t reason) {
  // Set up registers
  gp_regs_t *s_gpregs = get_gpregs_ctx(s_context);  
  
  // NWd spsr
  uint64_t ns_spsr = read_ctx_reg(get_el3state_ctx(ns_context), CTX_SPSR_EL3);
  write_ctx_reg(s_gpregs, CTX_GPREG_X2, ns_spsr);
  write_ctx_reg(s_gpregs, CTX_GPREG_X4, reason);
  // Entry to tbase
  el3_state_t *el3sysregs = get_el3state_ctx(s_context);
  write_ctx_reg(el3sysregs, CTX_SPSR_EL3, tbaseEntrySpsr);
  
  cm_set_elr_el3(SECURE,tbaseEntryBase);
}

// ************************************************************************************
// Set up fastcall or normal SMC entry from NWd to tbase

void tbase_setup_entry_nwd( cpu_context_t *ns_context, enum tbase_entry_reason_t reason ) {

  gp_regs_t *ns_gpregs = get_gpregs_ctx(ns_context);

  // Set up registers
  cpu_context_t *s_context = (cpu_context_t *)cm_get_context_by_mpidr(read_mpidr(), SECURE);

  gp_regs_t *s_gpregs = get_gpregs_ctx(s_context);  

  // Offset into registerFile
  uint64_t registerOffset = (uintptr_t)ns_gpregs -registerFileStart[REGISTER_FILE_NWD];
  write_ctx_reg(s_gpregs, CTX_GPREG_X0, registerOffset);
  // Flags
  write_ctx_reg(s_gpregs, CTX_GPREG_X1, (TBASE_NWD_REGISTER_COUNT<<8) | TBASE_SMC_NWD);

  tbase_setup_entry_common( s_context, ns_context, reason );
}

// ************************************************************************************
// Set up fastcall entry from monitor to tbase

void tbase_setup_entry_monitor( cpu_context_t *ns_context ) {
  uint64_t mpidr = read_mpidr();
  uint32_t linear_id = platform_get_core_pos(mpidr);
  uint64_t *monitorCallRegs = secure_context[linear_id].monitorCallRegs;
  
  // Set up registers
  cpu_context_t *s_context = cm_get_context_by_mpidr(read_mpidr(), SECURE);
  gp_regs_t *s_gpregs = get_gpregs_ctx(s_context);  

  /* calculate the offset that the TEE sees. We defined that it shall map the */
  /* ns_entry_context into its virtual address space using full pages. And */
  /* immediately following, the next pages hold the mapping of msm_area. */
  /*     :  page  :  page  :  page  :  page  :  page  :  page  : */
  /*   --+--------+--------+--------+--------+--------+--------+-- */
  /*     |        |        |        |        |        |        | */
  /*     |   ------------------     |    ----------------      | */
  /* ... |  [ ns_entry_context ]    |   [    msm_area    ]     | ... */
  /*     |   ------------------     |    ----------------      | */
  /*     |        |        |        |        |     :  |        | */
  /*   --+--------+--------+--------+--------+-----:--+--------+-- */
  /*    :                          :              : */
  /*     <--- lenNWdRegMapping ---> <-- offset --> */

  /* Thus we have to find the length of the ns_entry_context array mapping */
  /* first. As registerFileStart[REGISTER_FILE_NWD] is aligned down to the */
  /* start of the page where ns_entry_context is in, the full length is: */
  uint64_t lenNWdRegMapping = page_align(registerFileEnd[REGISTER_FILE_NWD] - registerFileStart[REGISTER_FILE_NWD], UP);
  /* monitorCallRegs is msm_area.secure_context[id].monitorCallRegs actually, */
  /* and registerFileStart[REGISTER_FILE_MONITOR] is already aligned down to */
  /* start of the page where msm_area is in. So the offset calculation is */
  /* straight forward. */
  uint64_t offsetMonitorCallRegs = (uintptr_t)monitorCallRegs - registerFileStart[REGISTER_FILE_MONITOR];

  /* the overall offset that the TEE sees is */
  uint64_t registerOffset = lenNWdRegMapping + offsetMonitorCallRegs;

  write_ctx_reg(s_gpregs, CTX_GPREG_X0, registerOffset);
  // Flags
  write_ctx_reg(s_gpregs, CTX_GPREG_X1, (TBASE_MAX_MONITOR_CALL_REGS<<8) | TBASE_SMC_MONITOR);

  tbase_setup_entry_common( s_context, ns_context, TBASE_ENTRY_FASTCALL );
}

// ************************************************************************************
// Print NWd parameters X0...X3

#if 1
void print_fastcall_params( char *msg, uint32_t secure ) {
  gp_regs_t *ns_gpregs = get_gpregs_ctx((cpu_context_t *) cm_get_context_by_mpidr(read_mpidr(), secure)); 
  DBG_PRINTF("tbase %s (%d) 0x%llx 0x%llx 0x%llx 0x%llx\n\r", msg, secure,
         (unsigned long long)read_ctx_reg(ns_gpregs, CTX_GPREG_X0), 
         (unsigned long long)read_ctx_reg(ns_gpregs, CTX_GPREG_X1), 
         (unsigned long long)read_ctx_reg(ns_gpregs, CTX_GPREG_X2), 
         (unsigned long long)read_ctx_reg(ns_gpregs, CTX_GPREG_X3));
}
#endif

// ************************************************************************************
// Forward fastcall to tbase
// Returns 0 in case of successfull call, and non-zero in case of error.
// If resp is non-NULL, return values are put there.
// If previous context was NON_SECURE, caller must save it before call.
// Function changes to SECURE context.

uint64_t tbase_monitor_fastcall(uint32_t smc_fid,
	uint64_t x1,
	uint64_t x2,
	uint64_t x3,
	uint64_t x4,
	fc_response_t *resp)
{
	uint64_t mpidr = read_mpidr();
	uint32_t linear_id = platform_get_core_pos(mpidr);
	tbase_context *tbase_ctx = &secure_context[linear_id];
	uint64_t *regs = tbase_ctx->monitorCallRegs;

	if ((tbaseExecutionStatus & TBASE_STATUS_FASTCALL_OK_BIT) == 0) {
		/* TBASE must be initialized to be usable */
		DBG_PRINTF("tbase_monitor_fastcall tbase not ready for fastcall\n\r");
		return 1;
	}
	if (tbase_ctx->state == TBASE_STATE_OFF) {
		DBG_PRINTF("tbase_monitor_fastcall tbase not ready for fastcall\n\r");
		return 1;
	}


	/* parameters for call */
	regs[0] = smc_fid;
	regs[1] = x1;
	regs[2] = x2;
	regs[3] = x3;
	/* regs[4] = x4; */

	cpu_context_t *ns_context = (cpu_context_t *) cm_get_context_by_mpidr(mpidr, NON_SECURE);

	tbase_setup_entry_monitor(ns_context);
	tbase_synchronous_sp_entry(tbase_ctx);

	if (resp != NULL) {
		resp->x1 = regs[1];
		resp->x2 = regs[2];
		resp->x3 = regs[3];
	}

	return 0;
}

// ************************************************************************************
// Output thru monitor

static void output(uint64_t x1,uint64_t x2) 
{
  switch(maskSWdRegister(x1)) {
    case TBASE_SMC_FASTCALL_OUTPUT_PUTC:
      TBASE_OUTPUT_PUTC(x2&0xFF);
    break;
  }
}


// ************************************************************************************
// Set tbase status

static void tbase_status(uint64_t x1,uint64_t x2) 
{
  DBG_PRINTF( "tbase_fastcall_handler TBASE_SMC_FASTCALL_STATUS %llx %llx\n\r", (unsigned long long)x1, (unsigned long long)x2 );
  switch(maskSWdRegister(x1)) {
    case TBASE_SMC_FASTCALL_STATUS_EXECUTION:
      tbaseExecutionStatus = maskSWdRegister(x2);
      TBASE_EXECUTION_STATUS(tbaseExecutionStatus);
      break;
  }
}


// ************************************************************************************
// tbase_fiqforward_configure(intrNo,status)
void tbase_fiqforward_configure(uint32_t intrNo, uint32_t enable)
{
  // Trigger monitor fastcall to Kinibi for FIQ configuration
  tbase_monitor_fastcall( (uint32_t)TBASE_SMC_FASTCALL_FIQFORWARD_CONFIG,
                          (uint64_t)intrNo,
                          (uint64_t)enable,
                          (uint64_t)0,
                          (uint64_t)0,
                          NULL);

  DBG_PRINTF( "%s: Forwarding interrupt %d is %s\n\r", 
              __func__,
              intrNo,
              (enable == TBASE_FLAG_SET)?"enabled":"disabled" );
}


// ************************************************************************************
// fastcall handler
#if (ATF_VERSION_CODE(VERSION_MAJOR, VERSION_MINOR) >= ATF_VERSION_CODE(1, 5))
static uintptr_t tbase_fastcall_handler(uint32_t smc_fid,
					u_register_t x1,
					u_register_t x2,
					u_register_t x3,
					u_register_t x4,
					void *cookie,
					void *handle,
					u_register_t flags)
#else
static uint64_t tbase_fastcall_handler(uint32_t smc_fid,
        uint64_t x1,
        uint64_t x2,
        uint64_t x3,
        uint64_t x4,
        void *cookie,
        void *handle,
        uint64_t flags)
#endif /* ATF version >= 1.5 */
{
  uint64_t mpidr = read_mpidr();
  uint32_t linear_id = platform_get_core_pos(mpidr);
  tbase_context *tbase_ctx = &secure_context[linear_id];
  int caller_security_state = flags&1;

	/* 'x' register are coming from AARCH32, and the top 32 bits are unknown.
	 * So we make make sure we only keep the bottom 32 bits
	 */
	x1 = maskSWdRegister(x1);
	x2 = maskSWdRegister(x2);
	x3 = maskSWdRegister(x3);
	x4 = maskSWdRegister(x4);

  if (caller_security_state==SECURE) {
    switch(maskSWdRegister(smc_fid)) {
      case TBASE_SMC_FASTCALL_RETURN: {
        // Return values from fastcall already in cpu_context!
        // TODO: Could we skip saving sysregs?
        tbase_synchronous_sp_exit(tbase_ctx, 0, 1);
      } 
      case TBASE_SMC_FASTCALL_CONFIG_OK: {
        configure_tbase(x1,x2);
        SMC_RET1(handle,smc_fid);
        break;
      } 
      case TBASE_SMC_FASTCALL_OUTPUT: {
        output(x1,x2);
        SMC_RET1(handle,smc_fid);
        break;
      }
      case TBASE_SMC_FASTCALL_STATUS: {
        DBG_PRINTF( "tbase_fastcall_handler TBASE_SMC_FASTCALL_STATUS\n\r");
        tbase_status(x1,x2);
        SMC_RET1(handle,smc_fid);
        break;
      }
      case TBASE_SMC_FASTCALL_INPUT: {
        smc_fid = plat_tbase_input(x1,&x2,&(tbase_ctx->tbase_input_fastcall));
		/************************************************************************
		 *                MONITOR VIEW : 2 BLOCKS
		 ************************************************************************
		cm_context_info[coreID].ptr[NON_SECURE]
		registerFileStart[REGISTER_FILE_NWD]    --------------------   --> ^
							|                     |     |
							|(cpu_context_t core0)|     |
							|---------------------|     |
							//                   //     |
							|---------------------|     X
							|(cpu_context_t coreN)|     |
							|                     |     |
		registerFileEnd[REGISTER_FILE_NWD]      ---------------------  --> v
							      msm_area                   tbase_context
		registerFileStart[REGISTER_FILE_MONITOR]---------------------       ---------------------- --> ^
							|                   |      |                      |    |
							|(tbase_ctxt core0) |      |       state          |    |
							|                   |      |       mpidr          |    |
							|-------------------|      |     c_rt_ctx         |    Y
							//                  // ==> |     cpu_ctx          |    |
							|-------------------|      |  monitorCallRegs     |    |
							|                   |      | tbase_input_fastcall--->  v
							|(tbase_ctxt coreN) |      |                      |
							|                   |       ----------------------
		registerFileEnd[REGISTER_FILE_MONITOR]   -------------------
		*/
		/************************************************************************
		 *                             TBASE VIEW : 1 BLOCK
		 ************************************************************************
		MonitorSharedArea                        ------------------- REGISTER_FILE_NWD  ------> ^
							|                   |                           |
							|                   |                           |
							|                   |                           |
							|                   |                           |
							|                   |                           Z
							|                   |                           |
							---------------------REGISTER_FILE_MONITOR      |
							|                   |                           |
							|                   |                           |
							|tbase_input_fastcal--------------------------> v
							|                   |
							|    (area core7)   |
							|                   |
							-------------------
		X = registerFileEnd[REGISTER_FILE_NWD] - registerFileStart[REGISTER_FILE_NWD]
		Z = Y - registerFileStart[REGISTER_FILE_MONITOR] + X
		*/
        SMC_RET3(handle, smc_fid,
            page_align(registerFileEnd[REGISTER_FILE_NWD]
               - registerFileStart[REGISTER_FILE_NWD], UP)
               + (uint64_t)&(tbase_ctx->tbase_input_fastcall)
               - registerFileStart[REGISTER_FILE_MONITOR],
               x2);
        break;
      }
      case TBASE_SMC_FASTCALL_FORWARD_FIQ: {
        DBG_PRINTF( "tbase_fastcall_handler TBASE_SMC_FASTCALL_FORWARD_FIQ\n\r");
        x1 = (uint64_t)plat_tbase_forward_fiq(x1);
        SMC_RET2(handle,smc_fid,x1);
        break;
      }
      
      default: {
		uint32_t ret = plat_tbase_handle_fastcall(smc_fid, x1, x2, x3, x4, handle);

		if (ret != PLAT_TBASE_INPUT_ERROR) {
			DBG_PRINTF("tbase_fastcall_handler : fastcall has been handled\n\r");
			SMC_RET2(handle, smc_fid, ret);
		} else
			DBG_PRINTF("tbase_fastcall_handler : fastcall input error\n\r");
        // What now?
        DBG_PRINTF( "tbase_fastcall_handler SMC_UNK %x\n\r", smc_fid );
        SMC_RET1(handle, SMC_UNK);
        break;
      }
    }
  }
  else
  {
    if ((tbaseExecutionStatus&TBASE_STATUS_FASTCALL_OK_BIT)==0) {
      // TBASE must be initialized to be usable
      // TODO: What is correct error code?
      DBG_PRINTF( "tbase_fastcall_handler tbase not ready for fastcall\n\r" );
      SMC_RET1(handle, SMC_UNK);
      return 0;
    }
    if(tbase_ctx->state == TBASE_STATE_OFF) {
      DBG_PRINTF( "tbase_fastcall_handler tbase not ready for fastcall\n\r" );
      SMC_RET1(handle, SMC_UNK);
      return 0;
    }

    uint32_t ret = plat_tbase_handle_fastcall(smc_fid, x1, x2, x3, x4, handle);
    
    if(ret != PLAT_TBASE_INPUT_ERROR) {
       /* Fastcall has been handled*/
       return ret;
    }
    DBG_PRINTF( "tbase_fastcall_handler NWd %x\n\r", smc_fid );
    // So far all fastcalls go to tbase
    // Save NWd context
    gp_regs_t *ns_gpregs = get_gpregs_ctx((cpu_context_t *)handle);
    write_ctx_reg(ns_gpregs, CTX_GPREG_X0, smc_fid ); // These are not saved yet
    write_ctx_reg(ns_gpregs, CTX_GPREG_X1, x1 );
    write_ctx_reg(ns_gpregs, CTX_GPREG_X2, x2 );
    write_ctx_reg(ns_gpregs, CTX_GPREG_X3, x3 );
    cm_el1_sysregs_context_save(NON_SECURE);

    // Load SWd context
    tbase_setup_entry_nwd((cpu_context_t *)handle, TBASE_ENTRY_FASTCALL);
#if 1
    print_fastcall_params("entry", NON_SECURE);
#endif
    tbase_synchronous_sp_entry(tbase_ctx);
    cm_el1_sysregs_context_restore(NON_SECURE);
    cm_set_next_eret_context(NON_SECURE);
	DBG_PRINTF( "[Cipher] Go back to NWd from here==============");
    return 0; // Does not seem to matter what we return
  }
}


// ************************************************************************************
// SMC handler
#if (ATF_VERSION_CODE(VERSION_MAJOR, VERSION_MINOR) >= ATF_VERSION_CODE(1, 5))
static uintptr_t tbase_smc_handler(uint32_t smc_fid,
				   u_register_t x1,
				   u_register_t x2,
				   u_register_t x3,
				   u_register_t x4,
				   void *cookie,
				   void *handle,
				   u_register_t flags)
#else
static uint64_t tbase_smc_handler(uint32_t smc_fid,
        uint64_t x1,
        uint64_t x2,
        uint64_t x3,
        uint64_t x4,
        void *cookie,
        void *handle,
        uint64_t flags)
#endif /* ATF version >= 1.5*/
{
  uint64_t mpidr = read_mpidr();
  uint32_t linear_id = platform_get_core_pos(mpidr);
  tbase_context *tbase_ctx = &secure_context[linear_id];
  int caller_security_state = flags&1;

  if (caller_security_state==SECURE) {
    // Yield to NWd
    // TODO: Check id
    if (tbaseInitStatus==TBASE_INIT_CONFIG_OK) {
      // Save sysregs to all cores.
      // After this tbase can work on any core.
      save_sysregs_allcore();
      tbaseInitStatus = TBASE_INIT_SYSREGS_OK;
      if (tbaseExecutionStatus==TBASE_STATUS_UNINIT) {
        tbaseExecutionStatus = TBASE_STATUS_NORMAL;
      }
    }
    // If above check fails, it is not possible to return to tbase.
    tbase_synchronous_sp_exit(tbase_ctx, 0, 1);
  } 
  else {
    if ((tbaseExecutionStatus&TBASE_STATUS_SMC_OK_BIT)==0) {
      // TBASE must be initialized to be usable
      DBG_PRINTF( "tbase_smc_handler tbase not ready for smc.\n\r");
      // TODO: What is correct error code?
      SMC_RET1(handle, SMC_UNK);
      return 1;
    }
    if(tbase_ctx->state == TBASE_STATE_OFF) {
      DBG_PRINTF( "tbase_smc_handler tbase not ready for fastcall\n\r" );
      return 1;
    }

    // NSIQ, go to SWd
    // TODO: Check id?
    
    // Save NWd
    gp_regs_t *ns_gpregs = get_gpregs_ctx((cpu_context_t *)handle);
    write_ctx_reg(ns_gpregs, CTX_GPREG_X0, smc_fid );
    write_ctx_reg(ns_gpregs, CTX_GPREG_X1, x1 );
    write_ctx_reg(ns_gpregs, CTX_GPREG_X2, x2 );
    write_ctx_reg(ns_gpregs, CTX_GPREG_X3, x3 );
    cm_el1_sysregs_context_save(NON_SECURE);
    
    // Load SWd
    tbase_setup_entry_nwd((cpu_context_t *)handle, TBASE_ENTRY_SMC);
    // Enter tbase. tbase must return using normal SMC, which will continue here.   
    tbase_synchronous_sp_entry(tbase_ctx);
    // Load NWd
    cm_el1_sysregs_context_restore(NON_SECURE);
    cm_set_next_eret_context(NON_SECURE);
  }
  return 0;
}

//************************************************************************************************
// FIQ handler for FIQ when in NWd
uint64_t tbase_fiq_handler( uint32_t id,
          uint32_t flags,
          void *handle,
          void *cookie)
{
  uint64_t mpidr;
  uint32_t linear_id;
  tbase_context *tbase_ctx;

  mpidr = read_mpidr();
  linear_id = platform_get_core_pos(mpidr);
  tbase_ctx = &secure_context[linear_id];
  assert(&tbase_ctx->cpu_ctx == cm_get_context_by_mpidr(mpidr, SECURE));
  /* Sanity check the pointer to this cpu's context*/
  assert(handle == cm_get_context_by_mpidr(mpidr, NON_SECURE));

  /* Check if the vector has been entered for SGI/FIQ dump reason */
  if (id == FIQ_SMP_CALL_SGI) {
#if 0
    /* ACK gic */
    {
        unsigned int iar;
        iar = gicc_read_IAR(get_plat_config()->gicc_base);
        gicc_write_EOIR(get_plat_config()->gicc_base, iar);
    }
#else
    plat_ic_acknowledge_interrupt();
    plat_ic_end_of_interrupt(id);
#endif
    /* Save the non-secure context before entering the TSP */
    cm_el1_sysregs_context_save(NON_SECURE);
    /* Call customer's dump implementation */
    plat_tbase_dump();
    // Load NWd
    //cm_el1_sysregs_context_restore(NON_SECURE);
    //cm_set_next_eret_context(NON_SECURE);
  } 
  else {

    /* Check the security state when the exception was generated */
    assert(get_interrupt_src_ss(flags) == NON_SECURE);

    if ((tbaseExecutionStatus&TBASE_STATUS_SMC_OK_BIT)==0) {
      // TBASE must be initialized to be usable
      // TODO: What should we really do here?
      // We should disable FIQs to prevent futher interrupts
      DBG_PRINTF( "tbase_interrupt_handler tbase not ready for interrupt\n\r" );
      return 1;
    }
    if(tbase_ctx->state == TBASE_STATE_OFF) {
      DBG_PRINTF( "tbase_interrupt_handler tbase not ready for fastcall\n\r" );
      return 1;
    }

    /* Save the non-secure context before entering the TSP */
    cm_el1_sysregs_context_save(NON_SECURE);

    // Load SWd context
    tbase_setup_entry_nwd((cpu_context_t *)handle,TBASE_ENTRY_FIQ);
  
    // Enter tbase. tbase must return using normal SMC, which will continue here.
    tbase_synchronous_sp_entry(tbase_ctx);

    // Load NWd
    cm_el1_sysregs_context_restore(NON_SECURE);
    cm_set_next_eret_context(NON_SECURE);
  }

  return 0;
}

//************************************************************************************************
/* Register tbase fastcalls service */

DECLARE_RT_SVC(
  tbase_fastcall,
  OEN_TOS_START,
  OEN_TOS_END,
  SMC_TYPE_FAST,
  tbase_fastcall_setup,
  tbase_fastcall_handler
);

/* Register tbase SMC service */
// Note: OEN_XXX constants do not apply to normal SMCs (only to fastcalls).
DECLARE_RT_SVC(
  tbase_smc,
  0,
  2, /* bastien 2014-10-07 : shouldn't this be 1 ? */
  SMC_TYPE_STD,
  NULL,
  tbase_smc_handler
);

#if TBASE_OEM_ROUTE_ENABLE
/* Register tbase OEM SMC handler service */
DECLARE_RT_SVC(
  tbase_oem_fastcall,
  OEN_OEM_START,
  OEN_OEM_END,
  SMC_TYPE_FAST,
  NULL,
  tbase_fastcall_handler
);
#endif

#if TBASE_SIP_ROUTE_ENABLE
/* Register tbase SIP SMC handler service */
DECLARE_RT_SVC(
  tbase_sip_fastcall,
  OEN_SIP_START,
  OEN_SIP_END,
  SMC_TYPE_FAST,
  NULL,
  tbase_fastcall_handler
);
#endif

#if TBASE_DUMMY_SIP_ROUTE_ENABLE
/* Register tbase SIP SMC handler service, unfortunately because of a typo in our
 * older versions we must serve the 0x81000000 fastcall range for backward compat */
DECLARE_RT_SVC(
  tbase_dummy_sip_fastcall,
  OEN_CPU_START,
  OEN_CPU_END,
  SMC_TYPE_FAST,
  NULL,
  tbase_fastcall_handler
);
#endif
