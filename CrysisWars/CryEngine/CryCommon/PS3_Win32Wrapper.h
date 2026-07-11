////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2001-2005.
// -------------------------------------------------------------------------
//  File name:   PS3_Win32Wrapper.h
//  Version:     v1.00
//  Created:     23/6/2005 by Timur.
//  Compilers:   Visual Studio.NET 2003
//  Description: 
// -------------------------------------------------------------------------
//  History:
//  - saschad: copied from Linux_Win32Wapper.h#11
//
////////////////////////////////////////////////////////////////////////////

#ifndef __PS3_Win32Wrapper_h__
#define __PS3_Win32Wrapper_h__
#pragma once


//////////////////////////////////////////////////////////////////////////
// function renaming
#define _chmod chmod 
#define _snprintf snprintf
#define _isnan isnan
#define _stricmp strcasecmp
using std::strcasecmp;
using std::strncasecmp;
#define strnicmp strncasecmp
#define _strnicmp strncasecmp
extern int wcsicmp (const wchar_t* s1, const wchar_t* s2);
extern int wcsnicmp (const wchar_t* s1, const wchar_t* s2, size_t count);

template <size_t SIZE>
inline int strcpy_s(char (&dst)[SIZE], const char *src)
{
  strncpy(dst, src, SIZE - 1);
  dst[SIZE - 1] = 0;
  return 0;
}
/*
inline int fileno(FILE *stream)
{
	return stream->_Handle;
}
*/
//PS3HACK
#define MAX_FILE_HANDLE_INDEX 256

inline int strcpy_s(char *dst, size_t size, const char *src)
{
	strncpy(dst, src, size);
	dst[size - 1] = 0;
	return 0;
}

#define __TIMESTAMP__ __DATE__" "__TIME__

#if !defined(__SPU__)
//#include <asm/msr.h>
#include <types.h>
//#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
//#include <aio.h>
#include <string.h>
//#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <errno.h>
//#include <dirent.h>
//#include <fnmatch.h>
//#include <termios.h>
//#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
//#include <stdlib.h>
#include <vector>
#include <string>

#include <sys/ppu_thread.h>
#include <sys/synchronization.h>
#include <sys/time_util.h>
#include <sys/sys_time.h>

/* Memory block identification */
#define _FREE_BLOCK      0
#define _NORMAL_BLOCK    1
#define _CRT_BLOCK       2
#define _IGNORE_BLOCK    3
#define _CLIENT_BLOCK    4
#define _MAX_BLOCKS      5

typedef void *HMODULE;

//for compatibility reason we got to create a class which actually contains an int rather than a void* and make sure it does not get mistreated
template <class T, T U>//U is default type for invalid handle value, T the encapsulated handle type to be used instead of void* (as under windows and never linux)
class CHandle
{
public:
	typedef T			HandleType;
	typedef void* PointerType;	//for compatibility reason to encapsulate a void* as an int

	static const HandleType sciInvalidHandleValue = U;

	CHandle(const CHandle<T,U>& cHandle) : m_Value(cHandle.m_Value){}
	CHandle(const HandleType cHandle = U) : m_Value(cHandle){}
	//CHandle(const PointerType cpHandle) : m_Value(reinterpret_cast<HandleType>(cpHandle)){}
	//CHandle(INVALID_HANDLE_VALUE_ENUM) : m_Value(U){}//to be able to use a common value for all InvalidHandle - types

	operator HandleType(){return m_Value;}
	bool operator!() const{return m_Value == sciInvalidHandleValue;}
	const CHandle& operator =(const CHandle& crHandle){m_Value = crHandle.m_Value;return *this;}
	const CHandle& operator =(const PointerType cpHandle){m_Value = reinterpret_cast<HandleType>(cpHandle);return *this;}
	const bool operator ==(const CHandle& crHandle)		const{return m_Value == crHandle.m_Value;}
	const bool operator ==(const HandleType cHandle)	const{return m_Value == cHandle;}
	//const bool operator ==(const PointerType cpHandle)const{return m_Value == reinterpret_cast<HandleType>(cpHandle);}
	const bool operator !=(const HandleType cHandle)	const{return m_Value != cHandle;}
	const bool operator !=(const CHandle& crHandle)		const{return m_Value != crHandle.m_Value;}
	//const bool operator !=(const PointerType cpHandle)const{return m_Value != reinterpret_cast<HandleType>(cpHandle);}
	const bool operator <	(const CHandle& crHandle)		const{return m_Value < crHandle.m_Value;}
	HandleType Handle()const{return m_Value;}

private:
	HandleType m_Value;	//the actual value, remember that file descriptors are ints under linux

	typedef void	ReferenceType;//for compatibility reason to encapsulate a void* as an int
	//forbid these function which would actually not work on an int
	PointerType operator->();
	PointerType operator->() const;
	ReferenceType operator*();
	ReferenceType operator*() const;
	operator PointerType();
};

typedef CHandle<INT_PTR, (INT_PTR)0> HANDLE;

typedef HANDLE EVENT_HANDLE;
typedef HANDLE THREAD_HANDLE;

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

