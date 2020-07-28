#ifndef __GET_DUMP_FILENAME_PREFIX__H__

#include <mtkcam/utils/metadata/IMetadata.h>

namespace NSCam {

//////////////////////////////////////////////////////////////////////////
// Get dump filename prefix from metadata
//
// Prefix:
//   (out) pointer to char buffer, in which will be stored dump path and filename prefix
//   ex: (2017.6.14)
//       /sdcard/camera_dump/uuuuuuuuu-ffff-rrrr
//   where:
//       uuuuuuuuu : 9 digits, pipeline model uniquekey
//       ffff      : 4 digits (maximun 8 digits), frame number
//       rrrr      : 4 digits, request number
//
// nPrefix:
//   (in)  sizeof Prefix (in byte), suggest 64 bytes.
// appMeta:
//   (in)  pointer to APP metadata (reserved, not used now)
// halMeta:
//   (in)  pointer to HAL metadata
//
// return value
//   if success, return a pointer to Prefix buffer
//   if fail, return a pointer to "" (empty string buffer)
const char *getDumpFilenamePrefix(char *pPrefix, int nPrefix, const IMetadata *appMeta, const IMetadata *halMeta);

}; //namespace NSCam

#endif //__GET_DUMP_FILENAME_PREFIX__H__


