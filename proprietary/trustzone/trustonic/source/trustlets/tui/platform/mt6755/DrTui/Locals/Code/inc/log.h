#include "drStd.h"
#include "DrApi/DrApi.h"

#if !defined(TAG)
#define TAG 
#endif

#define VALUE_64(value) \
        (unsigned int)(value >> (8*sizeof(uint32_t))), (unsigned int)value

#define TUI_LOGV(fmt, args...)					\
	do {								\
		if (tui_get_log_level() >= 3)					\
			drDbgPrintLnf("[DRTUI]"TAG fmt, ##args);		\
	} while (0)
	
#define TUI_LOGD(fmt, args...)					\
	do {								\
		if (tui_get_log_level() >= 2)					\
			drDbgPrintLnf("[DRTUI]"TAG fmt, ##args);		\
	} while (0)

#define TUI_LOGI(fmt, args...)					\
	do {								\
		if (tui_get_log_level() >= 1)					\
			drDbgPrintLnf("[DRTUI]"TAG fmt, ##args);		\
	} while (0)
	
#define TUI_LOGW(fmt, args...)					\
	do {								\
		drDbgPrintLnf("[DRTUI WARN]"TAG fmt, ##args);	\
	} while (0)

#define TUI_LOGE(fmt, args...)					\
	do {								\
		drDbgPrintLnf("[DRTUI ERROR]"TAG fmt, ##args);	\
	} while (0)

#define printf(fmt, args...)  drDbgPrintLnf("[DRTUI]"TAG fmt, ##args)