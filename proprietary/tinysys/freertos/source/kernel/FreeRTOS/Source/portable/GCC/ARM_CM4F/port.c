/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/*-----------------------------------------------------------
 * Implementation of functions defined in portable.h for the ARM CM4F port.
 *----------------------------------------------------------*/

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
/* add by MTK to include macro which defined by MTK platform*/
#ifdef CFG_XGPT_SUPPORT
#include <mt_gpt.h>
#endif
#include <utils.h>
#include "interrupt.h"
#ifdef CFG_WDT_SUPPORT
#include <wdt.h>
#endif
#ifndef __VFP_FP__
	#error This port can only be used when the project options are configured to enable hardware floating point support.
#endif

#ifndef configSYSTICK_CLOCK_HZ
	#define configSYSTICK_CLOCK_HZ 32768
	/* Ensure the SysTick is clocked at the same frequency as the core. */
	/* #define portNVIC_SYSTICK_CLK_BIT	( 1UL << 2UL ) */
	/* MTK: always use external clock 32K*/
	#define portNVIC_SYSTICK_CLK_BIT	( 0 )
#else
	/* The way the SysTick is clocked is not modified in case it is not the same
	 * as the core. */
	#define portNVIC_SYSTICK_CLK_BIT	( 0 )
#endif

/* Constants required to manipulate the core.  Registers first... */
#define portNVIC_SYSTICK_CTRL_REG			( * ( ( volatile uint32_t * ) 0xe000e010 ) )
#define portNVIC_SYSTICK_LOAD_REG			( * ( ( volatile uint32_t * ) 0xe000e014 ) )
#define portNVIC_SYSTICK_CURRENT_VALUE_REG	( * ( ( volatile uint32_t * ) 0xe000e018 ) )
#define portNVIC_SYSPRI1_REG                (*((volatile uint32_t*)0xe000ed1c))
#define portNVIC_SYSPRI2_REG				( * ( ( volatile uint32_t * ) 0xe000ed20 ) )

/* ...then bits in the registers. */
#define portNVIC_SYSTICK_INT_BIT			( 1UL << 1UL )
#define portNVIC_SYSTICK_ENABLE_BIT			( 1UL << 0UL )
#define portNVIC_SYSTICK_COUNT_FLAG_BIT		( 1UL << 16UL )
#define portNVIC_PENDSVCLEAR_BIT 			( 1UL << 27UL )
#define portNVIC_PEND_SYSTICK_CLEAR_BIT		( 1UL << 25UL )

#define portNVIC_SVC_PRI                    (((uint32_t) configKERNEL_INTERRUPT_PRIORITY) << 24UL)
#define portNVIC_PENDSV_PRI					( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 16UL )
#define portNVIC_SYSTICK_PRI				( ( ( uint32_t ) configKERNEL_INTERRUPT_PRIORITY ) << 24UL )

/* Constants required to check the validity of an interrupt priority. */
#define portFIRST_USER_INTERRUPT_NUMBER		( 16 )
#define portNVIC_IP_REGISTERS_OFFSET_16 	( 0xE000E3F0 )
#define portAIRCR_REG						( * ( ( volatile uint32_t * ) 0xE000ED0C ) )
#define portMAX_8_BIT_VALUE					( ( uint8_t ) 0xff )
#define portTOP_BIT_OF_BYTE					( ( uint8_t ) 0x80 )
#define portMAX_PRIGROUP_BITS				( ( uint8_t ) 7 )
#define portPRIORITY_GROUP_MASK				( 0x07UL << 8UL )
#define portPRIGROUP_SHIFT					( 8UL )

/* Masks off all bits but the VECTACTIVE bits in the ICSR register. */
#define portVECTACTIVE_MASK					( 0xFFUL )

/* Constants required to manipulate the VFP. */
#define portFPCCR					( ( volatile uint32_t * ) 0xe000ef34 ) /* Floating point context control register. */
#define portASPEN_AND_LSPEN_BITS	( 0x3UL << 30UL )

/* Constants required to set up the initial stack. */
#define portINITIAL_XPSR			( 0x01000000 )
#define portINITIAL_EXEC_RETURN		( 0xfffffffd )

