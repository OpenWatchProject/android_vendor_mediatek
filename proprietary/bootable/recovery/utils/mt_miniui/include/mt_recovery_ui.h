/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

#include "recovery_ui/device.h"
#include "recovery_ui/screen_ui.h"

class MTK_Device : public Device
{
public:
  MTK_Device(ScreenRecoveryUI* ui) : Device(ui) {}
  bool PostWipeData();

private:
  bool has_nvdata = false;
};

Device* make_device() {
  return new MTK_Device(new ScreenRecoveryUI);
}
