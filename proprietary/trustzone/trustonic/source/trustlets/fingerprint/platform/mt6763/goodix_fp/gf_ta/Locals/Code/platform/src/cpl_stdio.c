/*
 * Copyright (C) 2013-2016, Shenzhen Huiding Technology Co., Ltd.
 * All Rights Reserved.
 */
#include <cpl_stdio.h>
#include <cpl_string.h>
#include <string.h>
#include <stdarg.h>
#include "gf_type_define.h"

#define CHAR_TO_DECIMAL(x)  ((x) - 0x30)

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
    uint32_t dec_len = 0;
    int32_t tmp_len = 0;
    int32_t temp;
    int8_t ctemp;
    int8_t* stemp;
    int32_t i;

    va_start(arg_ptr, format);
    for (index = 0; index < format_length; ++index) {
        if (format[index] != '%') {
            (*buffer++) = format[index];
        } else {
            // only %02d -- %09d is valid
            if (format[index + 1] == '0'
                && format[index + 3] == 'd'
                && format[index + 2] > '1'
                && format[index + 2] <= '9') {
                temp = va_arg(arg_ptr, int32_t);
                dec_len = write_Int(buffer, temp);
                tmp_len = CHAR_TO_DECIMAL(format[index + 2]) - dec_len;
                if (tmp_len > 0) {
                    // prevent overlapping
                    for (i = dec_len; i > 0; i--) {
                        *(buffer + i - 1 + tmp_len) = *(buffer + i - 1);
                    }
                    // interpolated with zero
                    cpl_memset(buffer, '0', tmp_len);
                    buffer += tmp_len;
                }
                buffer += dec_len;
                index += 3;  // sizeof "02d"
            } else {
                switch (format[index + 1]) {
                    case 'd':
                        temp = va_arg(arg_ptr, int32_t);
                        buffer = buffer + write_Int(buffer, temp);
                        break;

                    case 's':
                        stemp = (int8_t*) va_arg(arg_ptr, int8_t*);
                        cpl_strcpy(buffer, stemp);
                        buffer += cpl_strlen(stemp);
                        break;

                    case 'c':
                        ctemp = va_arg(arg_ptr, int32_t);
                        *(buffer++) = ctemp;
                        break;
                    default:
                        break;
                }
                index++;
            }
        }
    }
    *buffer = 0;
    va_end(arg_ptr);

    return --index;
}

int32_t cpl_printf(const char *format, ...) {
#if 1
    /* warning,unused parameter */
    UNUSED_VAR(format);
#else
    va_list args;
    va_start(args, format);

    drDbgPrintLnf(format, args);

    va_end(args);
#endif

    return 0;
}
