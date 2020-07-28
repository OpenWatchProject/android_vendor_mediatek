/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef __COMMON_VOIP_STRING_H__
#define __COMMON_VOIP_STRING_H__

#include <string.h>

#define voip_strcpy(s, d)   strcpy((s), (d)) ///< string copy

/**
 * @brief string length
 * @param str [IN] the string to be accumulated.
 * @return string length
 */
int             voip_strlen(char *str);

/**
 * @brief string copy with length
 * @param dest [IN/OUT] the destination string pointer
 * @param src [IN] the source string pointer
 * @param len [IN] the length of source string to be copied
 * @return 0 if success
 */
int             voip_strncpy(char *dest, const char *src, int len);

/**
 * @brief string duplicate
 * @param str [IN] the source string pointer
 * @param file [IN] the file number that call this function
 * @param line [IN] the line number that call this function
 * @retval 0 if failed
 * @retval string pointer if success
 */
char *          VOIP_strdup(const char *str, char *file, int line);
#define voip_strdup(s)  VOIP_strdup((s), __FILE__, __LINE__)

/**
 * @brief compare strings while ignoring differences in case
 * @param s1 [IN] the source string 1
 * @param s2 [IN] the source string 2
 * @retval 1 if s1 is almost like s2
 * @retval 0 if s1 is s2
 * @retval (-1) if s1 is not s2 at all
 */
 signed short    voip_strcasecmp(const char *s1, const char *s2);

/**
 * @brief Get the next token from source string where tokens are strings separated from delim string.
 * @param str [IN] the source string pointer
 * @param delim [IN] the token string to separate the source string
 * @param ptrptr [OUT] the string pointer after the separation
 * @retval 0 if failed
 * @retval string pointer that find
 */
char *          voip_strtok_r(char *str, const char *delim, char **ptrptr);

/**
 * @brief Transfer source string to be capital latter
 * @param s [IN] source string
 * @return string pointer that has been transformed
 */
char *          voip_char_ucase(char *s);

/**
 * @brief Transfer source string to be lowercase latter
 * @param s [IN] source string
 * @return string pointer that has been transformed
 */
char *          voip_char_lcase(char *s);

/**
 * @brief Remove \t from the source string
 * @param str [IN] source string
 * @return string pointer that has been transformed
 */
char *          voip_triml(char *str);

/**
 * @brief Remove space from the source string
 * @param str [IN] source string
 * @return string pointer that has been transformed
 */
char *          voip_trimr(char *str);

/**
 * @brief Remove space and trim from the source string
 * @param str [IN] source string
 * @return string pointer that has been transformed
 */
char *          voip_trim(char *str);

/**
 * @brief this function is a printf function with length limitation
 * @param buffer [IN/OUT] destination buffer
 * @param len [IN] length of the buffer
 * @param fmt [IN] format to be written into the buffer
 * @retval len the length that is used in the buffer
 * @retval (-1) if the length of the buffer is not enough to use
 */
int             voip_snprintf(char *buffer, int len, const char* fmt, ...);

/**
 * @brief this function is a gets a new string which is defined by format and parameters
 * @param fmt [IN] format to be written into the buffer
 * @retval pointer to allocated c-string or NULL if string could not be made
*/
char *
voip_alloc_sprintf(const char* fmt, ...);

int             voip_parser_address_str(char *addr_str, char **addr, int *port, int mode);

#define VOIP_FIXED_BUFFER_STRCPY(target_buffer, source_string) {\
    (void)voip_strncpy((char *)(target_buffer), (const char *)(source_string), sizeof(target_buffer)); \
}
#endif /* __COMMON_VOIP_STRING_H__ */
