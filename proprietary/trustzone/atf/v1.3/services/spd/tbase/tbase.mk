##
# Copyright (c) 2016 TRUSTONIC LIMITED.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##

##
# Copyright (c) 2014, ARM Limited and Contributors. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# 3. Neither the name of the ARM nor the names of its
#    contributors may be used to endorse or promote products derived from
#    this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
# TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
# OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
# OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
# ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
##

ifneq (${SOC},)
    $(info Kinibi SPD found for SOC ${SOC})
    SPD_PLAT := ${SOC}
else
    $(info "Kinibi SPD found for platform ${PLAT}")
    SPD_PLAT := ${PLAT}
endif

-include plat/mediatek/${SPD_PLAT}/plat_tbase.mk

TBASE_AARCH ?= kinibi_aarch32
TBASE_SPD_DIR := services/spd/tbase/${TBASE_AARCH}

$(info TBASE_AARCH = ${TBASE_AARCH})

ifeq (${VERSION_MAJOR},)
    $(warning "VERSION_MAJOR not set. Forcing version to 0.4")
    VERSION_MAJOR:=0
    VERSION_MINOR:=4
endif

CFLAGS += "-DVERSION_MAJOR=${VERSION_MAJOR}"
CFLAGS += "-DVERSION_MINOR=${VERSION_MINOR}"

ASFLAGS += "-DVERSION_MAJOR=${VERSION_MAJOR}"
ASFLAGS += "-DVERSION_MINOR=${VERSION_MINOR}"

$(info "CFLAGS: ${CFLAGS}")
SPD_INCLUDES += -Iinclude/bl32/tsp \
                -I${TBASE_SPD_DIR} \
                -Iplat/mediatek/${SPD_PLAT}/include \
                -Iplat/mediatek/${SPD_PLAT}/include/${TBASE_AARCH}
SPD_SOURCES += tbase_fastcall.c \
               tbase_setup.c \
               tbase_helpers.S \
               tbase_common.c

ifeq (${TBASE_AARCH}, kinibi_aarch32)
SPD_SOURCES += tbase_pm.c
endif

vpath %.c ${TBASE_SPD_DIR}
vpath %.S ${TBASE_SPD_DIR}

