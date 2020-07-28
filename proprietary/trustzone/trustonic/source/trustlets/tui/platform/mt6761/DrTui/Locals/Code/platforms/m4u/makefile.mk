
LOCAL_GENERIC_PATH = Locals/Code/platforms/generic
LOCAL_M4U_PATH = Locals/Code/platforms/m4u
LOCAL_CMDQ_PATH = Locals/Code/platforms/cmdq

INCLUDE_DIRS += ${LOCAL_CMDQ_PATH}/
INCLUDE_DIRS += ${LOCAL_M4U_PATH}/
INCLUDE_DIRS += ${LOCAL_GENERIC_PATH}/

SRC_C += ${LOCAL_M4U_PATH}/tui_m4u.c