/* The systick is a 24-bit counter. */
#define portMAX_24_BIT_NUMBER				( 0xffffffUL )

/* A fiddle factor to estimate the number of SysTick counts that would have
 * occurred while the SysTick counter is stopped during tickless idle
 * calculations. */
#define portMISSED_COUNTS_FACTOR			( 45UL )

/* Let the user override the pre-loading of the initial LR with the address of
 * prvTaskExitError() in case is messes up unwinding of the stack in the
 * debugger. */
#ifdef configTASK_RETURN_ADDRESS
	#define portTASK_RETURN_ADDRESS	configTASK_RETURN_ADDRESS
#else
	#define portTASK_RETURN_ADDRESS	prvTaskExitError
#endif


/* Each task maintains its own interrupt status in the critical nesting
variable. */
UBaseType_t uxCriticalNesting = 0xaaaaaaaa;

/*
 * Setup the timer to generate the tick interrupts.  The implementation in this
 * file is weak to allow application writers to change the timer used to
 * generate the tick interrupt.
 */
void vPortSetupTimerInterrupt( void );

/*
 * Exception handlers.
 */
void xPortPendSVHandler( void ) __attribute__ (( naked ));
void xPortSysTickHandler( void );
void vPortSVCHandler( void ) __attribute__ (( naked ));

/*
 * Start first task is a separate function so it can be tested in isolation.
 */
static void prvPortStartFirstTask( void ) __attribute__ (( naked ));

/*
 * Function to enable the VFP.
 */
 static void vPortEnableVFP( void ) __attribute__ (( naked ));

/*
 * Used to catch tasks that attempt to return from their implementing function.
 */
static void prvTaskExitError( void );

/*-----------------------------------------------------------*/

/*
 * The number of SysTick increments that make up one tick period.
 */
#if configUSE_TICKLESS_IDLE == 1
	static uint32_t ulTimerCountsForOneTick = 0;
#endif /* configUSE_TICKLESS_IDLE */
#define TIMER_TICK_RATE     32768
/*
 * The maximum number of tick periods that can be suppressed is limited by the
 * 24 bit resolution of the SysTick timer.
 */
#if configUSE_TICKLESS_IDLE == 1
	static uint32_t xMaximumPossibleSuppressedTicks = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Compensate for the CPU cycles that pass while the SysTick is stopped (low
 * power functionality only.
 */
#if configUSE_TICKLESS_IDLE == 1
	static uint32_t ulStoppedTimerCompensation = 0;
#endif /* configUSE_TICKLESS_IDLE */

/*
 * Used by the portASSERT_IF_INTERRUPT_PRIORITY_INVALID() macro to ensure
 * FreeRTOS API functions are not called from interrupts that have been assigned
 * a priority above configMAX_SYSCALL_INTERRUPT_PRIORITY.
 */
#if ( configASSERT_DEFINED == 1 )
	 static uint8_t ucMaxSysCallPriority = 0;
	 static uint32_t ulMaxPRIGROUPValue = 0;
	 static const volatile uint8_t * const pcInterruptPriorityRegisters = ( const volatile uint8_t * const ) portNVIC_IP_REGISTERS_OFFSET_16;
#endif /* configASSERT_DEFINED */

/*-----------------------------------------------------------*/

/*
 * See header file for description.
 */
StackType_t *pxPortInitialiseStack( StackType_t *pxTopOfStack, TaskFunction_t pxCode, void *pvParameters )
{
	/* Simulate the stack frame as it would be created by a context switch
	 * interrupt. */

	/* Offset added to account for the way the MCU uses the stack on entry/exit
	 * of interrupts, and to ensure alignment. */
	pxTopOfStack--;

	*pxTopOfStack = portINITIAL_XPSR;	/* xPSR */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) pxCode;	/* PC */
	pxTopOfStack--;
	*pxTopOfStack = ( StackType_t ) portTASK_RETURN_ADDRESS;	/* LR */

	/* Save code space by skipping register initialisation. */
	pxTopOfStack -= 5;	/* R12, R3, R2 and R1. */
	*pxTopOfStack = ( StackType_t ) pvParameters;	/* R0 */

	/* A save method is being used that requires each task to maintain its
	 * own exec return value. */
	pxTopOfStack--;
	*pxTopOfStack = portINITIAL_EXEC_RETURN;

	pxTopOfStack -= 8;	/* R11, R10, R9, R8, R7, R6, R5 and R4. */

	return pxTopOfStack;
}
/*-----------------------------------------------------------*/

