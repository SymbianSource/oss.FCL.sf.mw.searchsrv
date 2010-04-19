/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#include "clucene/stdheader.h"
#include "clucene/util/misc.h"

#include <string>

#ifdef DBG_MEM_USAGE
#include "cpixstrtools.h" // for PeakDynMemUsage
#endif


bool _lucene_disable_debuglogging = true; //if LUCENE_ENABLE_CONSTRUCTOR_LOG is on, dont do log if this is true
bool _lucene_run_objectcheck = false; //run a memory check before deleting objects
int _lucene_counter_break = -1; //to break at this item, change this 
							 //and put break points at points below

CL_NS_USE(util)
CL_NS_DEF(debug)

#ifdef DBG_MEM_USAGE
uint32_t PeakDynMemUsage = 0;

void MeasureDynMemUsage()
{
    uint32_t
        curDynMemUsage = Cpt::getUsedDynamicMemory();
    if (curDynMemUsage > PeakDynMemUsage)
        {
            PeakDynMemUsage = curDynMemUsage;
        }
}
#endif


#ifdef LUCENE_ENABLE_MEMLEAKTRACKING
int32_t _instance_counter = 0; //counter for initnumber

struct _file{
   int32_t refcount; ///times this has been used
   char* value; ///reference to the the basefile
}; //structure for name counting
struct _pointers{
   _file* file;
   int32_t initline;
   int32_t initnumber;
};//structure for pointer-filename references

typedef CL_NS(util)::CLSet<const char*,_file*,Compare::Char,Deletor::Dummy,Deletor::Void<_file> > defFile;
typedef CL_NS(util)::CLSet<LuceneBase*,_pointers*,Compare::Void<LuceneBase>,Deletor::Dummy,Deletor::Void<_pointers> > defPointer;
typedef CL_NS(util)::CLSet<const void*,_pointers*,Compare::Void<const void>,Deletor::Dummy,Deletor::Void<_pointers> > defVoid;

#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
// CPIXASYNC DEFINE_MUTEX(memleak_lock)
GLOBAL_DEFINE_CRUCIAL_MUTEX(memleak_lock)
#define memleak_lock GetInstancemutex_pthreadmemleak_lock()

DECL_STATVAR(defFile,LuceneBase_Files,false,true)
#define LuceneBase_Files GetInstancedefFileLuceneBase_Files()

DECL_STATVAR(defPointer,LuceneBase_Pointers,false,true)
#define LuceneBase_Pointers GetInstancedefPointerLuceneBase_Pointers()

DECL_STATVAR(defVoid,LuceneBase_Voids,false,true)
#define LuceneBase_Voids GetInstancedefVoidLuceneBase_Voids()

#else
// CPIXASYNC DEFINE_MUTEX(memleak_lock)
DEFINE_CRUCIAL_MUTEX(memleak_lock)
defFile LuceneBase_Files(false,true); //list of filenames used
defPointer LuceneBase_Pointers(false,true); //list of pointers counted
defVoid LuceneBase_Voids(false,true); //list of arbitary data added
#endif


//variables to trim filenames to just the base names
char _files_trim_string[CL_MAX_DIR];
int32_t _files_trim_start=-1;

//trim the filename and return the refcounted _file* structure
_file* get_file(const char* file){
	if ( _files_trim_start == -1 ){
		//this trims the start of the name file name so
		//that the whole of the filename is not stored - more asthetic :)
		//need to find the base
		_files_trim_start = strlen(__FILE__) - 21; //(length of debug/memtracking.cpp)
		// NOTE: There is zero guarantee to what __FILE___ will return as it may not be
		// supported by all compilers. 
		_files_trim_start = _files_trim_start > 0 ? _files_trim_start : 0;
		strcpy(_files_trim_string,__FILE__);
		_files_trim_string[_files_trim_start] = 0;
	}
	if ( strncmp(file,_files_trim_string,_files_trim_start) == 0 ){
		//this file should be within the same directory area as we found lucenebase.cpp
		//to be, lets trim the start
		file+=_files_trim_start;
	}

   //now return an existing files structure (with refcount++) or create a new one
   defFile::iterator itr = LuceneBase_Files.find((const char*)file);
   if ( itr != LuceneBase_Files.end() ){
      _file* bf = itr->second;
      bf->refcount++;
      return bf;
   }else{
      _file* ref = new _file;
      ref->value = new char[strlen(file)+1]; //cannot use _CL_NEWARRAY otherwise recursion
	  strcpy(ref->value,file);

      ref->refcount = 1;
	  LuceneBase_Files.insert(pair<const char*,_file*>(ref->value,ref));
	  return ref;
   }
}

