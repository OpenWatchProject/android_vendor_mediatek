/*
* Copyright (c) 2015 MediaTek Inc.
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
    DCM TA implementation.
*/

#ifndef __TRUSTZONE_TA_DCM__
#define __TRUSTZONE_TA_DCM__

#define TZ_TA_DCM_UUID   "b3c1d950-f446-11e2-b778-0800200c9a66"

/* Command for DCM TA */
#define TZCMD_DCM_ENABLE_DCM        0
#define TZCMD_DCM_DISABLE_DCM       1
#define TZCMD_DCM_GET_DCM_STATUS    2
#define TZCMD_DCM_GET_DCM_OP_STATUS 3

/* Usage */
/*
    TZCMD_DCM_ENABLE_DCM
        Input:
            param[0].value.a = Type (Type = SMI_DCM)

    TZCMD_DCM_DISABLE_DCM
        Input:
            param[0].value.a = Type (Type = SMI_DCM)
      
    TZCMD_DCM_GET_DCM_STATUS
        Input:
            param[0].value.a = Type (Type = SMI_DCM)

        Output:
            Type = SMI_DCM
                param[1].value.a = register value for SMI_COMMON_DCM
                param[1].value.b = register value for SMI_SECURE_DCMCON
                param[2].value.a = register value for M4U_DCM
    TZCMD_DCM_GET_DCM_OP_STATUS
        Input:
            param[0].value.a = Type (Type = SMI_DCM)

        Output:
            Type = SMI_DCM
                param[1].value.a = register value for SMI_SECURE_DCMSET
                param[1].value.b = register value for SMI_SECURE_DCMCLR
*/

#endif /* __TRUSTZONE_TA_DCM__ */
