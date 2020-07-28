/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
#ifndef MT_PARTITION_H_
#define MT_PARTITION_H_

#include <stdint.h>
#include <ziparchive/zip_archive.h>

#define PRELOADER_PART "/dev/block/mmcblk0boot0"
#define PRELOADER2_PART "/dev/block/mmcblk0boot1"
#define UFS_PRELOADER_PART "/dev/block/sda"
#define UFS_PRELOADER2_PART "/dev/block/sdb"
#define EMMC_PART_GPT_PREFIX     "/dev/block/platform/bootdevice"
#define UFS_PART_GPT_PREFIX     "/dev/block/platform/bootdevice"

#define MAX_PARTITION_NUM (128)
#define PTENT_BUF_SIZE 64
#define OTA_RESULT_OFFSET    (2560)

#define MNTL_BLOCK_DEV_PATH "/sys/class/block/mntlblk_d"
#define EMMC_BLOCK_DEV_PATH "/sys/class/block/mmcblk0p"
#define UFS_BLOCK_DEV_PATH "/sys/class/block/sdc"
#define PARTITION_BLOCK_SIZE 512

#define DATA_UBI_EB_SIZE "/sys/class/ubi/ubi0/ubi0_0/usable_eb_size"
#define DATA_UBI_DATA_BYTES "/sys/class/ubi/ubi0/ubi0_0/data_bytes"
#define DATA_UBI_MIN_SIZE "/sys/class/ubi/ubi0/min_io_size"
#define DATA_UBI_DEVICE "/dev/ubi0_0"

enum mt_device_type {
     FS_TYPE_MTD = 0,
     FS_TYPE_EMMC,
     FS_TYPE_UFS,
     FS_TYPE_MNTL,
     FS_TYPE_UNKNOWN,
     FS_TYPE_INIT
};

typedef struct part_info_t {
    uint64_t offset;
    char name[PTENT_BUF_SIZE];
} part_info_t;

typedef struct part_detail_info {
    uint64_t part_start_addr;
    uint64_t part_size;
    int part_index;
    char part_name[PTENT_BUF_SIZE];
} part_detail_info;

int get_partition_index_by_name(part_info_t *part_scatter[], int part_num, const char *partition_name);
int force_upgrade(bool is_gpt, part_info_t *part_scatter[], part_info_t *part_mtd[]  ,int part_num);

int get_combo_emmc(void);
int get_emmc_phone(void);
int get_has_fat(void);
int get_MLC_case(void);
void set_MLC_case(int value);

uint64_t hex2ulong(char *x);

void set_phone_expdb_succeed(int value);
int get_phone_expdb_succeed(void);

void set_scatter_expdb_succeed(int value);
int get_scatter_expdb_succeed(void);

void get_partition_blk_dev(const char *name, char *blk);
uint64_t get_partition_size(const char *name);

int get_partition_info(part_info_t *part_mtd[], int *part_num);
int get_partition_info_mlc(part_info_t *part_mtd[], int *part_num, uint64_t *total_size);
int get_partition_info_mntl(part_info_t *part_mtd[], int *part_num);
int get_partition_info_from_scatter(part_info_t *part_scatter[], int *part_num_scatter, ZipArchiveHandle zip);


int parse_partition_info(const char *p_name, part_detail_info *part_info);
int parse_partition_info_by_index(int part_index, part_detail_info *part_info);

bool check_partition_layout(part_info_t *part_scatter[], int part_num, bool is_gpt);
void mt_init_partition_type(void);
enum  mt_device_type mt_get_phone_type(void);
bool mt_is_support_gpt(void);
char* get_partition_path(const char *partition);
int set_ota_result(int result, int offset);
#endif
