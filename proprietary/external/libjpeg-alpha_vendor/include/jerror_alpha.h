/*
 * jerror.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * Modified 1997-2009 by Guido Vollbeding.
 * libjpeg-turbo Modifications:
 * Copyright (C) 2014, D. R. Commander.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file defines the error and message codes for the JPEG library.
 * Edit this file to add new codes, or to translate the message strings to
 * some other language.
 * A set of error-reporting macros are defined too.  Some applications using
 * the JPEG library may wish to include this file to get the error codes
 * and/or the macros.
 */

/*
 * To define the enum list of message codes, include this file without
 * defining macro JMESSAGE.  To create a message string table, include it
 * again with a suitable JMESSAGE definition (see jerror.c for an example).
 */
#ifndef JMESSAGE_ALPHA
#ifndef JERROR_H_ALPHA
/* First time through, define the enum list */
#define JMAKE_ENUM_LIST_ALPHA
#else
/* Repeated inclusions of this file are no-ops unless JMESSAGE is defined */
#define JMESSAGE_ALPHA(code,string)
#endif /* JERROR_H */
#endif /* JMESSAGE */

#ifdef JMAKE_ENUM_LIST_ALPHA

typedef enum {

#define JMESSAGE_ALPHA(code,string)   code ,

#endif /* JMAKE_ENUM_LIST */

JMESSAGE_ALPHA(JMSG_NOMESSAGE_ALPHA, "Bogus message code %d") /* Must be first entry! */

/* For maintenance convenience, list is alphabetical by message code name */
#if JPEG_LIB_VERSION_ALPHA < 70
JMESSAGE_ALPHA(JERR_ARITH_NOTIMPL_ALPHA,
         "Sorry, arithmetic coding is not implemented")
#endif
JMESSAGE_ALPHA(JERR_BAD_ALIGN_TYPE_ALPHA, "ALIGN_TYPE is wrong, please fix")
JMESSAGE_ALPHA(JERR_BAD_ALLOC_CHUNK_ALPHA, "MAX_ALLOC_CHUNK is wrong, please fix")
JMESSAGE_ALPHA(JERR_BAD_BUFFER_MODE_ALPHA, "Bogus buffer control mode")
JMESSAGE_ALPHA(JERR_BAD_COMPONENT_ID_ALPHA, "Invalid component ID %d in SOS")
#if JPEG_LIB_VERSION_ALPHA >= 70
JMESSAGE_ALPHA(JERR_BAD_CROP_SPEC_ALPHA, "Invalid crop request")
#endif
JMESSAGE_ALPHA(JERR_BAD_DCT_COEF_ALPHA, "DCT coefficient out of range")
JMESSAGE_ALPHA(JERR_BAD_DCTSIZE_ALPHA, "IDCT output block size %d not supported")
#if JPEG_LIB_VERSION_ALPHA >= 70
JMESSAGE_ALPHA(JERR_BAD_DROP_SAMPLING_ALPHA,
         "Component index %d: mismatching sampling ratio %d:%d, %d:%d, %c")
#endif
JMESSAGE_ALPHA(JERR_BAD_HUFF_TABLE_ALPHA, "Bogus Huffman table definition")
JMESSAGE_ALPHA(JERR_BAD_IN_COLORSPACE_ALPHA, "Bogus input colorspace")
JMESSAGE_ALPHA(JERR_BAD_J_COLORSPACE_ALPHA, "Bogus JPEG colorspace")
JMESSAGE_ALPHA(JERR_BAD_LENGTH_ALPHA, "Bogus marker length")
JMESSAGE_ALPHA(JERR_BAD_LIB_VERSION_ALPHA,
         "Wrong JPEG library version: library is %d, caller expects %d")
JMESSAGE_ALPHA(JERR_BAD_MCU_SIZE_ALPHA, "Sampling factors too large for interleaved scan")
JMESSAGE_ALPHA(JERR_BAD_POOL_ID_ALPHA, "Invalid memory pool code %d")
JMESSAGE_ALPHA(JERR_BAD_PRECISION_ALPHA, "Unsupported JPEG data precision %d")
JMESSAGE_ALPHA(JERR_BAD_PROGRESSION_ALPHA,
         "Invalid progressive parameters Ss=%d Se=%d Ah=%d Al=%d")
JMESSAGE_ALPHA(JERR_BAD_PROG_SCRIPT_ALPHA,
         "Invalid progressive parameters at scan script entry %d")
JMESSAGE_ALPHA(JERR_BAD_SAMPLING_ALPHA, "Bogus sampling factors")
JMESSAGE_ALPHA(JERR_BAD_SCAN_SCRIPT_ALPHA, "Invalid scan script at entry %d")
JMESSAGE_ALPHA(JERR_BAD_STATE_ALPHA, "Improper call to JPEG library in state %d")
JMESSAGE_ALPHA(JERR_BAD_STRUCT_SIZE_ALPHA,
         "JPEG parameter struct mismatch: library thinks size is %u, caller expects %u")
JMESSAGE_ALPHA(JERR_BAD_VIRTUAL_ACCESS_ALPHA, "Bogus virtual array access")
JMESSAGE_ALPHA(JERR_BUFFER_SIZE_ALPHA, "Buffer passed to JPEG library is too small")
JMESSAGE_ALPHA(JERR_CANT_SUSPEND_ALPHA, "Suspension not allowed here")
JMESSAGE_ALPHA(JERR_CCIR601_NOTIMPL_ALPHA, "CCIR601 sampling not implemented yet")
JMESSAGE_ALPHA(JERR_COMPONENT_COUNT_ALPHA, "Too many color components: %d, max %d")
JMESSAGE_ALPHA(JERR_CONVERSION_NOTIMPL_ALPHA, "Unsupported color conversion request")
JMESSAGE_ALPHA(JERR_DAC_INDEX_ALPHA, "Bogus DAC index %d")
JMESSAGE_ALPHA(JERR_DAC_VALUE_ALPHA, "Bogus DAC value 0x%x")
JMESSAGE_ALPHA(JERR_DHT_INDEX_ALPHA, "Bogus DHT index %d")
JMESSAGE_ALPHA(JERR_DQT_INDEX_ALPHA, "Bogus DQT index %d")
JMESSAGE_ALPHA(JERR_EMPTY_IMAGE_ALPHA, "Empty JPEG image (DNL not supported)")
JMESSAGE_ALPHA(JERR_EMS_READ_ALPHA, "Read from EMS failed")
JMESSAGE_ALPHA(JERR_EMS_WRITE_ALPHA, "Write to EMS failed")
JMESSAGE_ALPHA(JERR_EOI_EXPECTED_ALPHA, "Didn't expect more than one scan")
JMESSAGE_ALPHA(JERR_FILE_READ_ALPHA, "Input file read error")
JMESSAGE_ALPHA(JERR_FILE_WRITE_ALPHA, "Output file write error --- out of disk space?")
JMESSAGE_ALPHA(JERR_FRACT_SAMPLE_NOTIMPL_ALPHA, "Fractional sampling not implemented yet")
JMESSAGE_ALPHA(JERR_HUFF_CLEN_OVERFLOW_ALPHA, "Huffman code size table overflow")
JMESSAGE_ALPHA(JERR_HUFF_MISSING_CODE_ALPHA, "Missing Huffman code table entry")
JMESSAGE_ALPHA(JERR_IMAGE_TOO_BIG_ALPHA, "Maximum supported image dimension is %u pixels")
JMESSAGE_ALPHA(JERR_INPUT_EMPTY_ALPHA, "Empty input file")
JMESSAGE_ALPHA(JERR_INPUT_EOF_ALPHA, "Premature end of input file")
JMESSAGE_ALPHA(JERR_MISMATCHED_QUANT_TABLE_ALPHA,
         "Cannot transcode due to multiple use of quantization table %d")
JMESSAGE_ALPHA(JERR_MISSING_DATA_ALPHA, "Scan script does not transmit all data")
JMESSAGE_ALPHA(JERR_MODE_CHANGE_ALPHA, "Invalid color quantization mode change")
JMESSAGE_ALPHA(JERR_NOTIMPL_ALPHA, "Not implemented yet")
JMESSAGE_ALPHA(JERR_NOT_COMPILED_ALPHA, "Requested feature was omitted at compile time")
#if JPEG_LIB_VERSION_ALPHA >= 70
JMESSAGE_ALPHA(JERR_NO_ARITH_TABLE_ALPHA, "Arithmetic table 0x%02x was not defined")
#endif
JMESSAGE_ALPHA(JERR_NO_BACKING_STORE_ALPHA, "Backing store not supported")
JMESSAGE_ALPHA(JERR_NO_HUFF_TABLE_ALPHA, "Huffman table 0x%02x was not defined")
JMESSAGE_ALPHA(JERR_NO_IMAGE_ALPHA, "JPEG datastream contains no image")
JMESSAGE_ALPHA(JERR_NO_QUANT_TABLE_ALPHA, "Quantization table 0x%02x was not defined")
JMESSAGE_ALPHA(JERR_NO_SOI_ALPHA, "Not a JPEG file: starts with 0x%02x 0x%02x")
JMESSAGE_ALPHA(JERR_OUT_OF_MEMORY_ALPHA, "Insufficient memory (case %d)")
JMESSAGE_ALPHA(JERR_QUANT_COMPONENTS_ALPHA,
         "Cannot quantize more than %d color components")
JMESSAGE_ALPHA(JERR_QUANT_FEW_COLORS_ALPHA, "Cannot quantize to fewer than %d colors")
JMESSAGE_ALPHA(JERR_QUANT_MANY_COLORS_ALPHA, "Cannot quantize to more than %d colors")
JMESSAGE_ALPHA(JERR_SOF_DUPLICATE_ALPHA, "Invalid JPEG file structure: two SOF markers")
JMESSAGE_ALPHA(JERR_SOF_NO_SOS_ALPHA, "Invalid JPEG file structure: missing SOS marker")
JMESSAGE_ALPHA(JERR_SOF_UNSUPPORTED_ALPHA, "Unsupported JPEG process: SOF type 0x%02x")
JMESSAGE_ALPHA(JERR_SOI_DUPLICATE_ALPHA, "Invalid JPEG file structure: two SOI markers")
JMESSAGE_ALPHA(JERR_SOS_NO_SOF_ALPHA, "Invalid JPEG file structure: SOS before SOF")
JMESSAGE_ALPHA(JERR_TFILE_CREATE_ALPHA, "Failed to create temporary file %s")
JMESSAGE_ALPHA(JERR_TFILE_READ_ALPHA, "Read failed on temporary file")
JMESSAGE_ALPHA(JERR_TFILE_SEEK_ALPHA, "Seek failed on temporary file")
JMESSAGE_ALPHA(JERR_TFILE_WRITE_ALPHA,
         "Write failed on temporary file --- out of disk space?")
JMESSAGE_ALPHA(JERR_TOO_LITTLE_DATA_ALPHA, "Application transferred too few scanlines")
JMESSAGE_ALPHA(JERR_UNKNOWN_MARKER_ALPHA, "Unsupported marker type 0x%02x")
JMESSAGE_ALPHA(JERR_VIRTUAL_BUG_ALPHA, "Virtual array controller messed up")
JMESSAGE_ALPHA(JERR_WIDTH_OVERFLOW_ALPHA, "Image too wide for this implementation")
JMESSAGE_ALPHA(JERR_XMS_READ_ALPHA, "Read from XMS failed")
JMESSAGE_ALPHA(JERR_XMS_WRITE_ALPHA, "Write to XMS failed")
JMESSAGE_ALPHA(JMSG_COPYRIGHT_ALPHA, JCOPYRIGHT_SHORT_ALPHA)
JMESSAGE_ALPHA(JMSG_VERSION_ALPHA, JVERSION_ALPHA)
JMESSAGE_ALPHA(JTRC_16BIT_TABLES_ALPHA,
         "Caution: quantization tables are too coarse for baseline JPEG")
JMESSAGE_ALPHA(JTRC_ADOBE_ALPHA,
         "Adobe APP14 marker: version %d, flags 0x%04x 0x%04x, transform %d")
JMESSAGE_ALPHA(JTRC_APP0_ALPHA, "Unknown APP0 marker (not JFIF), length %u")
JMESSAGE_ALPHA(JTRC_APP14_ALPHA, "Unknown APP14 marker (not Adobe), length %u")
JMESSAGE_ALPHA(JTRC_DAC_ALPHA, "Define Arithmetic Table 0x%02x: 0x%02x")
JMESSAGE_ALPHA(JTRC_DHT_ALPHA, "Define Huffman Table 0x%02x")
JMESSAGE_ALPHA(JTRC_DQT_ALPHA, "Define Quantization Table %d  precision %d")
JMESSAGE_ALPHA(JTRC_DRI_ALPHA, "Define Restart Interval %u")
JMESSAGE_ALPHA(JTRC_EMS_CLOSE_ALPHA, "Freed EMS handle %u")
JMESSAGE_ALPHA(JTRC_EMS_OPEN_ALPHA, "Obtained EMS handle %u")
JMESSAGE_ALPHA(JTRC_EOI_ALPHA, "End Of Image")
JMESSAGE_ALPHA(JTRC_HUFFBITS_ALPHA, "        %3d %3d %3d %3d %3d %3d %3d %3d")
JMESSAGE_ALPHA(JTRC_JFIF_ALPHA, "JFIF APP0 marker: version %d.%02d, density %dx%d  %d")
JMESSAGE_ALPHA(JTRC_JFIF_BADTHUMBNAILSIZE_ALPHA,
         "Warning: thumbnail image size does not match data length %u")
JMESSAGE_ALPHA(JTRC_JFIF_EXTENSION_ALPHA,
         "JFIF extension marker: type 0x%02x, length %u")
JMESSAGE_ALPHA(JTRC_JFIF_THUMBNAIL_ALPHA, "    with %d x %d thumbnail image")
JMESSAGE_ALPHA(JTRC_MISC_MARKER_ALPHA, "Miscellaneous marker 0x%02x, length %u")
JMESSAGE_ALPHA(JTRC_PARMLESS_MARKER_ALPHA, "Unexpected marker 0x%02x")
JMESSAGE_ALPHA(JTRC_QUANTVALS_ALPHA, "        %4u %4u %4u %4u %4u %4u %4u %4u")
JMESSAGE_ALPHA(JTRC_QUANT_3_NCOLORS_ALPHA, "Quantizing to %d = %d*%d*%d colors")
JMESSAGE_ALPHA(JTRC_QUANT_NCOLORS_ALPHA, "Quantizing to %d colors")
JMESSAGE_ALPHA(JTRC_QUANT_SELECTED_ALPHA, "Selected %d colors for quantization")
JMESSAGE_ALPHA(JTRC_RECOVERY_ACTION_ALPHA, "At marker 0x%02x, recovery action %d")
JMESSAGE_ALPHA(JTRC_RST_ALPHA, "RST%d")
JMESSAGE_ALPHA(JTRC_SMOOTH_NOTIMPL_ALPHA,
         "Smoothing not supported with nonstandard sampling ratios")
JMESSAGE_ALPHA(JTRC_SOF_ALPHA, "Start Of Frame 0x%02x: width=%u, height=%u, components=%d")
JMESSAGE_ALPHA(JTRC_SOF_COMPONENT_ALPHA, "    Component %d: %dhx%dv q=%d")
JMESSAGE_ALPHA(JTRC_SOI_ALPHA, "Start of Image")
JMESSAGE_ALPHA(JTRC_SOS_ALPHA, "Start Of Scan: %d components")
JMESSAGE_ALPHA(JTRC_SOS_COMPONENT_ALPHA, "    Component %d: dc=%d ac=%d")
JMESSAGE_ALPHA(JTRC_SOS_PARAMS_ALPHA, "  Ss=%d, Se=%d, Ah=%d, Al=%d")
JMESSAGE_ALPHA(JTRC_TFILE_CLOSE_ALPHA, "Closed temporary file %s")
JMESSAGE_ALPHA(JTRC_TFILE_OPEN_ALPHA, "Opened temporary file %s")
JMESSAGE_ALPHA(JTRC_THUMB_JPEG_ALPHA,
         "JFIF extension marker: JPEG-compressed thumbnail image, length %u")
JMESSAGE_ALPHA(JTRC_THUMB_PALETTE_ALPHA,
         "JFIF extension marker: palette thumbnail image, length %u")
JMESSAGE_ALPHA(JTRC_THUMB_RGB_ALPHA,
         "JFIF extension marker: RGB thumbnail image, length %u")
JMESSAGE_ALPHA(JTRC_UNKNOWN_IDS_ALPHA,
         "Unrecognized component IDs %d %d %d, assuming YCbCr")
JMESSAGE_ALPHA(JTRC_XMS_CLOSE_ALPHA, "Freed XMS handle %u")
JMESSAGE_ALPHA(JTRC_XMS_OPEN_ALPHA, "Obtained XMS handle %u")
JMESSAGE_ALPHA(JWRN_ADOBE_XFORM_ALPHA, "Unknown Adobe color transform code %d")
#if JPEG_LIB_VERSION_ALPHA >= 70
JMESSAGE_ALPHA(JWRN_ARITH_BAD_CODE_ALPHA, "Corrupt JPEG data: bad arithmetic code")
#endif
JMESSAGE_ALPHA(JWRN_BOGUS_PROGRESSION_ALPHA,
         "Inconsistent progression sequence for component %d coefficient %d")
JMESSAGE_ALPHA(JWRN_EXTRANEOUS_DATA_ALPHA,
         "Corrupt JPEG data: %u extraneous bytes before marker 0x%02x")
JMESSAGE_ALPHA(JWRN_HIT_MARKER_ALPHA, "Corrupt JPEG data: premature end of data segment")
JMESSAGE_ALPHA(JWRN_HUFF_BAD_CODE_ALPHA, "Corrupt JPEG data: bad Huffman code")
JMESSAGE_ALPHA(JWRN_JFIF_MAJOR_ALPHA, "Warning: unknown JFIF revision number %d.%02d")
JMESSAGE_ALPHA(JWRN_JPEG_EOF_ALPHA, "Premature end of JPEG file")
JMESSAGE_ALPHA(JWRN_MUST_RESYNC_ALPHA,
         "Corrupt JPEG data: found marker 0x%02x instead of RST%d")
JMESSAGE_ALPHA(JWRN_NOT_SEQUENTIAL_ALPHA, "Invalid SOS parameters for sequential JPEG")
JMESSAGE_ALPHA(JWRN_TOO_MUCH_DATA_ALPHA, "Application transferred too many scanlines")
#if JPEG_LIB_VERSION_ALPHA < 70
JMESSAGE_ALPHA(JERR_BAD_CROP_SPEC_ALPHA, "Invalid crop request")
#if defined(C_ARITH_CODING_SUPPORTED_ALPHA) || defined(D_ARITH_CODING_SUPPORTED_ALPHA)
JMESSAGE_ALPHA(JERR_NO_ARITH_TABLE_ALPHA, "Arithmetic table 0x%02x was not defined")
JMESSAGE_ALPHA(JWRN_ARITH_BAD_CODE_ALPHA, "Corrupt JPEG data: bad arithmetic code")
#endif
#endif

#ifdef JMAKE_ENUM_LIST_ALPHA

  JMSG_LASTMSGCODE_ALPHA
} J_MESSAGE_CODE_ALPHA;

