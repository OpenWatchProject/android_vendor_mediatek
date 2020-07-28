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

/**************************************************************/
/*        include                                             */
/**************************************************************/
#include <string.h>
#include <stdint.h>

/******************************/
#include "ReadDefine.h"

/**************************************************************/
/*        define                                              */
/**************************************************************/
#define HEADER_FIELD_SIZE       (8)
#define LIST_DATA_SIZE          (8)

#define CONFIG_SIZE             (2)
#define CONFIG_OFFSET           (0x04)
#define LIST_NUM_SIZE           (2)
#define LIST_NUM_OFFSET         (0x06)

#define TYPE_UNSIGNED           (1)
#define TYPE_SIGNED             (2)
#define TYPE_STRING             (3)

/**************************************************************/
/*        macro                                               */
/**************************************************************/

/******************************/
#ifdef __GNUC__

    #define FREAD(a, b, c, d)           fread((a), (b), (c), (d))

#else   /* __GNUC__ */

    #define FREAD(a, b, c, d)           fread_s((a), ((b) * (c)), (b), (c), (d))

#endif  /* __GNUC__ */

/**************************************************************/
/*        struct                                              */
/**************************************************************/

/******************************/
typedef struct tagListData {
    uint16_t id;
    uint8_t  size;
    uint8_t  type;
    uint32_t offset;
} ListData_t;

