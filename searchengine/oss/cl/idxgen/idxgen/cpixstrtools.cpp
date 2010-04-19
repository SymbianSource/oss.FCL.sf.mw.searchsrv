/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include "cpixstrtools.h"
// #include <e32std.h>

namespace Cpt
{

    void splitstring(const char             * orig,
                     char                   * delimiters,
                     std::list<std::string> & target)
    {
        using namespace std;

        target.clear();

        if (orig == NULL)
            {
                return;
            }

        const char
            * curBegin = orig,
            * curEnd = strpbrk(curBegin,
                               delimiters);

        while (curBegin != NULL)
            {
                if (curEnd == NULL)
                    {
                        target.push_back(string(curBegin,
                                                strlen(curBegin)));
                        curBegin = NULL;
                    }
                else
                    {
                        target.push_back(string(curBegin,
                                                curEnd));
                        curBegin = curEnd + 1;
                        if (*curBegin != 0)
                            {
                                curEnd = strpbrk(curBegin,
                                                 delimiters);
                            }
                        else
                            {
                                curBegin = NULL;
                            }
                    }
            } 
    }



    /* OBS 
    uint32_t getUsedDynamicMemory()
    {
        // TODO this is symbian specific code here

        TInt 
            largestBlock;
        TInt 
            mem = User::Heap().Size() - User::Heap().Available(largestBlock);

        return static_cast<uint32_t>(mem);
    }
    */

}
