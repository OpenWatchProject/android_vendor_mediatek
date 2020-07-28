/*
* Copyright (C) 2011-2017 MediaTek Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*    http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <stdio.h>
#include <stdlib.h>
#include <cutils/properties.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
/*#ifndef MTK_TC1_FEATURE
#include <mtk_log.h>
#endif*/  //wait for mtkLogI ready on alps-trunk-p0.bsp.tc1

#include "libnvram.h"
#include "libnvram_log.h"
#include "hardware/ccci_intf.h"
#include "ccci_common.h"
#include "mdinit_platform.h"
#include "CFG_MD_SBP_File.h"

#define CCCCI_LOG_TAG "ccci_mdinit"
#define MD_RESET_WAIT_TIME        "vendor.md.reset.wait"
#define MD_RESET_WAIT_TIME_V2        "vendor.mediatek.debug.md.reset.wait"
#define MAX_NVRAM_RETRY_TIMES       240

void delay_to_reset_md(int version)
{
    char buf[PROPERTY_VALUE_MAX];
    int val;

    if (version == 1)
        property_get(MD_RESET_WAIT_TIME, buf, "none");
    else
        property_get(MD_RESET_WAIT_TIME_V2, buf, "none");
    if (0 != strcmp(buf, "none")) {
        val = atoi(buf);
    CCCI_LOGD("Wait modem %ds to reset md\n",val);
    if(0< val && val < 10)
        sleep(val);
    else
        CCCI_LOGD("Wait modem time invalid:%s\n", buf);
    }
}

void store_md_statue(int md_id, char *buf, int len)
{
    char name[50];
    char prop_val[PROPERTY_VALUE_MAX];

    snprintf(name, sizeof(name), "vendor.mtk.md%d.status",md_id+1); /*stdio.h*/
    CCCI_LOGD("set md status:%s=%s \n", name, buf);
    buf[len - 1] = '\0';
    snprintf(prop_val, sizeof(prop_val),"%s", buf);

/*#ifndef MTK_TC1_FEATURE
    mtkLogI(CCCI_LOG_TAG, "MD%d set status: %s=%s \n", md_id+1, name, buf);
#else*/  //wait for mtkLogI ready on alps-trunk-p0.bsp.tc1
    RLOGI("MD%d set status: %s=%s \n", md_id+1, name, buf);
//#endif

    property_set(name,prop_val);
    if (md_id == MD_SYS3) {
        snprintf(name, sizeof(name), "vendor.net.cdma.mdmstat");
        CCCI_LOGD("set md status:%s=%s \n", name, prop_val);
        property_set(name, prop_val);
    }
}

static int get_usp_sbp_setting_v2(void)
{
#define CXP_SBP "/mnt/vendor/nvdata/APCFG/APRDCL/CXP_SBP"
    int fd = 0;
    char sbp_id[16] = {0};
    int RetVal = 0;
    int sbp_code = 0;

    fd = open(CXP_SBP, O_RDONLY);
    if (fd < 0) {
        CCCI_LOGE("fail to open %s: %d", CXP_SBP, errno);
        sbp_code = -1;
    } else {
        RetVal = read(fd, sbp_id, sizeof(sbp_id)-1);
        if (RetVal <= 0) {
            CCCI_LOGE("Failed to read from %s (%d) !! errno = %d", CXP_SBP, RetVal, errno);
            sbp_code = -1;
        } else {
            CCCI_LOGD("Read %d bytes from %s,sbp_id:%s", RetVal, CXP_SBP, sbp_id);
            sbp_code = atoi(sbp_id);
        }
        close(fd);
    }

    return sbp_code;
}
/* boot_env: sbp */
static int get_usp_sbp_setting(void)
{
#define MTK_USP_SBP_NAME "persist.vendor.mtk_usp_md_sbp_code"
    char buf[PROPERTY_VALUE_MAX] ={ 0 };
    int ret;
    int sbp_code = 0;

    ret = property_get(MTK_USP_SBP_NAME, buf, NULL);
    if (ret == 0) {
        CCCI_LOGI("USP_SBP:%s not exist\n", MTK_USP_SBP_NAME);
        return -1;
    }
    sbp_code = (int)str2uint(buf);
    if (sbp_code < 0) {
        CCCI_LOGE("USP_SBP:%s=%s,usp_sbp=%d is a invalide value\n", MTK_USP_SBP_NAME, buf, sbp_code);
        sbp_code = 0;
    } else
        CCCI_LOGI("USP_SBP:%s=%s,usp_sbp=%d\n", MTK_USP_SBP_NAME, buf, sbp_code);

    return sbp_code;
}

