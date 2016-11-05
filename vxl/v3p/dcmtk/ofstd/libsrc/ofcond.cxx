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
 *  Author:  Marco Eichelberg
 *
 *  Purpose: class OFCondition and helper classes
 *
 */

#include "osconfig.h"
#include "ofcond.h"


/* ---------- global condition constants ---------- */

const OFConditionConst ECC_Normal(          0, 0, OF_ok,      "Normal");
const OFConditionConst ECC_IllegalParameter(0, 1, OF_error,   "Illegal parameter");
const OFConditionConst ECC_MemoryExhausted( 0, 2, OF_failure, "Virtual Memory exhausted");

// NOTE:
// error codes 1024..2047 are reserved for inter-process communication
// errors defined elsewhere in module ofstd

const OFCondition EC_Normal(ECC_Normal);
const OFCondition EC_IllegalParameter(ECC_IllegalParameter);
const OFCondition EC_MemoryExhausted(ECC_MemoryExhausted);

/* ---------- class OFConditionConst ---------- */


const OFConditionBase *OFConditionConst::clone() const
{
  return this; // since we're not deletable, we just return a copy to ourselves
}

unsigned long OFConditionConst::codeAndModule() const
{
  return theCodeAndModule;
}

OFStatus OFConditionConst::status() const
{
  return theStatus;
}

const char *OFConditionConst::text() const
{
  return theText;
}

OFBool OFConditionConst::deletable() const
{
  return OFFalse;
}


/* ---------- class OFConditionString ---------- */


const OFConditionBase *OFConditionString::clone() const
{
  return new OFConditionString(*this);
}

unsigned long OFConditionString::codeAndModule() const
{
  return theCodeAndModule;
}

OFStatus OFConditionString::status() const
{
  return theStatus;
}

const char *OFConditionString::text() const
{
  return theText.c_str();
}

OFBool OFConditionString::deletable() const
{
  return OFTrue;
}
