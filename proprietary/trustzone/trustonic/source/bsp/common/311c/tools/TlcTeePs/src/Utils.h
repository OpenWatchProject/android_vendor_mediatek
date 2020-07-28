/*
 * Copyright (c) 2015-2016 TRUSTONIC LIMITED
 * All rights reserved
 *
 * The present software is the confidential and proprietary information of
 * TRUSTONIC LIMITED. You shall not disclose the present software and shall
 * use it only in accordance with the terms of the license agreement you
 * entered into with TRUSTONIC LIMITED. This software may be subject to
 * export or import laws in certain countries.
 */
#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <stdint.h>

// Couple of macro for output styling
// https://wiki.archlinux.org/index.php/Color_Bash_Prompt
#define BLACK(x)    "\e[30m" x "\e[0m"
#define WHITE(x)    "\e[37m" x "\e[0m"
#define BG_WHITE(x) "\e[47m" x "\e[0m"
#define BG_RED(x)   "\e[41m" x "\e[0m"
#define BOLD(x)     "\e[1m" x "\e[0m"
#define HIGHLIGHT(x) BG_WHITE(BLACK(BOLD(x)))
#define HIGHLIGHT_ERROR(x) BG_RED(WHITE(BOLD(x)))

//  Get error messages on the stderr output
#define IP_LOG_E(...) \
    do { \
        LOG_E(__VA_ARGS__); \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n"); \
    } while(0);


#define DATE_FORMAT "%Y-%m-%d @ %X"
#define DATE_BUFFER_SIZE 22 // according to above format

void clear_terminal();

void getCurrentDateTime(char* dst);

int showFileContent(const char* file_path);

#endif //(UTILS_H_)
