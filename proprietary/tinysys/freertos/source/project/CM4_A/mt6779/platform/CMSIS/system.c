#include <platform_mtk.h>

uint32_t SystemCoreClock = 26000000;

void SystemInit(void)
{
  /* FPU settings ------------------------------------------------------------*/
  #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */
  #endif

  SCB->SHCSR |= (1UL << SCB_SHCSR_MEMFAULTENA_Pos);
  /* Configure the Vector Table location add offset address ------------------*/
  // set by loader
  // SCB->VTOR = ((uint32_t)0x0);
}
