/*
 * jmorecfg.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 1997-2009 by Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2009, 2011, 2014-2015, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains additional configuration options that customize the
 * JPEG software for special applications or support machine-dependent
 * optimizations.  Most users will not need to touch this file.
 */


/*
 * Maximum number of components (color channels) allowed in JPEG image.
 * To meet the letter of the JPEG spec, set this to 255.  However, darn
 * few applications need more than 4 channels (maybe 5 for CMYK + alpha
 * mask).  We recommend 10 as a reasonable compromise; use 4 if you are
 * really short on memory.  (Each allowed component costs a hundred or so
 * bytes of storage, whether actually used in an image or not.)
 */

#define MAX_COMPONENTS_ALPHA  10      /* maximum number of image components */


/*
 * Basic data types.
 * You may need to change these if you have a machine with unusual data
 * type sizes; for example, "char" not 8 bits, "short" not 16 bits,
 * or "long" not 32 bits.  We don't care whether "int" is 16 or 32 bits,
 * but it had better be at least 16.
 */

/* Representation of a single sample (pixel element value).
 * We frequently allocate large arrays of these, so it's important to keep
 * them small.  But if you have memory to burn and access to char or short
 * arrays is very slow on your hardware, you might want to change these.
 */

#if BITS_IN_JSAMPLE_ALPHA == 8
/* JSAMPLE should be the smallest type that will hold the values 0..255.
 * You can use a signed char by having GETJSAMPLE mask it with 0xFF.
 */

#ifdef HAVE_UNSIGNED_CHAR_ALPHA

typedef unsigned char JSAMPLE_ALPHA;
#define GETJSAMPLE_ALPHA(value)  ((int) (value))

#else /* not HAVE_UNSIGNED_CHAR */

typedef char JSAMPLE_ALPHA;
#ifdef __CHAR_UNSIGNED__
#define GETJSAMPLE_ALPHA(value)  ((int) (value))
#else
#define GETJSAMPLE_ALPHA(value)  ((int) (value) & 0xFF)
#endif /* __CHAR_UNSIGNED__ */

#endif /* HAVE_UNSIGNED_CHAR */

#define MAXJSAMPLE_ALPHA      255
#define CENTERJSAMPLE_ALPHA   128

#endif /* BITS_IN_JSAMPLE == 8 */


#if BITS_IN_JSAMPLE_ALPHA == 12
/* JSAMPLE should be the smallest type that will hold the values 0..4095.
 * On nearly all machines "short" will do nicely.
 */

typedef short JSAMPLE_ALPHA;
#define GETJSAMPLE_ALPHA(value)  ((int) (value))

#define MAXJSAMPLE_ALPHA      4095
#define CENTERJSAMPLE_ALPHA   2048

#endif /* BITS_IN_JSAMPLE == 12 */


/* Representation of a DCT frequency coefficient.
 * This should be a signed value of at least 16 bits; "short" is usually OK.
 * Again, we allocate large arrays of these, but you can change to int
 * if you have memory to burn and "short" is really slow.
 */

typedef short JCOEF_ALPHA;


/* Compressed datastreams are represented as arrays of JOCTET.
 * These must be EXACTLY 8 bits wide, at least once they are written to
 * external storage.  Note that when using the stdio data source/destination
 * managers, this is also the data type passed to fread/fwrite.
 */

#ifdef HAVE_UNSIGNED_CHAR_ALPHA

typedef unsigned char JOCTET_ALPHA;
#define GETJOCTET_ALPHA(value)  (value)

#else /* not HAVE_UNSIGNED_CHAR */

typedef char JOCTET_ALPHA;
#ifdef __CHAR_UNSIGNED__
#define GETJOCTET_ALPHA(value)  (value)
#else
#define GETJOCTET_ALPHA(value)  ((value) & 0xFF)
#endif /* __CHAR_UNSIGNED__ */

#endif /* HAVE_UNSIGNED_CHAR */


/* These typedefs are used for various table entries and so forth.
 * They must be at least as wide as specified; but making them too big
 * won't cost a huge amount of memory, so we don't provide special
 * extraction code like we did for JSAMPLE.  (In other words, these
 * typedefs live at a different point on the speed/space tradeoff curve.)
 */

/* UINT8 must hold at least the values 0..255. */

#ifdef HAVE_UNSIGNED_CHAR_ALPHA
typedef unsigned char UINT8_ALPHA;
#else /* not HAVE_UNSIGNED_CHAR */
#ifdef __CHAR_UNSIGNED__
typedef char UINT8_ALPHA;
#else /* not __CHAR_UNSIGNED__ */
typedef short UINT8_ALPHA;
#endif /* __CHAR_UNSIGNED__ */
#endif /* HAVE_UNSIGNED_CHAR */

