/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <fs_mgr.h>
#include <libgen.h>
#include <android-base/properties.h>
#include <unistd.h>
#include <android-base/logging.h>
#include <ziparchive/zip_archive.h>
#include "mt_check_partition.h"
#include "mt_partition.h"
#include "mt_gpt.h"
#include "otautil/roots.h"

static int phone_expdb_succeed = 0;
static int scatter_expdb_succeed = 0;
static int MLC_case = 0;
static int has_fat = 0;
static int emmc_phone = 0;
static int combo_emmc = 0;
static int mt_is_gpt = -1;
static char *gpt_prefix = NULL;
static char *misc_blk_device_path = NULL;

static enum mt_device_type device_type = FS_TYPE_INIT;

using android::fs_mgr::Fstab;
using android::fs_mgr::FstabEntry;
using android::fs_mgr::ReadDefaultFstab;


static int get_block_deive_path(void)
{
    static Fstab fstab;
    int ret = -1;

    ret = ReadDefaultFstab(&fstab);
    if(!ret) {
        printf("read fstab fail\n");
        ret = -1;
    } else {
        auto v = GetEntryForMountPoint(&fstab, "/misc");
        if (v == nullptr) {
            LOG(ERROR)<<"failed to get block device path by mount point";
            ret = -1;
        } else {
            misc_blk_device_path = strdup(v->blk_device.c_str());
            ret = 0;
        }
    }

    if (ret) {
        char *fn = NULL;

        asprintf(&fn, "%s/by-name/para", EMMC_PART_GPT_PREFIX);

        if (fn) {
            if (access(fn, R_OK) == 0)
                asprintf(&misc_blk_device_path, "%s/by-name/para",
                     EMMC_PART_GPT_PREFIX);
            else
                asprintf(&misc_blk_device_path, "%s/by-name/misc",
                     EMMC_PART_GPT_PREFIX);
            free(fn);

            printf("read fstab fail misc_blk_device_path is %s\n",
               misc_blk_device_path);
	 } else
            printf("%s asprintf failed %d\n", __FUNCTION__, __LINE__);

    }

    return ret;
}

static int get_boot_type(void) {
    int fd;
    size_t s;
    char boot_type[4] = {'0'};

    fd = open("/sys/class/BOOT/BOOT/boot/boot_type", O_RDONLY);
    if (fd < 0) {
        printf("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_type");
        return -1;
    }

    s = read(fd, boot_type, sizeof(boot_type) - 1);
    close(fd);

    if (s <= 0) {
        printf("could not read boot type sys file\n");
        return -1;
    }

    boot_type[s] = '\0';

    return atoi(boot_type);
}

static int get_device_type(void)
{

    /* Try to get misc block device path firstly */
    if(get_block_deive_path() == -1)
        printf("platform device_type: get_block_deive_path failed\n");

    int mt_boot_type = get_boot_type();

    if(mt_boot_type == FS_TYPE_EMMC) {
        printf("platform device_type: EMMC\n");
        device_type = FS_TYPE_EMMC;
        return 0;
    }


    if(mt_boot_type == FS_TYPE_UFS) {
        printf("platform device_type: UFS\n");
        device_type = FS_TYPE_UFS;
        return 0;
    }

    std::string propbuf = android::base::GetProperty("ro.vendor.mntl_support", "");

    if(propbuf == "1") {
        printf("platform device_type: MNTL\n");
        device_type = FS_TYPE_MNTL;
        return 0;
    }

    printf("get device_type from ro.mtk_ufs_booting for ufs devices\n");
    propbuf = android::base::GetProperty("ro.mtk_ufs_booting", "");

    if(propbuf == "1") {
        printf("platform device_type: UFS\n");
        device_type = FS_TYPE_UFS;
        return 0;
    }

    if(mt_boot_type == FS_TYPE_MTD) {
        printf("platform device_type: MTD\n");
        device_type = FS_TYPE_MTD;
        return 0;
    }

    printf("platform device_type is unknown\n");
    printf("platform uses default device_type emmc\n");
    device_type = FS_TYPE_EMMC;
    return 0;
}

int get_partition_index_by_name(part_info_t *part_scatter[], int part_num, const char *partition_name)
{
   int i = 0;
   for (i = 0; i < part_num; i++) {
      if (!strcasecmp(part_scatter[i]->name, partition_name)) {
          return i;
      }
   }
   printf("Error: Can't find partition %s \n",partition_name);
   return -1;
}

int get_combo_emmc(void)
{
  return combo_emmc;
}

int get_emmc_phone(void)
{
  return emmc_phone;
}

int get_has_fat(void)
{
  return has_fat;
}

int get_MLC_case(void)
{
  return MLC_case;
}

void set_MLC_case(int value)
{
  MLC_case = value;
}

uint64_t hex2ulong(char *x) {
    uint64_t n = 0;

    while (*x) {
        switch (*x) {
            case '0': case '1': case '2': case '3': case '4':
            case '5': case '6': case '7': case '8': case '9':
                n = (n << 4) | (*x - '0');
                break;
            case 'a': case 'b': case 'c':
            case 'd': case 'e': case 'f':
                n = (n << 4) | (*x - 'a' + 10);
                break;
            case 'A': case 'B': case 'C':
            case 'D': case 'E': case 'F':
                n = (n << 4) | (*x - 'A' + 10);
                break;
            default:
                return n;
        }
        x++;
    }

    return n;
}

