/*
 *
 *  Copyright (C) 1997-2001, OFFIS
 *
 *  This software and supporting documentation were developed by
 *
 *    Kuratorium OFFIS e.V.
 *    Healthcare Information and Communication Systems
 *    Escherweg 2
 *    D-26121 Oldenburg, Germany
 *
 *  THIS SOFTWARE IS MADE AVAILABLE,  AS IS,  AND OFFIS MAKES NO  WARRANTY
 *  REGARDING  THE  SOFTWARE,  ITS  PERFORMANCE,  ITS  MERCHANTABILITY  OR
 *  FITNESS FOR ANY PARTICULAR USE, FREEDOM FROM ANY COMPUTER DISEASES  OR
 *  ITS CONFORMITY TO ANY SPECIFICATION. THE ENTIRE RISK AS TO QUALITY AND
 *  PERFORMANCE OF THE SOFTWARE IS WITH THE USER.
 *
 *  Module:  ofstd
 *
 *  Author:  Andreas Barth
 *
 *  Purpose:
 *    Implementation of supplementary methods for a template list class
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#if defined(HAVE_STL) || defined (HAVE_STL_LIST)
// We do not need to make this library
void OF__DUMMY()    // to make the linker happy!
{
}
#else

#include "oflist.h"

OFListBase::OFListBase()
: afterLast(NULL)
, listSize(0)
{
    afterLast = new OFListLinkBase();
    afterLast->prev = afterLast->next = afterLast;
    afterLast->dummy = OFTrue;
}


OFListBase::~OFListBase()
{
    base_clear();
    if (afterLast)
        delete afterLast;
}

OFListLinkBase * OFListBase::base_insert(OFListLinkBase * pos,
                                         OFListLinkBase * newElem)
{
    assert(pos && newElem);
    if (pos && newElem)
    {
        newElem->next = pos;
        newElem->prev = pos->prev;
        pos->prev->next = newElem;
        pos->prev = newElem;
        listSize++;
        return newElem;
    }
    return NULL;
}


OFListLinkBase * OFListBase::base_erase(OFListLinkBase * pos)
{
    assert(pos && pos != afterLast);
    OFListLinkBase * tmp = pos->next;;
    pos->next->prev=pos->prev;
    pos->prev->next=pos->next;
    delete pos;
    listSize--;
    return tmp;
}

void OFListBase::base_splice(OFListLinkBase * pos,
                             OFListLinkBase * begin,
                             OFListLinkBase * end)
{
    assert(pos && begin && end);
    if (begin != end)
    {
        OFListLinkBase * beginPrev = begin->prev;
        OFListLinkBase * posPrev = pos->prev;
        pos->prev->next = begin;
        pos->prev = end->prev;
        begin->prev->next = end;
        begin->prev = posPrev;
        end->prev->next = pos;
        end->prev = beginPrev;
        base_recalcListSize();
    }
}

void OFListBase::base_clear()
{
    while (listSize)
        base_erase(afterLast->next);
}

void OFListBase::base_recalcListSize()
{
    OFListLinkBase * elem;
    for (listSize = 0, elem = afterLast->next;
    elem != afterLast;
    elem = elem->next, ++listSize);
}

#endif
