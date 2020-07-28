#include <tz_private/swreg_common.h>

enum GZ_DREG_ID {
	GZ_DREG_FPSGO = 0,
	GZ_DREG_CODEC,
	GZ_DREG_NUM
};

struct GZ_data_reg_config {
	enum GZ_DREG_ID id;
	unsigned long long base;
	unsigned int size;
    void *va;
};

#define GZ_DREG_OFFSET_FPSGO 0x0000
#define GZ_DREG_OFFSET_CODEC 0x0200

#define GZ_DREG_SIZE_FPSGO 0x0200
#define GZ_DREG_SIZE_CODEC 0x0200

#define GZ_DREG_PA(offset) (DREG_BASE + offset)
