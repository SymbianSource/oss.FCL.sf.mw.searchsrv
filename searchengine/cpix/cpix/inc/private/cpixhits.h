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

#ifndef CPIX_CPIXHITS_H
#define CPIX_CPIXHITS_H


#include <stdlib.h>

#include <map>

#include "cpixinit.h"
#include "cpixerror.h"
#include "cpixdoc.h"
#include "cpixidxdb.h"

#include "CLucene.h"

#include "cpixsynctools.h"

#include "fwdtypes.h"
#include "common/refcountedbase.h"
#include "documentconsumer.h"


namespace Cpix
{
    class Document;
    class Document;

    /**
     * Hits represents a ranked list of documents, matching some query
     * or condition. This object is used for both representing
     * documents matching a search query but also for representing
     * terms returned by suggest request. Note that the hits object
     * may become invalid (and need recommit) if their originating idx
     * db has been modified.
     */
    class IHits : public RefCountedBase
    {
    public:
		
        /**
         *  Destructor 
         */
        virtual ~IHits(); 
		
        // Note: There is no const version of getDocument,
        // because getting document may change the internal
        // state of the object (references are converted into
        // real objects)
        /**
         * Provides access to the document at given
         * index. Note that the documents are ranked with
         * index 0 containing best match and index at length-1
         * having the worst match.
         */
        virtual Document * getDocument(int32_t index) = 0;
	
        /**
         * Returns the amount of hits
         */
        virtual int32_t length() = 0;

    };
    


    /**
     * There are two "real" implementations to interface IHits, but
     * they do share a bit of common functionality, hence this base
     * class.
     *
     * The common functionality is this: neither of the IHits real
     * implementations will own the lucene::document::Document
     * instance they keep returning wrapped by Cpix::Document wrapper
     * instances. But, someone has to own the wrappers, and it has to
     * be the hits instance. So there is a common code for managing
     * the lifetime of Cpix::Document wrappers in both
     * implementations.
     */
    class HitsBase : public IHits
    {
    private:
        //
        // private members
        //
        bool       docsOwnedByClucene_;


    protected:
        //
        // Someone has to own the Cpix::Document wrapper instances,
        // and this someone can be only this instance
        //
        typedef std::map<int32_t, Document*> WrapperMap;
        WrapperMap                           wrappers_;

        void destroyWrappers();

        
        /**
         * Implementors just have to make sure that the correct lucene
         * document is found and wrapped with a call to wrapDocument.
         *
         * They are allowed to fetch more and wrap them.
         *
         * If the call succeeds, the wrapped document MUST be in the
         * wrappers_ collection. Otherwise, it MUST throw.
         */
        virtual void getDocument_(int32_t index) = 0;


    public:
        /**
         * Wraps and stores the wrapped document at a given index.
         */
        void wrapDocument(int32_t                      index,
                          lucene::document::Document * doc);


        Document * getDocument(int32_t index);


        /**
         * The documents this hitsbase will keep wrapping can be
         * (a) owned by clucene
         * (b) owned by CPix
         */
        HitsBase(bool docsOwnedByClucene);
    };



    /**
     * DocumentConsumer for LuceneHits class:
     * ClhDocumentConsumer. This class helps LuceneHits instance to
     * pump pages of documents in one go (because locking idxdb /
     * multiidxdb instances can be very timeconsuming and recommitting
     * a search is just plain horrible).
     */
    class ClhDocumentConsumer : public DocumentConsumer
    {
    public:
        enum 
            { 
                UNUSED    = -1
            };

    private:
        //
        // private
        //
        const size_t       pageSize_;

        int32_t            beginIndex_;
        int32_t            endIndex_;

        LuceneHits      & clHits_;

        std::vector<int>   failedDocErrors_;

    public:
        //
        // public operations
        //


