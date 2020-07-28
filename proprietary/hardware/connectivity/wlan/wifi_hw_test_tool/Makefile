#
# Makefile for libwifitest
#

# definitions
# ------------------------------------------------------------------------------
CFLAGS				 = -pipe -g -Os $(EXTRA_CFLAGS)

# library info
# ------------------------------------------------------------------------------
LIBNAME				 = libwifitest_adb
LIB					 = $(LIBNAME).so
CFLAGS				+= -fPIC
LDFLAGS				+= -shared -W1,-soname,$(LIB)


# wifitest tool info
PROGRAM				 = wifitest
# objects
# ------------------------------------------------------------------------------
OBJS = libwifitest.o iwlib.o lib.o

# compiling and linking
# ------------------------------------------------------------------------------
all: $(LIB) $(PROGRAM)
clean:
	-rm -f $(OBJS) $(LIB) $(PROGRAM)
$(LIB): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)
$(PROGRAM): main.c
	$(CC) -L . -lwifitest -lm -o $@ $<
