#include <interrupt.h>
#include "ap_awake.h"
#include "wakelock.h"

#include "dvfs_common.h"

#define ADSP_A_AP_AWAKE (ADSP_AP_WAKELOCK_BASE + 0x0000)
#define AP_AWAKE_LOCK_BIT       (0)
#define AP_AWAKE_UNLOCK_BIT     (1)
#define AP_AWAKE_DUMP_BIT       (4)
#define AP_AWAKE_UPDATE_BIT     (5)
#define AP_AWAKE_STATE_BIT      (6)
#define ADSPPLL_UNLOCK_BIT      (8)
#define AP_AWAKE_LOCK_MASK      (0x1 << AP_AWAKE_LOCK_BIT)
#define AP_AWAKE_UNLOCK_MASK    (0x1 << AP_AWAKE_UNLOCK_BIT)
#define AP_AWAKE_DUMP_MASK      (0x1 << AP_AWAKE_DUMP_BIT)
#define AP_AWAKE_UPDATE_MASK    (0x1 << AP_AWAKE_UPDATE_BIT)
#define AP_AWAKE_STATE_MASK     (0x3 << AP_AWAKE_STATE_BIT)
#define ADSPPLL_UNLOCK_MASK     (0x1 << ADSPPLL_UNLOCK_BIT)

#define ADSP_SW_INT0    (0)
#define ADSP_SW_INT1    (1)
#define ADSP_SW_INT2    (2)
#define ADSP_SW_INT3    (3)

static wakelock_t ap_wakelock;

static void ap_awake_irq_handler(void)
{
    ADSP_SW_INT_CLR |= 1 << ADSP_SW_INT1;   /* Clear interrupt as early as possible to let AP leave busy waiting */
    {
        uint32_t reg_val = DRV_Reg32(ADSP_A_AP_AWAKE);
        if (!g_force_adsppll_on) {
            g_adsppll_unlock = (reg_val & ADSPPLL_UNLOCK_MASK) >> ADSPPLL_UNLOCK_BIT;
        }
        DRV_WriteReg32(ADSP_A_AP_AWAKE, reg_val & ~(AP_AWAKE_LOCK_MASK | AP_AWAKE_UNLOCK_MASK | AP_AWAKE_DUMP_MASK | AP_AWAKE_UPDATE_MASK));
        if (reg_val & AP_AWAKE_UPDATE_MASK) {
            wakelock_state_t curr_state = wake_lock_get_state();
            wakelock_state_t next_state = (reg_val & AP_AWAKE_STATE_MASK) >> AP_AWAKE_STATE_BIT;
            if (next_state != curr_state) {
                wake_lock_set_state(next_state);
            }
        }
        if (reg_val & AP_AWAKE_LOCK_MASK) {
            wake_lock_FromISR(&ap_wakelock);
        }
        if (reg_val & AP_AWAKE_UNLOCK_MASK) {
            wake_unlock_FromISR(&ap_wakelock);
        }
        if (reg_val & AP_AWAKE_DUMP_MASK) {
            wake_lock_list_dump();
        }
    }
    return;
}

void ap_awake_init(void)
{
    {   /* Initialize AP Awake register state */
        uint32_t reg_val = WAKELOCK_STATE_NORMAL << AP_AWAKE_STATE_BIT;
        DRV_WriteReg32(ADSP_A_AP_AWAKE, reg_val);
    }
    wake_lock_init(&ap_wakelock, "AP_W");
    request_irq(AP_AWAKE_IRQn, ap_awake_irq_handler, "APA");
    return;
}
