/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

#ifndef MT_ROOTS_H_
#define MT_ROOTS_H_

using android::fs_mgr::Fstab;
using android::fs_mgr::FstabEntry;
using android::fs_mgr::ReadDefaultFstab;

void mt_load_volume_table(Fstab *fstab);
void mt_ensure_dev_ready(const char *mount_point);
#endif