/* UINT16 must hold at least the values 0..65535. */

#ifdef HAVE_UNSIGNED_SHORT_ALPHA
typedef unsigned short UINT16_ALPHA;
#else /* not HAVE_UNSIGNED_SHORT */
typedef unsigned int UINT16_ALPHA;
#endif /* HAVE_UNSIGNED_SHORT */

/* INT16 must hold at least the values -32768..32767. */

#ifndef XMD_H                   /* X11/xmd.h correctly defines INT16 */
typedef short INT16_ALPHA;
#endif

/* INT32 must hold at least signed 32-bit values. */

#ifndef XMD_H                   /* X11/xmd.h correctly defines INT32 */
#ifndef _BASETSD_H_		/* Microsoft defines it in basetsd.h */
#ifndef _BASETSD_H		/* MinGW is slightly different */
#ifndef QGLOBAL_H		/* Qt defines it in qglobal.h */
#define __INT32_IS_ACTUALLY_LONG_ALPHA
typedef long INT32_ALPHA;
#endif
#endif
#endif
#endif

/* Datatype used for image dimensions.  The JPEG standard only supports
 * images up to 64K*64K due to 16-bit fields in SOF markers.  Therefore
 * "unsigned int" is sufficient on all machines.  However, if you need to
 * handle larger images and you don't mind deviating from the spec, you
 * can change this datatype.  (Note that changing this datatype will
 * potentially require modifying the SIMD code.  The x86-64 SIMD extensions,
 * in particular, assume a 32-bit JDIMENSION.)
 */

typedef unsigned int JDIMENSION_ALPHA;

#define JPEG_MAX_DIMENSION_ALPHA  65500L  /* a tad under 64K to prevent overflows */


/* These macros are used in all function definitions and extern declarations.
 * You could modify them if you need to change function linkage conventions;
 * in particular, you'll need to do that to make the library a Windows DLL.
 * Another application is to make all functions global for use with debuggers
 * or code profilers that require it.
 */

/* a function called through method pointers: */
#define METHODDEF_ALPHA(type)         static type
/* a function used only in its module: */
#define LOCAL_ALPHA(type)             static type
/* a function referenced thru EXTERNs: */
#define GLOBAL_ALPHA(type)            type
/* a reference to a GLOBAL function: */
#define EXTERN_ALPHA(type)            extern type


/* Originally, this macro was used as a way of defining function prototypes
 * for both modern compilers as well as older compilers that did not support
 * prototype parameters.  libjpeg-turbo has never supported these older,
 * non-ANSI compilers, but the macro is still included because there is some
 * software out there that uses it.
 */

#define JMETHOD_ALPHA(type,methodname,arglist)  type (*methodname) arglist


/* libjpeg-turbo no longer supports platforms that have far symbols (MS-DOS),
 * but again, some software relies on this macro.
 */

#undef FAR_ALPHA
#define FAR_ALPHA


/*
 * On a few systems, type boolean and/or its values FALSE, TRUE may appear
 * in standard header files.  Or you may have conflicts with application-
 * specific header files that you want to include together with these files.
 * Defining HAVE_BOOLEAN before including jpeglib.h should make it work.
 */

#ifndef HAVE_BOOLEAN_ALPHA
typedef int boolean_ALPHA;
#endif
#ifndef FALSE_ALPHA                   /* in case these macros already exist */
#define FALSE_ALPHA   0               /* values of boolean */
#endif
#ifndef TRUE_ALPHA
#define TRUE_ALPHA    1
#endif


/*
 * The remaining options affect code selection within the JPEG library,
 * but they don't need to be visible to most applications using the library.
 * To minimize application namespace pollution, the symbols won't be
 * defined unless JPEG_INTERNALS or JPEG_INTERNAL_OPTIONS has been defined.
 */

#ifdef JPEG_INTERNALS_ALPHA
#define JPEG_INTERNAL_OPTIONS_ALPHA
#endif

#ifdef JPEG_INTERNAL_OPTIONS_ALPHA


/*
 * These defines indicate whether to include various optional functions.
 * Undefining some of these symbols will produce a smaller but less capable
 * library.  Note that you can leave certain source files out of the
 * compilation/linking process if you've #undef'd the corresponding symbols.
 * (You may HAVE to do that if your compiler doesn't like null source files.)
 */

/* Capability options common to encoder and decoder: */

