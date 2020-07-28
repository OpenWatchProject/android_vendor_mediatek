/*
* Copyright (C) 2013-2018, Shenzhen Huiding Technology Co., Ltd.
* All Rights Reserved.
*/

#include <cpl_stdio.h>
#include <cpl_string.h>
#include <string.h>
#include <stdarg.h>

static uint32_t write_Int(int8_t *pBuf, int32_t value) {

    uint8_t stack[10];
    uint8_t length = 0;
    int8_t *_temp = pBuf;

    if (value < 0) {
        *(pBuf++) = '-';
        value *= -1;
    }

    do {
        stack[length] = '0' + (value % 10);
        value /= 10;
        length++;
    } while (value);

    while (length) {
        *pBuf = stack[length - 1];
        length--;
        pBuf++;
    }
    return pBuf - _temp;
}

int32_t cpl_sprintf(int8_t* buffer, const int8_t* format, ...) {

    va_list arg_ptr;
    uint32_t format_length = cpl_strlen(format);
    uint32_t index = 0;

    int32_t temp;
    int8_t ctemp;
    int8_t* stemp;

    va_start(arg_ptr, format);
    for (index = 0; index < format_length; ++index) {
        if (format[index] != '%') {
            (*buffer++) = format[index];
        } else {
            switch (format[index + 1]) {
                case 'd':
                    temp = va_arg(arg_ptr,int32_t);
                    buffer = buffer + write_Int(buffer, temp);
                    break;

                case 's':
                    stemp = (int8_t*) va_arg(arg_ptr,int8_t*);
                    cpl_strcpy(buffer, stemp);
                    buffer += cpl_strlen(stemp);
                    break;

                case 'c':
                    ctemp = va_arg(arg_ptr,int32_t);
                    *(buffer++) = ctemp;
                    break;
                default:
                    break;
            }
            index++;
        }
    }
    *buffer = 0;
    va_end(arg_ptr);

    return --index;
}

int32_t cpl_printf(const int8_t *format, ...) {
#if 1
    /* warning,unused parameter */
    format = format;
#else
    va_list args;
    va_start(args, format);

    drDbgPrintLnf(format, args);

    va_end(args);
#endif

    return 0;
}