//-------------------------------------socket stuff------------------------------------------

typedef union _LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	};
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	} u;
	long long QuadPart;
} LARGE_INTEGER;


// stdlib.h stuff
#define _MAX_DRIVE  3   // max. length of drive component
#define _MAX_DIR    256 // max. length of path component
#define _MAX_FNAME  256 // max. length of file name component
#define _MAX_EXT    256 // max. length of extension component

// fcntl.h
#define _O_RDONLY       0x0000  /* open for reading only */
#define _O_WRONLY       0x0001  /* open for writing only */
#define _O_RDWR         0x0002  /* open for reading and writing */
#define _O_APPEND       0x0008  /* writes done at eof */
#define _O_CREAT        0x0100  /* create and open file */
#define _O_TRUNC        0x0200  /* open and truncate */
#define _O_EXCL         0x0400  /* open only if file doesn't already exist */
#define _O_TEXT         0x4000  /* file mode is text (translated) */
#define _O_BINARY       0x8000  /* file mode is binary (untranslated) */
#define _O_RAW  _O_BINARY
#define _O_NOINHERIT    0x0080  /* child process doesn't inherit file */
#define _O_TEMPORARY    0x0040  /* temporary file bit */
#define _O_SHORT_LIVED  0x1000  /* temporary storage file, try not to flush */
#define _O_SEQUENTIAL   0x0020  /* file access is primarily sequential */
#define _O_RANDOM       0x0010  /* file access is primarily random */


//////////////////////////////////////////////////////////////////////////
// io.h stuff
typedef unsigned int _fsize_t;

#if 0
struct _OVERLAPPED;
typedef _OVERLAPPED* LPOVERLAPPED;

typedef void (*LPOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, struct _OVERLAPPED *lpOverlapped);

typedef struct _OVERLAPPED
{
	void* pCaller;//this is orginally reserved for internal purpose, we store the Caller pointer here
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine; ////this is orginally ULONG_PTR InternalHigh and reserved for internal purpose
	union {
		struct {
			DWORD Offset;
			DWORD OffsetHigh;
		};
		PVOID Pointer;
	};
	DWORD dwNumberOfBytesTransfered;	//additional member temporary speciying the number of bytes to be read
	/*HANDLE*/void*  hEvent;
} OVERLAPPED, *LPOVERLAPPED;

#else

typedef CellFsAio OVERLAPPED, *LPOVERLAPPED;
typedef void (*LPOVERLAPPED_COMPLETION_ROUTINE)(CellFsAio *, CellFsErrno, int id, uint64_t size);
#endif

typedef struct _SECURITY_ATTRIBUTES 
{
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct tagRECT
{
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
} RECT, *PRECT;

typedef struct tagPOINT
{
  LONG  x;
  LONG  y;
} POINT, *PPOINT;

#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
#endif

typedef union _ULARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		DWORD HighPart;
	};
	unsigned long long QuadPart;
} ULARGE_INTEGER;

typedef ULARGE_INTEGER *PULARGE_INTEGER;

#ifdef __cplusplus
inline LONG CompareFileTime(const FILETIME *lpFileTime1, const FILETIME *lpFileTime2)
#else
static LONG CompareFileTime(const FILETIME *lpFileTime1, const FILETIME *lpFileTime2)
#endif
{
	ULARGE_INTEGER u1, u2;
	memcpy(&u1, lpFileTime1, sizeof u1);
	memcpy(&u2, lpFileTime2, sizeof u2);
	if(u1.QuadPart < u2.QuadPart)
		return -1;
	else
		if(u1.QuadPart > u2.QuadPart)
			return 1;
	return 0;
}

