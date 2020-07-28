/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef ANDROID_POWERHALWRAP_H
#define ANDROID_POWERHALWRAP_H

__BEGIN_DECLS
extern int PowerHal_Wrap_mtkPowerHint(uint32_t hint, int32_t data);
extern int PowerHal_Wrap_mtkCusPowerHint(uint32_t hint, int32_t data);
extern int PowerHal_Wrap_querySysInfo(uint32_t cmd, int32_t param);
extern int PowerHal_Wrap_scnReg(void);
extern int PowerHal_Wrap_scnConfig(int32_t hdl, int32_t cmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4);
extern int PowerHal_Wrap_scnUnreg(int32_t hdl);
extern int PowerHal_Wrap_scnEnable(int32_t hdl, int32_t timeout);
extern int PowerHal_Wrap_scnDisable(int32_t hdl);
extern int PowerHal_Wrap_scnUltraCfg(int32_t hdl, int32_t ultracmd, int32_t param1, int32_t param2, int32_t param3, int32_t param4);
extern int PowerHal_Wrap_setSysInfo(int32_t type, const char *data);
__END_DECLS

#endif // ANDROID_POWERHALWRAP_H
