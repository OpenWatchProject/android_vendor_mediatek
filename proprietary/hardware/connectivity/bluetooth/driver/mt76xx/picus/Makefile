#---------------------------------------------------------------------------
TARGET = picus

OBJECT = \
		 main.o \
		 common.o \
		 bperf_util.o \
		 $(NULL)

CFLAGS = \
		 -g \
		 -Wall \
		 -Werror \
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
all: picus

picus: clean $(OBJECT)
	$(CC) -pthread -C -o $(TARGET) $(OBJECT)
	$(STRIP) $(TARGET)

%.o: %.c
	$(CC) $(LINKFLAGS) $(CFLAGS) $(INCLUDE) -c -o $@ $<

clean:
	rm -f $(TARGET) *.o

#---------------------------------------------------------------------------
