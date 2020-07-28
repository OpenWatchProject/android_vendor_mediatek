#include <stdio.h>
#include <stdarg.h>

#include <drv/isp_reg.h>
#if DIP_REG_NEEDED // defined in Android.mk
    #include <dip_reg.h>
#endif

int lomoApplyTuningSettingExt(void *ppIspPhyReg)
{
    if (ppIspPhyReg == NULL)
    {
        return -1;
    }
    return 0;
}
