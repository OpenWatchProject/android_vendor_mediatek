/* jconfig.h.  Generated from jconfig.h.in by configure.  */
/* Version ID for the JPEG library.
 * Might be useful for tests like "#if JPEG_LIB_VERSION >= 60".
 */
#define JPEG_LIB_VERSION_ALPHA 62

/* libjpeg-turbo version */
#define LIBJPEG_TURBO_VERSION_ALPHA 1.4.2

/* Support arithmetic encoding */
/* #define C_ARITH_CODING_SUPPORTED 1 */

/* Support arithmetic decoding */
/* #define D_ARITH_CODING_SUPPORTED 1 */

/*
 * Define BITS_IN_JSAMPLE as either
 *   8   for 8-bit sample values (the usual setting)
 *   12  for 12-bit sample values
 * Only 8 and 12 are legal data precisions for lossy JPEG according to the
 * JPEG standard, and the IJG code does not support anything else!
 * We do not support run-time selection of data precision, sorry.
 */

#define BITS_IN_JSAMPLE_ALPHA  8      /* use 8 or 12 */

/* Define to 1 if you have the <locale.h> header file. */
#define HAVE_LOCALE_H_ALPHA 1

/* Define to 1 if you have the <stddef.h> header file. */
#define HAVE_STDDEF_H_ALPHA 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H_ALPHA 1

/* Define to 1 if the system has the type `unsigned char'. */
#define HAVE_UNSIGNED_CHAR_ALPHA 1

/* Define to 1 if the system has the type `unsigned short'. */
#define HAVE_UNSIGNED_SHORT_ALPHA 1

/* Compiler does not support pointers to undefined structures. */
/* #undef INCOMPLETE_TYPES_BROKEN */

/* Support in-memory source/destination managers */
#define MEM_SRCDST_SUPPORTED_ALPHA 1

/* Define if you have BSD-like bzero and bcopy in <strings.h> rather than
   memset/memcpy in <string.h>. */
/* #undef NEED_BSD_STRINGS */

/* Define if you need to include <sys/types.h> to get size_t. */
/* #undef NEED_SYS_TYPES_H 1 */

/* Define if your (broken) compiler shifts signed values as if they were
   unsigned. */
/* #undef RIGHT_SHIFT_IS_UNSIGNED */

/* Use accelerated SIMD routines. */
#define WITH_SIMD_ALPHA 1

/* Define to 1 if type `char' is unsigned and you are not using gcc.  */
#ifndef __CHAR_UNSIGNED__
/* # undef __CHAR_UNSIGNED__ */
#endif

/* Define to empty if `const' does not conform to ANSI C. */
/* #undef const */

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

/* The size of `size_t', as computed by sizeof. */
#if __WORDSIZE==64 || defined(_WIN64) || defined(__ARM64__)
#define SIZEOF_SIZE_T_ALPHA 8
#else
#define SIZEOF_SIZE_T_ALPHA 4
#endif

/*
 * The remaining options do not affect the JPEG library proper,
 * but only the sample applications cjpeg/djpeg (see cjpeg.c, djpeg.c).
 * Other applications can ignore these.
 */

#ifdef JPEG_CJPEG_DJPEG_ALPHA

/* These defines indicate which image (non-JPEG) file formats are allowed. */

#define BMP_SUPPORTED_ALPHA		/* BMP image file format */
//#define GIF_SUPPORTED		/* GIF image file format */
//#define PPM_SUPPORTED		/* PBMPLUS PPM/PGM image file format */
//#undef RLE_SUPPORTED		/* Utah RLE image file format */
//#define TARGA_SUPPORTED		/* Targa image file format */

/* Define this if you want to name both input and output files on the command
 * line, rather than using stdout and optionally stdin.  You MUST do this if
 * your system can't cope with binary I/O to stdin/stdout.  See comments at
 * head of cjpeg.c or djpeg.c.
 */
#define TWO_FILE_COMMANDLINE_ALPHA

/* Define this if your system needs explicit cleanup of temporary files.
 * This is crucial under MS-DOS, where the temporary "files" may be areas
 * of extended memory; on most other systems it's not as important.
 */