typedef struct _SYSTEMTIME{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _TIME_FIELDS
{
	short Year;
	short Month;
	short Day;
	short Hour;
	short Minute;
	short Second;
	short Milliseconds;
	short Weekday;
} TIME_FIELDS, *PTIME_FIELDS;

#define DAYSPERNORMALYEAR  365
#define DAYSPERLEAPYEAR    366
#define MONSPERYEAR        12

inline void ZeroMemory(void *pPtr, int nSize)
{
  memset(pPtr, 0, nSize);
}

inline BOOL InflateRect(RECT *pRect, int dx, int dy)
{
  pRect->left -= dx;
  pRect->right += dx;
  pRect->top -= dy;
  pRect->bottom += dy;
	return TRUE;
}

//////////////////////////////////////////////////////////////////////////
extern BOOL SystemTimeToFileTime( const SYSTEMTIME *syst, LPFILETIME ft );
//Win32API function declarations actually used 
extern bool IsBadReadPtr(void* ptr, unsigned int size );

// winapi stuff
inline void OutputDebugString ( const char* lpOutputString )
{
//PS3HACK -> Add
//#if !defined(NDEBUG)
	printf(lpOutputString);
//#endif	
}
static void DebugBreak() {}

/*
//critical section stuff
#define pthread_attr_default NULL

typedef pthread_mutex_t CRITICAL_SECTION;
#ifdef __cplusplus
inline void InitializeCriticalSection(CRITICAL_SECTION *lpCriticalSection)
{
pthread_mutexattr_t pthread_mutexattr_def;
pthread_mutexattr_settype(&pthread_mutexattr_def, PTHREAD_MUTEX_RECURSIVE_NP);
pthread_mutex_init(lpCriticalSection, &pthread_mutexattr_def);
}
inline void EnterCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_lock(lpCriticalSection);}
inline void LeaveCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_unlock(lpCriticalSection);}
inline void DeleteCriticalSection(CRITICAL_SECTION *lpCriticalSection){}
#else
static void InitializeCriticalSection(CRITICAL_SECTION *lpCriticalSection)
{
pthread_mutexattr_t pthread_mutexattr_def;
pthread_mutexattr_settype(&pthread_mutexattr_def, PTHREAD_MUTEX_RECURSIVE_NP);
pthread_mutex_init(lpCriticalSection, &pthread_mutexattr_def);
}
static void EnterCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_lock(lpCriticalSection);}
static void LeaveCriticalSection(CRITICAL_SECTION *lpCriticalSection){pthread_mutex_unlock(lpCriticalSection);}
static void DeleteCriticalSection(CRITICAL_SECTION *lpCriticalSection){}
#endif
*/

ILINE bool QueryPerformanceCounter(LARGE_INTEGER *counter)
{
#if !defined(__SPU__)
	SYS_TIMEBASE_GET(counter->QuadPart);
#else
	counter->QuadPart = rdtsc();
#endif
	return true;
}

ILINE bool QueryPerformanceFrequency(LARGE_INTEGER *frequency)
{
#if !defined(__SPU__)
	frequency->QuadPart = sys_time_get_timebase_frequency();
#else
	frequency->QuadPart = 80*1024*1024;//1 tick every 40 cycles, same as PPU
#endif
	return true;
}

#define GetTickCount CryGetTicks
#define GetCurrentTime CryGetTicks

#define INVALID_FILE_ATTRIBUTES			((DWORD)-1)

#define IGNORE              0       // Ignore signal
#define INFINITE            0xFFFFFFFF  // Infinite timeout

//begin--------------------------------findfirst/-next declaration/implementation----------------------------------------------------

#define _A_RDONLY       (0x01)    /* Read only file */
#define _A_SUBDIR       (0x10)    /* Subdirectory */

#ifdef __cplusplus

//////////////////////////////////////////////////////////////////////////

typedef int64 __time64_t;     /* 64-bit time value */

typedef struct __finddata64_t
{
	//!< atributes set by find request
	unsigned    int attrib;			//!< attributes, only directory and readonly flag actually set
	__time64_t	time_create;		//!< creation time, cannot parse under linux, last modification time is used instead (game does nowhere makes decision based on this values)
	__time64_t	time_access;		//!< last access time
	__time64_t	time_write;			//!< last modification time
	__time64_t	size;						//!< file size (for a directory it will be the block size)
	char        name[256];			//!< file/directory name

private:
	int									m_LastIndex;					//!< last index for findnext
	char								m_DirectoryName[260];			//!< directory name, needed when getting file attributes on the fly
	char								m_ToMatch[260];						//!< pattern to match with
	int									m_Dir;								//!< directory file descriptor
	std::vector<string>	m_Entries;						//!< all file entries in the current directories
public:

	inline __finddata64_t():
	  attrib(0), time_create(0), time_access(0), time_write(0),
		size(0), m_LastIndex(-1), m_Dir(-1)
	{
		memset(name, '0', 256);	
	}
	~__finddata64_t();
	
	//!< copies and retrieves the data for an actual match (to not waste any effort retrioeving data for unused files)
	void CopyFoundData(const char * rMatchedFileName);

public:
	//!< global _findfirst64 function using struct above, can't be a member function due to required semantic match
	friend intptr_t _findfirst64(const char *pFileName, __finddata64_t *pFindData);
	//!< global _findnext64 function using struct above, can't be a member function due to required semantic match
	friend int _findnext64(intptr_t last, __finddata64_t *pFindData);
}__finddata64_t;

typedef struct _finddata_t : public __finddata64_t
{}_finddata_t;//!< need inheritance since in many places it get used as struct _finddata_t
extern int _findnext64(intptr_t last, __finddata64_t *pFindData);
extern intptr_t _findfirst64(const char *pFileName, __finddata64_t *pFindData);
#endif
//end--------------------------------findfirst/-next declaration/implementation----------------------------------------------------

extern BOOL GetUserName(LPSTR lpBuffer, LPDWORD nSize);

//error code stuff
//not thread specific, just a coarse implementation for the main thread
inline DWORD GetLastError() { return errno; }
inline void SetLastError( DWORD dwErrCode ) { errno = dwErrCode; }

//////////////////////////////////////////////////////////////////////////
#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#define FILE_SHARE_READ						0x00000001
#define FILE_SHARE_WRITE					0x00000002
#define OPEN_EXISTING							3
#define FILE_FLAG_OVERLAPPED			0x40000000
#define INVALID_FILE_SIZE					((DWORD)0xFFFFFFFFl)
#define FILE_BEGIN								0
#define FILE_CURRENT							1
#define FILE_END									2

#define ERROR_NO_SYSTEM_RESOURCES 1450L
#define ERROR_INVALID_USER_BUFFER	1784L
#define ERROR_NOT_ENOUGH_MEMORY   8L
#define ERROR_PATH_NOT_FOUND      3L
#define ERROR_FILE_EXISTS         80L
#define ERROR_ALREADY_EXISTS      183L

#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000

//////////////////////////////////////////////////////////////////////////
// Win32 FileAttributes.
//////////////////////////////////////////////////////////////////////////
#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000
#define FILE_WRITE_ATTRIBUTES								FILE_ATTRIBUTE_NORMAL //not entirely correct but shoudl work for now
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateFile(
												 const char* lpFileName,
												 DWORD dwDesiredAccess,
												 DWORD dwShareMode,
												 void* lpSecurityAttributes,
												 DWORD dwCreationDisposition,
												 DWORD dwFlagsAndAttributes,
												 HANDLE hTemplateFile
												 );

//////////////////////////////////////////////////////////////////////////
extern DWORD GetFileAttributes(LPCSTR lpFileName);

//////////////////////////////////////////////////////////////////////////
extern BOOL SetFileAttributes(LPCSTR,DWORD attributes);


extern BOOL SetFileTime(const char* cpFile, const FILETIME *lpLastAccessTime);
//////////////////////////////////////////////////////////////////////////
extern const uint64 GetFileModifTime(FILE* hFile);

//////////////////////////////////////////////////////////////////////////
extern DWORD GetFileSize(HANDLE hFile,DWORD *lpFileSizeHigh );

//////////////////////////////////////////////////////////////////////////
extern BOOL CloseHandle( HANDLE hObject );

//////////////////////////////////////////////////////////////////////////
extern BOOL CancelIo( HANDLE hFile );
//////////////////////////////////////////////////////////////////////////
extern HRESULT GetOverlappedResult( HANDLE hFile,void* lpOverlapped,LPDWORD lpNumberOfBytesTransferred, BOOL bWait );
//////////////////////////////////////////////////////////////////////////
extern BOOL ReadFile(
							HANDLE hFile,
							LPVOID lpBuffer,
							DWORD nNumberOfBytesToRead,
							LPDWORD lpNumberOfBytesRead,
							LPOVERLAPPED lpOverlapped
							);

//////////////////////////////////////////////////////////////////////////
extern BOOL ReadFileEx(
											 HANDLE hFile,
											 LPVOID lpBuffer,
											 DWORD nNumberOfBytesToRead,
											 LPOVERLAPPED lpOverlapped,
											 LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
											 );

//////////////////////////////////////////////////////////////////////////
extern DWORD SetFilePointer(
										 HANDLE hFile,
										 LONG lDistanceToMove,
										 PLONG lpDistanceToMoveHigh,
										 DWORD dwMoveMethod
										 );

//////////////////////////////////////////////////////////////////////////
extern DWORD GetCurrentThreadId();

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateEvent(
						 LPSECURITY_ATTRIBUTES lpEventAttributes,
						 BOOL bManualReset,
						 BOOL bInitialState,
						 LPCSTR lpName
						 );

//////////////////////////////////////////////////////////////////////////
extern DWORD Sleep( DWORD dwMilliseconds );

//////////////////////////////////////////////////////////////////////////
extern DWORD SleepEx( DWORD dwMilliseconds,BOOL bAlertable );

//////////////////////////////////////////////////////////////////////////
extern DWORD WaitForSingleObjectEx(
											HANDLE hHandle,
											DWORD dwMilliseconds,
											BOOL bAlertable );

//////////////////////////////////////////////////////////////////////////
extern DWORD WaitForMultipleObjectsEx(
												 DWORD nCount,
												 const HANDLE *lpHandles,
												 BOOL bWaitAll,
												 DWORD dwMilliseconds,
												 BOOL bAlertable );

//////////////////////////////////////////////////////////////////////////
extern DWORD WaitForSingleObject( HANDLE hHandle,DWORD dwMilliseconds );

//////////////////////////////////////////////////////////////////////////
extern BOOL SetEvent( HANDLE hEvent );

//////////////////////////////////////////////////////////////////////////
extern BOOL ResetEvent( HANDLE hEvent );

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateMutex(
						 LPSECURITY_ATTRIBUTES lpMutexAttributes,
						 BOOL bInitialOwner,
						 LPCSTR lpName
						 );

//////////////////////////////////////////////////////////////////////////
extern BOOL ReleaseMutex( HANDLE hMutex );

//////////////////////////////////////////////////////////////////////////
typedef DWORD (*PTHREAD_START_ROUTINE)( LPVOID lpThreadParameter );
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

//////////////////////////////////////////////////////////////////////////
extern HANDLE CreateThread(
						 LPSECURITY_ATTRIBUTES lpThreadAttributes,
						 SIZE_T dwStackSize,
						 LPTHREAD_START_ROUTINE lpStartAddress,
						 LPVOID lpParameter,
						 DWORD dwCreationFlags,
						 LPDWORD lpThreadId
						 );

extern BOOL DeleteFile(LPCSTR lpFileName);
extern BOOL MoveFile( LPCSTR lpExistingFileName,LPCSTR lpNewFileName );
extern BOOL RemoveDirectory(LPCSTR lpPathName);
extern DWORD GetCurrentDirectory( DWORD nBufferLength, char* lpBuffer );
//extern BOOL SetCurrentDirectory(LPCSTR lpPathName);

//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus

//helper function
#if !defined(USE_HDD0)
	extern const bool GetFilenameNoCase(const char *file, char*, const bool cCreateNew = false);
#endif
extern const int comparePathNames(const char* cpFirst, const char* cpSecond, const unsigned int len);//returns 0 if identical
//////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// Wrapper function.
//////////////////////////////////////////////////////////////////////////
inline int _CrtCheckMemory() { return 1; };

//////////////////////////////////////////////////////////////////////////
inline LONG _InterlockedCompareExchange( LONG volatile* Destination,LONG Exchange,LONG Comperand )
{
	LONG prev = *Destination;
	if (*Destination == Comperand)
		*Destination = Exchange;
	return prev;
}

//////////////////////////////////////////////////////////////////////////
inline LONG _InterlockedExchangeAdd( LONG volatile* Addend,LONG Value )
{
	LONG prev = *Addend;
	*Addend += Value;
	return prev;
}

//////////////////////////////////////////////////////////////////////////
extern char *_fullpath( char *absPath,const char *relPath,size_t maxLength );
extern intptr_t _findfirst64( const char *filespec,struct __finddata64_t *fileinfo );
extern int _findnext64( intptr_t handle,struct __finddata64_t *fileinfo );
extern int _findclose( intptr_t handle );

//////////////////////////////////////////////////////////////////////////
extern int _mkdir( const char *dirname );
extern void _makepath(char * path, const char * drive, const char *dir, const char * filename, const char * ext);
extern void _splitpath(const char* inpath, char * drv, char * dir, char* fname, char * ext);

//////////////////////////////////////////////////////////////////////////
extern int memicmp( LPCSTR s1, LPCSTR s2, DWORD len );
extern int strcmpi( const char *str1, const char *str2 );

#if defined PS3_GAMESPY && 0
extern "C" char* strlwr (char * str);
extern "C" char* strupr(char * str);
#else
extern char* strlwr (char * str);
extern char* strupr(char * str);
#endif

extern char * _ui64toa(unsigned long long value,	char *str, int radix);
extern long long _atoi64( char *str );

//////////////////////////////////////////////////////////////////////////
// function renaming
#define _chmod chmod 
#define _snprintf snprintf
#define _isnan isnan
#define _stricmp strcasecmp
using std::strcasecmp;
using std::strncasecmp;
#define strnicmp strncasecmp
#define _strnicmp strncasecmp

#define _exit exit

#define _strlwr	strlwr 
#define _strups	strupr

#define _snwprintf swprintf
#define _vsnprintf vsnprintf
#define _wtof( str ) wcstod( str, 0 )
//////////////////////////////////////////////////////////////////////////

#ifndef __TRLTOA__
#define __TRLTOA__
extern char *ltoa ( long i , char *a , int radix );
#endif
#define itoa ltoa


//////////////////////////////////////////////////////////////////////////
inline int _finite(double x)
{
	return 1;
}

//////////////////////////////////////////////////////////////////////////
#include <cmath>
using std::abs;
using std::sqrt;

//typedef DWORD FOURCC;         //a four character code

extern bool QueryPerformanceCounter(LARGE_INTEGER *counter);
extern bool QueryPerformanceFrequency(LARGE_INTEGER *frequency);
extern int gettimeofday(struct timeval *__restrict tp, void *__restrict);

extern char* _strtime(char* date);
extern char* _strdate(char* date);

_C_STD_BEGIN
#if __GNUC__ >= 4
	std::FILE *WrappedFopen(const char *_Restrict, const char *_Restrict);
	#if !defined(USE_HDD0)
		int WrappedFclose(std::FILE *);
	#endif
#else
	std::FILE *std::WrappedFopen(const char *_Restrict, const char *_Restrict);
	#if !defined(USE_HDD0)
		int std::WrappedFclose(std::FILE *);
	#endif
#endif
_C_STD_END

using std::WrappedFopen;
#if !defined(USE_HDD0)
	using std::WrappedFclose;
#endif
// Wrap fopen()/fclose() calls, implemented in WinBase.cpp.
//#if 0
#undef fopen
#define fopen WrappedFopen
#if !defined(USE_HDD0)
	#undef fclose
	#define fclose WrappedFclose
#endif
//#endif

#endif //__cplusplus

#else // !__SPU__
	#include <SPU/SPU.h>
#if defined __CRYCG__

// Note: Below is a collection of all declarations needed to compile all of
// CryEngine2 via CryCG.  The entire *Win32Wrapper.h and *Specific.h mess
// really needs a cleanup...

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include MATH_H
#include <time.h>

typedef int64 __time64_t;

struct __finddata64_t
{
	unsigned int attrib;
	__time64_t time_create;
	__time64_t time_access;
	__time64_t time_write;
	__time64_t size;
	char name[256];
	__finddata64_t();
	void CopyFoundData(const char *);
};

typedef struct _finddata_t : public __finddata64_t { } _finddata_t;

inline int _finite(double) { return 1; }

extern char *ltoa(long, char *, int);
extern char *itoa(int, char *, int);

extern char *_fullpath(char *, const char *, size_t);
extern intptr_t _findfirst64(const char *, struct __finddata64_t *);
extern int _findnext64(intptr_t, struct __finddata64_t *);
extern int _findclose(intptr_t);

extern void _makepath(char *, const char *, const char *, const char *, const char *);
extern void _splitpath(const char *, char *, char *, char *, char *);

extern char *strlwr(char *);
extern int strcmpi(const char *, const char *);

extern char *strlwr(char *);
extern char *strupr(char *);

extern char *_ui64toa(unsigned long long,	char *, int);
extern long long _atoi64(char *);

extern FILE *fopen(const char *, const char *);
extern FILE *fdopen(int, const char *);
extern FILE *freopen(const char *, const char *, FILE *);
extern size_t fread(void *, size_t, size_t, FILE *);
extern size_t fwrite(const void *, size_t, size_t, FILE *);
extern void clearerr(FILE *);
extern int feof(FILE *);
extern int ferror(FILE *);
//extern int fileno(FILE *);
extern int fclose(FILE *);
extern int fcloseall();
extern int fflush(FILE *);
extern int fseek(FILE *, long, int);
extern long ftell(FILE *);
extern void rewind(FILE *);
extern int remove(const char *);

inline int printf(const char *, ...) { return 0; }
inline int vprintf(const char *, void *) { return 0; }

extern int fprintf(FILE *, const char *, ...);
extern int vfprintf(FILE *, const char *, void *);

extern int scanf(const char *, ...);
extern int fscanf(FILE *, const char *, ...);
extern int vscanf(const char *, void *);
extern int vfscanf(FILE *, const char *, void *);
extern int vsscanf(const char *, const char *, void *);

extern int fgetc(FILE *);
extern char *fgets(char *, int, FILE *);
extern int getc(FILE *);
extern int getchar();
extern char *gets(char *);
extern int ungetc(int, FILE *);

extern int fputc(int, FILE *);
extern int fputs(const char *, FILE *);
extern int putc(int, FILE *);
extern int putchar(int);
extern int puts(const char *);

extern void setbuf(FILE *, char *);
extern void setbuffer(FILE *, char *, size_t);
extern void setlinebuf(FILE *);
extern int setvbuf(FILE *, char *, int, size_t);

extern void *alloca(size_t);

extern DWORD GetCurrentThreadId();
extern int64 CryGetTicks();
extern DWORD GetCurrentDirectory(DWORD, char *);
inline void OutputDebugString(const char *) { }

#define _vsnprintf vsnprintf

#define _A_RDONLY       (0x01)
#define _A_SUBDIR       (0x10)
#define _MAX_DRIVE      (3)
#define _MAX_DIR        (256)
#define _MAX_FNAME      (256)
#define _MAX_EXT        (256)

extern char *_strtime(char *);
extern char *_strdate(char *);

extern void _exit(int);

struct pollfd
{
	int fd;
	short events;
	short revents;
};

typedef int SOCKET;
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)

