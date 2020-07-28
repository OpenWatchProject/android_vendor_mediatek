# Copyright (c) 2015-2018 MICROTRUST Incorporated
# All rights reserved
#
# This file and software is confidential and proprietary to MICROTRUST Inc.
# Unauthorized copying of this file and software is strictly prohibited.
# You MUST NOT disclose this file and software unless you get a license
# agreement from MICROTRUST Incorporated.

ATF_MAJOR_VERSION := $(shell cd ../../../ | grep "VERSION_MAJOR" Makefile | grep -v "VERSION_STRING" | sed -r "s/:= /\\n/g" | grep -v "VERSION" | sed -r "s/\\n//g")
ATF_MINOR_VERSION := $(shell cd ../../../ | grep "VERSION_MINOR" Makefile | grep -v "VERSION_STRING" | sed -r "s/:= /\\n/g" | grep -v "VERSION" | sed -r "s/\\n//g")
CFLAGS += -DATF_MAJOR_VERSION=$(ATF_MAJOR_VERSION) -DATF_MINOR_VERSION=$(ATF_MINOR_VERSION)
ASFLAGS += -DATF_MAJOR_VERSION=$(ATF_MAJOR_VERSION) -DATF_MINOR_VERSION=$(ATF_MINOR_VERSION)

MTK_PLATFORM := $(shell echo $(PLAT) | grep -o "[0-9]\+")
CFLAGS += -DMTK_PLATFORM=$(MTK_PLATFORM)

TEEID_DIR		:=	services/spd/teeid/300/

SPD_INCLUDES		:=	-I${TEEID_DIR}

SPD_SOURCES		:=	teei_fastcall.c	\
	teei_main.c		\
	teei_helpers.S		\
	teei_common.c	\
	teei_pm.c

vpath %.c ${TEEID_DIR}
vpath %.S ${TEEID_DIR}
