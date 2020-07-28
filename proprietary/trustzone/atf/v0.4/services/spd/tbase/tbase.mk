

TBASE_DIR		:=	services/spd/tbase
SPD_INCLUDES		:=	-Iinclude/bl32/payloads \
				-I${TBASE_DIR}

SPD_SOURCES		:=	tbase_fastcall.c	\
				tbase_setup.c		\
				tbase_pm.c		\
				tbase_helpers.S		\
				tbase_common.c

vpath %.c ${TBASE_DIR}
vpath %.S ${TBASE_DIR}