// return: 0-fail,  others-setting value
static unsigned int get_cip_sbp_setting(int md_id)
{
#define CIP_SBP_FILE "CIP_MD_SBP"
#define CIP_MD2_SBP_FILE "CIP_MD2_SBP"
    char md_cip_sbp_file[100];
    int fd, read_len;
    struct stat sbp_stat; /* #include <sys/stat.h> */
    char cip_sbp_value[16];
    long retl = 0;
    unsigned int ret = 0;
    char *endptr = NULL;

    memset(md_cip_sbp_file, 0x0, sizeof(md_cip_sbp_file));
    if (md_id == MD_SYS1) {
        snprintf(md_cip_sbp_file, sizeof(md_cip_sbp_file), "%s%s", md_img_cip_folder, CIP_SBP_FILE);
    } else if (md_id == MD_SYS2) {
        snprintf(md_cip_sbp_file, sizeof(md_cip_sbp_file), "%s%s", md_img_cip_folder, CIP_MD2_SBP_FILE);
    } else {
        CCCI_LOGI("get_cip_sbp_setting, md_id:%d is error!\n", md_id);
        return 0;
    }

    umask(0007);
    if (stat(md_cip_sbp_file, &sbp_stat) < 0) {
        CCCI_LOGI("get_cip_sbp_setting, file %s NOT exists!\n", md_cip_sbp_file);
        return 0;
    }

    fd = open(md_cip_sbp_file, O_RDONLY);
    if(fd < 0) {
        CCCI_LOGI("get_cip_sbp_setting, open file %s Fail! err:%d\n", md_cip_sbp_file, errno);
        return 0;
    }

    memset(cip_sbp_value, 0x0, sizeof(cip_sbp_value));
    read_len = (int)read(fd, cip_sbp_value, sizeof(cip_sbp_value) - 1);
    if(read_len <= 0) {
        CCCI_LOGI("get_cip_sbp_setting, read file %s Fail! err:%d\n", md_cip_sbp_file, errno);
        close(fd);
        return 0;
    }
    close(fd);
    cip_sbp_value[read_len] = 0;

    retl = (long)str2uint(cip_sbp_value);
    if (retl > 0) {
        ret = (unsigned int)(retl & 0xFFFFFFFF);
        CCCI_LOGD("get_cip_sbp_setting, get sbp setting:0x%x\n", ret);
    } else {
        CCCI_LOGD("get_cip_sbp_setting, Error!! sbp setting is 0!\n");
    }

    return ret;
}

static int get_project_sbp_code_v1(int md_id)
{
    int sbp_code = 0;
    char tmp_buf[10] = {0};

    if (md_id == MD_SYS2) {
        // get md2 SBP code from ProjectConfig.mk from ccci_lib
        if (0 == query_prj_cfg_setting("MTK_MD2_SBP_CUSTOM_VALUE", tmp_buf, sizeof(tmp_buf)))
            sbp_code = (int)str2uint(tmp_buf);
    } else {
        // get md1 SBP code from ProjectConfig.mk from ccci_lib
        if (0 == query_prj_cfg_setting("MTK_MD_SBP_CUSTOM_VALUE", tmp_buf, sizeof(tmp_buf)))
            sbp_code = (int)str2uint(tmp_buf);
    }
    return sbp_code;
}

