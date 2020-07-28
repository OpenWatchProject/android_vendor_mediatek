SHELL = /bin/sh

COMPILER = g++
CROSS_PREFIX =
#CROSS_PREFIX = arm-linux-
CC = $(CROSS_PREFIX)$(COMPILER)
LD = $(CROSS_PREFIX)$(COMPILER)
RM = rm -f
CFLAGS = -Wall -DLINUX

OBJS = flash_cali_tools.o flash_cali_utils.o flash_cali_xml.o flash_custom_utils.o
PROGS = flash_cali_tools

MAKE = make
DIRS = lib

.PHONY: all clean binary

all: $(PROGS)

$(PROGS): $(OBJS)
	$(CC) -o $(PROGS) $(OBJS)

flash_cali_tools.o:
	$(CC) -o $@ $(CFLAGS) -c flash_cali_tools.cpp

flash_cali_utils.o:
	$(CC) -o $@ $(CFLAGS) -c flash_cali_utils.cpp

flash_cali_xml.o:
	$(CC) -o $@ $(CFLAGS) -c flash_cali_xml.cpp

flash_custom_utils.o:
	$(CC) -o $@ $(CFLAGS) -c flash_custom_utils.cpp

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	@-$(RM) $(PROGS) $(PROGS_D) $(OBJS) 

