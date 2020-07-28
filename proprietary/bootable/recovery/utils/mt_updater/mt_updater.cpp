/*
* Copyright (C) 2014 MediaTek Inc.
* Modification based on code covered by the mentioned copyright
* and/or permission notice(s).
*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <fcntl.h>
#if !defined(ARCH_X86)
#include <linux/scsi/ufs/ufs-mtk-ioctl.h>
#include <linux/mmc/mmc-mtk-ioctl.h>
#include <sys/ioctl.h>
#endif
#include <edify/expr.h>
#include <bootloader.h>
#include <mt_updater.h>
#include <otautil/error_code.h>
#include <bootloader_message/bootloader_message.h>
#include <android-base/logging.h>
#include "../include/mt_gpt.h"
#include "../include/mt_pmt.h"
#include "../include/mt_partition.h"

/* external functions defined in install.c */
extern void uiPrintf(State* state, const char* format, ...);

// Static Function

static int mt_update_erase_part(__unused State* state, const char *partition_name) {
    char* dev_name = get_partition_path(partition_name);

    int fd = open(dev_name, O_WRONLY | O_SYNC);
    if (fd != -1) {
        char *buf = (char *)malloc(1024);
        memset(buf, 0, 1024);
        if (write(fd, buf, 1024) == -1) {
            fprintf(stderr, "write to %s fail\n", dev_name);
            close(fd);
            free(dev_name);
            free(buf);
            return 1;
        }
        printf("write done\n");
        close(fd);
        free(buf);
    } else {
        fprintf(stderr, "open %s fail\n", dev_name);
        free(dev_name);
        return 1;
    }
    free(dev_name);
    return 0;
}

static int ufs_set_active_boot_part(unsigned int boot)
{
    struct ufs_ioctl_query_data idata;
    unsigned char buf[1];
    int fd, ret = 0;

    fd = open("/dev/block/sdc", O_RDWR);

    if (fd < 0) {
        printf("%s: open device failed, err: %d\n", __func__, fd);
        ret = -1;
        goto out;
    }

    buf[0] = boot;           /* 1: BootLU A, 2: BootLU B */

    idata.opcode = UPIU_QUERY_OPCODE_WRITE_ATTR;
    idata.idn = QUERY_ATTR_IDN_BOOT_LUN_EN;
    idata.idx = 0;
    idata.buf_ptr = &buf[0];
    idata.buf_byte = 1;

    ret = ioctl(fd, UFS_IOCTL_QUERY, &idata);
    if(ret < 0)
        printf("ufs_set boot_part fail: %s\n", strerror(errno));
out:
    if(fd >= 0)
        close(fd);
    return ret;
}

static int mmc_read_extcsd(int fd, __u8 *ext_csd)
{
    int ret = 0;
    struct mmc_ioc_cmd mmc_ioctl_cmd;

    memset(ext_csd, 0, sizeof(__u8) * 512);
    memset(&mmc_ioctl_cmd, 0, sizeof(mmc_ioctl_cmd));
    mmc_ioctl_cmd.blocks = 1;
    mmc_ioctl_cmd.blksz = 512;
    mmc_ioctl_cmd.opcode = MMC_SEND_EXT_CSD;
    mmc_ioctl_cmd.flags = MMC_CMD_ADTC | MMC_RSP_R1;
    mmc_ioc_cmd_set_data(mmc_ioctl_cmd, ext_csd);

    ret = ioctl(fd, MMC_IOC_CMD, &mmc_ioctl_cmd);

    if (ret)
        fprintf(stderr, "ioctl error, mmc_read_extcsd fail, ret = %d\n", ret);
    return ret;
}

static int mmc_switch_bootpart(int fd, __u8 *ext_csd, __u8 bootpart)
{
    int ret = 0;
    struct mmc_ioc_cmd mmc_ioctl_cmd;
    __u8 val;

    val = (ext_csd[EXT_CSD_PART_CONFIG] & ~(0x38)) | (bootpart << 3);
    memset(&mmc_ioctl_cmd, 0, sizeof(mmc_ioctl_cmd));
    mmc_ioctl_cmd.opcode = MMC_SWITCH;

    mmc_ioctl_cmd.arg = (MMC_SWITCH_MODE_WRITE_BYTE << 24)
        | (EXT_CSD_PART_CONFIG << 16)
        | val << 8
        | EXT_CSD_CMD_SET_NORMAL;
    mmc_ioctl_cmd.flags = MMC_CMD_AC | MMC_RSP_R1B;
    mmc_ioc_cmd_set_data(mmc_ioctl_cmd, ext_csd);

    ret = ioctl(fd, MMC_IOC_CMD, &mmc_ioctl_cmd);
    if (ret)
        fprintf(stderr, "ioctl error, mmc_switch_bootpart fail ret = %d\n", ret);
    return ret;
}

