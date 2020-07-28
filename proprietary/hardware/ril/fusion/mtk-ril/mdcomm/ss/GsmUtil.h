#ifndef GSM_UTIL
#define GSM_UTIL

/** USEFUL TYPES
 **/

typedef unsigned char  byte_t;
typedef byte_t*        bytes_t;
typedef const byte_t*  cbytes_t;

#ifndef MIN
    #define MIN(x, y)   ((x) <= (y))? (x): (y)
#endif

/** HEX
 **/

/* try to convert a hex string into a byte string, assumes 'dst' is properly sized, and hexlen is even.
 * returns the number of bytes on exit, or -1 in case of badly formatted data */
int gsm_hex_to_bytes  ( cbytes_t  hex, int  hexlen, bytes_t  dst );

/* convert a byte string into a hex string, assumes 'hex' is properly sized */
void gsm_hex_from_bytes( char*  hex, cbytes_t  src, int  srclen );

/* convert a hexchar to an int, returns -1 on error */
int gsm_hexchar_to_int( char  c );

/* convert a 2-char hex value into an int, returns -1 on error */
int gsm_hex2_to_byte( const char*  hex );

/* write a byte to a 2-byte hex string */
void gsm_hex_from_byte( char*  hex, int  val );

/** UTF-8 and GSM Alphabet
 **/

/* convert an unpacked GSM septets string into a utf-8 byte string. assumes that 'utf8' is NULL or properly
   sized. 'offset' is the starting bit offset in 'src', 'count' is the number of input septets.
   return the number of utf8 bytes. */
int utf8_from_unpackedgsm7( cbytes_t  src, int  offset, int  count, bytes_t  utf8 );

/* convert an unpacked 8-bit GSM septets string into a utf-8 byte string. assumes that 'utf8'
   is NULL or properly sized. 'count' is the number of input bytes.
   returns the number of utf8 bytes */
int utf8_from_gsm8( cbytes_t  src, int  count, bytes_t  utf8 );

/** UCS2 to/from UTF8
 **/

/* convert a ucs2 string into a utf8 byte string, assumes 'utf8' NULL or properly sized.
   returns the number of utf8 bytes*/
int ucs2_to_utf8( cbytes_t  ucs2, int  ucs2len, bytes_t  utf8 );

/* convert a utf8 byte string into a ucs2 string, assumes 'ucs2' NULL or properly sized.
   returns the number of ucs2 chars */
extern int utf8_to_ucs2( cbytes_t  utf8, int  utf8len, bytes_t  ucs2 );

int zero4_to_space(bytes_t  ucs2, int ucs2len) ;

#endif /* GSM_UTIL */
