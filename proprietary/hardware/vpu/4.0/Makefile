objs := earaUtil.o vpuBuffer.o vpuImpl.o

SUBDIRS = test
CXXFLAGS += -std=c++11
CXXFLAGS += -Iplatform/mt6779
CXXFLAGS += -I.
CXXFLAGS += -I../kmod
CXXFLAGS += -I../../apusys_driver/vpu/4.0
CXXFLAGS += -I../../apusys_driver/power/1.0
CXXFLAGS += -I$(ALPS)/system/core/libion/include
CXXFLAGS += -I$(ALPS)/system/core/libion/kernel-headers
CXXFLAGS += -I$(ALPS)/vendor/mediatek/proprietary/external
CXXFLAGS += -I$(ALPS)/device/mediatek/common/kernel-headers
CXXFLAGS += -I$(ALPS)/device/mediatek/$(PLATFORM)/kernel-headers

all: libvpu.a
#all: libvpu.a #test_dir

libvpu.a: $(objs)
	${AR} rcs libvpu.a $(objs)

%.o:%.c
	$(CXX) -c $^ -o $@ ${CFLAGS}

# test_dir:
# 	$(MAKE) -C test CC=${CC} AR=${AR} CFLAGS=${CFLAGS} STRIP=${STRIP} ALPS=$(ALPS)

#.PHONY: all clean

clean:
	rm -f *.o *.a
	$(MAKE) -C test clean