        //
        // from base class DocumentConsumer
        //
        virtual int32_t beginIndex() const;
        virtual int32_t endIndex() const;
        virtual void useDocument(int32_t                      index,
                                 lucene::document::Document * document);
        virtual void failedDocument(int32_t        index,
                                    int            clErrorNumber);


        //
        // own operations
        //
        
        /**
         * Constructor
         *
         * @param clHits the LuceneHits instance that this object
         * will pump documents to.
         */
        ClhDocumentConsumer(LuceneHits & clHits);


        /**
         * The owner LuceneHits is forced to load hit doc at index
         * 'index'. This instance needs to initialize the page
         * boundaries and other internal variables accordingly. Sanity
         * of index is also checked here (should not be ge then length
         * of hits).
         *
         * @param index the index to load, plus a number of other hit
         * docs around it in the same page (defined by PAGE_SIZE).
         */
        void setPageBoundaryForIndex(int32_t index);

        
        /**
         * When a document is not found wrapped by HitsBase, it will
         * try loading it. However, since this instance loads multiple
         * docs at the same time (in pages), it can happen that a
         * document load has been attempted, but failed. Such failure
         * is reported by this call, if there was any.
         */
        void throwIfFailedDocIndex(int32_t index);
    };



    /**
     * Repsesents hits, that is a ranked list of documents
     * matching a search query. The implementation encapsulates
     * CLucene Hits object, and the object merely provides an
     * interface to those results.
     * 
     * Note that CLucene Hits object contains references to the
     * underlying database in the filesystem.  If the underlying
     * database changes, the references contained by this Hits
     * object becames invalid thus invalidating the object.
     */
    class LuceneHits : public HitsBase 
    {
    private:
        //
        // private members
        //
        /** pointer to native (CLucene) impl, owned by the idxDb_
    	 */
    	lucene::search::Hits         * hits_; 

        int32_t                        length_;

        lucene::search::Query        * query_;
        
        //
        // In order to be able to sensibly invalidate actions on
        // objects that may not exist or are in an insensible state,
        // we track the Cpix::IdxDb instance these hits were yielded
        // by and whether the hits instance pointer to by ptr_ is
        // still valid.
        //
        IdxDbHndl                      idxDb_;

        Version                        idxDbVersion_;

        // the class that will pump batches of documents from hits
        friend class ClhDocumentConsumer;

        // the document consumer instance through which batches of hit
        // docs will be pumped to this hits instance to be stored in
        // wrapped form.
        ClhDocumentConsumer          * docConsumer_;

    public: 
        /**
         * Constructor. 
         */
        LuceneHits(lucene::search::Hits  * hits,
                    lucene::search::Query * query,
                    IdxDbHndl               idxDb,
                    Version                 idxDbVersion); 

        /**
         * ~LuceneHits
         */
        virtual ~LuceneHits(); 
		
    public: // from Hits
		
        virtual int32_t length();

    protected: 
    	
        virtual void getDocument_(int32_t index);        
    private: 

        /**
         * Releases used resources. Called on invalidation and destruction
         */
        void release(); 
    };

    /**
     * Represents hits that is a ranked list of documents. This
     * implementation owns the document list and it does contain
     * external references thus it cannot become invalid.
     */
    class HitDocumentList : public HitsBase
    {
    public: 

        /**
         * Constructs an empty hit document
         */
        HitDocumentList(); 

        /**
         * Destroys the object
         */
        virtual ~HitDocumentList(); 
		
    public: // from Hits
		
        virtual int32_t length();

    public: // For manipulating the object
		
        /**
         * Appends the object to the end of the hits list. Transfers
         * ownership
         */
        void add(lucene::document::Document* document); 
		
        /**
         * Destroys the document at given index
         */
        void remove(int index); 

    protected:
        virtual void getDocument_(int32_t index);

		
    private: 
		
        /**
         * The list of documents. All the listed documents are owned
         * by this object
         */
        std::vector<lucene::document::Document*> documents_; 
    };



}

#endif // CPIX_CPIXHITS_H