#define htonl(x)		(x)
#define htons(x)		(x)
#define ntohl(x)		(x)
#define ntohs(x)		(x)

extern DWORD Sleep(DWORD);

typedef void *HMODULE;

template <class T, T U>
class CHandle
{
public:
	typedef T HandleType;
	typedef void *PointerType;
	static const HandleType sciInvalidHandleValue = U;
	CHandle(const CHandle<T,U>& cHandle) : m_Value(cHandle.m_Value){}
	CHandle(const HandleType cHandle = U) : m_Value(cHandle){}
	operator HandleType(){return m_Value;}
	bool operator!() const{return m_Value == sciInvalidHandleValue;}
	const CHandle& operator =(const CHandle& crHandle){m_Value = crHandle.m_Value;return *this;}
	const CHandle& operator =(const PointerType cpHandle){m_Value = reinterpret_cast<HandleType>(cpHandle);return *this;}
	const bool operator ==(const CHandle& crHandle) const{return m_Value == crHandle.m_Value;}
	const bool operator ==(const HandleType cHandle) const{return m_Value == cHandle;}
	const bool operator !=(const HandleType cHandle) const{return m_Value != cHandle;}
	const bool operator !=(const CHandle& crHandle) const{return m_Value != crHandle.m_Value;}
	const bool operator <(const CHandle& crHandle) const{return m_Value < crHandle.m_Value;}
	HandleType Handle()const{return m_Value;}
private:
	HandleType m_Value;
	typedef void ReferenceType;
	PointerType operator->();
	PointerType operator->() const;
	ReferenceType operator*();
	ReferenceType operator*() const;
	operator PointerType();
};
typedef CHandle<INT_PTR, (INT_PTR)0> HANDLE;
typedef HANDLE EVENT_HANDLE;
typedef HANDLE THREAD_HANDLE;

