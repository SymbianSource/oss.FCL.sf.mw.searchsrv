#ifndef COMPILERGCCE_H_
#define COMPILERGCCE_H_

// TODO: Clean me

#include <sys/types.h>
#include <stdint.h>
#include <limits.h>

// XXX: Let's include std io before defining _close as ::close
#include <stdio.h>

// Likely should be included from Platform Symbian. 
// Original config include was anyway in CompilerXXX.h so following that lead
#include "CLucene/config/OpenCConfig.h" 


//#define _LUCENE_PRAGMA_ONCE
//#define _LUCENE_PRAGMA_WARNINGS //tell lucene to display warnings using pragmas instead of #warning
#define LUCENE_DISABLE_HASHING
#define LUCENE_STATIC_CONSTANT(type, assignment) enum { assignment }

//#undef LUCENE_ENABLE_MEMLEAKTRACKING //it has been reported that this causes problems

#define fileSeek lseek
#define fileTell tell
#define fileStat stat
#define fileHandleStat fstat

#define O_RANDOM 0
#define O_BINARY_CL 0
#define _S_IREAD  0444
#define _S_IWRITE 0333  // write and execute permissions

#define fileSize lucene_filelength
int64_t lucene_filelength(int handle);

#define _open open
#define _write write
#define _snprintf snprintf
#define _read ::read
#define _close ::close

// XXX: Realpath crashes randomly. Let's not use it
// TODO: Reimplement realpath for S60? 
#ifdef _CL_DISABLE_REALPATH
#define _realpath(rel,abs) strcpy(abs,rel)
#else
#define _realpath(rel,abs) realpath(rel,abs)
#endif

//clucene uses ascii for filename interactions
#define _realpath(rel,abs) realpath(rel,abs)
#define _mkdir(x) mkdir(x,0777)
#define _unlink unlink
#define _rename rename

//#include "CLucene/config/CompilerGcc.h" //make clucene-config.h file


#endif /*COMPILERGCCE_H_*/