bool mt_is_support_gpt(void) {
    DIR *dir;
    struct dirent *entry;
    char *fn = NULL;
    const char *PART_GPT_PREFIX = NULL;

    if(device_type == FS_TYPE_UFS)
        PART_GPT_PREFIX = UFS_PART_GPT_PREFIX;
    else
        PART_GPT_PREFIX = EMMC_PART_GPT_PREFIX;

    if (mt_is_gpt >= 0)
        return mt_is_gpt;

    if(device_type == FS_TYPE_MTD) {
        mt_is_gpt = 0;
    } else {
        mt_is_gpt = 1;
        gpt_prefix = dirname(misc_blk_device_path);
        printf("%s gpt prefix is %s\n", __FUNCTION__, gpt_prefix);
    }
    // to backward compatible with previous version hardcore gpt path
    asprintf(&fn, "%s/by-name/para", PART_GPT_PREFIX);

    if (fn && access(fn, R_OK) == 0) {
        mt_is_gpt = 1;
        //gpt_prefix = strdup(fn);
        asprintf(&gpt_prefix, "%s/by-name", PART_GPT_PREFIX);
        free(fn);
    } else
        printf("%s asprintf failed %d\n", __FUNCTION__, __LINE__);

    if (!mt_is_gpt) {
        dir = opendir(PART_GPT_PREFIX);
        if (dir) {
            while (!mt_is_gpt &&((entry = readdir(dir)) != NULL)) {
                if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0)) {
                    continue;
                }
                asprintf(&fn, "%s/%s/by-name/para", PART_GPT_PREFIX, entry->d_name);

                if (fn && access(fn, R_OK) == 0) {
                    mt_is_gpt = 1;
                    //gpt_prefix = strdup(fn);
                    asprintf(&gpt_prefix, "%s/%s/by-name", PART_GPT_PREFIX, entry->d_name);
                    free(fn);
                } else
                    printf("%s asprintf failed %d\n", __FUNCTION__, __LINE__);

            }
            closedir(dir);
        }
    }
    printf("is_gpt = %d\n", mt_is_gpt);
    if (mt_is_gpt)
        printf("gpt prefix is %s\n", gpt_prefix);
    return mt_is_gpt;
}

char* get_partition_path(const char *partition) {
    char *fn = NULL;
    const char *dev_name = NULL;
    char blk_dev_name[PATH_MAX];
    int is_gpt = 0;

    printf("%s, input partition is %s\n", __FUNCTION__, partition);
    if ((access(partition, R_OK) == 0) && !strncmp(partition, "/dev", strlen("/dev"))) // already full path from fstab
        return strdup(partition);

    if (mt_is_gpt < 0)
        is_gpt = mt_is_support_gpt();

    if (mt_get_phone_type() == FS_TYPE_EMMC || mt_get_phone_type() == FS_TYPE_UFS) {
        if (!strcmp(partition, "preloader")) {
            if(device_type == FS_TYPE_EMMC)
                fn = strdup(PRELOADER_PART);
            else if(device_type == FS_TYPE_UFS)
                fn = strdup(UFS_PRELOADER_PART);
        } else if (!strcmp(partition, "preloader2")) {
            if(device_type == FS_TYPE_EMMC)
                fn = strdup(PRELOADER2_PART);
            else if(device_type == FS_TYPE_UFS)
                fn = strdup(UFS_PRELOADER2_PART);
        } else {
            if (strstr(partition, "by-name")) { // already gpt path
                const char *mount_point = strrchr(partition, '/');
                asprintf(&fn, "%s%s", gpt_prefix, mount_point);
            } else {
                if (gpt_prefix) {   // is gpt and input is mount_point
                    asprintf(&fn, "%s/%s", gpt_prefix, partition);
                } else {
                    get_partition_blk_dev(partition, blk_dev_name);
                    fn = strdup(blk_dev_name);
                }
            }
        }
        if (fn && access(fn, R_OK) == 0) {
            printf("%s, translate partition to %s\n", __FUNCTION__, fn);
        } else {
            printf("%s, translated partition %s can not access (%s), ignore translated result and return %s\n", __FUNCTION__, fn, strerror(errno), partition);
            free(fn);
            fn = strdup(partition);
        }
    } else {
        // for NAND fstab, device name should be type@name, and this API should return name only
        dev_name = strrchr(partition, '@');
        if (dev_name)
            fn = strdup(dev_name + 1);
        else
            fn = strdup(partition);
        printf("%s, translate partition to %s\n", __FUNCTION__, fn);
    }

    return fn;
}

