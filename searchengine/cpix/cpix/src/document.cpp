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

#include "document.h"
#include "cpixexc.h"
#include "cpixdoc.h"

#include "idxdb.h"
#include "idxdbmgr.h"

#include "CLucene.h"

#include "indevicecfg.h"
#include "cpixstrtools.h"

#include "common/cloners.h"

namespace Cpix
{
    Field::Field(const wchar_t * name,
                 const wchar_t * value,
                 int             configs) 
        :	own_(true), field_(0) {
        resolveConfig(configs); 
        field_ = _CLNEW lucene::document::Field(name, value, configs); 		 
    }
	

    Field::Field(const wchar_t * name,
                 lucene::util::Reader* stream,
                 int             configs) 
	:  own_(true), field_(0) {
        resolveConfig(configs); 
        field_ = _CLNEW lucene::document::Field(name, stream, configs); 		 
    }


    void Field::resolveConfig(int& configs)
    {
        if (configs & cpix_AGGREGATE_NO & cpix_AGGREGATE_YES) {
            THROW_CPIXEXC("Field is marked to be both aggregated and not aggregated" );
        } else if (configs & cpix_AGGREGATE_NO) {
            aggregate_ = false; 
            configs &= ~cpix_AGGREGATE_NO; // erase it from config
        } else if (configs & cpix_AGGREGATE_YES) {
            aggregate_ = true; 
            configs &= ~cpix_AGGREGATE_YES; // erase it from config
        } else {
            // Aggregate indexed fields by default. 
            aggregate_ = !(configs & cpix_INDEX_NO);
        }
    }
				    

    Field::Field(lucene::document::Field* field,
                 bool aggregate,
                 bool own)	
	: own_(own),
      field_( field ), 
	  aggregate_( aggregate ) 
	   {
    }
	
    Field::~Field() {
        if (own_) {
            _CLDELETE( field_ );
        }
    }
	
    const wchar_t* Field::name() {
        return field_->name(); 
    }

    lucene::document::Field* Field::release() {
        if (own_) {
			own_ = false; 
            return field_; 
        } else {
            THROW_CPIXEXC("Native CLucene field, which is not owned by CPix::Field, cannot be released");
        }
    }

    lucene::document::Field& Field::native() {
        return *field_;; 
    }

    int Field::isBinary() const {
        return field_->isBinary(); 
    }
	
    int Field::isIndexed() const {
        return field_->isIndexed(); 
    }

    bool Field::isAggregated() const {
        return aggregate_;
    }
	
    float_t Field::boost() const {
        return field_->getBoost(); 
    }

    void Field::setBoost(float_t boost) {
        field_->setBoost(boost); 
    }

    const wchar_t* Field::stringValue() const {
        return field_->stringValue();
    }
	
    DocumentFieldIterator::DocumentFieldIterator(lucene::document::DocumentFieldEnumeration* fields, Document &document) 
	: 	fields_(fields), 
                document_(document){
    }
	
    DocumentFieldIterator::~DocumentFieldIterator() {
        _CLDELETE( fields_ );
    }
		
    Field* DocumentFieldIterator::operator++(int) {
        return document_.wrapField( fields_->nextElement() );
    }
	
    DocumentFieldIterator::operator bool() {
        return fields_->hasMoreElements();
    }

	
    Document::Document(const wchar_t * docUid,
                       const char    * appClass,
                       const wchar_t * excerpt,
                       const wchar_t * mimeType) 
	: document_( 0 ),
	  fieldWrappers_(), 
      own_(true)
    {
        if (docUid == NULL || *docUid == 0) {
            THROW_CPIXEXC("Document UID must be defined");
        }
		
        document_ = _CLNEW lucene::document::Document(); 

        setDocUid(docUid);
		
        if (appClass != NULL && *appClass != 0) {
            setAppClass(appClass); 
        }
		
        if (excerpt != NULL && *excerpt != 0) {
            setExcerpt(excerpt); 
        }

        if (mimeType != NULL && *mimeType != 0) {
            setMimeType(mimeType); 
        }
    }

    Document::Document(lucene::document::Document * document,
                       bool                         ownedByClucene) 
	: document_( document ), 
	  fieldWrappers_(), 
	  own_(ownedByClucene) {
        if (own_)
            {
                std::auto_ptr<lucene::document::Document>
                    clone(Clone(document));
                document_ = clone.get();
                clone.release();
            }
    }

    Document::~Document() 
    {
		// Release document, if owned (e.g. not owned by hit list) 
        if (own_)
        {
			_CLDELETE(document_);
        }
        
        
        // Clean up CLucene field wrappers
        FieldWrapperMap::iterator
            i = fieldWrappers_.begin(),
            end = fieldWrappers_.end();

        for (; i != end; ++i)
        {
            delete i->second;
        }
    }
	