void remove_file(_file* bf){
	bf->refcount--;
    if ( bf->refcount <= 0 ){
			defFile::iterator fi = LuceneBase_Files.find(bf->value);
			CND_PRECONDITION(fi!=LuceneBase_Files.end(),"fi==NULL");
			delete[] bf->value;
	    LuceneBase_Files.removeitr(fi);
    }
}

#ifdef LUCENE_ENABLE_CONSTRUCTOR_LOG
	void constructor_log(const char* type,const char* file,const int line, const int size){
		if ( _lucene_disable_debuglogging ){
			FILE* f = fopen("clucene.log","a");
			char buf[CL_MAX_DIR+5];
			sprintf(buf,"%s,%s,%d,%d\n",type,file,line,size);
			fwrite(buf,sizeof(char),strlen(buf),f);
			fclose(f);
		}
	}
	#define CONSTRUCTOR_LOG(type,file,line,size) constructor_log(type,file,line,size)
#else
	#define CONSTRUCTOR_LOG(type,file,line,size)
#endif

////////////////////////////////////////////////////////////////////////////////
// the _CLNEW&_CLDELETE new/delete operators
////////////////////////////////////////////////////////////////////////////////
void* LuceneBase::operator new (size_t size, const char * file, int32_t line)
{
    // CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

   void* p = malloc (size);
   LuceneBase* lb = (LuceneBase*)p;

   //create the pointer struct
   _file* br = get_file(file);
   _pointers* bp = new _pointers;
   bp->file = br;
   bp->initnumber = _instance_counter++;
   bp->initline = line;

   //associate this object with the pointer
   lb->__cl_initnum = bp->initnumber;

   //break if necessary
	if ( _lucene_counter_break == lb->__cl_initnum )
		CLDebugBreak(); //put break point here

	//add the pointer object
	LuceneBase_Pointers.insert(pair<LuceneBase*,_pointers*>(lb, bp));

	CONSTRUCTOR_LOG("newobj",file,line,size);

#ifdef DBG_MEM_USAGE
        MeasureDynMemUsage();
#endif

   return p;
} 
void LuceneBase::operator delete (void *p, char const * file, int32_t line)
{
    // CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

	LuceneBase* lb=(LuceneBase*)p;

	defPointer::iterator itr = LuceneBase_Pointers.find(lb);
    if ( itr != LuceneBase_Pointers.end() ){
      _pointers* bp = itr->second;
	  remove_file(bp->file);

      LuceneBase_Pointers.removeitr(itr);
    }else{
       //break
    }
	free(p); 
} 

///////////////////////////////////////////////////////////////////////////
// the generic new/delete operators
///////////////////////////////////////////////////////////////////////////
void* LuceneBase::operator new (size_t size)
{
    // CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

	void* p = malloc (size);
	LuceneBase* lb = (LuceneBase*)p;

	//create the pointer struct
   _file* br = get_file("undefined");
   _pointers* bp = new _pointers;
   bp->file = br;
   bp->initnumber = _instance_counter++;
   bp->initline = -1;

   //associate this object with the pointer
   lb->__cl_initnum = bp->initnumber;

   //break if necessary
	if ( _lucene_counter_break == lb->__cl_initnum )
		CLDebugBreak();

	//add the pointer object
	LuceneBase_Pointers.insert(pair<LuceneBase*,_pointers*>(lb,bp));
	
	CONSTRUCTOR_LOG("newobj","unknown",-1,size);

#ifdef DBG_MEM_USAGE
        MeasureDynMemUsage();
#endif

   return p;
} 


