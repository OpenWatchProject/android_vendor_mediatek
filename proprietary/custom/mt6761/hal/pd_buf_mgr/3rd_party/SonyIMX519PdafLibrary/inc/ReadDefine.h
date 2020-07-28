/**************************************************************
Copyright 2018 Sony Semiconductor Solutions Corporation

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, 
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors 
may be used to endorse or promote products derived from this software without 
specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS 
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; 
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 **************************************************************/

#ifndef __READ_DEFINE_H__
#define __READ_DEFINE_H__

#include <stdio.h>

#define DEFINE_FILE_NAME        "Sensor.def"

#define DEFINE_OK               (0)
#define DEFINE_ERROR_FILE       (-1)
#define DEFINE_ERROR_ID         (-2)
#define DEFINE_ERROR_SIZE       (-3)
#define DEFINE_ERROR_TYPE       (-4)
#define DEFINE_ERROR_OFFSET     (-5)
#define DEFINE_ERROR_DATA       (-6)

typedef struct tagDefine_STRING {
    unsigned char               str[255];
} Define_STRING_t;

extern int GetDefineConfig(FILE *fp, unsigned short *config);
extern int GetDefineData_S_CHAR(FILE *fp, unsigned short id, signed char *data);
extern int GetDefineData_U_CHAR(FILE *fp, unsigned short id, unsigned char *data);
extern int GetDefineData_S_SHORT(FILE *fp, unsigned short id, signed short *data);
extern int GetDefineData_U_SHORT(FILE *fp, unsigned short id, unsigned short *data);
extern int GetDefineData_S_INT(FILE *fp, unsigned short id, signed int *data);
extern int GetDefineData_U_INT(FILE *fp, unsigned short id, unsigned int *data);
extern int GetDefineData_S_LONG(FILE *fp, unsigned short id, signed long *data);
extern int GetDefineData_U_LONG(FILE *fp, unsigned short id, unsigned long *data);
extern int GetDefineData_STRING(FILE *fp, unsigned short id, Define_STRING_t *data);

#endif /* __READ_DEFINE_H__ */
