/*
 * Copyright (c) 2016 The Khronos Group Inc.
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software source and associated documentation files (the "Materials"),
 * to deal in the Materials without restriction, including without limitation
 * the rights to use, copy, modify, compile, merge, publish, distribute,
 * sublicense, and/or sell copies of the Materials, and to permit persons to
 * whom the Materials are furnished to do so, subject the following terms and
 * conditions:
 * 
 * All modifications to the Materials used to create a binary that is
 * distributed to third parties shall be provided to Khronos with an
 * unrestricted license to use for the purposes of implementing bug fixes and
 * enhancements to the Materials;
 * 
 * If the binary is used as part of an OpenCL(TM) implementation, whether binary
 * is distributed together with or separately to that implementation, then
 * recipient must become an OpenCL Adopter and follow the published OpenCL
 * conformance process for that implementation, details at:
 * http://www.khronos.org/conformance/;
 * 
 * The above copyright notice, the OpenCL trademark license, and this permission
 * notice shall be included in all copies or substantial portions of the
 * Materials.
 * 
 * THE MATERIALS ARE PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE MATERIALS OR THE USE OR OTHER DEALINGS IN
 * THE MATERIALS.
 * 
 * OpenCL is a trademark of Apple Inc. used under license by Khronos.
 */

#include "icd.h"
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <pthread.h>

static pthread_once_t initialized = PTHREAD_ONCE_INIT;

#ifdef CL_TRACE
	#include <log/log.h>
	#include <cutils/properties.h>

	cl_bool sCLSystraceEnabled = false;
	cl_bool sCLTraceLevel = false;
#endif

/*
 *
 * Vendor enumeration functions
 *
 */

// go through the list of vendors in the two configuration files
void khrIcdOsVendorsEnumerate(void)
{
	const char* libSearchList[] = {
#if defined(__LP64__)
		"/system/lib64/egl/libGLES_mali.so",
		"/system/vendor/lib64/egl/libGLES_mali.so",
		"/system/vendor/lib64/libPVROCL.so",
#else
		"/system/lib/egl/libGLES_mali.so",
		"/system/vendor/lib/egl/libGLES_mali.so",
		"/system/vendor/lib/libPVROCL.so",
#endif
	};

	int i;
	int size = sizeof(libSearchList) / sizeof(libSearchList[0]);

	for (i = 0; i < size; ++i)
		khrIcdVendorAdd(libSearchList[i]);
	
#ifdef CL_TRACE
	initCLTraceLevel();
#endif

}

// go through the list of vendors only once
void khrIcdOsVendorsEnumerateOnce(void)
{
    pthread_once(&initialized, khrIcdOsVendorsEnumerate);
}

#ifdef CL_TRACE
void initCLTraceLevel()
{
	char value[PROPERTY_VALUE_MAX];
	property_get( "debug.ocl.trace" , value, 0);

	sCLSystraceEnabled = !strcasecmp(value, "systrace");
	if(sCLSystraceEnabled)
	{
		sCLTraceLevel = 0;
		ALOGD("initCLTraceLevel sCLSystraceEnabled = %d\n", sCLSystraceEnabled);
		return;
	}

	int propertyLevel = atoi(value);
	sCLTraceLevel = propertyLevel;
	ALOGD("initCLTraceLevel sCLTraceLevel = %d\n", sCLTraceLevel);
}
#endif

/*
 *
 * Dynamic library loading functions
 *
 */

// dynamically load a library.  returns NULL on failure
void *khrIcdOsLibraryLoad(const char *libraryName)
{
    return dlopen (libraryName, RTLD_NOW);
}

// get a function pointer from a loaded library.  returns NULL on failure.
void *khrIcdOsLibraryGetFunctionAddress(void *library, const char *functionName)
{
    return dlsym(library, functionName);
}

// unload a library
void khrIcdOsLibraryUnload(void *library)
{
    dlclose(library);
}