#define WSAEINTR SYS_NET_EINTR
#define WSAEBADF SYS_NET_EBADF
#define WSAEACCES SYS_NET_EACCES
#define WSAEFAULT SYS_NET_EFAULT
#define WSAEACCES SYS_NET_EACCES
#define WSAEFAULT SYS_NET_EFAULT
#define WSAEINVAL SYS_NET_EINVAL
#define WSAEMFILE SYS_NET_EMFILE
#define WSAEWOULDBLOCK SYS_NET_EAGAIN
#define WSAEINPROGRESS SYS_NET_EINPROGRESS
#define WSAEALREADY SYS_NET_EALREADY
#define WSAENOTSOCK SYS_NET_ENOTSOCK 
#define WSAEDESTADDRREQ SYS_NET_EDESTADDRREQ
#define WSAEMSGSIZE SYS_NET_EMSGSIZE
#define WSAEPROTOTYPE SYS_NET_EPROTOTYPE
#define WSAENOPROTOOPT SYS_NET_ENOPROTOOPT
#define WSAEPROTONOSUPPORT SYS_NET_EPROTONOSUPPORT
#define WSAESOCKTNOSUPPORT SYS_NET_ESOCKTNOSUPPORT
#define WSAEOPNOTSUPP SYS_NET_EOPNOTSUPP
#define WSAEPFNOSUPPORT SYS_NET_EPFNOSUPPORT
#define WSAEAFNOSUPPORT SYS_NET_EAFNOSUPPORT
#define WSAEADDRINUSE SYS_NET_EADDRINUSE
#define WSAEADDRNOTAVAIL SYS_NET_EADDRNOTAVAIL
#define WSAENETDOWN SYS_NET_ENETDOWN
#define WSAENETUNREACH SYS_NET_ENETUNREACH
#define WSAENETRESET SYS_NET_ENETRESET
#define WSAECONNABORTED SYS_NET_ECONNABORTED
#define WSAECONNRESET SYS_NET_ECONNRESET
#define WSAENOBUFS SYS_NET_ENOBUFS
#define WSAEISCONN SYS_NET_EISCONN
#define WSAENOTCONN SYS_NET_ENOTCONN
#define WSAESHUTDOWN SYS_NET_ESHUTDOWN
#define WSAETOOMANYREFS SYS_NET_ETOOMANYREFS
#define WSAETIMEDOUT SYS_NET_ETIMEDOUT
#define WSAECONNREFUSED SYS_NET_ECONNREFUSED
#define WSAELOOP SYS_NET_ELOOP
#define WSAENAMETOOLONG SYS_NET_ENAMETOOLONG
#define WSAEHOSTDOWN SYS_NET_EHOSTDOWN
#define WSAEHOSTUNREACH SYS_NET_EHOSTUNREACH
#define WSAENOTEMPTY SYS_NET_ENOTEMPTY
#define WSAEPROCLIM SYS_NET_EPROCLIM
#define WSAEUSERS SYS_NET_EUSERS
#define WSAEDQUOT SYS_NET_EDQUOT
#define WSAESTALE SYS_NET_ESTALE
#define WSAEREMOTE SYS_NET_EREMOTE