static void prvTaskExitError( void )
{
	/* A function that implements a task must not exit or attempt to return to
	 * its caller as there is nothing to return to.  If a task wants to exit it
	 * should instead call vTaskDelete( NULL ).

	 * Artificially force an assert() to be triggered if configASSERT() is
	 * defined, then stop here so application writers can catch the error. */
	configASSERT( uxCriticalNesting == ~0UL );
	portDISABLE_INTERRUPTS();
	for( ;; );
}
/*-----------------------------------------------------------*/

#if 0 //def CFG_CHRE_SUPPORT
static void prvPortStartFirstTask(void) {
	__asm volatile(
		" ldr r0, =0xE000ED08 	\n" /* Use the NVIC offset register to locate the stack. */
		" ldr r0, [r0] 		\n"
		" ldr r0, [r0] 		\n"
		" msr msp, r0		\n" /* Set the msp back to the start of the stack. */
		" cpsie i		\n" /* Globally enable interrupts. */
		" cpsie f		\n"
		" dsb			\n"
		" isb			\n"
		" ldr r3, pxCurrentTCBConst2	\n" /* Restore the context. */
		" ldr r1, [r3]			\n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
		" ldr r0, [r1]			\n" /* The first item in pxCurrentTCB is the task top of stack. */
		" ldmia r0!, {r4-r11, r14}	\n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
		" msr psp, r0 			\n" /* Restore the task stack pointer. */
		" isb 				\n"
		" mov r1, r0                    \n" /* backup r0 to r1 */
		" ldr r14, [r0, #0x18]		\n" /* load pxCode directly */
		" mov r0, #0			\n"
		" msr basepri, r0 		\n"
		" ldr r0, [r1]			\n" /* load pvParameters to r0 */
		" bx r14			\n"
		"                    		\n"
		" .align 2			\n"
		"pxCurrentTCBConst2: .word pxCurrentTCB \n"
	);
}
#else
void vPortSVCHandler( void )
{
	__asm volatile (
					"	ldr	r3, pxCurrentTCBConst2		\n" /* Restore the context. */
					"	ldr r1, [r3]					\n" /* Use pxCurrentTCBConst to get the pxCurrentTCB address. */
					"	ldr r0, [r1]					\n" /* The first item in pxCurrentTCB is the task top of stack. */
					"	ldmia r0!, {r4-r11, r14}		\n" /* Pop the registers that are not automatically saved on exception entry and the critical nesting count. */
					"	msr psp, r0						\n" /* Restore the task stack pointer. */
					"	isb								\n"
					"	mov r0, #0 						\n"
					"	msr	basepri, r0					\n"
					"	bx r14							\n"
					"									\n"
					"	.align 2						\n"
					"pxCurrentTCBConst2: .word pxCurrentTCB				\n"
				);
}
/*-----------------------------------------------------------*/

static void prvPortStartFirstTask( void )
{
	__asm volatile(
					" ldr r0, =0xE000ED08 	\n" /* Use the NVIC offset register to locate the stack. */
					" ldr r0, [r0] 			\n"
					" ldr r0, [r0] 			\n"
					" msr msp, r0			\n" /* Set the msp back to the start of the stack. */
					" cpsie i				\n" /* Globally enable interrupts. */
					" cpsie f				\n"
					" dsb					\n"
					" isb					\n"
					" svc 0					\n" /* System call to start first task. */
					" nop					\n"
				);
}
/*-----------------------------------------------------------*/
#endif
/*
 * See header file for description.
 */