#undef NEED_SIGNAL_CATCHER_ALPHA

/* By default, we open image files with fopen(...,"rb") or fopen(...,"wb").
 * This is necessary on systems that distinguish text files from binary files,
 * and is harmless on most systems that don't.  If you have one of the rare
 * systems that complains about the "b" spec, define this symbol.
 */
#undef DONT_USE_B_MODE_ALPHA

/* Define this if you want percent-done progress reports from cjpeg/djpeg.
 */
#undef PROGRESS_REPORT_ALPHA

#endif				/* JPEG_CJPEG_DJPEG */

/* Common */
#define WRAPPED_API_ALPHA

/* Decoder Optimization*/
#define REGION_DECODE_M_ALPHA
#define USE_MERGED_UPSAMPLE_FOR_REGION_DECODE_M_ALPHA
#define H2V2_FANCY_UPSAMPLE_NEON_ALPHA
#define FILL_BIT_BUFFER_32BIT_EACH_TIME_ALPHA
#define DECODE_MCU_DISCARD_COEF_FAST_ALPHA
#define H2V2_MERGED_UPSAMPLE_NEON_ALPHA
#define H2V1_MERGED_UPSAMPLE_NEON_ALPHA

#define USE_HUFFMAN_OFFSET_STRUCT_V2_ALPHA
#ifdef USE_HUFFMAN_OFFSET_STRUCT_V2_ALPHA

#define DECODE_FULL_IMAGE_WITH_REGION_M_ALPHA
#ifdef DECODE_FULL_IMAGE_WITH_REGION_M_ALPHA
#define ASYNC_HUFFMAN_BUILDER_ALPHA
//#define DECODE_FULL_IMAGE_WITH_REGION_M_DEBUG_OPTION_ALPHA
#endif

#define REGION_DECODE_M_PIPELINED_ALPHA
#ifdef REGION_DECODE_M_PIPELINED_ALPHA
#define ASYNC_HUFFMAN_BUILDER_ALPHA
#endif

//#define ESL_PROFILER_ALPHA
#ifdef ESL_PROFILER_ALPHA
#undef DECODE_FULL_IMAGE_WITH_REGION_M_DEBUG_OPTION_ALPHA
#undef ASYNC_HUFFMAN_BUILDER_ALPHA
#endif

#ifdef ASYNC_HUFFMAN_BUILDER_ALPHA
#define BUILD_HUFFMAN_WITH_STANDALONE_CINFO_ALPHA
#else
#define HUFFMAN_BUILDER_STATE_BROADCAST_ALPHA
#endif
#endif

#define ENABLE_JPEG_OAL_ALPHA

#if defined(_WIN32)
#define MEASURE_TIME_ON_WINDOWS_ALPHA
#else
#define MEASURE_TIME_ON_ANDROID_ALPHA
#endif

/* Decoder Issue */
#define FIX_MERGED_UPSAMPLE_FOR_REGION_DECODE_N_ALPHA
#define FIX_USING_MERGED_UPSAMPLE_STATUS_UPDATE_ALPHA

//#define HYBRID_ALIGN_HW_BEHAVIOR_1_ALPHA
//#define HYBRID_ALIGN_HW_BEHAVIOR_2_ALPHA
//#define HYBRID_ALIGN_HW_BEHAVIOR_3_ALPHA
//#define HYBRID_ALIGN_HW_BEHAVIOR_4_ALPHA
//#define HYBRID_PATTERN_PARSER_ALPHA

/* Encoder Optimization */
#define EN_OPTIMIZE_RGB_YCC_NEON_ALPHA
#define EN_OPTIMIZE_DOWNSAMPLE_NEON_ALPHA
#define EN_OPTIMIZE_CONVSAMP_NEON_ALPHA
#define EN_OPTIMIZE_FDCT_NEON_ALPHA
#define EN_OPTIMIZE_QUANTIZE_NEON_ALPHA
#define EN_OPTIMIZE_HUFFMAN_NEON_ALPHA
#define EN_OPTIMIZE_INIT_SIMD_FLOW_ALPHA
