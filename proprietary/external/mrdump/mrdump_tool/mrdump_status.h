#if !defined(__MRDUMP_H__)
#define __MRDUMP_H__

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    MRDUMP_STATUS_NONE,
    MRDUMP_STATUS_FAILED,
    MRDUMP_STATUS_OK,
} MRDUMP_STATUS;

typedef enum {
    MRDUMP_OUTPUT_NULL,
    MRDUMP_OUTPUT_USB,
    MRDUMP_OUTPUT_DATA_FS,
    MRDUMP_OUTPUT_UNUSED, /* VFAT */
    MRDUMP_OUTPUT_PARTITION,
} MRDUMP_OUTPUT;

#define MRDUMP_OUTPUT_EXT4_DATA MRDUMP_OUTPUT_DATA_FS
#define MRDUMP_OUTPUT_PARTITION_DATA MRDUMP_OUTPUT_PARTITION

struct mrdump_status_result {
    uint32_t struct_size;

    MRDUMP_STATUS status;
    MRDUMP_OUTPUT output;
    char mode[32];

    char status_line[128];
    char log_buf[2048];
};

/* public api */
int mrdump_is_supported(void);
bool mrdump_status_clear(void);
bool mrdump_status_get(struct mrdump_status_result *result);

#endif
