#pragma once

/* variables definition */
#define MRDUMP_DATA_FS_NONE    0
#define MRDUMP_DATA_FS_EXT4    1
#define MRDUMP_DATA_FS_F2FS    2
#define MRDUMP_1K              1024
#define MRDUMP_1M              (MRDUMP_1K * MRDUMP_1K)

/* mrdump related */
#define MRDUMP_DATA_PARTITION  "/dev/block/platform/bootdevice/by-name/userdata"
#define MRDUMP_REST_BLOCKS     1024
#define MRDUMP_REST_SPACE      (MRDUMP_1M * MRDUMP_REST_BLOCKS)

/* mrdump flow control */
#define MRDUMP_MAX_BW_REQ      100
#define MRDUMP_MAX_BANDWIDTH   (MRDUMP_1M * MRDUMP_MAX_BW_REQ)

/* Function Prototypes */
void mrdump_close(int fd);
int mrdump_file_is_exist(const char *path);
int mrdump_get_data_os(void);
uint64_t mrdump_get_partition_free_size(const char *mountp);
uint64_t mrdump_get_partition_size(char *fullpath);
char *mrdump_get_device_node(const char *mountp);
