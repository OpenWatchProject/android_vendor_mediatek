/*
 * Copyright (c) 2018 MediaTek Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdio.h>

#include <ion.h>
#include <ion/ion.h>
#include <linux/ion.h>
#include <linux/ion_drv.h>
#ifdef ION_NUM_HEAP_IDS
#undef ION_NUM_HEAP_IDS
#endif
#include <linux/mtk_ion.h>

#include <unistd.h>

#include <tz_cross/trustzone.h>
#include <uree/system.h>
#include <uree/mem.h>
//#include "vpu_drv.h"
#include <cutils/properties.h>

#define LOG_TAG "FOD_CA"
/* LOG_TAG need to be defined before include log.h */
#include <android/log.h>

#ifdef TRUSTONIC_TEE_ENABLED
#include <MobiCoreDriverApi.h>
#endif
#ifdef GP_TEE_ENABLED
#include <tee_client_api.h>
#endif

/*---------------------------------------------------------------------------*/
/*  IOCTL Command                                                            */
/*---------------------------------------------------------------------------*/
#define VPU_MAGICNO                 'v'

#define VPU_IOCTL_SDSP_POWER_ON     _IOW(VPU_MAGICNO,   60, int)
#define VPU_IOCTL_SDSP_POWER_OFF    _IOW(VPU_MAGICNO,   61, int)


#define DBG_LOG(format, args...) printf("[DBG:%05d] "format, __LINE__, ##args)
#define ERR_LOG(format, args...) printf("[ERR:%05d] "format, __LINE__, ##args)

static uint32_t buf_num[4];
static uint32_t prop_size[4];
uint64_t MdShmForMteeTeePA, MdShmForMteeTeeSize;
UREE_SHAREDMEM_HANDLE shm_handle;
int file_len;
int CA_ion_fd;
struct timeval tv1;
struct timeval tv2;
int time_cost;
int my_rand1;
int my_rand2;
bool exit_test = false;
bool use_ion = false;
bool need_reboot = false;

#define DEFAULT 100
#define CMDS_SDSP 116
#define GOODIX_UT_01 166
#define GOODIX_UT_02 167
#define MTEE_SDSP_UT_01 168
#define MTEE_SDSP_UT_02 169
#define MTEE_SDSP_UT_03 170
#define FAKE_SDSP 198
#define MULTICORE 200
#define JUST_LOAD_TEST 300
int run_case=DEFAULT;

static void Lazy(char *server_name, uint32_t lazy_cmd);
static void AcqureVPU(void);
static void ReleaseVPU(void);

#ifdef TRUSTONIC_TEE_ENABLED
// clang-format off
#define FOD_DRIVER_UUID \
    {{ 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
       0x00, 0x00, 0x00, 0x00, 0x19, 0x79, 0x04, 0x14 }}
// clang-format on

static const uint32_t g_device_id = MC_DEVICE_ID_DEFAULT;
static const mcUuid_t g_drv_uuid = FOD_DRIVER_UUID;

struct fod_tci_params {
    int command;
    int ret;
    int params[8];
};

static mcSessionHandle_t fod_drv_session;
static struct fod_tci_params* fod_drv_params = NULL;

static struct fod_tci_params* TEE_Open(mcSessionHandle_t* pSessionHandle) {
    struct fod_tci_params* pTci = NULL;
    mcResult_t mcRet;

    do {
        if (pSessionHandle == NULL) {
            ERR_LOG("TEE_Open(): Invalid session handle!\n");
            return NULL;
        }

        memset(pSessionHandle, 0, sizeof(mcSessionHandle_t));

        mcRet = mcOpenDevice(g_device_id);
        if ((MC_DRV_OK != mcRet) && (MC_DRV_ERR_DEVICE_ALREADY_OPEN != mcRet)) {
            ERR_LOG("TEE_Open(): mcOpenDevice returned(%d)\n", mcRet);
            return NULL;
        }

        mcRet = mcMallocWsm(g_device_id, 0, sizeof(struct fod_tci_params), (uint8_t**)&pTci, 0);
        if (MC_DRV_OK != mcRet) {
            mcCloseDevice(g_device_id);
            ERR_LOG("TEE_Open(): mcMallocWsm returned(%d)\n", mcRet);
            return NULL;
        }

        pSessionHandle->deviceId = g_device_id;
        mcRet = mcOpenSession(pSessionHandle, &g_drv_uuid, (uint8_t*)pTci,
                              (uint32_t)sizeof(struct fod_tci_params));
        if (MC_DRV_OK != mcRet) {
            mcFreeWsm(g_device_id, (uint8_t*)fod_drv_params);
            mcCloseDevice(g_device_id);
            ERR_LOG("TEE_Open(): mcOpenSession returned(%d)\n", mcRet);
            return NULL;
        }
    } while (false);

    return pTci;
}