int get_project_sbp_code(int md_id)
{
#define MTK_OP_SBP_PRJ_CFG_NAME "ro.vendor.mtk_md_sbp_custom_value"
    char buf[PROPERTY_VALUE_MAX] ={ 0 };
    int ret;
    int sbp_code = 0;

    ret = property_get(MTK_OP_SBP_PRJ_CFG_NAME, buf, NULL);
    if (ret == 0) {
        CCCI_LOGI("PRJ_SBP_ID:%s not exist, using default value\n", MTK_OP_SBP_PRJ_CFG_NAME);
        return 0;
    }
    sbp_code = (int)str2uint(buf);
    if (sbp_code < 0) {
        CCCI_LOGE("PRJ_SBP_ID value abnormal, switch to 0(from:%s\n", buf);
        sbp_code = 0;
    } else
        CCCI_LOGI("PRJ_SBP_ID value: %d for md%d\n", sbp_code, md_id + 1);

    return sbp_code;
}

static int get_nvram_ef_data(int fid, int recsize, void* pdata)
{
    F_ID nvram_ef_fid = {0,0,0};
    int rec_size = 0;
    int rec_num = 0;
    bool isread = false;
    int ret = 0;

    if (pdata == NULL) {
        CCCI_LOGE("get_nvram_ef_data: pdata=NULL, fid:%d, recsize:%d\n", fid, recsize);
        return -1;
    }

    nvram_ef_fid = NVM_GetFileDesc(fid, &rec_size, &rec_num, isread);
    if (nvram_ef_fid.iFileDesc < 0) {
        CCCI_LOGE("get_nvram_ef_data: Fail to get nvram file descriptor!! fid:%d, errno:0x%x\n", fid, errno);
        return -1;
    }

    if (rec_size != read(nvram_ef_fid.iFileDesc, pdata, rec_size)) {
        CCCI_LOGE("get_nvram_ef_data: Fail to read nvram file!! fid:%d, errno:0x%x\n", fid, errno);
        return -1;
    }

    if (!NVM_CloseFileDesc(nvram_ef_fid)) {
        CCCI_LOGE("get_nvram_ef_data: Fail to close nvram file!! fid:%d, errno:0x%x\n", fid, errno);
    }

    return ret;
}

static int store_nvram_ef_data(int fid, int recsize, void* pdata)
{
    F_ID nvram_ef_fid = {0,0,0};
    int rec_size = 0;
    int rec_num = 0;
    bool isread = false;
    void *old_buf = NULL;
    unsigned int *pold_data = NULL;
    int ret = 0;

    CCCI_LOGD("store_nvram_ef_data fid:%d, size:%d, pdata:0x%x\n",
        fid, recsize, ((pdata == NULL) ? 0 : ((unsigned int) pdata)));

    if (!pdata)
        return -1;

    // back up the old data
    old_buf = (void *)malloc(recsize);
    if (!old_buf) {
        CCCI_LOGE("store_nvram_ef_data fid: allocate data memory err:0x%x\n", errno);
    } else {
        ret = get_nvram_ef_data(fid, recsize, old_buf);
        pold_data = (unsigned int *)old_buf;
    }

    if (pold_data != NULL)
        CCCI_LOGD("store_nvram_ef_data ret:%d, data1:%d, data2:0x%x\n",
                fid, (!ret) ? (*pold_data) : 0, (!ret) ? (*(pold_data + 1)) : 0);
    else
        CCCI_LOGE("pold_data is NULL.\n");

    nvram_ef_fid = NVM_GetFileDesc(fid, &rec_size, &rec_num, isread);
    if (nvram_ef_fid.iFileDesc < 0) {
        CCCI_LOGE("store_nvram_ef_data: Fail to get nvram file descriptor!! fid:%d, errno:0x%x\n", fid, errno);
        if (old_buf)
            free(old_buf);
        return -1;
    }

    if (rec_size != write(nvram_ef_fid.iFileDesc, pdata, rec_size)) {
        CCCI_LOGE("store_nvram_ef_data: Fail to write nvram file!! fid:%d, errno:0x%x, rec_size:%d, recsize:%d\n"
            , fid, errno, rec_size, recsize);
        ret = -1;
        // Try to recovery old data
        if (pold_data) {
            rec_size = write(nvram_ef_fid.iFileDesc, pold_data, recsize);
            CCCI_LOGE("store_nvram_ef_data: recovery data!! fid:%d, errno:0x%x, rec_size:%d, recsize:%d\n"
                , fid, errno, rec_size, recsize);
        }
    }

    if (!NVM_CloseFileDesc(nvram_ef_fid)) {
        CCCI_LOGE("store_nvram_ef_data: Fail to close nvram file!! fid:%d, errno:0x%x\n", fid, errno);
    }

    if (old_buf)
        free(old_buf);
    return ret;
}

