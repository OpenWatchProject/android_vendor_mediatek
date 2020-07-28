/*
 * Copyright (c) 2016-2017 TRUSTONIC LIMITED
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the TRUSTONIC LIMITED nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "cust_tee_keymaster_utils.h"
#include "km_util.h"

/*
 *
 */
long getFileContent(const char* pPath, uint8_t** ppContent)
{
    FILE*   pStream;
    long    filesize;
    uint8_t* content = NULL;

   /* Open the file */
   pStream = fopen(pPath, "rb");
   if (pStream == NULL)
   {
      LOG_E("Error: Cannot open file: %s.", pPath);
      goto error;
   }

   if (fseek(pStream, 0L, SEEK_END) != 0)
   {
      LOG_E("Error: Cannot read file: %s.", pPath);
      goto error;
   }

   filesize = ftell(pStream);
   if (filesize < 0)
   {
      LOG_E("Error: Cannot get the file size: %s.", pPath);
      goto error;
   }

   if (filesize == 0)
   {
      LOG_E("Error: Empty file: %s.", pPath);
      goto error;
   }

   /* Set the file pointer at the beginning of the file */
   if (fseek(pStream, 0L, SEEK_SET) != 0)
   {
      LOG_E("Error: Cannot read file: %s.", pPath);
      goto error;
   }

   /* Allocate a buffer for the content */
   content = (uint8_t*)malloc(filesize);
   if (content == NULL)
   {
      LOG_E("Error: Cannot read file: Out of memory.");
      goto error;
   }

   /* Read data from the file into the buffer */
   if (fread(content, (size_t)filesize, 1, pStream) != 1)
   {
      LOG_E("Error: Cannot read file: %s.", pPath);
      goto error;
   }

   /* Close the file */
   fclose(pStream);
   *ppContent = content;

   /* Return number of bytes read */

   return filesize;

error:
   if (content  != NULL)
   {
       free(content);
   }
   if (pStream != NULL)
   {
       fclose(pStream);
   }
   return 0;
}

