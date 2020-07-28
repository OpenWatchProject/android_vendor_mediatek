
MTEE_DIR		:=	services/spd/mtee
SPD_INCLUDES		:=	-Iinclude/bl32/payloads \
				-I${MTEE_DIR}/include \
				-I${MTEE_DIR}

SPD_SOURCES		:=	mtee_common.c		\
				mtee_helpers.S	\
				mtee_main.c		\
				mtee_pm.c

MTEE32 := 1
$(eval $(call add_define,MTEE32))

vpath %.c ${MTEE_DIR}
vpath %.S ${MTEE_DIR}

