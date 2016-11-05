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
 *  Author:  Gerd Ehlers
 *
 *  Purpose: Print debug information
 *
 */

#ifndef DCDEBUG_H
#define DCDEBUG_H

#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "ofstream.h"
#include "ofglobal.h"

extern OFGlobal<int> DcmDebugLevel; /* default 0 */

#ifdef DEBUG

void debug_print(const char* text, ... );

// Set the debug level
#define SetDebugLevel(level) DcmDebugLevel.set(level);

// debug prints a debug message in param if lev <= DcmDebugLevel. param has the
// format of the printf parameters (with round brackets)!
#define debug(lev, param) \
  { \
    if ((lev) <= DcmDebugLevel.get()) \
    { \
      ofConsole.lockCerr() << __FILE__ << ", LINE " << __LINE__ << ":"; \
      debug_print param ; \
      ofConsole.unlockCerr(); \
    } \
  }

// Cdebug does the same as debug but only if a condition cond is OFTrue
#define Cdebug(lev, cond, param) \
  { \
    if ((lev) <= DcmDebugLevel.get() && (cond)) \
    { \
      ofConsole.lockCerr() << __FILE__ << ", LINE " << __LINE__ << ":"; \
      debug_print param ; \
      ofConsole.unlockCerr(); \
    } \
  }

#else  // DEBUG

#define SetDebugLevel(param)
#define debug(lev, param)
#define Cdebug(lev, cond, param)

#endif // DEBUG

#endif // DCDEBUG_H
