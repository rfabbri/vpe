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
 *  Author:  Gerd Ehlers, Andreas Barth
 *
 *  Purpose: Implementation of class DcmUnsignedShort
 *
 */


#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "ofstream.h"
#include "dcvrus.h"
#include "dcvm.h"

#define INCLUDE_CSTDIO
#define INCLUDE_CSTRING
#include "ofstdinc.h"


// ********************************


DcmUnsignedShort::DcmUnsignedShort(const DcmTag &tag, const Uint32 len)
  : DcmElement(tag, len)
{
}


DcmUnsignedShort::DcmUnsignedShort(const DcmUnsignedShort &old)
  : DcmElement(old)
{
}


DcmUnsignedShort::~DcmUnsignedShort()
{
}


DcmUnsignedShort &DcmUnsignedShort::operator=(const DcmUnsignedShort &obj)
{
    DcmElement::operator=(obj);
    return *this;
}


// ********************************


DcmEVR DcmUnsignedShort::ident() const
{
    return EVR_US;
}


unsigned long DcmUnsignedShort::getVM()
{
    return Length / sizeof(Uint16);
}


// ********************************


void DcmUnsignedShort::print(ostream &out,
                             const size_t flags,
                             const int level,
                             const char * /*pixelFileName*/,
                             size_t * /*pixelCounter*/)
{
    if (valueLoaded())
    {
        /* get unsigned integer data */
        Uint16 *uintVals;
        errorFlag = getUint16Array(uintVals);
        if (uintVals != NULL)
        {
            const unsigned long count = getVM();
            const unsigned long maxLength = (flags & DCMTypes::PF_shortenLongTagValues) ?
                DCM_OptPrintLineLength : (unsigned long)-1 /*unlimited*/;
            unsigned long printedLength = 0;
            unsigned long newLength = 0;
            char buffer[32];
            /* print line start with tag and VR */
            printInfoLineStart(out, flags, level);
            /* print multiple values */
            for (unsigned int i = 0; i < count; i++, uintVals++)
            {
                /* check whether first value is printed (omit delimiter) */
                if (i == 0)
                    sprintf(buffer, "%hu", *uintVals);
                else
                    sprintf(buffer, "\\%hu", *uintVals);
                /* check whether current value sticks to the length limit */
                newLength = printedLength + strlen(buffer);
                if ((newLength <= maxLength) && ((i + 1 == count) || (newLength + 3 <= maxLength)))
                {
                    out << buffer;
                    printedLength = newLength;
                } else {
                    /* check whether output has been truncated */
                    if (i + 1 < count)
                    {
                        out << "...";
                        printedLength += 3;
                    }
                    break;
                }
            }
            /* print line end with length, VM and tag name */
            printInfoLineEnd(out, flags, printedLength);
        } else
            printInfoLine(out, flags, level, "(no value available)");
    } else
        printInfoLine(out, flags, level, "(not loaded)");
}


// ********************************


OFCondition DcmUnsignedShort::getUint16(Uint16 &uintVal,
                                        const unsigned long pos)
{
    /* get unsigned integer data */
    Uint16 *uintValues = NULL;
    errorFlag = getUint16Array(uintValues);
    /* check data before returning */
    if (errorFlag.good())
    {
        if (uintValues == NULL)
            errorFlag = EC_IllegalCall;
        else if (pos >= getVM())
            errorFlag = EC_IllegalParameter;
        else
            uintVal = uintValues[pos];
    }
    /* clear value in case of error */
    if (errorFlag.bad())
        uintVal = 0;
    return errorFlag;
}


OFCondition DcmUnsignedShort::getUint16Array(Uint16 *&uintVals)
{
    uintVals = (Uint16 *)getValue();
    return errorFlag;
}


// ********************************


OFCondition DcmUnsignedShort::getOFString(OFString &stringVal,
                                          const unsigned long pos,
                                          OFBool /*normalize*/)
{
    Uint16 uintVal;
    /* get the specified numeric value */
    errorFlag = getUint16(uintVal, pos);
    if (errorFlag.good())
    {
        /* ... and convert it to a character string */
        char buffer[32];
        sprintf(buffer, "%hu", uintVal);
        /* assign result */
        stringVal = buffer;
    }
    return errorFlag;
}


// ********************************


OFCondition DcmUnsignedShort::putUint16(const Uint16 uintVal,
                                        const unsigned long pos)
{
    Uint16 val = uintVal;
    errorFlag = changeValue(&val, sizeof(Uint16) * pos, sizeof(Uint16));
    return errorFlag;
}


OFCondition DcmUnsignedShort::putUint16Array(const Uint16 *uintVals,
                                             const unsigned long numUints)
{
    errorFlag = EC_Normal;
    if (numUints > 0)
    {
        /* check for valid data */
        if (uintVals != NULL)
            errorFlag = putValue(uintVals, sizeof(Uint16) * (Uint32)numUints);
        else
            errorFlag = EC_CorruptedData;
    } else
        errorFlag = putValue(NULL, 0);
    return errorFlag;
}


// ********************************


OFCondition DcmUnsignedShort::putString(const char *stringVal)
{
    errorFlag = EC_Normal;
    /* check input string */
    if ((stringVal != NULL) && (strlen(stringVal) > 0))
    {
        const unsigned long vm = getVMFromString(stringVal);
        if (vm > 0)
        {
            Uint16 *field = new Uint16[vm];
            const char *s = stringVal;
            char *value;
            /* retrieve unsigned integer data from character string */
            for (unsigned long i = 0; (i < vm) && errorFlag.good(); i++)
            {
                /* get first value stored in 's', set 's' to beginning of the next value */
                value = getFirstValueFromString(s);
                if ((value == NULL) || (sscanf(value, "%hu", &field[i]) != 1))
                    errorFlag = EC_CorruptedData;
                delete[] value;
            }
            /* set binary data as the element value */
            if (errorFlag.good())
                errorFlag = putUint16Array(field, vm);
            /* delete temporary buffer */
            delete[] field;
        } else
            errorFlag = putValue(NULL, 0);
    } else
        errorFlag = putValue(NULL, 0);
    return errorFlag;
}


// ********************************


OFCondition DcmUnsignedShort::verify(const OFBool autocorrect)
{
    /* check for valid value length */
    if (Length % (sizeof(Uint16)) != 0)
    {
        errorFlag = EC_CorruptedData;
        if (autocorrect)
        {
            /* strip to valid length */
            Length -= (Length % (sizeof(Uint16)));
        }
    } else
        errorFlag = EC_Normal;
    return errorFlag;
}