    bool Document::isSystemField(const wchar_t* name) {
        return wcscmp(name, LCPIX_DOCUID_FIELD) == 0
            || wcscmp(name, LCPIX_APPCLASS_FIELD) == 0
            || wcscmp(name, LCPIX_UNTOKENIZED_APPCLASS_FIELD) == 0
            || wcscmp(name, LCPIX_EXCERPT_FIELD) == 0
            || wcscmp(name, LCPIX_MIMETYPE_FIELD) == 0;
    }
	
    void Document::setDocUid(const wchar_t* docUid) {
        removeField(LCPIX_DOCUID_FIELD); 
        document_->add(*_CLNEW lucene::document::Field( LCPIX_DOCUID_FIELD,
                                                        docUid,
                                                        lucene::document::Field::STORE_YES | 
                                                        lucene::document::Field::INDEX_UNTOKENIZED));
    }
	
    void Document::setAppClass(const char* appClass) {
        removeField(LCPIX_APPCLASS_FIELD);
        removeField(LCPIX_UNTOKENIZED_APPCLASS_FIELD); 
        if (Cpix::IdxDbMgr::isQualBaseAppClass(appClass)) {
            THROW_CPIXEXC("Qualification on app class field '%s'",
                          appClass);
        }
	
        size_t
            wAppClassBufSize = strlen(appClass) + 1;
        /* OBS
        Cpt::auto_array<wchar_t>
            wAppClass(new wchar_t[wAppClassBufSize]);
        mbstowcs(wAppClass.get(),
                 appClass,
                 wAppClassBufSize);
        */
        Cpt::auto_array<wchar_t>
            wAppClass(appClass);

        document_->add(*_CLNEW lucene::document::Field(LCPIX_APPCLASS_FIELD,
                                                       wAppClass.get(),
                                                       lucene::document::Field::STORE_YES 
                                                       | lucene::document::Field::INDEX_TOKENIZED));
		
        document_->add(*_CLNEW lucene::document::Field(LCPIX_UNTOKENIZED_APPCLASS_FIELD,
                                                       wAppClass.get(),
                                                       lucene::document::Field::STORE_NO
                                                       | lucene::document::Field::INDEX_UNTOKENIZED));
    }

    void Document::setExcerpt(const wchar_t* excerpt) {
        removeField(LCPIX_EXCERPT_FIELD);
        document_->add(*_CLNEW lucene::document::Field(LCPIX_EXCERPT_FIELD,
                                                       excerpt,
                                                       lucene::document::Field::STORE_YES 
                                                       | lucene::document::Field::INDEX_NO));
    }
	
    void Document::setMimeType(const wchar_t* mimeType) {
        removeField(LCPIX_MIMETYPE_FIELD); 
        document_->add(*_CLNEW lucene::document::Field(LCPIX_MIMETYPE_FIELD,
                                                       mimeType,
                                                       lucene::document::Field::STORE_YES 
                                                       | lucene::document::Field::INDEX_UNTOKENIZED));
    }

    lucene::document::Document& Document::native() {
        return *document_;
    }

    void Document::removeField( const wchar_t* fieldName ) {
        lucene::document::Field* field = document_->getField( fieldName ); 
        if ( field )
            {
                if (fieldWrappers_.find(field) != fieldWrappers_.end())
                    {
                        delete fieldWrappers_[field];
                        fieldWrappers_.erase(field); 
                    }
            }
        
        document_->removeField( fieldName ); 
        if (wcscmp(fieldName, LCPIX_APPCLASS_FIELD) == 0) {
            document_->removeField( LCPIX_UNTOKENIZED_APPCLASS_FIELD ); 
        }
    }
	
    void Document::add(Field* field) {
        if (isSystemField(field->name())) {
            THROW_CPIXEXC("Document::add() must not used to modify system field %s", field->name());
        }
        if (document_->getField(field->name())) {
            removeField(field->name()); 
        }
        if (field->isAggregated()) {
            aggregate_.insert(std::wstring(field->name()));
        }
        
        // store the field as a wrapper
        fieldWrappers_[&field->native()] = field; 

        // add the native field 
		document_->add( *field->release() );
    }
	
    const wchar_t* Document::get(const wchar_t* fieldName) const {
        return document_->get(fieldName); 
    }

    DocumentFieldIterator* Document::fields() {
        return new DocumentFieldIterator( document_->fields(), *this ); 
    }
	
    float_t Document::boost() const {
        return document_->getBoost(); 
    }
	
    void Document::setBoost(float_t boost) {
        document_->setBoost( boost ); 
    }
	
    void Document::clear() {
        document_->clear(); 
    }
	
    bool Document::isAggregated(const std::wstring& fieldName) const {
        return aggregate_.count(fieldName);
    }
	
    Field* Document::wrapField(lucene::document::Field* field) {
		if (fieldWrappers_.find(field) == fieldWrappers_.end())
			{
			fieldWrappers_[field] =         		
				new Field(field, 
						  isAggregated(field->name()));
			}

		return fieldWrappers_[field];
    }

}