BaseType_t xPortStartScheduler( void )
{
	/* configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to 0.
	See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html */
	configASSERT( configMAX_SYSCALL_INTERRUPT_PRIORITY );

	#if( configASSERT_DEFINED == 1 )
	{
		volatile uint32_t ulOriginalPriority;
		volatile uint8_t * const pucFirstUserPriorityRegister = ( volatile uint8_t * const ) ( portNVIC_IP_REGISTERS_OFFSET_16 + portFIRST_USER_INTERRUPT_NUMBER );
		volatile uint8_t ucMaxPriorityValue;

		/* Determine the maximum priority from which ISR safe FreeRTOS API
		 * functions can be called.  ISR safe functions are those that end in
		 * "FromISR".  FreeRTOS maintains separate thread and ISR API functions
		 * to ensure interrupt entry is as fast and simple as possible.

		 * Save the interrupt priority value that is about to be clobbered. */
		ulOriginalPriority = *pucFirstUserPriorityRegister;

		/* Determine the number of priority bits available.  First write to all
		 * possible bits. */
		*pucFirstUserPriorityRegister = portMAX_8_BIT_VALUE;

		/* Read the value back to see how many bits stuck. */
		ucMaxPriorityValue = *pucFirstUserPriorityRegister;

		/* Use the same mask on the maximum system call priority. */
		ucMaxSysCallPriority = configMAX_SYSCALL_INTERRUPT_PRIORITY & ucMaxPriorityValue;

		/* Calculate the maximum acceptable priority group value for the number
		 * of bits read back. */
		ulMaxPRIGROUPValue = portMAX_PRIGROUP_BITS;
		while( ( ucMaxPriorityValue & portTOP_BIT_OF_BYTE ) == portTOP_BIT_OF_BYTE )
		{
			ulMaxPRIGROUPValue--;
			ucMaxPriorityValue <<= ( uint8_t ) 0x01;
		}

		/* Shift the priority group value back to its position within the AIRCR
		 * register. */
		ulMaxPRIGROUPValue <<= portPRIGROUP_SHIFT;
		ulMaxPRIGROUPValue &= portPRIORITY_GROUP_MASK;

		/* Restore the clobbered interrupt priority register to its original
		 * value. */
		*pucFirstUserPriorityRegister = ulOriginalPriority;
	}
	#endif /* conifgASSERT_DEFINED */

	/* Make PendSV and SysTick the lowest priority interrupts. */
#ifdef CFG_CHRE_SUPPORT
    //portNVIC_SYSPRI1_REG |= portNVIC_SVC_PRI;
#endif
	portNVIC_SYSPRI2_REG |= portNVIC_PENDSV_PRI;
	portNVIC_SYSPRI2_REG |= portNVIC_SYSTICK_PRI;


	/* Start the timer that generates the tick ISR.  Interrupts are disabled
	here already. */
	vPortSetupTimerInterrupt();

	/* Initialise the critical nesting count ready for the first task. */
	uxCriticalNesting = 0;

	/* Ensure the VFP is enabled - it should be anyway. */
	vPortEnableVFP();

	/* Lazy save always. */
	*( portFPCCR ) |= portASPEN_AND_LSPEN_BITS;

	/* Start the first task. */
	prvPortStartFirstTask();

	/* Should never get here as the tasks will now be executing!  Call the task
	 * exit error function to prevent compiler warnings about a static function
	 * not being called in the case that the application writer overrides this
	 * functionality by defining configTASK_RETURN_ADDRESS. */
	prvTaskExitError();

	/* Should not get here! */
	return 0;
}
/*-----------------------------------------------------------*/

void vPortEndScheduler( void )
{
	/* Not implemented in ports where there is nothing to return to.
	Artificially force an assert. */
	configASSERT( uxCriticalNesting == 1000UL );
}
/*-----------------------------------------------------------*/

void vPortEnterCritical( void )
{
	portDISABLE_INTERRUPTS();
	uxCriticalNesting++;

	/* This is not the interrupt safe version of the enter critical function so
	 * assert() if it is being called from an interrupt context.  Only API
	 * functions that end in "FromISR" can be used in an interrupt.  Only assert
	 * if the critical nesting count is 1 to protect against recursive calls if
	 * the assert function also uses a critical section. */
	traceINT_ON();
	if( uxCriticalNesting == 1 )
	{
		configASSERT( ( portNVIC_INT_CTRL_REG & portVECTACTIVE_MASK ) == 0 );
	}
}
/*-----------------------------------------------------------*/

