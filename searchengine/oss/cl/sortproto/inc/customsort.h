/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef SORTPROTO_CUSTOMSORT_H
#define SORTPROTO_CUSTOMSORT_H


#include <fstream>
#include <map>

namespace lucene 
{ 
    namespace search 
    {


        typedef std::map<size_t, std::wstring> SortValueCache;

        /**
         *
         */
        class AppDomainSortDocComparator : public ScoreDocComparator
        {
        private:
            std::map<const wchar_t *, size_t>   appDomainOrderCache_;
            std::map<std::wstring, size_t>      appDomainOrder_;

            SortValueCache                    & sortValueCache_;

            FieldCache::StringIndex           * index_;

            std::wofstream                    & ofs_;

        public:
            AppDomainSortDocComparator(const wchar_t           ** bacs,
                                       SortValueCache           & sortValueCache,
                                       FieldCache::StringIndex  * index,
                                       int32_t                    /* len */,
                                       std::wofstream           & ofs)
                : index_(index),
                  sortValueCache_(sortValueCache),
                  ofs_(ofs)
            {
                using namespace std;

                for (; *bacs != NULL; ++bacs)
                    {
                        appDomainOrder_[wstring(*bacs)] = appDomainOrder_.size();
                    }
            }


            int32_t compare(struct ScoreDoc * i,
                            struct ScoreDoc * j)
            {
                ofs_ << L"### COMPARE ";

                size_t
                    iOrder = order(i);

                ofs_ << L" <> ";

                size_t
                    jOrder = order(j);
                int32_t
                    rv = 0;

                if (iOrder < jOrder)
                    {
                        rv = -1;
                    }
                else if (iOrder > jOrder)
                    {
                        rv = 1;
                    }

                ofs_ << L": " << rv << std::endl;
                
                return rv;
            }
            

            CL_NS(util)::Comparable *
            sortValue(struct ScoreDoc * i)
            {
                // This special sorter works as if there was a string
                // field it sorts by. While the compare() function
                // does not actually create / use those strings, here
                // we
                //
                // (a) make sure they exist and owned by the
                //     sortValueCache_ (which, in turn, is owned by
                //     the factory that created this sorter)
                //
                // (b) create a comparator for that value
                //
                // (c) and give that back

                ofs_ << L"### SORTVALUE ";

                size_t
                    o = order(i);

                const wchar_t
                    * sortValue = NULL;

                if (sortValueCache_.find(o) == sortValueCache_.end())
                    {
                        std::wstring
                            tmp;
                        tmp += L'0' + static_cast<wchar_t>(o);
                        sortValueCache_[o] = tmp;
                        
                    }
 
                sortValue = sortValueCache_[o].c_str();
                       
                ofs_ << L": " << sortValue << endl;

                return _CLNEW CL_NS(util)::Compare::TChar(sortValue);
            }


            int32_t sortType()
            {
                // This special sorter works as if there was a string
                // field it sorts by. So we tell here STRING, but
                // probably CUSTOM would work the same too.
                return SortField::STRING;
            }


        private:
            size_t order(ScoreDoc * sd)
            {
                using namespace std;

                size_t
                    rv = 0;

                const wchar_t
                    * strValue = index_->lookup[index_->order[sd->doc]];

                if (appDomainOrderCache_.find(strValue) != appDomainOrderCache_.end())
                    {
                        rv = appDomainOrderCache_[strValue];
                    }
                else
                    {
                        wstring
                            tmp(strValue);

                        if (appDomainOrder_.find(tmp) != appDomainOrder_.end())
                            {
                                rv = appDomainOrder_[tmp];
                            }
                        else
                            {
                                rv = appDomainOrder_.size();
                            }

                        appDomainOrderCache_[strValue] = rv;
                    }

                ofs_ << strValue << L"(" << rv << L")";
                
                return rv;
            }
        }; // class



        #define ROOTCALENDAR L"root calendar"
        #define ROOTCONTACT  L"root contact"
        #define ROOTMSG      L"root msg"

        const wchar_t * BACS[] = {
            ROOTCONTACT,
            ROOTMSG,
            ROOTCALENDAR,
            NULL
        };



        /**
         * NOTE: use one instance for one search ...
         */
        class AppDomainSortFactory : public SortComparatorSource
        {
        private:
            std::wofstream             ofs_;
            SortValueCache             sortValueCache_;