void get_partition_blk_dev(const char *name, char *blk)
{
    FILE *fp = fopen("/proc/dumchar_info", "r");
    char buf[1024], p_name[PTENT_BUF_SIZE], p_size[PTENT_BUF_SIZE], p_addr[PTENT_BUF_SIZE], p_actname[PTENT_BUF_SIZE];
    unsigned int p_type = 0;

    if (!fp) {
        printf("open /proc/dumchar_info fail\n");
        return;
    }

    while (fgets(buf, sizeof(buf), fp)) {
        if (sscanf(buf, "%63s %63s %63s %d %63s", p_name, p_size, p_addr, &p_type, p_actname) == 5) {
            if (!strcmp(p_name, "otp")) {
                // OTP partition
                continue;
            }
            if (!strcmp(p_name, "bmtpool")) {
                // last partition
                break;
            }
            if ((!strcmp(name, "boot") && !strcmp(p_name, "bootimg"))
                    || (!strcmp(name, "uboot") && !strcmp(p_name, "lk"))
                    || (!strcmp(name, "system") && !strcmp(p_name, "android"))
                    || (!strcmp(name, "userdata") && !strcmp(p_name, "usrdata"))
                )
            {
                    snprintf(blk, sizeof(p_actname), "%s", p_actname);
                    break;
            }
            if (strcmp(p_name, name) == 0) {
                snprintf(blk, sizeof(p_actname), "%s", p_actname);
                break;
            }
        }
    }
    fclose(fp);
    return;
}

uint64_t get_partition_size(const char *name)
{
    bool is_gpt = mt_is_support_gpt();
    part_detail_info part_info;
    if (is_gpt) {

        if(parse_partition_info(name,&part_info))
          return part_info.part_size;
        else
          return 0;
    } else {
        FILE *fp = fopen("/proc/dumchar_info", "r");
        char buf[1024], p_name[PTENT_BUF_SIZE], p_size[PTENT_BUF_SIZE], p_addr[PTENT_BUF_SIZE], p_actname[PTENT_BUF_SIZE];
        unsigned int p_type = 0;

        if (!fp) {
            printf("open /proc/dumchar_info fail\n");
            return 0;
        }

        while (fgets(buf, sizeof(buf), fp)) {
            if (sscanf(buf, "%63s %63s %63s %d %63s", p_name, p_size, p_addr, &p_type, p_actname) == 5) {
                if (!strcmp(p_name, "otp")) {
                    // OTP partition
                    continue;
                }
                if (!strcmp(p_name, "bmtpool")) {
                    // last partition
                    break;
                }
                if (strcmp(p_name, name) == 0) {
                    fclose(fp);
                    return hex2ulong(p_size + 2);
                }
            }
        }
        fclose(fp);
        return 0;
    } // not gpt
}

static uint64_t get_value_from_mmcblk_file(const char *path)
{
   FILE *fp = NULL;
   uint64_t value = 0;
   char buf[256];

   fp = fopen(path, "r");

   if(fp == NULL){
     printf("Error: Open path %s error\n",path);
     return 0;
   }

   fgets(buf, sizeof(buf), fp);
   sscanf(buf,"%255s",buf);
   value = strtoull(buf, NULL, 10);
   value *= PARTITION_BLOCK_SIZE;

   fclose(fp);

   return value;
}

int check_block_dev_path(char* block_dev_path)
{

    if (device_type == FS_TYPE_UFS) {
        strncpy(block_dev_path, UFS_BLOCK_DEV_PATH, PATH_MAX-1);
        block_dev_path[PATH_MAX-1] = '\0';
//        printf("ufs block_dev_path = %s\n",block_dev_path);
        return 0;
    } else if (device_type == FS_TYPE_EMMC) {
        strncpy(block_dev_path, EMMC_BLOCK_DEV_PATH, PATH_MAX-1);
        block_dev_path[PATH_MAX-1] = '\0';
//        printf("emmc block_dev_path = %s\n",block_dev_path);
        return 0;
    } else if (device_type == FS_TYPE_MNTL) {
        strncpy(block_dev_path, MNTL_BLOCK_DEV_PATH, PATH_MAX-1);
        block_dev_path[PATH_MAX-1] = '\0';
//        printf("mntl block_dev_path = %s\n",block_dev_path);
        return 0;
    } else {
        printf("Error: can't find device_type %d\n",device_type);
        return 1;
    }
}

int parse_partition_info_by_index(int part_index, part_detail_info *part_info)
{
   char *uevent_path = NULL, *start_path = NULL, *size_path = NULL, *block_dev_path = NULL;
   FILE *fp_uevent = NULL;
   char *buf = NULL;
   int ret = 0, buf_size = 256;

   uevent_path = (char *)malloc(PATH_MAX);
   start_path = (char *)malloc(PATH_MAX);
   size_path = (char *)malloc(PATH_MAX);
   block_dev_path = (char *)malloc(PATH_MAX);
   buf = (char *)malloc(buf_size);

   if(!uevent_path || !start_path || !size_path || !block_dev_path || !buf) {
       ret = 0;
       printf("parse_partition_info Malloc memory fail\n");
       goto fail_out;
   }
   if(check_block_dev_path(block_dev_path)) {
      printf("check_block_dev_path fail\n");
      ret = 0;
      goto fail_out;
   }

   snprintf(uevent_path, PATH_MAX, "%s%d/uevent", block_dev_path, part_index);
   snprintf(start_path, PATH_MAX, "%s%d/start", block_dev_path, part_index);
   snprintf(size_path, PATH_MAX, "%s%d/size", block_dev_path, part_index);

   fp_uevent = fopen(uevent_path, "r");

    if(fp_uevent == NULL){
      printf("uevent %s is not exist\n",uevent_path);
      ret = 0;
      goto fail_out;
    }

      /* Parse partition name from uevent */
     while(fgets(buf, buf_size, fp_uevent)!= NULL) {
         if(strstr(buf,"PARTNAME")!=NULL){
           sscanf(buf,"PARTNAME=%63s",part_info->part_name);
           ret = 1;
           break;
         }
      }

      fclose(fp_uevent);

      part_info->part_start_addr = get_value_from_mmcblk_file(start_path);
      part_info->part_size = get_value_from_mmcblk_file(size_path);

fail_out:
   if(uevent_path)
       free(uevent_path);
   if(start_path)
       free(start_path);
   if(size_path)
       free(size_path);
   if(block_dev_path)
       free(block_dev_path);
   if(buf)
       free(buf);

   return ret;
}


