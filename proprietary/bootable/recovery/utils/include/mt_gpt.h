/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/
#ifndef MT_GPT_H_
#define MT_GPT_H_

#include "mt_partition.h"
#include <stdint.h>

#define MNTL_DEV_BLK "/dev/block/mntlblk_d"
#define EMMC_DEV_BLK "/dev/block/mmcblk0"
#define UFS_DEV_BLK "/dev/block/sdc"
#define MNTL_HW_SECTOR_SIZE "/sys/block/mntlblk_d/queue/hw_sector_size"
#define EMMC_HW_SECTOR_SIZE "/sys/block/mmcblk0/queue/hw_sector_size"
#define UFS_HW_SECTOR_SIZE "/sys/block/sdc/queue/hw_sector_size"
#define PRELOADER_FORCE_RO  "/sys/block/mmcblk0boot0/force_ro"
#define PRELOADER2_FORCE_RO  "/sys/block/mmcblk0boot1/force_ro"


typedef struct
{
  uint32_t time_low;
  uint16_t time_mid;
  uint16_t time_hi_and_version;
  uint8_t clock_seq_hi_and_reserved;
  uint8_t clock_seq_low;
  uint8_t node[6];
} efi_guid_t;

struct __attribute__ ((packed)) _GuidPartitionTableHeader_t
{
  uint64_t Signature;
  uint32_t Revision;
  uint32_t HeaderSize;
  uint32_t HeaderCRC32;
  uint32_t Reserved1;
  uint64_t MyLBA;
  uint64_t AlternateLBA;
  uint64_t FirstUsableLBA;
  uint64_t LastUsableLBA;
  efi_guid_t DiskGUID;
  uint64_t PartitionEntryLBA;
  uint32_t NumberOfPartitionEntries;
  uint32_t SizeOfPartitionEntry;
  uint32_t PartitionEntryArrayCRC32;
  uint8_t *Reserved2;
};

struct __attribute__ ((packed)) _GuidPartitionEntryAttributes_t
{
  uint64_t RequiredToFunction:1;
  uint64_t NoBlockIOProtocol:1;
  uint64_t LegacyBIOSBootable:1;
  uint64_t Reserved:45;
  uint64_t GuidSpecific:16;
};

typedef uint16_t efi_char16_t;
typedef struct _GuidPartitionEntryAttributes_t GuidPartitionEntryAttributes_t;

struct __attribute__ ((packed)) _GuidPartitionEntry_t
{
  efi_guid_t PartitionTypeGuid;
  efi_guid_t UniquePartitionGuid;
  uint64_t StartingLBA;
  uint64_t EndingLBA;
  GuidPartitionEntryAttributes_t Attributes;
  efi_char16_t PartitionName[72 / sizeof (efi_char16_t)];
};


typedef struct _GuidPartitionTableHeader_t GuidPartitionTableHeader_t;
typedef struct _GuidPartitionEntry_t GuidPartitionEntry_t;
int mt_gpt_update_active_part(const char* partition_name, int is_active);
#endif
