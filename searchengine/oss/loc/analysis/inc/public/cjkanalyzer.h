/**
 * Copyright 2002-2004 The Apache Software Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef _lucene_analysis_cjk_cjkanalyzer_
#define _lucene_analysis_cjk_cjkanalyzer_

#if defined(_LUCENE_PRAGMA_ONCE)
# pragma once
#endif

#include "CLucene/analysis/AnalysisHeader.h"

CL_NS_DEF2(analysis,cjk)

/**
 * CJKTokenizer was modified from StopTokenizer which does a decent job for
 * most European languages. It performs other token methods for double-byte
 * Characters: the token will return at each two charactors with overlap match.<br>
 * Example: "java C1C2C3C4" will be segment to: "java" "C1C2" "C2C3" "C3C4" it
 * also need filter filter zero length token ""<br>
 * for Digit: digit, '+', '#' will token as letter<br>
 * for more info on Asia language(Chinese Japanese Korean) text segmentation:
 * please search  <a
 * href="http://www.google.com/search?q=word+chinese+segment">google</a>
 *
 * @author Che, Dong
 */
class CJKTokenizer: public CL_NS(analysis)::Tokenizer {
private:
	/** word offset, used to imply which character(in ) is parsed */
    int32_t offset;

    /** the index used only for ioBuffer */
    int32_t bufferIndex;

    /** data length */
    int32_t dataLen;

    /**
     * character buffer, store the characters which are used to compose <br>
     * the returned Token
     */
    TCHAR buffer[LUCENE_MAX_WORD_LEN];

    /**
     * I/O buffer, used to store the content of the input(one of the <br>
     * members of Tokenizer)
     */
    const TCHAR* ioBuffer;

    /** word type: single=>ASCII  double=>non-ASCII word=>default */
    const TCHAR* tokenType;

	static const TCHAR* tokenTypeSingle;
	static const TCHAR* tokenTypeDouble;

    /**
     * tag: previous character is a cached double-byte character  "C1C2C3C4"
     * ----(set the C1 isTokened) C1C2 "C2C3C4" ----(set the C2 isTokened)
     * C1C2 C2C3 "C3C4" ----(set the C3 isTokened) "C1C2 C2C3 C3C4"
     */
    bool preIsTokened;


	bool ignoreSurrogates;

public:
    /**
     * Construct a token stream processing the given input.
     *
     * @param in I/O reader
     */
	CJKTokenizer(CL_NS(util)::Reader* in);

	/**
     * Returns the next token in the stream, or null at EOS.
     * See http://java.sun.com/j2se/1.3/docs/api/java/lang/Character.UnicodeBlock.html
     * for detail.
     *
     * @return Token
     *
     * @throws java.io.IOException - throw IOException when read error <br>
     *         hanppened in the InputStream
     *
     */
	bool next(CL_NS(analysis)::Token* token);

	bool getIgnoreSurrogates(){ return ignoreSurrogates; };
	void setIgnoreSurrogates(bool ignoreSurrogates){ this->ignoreSurrogates = ignoreSurrogates; };
};

class CJKAnalyzer : public lucene::analysis::Analyzer {

public: 

	virtual TokenStream* tokenStream(const TCHAR* fieldName, CL_NS(util)::Reader* reader);
	
};

CL_NS_END2
#endif