/**************************************************************/
/*        prototype                                           */
/**************************************************************/
static int GetListData(FILE *fp, ListData_t *list);
static int GetDefineDataCommon(FILE *fp, uint16_t find_id, uint8_t data_type, size_t *size, uint8_t* def_data);
static int GetData(FILE *fp, uint32_t offset, size_t size, uint8_t* data);
static int8_t   Convert_S08(uint8_t d);
static uint8_t  Convert_U08(uint8_t d);
static int16_t  Convert_S16(uint8_t d0, uint8_t d1);
static uint16_t Convert_U16(uint8_t d0, uint8_t d1);
static int32_t  Convert_S32(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
static uint32_t Convert_U32(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);

/**************************************************************/
/*        function : GetDefineConfig                          */
/**************************************************************/
extern int GetDefineConfig(FILE *fp, unsigned short *config)
{
    int      ret;
    uint8_t  data[CONFIG_SIZE] = {0};

    ret = GetData(fp, CONFIG_OFFSET, CONFIG_SIZE, data);

    if (ret == DEFINE_OK) {
        *config = (unsigned short)Convert_U16(data[0], data[1]);
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_S_CHAR                     */
/**************************************************************/
extern int GetDefineData_S_CHAR(FILE *fp, unsigned short id, signed char *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(signed char);

    ret = GetDefineDataCommon(fp, id, TYPE_SIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        *data = (signed char)Convert_S08(def_data[0]);
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_U_CHAR                     */
/**************************************************************/
extern int GetDefineData_U_CHAR(FILE *fp, unsigned short id, unsigned char *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(unsigned char);

    ret = GetDefineDataCommon(fp, id, TYPE_UNSIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        *data = (unsigned char)Convert_U08(def_data[0]);
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_S_SHORT                    */
/**************************************************************/
extern int GetDefineData_S_SHORT(FILE *fp, unsigned short id, signed short *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(signed short);

    ret = GetDefineDataCommon(fp, id, TYPE_SIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        switch (size) {
        case 2:
            *data = (signed short)Convert_S16(def_data[0], def_data[1]);
            break;

        default:
            *data = (signed short)Convert_S08(def_data[0]);
            break;
        }
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_U_SHORT                    */
/**************************************************************/
extern int GetDefineData_U_SHORT(FILE *fp, unsigned short id, unsigned short *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(unsigned short);

    ret = GetDefineDataCommon(fp, id, TYPE_UNSIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        switch (size) {
        case 2:
            *data = (unsigned short)Convert_U16(def_data[0], def_data[1]);
            break;

        default:
            *data = (unsigned short)Convert_U08(def_data[0]);
            break;
        }
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_S_INT                    */
/**************************************************************/
extern int GetDefineData_S_INT(FILE *fp, unsigned short id, signed int *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(signed int);

    ret = GetDefineDataCommon(fp, id, TYPE_SIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        switch (size) {
        case 2:
            *data = (signed int)Convert_S16(def_data[0], def_data[1]);
            break;

        case 4:
            *data = (signed int)Convert_S32(def_data[0], def_data[1], def_data[2], def_data[3]);
            break;

        default:
            *data = (signed int)Convert_S08(def_data[0]);
            break;
        }
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_U_INT                    */
/**************************************************************/
extern int GetDefineData_U_INT(FILE *fp, unsigned short id, unsigned int *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(unsigned int);

    ret = GetDefineDataCommon(fp, id, TYPE_UNSIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        switch (size) {
        case 2:
            *data = (unsigned int)Convert_U16(def_data[0], def_data[1]);
            break;

        case 4:
            *data = (unsigned int)Convert_U32(def_data[0], def_data[1], def_data[2], def_data[3]);
            break;

        default:
            *data = (unsigned int)Convert_U08(def_data[0]);
            break;
        }
    }

    return ret;
}
/**************************************************************/
/*        function : GetDefineData_S_LONG                    */
/**************************************************************/
extern int GetDefineData_S_LONG(FILE *fp, unsigned short id, signed long *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(signed long);

    ret = GetDefineDataCommon(fp, id, TYPE_SIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        switch (size) {
        case 2:
            *data = (signed long)Convert_S16(def_data[0], def_data[1]);
            break;

        case 4:
            *data = (signed long)Convert_S32(def_data[0], def_data[1], def_data[2], def_data[3]);
            break;

        default:
            *data = (signed long)Convert_S08(def_data[0]);
            break;
        }
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_U_LONG                    */
/**************************************************************/
extern int GetDefineData_U_LONG(FILE *fp, unsigned short id, unsigned long *data)
{
    int     ret;
    uint8_t def_data[4] = {0};
    size_t  size = sizeof(signed int);

    ret = GetDefineDataCommon(fp, id, TYPE_UNSIGNED, &size, def_data);

    if (ret == DEFINE_OK) {
        switch (size) {
        case 2:
            *data = (unsigned long)Convert_U16(def_data[0], def_data[1]);
            break;

        case 4:
            *data = (unsigned long)Convert_U32(def_data[0], def_data[1], def_data[2], def_data[3]);
            break;

        default:
            *data = (unsigned long)Convert_U08(def_data[0]);
            break;
        }
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineData_STRING                     */
/**************************************************************/
extern int GetDefineData_STRING(FILE *fp, unsigned short id, Define_STRING_t *data)
{
    int    ret;
    size_t size = sizeof(data->str);

    ret = GetDefineDataCommon(fp, id, TYPE_STRING, &size, data->str);

    return ret;
}

/**************************************************************/
/*        function : GetListData                              */
/**************************************************************/
static int GetListData(FILE *fp, ListData_t *list)
{
    int      ret;
    uint16_t list_num;
    uint8_t  data[LIST_DATA_SIZE] = {0};
    uint16_t i;

    ret = GetData(fp, LIST_NUM_OFFSET, LIST_NUM_SIZE, data);

    if (ret == DEFINE_OK) {
        list_num = Convert_U16(data[0], data[1]);

        for (i = 0; i < list_num; i++) {

            ret = GetData(fp, (HEADER_FIELD_SIZE + (i * LIST_DATA_SIZE)), LIST_DATA_SIZE, data);

            if (ret != DEFINE_OK) {
                break;
            }

            if (list->id == Convert_U16(data[0], data[1])) {
                list->size   = Convert_U08(data[2]);
                list->type   = Convert_U08(data[3]);
                list->offset = Convert_U32(data[4], data[5], data[6], data[7]);

                if (list->offset < (HEADER_FIELD_SIZE + ((uint32_t)list_num * LIST_DATA_SIZE))) {
                    ret = DEFINE_ERROR_OFFSET;
                }
                break;
            }
        }

        if (i >= list_num) {
            ret = DEFINE_ERROR_ID;
        }
    }

    return ret;
}

/**************************************************************/
/*        function : GetDefineDataCommon                      */
/**************************************************************/
static int GetDefineDataCommon(FILE *fp, uint16_t find_id, uint8_t data_type, size_t *size, uint8_t* def_data)
{
    int        ret;
    ListData_t list = {0};

    list.id = find_id;

    ret = GetListData(fp, &list);

    if (ret == DEFINE_OK) {
        if ((list.size > *size) || (list.size == 0) ||
            ((list.type != TYPE_STRING) && ((list.size != 1) && (list.size != 2) && (list.size != 4)))) {

            ret = DEFINE_ERROR_SIZE;
        } else if (list.type != data_type) {
            ret = DEFINE_ERROR_TYPE;
        }
    }

    if (ret == DEFINE_OK) {
        ret = GetData(fp, list.offset, list.size, def_data);
        *size = (size_t)list.size;
    }

    return ret;
}

/**************************************************************/
/*        function : GetData                                  */
/**************************************************************/
static int GetData(FILE *fp, uint32_t offset, size_t size, uint8_t* data)
{
    int ret = DEFINE_OK;

    if (fseek(fp, (long)offset, SEEK_SET) != 0) {
        ret = DEFINE_ERROR_DATA;
    } else {
        if (FREAD(data, sizeof(uint8_t), size, fp) != size) {
            ret = DEFINE_ERROR_DATA;
        }
    }

    return ret;
}

/**************************************************************/
/*        function : Convert_S08                              */
/**************************************************************/
static int8_t Convert_S08(uint8_t d)
{
    return (int8_t)d;
}

/**************************************************************/
/*        function : Convert_U08                              */
/**************************************************************/
static uint8_t Convert_U08(uint8_t d)
{
    return (uint8_t)d;
}

/**************************************************************/
/*        function : Convert_S16                              */
/**************************************************************/
static int16_t Convert_S16(uint8_t d0, uint8_t d1)
{
    return (int16_t)Convert_U16(d0, d1);
}

/**************************************************************/
/*        function : Convert_U16                              */
/**************************************************************/
static uint16_t Convert_U16(uint8_t d0, uint8_t d1)
{
    return (uint16_t)((((uint16_t)d1) << 8) | ((uint16_t)d0));
}

/**************************************************************/
/*        function : Convert_S32                              */
/**************************************************************/
static int32_t Convert_S32(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
    return (int32_t)Convert_U32(d0, d1, d2, d3);
}

/**************************************************************/
/*        function : Convert_U32                              */
/**************************************************************/
static uint32_t Convert_U32(uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
    return ((((uint32_t)d3) << 24) |
            (((uint32_t)d2) << 16) |
            (((uint32_t)d1) <<  8) |
             ((uint32_t)d0));
}
