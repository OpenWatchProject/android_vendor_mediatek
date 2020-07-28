/*
 * Copyright (c) 2013 TRUSTONIC LIMITED
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
/*
The content of this file is copied from b64.c (http://base64.sourceforge.net)
and modified to work with memory buffers instead of files.

The linebreak addition has been removed from the encoding part

COPYRIGHT AND LICENCE from the original code, applies only to this file:
                Copyright (c) 2001 Bob Trower, Trantor Standard Systems Inc.

                Permission is hereby granted, free of charge, to any person
                obtaining a copy of this software and associated
                documentation files (the "Software"), to deal in the
                Software without restriction, including without limitation
                the rights to use, copy, modify, merge, publish, distribute,
                sublicense, and/or sell copies of the Software, and to
                permit persons to whom the Software is furnished to do so,
                subject to the following conditions:

                The above copyright notice and this permission notice shall
                be included in all copies or substantial portions of the
                Software.

                THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
                KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
                WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
                PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
                OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
                OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
                OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
                SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <string.h>
#include "logging.h"
#include "base64.h"

static const char* cb64="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char* cd64="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";


static void decodeblock( unsigned char *in, unsigned char *out )
{
    out[ 0 ] = (unsigned char ) (in[0] << 2 | in[1] >> 4);
    out[ 1 ] = (unsigned char ) (in[1] << 4 | in[2] >> 2);
    out[ 2 ] = (unsigned char ) (((in[2] << 6) & 0xc0) | in[3]);
}

#define ENCODEDSIZE 4
#define PLAINSIZE 3

#define ASCIIPLUS 43
#define ASCIIz 122

#define FIRSTB64ASCII ASCIIPLUS
#define LASTB64ASCII ASCIIz

#define LINESIZE 74

/**
Decode base64 encoded NULL terminated string. If the string is not NULL terminated the behaviour is undetermined.

@param toBeDecoded the string to be decoded
@param resultP pointer to the pointer to the buffer where the decoded data is. The caller has to free the buffer when not needed.
@return size of the decoded string
*/
size_t base64DecodeStringRemoveEndZero(const char* toBeDecoded, char** resultP)
{
    size_t inSize;
    size_t outSize;

    unsigned char in[ENCODEDSIZE];
    unsigned char out[PLAINSIZE];
    int v;
    int i, len;
    size_t inIndex=0;
    int outIndex=0;

    LOGD(">> base64DecodeStringRemoveEndZero");
    if(NULL==toBeDecoded) return 0;

    inSize=strlen(toBeDecoded);
    outSize=((inSize*PLAINSIZE)/ENCODEDSIZE)+((inSize*PLAINSIZE)%ENCODEDSIZE);
    *resultP=(char *)malloc(outSize);

    if((*resultP)==NULL) return 0;

    LOGD("in %d out %d", (int) inSize, (int) outSize);

	*in = (unsigned char) 0;
	*out = (unsigned char) 0;


    while( inIndex < inSize )
    {
        for( len = 0, i = 0; i < ENCODEDSIZE && inIndex < inSize; i++ )
        {
            v = 0;
            // skip characters that do not belong to decoded base64 and set v
            while( inIndex < inSize && 0 == v )
            {
                v = toBeDecoded[ inIndex++ ];

                v = ((v < FIRSTB64ASCII || v > LASTB64ASCII) ? 0 : (int) cd64[ v - FIRSTB64ASCII ]);
	     		if( v != 0 )
                {
                    v = ((v == (int)'$') ? 0 : v - 61);
                }
            }

            // set the character to in buffer, but only if it is not 0 (last character in toBeDecoded illegal)
            if( inIndex <= inSize )
            {
                if( v != 0 )
                {
                    len++;
                    in[ i ] = (unsigned char) (v - 1);
                }
            }
            else
            {
                in[i] = (unsigned char) 0;
            }
        }

        if( len > 0 )
        {
            decodeblock( in, out );
            for( i = 0; i < (len - 1); i++ )
            {
                (*resultP)[outIndex++]=out[i];
            }
        }
    }

    LOGD("<< base64DecodeStringRemoveEndZero inIndex %d outIndex %d allocatedBuffer %d", (int) inIndex, (int) outIndex, (int) outSize);
    return( outIndex );
}

static void encodeblock( unsigned char *in, unsigned char *out, int len )
{
    out[0] = (unsigned char) cb64[ (int)(in[0] >> 2) ];
    out[1] = (unsigned char) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
    out[2] = (unsigned char) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
    out[3] = (unsigned char) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
}

/**
base64encode data to a NULL terminated string.

@param toBeEncoded the buffer to be encoded
@param length length of the buffer to be encoded
@return pointer to NULL terminated base64encoded string. The caller has to free the buffer pointer by the pointer.
*/
char* base64EncodeAddEndZero(const char* toBeEncoded, size_t length)
{
    size_t outSize;
    char* resultP;

    unsigned char in[PLAINSIZE];
	unsigned char out[ENCODEDSIZE];
    int i, len;
    size_t inIndex=0;
    int outIndex=0;

    LOGD(">> base64EncodeAddEndZero %d %s", (int) length, ((toBeEncoded!=NULL)?"ptr ok":"NULL"));
    if(NULL==toBeEncoded) return NULL;

    outSize=(length/PLAINSIZE + ((length%PLAINSIZE>0)?1:0))*ENCODEDSIZE+1;

//    outSize+=(outsize/LINESIZE)*2; // crlf after each full line

    resultP=(char *) malloc(outSize);

    if(resultP==NULL) return NULL;
    resultP[outSize-1]=0;

    LOGD("in %d out %d", (int) length, (int) outSize);

	*in = (unsigned char) 0;
	*out = (unsigned char) 0;

    while( inIndex < length )
    {
        len = 0;
        for( i = 0; i < PLAINSIZE; i++ )
        {
            if( inIndex < length )
            {
                in[i] = toBeEncoded[inIndex];
                len++;
            }
            else
            {
                in[i] = (unsigned char) 0;
            }
            inIndex++;
        }

        if( len > 0 )
        {
            encodeblock( in, out, len );
            for( i = 0; i < ENCODEDSIZE; i++ )
            {
                resultP[outIndex++]=out[i];
            }
        // could be adding line breaks here but then we need to calculate also some space for them

        }
    }
    resultP[outIndex]=0;
    LOGD("<< base64EncodeAddEndZero %d <= (%d - 1)", (int) outIndex, (int) outSize);
    return resultP;
}
