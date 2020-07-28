OBJS := main.o

INC = ../
STATIC_LIB=../libvpu.a

CXXFLAGS += -std=c++11
CXXFLAGS += -Iplatform/mt6779
CXXFLAGS += -I.
CXXFLAGS += -I../
CXXFLAGS += -I../kmod
CXXFLAGS += -I../../../apusys_driver/vpu/4.0
CXXFLAGS += -I../../../apusys_driver/power/1.0
CXXFLAGS += -I$(ALPS)/system/core/libion/include
CXXFLAGS += -I$(ALPS)/system/core/libion/kernel-headers
CXXFLAGS += -I$(ALPS)/vendor/mediatek/proprietary/external
CXXFLAGS += -I$(ALPS)/device/mediatek/common/kernel-headers
CXXFLAGS += -I$(ALPS)/device/mediatek/$(PLATFORM)/kernel-headers

all: vpu_test

vpu_test : $(OBJS)
	$(CXX) -o vpu_test $(OBJS) -I$(INC) -L../ -lvpu $(CFLAGS)
	# We shall try dynmic link version first
	#$(CXX) -o vpu_test $(OBJS) -I$(INC) -L../ -lvpu $(CFLAGS) -static

%.o:%.c
	$(CXX) -c $^ -o $@ ${CFLAGS}


clean:
	rm -f *.o *.out vpu_test
