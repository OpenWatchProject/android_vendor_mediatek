#ifndef _WIN2MAC_H_
#define _WIN2MAC_H_

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <wchar.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#define _UNICODE	//	UCS2

#define _ENABLE_LOG_
#define BOOLEAN	int
#define LPVOID	void*

#define OSAL_FALSE 0
#define OSAL_TRUE  1
#define DWORD	unsigned int
#define LPCSTR	const char*
#define LPSTR	char*
#define LPCWSTR	const wchar_t*
#define LPWSTR	wchar_t*

#ifdef _UNICODE
#define	TCHAR	wchar_t
#define _TUCHAR	TCHAR
#define LPTSTR	LPWSTR
#define LPCTSTR	LPCWSTR
#define	_T(x)	L ## x
#define _tcslen	wcslen
#define _tcscmp	wcscmp
#define _tcsncmp wcsncmp
#define _tcschr	wcschr
#define _tcsrchr	wcsrchr
#define _tcsstr	wcsstr
#define _tcscat wcscat
#define _tcscpy	wcscpy
#define _tcsncpy	wcsncpy
#define _tcspbrk	wcspbrk
#define _stscanf	swscanf
#define _fgetts		fgetws

#ifdef _ENABLE_LOG_
#define DebugOut	DebugOutW
#endif

#else
#define	TCHAR	char
#define _TUCHAR	unsigned char
#define LPTSTR	LPSTR
#define LPCTSTR	LPCSTR
#define	_T(x)	x
#define _tcslen	strlen
#define _tcscmp	strcmp
#define _tcsncmp strcmp
#define _tcschr	strchr
#define _tcsrchr	strrchr
#define _tcsstr	strstr
#define _tcscat strcat
#define _tcscpy	strcpy
#define _tcsncpy	strncpy
#define _tcspbrk	strpbrk
#define _stscanf	sscanf
#define _fgetts		fgets

#ifdef _ENABLE_LOG_
#define DebugOut	DebugOutA
#endif

#endif

#define _TCHAR	TCHAR
#define WAIT_TIMEOUT	0x102
#define WAIT_OBJECT_0	0x0
#define WAIT_FAILED		0xffffffff
#define INFINITE		0xffffffff
#define __stdcall
#define afx_msg
#define LRESULT		LONG
#define WINAPI		__stdcall
#define CALLBACK	__stdcall
//#define ASSERT(x)	assert(x)
#define _wcstombsz	wcstombs
#define Sleep(x)	usleep((x)*1000)
#define TRACE(x)	DebugOut(x)


#define	tagTHREAD		(1)
#define tagMUTEX		(1 << 1)
#define tagCONDITION	(1 << 2)

#define CP_ACP  0
#define MB_PRECOMPOSED  0x01


#ifdef DEBUG
#define _DEBUG
#define ASSERT assert
#define VERIFY assert
#else
#define ASSERT
#define VERIFY(expr) expr
#endif

#ifdef _ENABLE_LOG_
void DebugOutA(LPCSTR fmt, ...);
void DebugOutW(LPCWSTR fmt, ...);
#endif

typedef pthread_mutex_t CRITICAL_SECTION,*LPCRITICAL_SECTION;
typedef DWORD __stdcall (*winThreadFunc) (LPVOID);

typedef struct {
	winThreadFunc	fn;
	LPVOID			arg;
	BOOLEAN*		psig;
}	winThreadFuncWrapper;

typedef struct {
	unsigned char		type;
	pthread_t			thread;
	pthread_mutex_t		mutex;
	pthread_cond_t		condition;
	BOOLEAN				signaled;
	BOOLEAN				manualreset;
}	HANDLE_CONTENT,		*OSAL_HANDLE;


#define INVALID_HANDLE_VALUE (-1)
/*
#include "basestr.h"
#include "afxstr.h"
#include "afxwin.h"
*/
#endif
