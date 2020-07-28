#ifndef __SCP_SEM_H
#define __SCP_SEM_H

#include <stdio.h>

#define SEMAPHORE     ADSP_SEMAPHORE
#define TIMEOUT 5000


int get_scp_semaphore(int flag);
int release_scp_semaphore(int flag);
int scp_get_awake_semaphore(int flag);

#endif /* __SCP_SEM_H */
