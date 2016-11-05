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
 *  Author:  Gerd Ehlers, Joerg Riesmeier
 *
 *  Purpose: Implementation of class DcmTime
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */

#include "dcvrtm.h"
#include "ofstd.h"

#define INCLUDE_CSTDIO
#include "ofstdinc.h"


// ********************************


DcmTime::DcmTime(const DcmTag &tag,
                 const Uint32 len)
  : DcmByteString(tag, len)
{
    maxLength = 16;
}


DcmTime::DcmTime(const DcmTime &old)
  : DcmByteString(old)
{
}


DcmTime::~DcmTime()
{
}


DcmTime &DcmTime::operator=(const DcmTime &obj)
{
    DcmByteString::operator=(obj);
    return *this;
}


// ********************************


DcmEVR DcmTime::ident() const
{
    return EVR_TM;
}


// ********************************


OFCondition DcmTime::getOFString(OFString &stringVal,
                                 const unsigned long pos,
                                 OFBool normalize)
{
    OFCondition l_error = DcmByteString::getOFString(stringVal, pos, normalize);
    if (l_error.good() && normalize)
        normalizeString(stringVal, !MULTIPART, !DELETE_LEADING, DELETE_TRAILING);
    return l_error;
}


// ********************************


OFCondition DcmTime::getOFTime(OFTime &timeValue,
                               const unsigned long pos,
                               const OFBool supportOldFormat)
{
    OFString dicomTime;
    /* convert the current element value to OFTime format */
    OFCondition l_error = getOFString(dicomTime, pos);
    if (l_error.good())
        l_error = getOFTimeFromString(dicomTime, timeValue, supportOldFormat);
    else
        timeValue.clear();
    return l_error;
}


OFCondition DcmTime::getISOFormattedTime(OFString &formattedTime,
                                         const unsigned long pos,
                                         const OFBool seconds,
                                         const OFBool fraction,
                                         const OFBool createMissingPart,
                                         const OFBool supportOldFormat)
{
    OFString dicomTime;
    /* get current element value and convert to ISO formatted time */
    OFCondition l_error = getOFString(dicomTime, pos);
    if (l_error.good())
        l_error = getISOFormattedTimeFromString(dicomTime, formattedTime, seconds, fraction, createMissingPart, supportOldFormat);
    else
        formattedTime.clear();
    return l_error;
}


OFCondition DcmTime::setCurrentTime(const OFBool seconds,
                                    const OFBool fraction)
{
    OFString dicomTime;
    /* set the element value to the current system time */
    OFCondition l_error = getCurrentTime(dicomTime, seconds, fraction);
    if (l_error.good())
        l_error = putString(dicomTime.c_str());
    return l_error;
}


OFCondition DcmTime::setOFTime(const OFTime &timeValue)
{
    OFString dicomTime;
    /* convert OFTime value to DICOM TM format and set the element value */
    OFCondition l_error = getDicomTimeFromOFTime(timeValue, dicomTime);
    if (l_error.good())
        l_error = putString(dicomTime.c_str());
    return l_error;
}


// ********************************


OFCondition DcmTime::getCurrentTime(OFString &dicomTime,
                                    const OFBool seconds,
                                    const OFBool fraction)
{
    OFCondition l_error = EC_IllegalCall;
    OFTime timeValue;
    /* get the current system time */
    if (timeValue.setCurrentTime())
    {
        /* format: HHMM[SS[.FFFFFF]] */
        if (timeValue.getISOFormattedTime(dicomTime, seconds, fraction, OFFalse /*timeZone*/, OFFalse /*showDelimiter*/))
            l_error = EC_Normal;
    }
    /* set default time if an error occurred */
    if (l_error.bad())
    {
        /* if the current system time cannot be retrieved create a valid default time */
        if (seconds)
        {
            if (fraction)
            {
                /* format: HHMMSS.FFFFFF */
                dicomTime = "000000.000000";
            } else {
                /* format: HHMMS */
                dicomTime = "000000";
            }
        } else {
            /* format: HHMM */
            dicomTime = "0000";
        }
    }
    return l_error;
}


OFCondition DcmTime::getDicomTimeFromOFTime(const OFTime &timeValue,
                                            OFString &dicomTime,
                                            const OFBool seconds,
                                            const OFBool fraction)
{
    OFCondition l_error = EC_IllegalParameter;
    /* convert OFTime value to DICOM TM format */
    if (timeValue.getISOFormattedTime(dicomTime, seconds, fraction, OFFalse /*timeZone*/, OFFalse /*showDelimiter*/))
        l_error = EC_Normal;
    return l_error;
}


