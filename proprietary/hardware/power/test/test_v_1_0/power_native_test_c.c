#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <fcntl.h>
#include <errno.h>
#include <dlfcn.h>

#define LIB_FULL_NAME "libpowerhalwrap.so"

int (*phalwrap_mtk_power_hint)(int, int) = NULL;
int (*phalwrap_mtk_cus_power_hint)(int, int) = NULL;
int (*phalwrap_query_sys_info)(int, int) = NULL;
int (*phalwrap_scn_reg)(void) = NULL;
int (*phalwrap_scn_config)(int, int, int, int, int, int) = NULL;
int (*phalwrap_scn_unreg)(int) = NULL;
int (*phalwrap_scn_enable)(int, int) = NULL;
int (*phalwrap_scn_disable)(int) = NULL;

typedef int (*mtk_power_hint)(int, int);
typedef int (*mtk_cus_power_hint)(int, int);
typedef int (*query_sys_info)(int, int);
typedef int (*scn_reg)(void);
typedef int (*scn_config)(int, int, int, int, int, int);
typedef int (*scn_unreg)(int);
typedef int (*scn_enable)(int, int);
typedef int (*scn_disable)(int);

enum {
    CMD_POWER_HINT = 1,
    CMD_CUS_POWER_HINT,
    CMD_QUERY_INFO,
    CMD_SCN_REG,
    CMD_SCN_CONFIG,
    CMD_SCN_UNREG,
    CMD_SCN_ENABLE,
    CMD_SCN_DISABLE,
};


static void usage(char *cmd);
static int load_api(void);

int main(int argc, char* argv[])
{
    int command=0, hint=0, timeout=0, data=0;
    int cmd=0, p1=0, p2=0, p3=0, p4=0;
    int handle = -1;

    if(argc < 2) {
		usage(argv[0]);
        return 0;
    }

    command = atoi(argv[1]);
    //printf("argc:%d, command:%d\n", argc, command);
    switch(command) {
        case CMD_SCN_REG:
            if(argc!=2) {
                usage(argv[0]);
                return -1;
            }
            break;

        case CMD_SCN_UNREG:
        case CMD_SCN_DISABLE:
            if(argc!=3) {
                usage(argv[0]);
                return -1;
            }
            break;

        case CMD_POWER_HINT:
        case CMD_CUS_POWER_HINT:
        case CMD_QUERY_INFO:
        case CMD_SCN_ENABLE:
            if(argc!=4) {
                usage(argv[0]);
                return -1;
            }
            break;

        case CMD_SCN_CONFIG:
            if(argc!=8) {
                usage(argv[0]);
                return -1;
            }
            break;

        default:
            usage(argv[0]);
            return -1;
    }

	/* load api */
	if(load_api() != 0)
		return -1;
    
   if(command == CMD_POWER_HINT || command == CMD_CUS_POWER_HINT) {
        hint = atoi(argv[2]);
        data = atoi(argv[3]);
    }
    else if(command == CMD_QUERY_INFO) {
        cmd = atoi(argv[2]);
        p1 = atoi(argv[3]);
    }
    else if(command == CMD_SCN_UNREG || command == CMD_SCN_DISABLE) {
        handle = atoi(argv[2]);
    }
    else if(command == CMD_SCN_ENABLE) {
        handle = atoi(argv[2]);
        timeout = atoi(argv[3]);
    }
    else if(command == CMD_SCN_CONFIG) {
        handle = atoi(argv[2]);
        cmd = atoi(argv[3]);
        p1 = atoi(argv[4]);
        p2 = atoi(argv[5]);
        p3 = atoi(argv[6]);
        p4 = atoi(argv[7]);
    }

    /* command */
    if(command == CMD_POWER_HINT)
        phalwrap_mtk_power_hint(hint, data);
    else if(command == CMD_CUS_POWER_HINT)
        phalwrap_mtk_cus_power_hint(hint, data);
    else if(command == CMD_QUERY_INFO) {
        data = phalwrap_query_sys_info(cmd, p1);
        printf("data:%d\n", data);
    }
    else if(command == CMD_SCN_REG) {
        handle = phalwrap_scn_reg();
        printf("handle:%d\n", handle);
    }
    else if(command == CMD_SCN_CONFIG)
        phalwrap_scn_config(handle, cmd, p1, p2, p3, p4);
    else if(command == CMD_SCN_UNREG)
        phalwrap_scn_unreg(handle);
    else if(command == CMD_SCN_ENABLE)
        phalwrap_scn_enable(handle, timeout);
    else if(command == CMD_SCN_DISABLE)
        phalwrap_scn_disable(handle);

    return 0;
}

static int load_api(void)
{
    void *handle, *func;

	handle = dlopen(LIB_FULL_NAME, RTLD_NOW);

	func = dlsym(handle, "PowerHal_Wrap_mtkPowerHint");
	phalwrap_mtk_power_hint = (mtk_power_hint)(func);

	if (phalwrap_mtk_power_hint == NULL) {
        printf("phalwrap_mtk_power_hint error: %s", dlerror());
		dlclose(handle);
		return 0;
	}

	func = dlsym(handle, "PowerHal_Wrap_mtkCusPowerHint");
	phalwrap_mtk_cus_power_hint = (mtk_cus_power_hint)(func);

	if (phalwrap_mtk_cus_power_hint == NULL) {
        printf("phalwrap_mtk_cus_power_hint error: %s", dlerror());
		dlclose(handle);
		return 0;
	}

	func = dlsym(handle, "PowerHal_Wrap_querySysInfo");
	phalwrap_query_sys_info = (query_sys_info)(func);

	if (phalwrap_query_sys_info == NULL) {
        printf("phalwrap_query_sys_info error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PowerHal_Wrap_scnReg");
	phalwrap_scn_reg = (scn_reg)(func);

	if (phalwrap_scn_reg == NULL) {
        printf("phalwrap_scn_reg error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PowerHal_Wrap_scnConfig");
	phalwrap_scn_config = (scn_config)(func);

	if (phalwrap_scn_config == NULL) {
        printf("phalwrap_scn_config error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PowerHal_Wrap_scnUnreg");
	phalwrap_scn_unreg = (scn_unreg)(func);

	if (phalwrap_scn_unreg == NULL) {
        printf("phalwrap_scn_unreg error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PowerHal_Wrap_scnEnable");
	phalwrap_scn_enable = (scn_enable)(func);

	if (phalwrap_scn_enable == NULL) {
        printf("phalwrap_scn_enable error: %s", dlerror());
		dlclose(handle);
		return -1;
	}

	func = dlsym(handle, "PowerHal_Wrap_scnDisable");
	phalwrap_scn_disable = (scn_disable)(func);

	if (phalwrap_scn_disable == NULL) {
        printf("phalwrap_scn_disable error: %s", dlerror());
		dlclose(handle);
		return -1;
	}


    return 0;
}

static void usage(char *cmd) {
    fprintf(stderr, "\nUsage: %s command scenario\n"
                    "    command\n"
                    "        1: MTK power hint\n"
                    "        2: MTK cus power hint\n"
                    "        3: query info\n"
                    "        4: scn register\n"
                    "        5: scn config\n"
                    "        6: scn unregister\n"
                    "        7: scn enable\n"
                    "        8: scn disable\n", cmd);
}

