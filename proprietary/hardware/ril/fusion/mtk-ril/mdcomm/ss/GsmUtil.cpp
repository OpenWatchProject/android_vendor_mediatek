#include "GsmUtil.h"

#include <stdlib.h>
#include <string.h>

int
gsm_hexchar_to_int( char  c )
{
    if ((unsigned)(c - '0') < 10)
        return c - '0';
    if ((unsigned)(c - 'A') < 6)
        return 10 + (c - 'A');
    if ((unsigned)(c - 'a') < 6)
        return 10 + (c - 'a');
    return -1;
}

int
gsm_hex2_to_byte( const char*  hex )
{
    int  hi = gsm_hexchar_to_int(hex[0]);
    int  lo = gsm_hexchar_to_int(hex[1]);

    if (hi < 0 || lo < 0)
        return -1;

    return ( (hi << 4) | lo );
}

void
gsm_hex_from_byte( char*  hex, int val )
{
    static const char  hexdigits[] = "0123456789abcdef";

    hex[0] = hexdigits[(val >> 4) & 15];
    hex[1] = hexdigits[val & 15];
}

/** HEX
 **/

int
gsm_hex_to_bytes( cbytes_t  hex, int  hexlen, bytes_t  dst )
{
    int  nn;

    if (hexlen & 1)  /* must be even */
        return -1;

    for (nn = 0; nn < hexlen/2; nn++ ) {
        int  c = gsm_hex2_to_byte( (const char*)hex+2*nn );
        if (c < 0) return -1;
        dst[nn] = (byte_t) c;
    }
    return hexlen/2;
}

void
gsm_hex_from_bytes( char*  hex, cbytes_t  src, int  srclen )
{
    int  nn;

    for (nn = 0; nn < srclen; nn++) {
        gsm_hex_from_byte( hex + 2*nn, src[nn] );
    }
}

static __inline__ int
utf8_next( cbytes_t  *pp, cbytes_t  end )
{
    cbytes_t  p      = *pp;
    int       result = -1;

    if (p < end) {
        int  c= *p++;
        if (c >= 128) {
            if ((c & 0xe0) == 0xc0)
                c &= 0x1f;
            else if ((c & 0xf0) == 0xe0)
                c &= 0x0f;
            else
                c &= 0x07;

            while (p < end && (p[0] & 0xc0) == 0x80) {
                c = (c << 6) | (p[0] & 0x3f);
                p ++;
            }
        }
        result = c;
        *pp    = p;
    }
    return result;
}


__inline__ int
utf8_write( bytes_t  utf8, int  offset, int  v )
{
    int  result;

    if (v < 128) {
        result = 1;
        if (utf8)
            utf8[offset] = (byte_t) v;
    } else if (v < 0x800) {
        result = 2;
        if (utf8) {
            utf8[offset+0] = (byte_t)( 0xc0 | (v >> 6) );
            utf8[offset+1] = (byte_t)( 0x80 | (v & 0x3f) );
        }
    } else if (v < 0x10000) {
        result = 3;
        if (utf8) {
            utf8[offset+0] = (byte_t)( 0xe0 |  (v >> 12) );
            utf8[offset+1] = (byte_t)( 0x80 | ((v >> 6) & 0x3f) );
            utf8[offset+2] = (byte_t)( 0x80 |  (v & 0x3f) );
        }
    } else {
        result = 4;
        if (utf8) {
            utf8[offset+0] = (byte_t)( 0xf0 | ((v >> 18) & 0x7) );
            utf8[offset+1] = (byte_t)( 0x80 | ((v >> 12) & 0x3f) );
            utf8[offset+2] = (byte_t)( 0x80 | ((v >> 6) & 0x3f) );
            utf8[offset+3] = (byte_t)( 0x80 |  (v & 0x3f) );
        }
    }
    return  result;
}

static __inline__ int
ucs2_write( bytes_t  ucs2, int  offset, int  v )
{
    if (ucs2) {
        ucs2[offset+0] = (byte_t) (v >> 8);
        ucs2[offset+1] = (byte_t) (v);
    }
    return 2;
}

/** UCS2 to UTF8
 **/