#undef JMAKE_ENUM_LIST_ALPHA
#endif /* JMAKE_ENUM_LIST */

/* Zap JMESSAGE macro so that future re-inclusions do nothing by default */
#undef JMESSAGE_ALPHA


#ifndef JERROR_H_ALPHA
#define JERROR_H_ALPHA

/* Macros to simplify using the error and trace message stuff */
/* The first parameter is either type of cinfo pointer */

/* Fatal errors (print message and exit) */
#define ERREXIT_ALPHA(cinfo,code)  \
  ((cinfo)->err->msg_code = (code), \
   (*(cinfo)->err->error_exit) ((j_common_ptr_ALPHA) (cinfo)))
#define ERREXIT1_ALPHA(cinfo,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (*(cinfo)->err->error_exit) ((j_common_ptr_ALPHA) (cinfo)))
#define ERREXIT2_ALPHA(cinfo,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (*(cinfo)->err->error_exit) ((j_common_ptr_ALPHA) (cinfo)))
#define ERREXIT3_ALPHA(cinfo,code,p1,p2,p3)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (cinfo)->err->msg_parm.i[2] = (p3), \
   (*(cinfo)->err->error_exit) ((j_common_ptr_ALPHA) (cinfo)))
#define ERREXIT4_ALPHA(cinfo,code,p1,p2,p3,p4)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (cinfo)->err->msg_parm.i[2] = (p3), \
   (cinfo)->err->msg_parm.i[3] = (p4), \
   (*(cinfo)->err->error_exit) ((j_common_ptr_ALPHA) (cinfo)))