static void TEE_Close(mcSessionHandle_t* pSessionHandle) {
    mcResult_t mcRet;

    do {
        if (pSessionHandle == NULL) {
            ERR_LOG("TEE_Close(): Invalid session handle!");
            break;
        }

        mcRet = mcCloseSession(pSessionHandle);
        if (MC_DRV_OK != mcRet) {
            ERR_LOG("TEE_Close(): mcCloseSession returned(%d)", mcRet);
            break;
        }

        mcRet = mcFreeWsm(g_device_id, (uint8_t*)fod_drv_params);
        if (MC_DRV_OK != mcRet) {
            ERR_LOG("TEE_Close(): mcFreeWsm returned(%d)", mcRet);
            break;
        }

        mcRet = mcCloseDevice(g_device_id);
        if (MC_DRV_OK != mcRet) {
            ERR_LOG("TEE_Close(): mcCloseDevice returned(%d)", mcRet);
            break;
        }
    } while (false);
}
#endif	//end #ifdef TRUSTONIC_TEE_ENABLED

#ifdef TRUSTONIC_TEE_ENABLED
static void tee_open_fod_tdriver() {
    fod_drv_params = TEE_Open(&fod_drv_session);
    if (fod_drv_params == NULL) {
        ERR_LOG("open pmem t-driver failed!\n");
        exit(0);
    }
}

static void tee_close_fod_tdriver() {
    if (fod_drv_params != NULL) {
        TEE_Close(&fod_drv_session);
        fod_drv_params = NULL;
    }
}
#else
static void tee_open_fod_tdriver() {
}

static void tee_close_fod_tdriver() {
}
#endif	//#ifdef TRUSTONIC_TEE_ENABLED

#ifdef GP_TEE_ENABLED
// clang-format off
#define FOD_TA_GP_UUID \
	{ 0x08060000, 0x0000, 0x0000, \
	{ 0x00, 0x00, 0x00, 0x00, 0x19, 0x79, 0x04, 0x14 } }
// clang-format on

#define FOD_TA_GP_UUID_STRING "08060000000000000000000019790414"


static TEEC_Context g_ta_context;
static TEEC_Session g_ta_session;
static TEEC_Operation g_ta_operation;

static void tee_close_fod_ta() {
    TEEC_CloseSession(&g_ta_session);
    TEEC_FinalizeContext(&g_ta_context);
}

static void tee_open_fod_ta() {
    TEEC_Result ret;
    TEEC_UUID destination = FOD_TA_GP_UUID;

    memset(&g_ta_context, 0, sizeof(g_ta_context));
    ret = TEEC_InitializeContext(NULL, &g_ta_context);
    if (ret != TEEC_SUCCESS) {
        ERR_LOG("Could not initialize context with the TEE!!!!!");
        exit(0);
    }

	DBG_LOG("%s TA(0x%08x)\n", __func__, destination.timeLow);
    memset(&g_ta_session, 0, sizeof(g_ta_session));
    ret = TEEC_OpenSession(&g_ta_context, &g_ta_session, &destination, TEEC_LOGIN_PUBLIC, NULL,
                           NULL, NULL);
    if (ret != TEEC_SUCCESS) {
        TEEC_FinalizeContext(&g_ta_context);
        ERR_LOG("Could not open session with Trusted Application!");
        exit(0);
    }
}