#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)

const nomemtrace_t nomemtrace = {};

void* LuceneBase::operator new (size_t size, const nomemtrace_t&)
{
    // return malloc (size); 

    void
        * rv = malloc(size);

    LuceneBase* lb = (LuceneBase*)rv;
    lb->__cl_initnum = _instance_counter++;

#ifdef DBG_MEM_USAGE
        MeasureDynMemUsage();
#endif

        return rv;
} 

void LuceneBase::operator delete (void *p, const nomemtrace_t&) 
{
	free(p); 
}

#endif 

void LuceneBase::operator delete (void *p)
{
    //CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

	LuceneBase* lb=(LuceneBase*)p;

	defPointer::iterator itr = LuceneBase_Pointers.find(lb);
	if ( itr != LuceneBase_Pointers.end() ){
		_pointers* bp = itr->second;
		remove_file(bp->file);
		LuceneBase_Pointers.removeitr(itr);
	}else{
		CLDebugBreak();
	}
	free(p); 
}

///////////////////////////////////////////////////////////////////////////
// other memtracking functions
///////////////////////////////////////////////////////////////////////////
void LuceneBase::__cl_unregister(const void* obj){
    //CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

	LuceneBase* lb=(LuceneBase*)obj;
	defPointer::iterator itr = LuceneBase_Pointers.find(lb);
	CND_PRECONDITION(itr != LuceneBase_Pointers.end(),"__cl_unregister object not found");
	_pointers* bp = itr->second;
    LuceneBase_Pointers.removeitr(itr);
}

void* LuceneBase::__cl_voidpadd(void* data, const char* file, int line,size_t size){
    // CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

	_file* br = get_file(file);
	_pointers* bp = new _pointers;
	bp->file = br;
	bp->initnumber = _instance_counter++;
	bp->initline = line;

	LuceneBase_Voids.insert(pair<void*,_pointers*>(data,bp));
	CONSTRUCTOR_LOG("newarr",file,line,size);
	return data;
}
void LuceneBase::__cl_voidpremove(const void* data, const char* file, int line){
    //CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)
	defVoid::iterator itr = LuceneBase_Voids.find(data);
    if ( itr != LuceneBase_Voids.end() ){
      _pointers* bp = itr->second;
      remove_file(bp->file);
      LuceneBase_Voids.removeitr(itr);
    }else{
       printf("Data deleted when not added with _CL_NEWARRAY in %s at %d\n",file,line);
    } 
}


