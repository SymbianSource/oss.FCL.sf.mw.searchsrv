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

#ifndef CPIX_GPSSORT_H
#define CPIX_GPSSORT_H


namespace Cpix
{

    /**
     * Sorts an unsorted list of hits according to their distance from
     * a reference point. The sort is a stable sort, that is, the
     * relative order of equivalent items are unchanged.
     *
     * @param refPointGpsLat the GPS latitude of the reference point
     *
     * @param refPointGpsLong the GPS longitude of the reference point
     *
     * @param unsortedHits the list of hit documents in an unsorted
     * order:  the documents must have the following
     * fields: LLATITUDE_FIELD and LLONGITUDE_FIELD.
     *
     * @param result - to communicate errors with
     *
     * @return a newly created hit list, with newly created
     * documents. These documents will still have their
     * latitude/longitude fields, but they will also have the
     * LDISTANCE_FIELD field defined that will tell the km distance
     * from the given reference point. And, the documents in the newly
     * created list will be sorted according to their distance (in an
     * ascending order) from the reference point.
     */
    cpix_Hits * gpsSort(float              refPointGpsLat,
                        float              refPointGpsLong,
                        cpix_Hits        * unsortedHits,
                        cpix_Result      * result);
}


#endif

