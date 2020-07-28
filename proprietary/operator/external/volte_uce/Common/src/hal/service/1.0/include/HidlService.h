#ifndef VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_SERVICE_H
#define VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_SERVICE_H


#include <sys/cdefs.h>
#include <stdbool.h>
__BEGIN_DECLS

int startHidlService();
void eventNotify(int8_t* buffer, int32_t request_id, int32_t length);

__END_DECLS

#endif //VENDOR_MEDIATEK_HARDWARE_PRESENCE_V1_0_SERVICE_H