int parse_partition_info(const char *p_name_index, part_detail_info *part_info)
{
   int i = 1, ret = 0, buf_size = 256;
   FILE *fp_uevent = NULL;
   char *uevent_path = NULL, *start_path = NULL, *size_path = NULL, *block_dev_path = NULL;
   char *uevent_p_name = NULL;
   char *buf = NULL;
   uevent_path = (char *)malloc(PATH_MAX);
   start_path = (char *)malloc(PATH_MAX);
   size_path = (char *)malloc(PATH_MAX);
   block_dev_path = (char *)malloc(PATH_MAX);
   uevent_p_name = (char *)calloc(PTENT_BUF_SIZE, sizeof(char));
   buf = (char *)malloc(buf_size);

   if(!uevent_path || !start_path || !size_path || !block_dev_path || !uevent_p_name || !buf) {
       ret = 0;
       printf("parse_partition_info Malloc memory fail\n");
       goto fail_out;
   }

  if(check_block_dev_path(block_dev_path)) {
      printf("check_block_dev_path fail\n");
      ret = 0;
      goto fail_out;
   }

   for(i=1; i <= MAX_PARTITION_NUM;i++)
   {
      snprintf(uevent_path, PATH_MAX, "%s%d/uevent", block_dev_path,i);
      snprintf(start_path, PATH_MAX, "%s%d/start", block_dev_path,i);
      snprintf(size_path, PATH_MAX, "%s%d/size", block_dev_path,i);

      fp_uevent = fopen(uevent_path, "r");

      if(fp_uevent == NULL){
        printf("Error: Open uevent %s error\n",uevent_path);
        ret = 0;
        break;
      }

      /* Parse partition name from uevent */
      while(fgets(buf, buf_size, fp_uevent)!= NULL) {
         if(strstr(buf,"PARTNAME")!=NULL)  {
           sscanf(buf,"PARTNAME=%63s",uevent_p_name);
           break;
         }
      }

      fclose(fp_uevent);

      /* Find partition by p_name_index then get start addr nad size */
      if(uevent_p_name && !strcasecmp(uevent_p_name,p_name_index)){
         printf("Find partition %s\n",uevent_p_name);

         part_info->part_start_addr = get_value_from_mmcblk_file(start_path);
         part_info->part_size = get_value_from_mmcblk_file(size_path);

         ret = 1;
         break;
      }
      else if(i == MAX_PARTITION_NUM){
        printf("Error: Partition %s is not exist", p_name_index);
        ret = 0;
      }
   }
fail_out:
   if(uevent_path)
       free(uevent_path);
   if(start_path)
       free(start_path);
   if(size_path)
       free(size_path);
   if(block_dev_path)
       free(block_dev_path);
   if(uevent_p_name)
       free(uevent_p_name);
   if(buf)
       free(buf);
   return ret;
}

/* If sucess return 0 */
int get_partition_info_from_scatter(part_info_t *part_scatter[], int *part_num_scatter, ZipArchiveHandle zip)
{
    char *scatter = (char *)"/tmp/scatter";
    ZipString scatter_path("scatter.txt");
    ZipEntry scatter_entry;
    char p_name[32], p_offset[32];
    int part_index = 0, fd = -1, ret = -1;
    FILE *fp = NULL;
    char buf[256];
    part_info_t *part_device_ptr = NULL;

    if (FindEntry(zip, scatter_path, &scatter_entry) != 0) {
        printf("Error: Invalid OTA package, missing scatter\n");
        return 1;
    }

    unlink(scatter);
    fd = creat(scatter, 0755);
    if (fd < 0) {
        printf("Error: Can't make %s\n", scatter);
        goto fail_out;
    }

    ret = ExtractEntryToFile(zip, &scatter_entry, fd);

    if (ret != 0) {
        printf("Error: File system fail, scatter\n");
        goto fail_out;
    }

    fp = fopen(scatter, "r");
    printf("====== Scatter File:\n");

    if (!fp) {
        printf("fopen failed\n");
        ret = 1;
        goto fail_out;
    }

    while (fgets(buf, sizeof(buf), fp)) {
         printf("%s", buf);
         sscanf(buf, "%31s %31s", p_name, p_offset);

         if (!strcasecmp(p_name, "preloader") || !strcasecmp(p_name, "pgpt")) {
             //skip PGPT && preloader
             continue;
         }

         part_device_ptr = (part_info_t *)malloc(sizeof(part_info_t));
         if(part_device_ptr == NULL) {
             printf("Error: malloc memory fail\n");
             ret = 1;
             goto fail_out;
         }

         snprintf(part_device_ptr->name, sizeof(part_device_ptr->name), "%s", p_name);
         part_device_ptr->offset = hex2ulong(p_offset + 2);
         part_scatter[part_index] = part_device_ptr;
         part_index++;

         if (!strcasecmp(p_name, "userdata")) {
             // Skip after userdata
             break;
         }
    }

fail_out:

    *part_num_scatter = part_index;

    if(fd>=0)
        close(fd);
    if(fp)
        fclose(fp);
    unlink(scatter);

    return ret;
}

