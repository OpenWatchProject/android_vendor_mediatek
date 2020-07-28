/*
* Copyright (c) 2014 - 2016 MediaTek Inc.
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files
* (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/


/*
 * @file   rpmb_main.c
 * @brief  Implements the entry point of the driver.
 *
 */
#include <drv_fwk.h>
#include <drv_fwk_macro.h>
#include <drv_error.h>
#include <drv_defs.h>

#include "drRpmbOps.h"
/* Driver version (used by the IPC handler) */
#define DRIVER_VERSION       2
#define RPMB_DRV_ID DRIVER_ID

extern int rpmb_invoke_command(unsigned int sid, unsigned int cmd, unsigned long args);
/* extern int rpmb_ree_command(unsigned int cmd, unsigned long args); */
extern int rpmb_open_session(unsigned int sid, unsigned long args);
extern int rpmb_close_session(unsigned int sid);

/*
 * Initialization code
 */
int rpmb_init(void)
{
    int ret = DRV_FWK_API_OK;
    /*
     * TODO: This is the function where certain initialization
     * can be done before proceeding further. Such as HW related
     * initialization. Update the return code accordingly
     */
    ret = drRpmbInit();
    return ret;
}
/*
 * Main routine for the example driver.
 * Initializes the Api data structures and starts the required threads.
 */
DECLARE_DRIVER_MODULE("tdrv_rpmb", RPMB_DRV_ID, rpmb_init, rpmb_invoke_command, NULL/*rpmb_ree_command*/, rpmb_open_session, rpmb_close_session);