////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
//The lucene base memory leak printout functions
////////////////////////////////////////////////////////////
//static
void __internalcl_PrintUnclosedObject(bool isObject, string& sb,_pointers* bp,_file* bf, bool print){
	TCHAR ttmp[100];
	char atmp[100];
	
	sb.append("   ");
	{
		_i64tot(bp->initnumber,ttmp,10);
		STRCPY_TtoA(atmp,ttmp,100);
		sb.append(atmp);
	}
	if ( isObject ){
		sb.append("(obj). ");
	}else{
		sb.append(". ");
	}
	sb.append(bf->value);
	sb.append(", line ");
	{
		_i64tot(bp->initline,ttmp,10);
		STRCPY_TtoA(atmp,ttmp,100);
		sb.append(atmp);
	}
	sb.append("\n");

	if ( print && sb.length() > 0 ){
		printf("%s\n", sb.c_str());
		sb = "";
	}
}
char* __internalcl_GetUnclosedObjects(bool print){
	TCHAR ttmp[100];
	char atmp[100];
	// CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
        SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

	string sb;
    bool unknowns = false;
	if ( LuceneBase_Pointers.size() > 0 ){
		{
			_i64tot(LuceneBase_Pointers.size(),ttmp,10);
			STRCPY_TtoA(atmp,ttmp,100);
			sb.append(atmp);
		}
		sb.append(" clucene objects are still open\n");

		defPointer::iterator itr = LuceneBase_Pointers.begin();
		while ( itr != LuceneBase_Pointers.end() ){
			_pointers* bp = itr->second;
			_file* bf = bp->file;

			if ( bp->initline == -1 )
				unknowns = true;
			__internalcl_PrintUnclosedObject(true, sb,bp,bf,print);

			++itr;
		}

		defVoid::iterator itr2 = LuceneBase_Voids.begin();
		while ( itr2 != LuceneBase_Voids.end() ){
			_pointers* bp = itr2->second;
			_file* bf = bp->file;

			if ( bp->initline == -1 )
				unknowns = true;
			__internalcl_PrintUnclosedObject(false, sb,bp,bf,print);

			itr2++;
		}
	}	

	if ( unknowns == true ){
		sb.append("*** Some memory was not created with _CLNEW and was not tracked... ***\n");
		sb.append("*** Use _CLNEW instead of new when creating CLucene objects ***\n");
		sb.append("*** Memory may also have not been freed in the current context ***\n");
	}
	
	if ( print ){
		if ( sb.length() > 0 ){
			printf("%s\n", sb.c_str());
			sb = "";
		}
		return NULL;
	}else{
		if ( sb.length() > 0 )
			return STRDUP_AtoA(sb.c_str());
		else
			return NULL;
	}
}

void LuceneBase::__cl_ClearMemory(){
    //CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

	while ( LuceneBase_Files.size() > 0 ){
		defFile::iterator fi = LuceneBase_Files.begin();
		_file* f = fi->second;
		delete[] f->value;
        LuceneBase_Files.removeitr (fi);
	}
   LuceneBase_Pointers.clear();
   LuceneBase_Voids.clear();
}
char* LuceneBase::__cl_GetUnclosedObjects(){
	return __internalcl_GetUnclosedObjects(false);
}
//static
int32_t LuceneBase::__cl_GetUnclosedObjectsCount(){
    // CPIXASYNC
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)
    return LuceneBase_Pointers.size();
}

// CPIXASYNC const char* LuceneBase::__cl_GetUnclosedObject(int32_t item){
void LuceneBase::__cl_GetUnclosedObject(int32_t   item,
                                        char    * dest,
                                        size_t    destSize){
    // CPIXASYNC SCOPED_LOCK_MUTEX(memleak_lock)
    SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)

   defPointer::iterator itr=LuceneBase_Pointers.begin();
   int32_t i=0;
   for ( ;itr!=LuceneBase_Pointers.end() && i<item ;itr++ ){
      ++i;
   }
   if ( itr != LuceneBase_Pointers.end() )
       // CPIXASYNC return itr->second->file->value;
       {
           strncpy(dest,
                   itr->second->file->value,
                   destSize - 1);
           dest[destSize-1] = '\0';
       }
   else
       // CPIXASYNC return NULL;
       {
           dest[0] = '\0';
       }
}
void LuceneBase::__cl_PrintUnclosedObjects(){
	__internalcl_GetUnclosedObjects(true);
}
#if defined (__SYMBIAN32__) && defined (RVCT22_STATICS_WORKAROUND)
void _lucene_free_memtrace_infra() {
    { // scope to make the mutex unlocked before its destroyed
        // CPIXASYNC
        SCOPED_LOCK_CRUCIAL_MUTEX(memleak_lock)
            
        StaticInstances::shutdown(&LuceneBase_Files); 
	StaticInstances::shutdown(&LuceneBase_Pointers); 
	StaticInstances::shutdown(&LuceneBase_Voids); 
    }

#if !defined (_CL_DISABLE_MULTITHREADING)  || defined (CPIX_ASYNC_MT)
        // held by current thread, should be able to destroy it
        StaticInstances::shutdown(&CL_NS(debug)::memleak_lock);
#endif
}
#endif

////////////////////////////////////////////////////////////

#endif //LUCENE_ENABLE_MEMLEAKTRACKING
CL_NS_END
