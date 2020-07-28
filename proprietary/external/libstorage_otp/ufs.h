#ifndef __UFS_H_
#define __UFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "storage_otp.h"

int ufs_storage_init(struct otp *otp_device);

#ifdef __cplusplus
}
#endif

#endif /* __UFS_H_ */

