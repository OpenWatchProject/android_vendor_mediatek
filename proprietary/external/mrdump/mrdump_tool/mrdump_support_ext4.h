#pragma once

/* debug on/off */
//#define MRDUMP_DEBUG

/* Variables defined */
#define AE_DUMPSYS_DATA_PATH        "/data/vendor/dumpsys"
#define MRDUMP_EXT4_NEW_FILE        true
#define MRDUMP_EXT4_OLD_FILE        false
#define MRDUMP_EXT4_PARA_LBAOOO     "/sys/module/mrdump/parameters/lbaooo"
#define MRDUMP_EXT4_PROC_MOUNTS     "/proc/self/mounts"
#define MRDUMP_EXT4_MOUNT_POINT     "/data"
#define MRDUMP_EXT4_ALLOCATE_FILE   AE_DUMPSYS_DATA_PATH"/mrdump_preallocated"
#define MRDUMP_EXT4_MIN_ALLOCATE    256
#define MRDUMP_EXT4_BLKSIZE         4096
#define MRDUMP_EXT4_MAX_CONTINUE    64
#define MRDUMP_EXT4_EXSPACE         (MRDUMP_EXT4_BLKSIZE*MRDUMP_EXT4_MAX_CONTINUE)
#define MRDUMP_EXT4_LBA_PER_BLOCK   1022

/*
 * v1: support allocate size > 4G
 * v2: support timestamp
 */
#define MRDUMP_PAF_VERSION 0x0002

#define MRDUMP_PAF_INFO_LBA      4
#define MRDUMP_PAF_ADDR_LBA      8
#define MRDUMP_PAF_ALLOCSIZE    12
#define MRDUMP_PAF_COREDUMPSIZE 20
#define MRDUMP_PAF_TIMESTAMP    28
#define MRDUMP_PAF_CRC32        36
#define MRDUMP_LBA_DATAONLY     MRDUMP_PAF_CRC32
#define MRDUMP_PAF_TOTAL_SIZE   40

typedef enum {
    DEFAULT_DISABLE,
    DEFAULT_HALFMEM,
    DEFAULT_FULLMEM,
    USERDEFINED_MEM
} MRDUMP_DEFAULT_SIZE;

typedef enum {
    BDATA_STATE_CHECK_PASS,
    BDATA_STATE_FILE_ACCESS_ERROR,
    BDATA_STATE_BLOCK_HEADER_ERROR,
    BDATA_STATE_BLOCK_DATA_ERROR,
} MRDUMP_BDATA_STATE;

/* for chattr */
#define FS_IOC_GETFLAGS                 _IOR('f', 1, long)
#define FS_IOC_SETFLAGS                 _IOW('f', 2, long)
#define FS_SECRM_FL                     0x00000001 /* Secure deletion */
#define FS_IMMUTABLE_FL                 0x00000010 /* Immutable file */

struct mrdump_pafile_info {
    uint32_t info_lba;
    uint32_t addr_lba;
    uint64_t filesize;
    uint64_t coredump_size;
    uint64_t timestamp;
};

struct __attribute__((__packed__)) marked_block_data {
    uint32_t lba;
    uint64_t zero_padding[510];
    uint64_t timestamp;
    uint32_t crc;
};

/* Function Prototypes */
void mrdump_file_set_maxsize(int mrdump_size);
void mrdump_file_setup(bool reset);
bool mrdump_file_get_info(const char *allocfile, struct mrdump_pafile_info *info);
uint64_t mrdump_file_default_filesize(void);
bool mrdump_file_fetch_zip_coredump(const char *outfile);
bool mount_as_ext4(const char *mountp);