int get_partition_info(part_info_t *part_mtd[], int *part_num)
{
    int num = 0, i = 1;
    char buf[1024], p_name[PTENT_BUF_SIZE], p_size[PTENT_BUF_SIZE], p_addr[PTENT_BUF_SIZE], p_actname[PTENT_BUF_SIZE], p_id[PTENT_BUF_SIZE];
    unsigned int p_type = 0;
    part_info_t *part_mtd_ptr = NULL;
    uint64_t offset = 0, last_offset = 0, last_size = 0;
    set_phone_expdb_succeed(0);
    FILE *fp;
    part_detail_info part_info;
    if (mt_is_support_gpt()) {
        has_fat = 0;
        emmc_phone = 1;
        for(i = 1; i<= MAX_PARTITION_NUM; i++)
        {
            if(parse_partition_info_by_index(i, &part_info))  {

                if (!strcasecmp(part_info.part_name, "pgpt")) {
                    //skip PGPT && preloader
                    continue;
                }

                part_mtd_ptr = (part_info_t *)malloc(sizeof(part_info_t));

                if (part_mtd_ptr != NULL) {
                    part_mtd_ptr->offset = part_info.part_start_addr;
                    snprintf(part_mtd_ptr->name, sizeof(part_mtd_ptr->name), "%s", part_info.part_name);
                    if (!strcmp(part_info.part_name, "system")) {
                        if (part_mtd[num - 1] && !strcmp(part_mtd[num - 1]->name, "expdb")) {
                            set_phone_expdb_succeed(1);
                        }
                    }
                    part_mtd[num] = part_mtd_ptr;
                    num++;
                    offset += part_info.part_size;
                    last_offset = part_mtd_ptr->offset;
                    last_size = part_info.part_size;
                    if (num >= MAX_PARTITION_NUM) {
                        return ERROR_PARTITION_SETTING;
                    }

                    if (has_fat) {
                        //skip after FAT
                        break;
                    }
                } else {
                    return ERROR_OUT_OF_MEMORY;
                }
                if (!strcmp(part_info.part_name, "userdata")) {
                    // Skip after userdata
                    break;
                }
            }
        }

        printf("Parse Partition sucessfully\n");

        *part_num = num;

        return CHECK_OK;

    } else {
        //not gpt

        fp = fopen("/proc/dumchar_info", "r");
        if (!fp) {
            return ERROR_FILE_OPEN;
        }

        // ignore the header line
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            fclose(fp);
            return ERROR_INVALID_ARGS;
        }

        emmc_phone = 0;
        has_fat = 0;

        while (fgets(buf, sizeof(buf), fp)) {
            if (sscanf(buf, "%63s %63s %63s %d %63s %63s", p_name, p_size, p_addr, &p_type, p_actname, p_id) == 6) {
                combo_emmc = 1;
                if (!strcmp(p_name, "otp")) {
                    // OTP partition
                    continue;
                }
                if (!strcmp(p_name, "bmtpool")) {
                    // last partition
                    break;
                }
                part_mtd_ptr = (part_info_t *)malloc(sizeof(part_info_t));
                if (part_mtd_ptr != NULL) {
                    if (!strcmp(p_name, "preloader")) {
                        part_mtd_ptr->offset = hex2ulong(p_size + 2);
                    } else {
                        part_mtd_ptr->offset = offset;
                    }
                    //printf("get_partition_info : %s %lx\n", p_name, offset);
                    snprintf(part_mtd_ptr->name, sizeof(part_mtd_ptr->name), "%s", p_name);
                    if (!strcmp(p_name, "android")) {
                        if (part_mtd[num - 1] && !strcmp(part_mtd[num - 1]->name, "expdb")) {
                            set_phone_expdb_succeed(1);
                        }
                    }
                    //memcpy(part_mtd_ptr->name, p_name, strlen(p_name));
                    part_mtd[num] = part_mtd_ptr;
                    num++;

                    if (!strcmp(p_name, "preloader")) {
                        offset = 0;
                    } else {
                        offset += hex2ulong(p_size + 2);
                    }

                    if (p_type == 2) {
                        emmc_phone = 1;
                        if (strcmp(p_name, "fat") == 0) {
                            has_fat = 1;
                        }
                    }

                    if (num >= MAX_PARTITION_NUM) {
                        fclose(fp);
                        return ERROR_PARTITION_SETTING;
                    }
                } else {
                    fclose(fp);
                    return ERROR_OUT_OF_MEMORY;
                }
            } else if (sscanf(buf, "%63s %63s %63s %d %63s", p_name, p_size, p_addr, &p_type, p_actname) == 5) {
                if (!strcmp(p_name, "otp")) {
                    // OTP partition
                    continue;
                }
                if (!strcmp(p_name, "bmtpool")) {
                    // last partition
                    break;
                }
                part_mtd_ptr = (part_info_t *)malloc(sizeof(part_info_t));
                if (part_mtd_ptr != NULL) {
                    part_mtd_ptr->offset = offset;
                    //printf("get_partition_info : %s %lx\n", p_name, offset);
                    snprintf(part_mtd_ptr->name, sizeof(part_mtd_ptr->name), "%s", p_name);
                    if (!strcmp(p_name, "android")) {
                        if (part_mtd[num - 1] && !strcmp(part_mtd[num - 1]->name, "expdb")) {
                            set_phone_expdb_succeed(1);
                        }
                    }
                    //memcpy(part_mtd_ptr->name, p_name, strlen(p_name));
                    part_mtd[num] = part_mtd_ptr;
                    num++;
                    offset += hex2ulong(p_size + 2);
                    if (p_type == 2) {
                        emmc_phone = 1;
                        if (strcmp(p_name, "fat") == 0) {
                            has_fat = 1;
                        }
                    }

                    if (num >= MAX_PARTITION_NUM) {
                        fclose(fp);
                        return ERROR_PARTITION_SETTING;
                    }
                } else {
                    fclose(fp);
                    return ERROR_OUT_OF_MEMORY;
                }
            }
        }
        fclose(fp);

        *part_num = num;

        return CHECK_OK;
    } // end of not gpt
}