#define WSAHOST_NOT_FOUND (1024 + 1)
#define WSATRY_AGAIN (1024 + 2)
#define WSANO_RECOVERY (1024 + 3)
#define WSANO_DATA (1024 + 4)
#define WSANO_ADDRESS (WSANO_DATA)

#define SD_RECEIVE      SHUT_RD
#define SD_SEND         SHUT_WR
#define SD_BOTH         SHUT_RDWR

#define _O_RDONLY       0x0000
#define _O_WRONLY       0x0001
#define _O_RDWR         0x0002
#define _O_APPEND       0x0008
#define _O_CREAT        0x0100
#define _O_TRUNC        0x0200
#define _O_EXCL         0x0400
#define _O_TEXT         0x4000
#define _O_BINARY       0x8000
#define _O_RAW  _O_BINARY
#define _O_NOINHERIT    0x0080
#define _O_TEMPORARY    0x0040
#define _O_SHORT_LIVED  0x1000
#define _O_SEQUENTIAL   0x0020
#define _O_RANDOM       0x0010

extern int comparePathNames(const char *, const char *, unsigned);

#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000
#define FILE_WRITE_ATTRIBUTES               FILE_ATTRIBUTE_NORMAL

extern int memicmp(LPCSTR, LPCSTR, DWORD);