#define DCT_ISLOW_SUPPORTED_ALPHA     /* slow but accurate integer algorithm */
#define DCT_IFAST_SUPPORTED_ALPHA     /* faster, less accurate integer method */
#define DCT_FLOAT_SUPPORTED_ALPHA     /* floating-point: accurate, fast on fast HW */

/* Encoder capability options: */

#define C_MULTISCAN_FILES_SUPPORTED_ALPHA /* Multiple-scan JPEG files? */
#define C_PROGRESSIVE_SUPPORTED_ALPHA     /* Progressive JPEG? (Requires MULTISCAN)*/
#define ENTROPY_OPT_SUPPORTED_ALPHA       /* Optimization of entropy coding parms? */
/* Note: if you selected 12-bit data precision, it is dangerous to turn off
 * ENTROPY_OPT_SUPPORTED.  The standard Huffman tables are only good for 8-bit
 * precision, so jchuff.c normally uses entropy optimization to compute
 * usable tables for higher precision.  If you don't want to do optimization,
 * you'll have to supply different default Huffman tables.
 * The exact same statements apply for progressive JPEG: the default tables
 * don't work for progressive mode.  (This may get fixed, however.)
 */
#define INPUT_SMOOTHING_SUPPORTED_ALPHA   /* Input image smoothing option? */

/* Decoder capability options: */

#define D_MULTISCAN_FILES_SUPPORTED_ALPHA /* Multiple-scan JPEG files? */
#define D_PROGRESSIVE_SUPPORTED_ALPHA     /* Progressive JPEG? (Requires MULTISCAN)*/
#define SAVE_MARKERS_SUPPORTED_ALPHA      /* jpeg_save_markers() needed? */
#define BLOCK_SMOOTHING_SUPPORTED_ALPHA   /* Block smoothing? (Progressive only) */
#define IDCT_SCALING_SUPPORTED_ALPHA      /* Output rescaling via IDCT? */
#undef  UPSAMPLE_SCALING_SUPPORTED_ALPHA  /* Output rescaling at upsample stage? */
#define UPSAMPLE_MERGING_SUPPORTED_ALPHA  /* Fast path for sloppy upsampling? */
#define QUANT_1PASS_SUPPORTED_ALPHA       /* 1-pass color quantization? */
#define QUANT_2PASS_SUPPORTED_ALPHA       /* 2-pass color quantization? */

/* more capability options later, no doubt */


/*
 * The RGB_RED, RGB_GREEN, RGB_BLUE, and RGB_PIXELSIZE macros are a vestigial
 * feature of libjpeg.  The idea was that, if an application developer needed
 * to compress from/decompress to a BGR/BGRX/RGBX/XBGR/XRGB buffer, they could
 * change these macros, rebuild libjpeg, and link their application statically
 * with it.  In reality, few people ever did this, because there were some
 * severe restrictions involved (cjpeg and djpeg no longer worked properly,
 * compressing/decompressing RGB JPEGs no longer worked properly, and the color
 * quantizer wouldn't work with pixel sizes other than 3.)  Further, since all
 * of the O/S-supplied versions of libjpeg were built with the default values
 * of RGB_RED, RGB_GREEN, RGB_BLUE, and RGB_PIXELSIZE, many applications have
 * come to regard these values as immutable.
 *
 * The libjpeg-turbo colorspace extensions provide a much cleaner way of
 * compressing from/decompressing to buffers with arbitrary component orders
 * and pixel sizes.  Thus, we do not support changing the values of RGB_RED,
 * RGB_GREEN, RGB_BLUE, or RGB_PIXELSIZE.  In addition to the restrictions
 * listed above, changing these values will also break the SIMD extensions and
 * the regression tests.
 */

#define RGB_RED_ALPHA         0       /* Offset of Red in an RGB scanline element */
#define RGB_GREEN_ALPHA       1       /* Offset of Green */
#define RGB_BLUE_ALPHA        2       /* Offset of Blue */
#define RGB_PIXELSIZE_ALPHA   3       /* JSAMPLEs per RGB scanline element */

#define JPEG_NUMCS_ALPHA 17

#define EXT_RGB_RED_ALPHA        0
#define EXT_RGB_GREEN_ALPHA      1
#define EXT_RGB_BLUE_ALPHA       2
#define EXT_RGB_PIXELSIZE_ALPHA  3

#define EXT_RGBX_RED_ALPHA       0
#define EXT_RGBX_GREEN_ALPHA     1
#define EXT_RGBX_BLUE_ALPHA      2
#define EXT_RGBX_PIXELSIZE_ALPHA 4

#define EXT_BGR_RED_ALPHA        2
#define EXT_BGR_GREEN_ALPHA      1
#define EXT_BGR_BLUE_ALPHA       0
#define EXT_BGR_PIXELSIZE_ALPHA  3

