
LOCAL_GENERIC_PATH = Locals/Code/platforms/generic
LOCAL_DISP_PATH = Locals/Code/platforms/display/$(TEE_MACH_TYPE)
LOCAL_CMDQ_PATH = Locals/Code/platforms/cmdq
LOCAL_M4U_PATH = Locals/Code/platforms/m4u

INCLUDE_DIRS += ${LOCAL_DISP_PATH}/include/
INCLUDE_DIRS += ${LOCAL_CMDQ_PATH}/
INCLUDE_DIRS += ${LOCAL_DISP_PATH}/
INCLUDE_DIRS += ${LOCAL_M4U_PATH}/
#INCLUDE_DIRS += ${LOCAL_DISP_PATH}/$(TEE_MACH_TYPE)/
INCLUDE_DIRS += ${LOCAL_GENERIC_PATH}/

SRC_C += ${LOCAL_DISP_PATH}/display_tui.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_drv.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_info.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_debug.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_path.c
#SRC_C += ${LOCAL_DISP_PATH}/$(TEE_MACH_TYPE)/ddp_rdma.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_rdma.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_dsi.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_dump.c
SRC_C += ${LOCAL_DISP_PATH}/ddp_color_format.c

