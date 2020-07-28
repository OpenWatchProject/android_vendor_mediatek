#include "flash_duty.h"
#include "camera_custom_nvram.h"

int FlashDuty::ind2duty(int dutyIndex, int &duty, int &dutyLt,
        int dutyNum, int dutyNumLt, int isDual)
{
    /* verify index */
    if (dutyIndex < 0 || dutyIndex >= FLASH_CUSTOM_MAX_DUTY_NUM)
        goto err_exit;

    /* convert index to duty */
    if (isDual) {
        if (dutyIndex >= (dutyNum + 1) * (dutyNumLt + 1))
            goto err_exit;
        duty = dutyIndex % (dutyNum + 1) - 1;
        dutyLt = dutyIndex / (dutyNum + 1) - 1;
    } else {
        if (dutyIndex >= dutyNum)
            goto err_exit;
        duty = dutyIndex;
        dutyLt = -1;
    }

    return 0;
err_exit:
    return -1;
}

int FlashDuty::duty2ind(int &dutyIndex, int duty, int dutyLt,
        int dutyNum, int dutyNumLt, int isDual)
{
    /* convert duty to index */
    if (isDual) {
        if (duty < -1 || duty > dutyNum ||
                dutyLt < -1 || dutyLt >= dutyNumLt)
            goto err_exit;
        dutyIndex = (dutyLt + 1) * (dutyNum + 1) + (duty + 1);
    } else {
        if (duty < 0 || duty > dutyNum)
            goto err_exit;
        dutyIndex = duty;
    }

    /* check index */
    if (dutyIndex >= FLASH_CUSTOM_MAX_DUTY_NUM || dutyIndex < 0)
        goto err_exit;

    return 0;
err_exit:
    return -1;
}

