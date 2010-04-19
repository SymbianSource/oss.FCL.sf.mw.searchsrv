/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include <CLucene.h>

#include <idx.h>


struct FieldDesc<CalendarData> CalendarSchema[] = {
    { L"Summary",
      &CalendarData::summary_,
      ldField::STORE_NO | ldField::INDEX_TOKENIZED
    },

    { L"Description",
      &CalendarData::description_,
      ldField::STORE_NO | ldField::INDEX_TOKENIZED
    },

    { L"Location",
      &CalendarData::location_,
      ldField::STORE_NO | ldField::INDEX_TOKENIZED
    },

    { DATETIME,
      &CalendarData::startDateTime_,
      ldField::STORE_YES | ldField::INDEX_UNTOKENIZED
    },

    { NULL,
      NULL,
      0
    }
};



struct FieldDesc<ContactData> ContactSchema[] = {
    { L"FirstName",
      &ContactData::firstName_,
      ldField::STORE_NO | ldField::INDEX_TOKENIZED
    },

    { L"SecondName",
      &ContactData::secondName_,
      ldField::STORE_NO | ldField::INDEX_TOKENIZED
    },
    
    { L"PhoneNumber",
      &ContactData::phoneNumber_,
      ldField::STORE_NO | ldField::INDEX_UNTOKENIZED
    },

    { L"DateTime",
      &ContactData::dateTime_,
      ldField::STORE_YES | ldField::INDEX_UNTOKENIZED
    },

    { NULL,
      NULL,
      0
    }
};


struct FieldDesc<ContactAuxData> ContactAuxSchema[] = {

    { L"_refqbac",
      &ContactAuxData::refQbac_,
      ldField::STORE_YES | ldField::INDEX_NO
    },

    { L"_refs",
      &ContactAuxData::refs_,
      ldField::STORE_YES | ldField::INDEX_NO
    },

    { DATETIME,
      &ContactAuxData::dateTime_,
      ldField::STORE_YES | ldField::INDEX_UNTOKENIZED
    },


    { NULL,
      NULL,
      0
    }

};


struct FieldDesc<MessageData> MessageSchema[] = {
    { L"To",
      &MessageData::to_,
      ldField::STORE_NO | ldField::INDEX_UNTOKENIZED
    },

    { L"From",
      &MessageData::from_,
      ldField::STORE_NO | ldField::INDEX_UNTOKENIZED
    },

    { L"Body",
      &MessageData::body_,
      ldField::STORE_NO | ldField::INDEX_TOKENIZED
    },
      
    { DATETIME,
      &MessageData::dateTime_,
      ldField::STORE_YES | ldField::INDEX_UNTOKENIZED
    },

    { NULL,
      NULL,
      0
    }
};
