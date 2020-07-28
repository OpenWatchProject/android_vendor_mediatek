#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <tinyalsa/asoundlib.h>
#include <tinyxml.h>
#include "nxp_extamp_intf.h"

static int nxp_extamp_init(__unused struct SmartPa *smart_pa)
{
    struct mixer *mixer = smart_pa->attribute.mMixer;
    struct mixer_ctl *mixer_ctl;
    int ret = 0;

    ALOGD("%s: begin, mixer: %p\n", __func__, mixer);

    mixer_ctl = mixer_get_ctl_by_name(mixer, "TFA98XX Firmware");
    if (!mixer_ctl) {
        ALOGE("%s: TFA98XX Firmware not invalid ctl name\n", __func__);
    }
    ret = mixer_ctl_set_value(mixer_ctl, 0, 1);
    if (ret < 0)
        ALOGE("%s: TFA98XX Firmware set value fail\n", __func__);

    return 0;
}

int mtk_smartpa_init(struct SmartPa *smart_pa)
{
    smart_pa->ops.init = nxp_extamp_init;

    return 0;
}
