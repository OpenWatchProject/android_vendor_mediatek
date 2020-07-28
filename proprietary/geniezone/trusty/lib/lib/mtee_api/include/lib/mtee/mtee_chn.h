#include <tz_cross/trustzone.h>
#include <tz_private/system.h>

TZ_RESULT MTEE_GetSessionChannel(MTEE_SESSION_HANDLE session, int *channel_p);
TZ_RESULT MTEE_SetSessionChannel(MTEE_SESSION_HANDLE session, int channel);

