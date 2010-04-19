/*------------------------------------------------------------------------------
* Copyright (C) 2003-2006 Ben van Klinken and the CLucene Team
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/
#ifndef _lucene_util_Reader_
#define _lucene_util_Reader_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/util/streambase.h"
#include "CLucene/util/stringreader.h"
#include "CLucene/util/fileinputstream.h"
#include "CLucene/util/bufferedstream.h"

CL_NS_DEF(util)
/**
* An inline wrapper that reads from Jos van den Oever's jstreams
*/
class Reader:LUCENE_BASE {
typedef jstreams::StreamBase<TCHAR> jsReader;
public:
	bool deleteReader;
	jsReader* reader;

	Reader(jsReader* reader, bool deleteReader){
		this->reader = reader;
		this->deleteReader = deleteReader;
	}
	virtual ~Reader(){
		if ( deleteReader )
			delete reader;
		reader = NULL;
	}
	inline int read(){
            // AGGREGATE-FIELD
            initialize();

		const TCHAR*b;
		int32_t nread = reader->read(b, 1,1);
		if ( nread < -1 ) //if not eof
			_CLTHROWA(CL_ERR_IO,reader->getError() );
		else if ( nread == -1 )
			return -1;
		else
			return b[0];
	}
	/**
	* Read at least 1 character, and as much as is conveniently available
	*/
	inline int32_t read(const TCHAR*& start){
            // AGGREGATE-FIELD
            initialize();

		int32_t nread = reader->read(start,1,0);
		if ( nread < -1 ) //if not eof
			_CLTHROWA(CL_ERR_IO,reader->getError());
		else
			return nread;
	}
	inline int32_t read(const TCHAR*& start, int32_t len){
            // AGGREGATE-FIELD
            initialize();

		int32_t nread = reader->read(start, len, len);
		if ( nread < -1 ) //if not eof
			_CLTHROWA(CL_ERR_IO,reader->getError());
		else
			return nread;
	}
	inline int64_t skip(int64_t ntoskip){
            // AGGREGATE-FIELD
            initialize();

		int64_t skipped = reader->skip(ntoskip);
		if ( skipped < 0 ) 
			_CLTHROWA(CL_ERR_IO,reader->getError());
		else
			return skipped;
	}
	inline int64_t mark(int32_t readAheadlimit){
            // AGGREGATE-FIELD
            initialize();

		int64_t pos = reader->mark(readAheadlimit);
		if ( pos < 0 )
			_CLTHROWA(CL_ERR_IO,reader->getError());
		else
			return pos;
	}
	int64_t reset(int64_t pos){
            // AGGREGATE-FIELD
            initialize();

		int64_t r = reader->reset(pos);
		if ( r < 0 )
			_CLTHROWA(CL_ERR_IO,reader->getError());
		else
			return r;
	}

protected:
    // AGGREGATE-FIELD
    virtual void initialize() { ; }
};

///A helper class which constructs a the jstreams StringReader.
class StringReader: public Reader{
public:
	StringReader ( const TCHAR* value );
    StringReader ( const TCHAR* value, const int32_t length );
    StringReader ( const TCHAR* value, const int32_t length, bool copyData );
	~StringReader();
};

/** A very simple inputstreamreader implementation. For a
* more complete InputStreamReader, use the jstreams version
* located in the contrib package
*/
class SimpleInputStreamReader: public jstreams::BufferedInputStream<TCHAR>{
    int32_t decode(TCHAR* start, int32_t space);
	int encoding;
	enum{
		ASCII=1,
		UTF8=2,
		UCS2_LE=3
	};
    bool finishedDecoding;
    jstreams::StreamBase<char>* input;
    int32_t charsLeft;

    jstreams::InputStreamBuffer<char> charbuf;
    int32_t fillBuffer(TCHAR* start, int32_t space);
public:
	SimpleInputStreamReader(jstreams::StreamBase<char> *i, const char* encoding);
	~SimpleInputStreamReader();
};

/**
* A helper class which constructs a FileReader with a specified
* simple encodings, or a given inputstreamreader
*/
class FileReader: public Reader{
	jstreams::FileInputStream* input;
public:
	FileReader ( const char* path, const char* enc,
        const int32_t cachelen = 13,
        const int32_t cachebuff = 14 ); //todo: optimise these cache values
	~FileReader ();

    /* REMOVED by FDOSA
     * rationale: not needed, and anyway, these are not declared virtual
     * as they should have been. (Reader is also a botched base class...)
     *
    int32_t read(const TCHAR*& start, int32_t _min, int32_t _max);
    int64_t mark(int32_t readlimit);
    int64_t reset(int64_t);
    */
};

CL_NS_END
#endif
