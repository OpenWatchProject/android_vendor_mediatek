/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include "bootloader.h"
#include "mt_partition.h"
#include "kernel/uapi/linux/fs.h"
#include "mt_gpt.h"

//#define GPT_DEBUG
static char *hw_sector_path = NULL;
static char *blk_device = NULL;

static const uint32_t crc32_tab[] = {
      0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
      0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
      0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
      0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
      0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
      0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
      0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
      0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
      0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
      0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
      0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
      0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
      0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
      0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
      0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
      0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
      0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
      0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
      0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
      0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
      0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
      0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
      0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
      0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
      0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
      0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
      0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
      0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
      0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
      0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
      0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
      0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
      0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
      0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
      0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
      0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
      0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
      0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
      0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
      0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
      0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
      0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
      0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
      0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
      0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
      0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
      0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
      0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
      0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
      0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
      0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
      0x2d02ef8dL
   };

static uint32_t __attribute__ ((__pure__))
__efi_crc32(const void *buf, unsigned long len, uint32_t seed)
{
  unsigned long i;
  uint32_t crc32val;
  const unsigned char *s = (const unsigned char *)buf;

  crc32val = seed;
  for (i = 0;  i < len;  i ++)
    {
      crc32val =
	crc32_tab[(crc32val ^ s[i]) & 0xff] ^
	  (crc32val >> 8);
    }
  return crc32val;
}

static inline uint32_t
efi_crc32 (const void *buf, unsigned long len)
{
  return (__efi_crc32 (buf, len, ~0L) ^ ~0L);
}

static void set_blk_device_path(void)
{
   if(blk_device)
       return;
   if (mt_get_phone_type() == FS_TYPE_EMMC) {
       blk_device = strdup(EMMC_DEV_BLK);
   }else if (mt_get_phone_type() == FS_TYPE_UFS) {
       blk_device = strdup(UFS_DEV_BLK);
   } else if(mt_get_phone_type() == FS_TYPE_MNTL) {
        blk_device = strdup(MNTL_DEV_BLK);
   }
}

static void set_hw_sector_size_path(void)
{
   if(hw_sector_path)
       return;
   if (mt_get_phone_type() == FS_TYPE_EMMC) {
       hw_sector_path = strdup(EMMC_HW_SECTOR_SIZE);
   }else if (mt_get_phone_type() == FS_TYPE_UFS) {
       hw_sector_path = strdup(UFS_HW_SECTOR_SIZE);
   } else if (mt_get_phone_type() == FS_TYPE_MNTL) {
       hw_sector_path = strdup(MNTL_HW_SECTOR_SIZE);
   }
}

static int mt_get_sector_size(int *sector_size)
{
    FILE *fp = NULL;
    char buf[128];
    fp = fopen(hw_sector_path, "r");
    if (fp) {
        fgets(buf, sizeof(buf), fp);
        if (sscanf(buf, "%d", sector_size) != 1) {
            printf("find sector size fail\n");
        }
        fclose(fp);
        return 0;
    } else {
        printf("open %s fail\n",hw_sector_path);
        return 1;
    }
}

