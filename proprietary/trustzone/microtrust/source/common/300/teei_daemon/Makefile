CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)gcc

TARGET := teei_daemon
OBJS := main.o ../android_backend/virt32/properties.o

$(TARGET) : $(OBJS)
	$(LD) -o $@ $^ -lpthread

CFLAGS := -I../android_backend/virt32/include #-DMICROTRUST_TEE_DEBUG_BUILD
CFLAGS += -DSYSTEM_THH_PATH='"/system/thh/"'

clean:
	rm -f $(OBJS) $(TARGET)