/* convert a UCS2 string into a UTF8 byte string, assumes 'buf' is correctly sized */
int
ucs2_to_utf8( cbytes_t  ucs2,
              int       ucs2len,
              bytes_t   buf )
{
    int  nn;
    int  result = 0;

    for (nn = 0; nn < ucs2len; ucs2 += 2, nn++) {
        int  c= (ucs2[0] << 8) | ucs2[1];
        result += utf8_write(buf, result, c);
    }
    return result;
}

/* count the number of UCS2 chars contained in a utf8 byte string */
int
utf8_to_ucs2( cbytes_t  utf8,
              int       utf8len,
              bytes_t   ucs2 )
{
    cbytes_t  p      = utf8;
    cbytes_t  end    = p + utf8len;
    int       result = 0;

    while (p < end) {
        int  c = utf8_next(&p, end);

        if (c < 0)
            break;

        result += ucs2_write(ucs2, result, c);
    }
    return result/2;
}



/** GSM ALPHABET
 **/

#define  GSM_7BITS_ESCAPE   0x1b
#define  GSM_7BITS_UNKNOWN  0

static const unsigned short   gsm7bits_to_unicode[128] = {
  '@', 0xa3,  '$', 0xa5, 0xe8, 0xe9, 0xf9, 0xec, 0xf2, 0xc7, '\n', 0xd8, 0xf8, '\r', 0xc5, 0xe5,
0x394,  '_',0x3a6,0x393,0x39b,0x3a9,0x3a0,0x3a8,0x3a3,0x398,0x39e,    0, 0xc6, 0xe6, 0xdf, 0xc9,
  ' ',  '!',  '"',  '#', 0xa4,  '%',  '&', '\'',  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/',
  '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?',
 0xa1,  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O',
  'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z', 0xc4, 0xd6,0x147, 0xdc, 0xa7,
 0xbf,  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o',
  'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z', 0xe4, 0xf6, 0xf1, 0xfc, 0xe0,
};

static const unsigned short  gsm7bits_extend_to_unicode[128] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,'\f',   0,   0,   0,   0,   0,
    0,   0,   0,   0, '^',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0, '{', '}',   0,   0,   0,   0,   0,'\\',
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0, '[', '~', ']',   0,
  '|',   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,0x20ac, 0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
};

int
utf8_from_unpackedgsm7( cbytes_t  src,
                int       septet_offset,
                int       septet_count,
                bytes_t   utf8 )
{
    (void)(septet_offset);
    int  escaped = 0;
    int  result  = 0;

    for ( ; septet_count > 0; septet_count-- )
    {
        int  c = src[0] & 0x7f;
        int  v;


        if (escaped) {
            v = gsm7bits_extend_to_unicode[c];
            /* Solve [ALPS00271799][SW.GIN2_SINGLE]Missing var for USSD strings receipt for Movistar Peru, mtk04070, 20120424 */
            escaped = 0;
        } else if (c == GSM_7BITS_ESCAPE) {
            escaped = 1;
            goto NextSeptet;
        } else {
            v = gsm7bits_to_unicode[c];
        }

        result += utf8_write( utf8, result, v );

    NextSeptet:
        src   += 1;

    }
    return  result;
}

int
utf8_from_gsm8( cbytes_t  src, int  count, bytes_t  utf8 )
{
    int  result  = 0;
    int  escaped = 0;


    for ( ; count > 0; count-- )
    {
        int  c = *src++;

        if (c == 0xff)
            break;

        if (c == GSM_7BITS_ESCAPE) {
            if (escaped) { /* two escape characters => one space */
                c = 0x20;
                escaped = 0;
            } else {
                escaped = 1;
                continue;
            }
        }
        else
        {
            if (c >= 0x80) {
                c       = 0x20;
                escaped = 0;
            } else if (escaped) {
                c = gsm7bits_extend_to_unicode[c];
            } else
                c = gsm7bits_to_unicode[c];
        }

        result += utf8_write( utf8, result, c );
    }
    return  result;
}

/* Convert "00 00" to "00 20" */
int zero4_to_space(bytes_t  ucs2, int ucs2len) {
    int i, count = 0;

    /* Ignore the last character */
    for (i = 0; i < (ucs2len - 2); i+=2) {
        if ((ucs2[i] == 0) && (ucs2[i+1] == 0)) {
            ucs2[i+1] = 0x20; /* Space character */
            count++;
        }
    }
    return count;
}