#define ERREXITS_ALPHA(cinfo,code,str)  \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, (str), JMSG_STR_PARM_MAX_ALPHA), \
   (*(cinfo)->err->error_exit) ((j_common_ptr_ALPHA) (cinfo)))

#define MAKESTMT_ALPHA(stuff)         do { stuff } while (0)

/* Nonfatal errors (we can keep going, but the data is probably corrupt) */
#define WARNMS_ALPHA(cinfo,code)  \
  ((cinfo)->err->msg_code = (code), \
   (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), -1))
#define WARNMS1_ALPHA(cinfo,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), -1))
#define WARNMS2_ALPHA(cinfo,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), -1))

/* Informational/debugging messages */
#define TRACEMS_ALPHA(cinfo,lvl,code)  \
  ((cinfo)->err->msg_code = (code), \
   (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)))
#define TRACEMS1_ALPHA(cinfo,lvl,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)))
#define TRACEMS2_ALPHA(cinfo,lvl,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)))
#define TRACEMS3_ALPHA(cinfo,lvl,code,p1,p2,p3)  \
  MAKESTMT_ALPHA(int * _mp = (cinfo)->err->msg_parm.i; \
           _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); \
           (cinfo)->err->msg_code = (code); \
           (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)); )
#define TRACEMS4_ALPHA(cinfo,lvl,code,p1,p2,p3,p4)  \
  MAKESTMT_ALPHA(int * _mp = (cinfo)->err->msg_parm.i; \
           _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
           (cinfo)->err->msg_code = (code); \
           (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)); )
#define TRACEMS5_ALPHA(cinfo,lvl,code,p1,p2,p3,p4,p5)  \
  MAKESTMT_ALPHA(int * _mp = (cinfo)->err->msg_parm.i; \
           _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
           _mp[4] = (p5); \
           (cinfo)->err->msg_code = (code); \
           (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)); )
#define TRACEMS8_ALPHA(cinfo,lvl,code,p1,p2,p3,p4,p5,p6,p7,p8)  \
  MAKESTMT_ALPHA(int * _mp = (cinfo)->err->msg_parm.i; \
           _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
           _mp[4] = (p5); _mp[5] = (p6); _mp[6] = (p7); _mp[7] = (p8); \
           (cinfo)->err->msg_code = (code); \
           (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)); )
#define TRACEMSS_ALPHA(cinfo,lvl,code,str)  \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, (str), JMSG_STR_PARM_MAX_ALPHA), \
   (*(cinfo)->err->emit_message) ((j_common_ptr_ALPHA) (cinfo), (lvl)))

#endif /* JERROR_H */
