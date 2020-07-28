/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
#ifndef MT_PMT_H_
#define MT_PMT_H_

#include "mt_partition.h"

#define MAX_PARTITION_NAME_LEN 64

#define PMT_MAGIC              'p'
#define PMT_READ               _IOW(PMT_MAGIC, 1, int)
#define PMT_WRITE              _IOW(PMT_MAGIC, 2, int)
#define PMT_UPDATE             _IOW(PMT_MAGIC, 4, int)
#define MEMGETINFO             _IOR('M', 1, struct mtd_info_user)
#define PMT_VERSION            _IOW(PMT_MAGIC, 3, int)
#define PT_SIG                 0x50547632
#define PT_SIG_v3              0x50547633


typedef struct _DM_PARTITION_INFO
{
    char part_name[MAX_PARTITION_NAME_LEN];             /* the name of partition */
    unsigned int start_addr;                            /* the start address of partition */
    unsigned int part_len;                              /* the length of partition */
    unsigned char part_visibility;                      /* part_visibility is 0: this partition is hidden and CANNOT download */
    /* part_visibility is 1: this partition is visible and can download */
    unsigned char dl_selected;                          /* dl_selected is 0: this partition is NOT selected to download */
    /* dl_selected is 1: this partition is selected to download */
} DM_PARTITION_INFO;


typedef struct {
    unsigned int pattern;
    unsigned int part_num;                              /* The actual number of partitions */
    DM_PARTITION_INFO part_info[MAX_PARTITION_NUM];
} DM_PARTITION_INFO_PACKET;

typedef struct _DM_PARTITION_INFO_x
{
    char part_name[MAX_PARTITION_NAME_LEN];             /* the name of partition */
    unsigned long long start_addr;                      /* the start address of partition */
    unsigned long long part_len;                        /* the length of partition */
    unsigned char part_visibility;                      /* part_visibility is 0: this partition is hidden and CANNOT download */
    /* part_visibility is 1: this partition is visible and can download */
    unsigned char dl_selected;                          /* dl_selected is 0: this partition is NOT selected to download */
    /* dl_selected is 1: this partition is selected to download */
} DM_PARTITION_INFO_x;

typedef struct {
    unsigned int pattern;
    unsigned int part_num;                              /* The actual number of partitions */
    DM_PARTITION_INFO_x part_info[MAX_PARTITION_NUM];
} DM_PARTITION_INFO_PACKET_x;

#define PART_ATTR_FTL_IMG 0x00000001
#define PART_ATTR_ACTIVE_BIT 0x00000004
struct extension {
    unsigned int type;
    unsigned int attr;
};

typedef struct {
    unsigned char name[MAX_PARTITION_NAME_LEN];
    unsigned long long size;
    union {
        unsigned long long part_id;
        struct extension ext;
    };
    unsigned long long offset;
    unsigned long long mask_flags;
} pt_resident_nand;

typedef struct {
    unsigned char name[MAX_PARTITION_NAME_LEN];
    unsigned long size;
    unsigned long offset;
    unsigned long mask_flags;
} pt_resident_nand_32;

typedef struct _DM_PARTITION_INFO_64
{
    char part_name[MAX_PARTITION_NAME_LEN];             /* the name of partition */
    unsigned long long start_addr;                            /* the start address of partition */
    unsigned long long part_len;                              /* the length of partition */
    unsigned char part_visibility;                      /* part_visibility is 0: this partition is hidden and CANNOT download */
    /* part_visibility is 1: this partition is visible and can download */
    unsigned char dl_selected;                          /* dl_selected is 0: this partition is NOT selected to download */
    /* dl_selected is 1: this partition is selected to download */
} DM_PARTITION_INFO_64;


typedef struct {
    unsigned int pattern;
    unsigned int part_num;                              /* The actual number of partitions */
    DM_PARTITION_INFO_64 part_info[MAX_PARTITION_NUM];
} DM_PARTITION_INFO_PACKET_64;

typedef struct
{
    unsigned char name[MAX_PARTITION_NAME_LEN];
    unsigned long long size;
    unsigned long long offset;
    unsigned long long mask_flags;
} pt_resident_emmc;

typedef struct
{
    unsigned char name[MAX_PARTITION_NAME_LEN];
    unsigned long long size;
    unsigned long long part_id;
    unsigned long long offset;
    unsigned long long mask_flags;
} pt_resident_combo_emmc;

struct DM_PARTITION_x {
    char part_name[MAX_PARTITION_NAME_LEN];     /* the name of partition */
    unsigned long long part_id;                 /* the region of partition */
    unsigned long long start_addr;              /* the start address of partition */
    unsigned long long part_len;                /* the length of partition */
    unsigned char visible;                      /* visible is 0: this partition is hidden and NOT downloadable */
                                                /* visible is 1: this partition is visible and downloadable */
    unsigned char dl_selected;                  /* dl_selected is 0: this partition is NOT selected to download */
                                                /* dl_selected is 1: this partition is selected to download */
};

struct DM_PARTITION_PACKET_x {
    unsigned int pattern;
    unsigned int part_num;                      /* The actual number of partitions */
    struct DM_PARTITION_x part_info[MAX_PARTITION_NUM];
};

int update_pmt_nand(void);
int update_pmt_combo_emmc(part_info_t *part_scatter[], int part_num);
int update_pmt_emmc(part_info_t *part_scatter[], int part_num);
int update_pmt_emmc(part_info_t *part_scatter[], int part_num);
int get_pmt_nand(pt_resident_nand *pt);
int get_pmt_version(void);
int get_part_size_changed(void);
void copy_part_scatter_dup(part_info_t *part_scatter[], int mtd_cnt);
int mt_pmt_update_active_part(const char* partition_name, int is_active);

#endif