#define EXT_BGRX_RED_ALPHA       2
#define EXT_BGRX_GREEN_ALPHA     1
#define EXT_BGRX_BLUE_ALPHA      0
#define EXT_BGRX_PIXELSIZE_ALPHA 4

#define EXT_XBGR_RED_ALPHA       3
#define EXT_XBGR_GREEN_ALPHA     2
#define EXT_XBGR_BLUE_ALPHA      1
#define EXT_XBGR_PIXELSIZE_ALPHA 4

#define EXT_XRGB_RED_ALPHA       1
#define EXT_XRGB_GREEN_ALPHA     2
#define EXT_XRGB_BLUE_ALPHA      3
#define EXT_XRGB_PIXELSIZE_ALPHA 4

static const int rgb_red[JPEG_NUMCS_ALPHA] = {
  -1, -1, RGB_RED_ALPHA, -1, -1, -1, EXT_RGB_RED_ALPHA, EXT_RGBX_RED_ALPHA,
  EXT_BGR_RED_ALPHA, EXT_BGRX_RED_ALPHA, EXT_XBGR_RED_ALPHA, EXT_XRGB_RED_ALPHA,
  EXT_RGBX_RED_ALPHA, EXT_BGRX_RED_ALPHA, EXT_XBGR_RED_ALPHA, EXT_XRGB_RED_ALPHA,
  -1
};

static const int rgb_green[JPEG_NUMCS_ALPHA] = {
  -1, -1, RGB_GREEN_ALPHA, -1, -1, -1, EXT_RGB_GREEN_ALPHA, EXT_RGBX_GREEN_ALPHA,
  EXT_BGR_GREEN_ALPHA, EXT_BGRX_GREEN_ALPHA, EXT_XBGR_GREEN_ALPHA, EXT_XRGB_GREEN_ALPHA,
  EXT_RGBX_GREEN_ALPHA, EXT_BGRX_GREEN_ALPHA, EXT_XBGR_GREEN_ALPHA, EXT_XRGB_GREEN_ALPHA,
  -1
};

static const int rgb_blue[JPEG_NUMCS_ALPHA] = {
  -1, -1, RGB_BLUE_ALPHA, -1, -1, -1, EXT_RGB_BLUE_ALPHA, EXT_RGBX_BLUE_ALPHA,
  EXT_BGR_BLUE_ALPHA, EXT_BGRX_BLUE_ALPHA, EXT_XBGR_BLUE_ALPHA, EXT_XRGB_BLUE_ALPHA,
  EXT_RGBX_BLUE_ALPHA, EXT_BGRX_BLUE_ALPHA, EXT_XBGR_BLUE_ALPHA, EXT_XRGB_BLUE_ALPHA,
  -1
};

static const int rgb_pixelsize[JPEG_NUMCS_ALPHA] = {
  -1, -1, RGB_PIXELSIZE_ALPHA, -1, -1, -1, EXT_RGB_PIXELSIZE_ALPHA, EXT_RGBX_PIXELSIZE_ALPHA,
  EXT_BGR_PIXELSIZE_ALPHA, EXT_BGRX_PIXELSIZE_ALPHA, EXT_XBGR_PIXELSIZE_ALPHA, EXT_XRGB_PIXELSIZE_ALPHA,
  EXT_RGBX_PIXELSIZE_ALPHA, EXT_BGRX_PIXELSIZE_ALPHA, EXT_XBGR_PIXELSIZE_ALPHA, EXT_XRGB_PIXELSIZE_ALPHA,
  -1
};

/* Definitions for speed-related optimizations. */

/* On some machines (notably 68000 series) "int" is 32 bits, but multiplying
 * two 16-bit shorts is faster than multiplying two ints.  Define MULTIPLIER
 * as short on such a machine.  MULTIPLIER must be at least 16 bits wide.
 */

#ifndef MULTIPLIER_ALPHA
#ifndef WITH_SIMD_ALPHA
#define MULTIPLIER_ALPHA  int         /* type for fastest integer multiply */
#else
#define MULTIPLIER_ALPHA short  /* prefer 16-bit with SIMD for parellelism */
#endif
#endif


/* FAST_FLOAT should be either float or double, whichever is done faster
 * by your compiler.  (Note that this type is only used in the floating point
 * DCT routines, so it only matters if you've defined DCT_FLOAT_SUPPORTED.)
 */

#ifndef FAST_FLOAT_ALPHA
#define FAST_FLOAT_ALPHA  float
#endif

#endif /* JPEG_INTERNAL_OPTIONS */
