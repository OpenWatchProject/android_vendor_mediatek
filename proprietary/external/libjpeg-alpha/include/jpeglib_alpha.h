/*
 * jpeglib.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1998, Thomas G. Lane.
 * Modified 2002-2009 by Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2009-2011, 2013-2014, 2016, D. R. Commander.
 * Copyright (C) 2015, Google, Inc.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file defines the application interface for the JPEG library.
 * Most applications using the library need only include this file,
 * and perhaps jerror.h if they want to know the exact error codes.
 */

#ifndef JPEGLIB_H_ALPHA
#define JPEGLIB_H_ALPHA

/*
 * First we include the configuration files that record how this
 * installation of the JPEG library is set up.  jconfig.h can be
 * generated automatically for many systems.  jmorecfg.h contains
 * manual configuration options that most people need not worry about.
 */

#ifndef JCONFIG_INCLUDED_ALPHA        /* in case jinclude.h already did */
#include "jconfig_alpha.h"            /* widely used configuration options */
#endif
#include "jmorecfg_alpha.h"           /* seldom changed options */


#ifdef __cplusplus
#ifndef DONT_USE_EXTERN_C
extern "C" {
#endif
#endif


/* Various constants determining the sizes of things.
 * All of these are specified by the JPEG standard, so don't change them
 * if you want to be compatible.
 */

#define DCTSIZE_ALPHA             8   /* The basic DCT block is 8x8 samples */
#define DCTSIZE2_ALPHA            64  /* DCTSIZE squared; # of elements in a block */
#define NUM_QUANT_TBLS_ALPHA      4   /* Quantization tables are numbered 0..3 */
#define NUM_HUFF_TBLS_ALPHA       4   /* Huffman tables are numbered 0..3 */
#define NUM_ARITH_TBLS_ALPHA      16  /* Arith-coding tables are numbered 0..15 */
#define MAX_COMPS_IN_SCAN_ALPHA   4   /* JPEG limit on # of components in one scan */
#define MAX_SAMP_FACTOR_ALPHA     4   /* JPEG limit on sampling factors */
/* Unfortunately, some bozo at Adobe saw no reason to be bound by the standard;
 * the PostScript DCT filter can emit files with many more than 10 blocks/MCU.
 * If you happen to run across such a file, you can up D_MAX_BLOCKS_IN_MCU
 * to handle it.  We even let you do this from the jconfig.h file.  However,
 * we strongly discourage changing C_MAX_BLOCKS_IN_MCU; just because Adobe
 * sometimes emits noncompliant files doesn't mean you should too.
 */
#define C_MAX_BLOCKS_IN_MCU_ALPHA   10 /* compressor's limit on blocks per MCU */
#ifndef D_MAX_BLOCKS_IN_MCU_ALPHA
#define D_MAX_BLOCKS_IN_MCU_ALPHA   10 /* decompressor's limit on blocks per MCU */
#endif


/* Data structures for images (arrays of samples and of DCT coefficients).
 */

typedef JSAMPLE_ALPHA *JSAMPROW_ALPHA;      /* ptr to one image row of pixel samples. */
typedef JSAMPROW_ALPHA *JSAMPARRAY_ALPHA;   /* ptr to some rows (a 2-D sample array) */
typedef JSAMPARRAY_ALPHA *JSAMPIMAGE_ALPHA; /* a 3-D sample array: top index is color */

typedef JCOEF_ALPHA JBLOCK_ALPHA[DCTSIZE2_ALPHA]; /* one block of coefficients */
typedef JBLOCK_ALPHA *JBLOCKROW_ALPHA;      /* pointer to one row of coefficient blocks */
typedef JBLOCKROW_ALPHA *JBLOCKARRAY_ALPHA;         /* a 2-D array of coefficient blocks */
typedef JBLOCKARRAY_ALPHA *JBLOCKIMAGE_ALPHA;       /* a 3-D array of coefficient blocks */

typedef JCOEF_ALPHA *JCOEFPTR_ALPHA;        /* useful in a couple of places */


/* Types for JPEG compression parameters and working tables. */


/* DCT coefficient quantization tables. */

typedef struct {
  /* This array gives the coefficient quantizers in natural array order
   * (not the zigzag order in which they are stored in a JPEG DQT marker).
   * CAUTION: IJG versions prior to v6a kept this array in zigzag order.
   */
  UINT16_ALPHA quantval[DCTSIZE2_ALPHA];    /* quantization step for each coefficient */
  /* This field is used only during compression.  It's initialized FALSE when
   * the table is created, and set TRUE when it's been output to the file.
   * You could suppress output of a table by setting this to TRUE.
   * (See jpeg_suppress_tables for an example.)
   */
  boolean_ALPHA sent_table;           /* TRUE when table has been output */
} JQUANT_TBL_ALPHA;


/* Huffman coding tables. */

typedef struct {
  /* These two fields directly represent the contents of a JPEG DHT marker */
  UINT8_ALPHA bits[17];               /* bits[k] = # of symbols with codes of */
                                /* length k bits; bits[0] is unused */
  UINT8_ALPHA huffval[256];           /* The symbols, in order of incr code length */
  /* This field is used only during compression.  It's initialized FALSE when
   * the table is created, and set TRUE when it's been output to the file.
   * You could suppress output of a table by setting this to TRUE.
   * (See jpeg_suppress_tables for an example.)
   */
  boolean_ALPHA sent_table;           /* TRUE when table has been output */
} JHUFF_TBL_ALPHA;


/* Basic info about one component (color channel). */

typedef struct {
  /* These values are fixed over the whole image. */
  /* For compression, they must be supplied by parameter setup; */
  /* for decompression, they are read from the SOF marker. */
  int component_id;             /* identifier for this component (0..255) */
  int component_index;          /* its index in SOF or cinfo->comp_info[] */
  int h_samp_factor;            /* horizontal sampling factor (1..4) */
  int v_samp_factor;            /* vertical sampling factor (1..4) */
  int quant_tbl_no;             /* quantization table selector (0..3) */
  /* These values may vary between scans. */
  /* For compression, they must be supplied by parameter setup; */
  /* for decompression, they are read from the SOS marker. */
  /* The decompressor output side may not use these variables. */
  int dc_tbl_no;                /* DC entropy table selector (0..3) */
  int ac_tbl_no;                /* AC entropy table selector (0..3) */

  /* Remaining fields should be treated as private by applications. */

  /* These values are computed during compression or decompression startup: */
  /* Component's size in DCT blocks.
   * Any dummy blocks added to complete an MCU are not counted; therefore
   * these values do not depend on whether a scan is interleaved or not.
   */
  JDIMENSION_ALPHA width_in_blocks;
  JDIMENSION_ALPHA height_in_blocks;
  /* Size of a DCT block in samples.  Always DCTSIZE for compression.
   * For decompression this is the size of the output from one DCT block,
   * reflecting any scaling we choose to apply during the IDCT step.
   * Values from 1 to 16 are supported.
   * Note that different components may receive different IDCT scalings.
   */
#if JPEG_LIB_VERSION_ALPHA >= 70
  int DCT_h_scaled_size;
  int DCT_v_scaled_size;
#else
  int DCT_scaled_size;
#endif
  /* The downsampled dimensions are the component's actual, unpadded number
   * of samples at the main buffer (preprocessing/compression interface), thus
   * downsampled_width = ceil(image_width * Hi/Hmax)
   * and similarly for height.  For decompression, IDCT scaling is included, so
   * downsampled_width = ceil(image_width * Hi/Hmax * DCT_[h_]scaled_size/DCTSIZE)
   */
  JDIMENSION_ALPHA downsampled_width;  /* actual width in samples */
  JDIMENSION_ALPHA downsampled_height; /* actual height in samples */
  /* This flag is used only for decompression.  In cases where some of the
   * components will be ignored (eg grayscale output from YCbCr image),
   * we can skip most computations for the unused components.
   */
  boolean_ALPHA component_needed;     /* do we need the value of this component? */

  /* These values are computed before starting a scan of the component. */
  /* The decompressor output side may not use these variables. */
  int MCU_width;                /* number of blocks per MCU, horizontally */
  int MCU_height;               /* number of blocks per MCU, vertically */
  int MCU_blocks;               /* MCU_width * MCU_height */
  int MCU_sample_width;         /* MCU width in samples, MCU_width*DCT_[h_]scaled_size */
  int last_col_width;           /* # of non-dummy blocks across in last MCU */
  int last_row_height;          /* # of non-dummy blocks down in last MCU */

  /* Saved quantization table for component; NULL if none yet saved.
   * See jdinput.c comments about the need for this information.
   * This field is currently used only for decompression.
   */
  JQUANT_TBL_ALPHA * quant_table;

  /* Private per-component storage for DCT or IDCT subsystem. */
  void * dct_table;
} jpeg_component_info_ALPHA;


/* The script for encoding a multiple-scan file is an array of these: */

typedef struct {
  int comps_in_scan;            /* number of components encoded in this scan */
  int component_index[MAX_COMPS_IN_SCAN_ALPHA]; /* their SOF/comp_info[] indexes */
  int Ss, Se;                   /* progressive JPEG spectral selection parms */
  int Ah, Al;                   /* progressive JPEG successive approx. parms */
} jpeg_scan_info_ALPHA;

/* The decompressor can save APPn and COM markers in a list of these: */

typedef struct jpeg_marker_struct_ALPHA * jpeg_saved_marker_ptr_ALPHA;

struct jpeg_marker_struct_ALPHA {
  jpeg_saved_marker_ptr_ALPHA next;   /* next in list, or NULL */
  UINT8_ALPHA marker;                 /* marker code: JPEG_COM, or JPEG_APP0+n */
  unsigned int original_length; /* # bytes of data in the file */
  unsigned int data_length;     /* # bytes of data saved at data[] */
  JOCTET_ALPHA * data;                /* the data contained in the marker */
  /* the marker length word is not counted in data_length or original_length */
};

/* Known color spaces. */

#define JCS_EXTENSIONS_ALPHA 1
#define JCS_ALPHA_EXTENSIONS_ALPHA 1

typedef enum {
  JCS_UNKNOWN_ALPHA,            /* error/unspecified */
  JCS_GRAYSCALE_ALPHA,          /* monochrome */
  JCS_RGB_ALPHA,                /* red/green/blue as specified by the RGB_RED,
                             RGB_GREEN, RGB_BLUE, and RGB_PIXELSIZE macros */
  JCS_YCbCr_ALPHA,              /* Y/Cb/Cr (also known as YUV) */
  JCS_CMYK_ALPHA,               /* C/M/Y/K */
  JCS_YCCK_ALPHA,               /* Y/Cb/Cr/K */
  JCS_EXT_RGB_ALPHA,            /* red/green/blue */
  JCS_EXT_RGBX_ALPHA,           /* red/green/blue/x */
  JCS_EXT_BGR_ALPHA,            /* blue/green/red */
  JCS_EXT_BGRX_ALPHA,           /* blue/green/red/x */
  JCS_EXT_XBGR_ALPHA,           /* x/blue/green/red */
  JCS_EXT_XRGB_ALPHA,           /* x/red/green/blue */
  /* When out_color_space it set to JCS_EXT_RGBX, JCS_EXT_BGRX, JCS_EXT_XBGR,
     or JCS_EXT_XRGB during decompression, the X byte is undefined, and in
     order to ensure the best performance, libjpeg-turbo can set that byte to
     whatever value it wishes.  Use the following colorspace constants to
     ensure that the X byte is set to 0xFF, so that it can be interpreted as an
     opaque alpha channel. */
  JCS_EXT_RGBA_ALPHA,           /* red/green/blue/alpha */
  JCS_EXT_BGRA_ALPHA,           /* blue/green/red/alpha */
  JCS_EXT_ABGR_ALPHA,           /* alpha/blue/green/red */
  JCS_EXT_ARGB_ALPHA,           /* alpha/red/green/blue */
  JCS_RGB565_ALPHA             /* 5-bit red/6-bit green/5-bit blue */
} J_COLOR_SPACE_ALPHA;

/* DCT/IDCT algorithm options. */

typedef enum {
  JDCT_ISLOW_ALPHA,             /* slow but accurate integer algorithm */
  JDCT_IFAST_ALPHA,             /* faster, less accurate integer method */
  JDCT_FLOAT_ALPHA              /* floating-point: accurate, fast on fast HW */
} J_DCT_METHOD_ALPHA;

#ifndef JDCT_DEFAULT_ALPHA            /* may be overridden in jconfig.h */
#define JDCT_DEFAULT_ALPHA  JDCT_ISLOW_ALPHA
#endif
#ifndef JDCT_FASTEST_ALPHA            /* may be overridden in jconfig.h */
#define JDCT_FASTEST_ALPHA  JDCT_IFAST_ALPHA
#endif

/* Dithering options for decompression. */

typedef enum {
  JDITHER_NONE_ALPHA,           /* no dithering */
  JDITHER_ORDERED_ALPHA,        /* simple ordered dither */
  JDITHER_FS_ALPHA              /* Floyd-Steinberg error diffusion dither */
} J_DITHER_MODE_ALPHA;


/* Common fields between JPEG compression and decompression master structs. */

#define jpeg_common_fields_ALPHA \
  struct jpeg_error_mgr_ALPHA * err;  /* Error handler module */\
  struct jpeg_memory_mgr_ALPHA * mem; /* Memory manager module */\
  struct jpeg_progress_mgr_ALPHA * progress; /* Progress monitor, or NULL if none */\
  void * client_data;           /* Available for use by application */\
  boolean_ALPHA is_decompressor;      /* So common code can tell which is which */\
  int global_state              /* For checking call sequence validity */

/* Routines that are to be used by both halves of the library are declared
 * to receive a pointer to this structure.  There are no actual instances of
 * jpeg_common_struct, only of jpeg_compress_struct and jpeg_decompress_struct.
 */
struct jpeg_common_struct_ALPHA {
  jpeg_common_fields_ALPHA;           /* Fields common to both master struct types */
  /* Additional fields follow in an actual jpeg_compress_struct or
   * jpeg_decompress_struct.  All three structs must agree on these
   * initial fields!  (This would be a lot cleaner in C++.)
   */
};

typedef struct jpeg_common_struct_ALPHA * j_common_ptr_ALPHA;
typedef struct jpeg_compress_struct_ALPHA * j_compress_ptr_ALPHA;
typedef struct jpeg_decompress_struct_ALPHA * j_decompress_ptr_ALPHA;

/* Encode speed options for multi-core algorithm */
typedef enum {
  SLOW_ALPHA,
  FAST_ALPHA,
  ULTRA_FAST_ALPHA
} J_ENCODE_SPEED_ALPHA;

/* Master record for a compression instance */

struct jpeg_compress_struct_ALPHA {
  jpeg_common_fields_ALPHA;           /* Fields shared with jpeg_decompress_struct */
#ifdef ENABLE_JPEG_OAL_ALPHA
  struct jpeg_oal_mgr_ALPHA * oal;
  int is_oal_init;
#endif
  /* Destination for compressed data */
  struct jpeg_destination_mgr_ALPHA * dest;

  /* Description of source image --- these fields must be filled in by
   * outer application before starting compression.  in_color_space must
   * be correct before you can even call jpeg_set_defaults().
   */

  JDIMENSION_ALPHA image_width;       /* input image width */
  JDIMENSION_ALPHA image_height;      /* input image height */
  int input_components;         /* # of color components in input image */
  J_COLOR_SPACE_ALPHA in_color_space; /* colorspace of input image */

  double input_gamma;           /* image gamma of input image */

  /* Compression parameters --- these fields must be set before calling
   * jpeg_start_compress().  We recommend calling jpeg_set_defaults() to
   * initialize everything to reasonable defaults, then changing anything
   * the application specifically wants to change.  That way you won't get
   * burnt when new parameters are added.  Also note that there are several
   * helper routines to simplify changing parameters.
   */

#if JPEG_LIB_VERSION_ALPHA >= 70
  unsigned int scale_num, scale_denom; /* fraction by which to scale image */

  JDIMENSION_ALPHA jpeg_width;        /* scaled JPEG image width */
  JDIMENSION_ALPHA jpeg_height;       /* scaled JPEG image height */
  /* Dimensions of actual JPEG image that will be written to file,
   * derived from input dimensions by scaling factors above.
   * These fields are computed by jpeg_start_compress().
   * You can also use jpeg_calc_jpeg_dimensions() to determine these values
   * in advance of calling jpeg_start_compress().
   */
#endif

  int data_precision;           /* bits of precision in image data */

  int num_components;           /* # of color components in JPEG image */
  J_COLOR_SPACE_ALPHA jpeg_color_space; /* colorspace of JPEG image */

  jpeg_component_info_ALPHA * comp_info;
  /* comp_info[i] describes component that appears i'th in SOF */

  JQUANT_TBL_ALPHA * quant_tbl_ptrs[NUM_QUANT_TBLS_ALPHA];
#if JPEG_LIB_VERSION_ALPHA >= 70
  int q_scale_factor[NUM_QUANT_TBLS_ALPHA];
#endif
  /* ptrs to coefficient quantization tables, or NULL if not defined,
   * and corresponding scale factors (percentage, initialized 100).
   */

  JHUFF_TBL_ALPHA * dc_huff_tbl_ptrs[NUM_HUFF_TBLS_ALPHA];
  JHUFF_TBL_ALPHA * ac_huff_tbl_ptrs[NUM_HUFF_TBLS_ALPHA];
  /* ptrs to Huffman coding tables, or NULL if not defined */

  UINT8_ALPHA arith_dc_L[NUM_ARITH_TBLS_ALPHA]; /* L values for DC arith-coding tables */
  UINT8_ALPHA arith_dc_U[NUM_ARITH_TBLS_ALPHA]; /* U values for DC arith-coding tables */
  UINT8_ALPHA arith_ac_K[NUM_ARITH_TBLS_ALPHA]; /* Kx values for AC arith-coding tables */

  int num_scans;                /* # of entries in scan_info array */
  const jpeg_scan_info_ALPHA * scan_info; /* script for multi-scan file, or NULL */
  /* The default value of scan_info is NULL, which causes a single-scan
   * sequential JPEG file to be emitted.  To create a multi-scan file,
   * set num_scans and scan_info to point to an array of scan definitions.
   */

  boolean_ALPHA raw_data_in;          /* TRUE=caller supplies downsampled data */
  boolean_ALPHA arith_code;           /* TRUE=arithmetic coding, FALSE=Huffman */
  boolean_ALPHA optimize_coding;      /* TRUE=optimize entropy encoding parms */
  boolean_ALPHA CCIR601_sampling;     /* TRUE=first samples are cosited */
#if JPEG_LIB_VERSION_ALPHA >= 70
  boolean_ALPHA do_fancy_downsampling; /* TRUE=apply fancy downsampling */
#endif
  int smoothing_factor;         /* 1..100, or 0 for no input smoothing */
  J_DCT_METHOD_ALPHA dct_method;      /* DCT algorithm selector */

  /* The restart interval can be specified in absolute MCUs by setting
   * restart_interval, or in MCU rows by setting restart_in_rows
   * (in which case the correct restart_interval will be figured
   * for each scan).
   */
  unsigned int restart_interval; /* MCUs per restart, or 0 for no restart */
  int restart_in_rows;          /* if > 0, MCU rows per restart interval */

  /* Parameters controlling emission of special markers. */

  boolean_ALPHA write_JFIF_header;    /* should a JFIF marker be written? */
  UINT8_ALPHA JFIF_major_version;     /* What to write for the JFIF version number */
  UINT8_ALPHA JFIF_minor_version;
  /* These three values are not used by the JPEG code, merely copied */
  /* into the JFIF APP0 marker.  density_unit can be 0 for unknown, */
  /* 1 for dots/inch, or 2 for dots/cm.  Note that the pixel aspect */
  /* ratio is defined by X_density/Y_density even when density_unit=0. */
  UINT8_ALPHA density_unit;           /* JFIF code for pixel size units */
  UINT16_ALPHA X_density;             /* Horizontal pixel density */
  UINT16_ALPHA Y_density;             /* Vertical pixel density */
  boolean_ALPHA write_Adobe_marker;   /* should an Adobe marker be written? */

  /* State variable: index of next scanline to be written to
   * jpeg_write_scanlines().  Application may use this to control its
   * processing loop, e.g., "while (next_scanline < image_height)".
   */

  JDIMENSION_ALPHA next_scanline;     /* 0 .. image_height-1  */

  /* Remaining fields are known throughout compressor, but generally
   * should not be touched by a surrounding application.
   */

  /*
   * These fields are computed during compression startup
   */
  boolean_ALPHA progressive_mode;     /* TRUE if scan script uses progressive mode */
  int max_h_samp_factor;        /* largest h_samp_factor */
  int max_v_samp_factor;        /* largest v_samp_factor */

#if JPEG_LIB_VERSION_ALPHA >= 70
  int min_DCT_h_scaled_size;    /* smallest DCT_h_scaled_size of any component */
  int min_DCT_v_scaled_size;    /* smallest DCT_v_scaled_size of any component */
#endif

  JDIMENSION_ALPHA total_iMCU_rows;   /* # of iMCU rows to be input to coef ctlr */
  /* The coefficient controller receives data in units of MCU rows as defined
   * for fully interleaved scans (whether the JPEG file is interleaved or not).
   * There are v_samp_factor * DCTSIZE sample rows of each component in an
   * "iMCU" (interleaved MCU) row.
   */

  /*
   * These fields are valid during any one scan.
   * They describe the components and MCUs actually appearing in the scan.
   */
  int comps_in_scan;            /* # of JPEG components in this scan */
  jpeg_component_info_ALPHA * cur_comp_info[MAX_COMPS_IN_SCAN_ALPHA];
  /* *cur_comp_info[i] describes component that appears i'th in SOS */

  JDIMENSION_ALPHA MCUs_per_row;      /* # of MCUs across the image */
  JDIMENSION_ALPHA MCU_rows_in_scan;  /* # of MCU rows in the image */

  int blocks_in_MCU;            /* # of DCT blocks per MCU */
  int MCU_membership[C_MAX_BLOCKS_IN_MCU_ALPHA];
  /* MCU_membership[i] is index in cur_comp_info of component owning */
  /* i'th block in an MCU */

  int Ss, Se, Ah, Al;           /* progressive JPEG parameters for scan */

#if JPEG_LIB_VERSION_ALPHA >= 80
  int block_size;               /* the basic DCT block size: 1..16 */
  const int * natural_order;    /* natural-order position array */
  int lim_Se;                   /* min( Se, DCTSIZE2-1 ) */
#endif

  /*
   * Links to compression subobjects (methods and private variables of modules)
   */
  struct jpeg_comp_master_ALPHA * master;
  struct jpeg_c_main_controller_ALPHA * main;
  struct jpeg_c_prep_controller_ALPHA * prep;
  struct jpeg_c_coef_controller_ALPHA * coef;
  struct jpeg_marker_writer_ALPHA * marker;
  struct jpeg_color_converter_ALPHA * cconvert;
  struct jpeg_downsampler_ALPHA * downsample;
  struct jpeg_forward_dct_ALPHA * fdct;
  struct jpeg_entropy_encoder_ALPHA * entropy;
  jpeg_scan_info_ALPHA * script_space; /* workspace for jpeg_simple_progression */
  int script_space_size;

  J_ENCODE_SPEED_ALPHA encode_speed;
  int en_soi;
};


/* Master record for a decompression instance */

struct jpeg_decompress_struct_ALPHA {
  jpeg_common_fields_ALPHA;           /* Fields shared with jpeg_compress_struct */
#ifdef ENABLE_JPEG_OAL_ALPHA
  struct jpeg_oal_mgr_ALPHA * oal;
  int is_oal_init;
#endif

  /* Source of compressed data */
  struct jpeg_source_mgr_ALPHA * src;

  /* Basic description of image --- filled in by jpeg_read_header(). */
  /* Application may inspect these values to decide how to process image. */
#ifdef REGION_DECODE_M_ALPHA
  JDIMENSION_ALPHA original_image_width;
#endif
  JDIMENSION_ALPHA image_width;       /* nominal image width (from SOF marker) */
  JDIMENSION_ALPHA image_height;      /* nominal image height */
  int num_components;           /* # of color components in JPEG image */
  J_COLOR_SPACE_ALPHA jpeg_color_space; /* colorspace of JPEG image */

  /* Decompression processing parameters --- these fields must be set before
   * calling jpeg_start_decompress().  Note that jpeg_read_header() initializes
   * them to default values.
   */

  J_COLOR_SPACE_ALPHA out_color_space; /* colorspace for output */

  unsigned int scale_num, scale_denom; /* fraction by which to scale image */

  double output_gamma;          /* image gamma wanted in output */

  boolean_ALPHA buffered_image;       /* TRUE=multiple output passes */
  boolean_ALPHA raw_data_out;         /* TRUE=downsampled data wanted */

  J_DCT_METHOD_ALPHA dct_method;      /* IDCT algorithm selector */
  boolean_ALPHA do_fancy_upsampling;  /* TRUE=apply fancy upsampling */
  boolean_ALPHA do_block_smoothing;   /* TRUE=apply interblock smoothing */

  boolean_ALPHA quantize_colors;      /* TRUE=colormapped output wanted */
  /* the following are ignored if not quantize_colors: */
  J_DITHER_MODE_ALPHA dither_mode;    /* type of color dithering to use */
  boolean_ALPHA two_pass_quantize;    /* TRUE=use two-pass color quantization */
  int desired_number_of_colors; /* max # colors to use in created colormap */
  /* these are significant only in buffered-image mode: */
  boolean_ALPHA enable_1pass_quant;   /* enable future use of 1-pass quantizer */
  boolean_ALPHA enable_external_quant;/* enable future use of external colormap */
  boolean_ALPHA enable_2pass_quant;   /* enable future use of 2-pass quantizer */

  /* Description of actual output image that will be returned to application.
   * These fields are computed by jpeg_start_decompress().
   * You can also use jpeg_calc_output_dimensions() to determine these values
   * in advance of calling jpeg_start_decompress().
   */

  JDIMENSION_ALPHA output_width;      /* scaled image width */
  JDIMENSION_ALPHA output_height;     /* scaled image height */
  int out_color_components;     /* # of color components in out_color_space */
  int output_components;        /* # of color components returned */
  /* output_components is 1 (a colormap index) when quantizing colors;
   * otherwise it equals out_color_components.
   */
  int rec_outbuf_height;        /* min recommended height of scanline buffer */
  /* If the buffer passed to jpeg_read_scanlines() is less than this many rows
   * high, space and time will be wasted due to unnecessary data copying.
   * Usually rec_outbuf_height will be 1 or 2, at most 4.
   */

  /* When quantizing colors, the output colormap is described by these fields.
   * The application can supply a colormap by setting colormap non-NULL before
   * calling jpeg_start_decompress; otherwise a colormap is created during
   * jpeg_start_decompress or jpeg_start_output.
   * The map has out_color_components rows and actual_number_of_colors columns.
   */
  int actual_number_of_colors;  /* number of entries in use */
  JSAMPARRAY_ALPHA colormap;          /* The color map as a 2-D pixel array */

  /* State variables: these variables indicate the progress of decompression.
   * The application may examine these but must not modify them.
   */

  /* Row index of next scanline to be read from jpeg_read_scanlines().
   * Application may use this to control its processing loop, e.g.,
   * "while (output_scanline < output_height)".
   */
  JDIMENSION_ALPHA output_scanline;   /* 0 .. output_height-1  */

  /* Current input scan number and number of iMCU rows completed in scan.
   * These indicate the progress of the decompressor input side.
   */
  int input_scan_number;        /* Number of SOS markers seen so far */
  JDIMENSION_ALPHA input_iMCU_row;    /* Number of iMCU rows completed */

  /* The "output scan number" is the notional scan being displayed by the
   * output side.  The decompressor will not allow output scan/row number
   * to get ahead of input scan/row, but it can fall arbitrarily far behind.
   */
  int output_scan_number;       /* Nominal scan number being displayed */
  JDIMENSION_ALPHA output_iMCU_row;   /* Number of iMCU rows read */

  /* Current progression status.  coef_bits[c][i] indicates the precision
   * with which component c's DCT coefficient i (in zigzag order) is known.
   * It is -1 when no data has yet been received, otherwise it is the point
   * transform (shift) value for the most recent scan of the coefficient
   * (thus, 0 at completion of the progression).
   * This pointer is NULL when reading a non-progressive file.
   */
  int (*coef_bits)[DCTSIZE2_ALPHA];   /* -1 or current Al value for each coef */

  /* Internal JPEG parameters --- the application usually need not look at
   * these fields.  Note that the decompressor output side may not use
   * any parameters that can change between scans.
   */

  /* Quantization and Huffman tables are carried forward across input
   * datastreams when processing abbreviated JPEG datastreams.
   */

  JQUANT_TBL_ALPHA * quant_tbl_ptrs[NUM_QUANT_TBLS_ALPHA];
  /* ptrs to coefficient quantization tables, or NULL if not defined */

  JHUFF_TBL_ALPHA * dc_huff_tbl_ptrs[NUM_HUFF_TBLS_ALPHA];
  JHUFF_TBL_ALPHA * ac_huff_tbl_ptrs[NUM_HUFF_TBLS_ALPHA];
  /* ptrs to Huffman coding tables, or NULL if not defined */

  /* These parameters are never carried across datastreams, since they
   * are given in SOF/SOS markers or defined to be reset by SOI.
   */

  int data_precision;           /* bits of precision in image data */

  jpeg_component_info_ALPHA * comp_info;
  /* comp_info[i] describes component that appears i'th in SOF */

#if JPEG_LIB_VERSION_ALPHA >= 80
  boolean_ALPHA is_baseline;          /* TRUE if Baseline SOF0 encountered */
#endif
#ifdef REGION_DECODE_M_ALPHA
  boolean_ALPHA tile_decode;          /* TRUE if using tile based decoding */
#endif
  boolean_ALPHA progressive_mode;     /* TRUE if SOFn specifies progressive mode */
  boolean_ALPHA arith_code;           /* TRUE=arithmetic coding, FALSE=Huffman */

  UINT8_ALPHA arith_dc_L[NUM_ARITH_TBLS_ALPHA]; /* L values for DC arith-coding tables */
  UINT8_ALPHA arith_dc_U[NUM_ARITH_TBLS_ALPHA]; /* U values for DC arith-coding tables */
  UINT8_ALPHA arith_ac_K[NUM_ARITH_TBLS_ALPHA]; /* Kx values for AC arith-coding tables */

  unsigned int restart_interval; /* MCUs per restart interval, or 0 for no restart */

  /* These fields record data obtained from optional markers recognized by
   * the JPEG library.
   */
  boolean_ALPHA saw_JFIF_marker;      /* TRUE iff a JFIF APP0 marker was found */
  /* Data copied from JFIF marker; only valid if saw_JFIF_marker is TRUE: */
  UINT8_ALPHA JFIF_major_version;     /* JFIF version number */
  UINT8_ALPHA JFIF_minor_version;
  UINT8_ALPHA density_unit;           /* JFIF code for pixel size units */
  UINT16_ALPHA X_density;             /* Horizontal pixel density */
  UINT16_ALPHA Y_density;             /* Vertical pixel density */
  boolean_ALPHA saw_Adobe_marker;     /* TRUE iff an Adobe APP14 marker was found */
  UINT8_ALPHA Adobe_transform;        /* Color transform code from Adobe marker */

  boolean_ALPHA CCIR601_sampling;     /* TRUE=first samples are cosited */

  /* Aside from the specific data retained from APPn markers known to the
   * library, the uninterpreted contents of any or all APPn and COM markers
   * can be saved in a list for examination by the application.
   */
  jpeg_saved_marker_ptr_ALPHA marker_list; /* Head of list of saved markers */

  /* Remaining fields are known throughout decompressor, but generally
   * should not be touched by a surrounding application.
   */

  /*
   * These fields are computed during decompression startup
   */
  int max_h_samp_factor;        /* largest h_samp_factor */
  int max_v_samp_factor;        /* largest v_samp_factor */

#if JPEG_LIB_VERSION_ALPHA >= 70
  int min_DCT_h_scaled_size;    /* smallest DCT_h_scaled_size of any component */
  int min_DCT_v_scaled_size;    /* smallest DCT_v_scaled_size of any component */
#else
  int min_DCT_scaled_size;      /* smallest DCT_scaled_size of any component */
#endif

  JDIMENSION_ALPHA total_iMCU_rows;   /* # of iMCU rows in image */
  /* The coefficient controller's input and output progress is measured in
   * units of "iMCU" (interleaved MCU) rows.  These are the same as MCU rows
   * in fully interleaved JPEG scans, but are used whether the scan is
   * interleaved or not.  We define an iMCU row as v_samp_factor DCT block
   * rows of each component.  Therefore, the IDCT output contains
   * v_samp_factor*DCT_[v_]scaled_size sample rows of a component per iMCU row.
   */

  JSAMPLE_ALPHA * sample_range_limit; /* table for fast range-limiting */

  /*
   * These fields are valid during any one scan.
   * They describe the components and MCUs actually appearing in the scan.
   * Note that the decompressor output side must not use these fields.
   */
  int comps_in_scan;            /* # of JPEG components in this scan */
  jpeg_component_info_ALPHA * cur_comp_info[MAX_COMPS_IN_SCAN_ALPHA];
  /* *cur_comp_info[i] describes component that appears i'th in SOS */

  JDIMENSION_ALPHA MCUs_per_row;      /* # of MCUs across the image */
  JDIMENSION_ALPHA MCU_rows_in_scan;  /* # of MCU rows in the image */

  int blocks_in_MCU;            /* # of DCT blocks per MCU */
  int MCU_membership[D_MAX_BLOCKS_IN_MCU_ALPHA];
  /* MCU_membership[i] is index in cur_comp_info of component owning */
  /* i'th block in an MCU */

  int Ss, Se, Ah, Al;           /* progressive JPEG parameters for scan */

#if JPEG_LIB_VERSION_ALPHA >= 80
  /* These fields are derived from Se of first SOS marker.
   */
  int block_size;               /* the basic DCT block size: 1..16 */
  const int * natural_order; /* natural-order position array for entropy decode */
  int lim_Se;                   /* min( Se, DCTSIZE2-1 ) for entropy decode */
#endif

  /* This field is shared between entropy decoder and marker parser.
   * It is either zero or the code of a JPEG marker that has been
   * read from the data source, but has not yet been processed.
   */
  int unread_marker;
  unsigned int unread_marker_length;
  boolean_ALPHA fake_eoi;

  /*
   * Links to decompression subobjects (methods, private variables of modules)
   */
  struct jpeg_decomp_master_ALPHA * master;
  struct jpeg_d_main_controller_ALPHA * main;
  struct jpeg_d_coef_controller_ALPHA * coef;
  struct jpeg_d_post_controller_ALPHA * post;
  struct jpeg_input_controller_ALPHA * inputctl;
  struct jpeg_marker_reader_ALPHA * marker;
  struct jpeg_entropy_decoder_ALPHA * entropy;
  struct jpeg_inverse_dct_ALPHA * idct;
  struct jpeg_upsampler_ALPHA * upsample;
  struct jpeg_color_deconverter_ALPHA * cconvert;
  struct jpeg_color_quantizer_ALPHA * cquantize;

#ifdef DECODE_FULL_IMAGE_WITH_REGION_M_ALPHA
  boolean_ALPHA multithread_decode;
  void *multithread_decode_info;
#endif
#ifdef ASYNC_HUFFMAN_BUILDER_ALPHA
  boolean_ALPHA async_index_builder;
#endif
};

#ifdef REGION_DECODE_M_ALPHA

typedef size_t bit_buf_type_ALPHA;    /* type of bit-extraction buffer */

typedef struct {

  // byte offset
  size_t bitstream_offset;
  // bit left
  int bit_left;
  short prev_dc[3];

  // remaining EOBs in EOBRUN
  unsigned short EOBRUN;

  // save the decoder current bit buffer, entropy->bitstate.get_buffer.
  bit_buf_type_ALPHA get_buffer;

  // save the restart info.
  unsigned short restarts_to_go;
  unsigned char next_restart_num;
  
  // patch from mtkjpeg
  boolean_ALPHA insufficient_data;
  int unread_marker;
} huffman_offset_data_ALPHA;

typedef struct {

  // The header starting position of this scan
  size_t bitstream_offset;

  // Number of components in this scan
  int comps_in_scan;

  // Number of MCUs in each row
  int MCUs_per_row;
  int MCU_rows_per_iMCU_row;

  // The last MCU position and its dc value in this scan
  huffman_offset_data_ALPHA prev_MCU_offset;

  huffman_offset_data_ALPHA **offset;
} huffman_scan_header_ALPHA;

#define DEFAULT_MCU_SAMPLE_SIZE_ALPHA 16

typedef struct {

  // The number of MCUs that we sample each time as an index point
  int MCU_sample_size;

  // Number of scan in this image
  int scan_count;

  // Number of iMCUs rows in this image
  int total_iMCU_rows;

  // Memory used by scan struct
  size_t mem_used;
  huffman_scan_header_ALPHA *scan;
#ifdef USE_HUFFMAN_OFFSET_STRUCT_V2_ALPHA
  void *extension;
#endif
} huffman_index_ALPHA;
#endif

/* "Object" declarations for JPEG modules that may be supplied or called
 * directly by the surrounding application.
 * As with all objects in the JPEG library, these structs only define the
 * publicly visible methods and state variables of a module.  Additional
 * private fields may exist after the public ones.
 */


/* Error handler object */

struct jpeg_error_mgr_ALPHA {
  /* Error exit handler: does not return to caller */
  void (*error_exit) (j_common_ptr_ALPHA cinfo);
  /* Conditionally emit a trace or warning message */
  void (*emit_message) (j_common_ptr_ALPHA cinfo, int msg_level);
  /* Routine that actually outputs a trace or error message */
  void (*output_message) (j_common_ptr_ALPHA cinfo);
  /* Format a message string for the most recent JPEG error or message */
  void (*format_message) (j_common_ptr_ALPHA cinfo, char * buffer);
#define JMSG_LENGTH_MAX  200    /* recommended size of format_message buffer */
  /* Reset error state variables at start of a new image */
  void (*reset_error_mgr) (j_common_ptr_ALPHA cinfo);

  /* The message ID code and any parameters are saved here.
   * A message can have one string parameter or up to 8 int parameters.
   */
  int msg_code;
#define JMSG_STR_PARM_MAX_ALPHA  80
  union {
    int i[8];
    char s[JMSG_STR_PARM_MAX_ALPHA];
  } msg_parm;

  /* Standard state variables for error facility */

  int trace_level;              /* max msg_level that will be displayed */

  /* For recoverable corrupt-data errors, we emit a warning message,
   * but keep going unless emit_message chooses to abort.  emit_message
   * should count warnings in num_warnings.  The surrounding application
   * can check for bad data by seeing if num_warnings is nonzero at the
   * end of processing.
   */
  long num_warnings;            /* number of corrupt-data warnings */

  /* These fields point to the table(s) of error message strings.
   * An application can change the table pointer to switch to a different
   * message list (typically, to change the language in which errors are
   * reported).  Some applications may wish to add additional error codes
   * that will be handled by the JPEG library error mechanism; the second
   * table pointer is used for this purpose.
   *
   * First table includes all errors generated by JPEG library itself.
   * Error code 0 is reserved for a "no such error string" message.
   */
  const char * const * jpeg_message_table; /* Library errors */
  int last_jpeg_message;    /* Table contains strings 0..last_jpeg_message */
  /* Second table can be added by application (see cjpeg/djpeg for example).
   * It contains strings numbered first_addon_message..last_addon_message.
   */
  const char * const * addon_message_table; /* Non-library errors */
  int first_addon_message;      /* code for first string in addon table */
  int last_addon_message;       /* code for last string in addon table */
};


/* Progress monitor object */

struct jpeg_progress_mgr_ALPHA {
  void (*progress_monitor) (j_common_ptr_ALPHA cinfo);

  long pass_counter;            /* work units completed in this pass */
  long pass_limit;              /* total number of work units in this pass */
  int completed_passes;         /* passes completed so far */
  int total_passes;             /* total number of passes expected */
};


/* Data destination object for compression */

struct jpeg_destination_mgr_ALPHA {
  JOCTET_ALPHA * next_output_byte;    /* => next byte to write in buffer */
  size_t free_in_buffer;        /* # of byte spaces remaining in buffer */

  void (*init_destination) (j_compress_ptr_ALPHA cinfo);
  boolean_ALPHA (*empty_output_buffer) (j_compress_ptr_ALPHA cinfo);
  void (*term_destination) (j_compress_ptr_ALPHA cinfo);
};


/* Data source object for decompression */

struct jpeg_source_mgr_ALPHA {
  const JOCTET_ALPHA * next_input_byte; /* => next byte to read from buffer */
  size_t bytes_in_buffer;       /* # of bytes remaining in buffer */
#ifdef REGION_DECODE_M_ALPHA
  const JOCTET_ALPHA * start_input_byte; /* => first byte to read from input */
  size_t current_offset; /* current readed input offset */
#endif
  void (*init_source) (j_decompress_ptr_ALPHA cinfo);
  boolean_ALPHA (*fill_input_buffer) (j_decompress_ptr_ALPHA cinfo);
  void (*skip_input_data) (j_decompress_ptr_ALPHA cinfo, long num_bytes);
  boolean_ALPHA (*resync_to_restart) (j_decompress_ptr_ALPHA cinfo, int desired);
  void (*term_source) (j_decompress_ptr_ALPHA cinfo);
#ifdef REGION_DECODE_M_ALPHA
  JMETHOD_ALPHA(boolean_ALPHA, seek_input_data, (j_decompress_ptr_ALPHA cinfo, long byte_offset));
#endif
};


/* Memory manager object.
 * Allocates "small" objects (a few K total), "large" objects (tens of K),
 * and "really big" objects (virtual arrays with backing store if needed).
 * The memory manager does not allow individual objects to be freed; rather,
 * each created object is assigned to a pool, and whole pools can be freed
 * at once.  This is faster and more convenient than remembering exactly what
 * to free, especially where malloc()/free() are not too speedy.
 * NB: alloc routines never return NULL.  They exit to error_exit if not
 * successful.
 */

#define JPOOL_PERMANENT_ALPHA 0       /* lasts until master record is destroyed */
#define JPOOL_IMAGE_ALPHA     1       /* lasts until done with image/datastream */
#define JPOOL_NUMPOOLS_ALPHA  2

typedef struct jvirt_sarray_control_ALPHA * jvirt_sarray_ptr_ALPHA;
typedef struct jvirt_barray_control_ALPHA * jvirt_barray_ptr_ALPHA;


struct jpeg_memory_mgr_ALPHA {
  /* Method pointers */
  void * (*alloc_small) (j_common_ptr_ALPHA cinfo, int pool_id, size_t sizeofobject);
  void * (*alloc_large) (j_common_ptr_ALPHA cinfo, int pool_id,
                         size_t sizeofobject);
  JSAMPARRAY_ALPHA (*alloc_sarray) (j_common_ptr_ALPHA cinfo, int pool_id,
                              JDIMENSION_ALPHA samplesperrow, JDIMENSION_ALPHA numrows);
  JBLOCKARRAY_ALPHA (*alloc_barray) (j_common_ptr_ALPHA cinfo, int pool_id,
                               JDIMENSION_ALPHA blocksperrow, JDIMENSION_ALPHA numrows);
  jvirt_sarray_ptr_ALPHA (*request_virt_sarray) (j_common_ptr_ALPHA cinfo, int pool_id,
                                           boolean_ALPHA pre_zero,
                                           JDIMENSION_ALPHA samplesperrow,
                                           JDIMENSION_ALPHA numrows,
                                           JDIMENSION_ALPHA maxaccess);
  jvirt_barray_ptr_ALPHA (*request_virt_barray) (j_common_ptr_ALPHA cinfo, int pool_id,
                                           boolean_ALPHA pre_zero,
                                           JDIMENSION_ALPHA blocksperrow,
                                           JDIMENSION_ALPHA numrows,
                                           JDIMENSION_ALPHA maxaccess);
  void (*realize_virt_arrays) (j_common_ptr_ALPHA cinfo);
  JSAMPARRAY_ALPHA (*access_virt_sarray) (j_common_ptr_ALPHA cinfo, jvirt_sarray_ptr_ALPHA ptr,
                                    JDIMENSION_ALPHA start_row, JDIMENSION_ALPHA num_rows,
                                    boolean_ALPHA writable);
  JBLOCKARRAY_ALPHA (*access_virt_barray) (j_common_ptr_ALPHA cinfo, jvirt_barray_ptr_ALPHA ptr,
                                     JDIMENSION_ALPHA start_row, JDIMENSION_ALPHA num_rows,
                                     boolean_ALPHA writable);
  void (*free_pool) (j_common_ptr_ALPHA cinfo, int pool_id);
  void (*self_destruct) (j_common_ptr_ALPHA cinfo);

  /* Limit on memory allocation for this JPEG object.  (Note that this is
   * merely advisory, not a guaranteed maximum; it only affects the space
   * used for virtual-array buffers.)  May be changed by outer application
   * after creating the JPEG object.
   */
  long max_memory_to_use;

  /* Maximum allocation request accepted by alloc_large. */
  long max_alloc_chunk;
};

#ifdef ENABLE_JPEG_OAL_ALPHA
struct jpeg_oal_mgr_ALPHA {
  unsigned int version;
  void *interfaces;
  int (*queryHAL) (unsigned int version, unsigned int size, void *interfaces);
};

typedef struct jpeg_oal_mgr_ALPHA *jpeg_oal_ptr_ALPHA;
#endif

/* Routine signature for application-supplied marker processing methods.
 * Need not pass marker code since it is stored in cinfo->unread_marker.
 */
typedef boolean_ALPHA (*jpeg_marker_parser_method_ALPHA) (j_decompress_ptr_ALPHA cinfo);


/* Originally, this macro was used as a way of defining function prototypes
 * for both modern compilers as well as older compilers that did not support
 * prototype parameters.  libjpeg-turbo has never supported these older,
 * non-ANSI compilers, but the macro is still included because there is some
 * software out there that uses it.
 */

#define JPP_ALPHA(arglist)    arglist


/* Default error-management setup */
EXTERN_ALPHA(struct jpeg_error_mgr_ALPHA *) jpeg_std_error_ALPHA (struct jpeg_error_mgr_ALPHA * err);

/* Initialization of JPEG compression objects.
 * jpeg_create_compress() and jpeg_create_decompress() are the exported
 * names that applications should call.  These expand to calls on
 * jpeg_CreateCompress and jpeg_CreateDecompress with additional information
 * passed for version mismatch checking.
 * NB: you must set up the error-manager BEFORE calling jpeg_create_xxx.
 */
#define jpeg_create_compress_ALPHA(cinfo) \
    jpeg_CreateCompress_ALPHA((cinfo), JPEG_LIB_VERSION_ALPHA, \
                        (size_t) sizeof(struct jpeg_compress_struct_ALPHA))
#define jpeg_create_decompress_ALPHA(cinfo) \
    jpeg_CreateDecompress_ALPHA((cinfo), JPEG_LIB_VERSION_ALPHA, \
                          (size_t) sizeof(struct jpeg_decompress_struct_ALPHA))
EXTERN_ALPHA(void) jpeg_CreateCompress_ALPHA (j_compress_ptr_ALPHA cinfo, int version,
                                  size_t structsize);
EXTERN_ALPHA(void) jpeg_CreateDecompress_ALPHA (j_decompress_ptr_ALPHA cinfo, int version,
                                    size_t structsize);
/* Destruction of JPEG compression objects */
EXTERN_ALPHA(void) jpeg_destroy_compress_ALPHA (j_compress_ptr_ALPHA cinfo);
EXTERN_ALPHA(void) jpeg_destroy_decompress_ALPHA (j_decompress_ptr_ALPHA cinfo);

/* Standard data source and destination managers: stdio streams. */
/* Caller is responsible for opening the file before and closing after. */
EXTERN_ALPHA(void) jpeg_stdio_dest_ALPHA (j_compress_ptr_ALPHA cinfo, FILE * outfile);
EXTERN_ALPHA(void) jpeg_stdio_src_ALPHA (j_decompress_ptr_ALPHA cinfo, FILE * infile);

#if JPEG_LIB_VERSION_ALPHA >= 80 || defined(MEM_SRCDST_SUPPORTED_ALPHA)
/* Data source and destination managers: memory buffers. */
EXTERN_ALPHA(void) jpeg_mem_dest_ALPHA (j_compress_ptr_ALPHA cinfo, unsigned char ** outbuffer,
                            unsigned long * outsize);
EXTERN_ALPHA(void) jpeg_mem_src_ALPHA (j_decompress_ptr_ALPHA cinfo, unsigned char * inbuffer,
                           unsigned long insize);
#endif

/* Default parameter setup for compression */
EXTERN_ALPHA(void) jpeg_set_defaults_ALPHA (j_compress_ptr_ALPHA cinfo);
/* Compression parameter setup aids */
EXTERN_ALPHA(void) jpeg_set_colorspace_ALPHA (j_compress_ptr_ALPHA cinfo,
                                  J_COLOR_SPACE_ALPHA colorspace);
EXTERN_ALPHA(void) jpeg_default_colorspace_ALPHA (j_compress_ptr_ALPHA cinfo);
EXTERN_ALPHA(void) jpeg_set_quality_ALPHA (j_compress_ptr_ALPHA cinfo, int quality,
                               boolean_ALPHA force_baseline);
EXTERN_ALPHA(void) jpeg_set_linear_quality_ALPHA (j_compress_ptr_ALPHA cinfo, int scale_factor,
                                      boolean_ALPHA force_baseline);
#if JPEG_LIB_VERSION_ALPHA >= 70
EXTERN_ALPHA(void) jpeg_default_qtables_ALPHA (j_compress_ptr_ALPHA cinfo,
                                   boolean_ALPHA force_baseline);
#endif
EXTERN_ALPHA(void) jpeg_add_quant_table_ALPHA (j_compress_ptr_ALPHA cinfo, int which_tbl,
                                   const unsigned int *basic_table,
                                   int scale_factor, boolean_ALPHA force_baseline);
EXTERN_ALPHA(int) jpeg_quality_scaling_ALPHA (int quality);
EXTERN_ALPHA(void) jpeg_simple_progression_ALPHA (j_compress_ptr_ALPHA cinfo);
EXTERN_ALPHA(void) jpeg_suppress_tables_ALPHA (j_compress_ptr_ALPHA cinfo, boolean_ALPHA suppress);
EXTERN_ALPHA(JQUANT_TBL_ALPHA *) jpeg_alloc_quant_table_ALPHA (j_common_ptr_ALPHA cinfo);
EXTERN_ALPHA(JHUFF_TBL_ALPHA *) jpeg_alloc_huff_table_ALPHA (j_common_ptr_ALPHA cinfo);

/* Main entry points for compression */
EXTERN_ALPHA(void) jpeg_start_compress_ALPHA (j_compress_ptr_ALPHA cinfo,
                                  boolean_ALPHA write_all_tables);
#ifdef REGION_DECODE_M_ALPHA
EXTERN_ALPHA(boolean_ALPHA) jpeg_start_tile_decompress_ALPHA JPP_ALPHA((j_decompress_ptr_ALPHA cinfo));
#endif
EXTERN_ALPHA(JDIMENSION_ALPHA) jpeg_write_scanlines_ALPHA (j_compress_ptr_ALPHA cinfo,
                                         JSAMPARRAY_ALPHA scanlines,
                                         JDIMENSION_ALPHA num_lines);
EXTERN_ALPHA(void) jpeg_finish_compress_ALPHA (j_compress_ptr_ALPHA cinfo);

#if JPEG_LIB_VERSION_ALPHA >= 70
/* Precalculate JPEG dimensions for current compression parameters. */
EXTERN_ALPHA(void) jpeg_calc_jpeg_dimensions_ALPHA (j_compress_ptr_ALPHA cinfo);
#endif

/* Replaces jpeg_write_scanlines when writing raw downsampled data. */
EXTERN_ALPHA(JDIMENSION_ALPHA) jpeg_write_raw_data_ALPHA (j_compress_ptr_ALPHA cinfo, JSAMPIMAGE_ALPHA data,
                                        JDIMENSION_ALPHA num_lines);

/* Write a special marker.  See libjpeg.txt concerning safe usage. */
EXTERN_ALPHA(void) jpeg_write_marker_ALPHA (j_compress_ptr_ALPHA cinfo, int marker,
                                const JOCTET_ALPHA * dataptr, unsigned int datalen);
/* Same, but piecemeal. */
EXTERN_ALPHA(void) jpeg_write_m_header_ALPHA (j_compress_ptr_ALPHA cinfo, int marker,
                                  unsigned int datalen);
EXTERN_ALPHA(void) jpeg_write_m_byte_ALPHA (j_compress_ptr_ALPHA cinfo, int val);

/* Alternate compression function: just write an abbreviated table file */
EXTERN_ALPHA(void) jpeg_write_tables_ALPHA (j_compress_ptr_ALPHA cinfo);

/* Decompression startup: read start of JPEG datastream to see what's there */
EXTERN_ALPHA(int) jpeg_read_header_ALPHA (j_decompress_ptr_ALPHA cinfo, boolean_ALPHA require_image);
/* Return value is one of: */
#define JPEG_SUSPENDED_ALPHA          0 /* Suspended due to lack of input data */
#define JPEG_HEADER_OK_ALPHA          1 /* Found valid image datastream */
#define JPEG_HEADER_TABLES_ONLY_ALPHA 2 /* Found valid table-specs-only datastream */
/* If you pass require_image = TRUE (normal case), you need not check for
 * a TABLES_ONLY return code; an abbreviated file will cause an error exit.
 * JPEG_SUSPENDED is only possible if you use a data source module that can
 * give a suspension return (the stdio source module doesn't).
 */

/* Main entry points for decompression */
EXTERN_ALPHA(boolean_ALPHA) jpeg_start_decompress_ALPHA (j_decompress_ptr_ALPHA cinfo);
EXTERN_ALPHA(JDIMENSION_ALPHA) jpeg_read_scanlines_ALPHA (j_decompress_ptr_ALPHA cinfo,
                                        JSAMPARRAY_ALPHA scanlines,
                                        JDIMENSION_ALPHA max_lines);
EXTERN_ALPHA(JDIMENSION_ALPHA) jpeg_skip_scanlines_ALPHA (j_decompress_ptr_ALPHA cinfo,
                                        JDIMENSION_ALPHA num_lines);
EXTERN_ALPHA(void) jpeg_crop_scanline_ALPHA (j_decompress_ptr_ALPHA cinfo, JDIMENSION_ALPHA *xoffset,
                                 JDIMENSION_ALPHA *width);
EXTERN_ALPHA(boolean_ALPHA) jpeg_finish_decompress_ALPHA (j_decompress_ptr_ALPHA cinfo);

/* Replaces jpeg_read_scanlines when reading raw downsampled data. */
EXTERN_ALPHA(JDIMENSION_ALPHA) jpeg_read_raw_data_ALPHA (j_decompress_ptr_ALPHA cinfo, JSAMPIMAGE_ALPHA data,
                                       JDIMENSION_ALPHA max_lines);
#ifdef REGION_DECODE_M_ALPHA
EXTERN_ALPHA(JDIMENSION_ALPHA) jpeg_read_tile_scanline_ALPHA JPP_ALPHA((j_decompress_ptr_ALPHA cinfo,
                        huffman_index_ALPHA *index,
                        JSAMPARRAY_ALPHA scanlines));
EXTERN_ALPHA(void) jpeg_init_read_tile_scanline_ALPHA JPP_ALPHA((j_decompress_ptr_ALPHA cinfo,
                        huffman_index_ALPHA *index,
                        int *start_x, int *start_y,
                        int *width, int *height));
#endif
/* Additional entry points for buffered-image mode. */
EXTERN_ALPHA(boolean_ALPHA) jpeg_has_multiple_scans_ALPHA (j_decompress_ptr_ALPHA cinfo);
EXTERN_ALPHA(boolean_ALPHA) jpeg_start_output_ALPHA (j_decompress_ptr_ALPHA cinfo, int scan_number);
EXTERN_ALPHA(boolean_ALPHA) jpeg_finish_output_ALPHA (j_decompress_ptr_ALPHA cinfo);
EXTERN_ALPHA(boolean_ALPHA) jpeg_input_complete_ALPHA (j_decompress_ptr_ALPHA cinfo);
EXTERN_ALPHA(void) jpeg_new_colormap_ALPHA (j_decompress_ptr_ALPHA cinfo);
EXTERN_ALPHA(int) jpeg_consume_input_ALPHA (j_decompress_ptr_ALPHA cinfo);
/* Return value is one of: */
/* #define JPEG_SUSPENDED       0    Suspended due to lack of input data */
#define JPEG_REACHED_SOS_ALPHA        1 /* Reached start of new scan */
#define JPEG_REACHED_EOI_ALPHA        2 /* Reached end of image */
#define JPEG_ROW_COMPLETED_ALPHA      3 /* Completed one iMCU row */
#define JPEG_SCAN_COMPLETED_ALPHA     4 /* Completed last iMCU row of a scan */

/* Precalculate output dimensions for current decompression parameters. */
#if JPEG_LIB_VERSION_ALPHA >= 80
EXTERN_ALPHA(void) jpeg_core_output_dimensions_ALPHA (j_decompress_ptr_ALPHA cinfo);
#endif
EXTERN_ALPHA(void) jpeg_calc_output_dimensions_ALPHA (j_decompress_ptr_ALPHA cinfo);

/* Control saving of COM and APPn markers into marker_list. */
EXTERN_ALPHA(void) jpeg_save_markers_ALPHA (j_decompress_ptr_ALPHA cinfo, int marker_code,
                                unsigned int length_limit);

/* Install a special processing method for COM or APPn markers. */
EXTERN_ALPHA(void) jpeg_set_marker_processor_ALPHA (j_decompress_ptr_ALPHA cinfo,
                                        int marker_code,
                                        jpeg_marker_parser_method_ALPHA routine);

/* Read or write raw DCT coefficients --- useful for lossless transcoding. */
EXTERN_ALPHA(jvirt_barray_ptr_ALPHA *) jpeg_read_coefficients_ALPHA (j_decompress_ptr_ALPHA cinfo);
EXTERN_ALPHA(void) jpeg_write_coefficients_ALPHA (j_compress_ptr_ALPHA cinfo,
                                      jvirt_barray_ptr_ALPHA * coef_arrays);
EXTERN_ALPHA(void) jpeg_copy_critical_parameters_ALPHA (j_decompress_ptr_ALPHA srcinfo,
                                            j_compress_ptr_ALPHA dstinfo);

/* If you choose to abort compression or decompression before completing
 * jpeg_finish_(de)compress, then you need to clean up to release memory,
 * temporary files, etc.  You can just call jpeg_destroy_(de)compress
 * if you're done with the JPEG object, but if you want to clean it up and
 * reuse it, call this:
 */
EXTERN_ALPHA(void) jpeg_abort_compress_ALPHA (j_compress_ptr_ALPHA cinfo);
EXTERN_ALPHA(void) jpeg_abort_decompress_ALPHA (j_decompress_ptr_ALPHA cinfo);

/* Generic versions of jpeg_abort and jpeg_destroy that work on either
 * flavor of JPEG object.  These may be more convenient in some places.
 */
EXTERN_ALPHA(void) jpeg_abort_ALPHA (j_common_ptr_ALPHA cinfo);
EXTERN_ALPHA(void) jpeg_destroy_ALPHA (j_common_ptr_ALPHA cinfo);

/* Default restart-marker-resync procedure for use by data source modules */
EXTERN_ALPHA(boolean_ALPHA) jpeg_resync_to_restart_ALPHA (j_decompress_ptr_ALPHA cinfo, int desired);

#ifdef REGION_DECODE_M_ALPHA
EXTERN_ALPHA(boolean_ALPHA) jpeg_build_huffman_index_ALPHA
                        JPP_ALPHA((j_decompress_ptr_ALPHA cinfo, huffman_index_ALPHA *index));
//EXTERN_ALPHA(void) jpeg_configure_huffman_decoder_ALPHA(j_decompress_ptr_ALPHA cinfo,
//                        huffman_offset_data_ALPHA offset);
//EXTERN_ALPHA(void) jpeg_get_huffman_decoder_configuration_ALPHA(j_decompress_ptr_ALPHA cinfo,
//                        huffman_offset_data_ALPHA *offset);
EXTERN_ALPHA(void) jpeg_create_huffman_index_ALPHA(j_decompress_ptr_ALPHA cinfo,
                        huffman_index_ALPHA *index);
EXTERN_ALPHA(void) jpeg_configure_huffman_index_scan_ALPHA(j_decompress_ptr_ALPHA cinfo,
                        huffman_index_ALPHA *index, int scan_no, size_t offset);
EXTERN_ALPHA(void) jpeg_destroy_huffman_index_ALPHA(huffman_index_ALPHA *index);
#endif

#ifdef DECODE_FULL_IMAGE_WITH_REGION_M_DEBUG_OPTION_ALPHA
EXTERN_ALPHA(void) jpeg_configure_decompress_ALPHA(j_decompress_ptr_ALPHA cinfo, JDIMENSION_ALPHA thread_num, JDIMENSION_ALPHA tile_width, JDIMENSION_ALPHA tile_height);
#endif

#ifdef ENABLE_JPEG_OAL_ALPHA
EXTERN_ALPHA(void) jpeg_std_oal_ALPHA(j_decompress_ptr_ALPHA cinfo, jpeg_oal_ptr_ALPHA oal);
#endif

/* These marker codes are exported since applications and data source modules
 * are likely to want to use them.
 */

#define JPEG_RST0_ALPHA       0xD0    /* RST0 marker code */
#define JPEG_EOI_ALPHA        0xD9    /* EOI marker code */
#define JPEG_APP0_ALPHA       0xE0    /* APP0 marker code */
#define JPEG_COM_ALPHA        0xFE    /* COM marker code */


/* If we have a brain-damaged compiler that emits warnings (or worse, errors)
 * for structure definitions that are never filled in, keep it quiet by
 * supplying dummy definitions for the various substructures.
 */

#ifdef INCOMPLETE_TYPES_BROKEN_ALPHA
#ifndef JPEG_INTERNALS_ALPHA          /* will be defined in jpegint.h */
struct jvirt_sarray_control_ALPHA { long dummy; };
struct jvirt_barray_control_ALPHA { long dummy; };
struct jpeg_comp_master_ALPHA { long dummy; };
struct jpeg_c_main_controller_ALPHA { long dummy; };
struct jpeg_c_prep_controller_ALPHA { long dummy; };
struct jpeg_c_coef_controller_ALPHA { long dummy; };
struct jpeg_marker_writer_ALPHA { long dummy; };
struct jpeg_color_converter_ALPHA { long dummy; };
struct jpeg_downsampler_ALPHA { long dummy; };
struct jpeg_forward_dct_ALPHA { long dummy; };
struct jpeg_entropy_encoder_ALPHA { long dummy; };
struct jpeg_decomp_master_ALPHA { long dummy; };
struct jpeg_d_main_controller_ALPHA { long dummy; };
struct jpeg_d_coef_controller_ALPHA { long dummy; };
struct jpeg_d_post_controller_ALPHA { long dummy; };
struct jpeg_input_controller_ALPHA { long dummy; };
struct jpeg_marker_reader_ALPHA { long dummy; };
struct jpeg_entropy_decoder_ALPHA { long dummy; };
struct jpeg_inverse_dct_ALPHA { long dummy; };
struct jpeg_upsampler_ALPHA { long dummy; };
struct jpeg_color_deconverter_ALPHA { long dummy; };
struct jpeg_color_quantizer_ALPHA { long dummy; };
#endif /* JPEG_INTERNALS */
#endif /* INCOMPLETE_TYPES_BROKEN */


/*
 * The JPEG library modules define JPEG_INTERNALS before including this file.
 * The internal structure declarations are read only when that is true.
 * Applications using the library should not include jpegint.h, but may wish
 * to include jerror.h.
 */

#ifdef JPEG_INTERNALS_ALPHA
#include "jpegint_alpha.h"            /* fetch private declarations */
#include "jerror_alpha.h"             /* fetch error codes too */
#endif

#ifdef __cplusplus
#ifndef DONT_USE_EXTERN_C
}
#endif
#endif

#endif /* JPEGLIB_H */
