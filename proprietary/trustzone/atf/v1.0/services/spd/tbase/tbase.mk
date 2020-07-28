

ifneq (${SOC},)
    $(info "kinibi SPD found for SOC ${SOC}")
    SPD_PLAT := ${SOC}
else
    $(info "Kinibi SPD found for platform ${PLAT}")
    SPD_PLAT := ${PLAT}
endif

-include plat/${SPD_PLAT}/plat_tbase.mk

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
                -Iplat/${SPD_PLAT}/include \
                -Iplat/${SPD_PLAT}/include/${TBASE_AARCH}
SPD_SOURCES += tbase_fastcall.c \
               tbase_setup.c \
               tbase_helpers.S \
               tbase_common.c

ifeq (${TBASE_AARCH}, kinibi_aarch32)
SPD_SOURCES += tbase_pm.c
endif

vpath %.c ${TBASE_SPD_DIR}
vpath %.S ${TBASE_SPD_DIR}