        public:
            AppDomainSortFactory()
                : ofs_("c:\\Data\\sortproto\\adsf-dump.txt")
            {
                ;
            }


            virtual wchar_t * getName()
            {
                return L"AppDomain (category) sorting";
            }


            virtual size_t hashCode()
            {
                return 1928378;
            }

            
            virtual ScoreDocComparator *
            newComparator(CL_NS(index)::IndexReader * reader,
                          const wchar_t             * fieldName)
            {
                FieldCacheAuto
                    * fca = FieldCache::DEFAULT->getStringIndex(reader,
                                                                fieldName);
                fca->ownContents = false;
                
                return _CLNEW AppDomainSortDocComparator(BACS,
                                                         sortValueCache_,
                                                         fca->stringIndex,
                                                         fca->contentLen,
                                                         ofs_);
            }
        };


        
        /**
         * For storing translated datetime values for calendar sorting
         * purposes.
         */
        typedef std::map<const wchar_t *, std::wstring> CalXlatedDTCache;


        class UpcomingEventSortDocComparator : public ScoreDocComparator
        {
        private:
            const std::wstring        nowDateTime_;
            CalXlatedDTCache        & calXlatedDTCache_;
            FieldCache::StringIndex * index_;

        public:
            UpcomingEventSortDocComparator(const std::wstring     & nowDateTime,
                                           CalXlatedDTCache       & calXlatedDTCache,
                                           FieldCache::StringIndex * index,
                                           int32_t                   /* len */)
                : nowDateTime_(nowDateTime),
                  calXlatedDTCache_(calXlatedDTCache),
                  index_(index)
            {
                ;
            }


            int32_t compare(struct ScoreDoc * i,
                            struct ScoreDoc * j)
            {
                const wchar_t
                    * iStrValue = index_->lookup[index_->order[i->doc]],
                    * jStrValue = index_->lookup[index_->order[j->doc]];

                const wchar_t
                    * iXlated(getXlatedDTVal(iStrValue)),
                    * jXlated(getXlatedDTVal(jStrValue));

                int32_t
                    rv = wcscmp(iXlated,
                                jXlated);

                return rv;
            }


            CL_NS(util)::Comparable *
                        sortValue(struct ScoreDoc * i)
            {
                const wchar_t
                    * strValue = index_->lookup[index_->order[i->doc]];

                const wchar_t
                    * xlated(getXlatedDTVal(strValue));

                return _CLNEW CL_NS(util)::Compare::TChar(xlated);
            }

            
            int32_t sortType()
            {
                return SortField::STRING;
            }

        private:

            const wchar_t * getXlatedDTVal(const wchar_t * dtVal)
            {
                if (calXlatedDTCache_.find(dtVal) == calXlatedDTCache_.end())
                    {
                        std::wstring
                            tmp;

                        if (wcscmp(dtVal, nowDateTime_.c_str()) >= 0)
                            {
                                tmp += L'A';
                                tmp += dtVal;
                            }
                        else
                            {
                                tmp += L'B';
                                for (; *dtVal != 0; ++dtVal)
                                    {
                                        tmp += L'9' - (*dtVal - L'0');
                                    }
                            }

                        calXlatedDTCache_[dtVal] = tmp;
                    }

                return calXlatedDTCache_[dtVal].c_str();
            }

        };


        class UpcomingEventSortFactory : public SortComparatorSource
        {
        private:
            const std::wstring      nowDateTime_;
            CalXlatedDTCache        calXlatedDTCache_;

        public:
            UpcomingEventSortFactory(const wchar_t * nowDateTime)
                : nowDateTime_(nowDateTime)
            {
                ;
            }


            virtual wchar_t * getName()
            {
                return L"Calendar sorting";
            }


            virtual size_t hashCode()
            {
                return 2837465;
            }

            virtual ScoreDocComparator *
            newComparator(CL_NS(index)::IndexReader * reader,
                          const wchar_t             * fieldName)
            {
                FieldCacheAuto
                    * fca = FieldCache::DEFAULT->getStringIndex(reader,
                                                                fieldName);
                fca->ownContents = false;
                return _CLNEW UpcomingEventSortDocComparator(nowDateTime_,
                                                             calXlatedDTCache_,
                                                             fca->stringIndex,
                                                             fca->contentLen);
            }


        };

        
    }
}

#endif SORTPROTO_CUSTOMSORT_H