static bool check_nvram_ready(void)
{
    int read_nvram_ready_retry = 0;
    int ret = 0;
    char nvram_init_val[PROPERTY_VALUE_MAX];
    const int MAX_RETRY_COUNT = 30;
    while (true)
    {
        read_nvram_ready_retry++;
        property_get("vendor.service.nvram_init", nvram_init_val, NULL);
        if (strcmp(nvram_init_val, "Ready") == 0 ||
            strcmp(nvram_init_val, "Pre_Ready") == 0)
        {
            ret = true;
            break;
        }
        else
        {
            CCCI_LOGI("%s(), property_get(\"vendor.service.nvram_init\") = %s, read_nvram_ready_retry = %d",
                  __FUNCTION__, nvram_init_val, read_nvram_ready_retry);
            usleep(500 * 1000);
        }
    }
    if (read_nvram_ready_retry >= MAX_RETRY_COUNT)
    {
        CCCI_LOGE("Get nvram restore ready faild !!!\n");
        ret = false;
    }
    return ret;
}

int get_nvram_sbp_code(int md_id)
{
#define MD_SBP_PATH_FILE "/vendor/nvdata/APCFG/APRDCL/MD_SBP"
    int getsbpcode = 0;
    int store_sbp_code = 0;
    MD_SBP_Struct *nvram_sbp_info = NULL;
    int md_sbp_lid = -1;

    check_nvram_ready();
    md_sbp_lid = NVM_GetLIDByName(MD_SBP_PATH_FILE);
    if (md_sbp_lid < 0) {
        CCCI_LOGE("Error!! get sbp lid fail!!!%d\n", md_sbp_lid);
        goto EXIT_FUN;
    }
    nvram_sbp_info = (MD_SBP_Struct *)malloc(sizeof(MD_SBP_Struct));
    if (nvram_sbp_info == NULL) {
        CCCI_LOGD("Error!! malloc md sbp code fail! errno:%d\n", errno);
        goto EXIT_FUN;
    }
    memset((void *)nvram_sbp_info, 0, sizeof(MD_SBP_Struct));
    getsbpcode = get_nvram_ef_data(md_sbp_lid,    sizeof(MD_SBP_Struct), nvram_sbp_info);
    if (getsbpcode != 0) {
        CCCI_LOGD("Error!! get_nvram_ef_data fail lid=%d,ret:%d\n", md_sbp_lid, getsbpcode);
        goto EXIT_FUN;

    }
    if (md_id == MD_SYS2)
        store_sbp_code = nvram_sbp_info->md2_sbp_code;
    else
        store_sbp_code = nvram_sbp_info->md_sbp_code;
EXIT_FUN:
    if (nvram_sbp_info) {
        free(nvram_sbp_info);
        nvram_sbp_info = NULL;
    }

    return store_sbp_code;
}

/* boot env */

