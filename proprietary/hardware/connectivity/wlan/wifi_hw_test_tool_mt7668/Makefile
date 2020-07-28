#
# Makefile for libwifitest
#

# definitions
# ------------------------------------------------------------------------------

# wifitest tool info
PROGRAM				 = wifitest


# compiling and linking
# ------------------------------------------------------------------------------
all: $(PROGRAM)
clean:
	-rm -f $(PROGRAM)
$(PROGRAM): main.c lib.c libwifitest.c 
	$(CC) $(LDFLAGS) -o $@ $< lib.c libwifitest.c