struct _OVERLAPPED;
typedef _OVERLAPPED* LPOVERLAPPED;

typedef void (*LPOVERLAPPED_COMPLETION_ROUTINE)(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, struct _OVERLAPPED *lpOverlapped);

typedef struct _OVERLAPPED
{
	void *pCaller;
	LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine;
	union {
		struct {
			DWORD Offset;
			DWORD OffsetHigh;
		};
		PVOID Pointer;
	};
	DWORD dwNumberOfBytesTransfered;
	void *hEvent;
} OVERLAPPED, *LPOVERLAPPED;

#define INVALID_HANDLE_VALUE ((HANDLE)(LONG_PTR)-1)

#define GENERIC_READ                     (0x80000000L)
#define GENERIC_WRITE                    (0x40000000L)
#define GENERIC_EXECUTE                  (0x20000000L)
#define GENERIC_ALL                      (0x10000000L)

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

#define FILE_SHARE_READ						0x00000001
#define FILE_SHARE_WRITE					0x00000002
#define OPEN_EXISTING							3
#define FILE_FLAG_OVERLAPPED			0x40000000
#define INVALID_FILE_SIZE					((DWORD)0xFFFFFFFFl)
#define FILE_BEGIN								0
#define FILE_CURRENT							1
#define FILE_END									2
#define ERROR_NO_SYSTEM_RESOURCES 1450L
#define ERROR_INVALID_USER_BUFFER	1784L
#define ERROR_NOT_ENOUGH_MEMORY   8L
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000