void vPortExitCritical( void )
{
	configASSERT( uxCriticalNesting );
	uxCriticalNesting--;
	traceINT_OFF();
	if( uxCriticalNesting == 0 )
	{
		portENABLE_INTERRUPTS();
	}
}
/*-----------------------------------------------------------*/

void xPortPendSVHandler( void )
{
	/* This is a naked function. */

	__asm volatile
	(
	"	mrs r0, psp							\n"
	"	isb									\n"
	"										\n"
	"	ldr	r3, pxCurrentTCBConst			\n" /* Get the location of the current TCB. */
	"	ldr	r2, [r3]						\n"
	"										\n"
	"	tst r14, #0x10						\n" /* Is the task using the FPU context?  If so, push high vfp registers. */
	"	it eq								\n"
	"	vstmdbeq r0!, {s16-s31}				\n"
	"										\n"
	"	stmdb r0!, {r4-r11, r14}			\n" /* Save the core registers. */
	"										\n"
	"	str r0, [r2]						\n" /* Save the new top of stack into the first member of the TCB. */
	"										\n"
	"	stmdb sp!, {r3}						\n"
	"	mov r0, %0 							\n"
	"	msr basepri, r0						\n"
	"	dsb									\n"
	"   isb									\n"
	"	bl vTaskSwitchContext				\n"
	"	mov r0, #0							\n"
	"	msr basepri, r0						\n"
	"	ldmia sp!, {r3}						\n"
	"										\n"
	"	ldr r1, [r3]						\n" /* The first item in pxCurrentTCB is the task top of stack. */
	"	ldr r0, [r1]						\n"
	"										\n"
	"	ldmia r0!, {r4-r11, r14}			\n" /* Pop the core registers. */
	"										\n"
	"	tst r14, #0x10						\n" /* Is the task using the FPU context?  If so, pop the high vfp registers too. */
	"	it eq								\n"
	"	vldmiaeq r0!, {s16-s31}				\n"
	"										\n"
	"	msr psp, r0							\n"
	"	isb									\n"
	"										\n"
	#ifdef WORKAROUND_PMU_CM001 /* XMC4000 specific errata workaround. */
		#if WORKAROUND_PMU_CM001 == 1
	"			push { r14 }				\n"
	"			pop { pc }					\n"
		#endif
	#endif
	"										\n"
	"	bx r14								\n"
	"										\n"
	"	.align 2							\n"
	"pxCurrentTCBConst: .word pxCurrentTCB	\n"
	::"i"(configMAX_SYSCALL_INTERRUPT_PRIORITY)
	);
}
/*-----------------------------------------------------------*/


/******************************************************************************
******************************************************************************/
UBaseType_t GetPendedTicks(void);


#ifdef CFG_WDT_SUPPORT
extern void mtk_wdt_restart(void);
extern void mtk_wdt_restart_interval(unsigned long long interval);
#endif


/******************************************************************************
* The SysTick runs at the lowest interrupt priority, so when this interrupt
* executes all interrupts must be unmasked.  There is therefore no need to
* save and then restore the interrupt mask value as its value is already
* known.
******************************************************************************/
void xPortSysTickHandler( void )
{
#ifdef CFG_XGPT_SUPPORT
    unsigned long long ullTime_ns;
    TickType_t ulTimerTick, ulSystemTick, ulDelta;
#endif  // CFG_XGPT_SUPPORT

    ( void ) portSET_INTERRUPT_MASK_FROM_ISR();

    /* MTK:put kick wdt in timer hardirq to confirm timing*/
    #ifdef CFG_WDT_SUPPORT
    //mtk_wdt_restart();
    mtk_wdt_restart_interval(5000000000); /* 5s */
    #endif

#ifdef CFG_XGPT_SUPPORT
    ullTime_ns = read_xgpt_stamp_ns();
    ulTimerTick = (TickType_t)(ullTime_ns / 1000000);
    ulSystemTick = xTaskGetTickCount() + (TickType_t)GetPendedTicks();

    /** 0x10000000 is a randomly chosen value.               **/
    /** Any value big enough to detect overflow can be used. **/
    ulDelta = ulTimerTick - ulSystemTick;
    if (ulDelta > 0x10000000) {
        /** Supress system tick count from incrementing. **/
        // PRINTF_E("SysTick: %u, TimerTick: %u\n", ulSystemTick, ulTimerTick);
    } else
#endif  // CFG_XGPT_SUPPORT
    {
        /* Increment the RTOS tick. */
        if( xTaskIncrementTick() != pdFALSE )
        {
            /* A context switch is required.  Context switching is performed in
            the PendSV interrupt.  Pend the PendSV interrupt. */
            portNVIC_INT_CTRL_REG = portNVIC_PENDSVSET_BIT;
        }
    }

    portCLEAR_INTERRUPT_MASK_FROM_ISR( 0 );
}