int get_partition_info_mlc(part_info_t *part_mtd[], int *part_num, uint64_t *total_size)
{
    int num = 0;
    char buf[1024], p_name[PTENT_BUF_SIZE], p_size[PTENT_BUF_SIZE], p_addr[PTENT_BUF_SIZE], p_actname[PTENT_BUF_SIZE], p_id[PTENT_BUF_SIZE];
    unsigned int p_type = 0;
    part_info_t *part_mtd_ptr = NULL;
    uint64_t offset = 0;
    uint64_t last_offset =  0, last_size = 0;
    set_phone_expdb_succeed(0);
    FILE *fp;

    fp = fopen("/proc/dumchar_info", "r");
    if (!fp) {
        return ERROR_FILE_OPEN;
    }

    // ignore the header line
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return ERROR_INVALID_ARGS;
    }

    emmc_phone = 0;
    has_fat = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        if (sscanf(buf, "%63s %63s %63s %d %63s %63s", p_name, p_size, p_addr, &p_type, p_actname, p_id) == 6) {
            combo_emmc = 1;
            if (!strcmp(p_name, "otp")) {
                // OTP partition
                continue;
            }
            if (!strcmp(p_name, "bmtpool")) {
                // last partition
                break;
            }
            part_mtd_ptr = (part_info_t *)malloc(sizeof(part_info_t));
            if (part_mtd_ptr != NULL) {
                if (!strcmp(p_name, "preloader")) {
                    part_mtd_ptr->offset = hex2ulong(p_size + 2);
                } else {
                    part_mtd_ptr->offset = offset;
                }
                //printf("get_partition_info : %s %lx\n", p_name, offset);
                snprintf(part_mtd_ptr->name, sizeof(part_mtd_ptr->name), "%s", p_name);
                if (!strcmp(p_name, "android")) {
                    if (part_mtd[num - 1] && !strcmp(part_mtd[num - 1]->name, "expdb")) {
                        set_phone_expdb_succeed(1);
                    }
                }
                //memcpy(part_mtd_ptr->name, p_name, strlen(p_name));
                part_mtd[num] = part_mtd_ptr;
                num++;

                if (!strcmp(p_name, "preloader")) {
                    offset = 0;
                } else {
                    offset += hex2ulong(p_size + 2);
                }

                if (p_type == 2) {
                    emmc_phone = 1;
                    if (strcmp(p_name, "fat") == 0) {
                        has_fat = 1;
                    }
                }

                if (num >= MAX_PARTITION_NUM) {
                    fclose(fp);
                    return ERROR_PARTITION_SETTING;
                }
            } else {
                fclose(fp);
                return ERROR_OUT_OF_MEMORY;
            }
        } else if (sscanf(buf, "%63s %63s %63s %d %63s", p_name, p_size, p_addr, &p_type, p_actname) == 5) {
            if (!strcmp(p_name, "otp")) {
                // OTP partition
                continue;
            }
            if (!strcmp(p_name, "bmtpool")) {
                // last partition
                break;
            }
            part_mtd_ptr = (part_info_t *)malloc(sizeof(part_info_t));
            if (part_mtd_ptr != NULL) {
                if (!strcmp(p_name, "preloader")) {
                    part_mtd_ptr->offset = hex2ulong(p_size + 2);
                } else {
                    part_mtd_ptr->offset = hex2ulong(p_addr+ 2);
                }
                //printf("get_partition_info : %s %lx\n", p_name, part_mtd_ptr->offset);
                snprintf(part_mtd_ptr->name, sizeof(part_mtd_ptr->name), "%s", p_name);
                if (!strcmp(p_name, "android")) {
                    if (part_mtd[num - 1] && !strcmp(part_mtd[num - 1]->name, "expdb")) {
                        set_phone_expdb_succeed(1);
                    }
                }
                //memcpy(part_mtd_ptr->name, p_name, strlen(p_name));
                part_mtd[num] = part_mtd_ptr;
                num++;
                if (!(last_offset == 0 && last_size == 0))  {
                    if ((part_mtd_ptr->offset - last_offset) != last_size)
                        MLC_case = 1;
                }
                last_offset = part_mtd_ptr->offset;
                last_size = hex2ulong(p_size + 2);
                // mlc size is half of partition size, different from emmc
                offset = part_mtd_ptr->offset;
                if (p_type == 2) {
                    emmc_phone = 1;
                    if (strcmp(p_name, "fat") == 0) {
                        has_fat = 1;
                    }
                }

                if (num >= MAX_PARTITION_NUM) {
                    fclose(fp);
                    return ERROR_PARTITION_SETTING;
                }
            } else {
                fclose(fp);
                return ERROR_OUT_OF_MEMORY;
            }
        }
    }
    fclose(fp);

    *total_size = (uint64_t) offset;

    *part_num = num;

    return CHECK_OK;

}