OFCondition DcmTime::getOFTimeFromString(const OFString &dicomTime,
                                         OFTime &timeValue,
                                         const OFBool supportOldFormat)
{
    OFCondition l_error = EC_IllegalParameter;
    /* clear result variable */
    timeValue.clear();
    /* minimal check for valid format */
    if (supportOldFormat || (dicomTime.find(":") == OFString_npos))
    {
        unsigned int hour, minute = 0;
        double second = 0.0;
        /* normalize time format (remove ":" chars) */
        OFString string = dicomTime;
        if ((string.length() > 5) && (string[5] == ':'))
            string.erase(5, 1);
        if ((string.length() > 2) && (string[2] == ':'))
            string.erase(2, 1);
        /* extract components from time string: HH[MM[SS[.FFFFFF]]] */
        /* scan seconds using OFStandard::atof to avoid locale issues */
        if (sscanf(string.c_str(), "%02u%02u", &hour, &minute) >= 1)
        {
            if (string.length() > 4)
            {
                /* get optional seconds part */
                string.erase(0, 4);
                second = OFStandard::atof(string.c_str());
            }
            /* always use the local time zone */
            if (timeValue.setTime(hour, minute, second, OFTime::getLocalTimeZone()))
                l_error = EC_Normal;
        }
    }
    return l_error;
}


OFCondition DcmTime::getISOFormattedTimeFromString(const OFString &dicomTime,
                                                   OFString &formattedTime,
                                                   const OFBool seconds,
                                                   const OFBool fraction,
                                                   const OFBool createMissingPart,
                                                   const OFBool supportOldFormat)
{
    OFCondition result = EC_IllegalParameter;
    /* minimal check for valid format */
    if (supportOldFormat || (dicomTime.find(":") == OFString_npos))
    {
        const size_t length = dicomTime.length();
        /* check for prior V3.0 version of VR=TM: HH:MM:SS.frac */
        const size_t minPos = (supportOldFormat && (length > 2) && (dicomTime[2] == ':')) ? 3 : 2;
        const size_t secPos = (supportOldFormat && (length > minPos + 2) && (dicomTime[minPos + 2] == ':')) ? minPos + 3 : minPos + 2;
        /* decimal point for fractional seconds */
        const size_t decPoint = dicomTime.find(".");
        const size_t decLength = (decPoint != OFString_npos) ? decPoint : length;
        OFString hourStr, minStr, secStr, fracStr;
        /* hours */
        if (decLength >= 2)
            hourStr = dicomTime.substr(0, 2);
        else
            hourStr = "00";
        /* minutes */
        if (decLength >= minPos + 2)
            minStr = dicomTime.substr(minPos, 2);
        else
            minStr = "00";
        /* seconds */
        if (decLength >= secPos + 2)
            secStr = dicomTime.substr(secPos, 2);
        else if (createMissingPart)
            secStr = "00";
        /* fractional seconds */
        if ((length >= secPos + 4) && (decPoint == secPos + 2))
        {
            if (length < secPos + 9)
            {
                fracStr = dicomTime.substr(secPos + 3);
                fracStr.append(secPos + 9 - length, '0');
            } else
                fracStr = dicomTime.substr(secPos + 3, 6);
        } else if (createMissingPart)
            fracStr = "000000";
        /* concatenate time components */
        formattedTime = hourStr;
        formattedTime += ":";
        formattedTime += minStr;
        if (seconds && (secStr.length() > 0))
        {
            formattedTime += ":";
            formattedTime += secStr;
            if (fraction && (fracStr.length() > 0))
            {
                formattedTime += ".";
                formattedTime += fracStr;
            }
        }
        result = EC_Normal;
    } else
        formattedTime.clear();
    return result;
}


OFCondition DcmTime::getTimeZoneFromString(const OFString &dicomTimeZone,
                                           double &timeZone)
{
    OFCondition result = EC_IllegalParameter;
    /* init return value */
    timeZone = 0;
    /* minimal check for valid format */
    if ((dicomTimeZone.length() == 5) && ((dicomTimeZone[0] == '+') || (dicomTimeZone[0] == '-')))
    {
        signed int hour;
        unsigned int minute;
        /* extract components from time zone string */
        if (sscanf(dicomTimeZone.c_str(), "%03i%02u", &hour, &minute) == 2)
        {
            timeZone = (double)hour + (double)minute / 60;
            result = EC_Normal;
        }
    }
    return result;
}