/*
 *  for MD SBP feature, diferent operators use the same md image
 *  MTK_MD_SBP_CUSTOM_VALUE_ must be definied on ProjectConfig.mk
 *    0: INVALID value, the project need SBP feature, but value 0 needn't transform to modem
 *  related files:
 *    MD_SBP: under /vendor/nvdata/APCFG/APRDCL/, see about CFG_MD_SBP_File.h
 *      the md_sbp_value of MD_SBP could be assigned by MTK_MD_SBP_CUSTOM_VALUE_ in ProjectConfig.mk
 *      if MTK_MD_SBP_CUSTOM_VALUE_=0, it means need SBP process flow, but not care ProjectConfig value.
 *    CIP_MD_SBP: under /custom/etc/firmware/, It is Hexadecimai number string, ex: 0x3
 *  Rules:
 *    wwop project use CIP_MD_SBP file, the number transform from CIP_MD_SBP MUST NOT be 0
 *        MTK_MD_SBP_CUSTOM_VALUE_ SHOULD be defined as 0x0(or 0) in ProjectConfig.mk of wwop project
 *    in non-wwop project:
 *      MTK_MD_SBP_CUSTOM_VALUE_ Defined: the MTK_MD_SBP_CUSTOM_VALUE_ should be transfer to md
 *      MTK_MD_SBP_CUSTOM_VALUE_ Undefined: the value in MD_SBP should be transfer to md
 *    The sb_code needn't transfer to md from the second boot up time, if the sb_code is not changed from the first boot.
 */
int get_md_sbp_code(int md_id, int version)
{
    int sbp_default = 0;
    int cip_sbp_value = 0;
    int stored_sbp_code = 0;
    static int sbpc = 0;
    int usp_sbp_value;

    /* NOTES:
    * priority: USP > CIP > ProjectConfig > meta tool
    * Assume:
    *    0. uniservice pack property for global device
    *    1. wwop(CIP) project could not be modified by meta tool
    *    2. ProjectConfiged project could not be modified by meta tool
    *    3. meta tool could modified project MUST NOT define MTK_MD_SBP_CUSTOM
    */
    usp_sbp_value = get_usp_sbp_setting_v2();
    if (usp_sbp_value >= 0) { /* Carrier 2.0 */
        sbpc = usp_sbp_value;
        CCCI_LOGI("Get: usp_sbp=%d\n", usp_sbp_value);
        return sbpc;
    }

    cip_sbp_value = get_cip_sbp_setting(md_id);
    if (version == 1)
        sbp_default = get_project_sbp_code_v1(md_id);
    else
        sbp_default = get_project_sbp_code(md_id);
    stored_sbp_code = get_nvram_sbp_code(md_id);

    if (cip_sbp_value > 0)
        sbpc = cip_sbp_value;
    else if (sbp_default > 0)
        sbpc = sbp_default;
    else if (stored_sbp_code > 0)
        sbpc = stored_sbp_code;

    CCCI_LOGD("Get: usp_sbp=%d, cip_sbp=%d, project_sbp=%d, nvram_sbp=%d, set sbp=%d\n",
        usp_sbp_value, cip_sbp_value, sbp_default, stored_sbp_code, sbpc);

    return sbpc;
}

typedef enum {
    MODE_UNKNOWN = -1,      /* -1 */
    MODE_IDLE,              /* 0 */
    MODE_USB,               /* 1 */
    MODE_SD,                /* 2 */
    MODE_POLLING,           /* 3 */
    MODE_WAITSD,            /* 4 */
} LOGGING_MODE;
/* MD logger configure file */
#define MD1_LOGGER_FILE_PATH "/data/mdlog/mdlog1_config"
#define MD2_LOGGER_FILE_PATH "/data/mdlog/mdlog2_config"
#define MD3_LOGGER_FILE_PATH "/data/mdlog/mdlog3_config"
#define MDLOGGER_FILE_PATH   "/data/mdl/mdl_config"
#define MD_LOG_MEMDUMP_WAIT  "ro.vendor.md_log_memdump_wait"