static int emmc_set_active_boot_part(unsigned int bootpart)
{
    __u8 ext_csd[512];
    __u8 cur_bootpart;
    int ret;
    int fd= open("/dev/block/mmcblk0", O_RDWR);
    if (fd >= 0) {
        ret = mmc_read_extcsd(fd, ext_csd);
        if (ret) {
            fprintf(stderr, "Could not read EXT_CSD, error=%d\n", ret);
            close(fd);
            return 1;
        }

        /* check current boot part */
        cur_bootpart = (ext_csd[EXT_CSD_PART_CONFIG] >> 3) & 0x7;
        if (cur_bootpart == bootpart)
        {
            fprintf(stderr, "Current boot part is boot%d, no neeed switch\n", cur_bootpart);
        } else {
            fprintf(stderr, "Current boot part is boot%d, need switch to %d\n",cur_bootpart, bootpart);
            ret = mmc_switch_bootpart(fd, ext_csd, bootpart);
                if(ret) {
                    fprintf(stderr, "Could not switch boot part, error=%d\n", ret);
                    close(fd);
                    return 1;
                }
        }
        close(fd);
        return 0;
        } else {
            fprintf(stderr, "open /dev/block/mmcblk0 fail\n");
            return 1;
        }
}

static int mt_update_active_part(State* state, const char* from_partition, const char *to_partition)
{
    if (((mt_get_phone_type() == FS_TYPE_MTD) && (!strncasecmp(from_partition, "preloader", strlen("preloader"))))  // preloader on NAND not use active bit
        ) {
        if (!strcasecmp(from_partition, "preloader")) // only do erase when main partition to alt partition
            return mt_update_erase_part(state, from_partition);
    } else if((mt_get_phone_type() == FS_TYPE_UFS || mt_get_phone_type() == FS_TYPE_EMMC)
              && !strncasecmp(from_partition, "preloader", strlen("preloader")))  {  // preloader on UFS or EMMC
        unsigned int bootpart = 0;
        int ret = -1;

        if (!strcmp(to_partition, "preloader"))
            bootpart = 1;
        else
            bootpart = 2;

        if (mt_get_phone_type() == FS_TYPE_UFS)
            ret = ufs_set_active_boot_part(bootpart);
        else
            ret = emmc_set_active_boot_part(bootpart);
        return ret;
    } else if ((mt_get_phone_type() == FS_TYPE_MNTL) || (mt_get_phone_type() == FS_TYPE_MTD)) {
        // need to set to_partition active bit to 1 and then set from_partition active bit to 0
        int ret = mt_pmt_update_active_part(to_partition, 1) | mt_pmt_update_active_part(from_partition, 0);
        return ret;
    } else if (mt_is_support_gpt()) {
        // need to set to_partition active bit to 1 and then set from_partition active bit to 0
        int ret = mt_gpt_update_active_part(to_partition, 1) | mt_gpt_update_active_part(from_partition, 0);
        return ret;
    } else {
        // TODO: pmt type active bit switch
    }
    return 1;
}

static void mtSetOTAResultForDMVerity(void)
{
    int ota_result_offset;

    ota_result_offset = sizeof(bootloader_message_ab);
    set_ota_result(1, ota_result_offset);
}

static void write_result_file(void)
{
    printf("INSTALL_SUCCESS set_ota_result\n");
    set_ota_result(1, OTA_RESULT_OFFSET);
}

// Callback function for updater

Value* mtDeleteFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    std::string path;
    if(!Evaluate(state, argv[0], &path)) {
       fprintf(stderr, "mtDeleteFn fail name is %s: ,path is %s\n", name, path.c_str());
       return NULL;
    }

    int success = 0;
    if (unlink(path.c_str()) == 0)
        ++success;

    char buffer[10];
    snprintf(buffer, sizeof(buffer), "%d", success);
    return StringValue(strdup(buffer));
}

Value* mtGetUpdateStageFn(__unused const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    char buf[64];
    std::vector<std::string> args;
    if (!ReadArgs(state, argv, &args))
        return NULL;
    const std::string& filename = args[0];
    FILE *fp = fopen(filename.c_str(), "r");
    strncpy(buf, "0", sizeof(buf)-1);
    buf[sizeof(buf)-1] = '\0';
    if (fp) {
        fgets(buf, sizeof(buf), fp);
        fclose(fp);
    }
    return StringValue(strdup(buf));
}

Value* mtSetUpdateStageFn(__unused const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    std::vector<std::string> args;
    if (!ReadArgs(state, argv, &args))
        return NULL;
    const std::string& filename = args[0];
    const std::string& buf = args[1];
    FILE *fp = fopen(filename.c_str(), "w+");
    if (fp) {
        fputs(buf.c_str(), fp);
        fclose(fp);
    }
    return StringValue(filename);
}

Value* mtShowUpdateStageFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    Value* retval = mtGetUpdateStageFn(name, state, argv);
    printf("Current Stage is %s\n", retval->data.c_str());
    return retval;
}