/*-----------------------------------------------------------*/
extern int g_sleep_flag;

#ifdef CFG_XGPT_SUPPORT
#if configUSE_TICKLESS_IDLE == 1
void sleep_wakeup_timer_fun(void *arg)
{
    /* null, only for wakeup */
}
#endif
#endif
#define WAKEUP_TIMER1 16

#if configUSE_TICKLESS_IDLE == 1

__attribute__((weak)) void vPortSuppressTicksAndSleep( TickType_t xExpectedIdleTime )
{
    uint32_t ulCompleteTickPeriods;
    TickType_t xModifiableIdleTime;
    TickType_t ulAbsCurTick, ulAbsPreTick, ulDiffTick;

    /* Make sure the SysTick reload value does not overflow the counter. */
    if( xExpectedIdleTime > xMaximumPossibleSuppressedTicks )
    {
        xExpectedIdleTime = xMaximumPossibleSuppressedTicks;
    }

    /* Stop the SysTick momentarily.  The time the SysTick is stopped for
     * is accounted for as best it can be, but using the tickless mode will
     * inevitably result in some tiny drift of the time maintained by the
     * kernel with respect to calendar time. */
    portNVIC_SYSTICK_CTRL_REG &= ~portNVIC_SYSTICK_ENABLE_BIT;

    /* Enter a critical section but don't use the taskENTER_CRITICAL()
     * method as that will mask interrupts that should exit sleep mode. */
    __asm volatile( "cpsid i" );

    /* If a context switch is pending or a task is waiting for the scheduler
     * to be unsuspended then abandon the low power entry. */
    if( eTaskConfirmSleepModeStatus() == eAbortSleep )
    {
        /* Restart from whatever is left in the count register to complete
         * this tick period. */
        portNVIC_SYSTICK_LOAD_REG = portNVIC_SYSTICK_CURRENT_VALUE_REG;
        /* Restart SysTick. */
        portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;
        /* Reset the reload register to the value required for normal tick
         * periods. */
        portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
    }
    else
    {
        /* If xExpectedIdleTime less than MCU-wake-up time, it had better
         * loop rather than go to sleep.
         */
        if (xExpectedIdleTime > portMCU_WAKEUP_COST)
        {
            xModifiableIdleTime = xExpectedIdleTime;
            /* Sleep until something happens.  configPRE_SLEEP_PROCESSING() can
             * set its parameter to 0 to indicate that its implementation contains
             * its own wait for interrupt or wait for event instruction, and so wfi
             * should not be executed again.  However, the original expected idle
             * time variable must remain unmodified, so a copy is taken. */
#ifdef CFG_WDT_SUPPORT
            mtk_wdt_disable();
#endif
#ifdef CFG_XGPT_SUPPORT
            platform_set_periodic_timer(sleep_wakeup_timer_fun, NULL, xModifiableIdleTime);
#endif
#ifdef CFG_VCORE_DVFS_SUPPORT
            configPRE_SLEEP_PROCESSING( xModifiableIdleTime );
#endif
            __asm volatile( "dsb" );
            __asm volatile( "wfi" );
            __asm volatile( "isb" );

#ifdef CFG_VCORE_DVFS_SUPPORT
            configPOST_SLEEP_PROCESSING( xModifiableIdleTime );
#endif
#ifdef CFG_WDT_SUPPORT
            mtk_wdt_restart();
            mtk_wdt_enable();
#endif

            ulAbsPreTick = xTaskGetTickCountFromISR();
            /* absolute ticks (ms) from start */
            ulAbsCurTick = read_xgpt_stamp_ns() / 1000000;

            /* Time Guard:
             * Somehow xgpt gives reverse time which makes former given
             * time, xTickCount, begger than the latter given time. In this
             * case sync current time to previous time because it'd better
             * not modify the original tick count.
             */
            if (ulAbsPreTick > ulAbsCurTick) {
                /* if time reverse happens, the difference of previous tick and
                 * current tick would be very small, or otherwise current tick
                 * would be the value for overflow.
                 */
                if (ulAbsPreTick - ulAbsCurTick < portTIME_SKEW_FACTOR)
                    ulAbsCurTick = ulAbsPreTick;
            }

            ulDiffTick = ulAbsCurTick - ulAbsPreTick;

            /** 0x10000000 is a randomly chosen value.               **/
            /** Any value big enough to detect overflow can be used. **/
            if (ulDiffTick > 0x10000000) {
                PRINTF_E("Tick Pre: %lu, Cur: %lu\n", ulAbsPreTick, ulAbsCurTick);
                configASSERT(0);
            }

            /* mtime < difftime: sleep overhead makes current time exeeding the
             *                   expected time.
             * mtime > difftime: being disturbed by some interrupt events.
             * mtime = difftime: wake up on time, so minus one for ready tasks
             *                   to be selected by xTaskIncrementTick.
             */
            ulCompleteTickPeriods =
                (xModifiableIdleTime < ulDiffTick) ? xModifiableIdleTime :
                (xModifiableIdleTime > ulDiffTick) ? ulDiffTick :
                                                     ulDiffTick - 1;

            /* To step to xNextTaskUnblockTime in one go */
            vTaskStepTick( ulCompleteTickPeriods );

            /* The remain tick count says how many ticks exceed unblock time and
             * there must be some tasks who are needed to be waken up.
             */
            while (ulCompleteTickPeriods < ulDiffTick) {
                ulCompleteTickPeriods++;
                /* The increment of ticks is to only increase the variable,
                 * uxPendedTicks. The value will be taken in the following
                 * function, xTaskResumeAll, and then put ready tasks in ready
                 * queue.
                 */
                xTaskIncrementTick();
            }

            /* Restart SysTick so it runs from portNVIC_SYSTICK_LOAD_REG
             * again, then set portNVIC_SYSTICK_LOAD_REG back to its standard
             * value.  The critical section is used to ensure the tick interrupt
             * can only execute once in the case that the reload register is near
             * zero. */
            SCB->ICSR |= portNVIC_PEND_SYSTICK_CLEAR_BIT; /* Clear SYSTICK pending bit */
            portNVIC_SYSTICK_CURRENT_VALUE_REG = 0UL;
            portNVIC_SYSTICK_LOAD_REG = ulTimerCountsForOneTick - 1UL;
        }

        /* Restart SysTick for sleep or busy loop cases in the else scope. */
        portNVIC_SYSTICK_CTRL_REG |= portNVIC_SYSTICK_ENABLE_BIT;
    }

    /* Re-enable interrupts - see comments above the cpsid instruction()
     * above.
     */
    __asm volatile( "cpsie i" );
}

