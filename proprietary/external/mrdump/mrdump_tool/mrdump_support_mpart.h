#pragma once

/* for DPART(Dedicated Partition) solution */
#define MRDUMP_MPART_PARTITION "/dev/block/platform/bootdevice/by-name/mrdump"
#define MRDUMP_MPART_START_OFFSET 4096
#define MAX_READ_BLK 64
#define BLK_SIZE 4096
#define MRDUMP_PARTITION_EXSPACE MAX_READ_BLK*BLK_SIZE

/* functions */
int mrdump_check_partition(void);
bool mrdump_file_fetch_zip_coredump_partition(const char *outfile);