static int mt_gpt_get_part_entry(GuidPartitionTableHeader_t *pgpt_header, char** part_entry, int sector_size)
{
    char *ptr = NULL;
    int fd = 0;
    uint32_t crc32_k, crc32_c;

    fd = open(blk_device, O_RDONLY);
    if (fd < 0) {
        printf("open %s fail\n", blk_device);
        return 1;
    }

    uint64_t len = 2 * sector_size;
    ptr = (char *)malloc(len);
    if (ptr == NULL) {
        close(fd);
        printf("malloc fail\n");
        return 1;
    }

    memset(ptr, 0, len);
    if (read(fd, ptr, (ssize_t)len) != (ssize_t)len) {
        printf("read sector fail\n");
        free(ptr);
        close(fd);
        return 1;
    }
    memcpy(pgpt_header, &ptr[sector_size], sizeof(*pgpt_header));
    free(ptr);
#ifdef GPT_DEBUG
    printf("PGPT:\n");
    printf("Header CRC=0x%08x\n", pgpt_header->HeaderCRC32);
    printf("Header Size=%d\n", pgpt_header->HeaderSize);
    printf("Current LBA=%ju\n", pgpt_header->MyLBA);
    printf("Backup LBA=%ju\n", pgpt_header->AlternateLBA);
    printf("First usable LBA=%ju\n", pgpt_header->FirstUsableLBA);
    printf("Last usable LBA=%ju\n", pgpt_header->LastUsableLBA);
    printf("Starting PE LBA=%ju\n", pgpt_header->PartitionEntryLBA);
    printf("Number of PE=%d\n", pgpt_header->NumberOfPartitionEntries);
    printf("Size of PE=%d\n", pgpt_header->SizeOfPartitionEntry);
    printf("PE CRC=0x%08x\n", pgpt_header->PartitionEntryArrayCRC32);
#endif

    crc32_k = pgpt_header->HeaderCRC32;
    pgpt_header->HeaderCRC32 = 0;
    crc32_c = efi_crc32(pgpt_header, pgpt_header->HeaderSize);
    if (crc32_k != crc32_c) {
        printf("crc check fail k:%08x h:%08x\n", crc32_k,  crc32_c);
        close(fd);
        return 1;
    }
    pgpt_header->HeaderCRC32 = crc32_k;

    if (lseek64(fd, pgpt_header->PartitionEntryLBA * sector_size, SEEK_SET) == -1) {
        printf("leesk %ju fail\n", pgpt_header->PartitionEntryLBA * sector_size);
        close(fd);
        return 1;
    }

    len = ((uint64_t) pgpt_header->NumberOfPartitionEntries * (uint64_t)pgpt_header->SizeOfPartitionEntry);
    if ((len >> 32) > 0) {
        printf("currently, we do not support allocate buffer size larger than 32-bit\n");
        close(fd);
        return 1;
    }
    *part_entry = (char *)malloc(len);

    if (*part_entry == NULL) {
        close(fd);
        printf("malloc fail\n");
        return 1;
    }

    if (read(fd, *part_entry, (ssize_t)len) != (ssize_t)len) {
        printf("read PE fail\n");
        close(fd);
        free(*part_entry);
        return 1;
    }

    crc32_c = efi_crc32(*part_entry, len);
    if (pgpt_header->PartitionEntryArrayCRC32 != crc32_c) {
        printf("pentry crc check fail k:%08x h:%08x\n",
               pgpt_header->PartitionEntryArrayCRC32,  crc32_c);
        close(fd);
        free(*part_entry);
        return 1;
    }

    close(fd);
    return 0;
}


static int mt_gpt_update_part_entry(GuidPartitionTableHeader_t *pgpt_header, char** part_entry, int sector_size)
{
    int fd = 0;
    int write_len = 0;
    GuidPartitionTableHeader_t sgpt_header;

    if (pgpt_header == NULL) {
        printf("invalid gpt partition table header!\n");
        return 1;
    }

    fd = open(blk_device, O_RDWR | O_SYNC);
    if (fd < 0) {
        printf("open %s fail\n", blk_device);
        return 1;
    }

    uint64_t len = (uint64_t)pgpt_header->NumberOfPartitionEntries * (uint64_t)pgpt_header->SizeOfPartitionEntry;
    pgpt_header->PartitionEntryArrayCRC32 = efi_crc32(*part_entry, len);
#ifdef GPT_DEBUG
    printf("PE CRC changed=0x%08x\n", pgpt_header->PartitionEntryArrayCRC32);
#endif

    pgpt_header->HeaderCRC32 = 0;
    pgpt_header->HeaderCRC32 = efi_crc32(pgpt_header, pgpt_header->HeaderSize);
#ifdef GPT_DEBUG
    printf("PGTP Header CRC changed=0x%08x\n", pgpt_header->HeaderCRC32);
#endif

    //SGPT
    if (lseek64(fd, pgpt_header->AlternateLBA * sector_size, SEEK_SET) == -1) {
        printf("leesk %ju fail\n", pgpt_header->AlternateLBA * sector_size);
        close(fd);
        return 1;
    }

    if (read(fd, &sgpt_header, sizeof(sgpt_header)) != sizeof(sgpt_header)) {
        printf("read SGPT header fail\n");
        close(fd);
        return 1;
    }

#ifdef GPT_DEBUG
    printf("SGPT:\n");
    printf("Header CRC=0x%08x\n", sgpt_header.HeaderCRC32);
    printf("Current LBA=%ju\n", sgpt_header.MyLBA);
    printf("Backup LBA=%ju\n", sgpt_header.AlternateLBA);
    printf("First usable LBA=%ju\n", sgpt_header.FirstUsableLBA);
    printf("Last usable LBA=%ju\n", sgpt_header.LastUsableLBA);
    printf("Starting PE LBA=%ju\n", sgpt_header.PartitionEntryLBA);
    printf("Number of PE=%d\n", sgpt_header.NumberOfPartitionEntries);
    printf("Size of PE=%d\n", sgpt_header.SizeOfPartitionEntry);
    printf("PE CRC=0x%08x\n", sgpt_header.PartitionEntryArrayCRC32);
#endif
    sgpt_header.PartitionEntryArrayCRC32 = pgpt_header->PartitionEntryArrayCRC32;

    sgpt_header.HeaderCRC32 = 0;
    sgpt_header.HeaderCRC32 = efi_crc32(&sgpt_header, sgpt_header.HeaderSize);
#ifdef GPT_DEBUG
    printf("SGTP Header CRC changed=0x%08x\n", sgpt_header.HeaderCRC32);
#endif

    //update PGPT header
    if (lseek64(fd, pgpt_header->MyLBA * sector_size, SEEK_SET) == -1) {
        printf("leesk %ju fail\n", pgpt_header->MyLBA * sector_size);
        close(fd);
        return 1;
    }

    if ((write_len = write(fd, pgpt_header, pgpt_header->HeaderSize)) != (int)pgpt_header->HeaderSize) {
        printf("write PGPT fail %d (%s)\n", write_len, strerror(errno));
        close(fd);
        return 1;
    }

    //update PPE
    if (lseek64(fd, pgpt_header->PartitionEntryLBA * sector_size, SEEK_SET) == -1) {
        printf("leesk %ju fail\n", pgpt_header->PartitionEntryLBA * sector_size);
        close(fd);
        return 1;
    }

    if ((write_len = write(fd, *part_entry, (ssize_t)len)) != (ssize_t)len) {
        printf("write PPE fail %d (%s)\n", write_len, strerror(errno));
        close(fd);
        return 1;
    }

    //update SPE
    if (lseek64(fd, sgpt_header.PartitionEntryLBA * sector_size, SEEK_SET) == -1) {
        printf("leesk %ju fail\n", sgpt_header.PartitionEntryLBA * sector_size);
        close(fd);
        return 1;
    }

    if ((write_len = write(fd, *part_entry, (ssize_t)len)) != (ssize_t)len) {
        printf("write SPE fail %d (%s)\n", write_len, strerror(errno));
        close(fd);
        return 1;
    }

    //update SGPT header
    if (lseek64(fd, sgpt_header.MyLBA * sector_size, SEEK_SET) == -1) {
        printf("leesk %ju fail\n", sgpt_header.MyLBA * sector_size);
        close(fd);
        return 1;
    }

    if ((write_len = write(fd, &sgpt_header, sgpt_header.HeaderSize)) != (int)sgpt_header.HeaderSize) {
        printf("write SGPT fail %d (%s)\n", write_len, strerror(errno));
        close(fd);
        return 1;
    }
    close(fd);
    sync();
    return 0;
}

