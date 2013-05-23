/*
 * jerror.h
 *
 * Copyright (C) 1994-1997, Thomas G. Lane.
 * Modified 1997-2009 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
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
#ifndef JMESSAGE
#ifndef JERROR_H
/* First time through, define the enum list */
#define JMAKE_ENUM_LIST
#else
/* Repeated inclusions of this file are no-ops unless JMESSAGE is defined */
#define JMESSAGE(code,string)
#endif /* JERROR_H */
#endif /* JMESSAGE */

#ifdef JMAKE_ENUM_LIST

typedef enum {

#define JMESSAGE(code,string)	code ,

#endif /* JMAKE_ENUM_LIST */

JMESSAGE(JMSG_NOMESSAGE, (const char*)"Bogus message code %d") /* Must be first entry! */

/* For maintenance convenience, list is alphabetical by message code name */
JMESSAGE(JERR_BAD_ALIGN_TYPE, (const char*)"ALIGN_TYPE is wrong, please fix")
JMESSAGE(JERR_BAD_ALLOC_CHUNK, (const char*)"MAX_ALLOC_CHUNK is wrong, please fix")
JMESSAGE(JERR_BAD_BUFFER_MODE, (const char*)"Bogus buffer control mode")
JMESSAGE(JERR_BAD_COMPONENT_ID, (const char*)"Invalid component ID %d in SOS")
JMESSAGE(JERR_BAD_CROP_SPEC, (const char*)"Invalid crop request")
JMESSAGE(JERR_BAD_DCT_COEF, (const char*)"DCT coefficient out of range")
JMESSAGE(JERR_BAD_DCTSIZE, (const char*)"DCT scaled block size %dx%d not supported")
JMESSAGE(JERR_BAD_DROP_SAMPLING,
	 (const char*)"Component index %d: mismatching sampling ratio %d:%d, %d:%d, %c")
JMESSAGE(JERR_BAD_HUFF_TABLE, (const char*)"Bogus Huffman table definition")
JMESSAGE(JERR_BAD_IN_COLORSPACE, (const char*)"Bogus input colorspace")
JMESSAGE(JERR_BAD_J_COLORSPACE, (const char*)"Bogus JPEG colorspace")
JMESSAGE(JERR_BAD_LENGTH, (const char*)"Bogus marker length")
JMESSAGE(JERR_BAD_LIB_VERSION,
	 (const char*)"Wrong JPEG library version: library is %d, caller expects %d")
JMESSAGE(JERR_BAD_MCU_SIZE, (const char*)"Sampling factors too large for interleaved scan")
JMESSAGE(JERR_BAD_POOL_ID, (const char*)"Invalid memory pool code %d")
JMESSAGE(JERR_BAD_PRECISION, (const char*)"Unsupported JPEG data precision %d")
JMESSAGE(JERR_BAD_PROGRESSION,
	 (const char*)"Invalid progressive parameters Ss=%d Se=%d Ah=%d Al=%d")
JMESSAGE(JERR_BAD_PROG_SCRIPT,
	 (const char*)"Invalid progressive parameters at scan script entry %d")
JMESSAGE(JERR_BAD_SAMPLING, (const char*)"Bogus sampling factors")
JMESSAGE(JERR_BAD_SCAN_SCRIPT, (const char*)"Invalid scan script at entry %d")
JMESSAGE(JERR_BAD_STATE, (const char*)"Improper call to JPEG library in state %d")
JMESSAGE(JERR_BAD_STRUCT_SIZE,
	 (const char*)"JPEG parameter struct mismatch: library thinks size is %u, caller expects %u")
JMESSAGE(JERR_BAD_VIRTUAL_ACCESS, (const char*)"Bogus virtual array access")
JMESSAGE(JERR_BUFFER_SIZE, (const char*)"Buffer passed to JPEG library is too small")
JMESSAGE(JERR_CANT_SUSPEND, (const char*)"Suspension not allowed here")
JMESSAGE(JERR_CCIR601_NOTIMPL, (const char*)"CCIR601 sampling not implemented yet")
JMESSAGE(JERR_COMPONENT_COUNT, (const char*)"Too many color components: %d, max %d")
JMESSAGE(JERR_CONVERSION_NOTIMPL, "Unsupported color conversion request")
JMESSAGE(JERR_DAC_INDEX, (const char*)"Bogus DAC index %d")
JMESSAGE(JERR_DAC_VALUE, (const char*)"Bogus DAC value 0x%x")
JMESSAGE(JERR_DHT_INDEX, (const char*)"Bogus DHT index %d")
JMESSAGE(JERR_DQT_INDEX, (const char*)"Bogus DQT index %d")
JMESSAGE(JERR_EMPTY_IMAGE, (const char*)"Empty JPEG image (DNL not supported)")
JMESSAGE(JERR_EMS_READ, (const char*)"Read from EMS failed")
JMESSAGE(JERR_EMS_WRITE, (const char*)"Write to EMS failed")
JMESSAGE(JERR_EOI_EXPECTED, (const char*)"Didn't expect more than one scan")
JMESSAGE(JERR_FILE_READ, (const char*)"Input file read error")
JMESSAGE(JERR_FILE_WRITE, (const char*)"Output file write error --- out of disk space?")
JMESSAGE(JERR_FRACT_SAMPLE_NOTIMPL, (const char*)"Fractional sampling not implemented yet")
JMESSAGE(JERR_HUFF_CLEN_OVERFLOW, (const char*)"Huffman code size table overflow")
JMESSAGE(JERR_HUFF_MISSING_CODE, (const char*)"Missing Huffman code table entry")
JMESSAGE(JERR_IMAGE_TOO_BIG, (const char*)"Maximum supported image dimension is %u pixels")
JMESSAGE(JERR_INPUT_EMPTY, (const char*)"Empty input file")
JMESSAGE(JERR_INPUT_EOF, (const char*)"Premature end of input file")
JMESSAGE(JERR_MISMATCHED_QUANT_TABLE,
	 (const char*)"Cannot transcode due to multiple use of quantization table %d")
JMESSAGE(JERR_MISSING_DATA, (const char*)"Scan script does not transmit all data")
JMESSAGE(JERR_MODE_CHANGE, (const char*)"Invalid color quantization mode change")
JMESSAGE(JERR_NOTIMPL, (const char*)"Not implemented yet")
JMESSAGE(JERR_NOT_COMPILED, (const char*)"Requested feature was omitted at compile time")
JMESSAGE(JERR_NO_ARITH_TABLE, (const char*)"Arithmetic table 0x%02x was not defined")
JMESSAGE(JERR_NO_BACKING_STORE, (const char*)"Backing store not supported")
JMESSAGE(JERR_NO_HUFF_TABLE, (const char*)"Huffman table 0x%02x was not defined")
JMESSAGE(JERR_NO_IMAGE, (const char*)"JPEG datastream contains no image")
JMESSAGE(JERR_NO_QUANT_TABLE, (const char*)"Quantization table 0x%02x was not defined")
JMESSAGE(JERR_NO_SOI, (const char*)"Not a JPEG file: starts with 0x%02x 0x%02x")
JMESSAGE(JERR_OUT_OF_MEMORY, (const char*)"Insufficient memory (case %d)")
JMESSAGE(JERR_QUANT_COMPONENTS,
	 (const char*)"Cannot quantize more than %d color components")
JMESSAGE(JERR_QUANT_FEW_COLORS, (const char*)"Cannot quantize to fewer than %d colors")
JMESSAGE(JERR_QUANT_MANY_COLORS, (const char*)"Cannot quantize to more than %d colors")
JMESSAGE(JERR_SOF_DUPLICATE, (const char*)"Invalid JPEG file structure: two SOF markers")
JMESSAGE(JERR_SOF_NO_SOS, (const char*)"Invalid JPEG file structure: missing SOS marker")
JMESSAGE(JERR_SOF_UNSUPPORTED, (const char*)"Unsupported JPEG process: SOF type 0x%02x")
JMESSAGE(JERR_SOI_DUPLICATE, (const char*)"Invalid JPEG file structure: two SOI markers")
JMESSAGE(JERR_SOS_NO_SOF, (const char*)"Invalid JPEG file structure: SOS before SOF")
JMESSAGE(JERR_TFILE_CREATE, (const char*)"Failed to create temporary file %s")
JMESSAGE(JERR_TFILE_READ, (const char*)"Read failed on temporary file")
JMESSAGE(JERR_TFILE_SEEK, (const char*)"Seek failed on temporary file")
JMESSAGE(JERR_TFILE_WRITE,
	 (const char*)"Write failed on temporary file --- out of disk space?")
JMESSAGE(JERR_TOO_LITTLE_DATA, (const char*)"Application transferred too few scanlines")
JMESSAGE(JERR_UNKNOWN_MARKER, (const char*)"Unsupported marker type 0x%02x")
JMESSAGE(JERR_VIRTUAL_BUG, (const char*)"Virtual array controller messed up")
JMESSAGE(JERR_WIDTH_OVERFLOW, (const char*)"Image too wide for this implementation")
JMESSAGE(JERR_XMS_READ, (const char*)"Read from XMS failed")
JMESSAGE(JERR_XMS_WRITE, (const char*)"Write to XMS failed")
JMESSAGE(JMSG_COPYRIGHT, JCOPYRIGHT)
JMESSAGE(JMSG_VERSION, JVERSION)
JMESSAGE(JTRC_16BIT_TABLES,
	 (const char*)"Caution: quantization tables are too coarse for baseline JPEG")
JMESSAGE(JTRC_ADOBE,
	 (const char*)"Adobe APP14 marker: version %d, flags 0x%04x 0x%04x, transform %d")
JMESSAGE(JTRC_APP0, (const char*)"Unknown APP0 marker (not JFIF), length %u")
JMESSAGE(JTRC_APP14, (const char*)"Unknown APP14 marker (not Adobe), length %u")
JMESSAGE(JTRC_DAC, (const char*)"Define Arithmetic Table 0x%02x: 0x%02x")
JMESSAGE(JTRC_DHT, (const char*)"Define Huffman Table 0x%02x")
JMESSAGE(JTRC_DQT, (const char*)"Define Quantization Table %d  precision %d")
JMESSAGE(JTRC_DRI, (const char*)"Define Restart Interval %u")
JMESSAGE(JTRC_EMS_CLOSE, (const char*)"Freed EMS handle %u")
JMESSAGE(JTRC_EMS_OPEN, (const char*)"Obtained EMS handle %u")
JMESSAGE(JTRC_EOI, (const char*)"End Of Image")
JMESSAGE(JTRC_HUFFBITS, (const char*)"        %3d %3d %3d %3d %3d %3d %3d %3d")
JMESSAGE(JTRC_JFIF, (const char*)"JFIF APP0 marker: version %d.%02d, density %dx%d  %d")
JMESSAGE(JTRC_JFIF_BADTHUMBNAILSIZE,
	 (const char*)"Warning: thumbnail image size does not match data length %u")
JMESSAGE(JTRC_JFIF_EXTENSION,
	 (const char*)"JFIF extension marker: type 0x%02x, length %u")
JMESSAGE(JTRC_JFIF_THUMBNAIL, (const char*)"    with %d x %d thumbnail image")
JMESSAGE(JTRC_MISC_MARKER, (const char*)"Miscellaneous marker 0x%02x, length %u")
JMESSAGE(JTRC_PARMLESS_MARKER, (const char*)"Unexpected marker 0x%02x")
JMESSAGE(JTRC_QUANTVALS, (const char*)"        %4u %4u %4u %4u %4u %4u %4u %4u")
JMESSAGE(JTRC_QUANT_3_NCOLORS, (const char*)"Quantizing to %d = %d*%d*%d colors")
JMESSAGE(JTRC_QUANT_NCOLORS, (const char*)"Quantizing to %d colors")
JMESSAGE(JTRC_QUANT_SELECTED, (const char*)"Selected %d colors for quantization")
JMESSAGE(JTRC_RECOVERY_ACTION, (const char*)"At marker 0x%02x, recovery action %d")
JMESSAGE(JTRC_RST, (const char*)"RST%d")
JMESSAGE(JTRC_SMOOTH_NOTIMPL,
	(const char*)"Smoothing not supported with nonstandard sampling ratios")
JMESSAGE(JTRC_SOF, (const char*)"Start Of Frame 0x%02x: width=%u, height=%u, components=%d")
JMESSAGE(JTRC_SOF_COMPONENT, (const char*)"    Component %d: %dhx%dv q=%d")
JMESSAGE(JTRC_SOI, (const char*)"Start of Image")
JMESSAGE(JTRC_SOS, (const char*)"Start Of Scan: %d components")
JMESSAGE(JTRC_SOS_COMPONENT, (const char*)"    Component %d: dc=%d ac=%d")
JMESSAGE(JTRC_SOS_PARAMS, (const char*)"  Ss=%d, Se=%d, Ah=%d, Al=%d")
JMESSAGE(JTRC_TFILE_CLOSE, (const char*)"Closed temporary file %s")
JMESSAGE(JTRC_TFILE_OPEN, (const char*)"Opened temporary file %s")
JMESSAGE(JTRC_THUMB_JPEG,
	 (const char*)"JFIF extension marker: JPEG-compressed thumbnail image, length %u")
JMESSAGE(JTRC_THUMB_PALETTE,
	 (const char*)"JFIF extension marker: palette thumbnail image, length %u")
JMESSAGE(JTRC_THUMB_RGB,
	 (const char*)"JFIF extension marker: RGB thumbnail image, length %u")
JMESSAGE(JTRC_UNKNOWN_IDS,
	 (const char*)"Unrecognized component IDs %d %d %d, assuming YCbCr")
JMESSAGE(JTRC_XMS_CLOSE, (const char*)"Freed XMS handle %u")
JMESSAGE(JTRC_XMS_OPEN, (const char*)"Obtained XMS handle %u")
JMESSAGE(JWRN_ADOBE_XFORM, (const char*)"Unknown Adobe color transform code %d")
JMESSAGE(JWRN_ARITH_BAD_CODE, (const char*)"Corrupt JPEG data: bad arithmetic code")
JMESSAGE(JWRN_BOGUS_PROGRESSION,
	 (const char*)"Inconsistent progression sequence for component %d coefficient %d")
JMESSAGE(JWRN_EXTRANEOUS_DATA,
	 (const char*)"Corrupt JPEG data: %u extraneous bytes before marker 0x%02x")
JMESSAGE(JWRN_HIT_MARKER, (const char*)"Corrupt JPEG data: premature end of data segment")
JMESSAGE(JWRN_HUFF_BAD_CODE, (const char*)"Corrupt JPEG data: bad Huffman code")
JMESSAGE(JWRN_JFIF_MAJOR, (const char*)"Warning: unknown JFIF revision number %d.%02d")
JMESSAGE(JWRN_JPEG_EOF, (const char*)"Premature end of JPEG file")
JMESSAGE(JWRN_MUST_RESYNC,
	 (const char*)"Corrupt JPEG data: found marker 0x%02x instead of RST%d")
JMESSAGE(JWRN_NOT_SEQUENTIAL, (const char*)"Invalid SOS parameters for sequential JPEG")
JMESSAGE(JWRN_TOO_MUCH_DATA, (const char*)"Application transferred too many scanlines")

#ifdef JMAKE_ENUM_LIST

  JMSG_LASTMSGCODE
} J_MESSAGE_CODE;

