/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

#ifndef CLUTIL_H_
#define CLUTIL_H_

#include "CLucene.h"

namespace analysis {

	/**
	 * Defines analyzer that uses given tokenizer T
	 */
    template <class T>
    class TemplateAnalyzer : public lucene::analysis::Analyzer {
        public:
            TemplateAnalyzer();
            virtual lucene::analysis::TokenStream* 
                tokenStream(const TCHAR* fieldName, 
                            CL_NS(util)::Reader* reader) {
                return new T(reader, true);
            }
    };

	/**
	 * Defines analyzer that uses given tokenizer T with an argument A1 
	 * combinged with filter F1. By using cpix analyzer definition syntax, 
	 * the defined analyzer is T(A1)>F1 
	 */
    template <class T, class A1, class F1>
    class TemplateAnalyzer1A1F : public lucene::analysis::Analyzer {
        public:
            TemplateAnalyzer1A1F(A1 a1) : a1_(a1) {};
            virtual lucene::analysis::TokenStream* 
                tokenStream(const TCHAR* fieldName, 
                            CL_NS(util)::Reader* reader) {
                return new F1(new T(reader, a1_), true);
            }
        private:
            A1 a1_;
    };
    
	/**
	 * Defines analyzer that uses given tokenizer T combined with filter F1. 
	 * By using cpix analyzer definition syntax, the defined analyzer is 
	 * T>F1. 
	 */
    template <class T, class F1>
    class TemplateAnalyzer1F : public lucene::analysis::Analyzer {
        public:
            TemplateAnalyzer1F() {};
            virtual lucene::analysis::TokenStream* 
                tokenStream(const TCHAR* fieldName, 
                            CL_NS(util)::Reader* reader) {
                return new F1(new T(reader), true);
            }
    };

}

#endif /* CLUTIL_H_ */