static void tee_invoke_cmd(uint32_t cmd);
static void tee_invoke_cmd(uint32_t cmd)
{
    TEEC_Result ret;

    memset(&g_ta_operation, 0, sizeof(g_ta_operation));
    g_ta_operation.paramTypes = TEEC_PARAM_TYPES(
        TEEC_VALUE_INOUT, TEEC_VALUE_INOUT, TEEC_VALUE_INOUT, TEEC_VALUE_INOUT);
    g_ta_operation.params[0].value.a = 0x121;
    g_ta_operation.params[0].value.b = 0x232;

	DBG_LOG("%s TA(%s) cmd(0x%x)\n", __func__, FOD_TA_GP_UUID_STRING, cmd);
    ret = TEEC_InvokeCommand(&g_ta_session, cmd, &g_ta_operation, NULL);

    if (ret != TEEC_SUCCESS) {
        ERR_LOG("Invoke command failed!\n");
        exit(0);
    }

	DBG_LOG("%s TA(%s) cmd(0x%x) done\n",  __func__, FOD_TA_GP_UUID_STRING, cmd);
	DBG_LOG("[1]0x%x 0x%x [2]0x%x 0x%x [3]0x%x 0x%x\n",
		g_ta_operation.params[1].value.a, g_ta_operation.params[1].value.b,
		g_ta_operation.params[2].value.a, g_ta_operation.params[2].value.b,
		g_ta_operation.params[3].value.a, g_ta_operation.params[3].value.b);

	if (cmd==0x10101 || cmd==0x10169 || cmd==0x1016a ||cmd==0x2019) {
		buf_num[0] = g_ta_operation.params[0].value.a;
		buf_num[1] = g_ta_operation.params[1].value.a;
		buf_num[2] = g_ta_operation.params[2].value.a;
		buf_num[3] = g_ta_operation.params[3].value.a;
		prop_size[0] = g_ta_operation.params[0].value.b;
		prop_size[1] = g_ta_operation.params[1].value.b;
		prop_size[2] = g_ta_operation.params[2].value.b;
		prop_size[3] = g_ta_operation.params[3].value.b;
		DBG_LOG("buf num(%u %u %u %u)\n", buf_num[0], buf_num[1], buf_num[2], buf_num[3]);
		DBG_LOG("prop size(%u %u %u %u)\n", prop_size[0], prop_size[1], prop_size[2], prop_size[3]);
	}
	if (cmd==0x10102) {
		if ( (g_ta_operation.params[2].value.a!=100) || (g_ta_operation.params[2].value.b!=100) ||
			 (g_ta_operation.params[3].value.a!=100) || (g_ta_operation.params[3].value.b!=100)  )
		{
			ERR_LOG("%u %u %u %u\n",
				g_ta_operation.params[2].value.a, g_ta_operation.params[2].value.b,
				g_ta_operation.params[3].value.a, g_ta_operation.params[3].value.b);
			ERR_LOG("please check cache clean & invalidate !!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
			Lazy("com.mediatek.gz.fod_sample", 103);     //DumpLog
			if (MULTICORE>=run_case && run_case>=DEFAULT && run_case!=FAKE_SDSP) {
				ReleaseVPU();
			}
			tee_invoke_cmd(0x10103);
			tee_close_fod_ta();
			tee_close_fod_tdriver();
			ion_close(CA_ion_fd);
			exit(0);
		} else
		{
			DBG_LOG("%u %u %u %u $$$$$$$$$$$$$$$$$$$$$$$$$$$$\n",
				g_ta_operation.params[2].value.a, g_ta_operation.params[2].value.b,
				g_ta_operation.params[3].value.a, g_ta_operation.params[3].value.b);
		}
	}
}
#endif

static void Lazy(char *server_name, uint32_t lazy_cmd)
{
	TZ_RESULT ret;
	UREE_SESSION_HANDLE session;
	MTEEC_PARAM param[4];
	uint32_t types;
	DBG_LOG("%s %d\n", __func__, lazy_cmd);

	ret = UREE_CreateSession(server_name, &session);
	if (ret) {
		ERR_LOG("UREE_CreateSession fail(%s)\n", server_name);
		exit(0);
	}

	types = TZ_ParamTypes4(TZPT_VALUE_INOUT, TZPT_VALUE_INOUT, TZPT_VALUE_INOUT, TZPT_VALUE_INOUT);
	param[0].value.a = buf_num[0];
	param[0].value.b = prop_size[0];
	param[1].value.a = buf_num[1];
	param[1].value.b = prop_size[1];
	param[2].value.a = buf_num[2];
	param[2].value.b = prop_size[2];
	param[3].value.a = buf_num[3];
	param[3].value.b = prop_size[3];

	if (lazy_cmd==202 || lazy_cmd==203 || lazy_cmd==206 || lazy_cmd==208 || lazy_cmd==209) {
		param[0].value.a = shm_handle;
		param[0].value.b = file_len;
	}
	if (lazy_cmd==300 || lazy_cmd==301) {
		param[0].value.a = my_rand1;
		param[0].value.b = my_rand2;
		my_rand1++;	//for lazy_cmd_301 verify use
		my_rand2++;
	}

	gettimeofday(&tv1, NULL);
	ret = UREE_TeeServiceCall(session, lazy_cmd, types, param);
	gettimeofday(&tv2, NULL);
	if (ret) {
		ERR_LOG("UREE_TeeServiceCall Error: handle 0x%x, ret %d\n", (uint32_t)session, ret);
		exit(0);
	}

	if (lazy_cmd==202) {
		DBG_LOG("vpu0 cmd(0x%x), result(%u)\n", param[1].value.a, param[1].value.b);
		time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
		DBG_LOG("0x16b cost time: %dus\n", time_cost);
		if (param[1].value.b!=100) {
			exit_test = true;
			DBG_LOG("please check cache flush $$$$$$$$$$\n");
		}
	}
	if (lazy_cmd==203) {
		DBG_LOG("vpu0 cmd(0x%x), result(%u)\n", param[1].value.a, param[1].value.b);
		DBG_LOG("vpu1 cmd(0x%x), result(%u)\n", param[2].value.a, param[2].value.b);
		time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
		DBG_LOG("0x16b cost time: %dus\n", time_cost);
		if (param[1].value.b!=100 || param[2].value.b!=100) {
			exit_test = true;
			DBG_LOG("please check cache flush $$$$$$$$$$\n");
		}
	}
	time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
	DBG_LOG("lazy_cmd(%u) cost time: %dus\n", lazy_cmd, time_cost);

	if (lazy_cmd==300) {
		if (param[1].value.a == 0x100) {
			DBG_LOG("setup rpmb data done\n");
		} else {
			ERR_LOG("error, break\n");
			exit_test = true;
		}
	}

	if (lazy_cmd==301) {
		if (param[1].value.a != 0x100) {
			ERR_LOG("needed value=0x%x, but shm value[%u]=0x%x\n", param[0].value.a, param[2].value.a, param[2].value.b);
			ERR_LOG("error, break(0x%x), shm\n", param[0].value.b);
			exit_test = true;
		}
		DBG_LOG("compare len(0x%x)(0x%x)\n", param[2].value.a, param[3].value.a);
	}

	ret = UREE_CloseSession(session);
	if (ret) {
		ERR_LOG("UREE_CloseSession fail(%s)\n", server_name);
		exit(0);
	}
}

#define MTK_M4U_MAGICNO 'g'
#define MTK_M4U_T_SEC_INIT _IOW(MTK_M4U_MAGICNO, 50, int)
char dmesg_str[256];
char logcat_str[256];
char fod_prop_str[256];
int  fod_prop_val;
static void secure_m4u_open(void) {
	int m4u_fd;
	int ret;

	m4u_fd = open("/proc/m4u", O_RDONLY);
    if (m4u_fd <0) {
        ERR_LOG("%s open fd fail, err = %s\n", __FUNCTION__, strerror(errno));
		if (need_reboot) {
			system("/system/bin/mkdir -p /data/fod_ca");
			property_get("persist.fod_ca.filecnt", fod_prop_str, "1");
			DBG_LOG("persist.fod_ca.filecnt=%s\n", fod_prop_str);
			fod_prop_val = atoi(fod_prop_str);
			DBG_LOG("persist.fod_ca.filecnt=%d\n", fod_prop_val);
			snprintf(dmesg_str, 256, "%s_%02d_open%s", "/system/bin/dmesg > /data/fod_ca/dmesg", fod_prop_val, ".log");
			DBG_LOG("dmesg_str=%s\n", dmesg_str);
			system(dmesg_str);
			snprintf(logcat_str, 256, "%s_%02d_open%s", "/system/bin/logcat -d > /data/fod_ca/logcat", fod_prop_val, ".log");
			DBG_LOG("logcat_str=%s\n", logcat_str);
			system(logcat_str);
			snprintf(fod_prop_str, 16, "%d", ++fod_prop_val);
			property_set("persist.fod_ca.filecnt", fod_prop_str);
			system("/system/bin/sync");
			system("/system/bin/reboot");
		}
		exit(0);
    }

	ret = ioctl(m4u_fd, MTK_M4U_T_SEC_INIT, NULL);
	if (ret!=0) {
		ERR_LOG("%s ioctl MTK_M4U_T_SEC_INIT fail, err = %s\n", __FUNCTION__, strerror(errno));
		if (need_reboot) {
			system("/system/bin/mkdir -p /data/fod_ca");
			property_get("persist.fod_ca.filecnt", fod_prop_str, "1");
			DBG_LOG("persist.fod_ca.filecnt=%s\n", fod_prop_str);
			fod_prop_val = atoi(fod_prop_str);
			DBG_LOG("persist.fod_ca.filecnt=%d\n", fod_prop_val);
			snprintf(dmesg_str, 256, "%s_%02d_ioctl%s", "/system/bin/dmesg > /data/fod_ca/dmesg", fod_prop_val, ".log");
			DBG_LOG("dmesg_str=%s\n", dmesg_str);
			system(dmesg_str);
			snprintf(logcat_str, 256, "%s_%02d_ioctl%s", "/system/bin/logcat -d > /data/fod_ca/logcat", fod_prop_val, ".log");
			DBG_LOG("logcat_str=%s\n", logcat_str);
			system(logcat_str);
			snprintf(fod_prop_str, 16, "%d", ++fod_prop_val);
			property_set("persist.fod_ca.filecnt", fod_prop_str);
			system("/system/bin/sync");
			system("/system/bin/reboot");
		}
		exit(0);
	}
    else
		DBG_LOG("%s ioctl MTK_M4U_T_SEC_INIT done\n", __FUNCTION__);

	ret = close(m4u_fd);
	if (ret!=0) {
		ERR_LOG("%s close fd fail, err = %s\n", __FUNCTION__, strerror(errno));
		if (need_reboot) {
			system("/system/bin/mkdir -p /data/fod_ca");
			property_get("persist.fod_ca.filecnt", fod_prop_str, "1");
			DBG_LOG("persist.fod_ca.filecnt=%s\n", fod_prop_str);
			fod_prop_val = atoi(fod_prop_str);
			DBG_LOG("persist.fod_ca.filecnt=%d\n", fod_prop_val);
			snprintf(dmesg_str, 256, "%s_%02d_close%s", "/system/bin/dmesg > /data/fod_ca/dmesg", fod_prop_val, ".log");
			DBG_LOG("dmesg_str=%s\n", dmesg_str);
			system(dmesg_str);
			snprintf(logcat_str, 256, "%s_%02d_close%s", "/system/bin/logcat -d > /data/fod_ca/logcat", fod_prop_val, ".log");
			DBG_LOG("logcat_str=%s\n", logcat_str);
			system(logcat_str);
			snprintf(fod_prop_str, 16, "%d", ++fod_prop_val);
			property_set("persist.fod_ca.filecnt", fod_prop_str);
			system("/system/bin/sync");
			system("/system/bin/reboot");
		}
		exit(0);
	}
    else
		DBG_LOG("%s close fd done\n", __FUNCTION__);
}

static void AcqureVPU(void) {
    int fd;
	int ret;

    fd = open("/dev/vpu", O_RDWR);
    if (fd <0) {
        ERR_LOG("%s open fd fail, err = %s\n", __FUNCTION__, strerror(errno));
		exit(0);
    }

	gettimeofday(&tv1, NULL);
	ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_ON, NULL);
	gettimeofday(&tv2, NULL);
	time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
	DBG_LOG("VPU_IOCTL_SDSP_POWER_ON cost time: %dus\n", time_cost);
	if (ret!=0) {
		if (ret>0) {
			DBG_LOG("%s ioctl VPU_IOCTL_SDSP_POWER_ON done by cut in line\n", __FUNCTION__);
			ERR_LOG("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		}
		else
			ERR_LOG("%s ioctl VPU_IOCTL_SDSP_POWER_ON fail, err = %s\n", __FUNCTION__, strerror(errno));
	}
    else
		DBG_LOG("%s ioctl VPU_IOCTL_SDSP_POWER_ON done\n", __FUNCTION__);

	ret = close(fd);
	if (ret!=0)
		ERR_LOG("%s close fd fail, err = %s\n", __FUNCTION__, strerror(errno));
    else
		DBG_LOG("%s close fd done\n", __FUNCTION__);
}

static void ReleaseVPU(void) {
    int fd;
	int ret;

    fd = open("/dev/vpu", O_RDWR);
    if (fd <0) {
        ERR_LOG("%s open fd fail, err = %s\n", __FUNCTION__, strerror(errno));
		exit(0);
    }

	gettimeofday(&tv1, NULL);
	ret = ioctl(fd, VPU_IOCTL_SDSP_POWER_OFF, NULL);
	gettimeofday(&tv2, NULL);
	time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
	DBG_LOG("VPU_IOCTL_SDSP_POWER_OFF cost time: %dus\n", time_cost);
	if (ret!=0)
		ERR_LOG("%s ioctl VPU_IOCTL_SDSP_POWER_OFF fail, err = %s\n", __FUNCTION__, strerror(errno));
    else
		DBG_LOG("%s ioctl VPU_IOCTL_SDSP_POWER_OFF done\n", __FUNCTION__);

	ret = close(fd);
	if (ret!=0)
		ERR_LOG("%s close fd fail, err = %s\n", __FUNCTION__, strerror(errno));
    else
		DBG_LOG("%s close fd done\n", __FUNCTION__);
}

static const char mem_srv_name[] = "com.mediatek.geniezone.srv.mem";

int main(int argc, char *argv[])
{
	TZ_RESULT ret=0;
	ion_user_handle_t CA_ion_handle;
	int loop_max=1, loop_cnt=0;
	char *patten_file;
	int read_len;
	int shm_size;
	int fd;
	struct stat st;
	char *shm_buf = NULL;
	UREE_SHAREDMEM_PARAM shm_param;
	UREE_SESSION_HANDLE mem_session;

	(void)argc;
	if ( argv[1]!=NULL ) {
		if (strncmp(argv[1], "-f", sizeof("-f"))==0 ) {
			if (argv[2]==NULL) ERR_LOG("not file for -f\n");
			patten_file = argv[2];
			DBG_LOG("file is %s\n", patten_file);
			fd = open(patten_file, O_RDONLY);
			if (fd == -1) {
				ERR_LOG("file %s not found!\n", patten_file);
				return -1;
			}
			if (fstat(fd, &st)) {
				ERR_LOG("fstat failed!\n");
				return -1;
			}
			file_len = st.st_size;
			shm_size = (file_len + (0x1000-1)) & ~(0x1000-1);
			shm_buf = (char *)memalign(PAGE_SIZE,shm_size);
			memset(shm_buf, 0x98, shm_size);
			read_len = read(fd, (void *)shm_buf, file_len);
			if (read_len != file_len) {
				ERR_LOG("read_len %d != file_len %d\n", read_len, file_len);
				return -1;
			}
			DBG_LOG("read_len %d, file_len %d, shm_size %d\n", read_len, file_len, shm_size);
			close(fd);
			ret = UREE_CreateSession(mem_srv_name, &mem_session);
			if (ret) {
				ERR_LOG("CreateSession mem fail\n");
			}
			shm_param.buffer = (void *)shm_buf;
			shm_param.size = shm_size;
			ret = UREE_RegisterSharedmem(mem_session, &shm_handle, &shm_param);
			if (ret) {
				ERR_LOG("register linux<->mtee shm fail\n");
			}
			run_case = GOODIX_UT_02;
			if ( (argc>3) && (argv[3]!=NULL) && (strncmp(argv[3], "1", sizeof("1"))==0) ) {
				run_case = GOODIX_UT_01;
			}
			if ( (argc>3) && (argv[3]!=NULL) && (strncmp(argv[3], "5", sizeof("3"))==0) ) {
				run_case = MTEE_SDSP_UT_01;
			}
			if ( (argc>3) && (argv[3]!=NULL) && (strncmp(argv[3], "6", sizeof("6"))==0) ) {
				run_case = MTEE_SDSP_UT_02;
			}
			if ( (argc>3) && (argv[3]!=NULL) && (strncmp(argv[3], "7", sizeof("7"))==0) ) {
				run_case = MTEE_SDSP_UT_03;
			}
			if ( (argc>4) && (argv[4]!=NULL) ) {
				loop_max = strtol(argv[4], NULL, 10);
				DBG_LOG("ttt=%d\n", loop_max);
			}
			if ( (argc>5) && (argv[5]!=NULL) && (strncmp(argv[5], "with_ion", sizeof("with_ion"))==0) ) {
				use_ion = true;
				DBG_LOG("use_ion=%d\n", use_ion);
			}
			DBG_LOG("[0]0x%02x [1]0x%02x [%d]0x%02x [%d]0x%02x\n",
				shm_buf[0], shm_buf[1],
				file_len-1, shm_buf[file_len-1],
				file_len, shm_buf[file_len]);
		} else {
			loop_max = strtol(argv[1], NULL, 10);
		}
	}

	if (run_case < GOODIX_UT_01 || run_case > MTEE_SDSP_UT_03)
	{
		if ( argv[2]!=NULL ) {
			if ( strncmp(argv[2], "multicore", sizeof("multicore"))==0 ) {
				run_case = MULTICORE;
			}
		}

		if ( argv[2]!=NULL ) {
			if ( strncmp(argv[2], "justload", sizeof("justload"))==0 ) {
				run_case = JUST_LOAD_TEST;
			}
		}

		if ( argv[2]!=NULL ) {
			if ( strncmp(argv[2], "fake", sizeof("fake"))==0 ) {
				run_case = FAKE_SDSP;
			}
		}

		if ( argv[2]!=NULL ) {
			if ( strncmp(argv[2], "cmds", sizeof("cmds"))==0 ) {
				run_case = CMDS_SDSP;
			}
		}

		if ( argv[2]!=NULL ) {
			if ( strncmp(argv[2], "cmds-reboot", sizeof("cmds-reboot"))==0 ) {
				run_case = CMDS_SDSP;
				need_reboot = true;
			}
		}

		if ( argv[3]!=NULL ) {
			if ( strncmp(argv[3], "with_ion", sizeof("with_ion"))==0 ) {
				use_ion = true;
			}
		}

		if ( argv[2]!=NULL ) {
			if ( strncmp(argv[2], "rpmb", sizeof("rpmb"))==0 ) {
				DBG_LOG("just rpmb test\n");
				srand(time(NULL));
				tee_open_fod_tdriver();
				tee_open_fod_ta();
				while (loop_cnt < loop_max)
				{
					my_rand1=(rand()%255);
					my_rand2=(rand()%255);
					DBG_LOG("test patten=0x%08x 0x%08x\n", my_rand1, my_rand2);
					Lazy("com.mediatek.geniezone.srv.echo", 300);		//assign random into MdShmForMteeTee
					if (loop_cnt==0) {
						tee_invoke_cmd(0x10107);						//do for-ut-test1 mmap
						tee_invoke_cmd(0x10108);						//do for-ut-test2 mmap
					}
					tee_invoke_cmd(0x10109);							//assign test data
					Lazy("com.mediatek.geniezone.srv.echo", 301);		//check random in MdShmForMteeTee
					if (exit_test) break;
					loop_cnt++;
					DBG_LOG("<<<<<<<<<< test pass %06d >>>>>>>>>>\n", loop_cnt);
				}
				tee_invoke_cmd(0x1010a);								//pass pa/size into tee for unmap
				tee_close_fod_ta();
				tee_close_fod_tdriver();
				exit(0);
			}
		}
	}
	DBG_LOG("hello world(%d)\n", ret);

	if (run_case==JUST_LOAD_TEST) {
		DBG_LOG("just load test\n");
		Lazy("com.mediatek.gz.fod_sample", 105);
	}

	while (loop_cnt < loop_max && MULTICORE>=run_case && run_case>=DEFAULT)
	{
		if (use_ion) {
			gettimeofday(&tv1, NULL);
			CA_ion_fd = ion_open();
			gettimeofday(&tv2, NULL);
			time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
			DBG_LOG("ion_open cost time: %dus\n", time_cost);
			DBG_LOG("open CA_ion_fd = %d \n", CA_ion_fd);
			if (CA_ion_fd<=0) {
				ERR_LOG("ion_open fail(%d)\n", CA_ion_fd);
				exit(0);
			}

			// param 4096 can any value, but need >0
			gettimeofday(&tv1, NULL);
			ret = ion_alloc(CA_ion_fd, 4096, 0, ION_HEAP_MULTIMEDIA_SDSP_SHARED_MASK, 0, &CA_ion_handle);
			gettimeofday(&tv2, NULL);
			time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
			DBG_LOG("ion_alloc cost time: %dus\n", time_cost);
			if (ret) {
				ERR_LOG("ion_alloc fail(%d)\n", ret);
				exit(0);
			}
			DBG_LOG("ion_alloc done\n");
		}

		if (run_case != FAKE_SDSP) {
			secure_m4u_open();
		}

		if (run_case == GOODIX_UT_01)
		{
			DBG_LOG("GOODIX_UT_01\n");
			AcqureVPU();
			tee_open_fod_ta();
			tee_invoke_cmd(0x10105);						//setup secure m4u table
			Lazy("com.mediatek.gz.fod_sample", 106);		//BOOT
			Lazy("com.mediatek.gz.fod_sample", 200);		//Get tee/sDSP shm
			Lazy("com.mediatek.gz.fod_sample", 202);		//Setup Data & D2D(0x169)
			Lazy("com.mediatek.gz.fod_sample", 103);		//DumpLog
			Lazy("com.mediatek.gz.fod_sample", 201);		//Release tee/sDSP shm
			ReleaseVPU();
			tee_close_fod_ta();
			if (exit_test) exit(0);
		}
		else if (run_case == GOODIX_UT_02)
		{
			DBG_LOG("GOODIX_UT_02\n");
			AcqureVPU();
			tee_open_fod_ta();
			tee_invoke_cmd(0x10105);						//setup secure m4u table
			Lazy("com.mediatek.gz.fod_sample", 106);		//BOOT
			Lazy("com.mediatek.gz.fod_sample", 200);		//Get tee/sDSP shm
			Lazy("com.mediatek.gz.fod_sample", 203);		//Setup Data & D2D(0x16b)
			Lazy("com.mediatek.gz.fod_sample", 103);		//DumpLog
			Lazy("com.mediatek.gz.fod_sample", 201);		//Release tee/sDSP shm
			ReleaseVPU();
			tee_close_fod_ta();
			if (exit_test) exit(0);
		}
		else if (run_case == MTEE_SDSP_UT_01)
		{
			DBG_LOG("MTEE_SDSP_UT_01\n");
			AcqureVPU();
			tee_open_fod_ta();
			tee_invoke_cmd(0x10105);						//setup secure m4u table
			Lazy("com.mediatek.gz.fod_sample", 106);		//BOOT
			Lazy("com.mediatek.gz.fod_sample", 200);		//Get tee/sDSP shm
			Lazy("com.mediatek.gz.fod_sample", 206);		//just 0x16c for vpu0 then 0x16c for vpu1
			Lazy("com.mediatek.gz.fod_sample", 201);		//Release tee/sDSP shm
			ReleaseVPU();
			tee_close_fod_ta();
			if (exit_test) exit(0);
		}
		else if (run_case == MTEE_SDSP_UT_02)
		{
			DBG_LOG("MTEE_SDSP_UT_02\n");
			AcqureVPU();
			tee_open_fod_ta();
			tee_invoke_cmd(0x10105);						//setup secure m4u table
			Lazy("com.mediatek.gz.fod_sample", 210);		//Create Thread
			Lazy("com.mediatek.gz.fod_sample", 106);		//BOOT
			Lazy("com.mediatek.gz.fod_sample", 200);		//Get tee/sDSP shm
			Lazy("com.mediatek.gz.fod_sample", 208);		//service_call use vpu0, fork_thread use vpu1
			Lazy("com.mediatek.gz.fod_sample", 201);		//Release tee/sDSP shm
			ReleaseVPU();
			tee_close_fod_ta();
			if (exit_test) exit(0);
		}
		else if (run_case == MTEE_SDSP_UT_03)
		{
			DBG_LOG("MTEE_SDSP_UT_02\n");
			AcqureVPU();
			tee_open_fod_ta();
			tee_invoke_cmd(0x10105);
			Lazy("com.mediatek.gz.fod_sample", 210);		//Create Thread
			Lazy("com.mediatek.gz.fod_sample", 106);		//BOOT
			Lazy("com.mediatek.gz.fod_sample", 200);		//Get tee/sDSP shm
			Lazy("com.mediatek.gz.fod_sample", 209);		//service_call use vpu1, fork_thread use vpu1
			Lazy("com.mediatek.gz.fod_sample", 201);		//Release tee/sDSP shm
			ReleaseVPU();
			tee_close_fod_ta();
			if (exit_test) exit(0);
		}
		else if (run_case == FAKE_SDSP)
		{
			tee_open_fod_tdriver();
			tee_open_fod_ta();
			tee_invoke_cmd(0x2019);
		}
		else
		{
			tee_open_fod_tdriver();
			tee_open_fod_ta();
			tee_invoke_cmd(0x10101);
		}

		if (run_case==DEFAULT) {
			AcqureVPU();
			Lazy("com.mediatek.gz.fod_sample", 107);		//DBG+D2D(0x168)
			tee_invoke_cmd(0x10102);
			Lazy("com.mediatek.gz.fod_sample", 103);		//DumpLog
			ReleaseVPU();

			AcqureVPU();
			Lazy("com.mediatek.gz.fod_sample", 107);		//DBG+D2D(0x168)
			tee_invoke_cmd(0x10102);
			Lazy("com.mediatek.gz.fod_sample", 103);		//DumpLog
			ReleaseVPU();
		}
		if (run_case==CMDS_SDSP) {
			AcqureVPU();
			Lazy("com.mediatek.gz.fod_sample", 106);		//BOOT
			Lazy("com.mediatek.gz.fod_sample", 108);		//D2D(0x168)
			tee_invoke_cmd(0x10102);

			tee_invoke_cmd(0x10169);
			Lazy("com.mediatek.gz.fod_sample", 108);		//D2D(0x169)
			Lazy("com.mediatek.gz.fod_sample", 109);		//use reg to replace to D2D(0x16a)

			tee_invoke_cmd(0x1016a);
			Lazy("com.mediatek.gz.fod_sample", 108);		//D2D(0x16a)

			Lazy("com.mediatek.gz.fod_sample", 103);		//DumpLog
			ReleaseVPU();
		}
		if (run_case==FAKE_SDSP) {

			DBG_LOG(" FOD fake DSP UT \n");
			ret = UREE_FOD_ShareMem_Enable(1);
			Lazy("com.mediatek.gz.fod_sample", 2019); //Get tee/sDSP shm & copy vpu buffer
			Lazy("com.mediatek.gz.fod_sample", 201); //Release tee/sDSP shm
			ret = UREE_FOD_ShareMem_Enable(0);

		}
		if (run_case==MULTICORE) {
			AcqureVPU();
			DBG_LOG("run multicore cmd\n");
			Lazy("com.mediatek.gz.fod_sample", 178);
			ReleaseVPU();
		}

		if (run_case>=FAKE_SDSP) {
			tee_invoke_cmd(0x10102);
		}


		if (MTEE_SDSP_UT_03>=run_case && run_case>=GOODIX_UT_01) {
			if (loop_cnt+1 == loop_max) {
				ret = UREE_UnregisterSharedmem(mem_session, shm_handle);
				if (ret) {
					ERR_LOG("UnregisterSharedmem fail\n");
				}
				ret = UREE_CloseSession(mem_session);
				if (ret) {
					ERR_LOG("CloseSession mem_session fail\n");
				}
				if (shm_buf != NULL)
					free(shm_buf);
			} else {
				usleep(1000*1000*2);
			}
		} else {
			tee_invoke_cmd(0x10103);
			tee_close_fod_ta();
			tee_close_fod_tdriver();
		}

		/*
		ret = UREE_ReleaseTeeSdspShm();
		if (ret) {
			DBG_LOG("UREE_ReleaseTeeSdspShm fail(%d)\n", ret);
			return 0;
		}
		*/

		if (use_ion) {
			gettimeofday(&tv1, NULL);
			ret = ion_close(CA_ion_fd);
			gettimeofday(&tv2, NULL);
			time_cost = (tv2.tv_sec-tv1.tv_sec)*1000000 + (tv2.tv_usec-tv1.tv_usec);
			DBG_LOG("ion_close cost time: %dus\n", time_cost);
			if (ret) {
				ERR_LOG("ion_close fail(%d)\n", ret);
				exit(0);
			}
		}

		loop_cnt++;
		DBG_LOG("<<<<<<<<<< test pass %06d >>>>>>>>>>\n", loop_cnt);
		if (need_reboot) {
			/*
			property_get("persist.fod_ca.filecnt", fod_prop_str, "1");
			DBG_LOG("persist.fod_ca.filecnt=%s\n", fod_prop_str);
			fod_prop_val = atoi(fod_prop_str);
			DBG_LOG("persist.fod_ca.filecnt=%d\n", fod_prop_val);
			snprintf(dmesg_str, 256, "%s_%02d_00%s", "/system/bin/dmesg > /data/dmesg", fod_prop_val, ".log");
			DBG_LOG("dmesg_str=%s\n", dmesg_str);
			system(dmesg_str);
			//system("/system/bin/dmesg > /data/dmesg.log");
			snprintf(logcat_str, 256, "%s_%02d_00%s", "/system/bin/logcat -d > /data/logcat", fod_prop_val, ".log");
			DBG_LOG("logcat_str=%s\n", logcat_str);
			system(logcat_str);
			//system("/system/bin/logcat -d > /data/logcat.log");
			snprintf(fod_prop_str, 16, "%d", ++fod_prop_val);
			property_set("persist.fod_ca.filecnt", fod_prop_str);
			*/
			system("/system/bin/reboot");
		}
	}
}