typedef union _LARGE_INTEGER
{
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	};
	struct
	{
		DWORD LowPart;
		LONG HighPart;
	} u;
	long long QuadPart;
} LARGE_INTEGER;

typedef struct _SECURITY_ATTRIBUTES 
{
	DWORD nLength;
	LPVOID lpSecurityDescriptor;
	BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

extern HANDLE CreateFile(const char *, DWORD, DWORD, void *, DWORD, DWORD, HANDLE);
extern DWORD GetLastError();
extern DWORD GetFileSize(HANDLE,DWORD *);
extern BOOL CloseHandle(HANDLE);
extern BOOL CancelIo(HANDLE);
extern HRESULT GetOverlappedResult(HANDLE, void *, LPDWORD, BOOL);
extern BOOL ReadFileEx(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE);
extern DWORD SetFilePointer(HANDLE, LONG, PLONG, DWORD);
extern BOOL ReadFile(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
extern bool QueryPerformanceFrequency(LARGE_INTEGER *);
extern bool QueryPerformanceCounter(LARGE_INTEGER *);
extern HANDLE CreateEvent(LPSECURITY_ATTRIBUTES, BOOL, BOOL, LPCSTR);
extern BOOL SetEvent(HANDLE);
extern DWORD WaitForSingleObject(HANDLE, DWORD);
extern DWORD WaitForSingleObjectEx(HANDLE, DWORD,	BOOL);
extern BOOL ResetEvent(HANDLE);
extern DWORD SleepEx(DWORD, BOOL);
extern BOOL SetFileAttributes(LPCSTR, DWORD);

typedef DWORD (*PTHREAD_START_ROUTINE)(LPVOID);
typedef PTHREAD_START_ROUTINE LPTHREAD_START_ROUTINE;

extern HANDLE CreateThread(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD);

#define IGNORE              0       // Ignore signal
#define INFINITE            0xFFFFFFFF  // Infinite timeout
#if !defined(PS3)
	#define VK_SCROLL         0x91
	#define VK_UP             0x26
	#define VK_CONTROL        0x11
	#define VK_DOWN           0x28
	#define VK_CONTROL        0x11
	#define VK_RIGHT          0x27
	#define VK_LEFT           0x25
	#define VK_SPACE          0x20
#endif
extern BOOL DeleteFile(LPCSTR);
extern BOOL RemoveDirectory(LPCSTR);
extern BOOL MoveFile(LPCSTR, LPCSTR);

typedef struct _SYSTEMTIME{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
} SYSTEMTIME, *PSYSTEMTIME, *LPSYSTEMTIME;

typedef struct _TIME_FIELDS
{
	short Year;
	short Month;
	short Day;
	short Hour;
	short Minute;
	short Second;
	short Milliseconds;
	short Weekday;
} TIME_FIELDS, *PTIME_FIELDS;

#ifndef _FILETIME_
#define _FILETIME_
typedef struct _FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} FILETIME, *PFILETIME, *LPFILETIME;
#endif

extern BOOL SystemTimeToFileTime(const SYSTEMTIME *, LPFILETIME);

#define SCOPED_ENABLE_FLOAT_EXCEPTIONS

typedef struct tagRECT
{
	LONG    left;
	LONG    top;
	LONG    right;
	LONG    bottom;
} RECT, *PRECT;

typedef struct tagPOINT
{
  LONG  x;
  LONG  y;
} POINT, *PPOINT;

#endif // __CRYCG__

#include MATH_H // For abs(float), abs(double), ...
#if !defined(__SPU__)
	#include <time.h>
#endif

namespace std
{
  inline long abs(long x) { return labs(x); }
  inline long long abs(long long x) { return llabs(x); }
}
using std::abs;

#endif //__SPU__

#if !defined(__SPU__)
	#include <PPU/SPUJobBase.h>
	#include <SPU/GenericSPUJob.h>
#endif

#endif // __PS3_Win32Wrapper_h__

// vim:ts=2