int mt_gpt_update_active_part(const char* partition_name, int is_active)
{
    char *part_entry = NULL;
    GuidPartitionTableHeader_t pgpt_header;
    GuidPartitionEntry_t *pe;
    int sector_size = 0;
    unsigned int i = 0;

    // initial path
    set_blk_device_path();
    set_hw_sector_size_path();

    // get sector size
    if (mt_get_sector_size(&sector_size) != 0) {
        printf("Get sector size fail!\n");
        return 1;
    }

    // read partition entry
    if (mt_gpt_get_part_entry(&pgpt_header, &part_entry, sector_size) > 0) {
        printf("Get partition entry fail!\n");
        return 1;
    }
    // Set active bit for corresponding partition entry
    pe = (GuidPartitionEntry_t *)part_entry;
    for (i = 0; i < pgpt_header.NumberOfPartitionEntries; i++, pe++) {
        unsigned int j;
        char name[37];
#ifdef GPT_DEBUG
        printf("Partition Entry %d\n", i + 1);
        printf("\tFirst LBA=%ju\n", pe->StartingLBA);
        printf("\tLast LBA=%ju\n", pe->EndingLBA);
#endif
        for (j = 0; j < 72 / sizeof(efi_char16_t); j++) {
            name[j] = (uint16_t)pe->PartitionName[j];
        }
        name[j] = 0;
#ifdef GPT_DEBUG
        printf("\tName=%s\n", name);
#endif

        if (!strcmp(name, partition_name))
            break;
    }

    if (i >= pgpt_header.NumberOfPartitionEntries) {
        printf("partition %s not found!\n", partition_name);
        if (part_entry)
            free(part_entry);
        return 1;
    }
#ifdef GPT_DEBUG
    printf("set %s active bit to %d\n", partition_name, is_active);
    printf("Original active: %d, new active: %d\n", (unsigned int)pe->Attributes.LegacyBIOSBootable, is_active);
#endif
    pe->Attributes.LegacyBIOSBootable = is_active;

    // Write partition entry to gpt
    mt_gpt_update_part_entry(&pgpt_header, &part_entry, sector_size);

    // resource release
    if (part_entry)
        free(part_entry);
    return 0;
}
