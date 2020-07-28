#pragma once

/* Map size for ZeroPage */
#define F2FS_MAPUNIT 8
#define F2FS_MAPSIZE (1024*1024*F2FS_MAPUNIT)

/* for IOCTL */
#define F2FS_IOCTL_MAGIC        0xf5
#define F2FS_IOC_SET_PIN_FILE   _IOW(F2FS_IOCTL_MAGIC, 13, __u32)
#define F2FS_IOC_GET_PIN_FILE   _IOR(F2FS_IOCTL_MAGIC, 14, __u32)

/* Function Prototypes */
bool mount_as_f2fs(const char *mountp);
bool f2fs_fallocate(const char *allocfile, uint64_t allocsize);