#undef JMAKE_ENUM_LIST
#endif /* JMAKE_ENUM_LIST */

/* Zap JMESSAGE macro so that future re-inclusions do nothing by default */
#undef JMESSAGE


#ifndef JERROR_H
#define JERROR_H

/* Macros to simplify using the error and trace message stuff */
/* The first parameter is either type of cinfo pointer */

/* Fatal errors (print message and exit) */
#define ERREXIT(cinfo,code)  \
  ((cinfo)->err->msg_code = (code), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT1(cinfo,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT2(cinfo,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT3(cinfo,code,p1,p2,p3)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (cinfo)->err->msg_parm.i[2] = (p3), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT4(cinfo,code,p1,p2,p3,p4)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (cinfo)->err->msg_parm.i[2] = (p3), \
   (cinfo)->err->msg_parm.i[3] = (p4), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXIT6(cinfo,code,p1,p2,p3,p4,p5,p6)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (cinfo)->err->msg_parm.i[2] = (p3), \
   (cinfo)->err->msg_parm.i[3] = (p4), \
   (cinfo)->err->msg_parm.i[4] = (p5), \
   (cinfo)->err->msg_parm.i[5] = (p6), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))
#define ERREXITS(cinfo,code,str)  \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, (str), JMSG_STR_PARM_MAX), \
   (*(cinfo)->err->error_exit) ((j_common_ptr) (cinfo)))

