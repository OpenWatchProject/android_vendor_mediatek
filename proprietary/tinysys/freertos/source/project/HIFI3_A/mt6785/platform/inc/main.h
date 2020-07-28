/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H
/* Includes ------------------------------------------------------------------*/
#include "stdio.h"

/* Exported macro ------------------------------------------------------------*/
/* Place function into i/dTCM */
#define INTERNAL_FUNCTION  __attribute__ ((section(".iram.text"), rodata_section(".dram.rodata")))
/* Variable with initial value (Non-zero) */
#define INTERNAL_INITIAL_DATA       __attribute__ ((section(".dram.data")))
#define INTERNAL_NON_INITIAL_DATA   __attribute__ ((section(".dram.bss")))
/* Exported functions ------------------------------------------------------- */

typedef enum {
    HW_BOOT = 0,
    ADSP_REBOOT = 1,
}boot_mode_t;

extern unsigned long long g_cpu_clock_hz;

extern void set_uart_switch(unsigned int enable);
__inline__ unsigned int read_ccount(void);

#endif /* __MAIN_H */