int get_partition_info_mntl(part_info_t *part_mtd[], int *part_num)
{
    int num = 0;
    char buf[1024], p_name[PTENT_BUF_SIZE], p_size[PTENT_BUF_SIZE], p_addr[PTENT_BUF_SIZE], p_actname[PTENT_BUF_SIZE], p_id[PTENT_BUF_SIZE];
    unsigned int p_type = 0;
    part_info_t *part_mtd_ptr = NULL;
    uint64_t offset = 0;
    uint64_t last_offset =  0, last_size = 0;
    set_phone_expdb_succeed(0);
    FILE *fp;

    fp = fopen("/proc/mtd", "r");
    if (!fp) {
        return ERROR_FILE_OPEN;
    }

    // ignore the header line
    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fclose(fp);
        return ERROR_INVALID_ARGS;
    }

    emmc_phone = 0;
    has_fat = 0;

    while (fgets(buf, sizeof(buf), fp)) {
        if (sscanf(buf, "%63s %63s %63s %d %63s %63s", p_name, p_size, p_addr, &p_type, p_actname, p_id) == 6) {
            combo_emmc = 1;
            if (!strcmp(p_name, "otp")) {
                // OTP partition
                continue;
            }
            if (!strcmp(p_name, "bmtpool")) {
                // last partition
                break;
            }
            part_mtd_ptr = (part_info_t *)malloc(sizeof(part_info_t));
            if (part_mtd_ptr != NULL) {
                if (!strcmp(p_name, "preloader")) {
                    part_mtd_ptr->offset = hex2ulong(p_size + 2);
                } else {
                    part_mtd_ptr->offset = offset;
                }
                //printf("get_partition_info : %s %lx\n", p_name, offset);
                snprintf(part_mtd_ptr->name, sizeof(part_mtd_ptr->name), "%s", p_name);
                if (!strcmp(p_name, "android")) {
                    if (part_mtd[num - 1] && !strcmp(part_mtd[num - 1]->name, "expdb")) {
                        set_phone_expdb_succeed(1);
                    }
                }
                //memcpy(part_mtd_ptr->name, p_name, strlen(p_name));
                part_mtd[num] = part_mtd_ptr;
                num++;

                if (!strcmp(p_name, "preloader")) {
                    offset = 0;
                } else {
                    offset += hex2ulong(p_size + 2);
                }

                if (p_type == 2) {
                    emmc_phone = 1;
                    if (strcmp(p_name, "fat") == 0) {
                        has_fat = 1;
                    }
                }

                if (num >= MAX_PARTITION_NUM) {
                    fclose(fp);
                    return ERROR_PARTITION_SETTING;
                }
            } else {
                fclose(fp);
                return ERROR_OUT_OF_MEMORY;
            }
        } else if (sscanf(buf, "%63s %63s %63s %d %63s", p_name, p_size, p_addr, &p_type, p_actname) == 5) {
            if (!strcmp(p_name, "otp")) {
                // OTP partition
                continue;
            }
            if (!strcmp(p_name, "bmtpool")) {
                // last partition
                break;
            }
            part_mtd_ptr = (part_info_t *)malloc(sizeof(part_info_t));
            if (part_mtd_ptr != NULL) {
                if (!strcmp(p_name, "preloader")) {
                    part_mtd_ptr->offset = hex2ulong(p_size + 2);
                } else {
                    part_mtd_ptr->offset = hex2ulong(p_addr+ 2);
                }
                //printf("get_partition_info : %s %lx\n", p_name, part_mtd_ptr->offset);
                snprintf(part_mtd_ptr->name, sizeof(part_mtd_ptr->name), "%s", p_name);
                if (!strcmp(p_name, "android")) {
                    if (part_mtd[num - 1] && !strcmp(part_mtd[num - 1]->name, "expdb")) {
                        set_phone_expdb_succeed(1);
                    }
                }
                //memcpy(part_mtd_ptr->name, p_name, strlen(p_name));
                part_mtd[num] = part_mtd_ptr;
                num++;
                if (!(last_offset == 0 && last_size == 0))  {
                    if ((part_mtd_ptr->offset - last_offset) != last_size)
                        MLC_case = 1;
                }
                last_offset = part_mtd_ptr->offset;
                last_size = hex2ulong(p_size + 2);
                offset = part_mtd_ptr->offset;
                if (p_type == 2) {
                    emmc_phone = 1;
                    if (strcmp(p_name, "fat") == 0) {
                        has_fat = 1;
                    }
                }

                if (num >= MAX_PARTITION_NUM) {
                    fclose(fp);
                    return ERROR_PARTITION_SETTING;
                }
            } else {
                fclose(fp);
                return ERROR_OUT_OF_MEMORY;
            }
        }
    }
    fclose(fp);

    *part_num = num;

    return CHECK_OK;

}


