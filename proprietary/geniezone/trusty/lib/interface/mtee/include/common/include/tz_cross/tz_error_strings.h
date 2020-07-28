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
 * Helper to implement TZ_GetErrorString in MTEE/KREE/UREE
 * Please don't include this.
 */

#ifndef __TZ_ERROR_STRINGS__
#define __TZ_ERROR_STRINGS__

static const char *TZ_ErrorStrings[] = {
    "The operation was successful.",
    "Non-specific cause.",
    "Access privileges are not sufficient.",
    "The operation was cancelled.",
    "Concurrent accesses caused conflict.",
    "Too much data for the requested operation was passed.",
    "Input data was of invalid format.",
    "Input parameters were invalid.",
    "Operation is not valid in the current state.",
    "The requested data item is not found.",
    "The requested operation should exist but is not yet implemented.",
    "The requested operation is valid but is not supported in this Implementation.",
    "Expected data was missing.",
    "System ran out of resources.",
    "The system is busy working on something else.",
    "Communication with a remote party failed.",
    "A security fault was detected.",
    "The supplied buffer is too short for the generated output.",
    "The handle is invalid.",
    "Unknown error.",
};
#define TZ_ErrorStrings_num  (sizeof(TZ_ErrorStrings)/sizeof(TZ_ErrorStrings[0]))


static const char *_TZ_GetErrorString(TZ_RESULT res)
{
    unsigned int num;

    if (res == 0)
        return TZ_ErrorStrings[0];

    num = ((unsigned int)res & 0xffff) + 1;
    if (num > (TZ_ErrorStrings_num-1))
        num = TZ_ErrorStrings_num-1;
    return TZ_ErrorStrings[num];
}

#endif /* __TZ_ERROR_STRINGS__ */
