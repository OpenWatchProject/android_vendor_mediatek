/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

#include <string.h>
#include <errno.h>
#include <fs_mgr.h>
#include <android-base/logging.h>
#include "common.h"
#include "otautil/roots.h"
#include "install/install.h"
#include "mt_install.h"
#include "mt_pmt.h"
#include "bootloader.h"
#include "mt_check_partition.h"

/* Setup backup restore check_part_size() before installing the package
   return values
   0: operation successful.
   1: there's some error and caller really_install_package() shall return "ret".
*/
int mt_really_install_package_check_part_size(int &ret, const char *path, ZipArchiveHandle zip)
{
        if (path == nullptr || path[0] == '\0') {
            LOG(ERROR) << "Invalid path ";
            ret=INSTALL_ERROR;
            return 1;
        }

        int retval = check_part_size(zip);
        if (retval != CHECK_OK) {
            std::string err;
            if (!clear_bootloader_message(&err)) {
                LOG(ERROR) << "Failed to clear BCB message: " << err;
            }
            ret=INSTALL_ERROR;
            return 1;
        }
    return 0;
}
