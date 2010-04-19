/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef SORTPROTO_IDX_H
#define SORTPROTO_IDX_H

#include <wchar.h>

#include <string>
#include <memory>


#define DOCUID    L"_docuid"
#define APPCLASS  L"_appclass"
#define APPCLASSTOKEN L"_appclasstoken"
#define DUMP      L"_dump"
#define AGGREGATE L"_aggregate"

#define DATETIME  L"DateTime"


template<typename APPDATA>
struct FieldDesc
{
    const wchar_t             * name_;
    const wchar_t * APPDATA:: * valueMember_;
    int                         flags_;
};




template<typename APPDATA>
class Idx
{
private:
    std::string                                    idxDbPath_;
    const FieldDesc<APPDATA>                     * schema_;

    lucene::analysis::standard::StandardAnalyzer   analyzer_;
    lucene::index::IndexWriter                     idxWriter_;
    

    // only to disable value semantics
    Idx(const Idx &);
    Idx & operator=(const Idx &);

public:
    Idx(const char               * idxDbPath,
        const FieldDesc<APPDATA> * schema)
        : idxDbPath_(idxDbPath),
          schema_(schema),
          idxWriter_(idxDbPath,
                     &analyzer_,
                     true)
    {
        ;
    }

    
    
    void indexItems(const APPDATA * appData)
    {
        for (; appData->docUid_ != NULL; ++appData)
            {
                indexItem(appData);
            }
    }


    ~Idx()
    {
        ;
    }


private:
    void indexItem(const APPDATA * appData)
    {
        using namespace std;
        using namespace lucene::document;

        auto_ptr<Document>
            doc(new Document());

        doc->add(* new Field(DOCUID,
                             appData->docUid_,
                             Field::STORE_YES | Field::INDEX_UNTOKENIZED));
        doc->add(* new Field(APPCLASS,
                             appData->appClass_,
                             Field::STORE_YES | Field::INDEX_TOKENIZED));
        doc->add(* new Field(APPCLASSTOKEN,
                             appData->appClass_,
                             Field::STORE_YES | Field::INDEX_UNTOKENIZED));
        doc->add(* new Field(DUMP,
                             DUMP,
                             Field::STORE_NO | Field::INDEX_UNTOKENIZED));

        wstring
            aggregate;

        indexAppData(doc.get(),
                     appData,
                     aggregate);

        doc->add(* new Field(AGGREGATE,
                             aggregate.c_str(),
                             Field::STORE_YES | Field::INDEX_TOKENIZED));

        if (appData->boost_ != 1.0)
            {
                doc->setBoost(appData->boost_);
            }
        
        idxWriter_.addDocument(doc.get());
        doc.reset();
    }


    void indexAppData(lucene::document::Document * doc,
                      const APPDATA              * appData,
                      std::wstring               & aggregate)
    {
        using namespace lucene::document;
        
        const FieldDesc<APPDATA>
            * fd = schema_;

        for (; fd->name_ != NULL; ++fd)
            {
                const wchar_t
                    * value = appData->*(fd->valueMember_);

                if (value != NULL)
                    {
                        doc->add(* new Field(fd->name_,
                                             value,
                                             fd->flags_));

                        if (aggregate.length() > 0)
                            {
                                aggregate += L' ';
                            }

                        aggregate += value;
                    }
            }
    }
};



typedef lucene::document::Field ldField;

/**************************************************
 *
 *     C A L E N D A R
 *
 */
struct CalendarData
{
    const wchar_t       * docUid_;
    const wchar_t       * appClass_;
    float_t               boost_;

    const wchar_t       * summary_;
    const wchar_t       * description_;
    const wchar_t       * location_;
    const wchar_t       * startDateTime_;
};

extern struct FieldDesc<CalendarData> CalendarSchema[];


/**************************************************
 *
 *     C O N T A C T
 *
 */
struct ContactData
{
    const wchar_t       * docUid_;
    const wchar_t       * appClass_;
    float_t               boost_;

    const wchar_t       * firstName_;
    const wchar_t       * secondName_;
    const wchar_t       * phoneNumber_;
    const wchar_t       * dateTime_;     // dummy
};


extern struct FieldDesc<ContactData> ContactSchema[];


// Contact Auxiliary Data

struct ContactAuxData
{
    const wchar_t       * docUid_;
    const wchar_t       * appClass_;
    float_t               boost_;

    const wchar_t       * refQbac_;
    const wchar_t       * refs_;
    const wchar_t       * dateTime_;
};


extern struct FieldDesc<ContactAuxData> ContactAuxSchema[];


/**************************************************
 *
 *     M E S S A G E
 *
 */
struct MessageData
{
    const wchar_t       * docUid_;
    const wchar_t       * appClass_;
    float_t               boost_;

    const wchar_t       * to_;
    const wchar_t       * from_;
    const wchar_t       * body_;
    const wchar_t       * dateTime_;
};

extern struct FieldDesc<MessageData> MessageSchema[];


#endif SORTPROTO_IDX_H
