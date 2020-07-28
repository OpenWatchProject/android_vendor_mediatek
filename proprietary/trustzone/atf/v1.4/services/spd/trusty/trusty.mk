#
# Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#

TRUSTY_DIR		:= services/spd/trusty
SPD_INCLUDES	:= -I${TRUSTY_DIR}


SPD_SOURCES		:=	services/spd/trusty/trusty.c		\
				services/spd/trusty/trusty_helpers.S

NEED_BL32		:= no