Value* mtSwitchActiveFn(const char* name, State* state, const std::vector<std::unique_ptr<Expr>>& argv) {
    if (argv.size() != 2)
        return ErrorAbort(state, kArgsParsingFailure,"%s() expects 2 arg, got %zu", name, argv.size());

    std::vector<std::string> args;
    if (!ReadArgs(state, argv, &args))
         return NULL;
    const std::string& from_partition = args[0];
    const std::string& to_partition = args[1];

    mt_update_active_part(state, from_partition.c_str(), to_partition.c_str());
    printf("Switch %s active to %s\n", from_partition.c_str(), to_partition.c_str());
    return StringValue(from_partition);
}

Value* mtSetEmmcWR(const char * name, State* state, const std::vector<std::unique_ptr<Expr>>& argv)
{
    if(mt_get_phone_type() == FS_TYPE_UFS)
        return StringValue(strdup("t"));

    std::string path;
    if(!Evaluate(state, argv[0], &path)) {
       fprintf(stderr, "mtSetEmmcWR fail name is %s: ,path is %s\n", name, path.c_str());
       return NULL;
    }

    char const *buf = "0\n";
    int ret = 0;
    int fd = open(path.c_str(), O_WRONLY | O_TRUNC);
    if (fd >= 0) {
        write(fd, buf, strlen(buf));
        close(fd);
        ret = 1;
    } else {
        printf("can not open %s to disable force_ro\n", path.c_str());
    }
    return StringValue(ret == 1 ? strdup("t") : strdup(""));
}

Value* mtGetStorageType(__unused const char* name, __unused State* state, __unused const std::vector<std::unique_ptr<Expr>>& argv)
{
    if (mt_get_phone_type() == FS_TYPE_UFS) {
        return StringValue(strdup("t"));
    } else if (mt_get_phone_type() == FS_TYPE_EMMC) {
        return StringValue(strdup(""));
    } else {
        printf("Storage is not EMMC or UFS\n");
    }
    return StringValue(strdup(""));
}

Value* mt_SetStageFn(const char* name, State* state, char** filename, char** stagestr)
{
    char *dev_path = get_partition_path(*filename);
    free(*filename);
    *filename = dev_path;

    //misc write needs aligment
    int fd = open(*filename, O_RDWR | O_SYNC);
    struct bootloader_message bm;
    if (fd < 0)  {
        return ErrorAbort(state, kFileOpenFailure,"%s() open %s fail", name, *filename);
    }
    int count = read(fd, &bm, sizeof(bm));
    if (count != sizeof(bm)) {
        close(fd);
        return ErrorAbort(state, kFreadFailure, "%s() read %s fail, count=%d %s", name, *filename, count, strerror(errno));
    }
    memset(bm.stage, 0, sizeof(bm.stage));
    snprintf(bm.stage, sizeof(bm.stage) - 1, "%s", *stagestr);

    lseek(fd, 0, SEEK_SET);
    count = write(fd, &bm, sizeof(bm));
    if (count != sizeof(bm)) {
        close(fd);
        return ErrorAbort(state, kFwriteFailure, "%s() write %s fail, count=%d %s", name, *filename, count, strerror(errno));
    }
    if (close(fd) != 0) {
        return ErrorAbort(state, kVendorFailure, "%s() close %s fail", name, *filename);
    }
    sync();

    return NULL;
}

Value* mt_GetStageFn(const char* name, State* state, char** filename, char *buffer)
{
    char *dev_path = get_partition_path(*filename);
    free(*filename);
    *filename = dev_path;

    int fd = open(*filename, O_RDONLY);
    struct bootloader_message bm;

    if (fd < 0)
        return ErrorAbort(state, kFileOpenFailure,"%s() open %s fail", name, *filename);

    int count = read(fd, &bm, sizeof(bm));
    if (count != sizeof(bm)) {
        close(fd);
        return ErrorAbort(state, kFreadFailure,"%s() read fail, count=%d", name, count);
    }
    if (close(fd) != 0) {
        return ErrorAbort(state, kVendorFailure,"%s() close %s fail", name, *filename);
    }

    memcpy(buffer, bm.stage, sizeof(bm.stage));

    return NULL;
}

Value* mtPostOTAAction(__unused const char* name, __unused State* state, __unused const std::vector<std::unique_ptr<Expr>>& argv)
{
    int ret = 1;

    // Add Post OTA Action
    mtSetOTAResultForDMVerity();
    write_result_file();

    return StringValue(ret == 1 ? strdup("t") : strdup(""));
}

// Map callback function to edify function name

void Register_librecovery_updater_mtk(void)
{
    mt_init_partition_type();

    RegisterFunction("get_mtupdate_stage", mtGetUpdateStageFn);
    RegisterFunction("set_mtupdate_stage", mtSetUpdateStageFn);
    RegisterFunction("show_mtupdate_stage", mtShowUpdateStageFn);
    RegisterFunction("switch_active", mtSwitchActiveFn);
    RegisterFunction("delete", mtDeleteFn);
    RegisterFunction("set_emmc_writable", mtSetEmmcWR);
    RegisterFunction("get_storage_type", mtGetStorageType);
    RegisterFunction("post_ota_action", mtPostOTAAction);
}
