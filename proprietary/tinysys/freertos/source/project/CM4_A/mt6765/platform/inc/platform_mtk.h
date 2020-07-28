#ifndef __PLATFORM_H
#define __PLATFORM_H

#if defined(MT6761)
#include <mt6761.h>
#elif defined(MT6765)
#include <mt6765.h>
#else
#error "undefined platform"
#endif


#include <scp_it.h>
#endif
