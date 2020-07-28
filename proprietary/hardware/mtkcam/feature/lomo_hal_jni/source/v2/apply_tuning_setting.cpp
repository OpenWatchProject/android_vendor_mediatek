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
    
    dip_x_reg_t* ppIspPhyRegLocal = (dip_x_reg_t*)ppIspPhyReg;
    
    ppIspPhyRegLocal->DIP_X_GGM_CTRL.Bits.GGM_END_VAR = 1023;
    ppIspPhyRegLocal->DIP_X_GGM_CTRL.Bits.GGM_RMP_VAR = 4099;
    
    return 0;
}