int get_md_wait_time(void)
{
    int ret = 0;
    char buf[PROPERTY_VALUE_MAX] = {0};
    int md_wait_time = 0;

    ret = property_get(MD_LOG_MEMDUMP_WAIT, buf, NULL);
    if (ret == 0) {
      CCCI_LOGI("MD_LOG_MEMDUMP_WAIT:%s not exist, using default value\n", MD_LOG_MEMDUMP_WAIT);
      md_wait_time = 0;
    }
    md_wait_time = (int)str2uint(buf);
    if (md_wait_time < 0) {
      CCCI_LOGE("MD_LOG_MEMDUMP_WAIT value abnormal, switch to 0 from:%s\n", buf);
      md_wait_time = 0;
    } else
      CCCI_LOGI("MD_LOG_MEMDUMP_WAIT value: %d\n", md_wait_time);

    return md_wait_time;
}

int get_mdlog_boot_mode(int md_id)
{
    int fd, ret;
    unsigned int mdl_mode = 0;
    char pBuildType[PROPERTY_VALUE_MAX] = {0};

    switch(md_id) {
    case 0:
        fd = open(MD1_LOGGER_FILE_PATH, O_RDONLY);
        if(fd < 0)
            fd = open(MDLOGGER_FILE_PATH, O_RDONLY);
        break;
    case 1:
        fd = open(MD2_LOGGER_FILE_PATH, O_RDONLY);
        break;
    case 3:
        fd = open(MD3_LOGGER_FILE_PATH, O_RDONLY);
        break;
    default:
        CCCI_LOGE("Open md_id=%d error!\n", md_id);
        fd = -1;
        break;
    }
    if (fd < 0) {
        CCCI_LOGE("Open md_log_config file failed, errno=%d!\n", errno);
        property_get("ro.build.type", pBuildType, "eng");
        if (0 == strcmp(pBuildType, "eng"))
                mdl_mode = MODE_SD;
        else
                mdl_mode = MODE_IDLE;
        mdl_mode |= get_md_wait_time() << 16;
        return mdl_mode;
    }
    ret = read(fd, &mdl_mode, sizeof(unsigned int));
    if (ret != sizeof(unsigned int)) {
        CCCI_LOGE("read failed ret=%d, errno=%d!\n", ret, errno);
        property_get("ro.build.type", pBuildType, "eng");
        if (0 == strcmp(pBuildType, "eng"))
                mdl_mode = MODE_SD;
        else
                mdl_mode = MODE_IDLE;
        mdl_mode |= get_md_wait_time() << 16;
    }
    close(fd);
    return mdl_mode;
}

int get_md_dbg_dump_flag(int md_id)
{
    char buf[PROPERTY_VALUE_MAX] = { '\0' };
    int ret = -1; /* equal to 0xFFFFFFFF as Uint32 in kernel */

    if (md_id == MD_SYS1)
        property_get("persist.vendor.ccci.md1.dbg_dump", buf, "none");
    else if (md_id == MD_SYS3)
        property_get("persist.vendor.ccci.md3.dbg_dump", buf, "none");

    if (0 != strcmp(buf, "none"))
        ret = strtoul(buf, NULL, 16);
    return ret;
}

int get_sbp_subid_setting(void)
{
#define MTK_OP_SBP_SUB_ID_NAME "persist.vendor.operator.subid"
    char buf[PROPERTY_VALUE_MAX] ={ 0 };
    int ret;
    int sub_id_code = 0;

    ret = property_get(MTK_OP_SBP_SUB_ID_NAME, buf, NULL);
    if (ret == 0) {
        CCCI_LOGI("SBP_SUB_ID:%s not exist\n", MTK_OP_SBP_SUB_ID_NAME);
        return 0;
    }
    sub_id_code = (int)str2uint(buf);
    if (sub_id_code < 0) {
        CCCI_LOGE("SBP_SUB_ID:%s=%s,sub_id=%d is a invalide value\n", MTK_OP_SBP_SUB_ID_NAME, buf, sub_id_code);
        sub_id_code = 0;
    } else
        CCCI_LOGI("SBP_SUB_ID:%s=%s,usp_sbp=%d\n", MTK_OP_SBP_SUB_ID_NAME, buf, sub_id_code);

    return sub_id_code;
}


