#ifndef __AAL_CLIENT_PUBLIC_H__
#define __AAL_CLIENT_PUBLIC_H__

#include <AALIP.h>

#if defined(__AAL_10__)
#include <AAL10/AALClient.h>
#elif defined(__AAL_20__)
#include <AAL20/AALClient.h>
#else
#include <dummy/AALClient.h>
#endif

#endif