#endif /* #if configUSE_TICKLESS_IDLE */
/*-----------------------------------------------------------*/

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
__attribute__(( weak )) void vPortSetupTimerInterrupt( void )
{
	/* Calculate the constants required to configure the tick interrupt. */
	#if configUSE_TICKLESS_IDLE == 1
	{
		ulTimerCountsForOneTick = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ );
		xMaximumPossibleSuppressedTicks = portMAX_24_BIT_NUMBER / ulTimerCountsForOneTick;
		ulStoppedTimerCompensation = portMISSED_COUNTS_FACTOR / ( configCPU_CLOCK_HZ / configSYSTICK_CLOCK_HZ );
	}
	#endif /* configUSE_TICKLESS_IDLE */

#if ( configUSE_TICKLESS_IDLE != 0 )
	/* Adjust the tick counts to the tick corresponding the current time */
	vTaskStepTick(read_xgpt_stamp_ns() / 1000000);
#endif

	/* Configure SysTick to interrupt at the requested rate. */
	portNVIC_SYSTICK_LOAD_REG = ( configSYSTICK_CLOCK_HZ / configTICK_RATE_HZ ) - 1UL;
	portNVIC_SYSTICK_CTRL_REG = ( portNVIC_SYSTICK_CLK_BIT | portNVIC_SYSTICK_INT_BIT | portNVIC_SYSTICK_ENABLE_BIT );
}
/*-----------------------------------------------------------*/