bool check_partition_layout(part_info_t *part_scatter[], int part_num, bool is_gpt)
{
    if (device_type == FS_TYPE_MNTL) {
            return true;
     }
    if (is_gpt) {
        int i;
        int userdata = 0;

        for (i = 0; i < part_num; i++) {
            if (!strcmp(part_scatter[i]->name, "userdata")) {
                userdata = 1;
                break;
            }
        }

        if (!userdata) {
            printf("userdata not found\n");
            return false;
        } else {
            if (strcmp(part_scatter[i - 1]->name, "cache")) {
                printf("match cache fail -> (%d) %s\n", i - 1, part_scatter[i - 1]->name);
                return false;
            }
            if (strcmp(part_scatter[i - 2]->name, "system")) {
                printf("match system fail -> (%d) %s\n", i - 2, part_scatter[i - 2]->name);
                return false;
            }

            if (has_fat) {
                for (i--; i >= 0; i--) {
                    if (!strcmp(part_scatter[i]->name, "intsd")) {
                        printf("intsd should not before userdata -> (%d)\n", i);
                        return false;
                    }
                }
            }
            return true;
        }

    } else {
        unsigned int i;
        static char nand_part_name[][16] = {  "", "USRDATA", "ANDROID" };
        static char emmc_part_name[][16] = {  "", "USRDATA", "CACHE", "ANDROID" };
        static char emmc_part_name2[][16] = {  "", "__NODL_FAT", "USRDATA", "CACHE", "ANDROID" };
        static char emmc_part_name3[][16] = {  "", "FAT", "USRDATA", "CACHE", "ANDROID" };


        for (i = 1; i < sizeof(nand_part_name) / sizeof(nand_part_name[0]); i++) {
            if (strncmp(part_scatter[part_num - i]->name, nand_part_name[i], strlen(nand_part_name[i]))) {
                break;
            }
        }

        if (i == sizeof(nand_part_name) / sizeof(nand_part_name[0])) {
            return true;
        }

        for (i = 1; i < sizeof(emmc_part_name) / sizeof(emmc_part_name[0]); i++) {
            if (strncmp(part_scatter[part_num - i]->name, emmc_part_name[i], strlen(emmc_part_name[i]))) {
                break;
            }
        }

        if (i == sizeof(emmc_part_name) / sizeof(emmc_part_name[0])) {
            return true;
        }

        for (i = 1; i < sizeof(emmc_part_name2) / sizeof(emmc_part_name2[0]); i++) {
            if (strncmp(part_scatter[part_num - i]->name, emmc_part_name2[i], strlen(emmc_part_name2[i]))) {
                break;
            }
        }

        if (i == sizeof(emmc_part_name2) / sizeof(emmc_part_name2[0])) {
            return true;
        }

        for (i = 1; i < sizeof(emmc_part_name3) / sizeof(emmc_part_name3[0]); i++) {
            if (strncmp(part_scatter[part_num - i]->name, emmc_part_name3[i], strlen(emmc_part_name3[i]))) {
                break;
            }
        }

        if (i == sizeof(emmc_part_name3) / sizeof(emmc_part_name3[0])) {
            return true;
        }

        return false;
    }
}

void set_phone_expdb_succeed(int value)
{
  phone_expdb_succeed = value;
}

int get_phone_expdb_succeed(void)
{
  return phone_expdb_succeed;
}

void set_scatter_expdb_succeed(int value)
{
  scatter_expdb_succeed = value;
}

int get_scatter_expdb_succeed(void)
{
  return scatter_expdb_succeed;
}


enum mt_device_type mt_get_phone_type(void)
{
    if (device_type == FS_TYPE_INIT) {
        mt_init_partition_type();
    }
    return device_type;
}

/* Initialize global variable device_type, is_gpt */
void mt_init_partition_type(void)
{
    /* We don't have to do it again, if they are already initialized (FS_TYPE_INIT) */
    if (device_type != FS_TYPE_INIT)
        return;

    get_device_type();

    return;
}

int set_ota_result(int result, int offset) {

    if (misc_blk_device_path == NULL) {
      printf("Error: misc_blk_device_path is NULL\n");
      return -1;
    }

    int dev = -1;
    int count;

    dev = open(misc_blk_device_path, O_WRONLY | O_SYNC);
    if (dev < 0)  {
        printf("Can't open %s\n(%s)\n", misc_blk_device_path, strerror(errno));
        return -1;
    }

    if (lseek(dev, offset, SEEK_SET) == -1) {
        printf("Failed seeking %s\n(%s)\n", misc_blk_device_path, strerror(errno));
        close(dev);
        return -1;
    }

    count = write(dev, &result, sizeof(result));
    if (count != sizeof(result)) {
        printf("Failed writing %s\n(%s)\n", misc_blk_device_path, strerror(errno));
        close(dev);
        return -1;
    }
    if (close(dev) != 0) {
        printf("Failed closing %s\n(%s)\n", misc_blk_device_path, strerror(errno));
        return -1;
    }
    sync();
    return 0;
}
