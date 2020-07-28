

#ifdef __cplusplus
extern "C" {
#endif


/*** STANDARD INCLUDES *******************************************************/
#include <stdlib.h>


/*** PROJECT INCLUDES ********************************************************/
#include "ports.h"
#include "mi_types.h"
#include "mi_util.h"

#include "powerd_cmd.h"
#include "powerd_core.h"
#include "power_ipc.h"


/*** MACROS ******************************************************************/


/*** GLOBAL VARIABLE DECLARATIONS (EXTERN) ***********************************/


/*** PRIVATE TYPES DEFINITIONS ***********************************************/


/*** PRIVATE VARIABLE DECLARATIONS (STATIC) **********************************/


/*** PRIVATE FUNCTION PROTOTYPES *********************************************/


/*** PUBLIC FUNCTION DEFINITIONS *********************************************/
int powerd_cmd_create(void ** ppCmd)
{
   tPS_CMD * vpCmd = NULL;

   *ppCmd = NULL;

   vpCmd = (tPS_CMD *) malloc(sizeof(tPS_CMD));

   if (vpCmd == NULL)
      return -1;

   powerd_ipc_init_pscmd(vpCmd);

   *ppCmd = vpCmd;

   return 0;
}

int powerd_cmd_destory(void * pCmd)
{
   tPS_CMD * vpCmd = (tPS_CMD *) pCmd;

   // to do, if support undefinded Config, need to travel all Config list and delete

   free(vpCmd);

   return 0;
}


#ifdef __cplusplus
}
#endif

