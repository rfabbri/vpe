/*
 *
 *  Copyright (C) 1994-2002, OFFIS
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
 *  Module:  dcmdata
 *
 *  Author:  Andreas Barth
 *
 *  Purpose: common defines for configuration
 *
 */

#ifndef DCDEFINE_H
#define DCDEFINE_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#define INCLUDE_CSTRING
#include "ofstdinc.h"

/* memzero */
#ifdef HAVE_MEMSET
#  undef memzero
#  define memzero(d, n) memset((d), 0, (n))
#else
#  ifdef HAVE_BZERO
#    undef memzero
#    define memzero(d, n) bzero((d), (n))
#  endif
#endif

/* memcpy */
#ifndef HAVE_MEMCPY
#  ifdef HAVE_BCOPY
#    undef memcpy
#    define memcpy(d, s, n) bcopy((s), (d), (n))
#  endif
#endif

/* memmove */
#ifndef HAVE_MEMMOVE
#  ifdef HAVE_BCOPY
#    undef memmove
#    define memmove(d, s, n) bcopy ((s), (d), (n))
#  endif
#endif

/* memcmp */
#ifndef HAVE_MEMCMP
#  ifdef HAVE_BCMP
#    undef memcmp
#    define memcmp(d, s, n) bcmp((s), (d), (n))
#  endif
#endif

/* strchr, strrchr */
#ifndef HAVE_STRCHR
#  ifdef HAVE_INDEX
#    undef strchr
#    define strchr index
#    undef strrchr
#    define strrchr rindex
#  endif
#endif

#endif
