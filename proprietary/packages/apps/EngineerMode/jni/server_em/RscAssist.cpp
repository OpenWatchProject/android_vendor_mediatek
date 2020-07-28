#define LOG_TAG "EM/NATIVE/RSC"
#define MTK_LOG_ENABLE 1
#define BUFFER_SIZE 256
#define PATH_LENGTH 64

#include <cutils/log.h>
#include <stdlib.h>
#include <fs_mgr.h>

#include "RPCClient.h"
#include "RscAssist.h"
#include "sysenv_utils.h"

static const unsigned int ITEM_SIZE = 8;
static const char* itemArray[ITEM_SIZE] = {"md_rat_cs",
            "opt_md1_support",
            "opt_md3_support",
            "opt_irat_support",
            "opt_c2k_lte_mode",
            "opt_lte_support",
            "opt_ps1_rat",
            "msim_config"};
void RscAssist::switchProject(RPCClient* msgSender) {
    rsc_field_t rscData;
    fillData(msgSender, &rscData);
    int result = writePart(msgSender, &rscData);
    char response[2] = { 0 };
    response[0] = (result >= 0) ? '1' : '0';
    msgSender->PostMsg(response);
}

void RscAssist::fillData(RPCClient* msgSender, rsc_field_t* data) {

    // version
    (*data).rsc_info.version = extractInt(msgSender);

    //magic number
    (*data).rsc_info.magic_number = 0x5253434D;

    //selected project index
    (*data).rsc_info.dtbo_index = extractInt(msgSender);

    //selected project name
    memset((*data).rsc_info.full_project_name, 0, FULL_PRJ_NAME_LEN);
    extractStringToUnsighed(msgSender, (*data).rsc_info.full_project_name);
    ALOGD("full_project_name: %s", (*data).rsc_info.full_project_name);

    //selected project optr
    memset((*data).rsc_info.op_name, 0, OP_NAME_LEN);
    extractStringToUnsighed(msgSender, (*data).rsc_info.op_name);
    ALOGD("op_name: %s", (*data).rsc_info.op_name);
}

int RscAssist::extractInt(RPCClient* msgSender) {
    int type = msgSender->ReadInt();
    ALOGD("type:%d", type);
    //2.para type
    if (type != PARAM_TYPE_INT) {
        return -1;
    }
    //3.para len
    int len = msgSender->ReadInt();
    ALOGD("len:%d", len);

    //4.para value
    int value = msgSender->ReadInt();
    ALOGD("value = %d\n", value);
    return value;
}

void RscAssist::extractStringToUnsighed(RPCClient* msgSender, unsigned char* outstr) {
    int type = msgSender->ReadInt();
    ALOGD("type:%d", type);
    if (type != PARAM_TYPE_STRING) {
        return;
    }
    int len = msgSender->ReadInt();
    ALOGD("len:%d", len);
    if (len > BUFFER_SIZE - 1) {
        return;
    }
    int recv_count = 0;
    int ret = 0;
    do {
        ret = msgSender->Receive(outstr + recv_count, len - recv_count);
        if (ret < 0) {
            ALOGD("Receive fail; error:%s", strerror(errno));
            return;
        }
        recv_count += ret;
    } while (recv_count < len);
    ALOGD("recv_count %d", recv_count);
}

void RscAssist::extractString(RPCClient* msgSender, char* outstr) {
    int type = msgSender->ReadInt();
    ALOGD("type:%d", type);
    if (type != PARAM_TYPE_STRING) {
        return;
    }
    int len = msgSender->ReadInt();
    ALOGD("len:%d", len);
    if (len > BUFFER_SIZE - 1) {
        return;
    }
    int recv_count = 0;
    int ret = 0;
    do {
        ret = msgSender->Receive(outstr + recv_count, len - recv_count);
        if (ret < 0) {
            ALOGD("Receive fail; error:%s", strerror(errno));
            return;
        }
        recv_count += ret;
    } while (recv_count < len);
    ALOGD("recv_count %d", recv_count);
}

int RscAssist::writePart(RPCClient* msgSender, rsc_field_t* data) {

    char offsetStr[BUFFER_SIZE] = { 0 };
    extractString(msgSender, offsetStr);
    ALOGD("offsetStr: %s", offsetStr);
    long long offset = strtoll(offsetStr, NULL, 16);
    ALOGD("offset %lld", offset);
    char partitionPath[PATH_LENGTH] = { 0 };
    if (getPartPath(partitionPath, PATH_LENGTH) != 0) {
        ALOGE("getPartPath failed");
        return -1;
    }
    if (writeRawData(partitionPath, data, offset) != 0) {
        ALOGE("writeRawData failed");
        return -1;
    }
    clearItems();

    return 0;
}

int RscAssist::getPartPath(char* path, int size)
{
    android::fs_mgr::Fstab fstab;

    if (!ReadDefaultFstab(&fstab)) {
        ALOGE("failed to get fstab to get partition path");
        return -1;
    }
    auto rec = GetEntryForMountPoint(&fstab, "/para");
    if (rec == nullptr) {
        ALOGE("failed to get device path by mount point /para, trying /misc");
        rec = GetEntryForMountPoint(&fstab, "/misc");
        if (rec == nullptr) {
            ALOGE("failed to get device path by mount point /misc");
            return -1;
        }
    }
    snprintf(path, size, "%s", rec->blk_device.c_str());
    ALOGD("partition path = %s", path);
    return 0;
}

int RscAssist::writeRawData(char* path, rsc_field_t* data, long long offset) {
    ALOGD("writeRawData");
    int fd = open(path, O_RDWR);
    if (fd < 0) {

        ALOGE("open %s fail: %s", path, strerror(errno));
        return -1;
    }
    ALOGD("seek to %lld", offset);
    if (lseek(fd, offset, SEEK_SET) != offset) {
        ALOGE("seek to %lld fail: %s", offset, strerror(errno));
        close(fd);
        return -1;
    }
    int ret = write(fd, (char *)data, sizeof(*data));

    if (ret < 0) {
        ALOGE("write env fail: %s", strerror(errno));
        close(fd);
        return -1;
    }

    if (fsync(fd) < 0) {
        ALOGE("write env sync fail: %s\n", strerror(errno));
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

void RscAssist::clearItems() {

    for (unsigned int k = 0; k < ITEM_SIZE; k++) {
        ALOGD("name: %s\n", itemArray[k]);
        if (sysenv_set_static(itemArray[k], "") != 0) {
            ALOGE("sysenv_set_static failed");
        }
        ALOGD("after: %s\n", sysenv_get_static(itemArray[k]));
    }
}
