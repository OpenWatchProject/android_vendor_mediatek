#	@see http://mtkswits01/~wimac
#	@see libbase/Makefile
#	@see libbase/Makefile.config
PREFIX   = ../install
NAME     = volte_rcs_ua
COMPILER = GNU
CROSS    = arm-none-linux-
CROSS_SUP= arm-none-linux- native arm-eabi-
CFLAGS   = $(DEF_CFLAGS) $(THUMB) $(OFLAGS)  -mthumb
CXXFLAGS = $(CFLAGS) $(CXXFLAGS_ADD)
MIN      = 1
MLOG_MODULE = 0
CONFIG   = -O0
CONFIG  += -g

CONFIG  += -D ENABLED_EXTERNAL_TIMER

ifeq ($(shell echo $(CROSS) | grep -e "arm-none-eabi-\|arm-none-elf-\|i586-mingw32msvc-"),)
  ##
  # NEWLIB=2: using linux
  #
  ##
  LINUX  = 1
  NEWLIB = 2
  DEP_LIBS = base
else
  LINUX = 0
  ##
  # @brief OS switch 
  #		NEWLIB=0: using eCos
  #		NEWLIB=1: using threadX
  ##
  NEWLIB   = 0
  DEP_LIBS = base
endif

DEP_LIBS +=  volte_core volte_xdmc

##
#	@brief
#		SUBVPATH
##
SUBVPATH    += src              \
               src/lib          \
               src/comm         \
               src/receiver   \
               src/include   \


INCLUDE_ADD += -I ../install/arm-none-linux/usr/include \
               -I ../../../frameworks/opt/volte/volte_core/include   \
               -I src/include       \