#define MAKESTMT(stuff)		do { stuff } while (0)

/* Nonfatal errors (we can keep going, but the data is probably corrupt) */
#define WARNMS(cinfo,code)  \
  ((cinfo)->err->msg_code = (code), \
   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), -1))
#define WARNMS1(cinfo,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), -1))
#define WARNMS2(cinfo,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), -1))

/* Informational/debugging messages */
/*
#define TRACEMS(cinfo,lvl,code)  \
  ((cinfo)->err->msg_code = (code), \
   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))
#define TRACEMS1(cinfo,lvl,code,p1)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))
#define TRACEMS2(cinfo,lvl,code,p1,p2)  \
  ((cinfo)->err->msg_code = (code), \
   (cinfo)->err->msg_parm.i[0] = (p1), \
   (cinfo)->err->msg_parm.i[1] = (p2), \
   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))
#define TRACEMS3(cinfo,lvl,code,p1,p2,p3)  \
  MAKESTMT(int * _mp = (cinfo)->err->msg_parm.i; \
	   _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); \
	   (cinfo)->err->msg_code = (code); \
	   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)); )
#define TRACEMS4(cinfo,lvl,code,p1,p2,p3,p4)  \
  MAKESTMT(int * _mp = (cinfo)->err->msg_parm.i; \
	   _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
	   (cinfo)->err->msg_code = (code); \
	   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)); )
#define TRACEMS5(cinfo,lvl,code,p1,p2,p3,p4,p5)  \
  MAKESTMT(int * _mp = (cinfo)->err->msg_parm.i; \
	   _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
	   _mp[4] = (p5); \
	   (cinfo)->err->msg_code = (code); \
	   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)); )
#define TRACEMS8(cinfo,lvl,code,p1,p2,p3,p4,p5,p6,p7,p8)  \
  MAKESTMT(int * _mp = (cinfo)->err->msg_parm.i; \
	   _mp[0] = (p1); _mp[1] = (p2); _mp[2] = (p3); _mp[3] = (p4); \
	   _mp[4] = (p5); _mp[5] = (p6); _mp[6] = (p7); _mp[7] = (p8); \
	   (cinfo)->err->msg_code = (code); \
	   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)); )
#define TRACEMSS(cinfo,lvl,code,str)  \
  ((cinfo)->err->msg_code = (code), \
   strncpy((cinfo)->err->msg_parm.s, (str), JMSG_STR_PARM_MAX), \
   (*(cinfo)->err->emit_message) ((j_common_ptr) (cinfo), (lvl)))
   */
#define TRACEMS(cinfo,lvl,code)
#define TRACEMS1(cinfo,lvl,code,p1)
#define TRACEMS2(cinfo,lvl,code,p1,p2)
#define TRACEMS3(cinfo,lvl,code,p1,p2,p3)
#define TRACEMS4(cinfo,lvl,code,p1,p2,p3,p4)
#define TRACEMS5(cinfo,lvl,code,p1,p2,p3,p4,p5)
#define TRACEMS8(cinfo,lvl,code,p1,p2,p3,p4,p5,p6,p7,p8)
#define TRACEMSS(cinfo,lvl,code,str)


#endif /* JERROR_H */