/* This is a naked function. */
static void vPortEnableVFP( void )
{
	__asm volatile
	(
		"	ldr.w r0, =0xE000ED88		\n" /* The FPU enable bits are in the CPACR. */
		"	ldr r1, [r0]				\n"
		"								\n"
		"	orr r1, r1, #( 0xf << 20 )	\n" /* Enable CP10 and CP11 coprocessors, then save back. */
		"	str r1, [r0]				\n"
		"	bx r14						"
	);
}
/*-----------------------------------------------------------*/

#if( configASSERT_DEFINED == 1 )

	void vPortValidateInterruptPriority( void )
	{
	uint32_t ulCurrentInterrupt;
	uint8_t ucCurrentPriority;

		/* Obtain the number of the currently executing interrupt. */
		__asm volatile( "mrs %0, ipsr" : "=r"( ulCurrentInterrupt ) );

		/* Is the interrupt number a user defined interrupt? */
		if( ulCurrentInterrupt >= portFIRST_USER_INTERRUPT_NUMBER )
		{
			/* Look up the interrupt's priority. */
			ucCurrentPriority = pcInterruptPriorityRegisters[ ulCurrentInterrupt ];

			/* The following assertion will fail if a service routine (ISR) for
			 * an interrupt that has been assigned a priority above
			 * configMAX_SYSCALL_INTERRUPT_PRIORITY calls an ISR safe FreeRTOS API
			 * function.  ISR safe FreeRTOS API functions must *only* be called
			 * from interrupts that have been assigned a priority at or below
			 * configMAX_SYSCALL_INTERRUPT_PRIORITY.

			 * Numerically low interrupt priority numbers represent logically high
			 * interrupt priorities, therefore the priority of the interrupt must
			 * be set to a value equal to or numerically *higher* than
			 * configMAX_SYSCALL_INTERRUPT_PRIORITY.

			 * Interrupts that	use the FreeRTOS API must not be left at their
			 * default priority of	zero as that is the highest possible priority,
			 * which is guaranteed to be above configMAX_SYSCALL_INTERRUPT_PRIORITY,
			 * and	therefore also guaranteed to be invalid.

			 * FreeRTOS maintains separate thread and ISR API functions to ensure
			 * interrupt entry is as fast and simple as possible.

			 * The following links provide detailed information:
			 * http://www.freertos.org/RTOS-Cortex-M3-M4.html
			 * http://www.freertos.org/FAQHelp.html */
			configASSERT( ucCurrentPriority >= ucMaxSysCallPriority );
		}

		/* Priority grouping:  The interrupt controller (NVIC) allows the bits
		 * that define each interrupt's priority to be split between bits that
		 * define the interrupt's pre-emption priority bits and bits that define
		 * the interrupt's sub-priority.  For simplicity all bits must be defined
		 * to be pre-emption priority bits.  The following assertion will fail if
		 * this is not the case (if some bits represent a sub-priority).

		 * If the application only uses CMSIS libraries for interrupt
		 * configuration then the correct setting can be achieved on all Cortex-M
		 * devices by calling NVIC_SetPriorityGrouping( 0 ); before starting the
		 * scheduler.  Note however that some vendor specific peripheral libraries
		 * assume a non-zero priority group setting, in which cases using a value
		 * of zero will result in unpredicable behaviour. */
		configASSERT( ( portAIRCR_REG & portPRIORITY_GROUP_MASK ) <= ulMaxPRIGROUPValue );
	}

#endif /* configASSERT_DEFINED */

/*-----------------------------------------------------------*/
/* Let user to get this value but not allow to modify it */
UBaseType_t xGetCriticalNesting(void)
{
    return uxCriticalNesting;
}
