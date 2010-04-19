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

#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <string>
#include <set>
#include <map>
#include <wchar.h>
#include <math.h> // float_t

#include "common/refcountedbase.h"

namespace lucene {
    namespace document {
        class Document;
        class DocumentFieldEnumeration;
        class Field;
    }
    namespace util {
        class Reader;
    }
}

namespace Cpix {
    class Document; 
}

namespace Cpix
{

    /**
     * Wrapper for lucene::document::Field.
     */
    class Field {
    public: 
		
        Field(const wchar_t * name,
              const wchar_t * value,
              int             configs);

        Field(const wchar_t * name,
              lucene::util::Reader* stream,
              int             configs);

        Field(lucene::document::Field* field,
              bool aggregate,
              bool own = false);

        ~Field(); 
		
        const wchar_t* name(); 
		
        lucene::document::Field* release(); 

        lucene::document::Field& native(); 

        int isBinary() const;

        int isIndexed() const;

        bool isAggregated()  const; 
		
        float_t boost() const;

        void setBoost(float_t boost);

        const wchar_t * stringValue() const;
	    
    protected: 
		
        void resolveConfig(int& config); 
	    
    private: 
		
        bool own_;
		
        lucene::document::Field* field_;
		
        bool aggregate_;
	
    };
	
    class DocumentFieldIterator {
    public: 
        DocumentFieldIterator(lucene::document::DocumentFieldEnumeration* fields,
                              Document &document); 
        ~DocumentFieldIterator(); 
        Field* operator++(int); 
        operator bool();
    private: 
        lucene::document::DocumentFieldEnumeration* fields_;
        Document& document_; 
    };



    /**
     * Wraps lucene::document::Document instances.
     */
    class Document : public RefCountedBase {
    public: 
	
        /**
         * Constructs a document wrapping a newly created lucene
         * document based on the arguments given here. Use case:
         * during indexing.
         */
        Document(const wchar_t * docUid,
                 const char    * appClass,
                 const wchar_t * excerpt,
                 const wchar_t * mimeType);

        /**
         * Constructing a document wrapping an existing lucene
         * document. Use case: during search. The exiting lucene
         * document may be owned by a lucene party (in which case we
         * clone it), or it might be owned by a cpix entity
         * (HitsDocumentList) in which case we don't clone it. The
         * whole point is that clucene should be able to destroys
         * everything it owns during certain operations without
         * breaking clients.
         */
        Document(lucene::document::Document * docUid,
                 bool                         ownedByClucene);
		
        ~Document(); 
		
    public: 
        /**
         * 
         */
        lucene::document::Document& native(); 

        void setDocUid(const wchar_t* docUid);
        void setAppClass(const char* appClass);
        void setExcerpt(const wchar_t* excerpt);
        void setMimeType(const wchar_t* mimeType);

        bool isSystemField(const wchar_t* fieldName); 
		
        /**
         * Ownership is transferred
         */
        void add(Field* field); 
		
        void removeField( const wchar_t* fieldName ); 

        const wchar_t* get( const wchar_t* fieldName ) const; 
		
        DocumentFieldIterator* fields(); 

        float_t boost() const;

        void setBoost(float_t boost);

        void clear();

    protected: // For friend DocumentFieldIterator 
		
        friend class DocumentFieldIterator; 

        bool isAggregated(const std::wstring& fieldName) const; 

        Field* wrapField(lucene::document::Field* field); 
	    
    private:

        typedef std::map<lucene::document::Field*, Field*> FieldWrapperMap;         

        lucene::document::Document* document_; 
		
        /**
         * No aggregation for fields with given names
         */
        std::set<std::wstring> aggregate_;
        
        /**
         * A pool for field wrappers
         */
        FieldWrapperMap fieldWrappers_;
        		
        bool own_;

    };
}

#endif /* DOCUMENT_H_ */
