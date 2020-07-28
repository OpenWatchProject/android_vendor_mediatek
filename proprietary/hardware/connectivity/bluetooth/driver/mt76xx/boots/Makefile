include ./Makefile.rules

OBJS_BOOTS = \
    boots.o \
    boots_pkt.o \
    boots_skt.o \
    boots_eth.o \
    boots_uart.o \
    boots_script.o \
    $(NULL)

OBJS_BOOTS_SRV = \
    boots_srv.o \
    boots_btif.o \
    boots_pkt.o \
    boots_skt.o \
    boots_eth.o \
    boots_mt6xx.o \
    boots_uart.o \
    boots_script.o \
    $(NULL)

CCFLAGS = \
    -Werror \
    -Wall \
    $(LOCAL_CFLAGS) \
    $(NULL)

LINKFLAGS = \
    $(NULL)

#---------------------------------------------------------------------------
ifeq ($(CROSS_COMPILE),)
    CC = gcc
    STRIP = strip
else
    CC = $(CROSS_COMPILE)-gcc
    STRIP = $(CROSS_COMPILE)-strip
endif

#---------------------------------------------------------------------------
all: boots boots_srv

%.o: %.c Makefile
	$(CC) $(CCFLAGS) $(INCLUDES) -c -o $@ $<

boots: $(OBJS_BOOTS)
	$(CC) -static $(OBJS_BOOTS) $(LINKFLAGS) -o boots
	$(STRIP) boots

boots_srv: $(OBJS_BOOTS_SRV)
	$(CC) -static $(OBJS_BOOTS_SRV) $(LINKFLAGS) -o boots_srv
	$(STRIP) boots_srv

clean:
	rm -rf boots
	rm -rf boots_srv
	rm -rf *.o

#---------------------------------------------------------------------------
