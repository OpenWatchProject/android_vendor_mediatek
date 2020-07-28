/*
 * jinclude.h
 *
 * This file was part of the Independent JPEG Group's software:
 * Copyright (C) 1991-1994, Thomas G. Lane.
 * It was modified by The libjpeg-turbo Project to include only code relevant
 * to libjpeg-turbo.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file exists to provide a single place to fix any problems with
 * including the wrong system include files.  (Common problems are taken
 * care of by the standard jconfig symbols, but on really weird systems
 * you may have to edit this file.)
 *
 * NOTE: this file is NOT intended to be included by applications using the
 * JPEG library.  Most applications need only include jpeglib.h.
 */


/* Include auto-config file to find out which system include files we need. */

#include "jconfig_alpha.h"            /* auto configuration options */
#define JCONFIG_INCLUDED_ALPHA        /* so that jpeglib.h doesn't do it again */

/*
 * We need the NULL macro and size_t typedef.
 * On an ANSI-conforming system it is sufficient to include <stddef.h>.
 * Otherwise, we get them from <stdlib.h> or <stdio.h>; we may have to
 * pull in <sys/types.h> as well.
 * Note that the core JPEG library does not require <stdio.h>;
 * only the default error handler and data source/destination modules do.
 * But we must pull it in because of the references to FILE in jpeglib.h.
 * You can remove those references if you want to compile without <stdio.h>.
 */

#ifdef HAVE_STDDEF_H_ALPHA
#include <stddef.h>
#endif

#ifdef HAVE_STDLIB_H_ALPHA
#include <stdlib.h>
#endif

#ifdef NEED_SYS_TYPES_H_ALPHA
#include <sys/types.h>
#endif

#include <stdio.h>

/*
 * We need memory copying and zeroing functions, plus strncpy().
 * ANSI and System V implementations declare these in <string.h>.
 * BSD doesn't have the mem() functions, but it does have bcopy()/bzero().
 * Some systems may declare memset and memcpy in <memory.h>.
 *
 * NOTE: we assume the size parameters to these functions are of type size_t.
 * Change the casts in these macros if not!
 */

#ifdef NEED_BSD_STRINGS_ALPHA

#include <strings.h>
#define MEMZERO_ALPHA(target,size)    bzero((void *)(target), (size_t)(size))
#define MEMCOPY_ALPHA(dest,src,size)  bcopy((const void *)(src), (void *)(dest), (size_t)(size))

#else /* not BSD, assume ANSI/SysV string lib */

#include <string.h>
#define MEMZERO_ALPHA(target,size)    jpeg_memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY_ALPHA(dest,src,size)  jpeg_memcpy((void *)(dest), (const void *)(src), (size_t)(size))

#endif

/*
 * The modules that use fread() and fwrite() always invoke them through
 * these macros.  On some systems you may need to twiddle the argument casts.
 * CAUTION: argument order is different from underlying functions!
 */

#ifdef WRAPPED_API
#define JFREAD_ALPHA(file,buf,sizeofbuf)  \
  ((size_t) jpeg_fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE_ALPHA(file,buf,sizeofbuf)  \
  ((size_t) jpeg_fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JREWIND_ALPHA(file) \
  jpeg_rewind((file))
#else
#define JFREAD_ALPHA(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE_ALPHA(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JREWIND_ALPHA(file) \
  rewind((file))
#endif