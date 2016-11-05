/*
 *
 *  Copyright (C) 2002, OFFIS
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
 *  Author:  Joerg Riesmeier
 *
 *  Purpose: Class for date and time functions (Source)
 *
 */


#include "osconfig.h"

#define INCLUDE_CTIME
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>    /* for struct time_t */
#endif
END_EXTERN_C

#include "ofdatime.h"


/*------------------*
 *  implementation  *
 *------------------*/

OFDateTime::OFDateTime()
  : Date(),
    Time()
{
}


OFDateTime::OFDateTime(const OFDateTime &dateTime)
  : Date(dateTime.Date),
    Time(dateTime.Time)
{
}


OFDateTime::OFDateTime(const OFDate &dateVal,
                      const OFTime &timeVal)
  : Date(dateVal),
    Time(timeVal)
{
}


OFDateTime::~OFDateTime()
{
}


OFDateTime &OFDateTime::operator=(const OFDateTime &dateTime)
{
    Date = dateTime.Date;
    Time = dateTime.Time;
    return *this;
}


OFBool OFDateTime::operator==(const OFDateTime &dateTime)
{
    return (Date == dateTime.Date) && (Time == dateTime.Time);
}


OFBool OFDateTime::operator!=(const OFDateTime &dateTime)
{
    return (Date != dateTime.Date) || (Time != dateTime.Time);
}


void OFDateTime::clear()
{
    Date.clear();
    Time.clear();
}


OFBool OFDateTime::isValid() const
{
    /* check current date settings */
    return Date.isValid() && Time.isValid();
}


OFBool OFDateTime::setDateTime(const unsigned int year,
                               const unsigned int month,
                               const unsigned int day,
                               const unsigned int hour,
                               const unsigned int minute,
                               const double second,
                               const double timeZone)
{
    OFBool result = OFFalse;
    /* check whether new date and time are valid */
    if (OFDate::isDateValid(year, month, day) && OFTime::isTimeValid(hour, minute, second, timeZone))
       result = Date.setDate(year, month, day) && Time.setTime(hour, minute, second, timeZone);
    return result;
}


OFBool OFDateTime::setDate(const OFDate &dateVal)
{
    OFBool result = OFFalse;
    /* check whether new date is valid */
    if (dateVal.isValid())
    {
       Date = dateVal;
       result = OFTrue;
    }
    return result;
}


OFBool OFDateTime::setTime(const OFTime &timeVal)
{
    OFBool result = OFFalse;
    /* check whether new time is valid */
    if (timeVal.isValid())
    {
       Time = timeVal;
       result = OFTrue;
    }
    return result;
}


OFBool OFDateTime::setCurrentDateTime()
{
    /* set current system date and time, use the same "time_t" struct to avoid inconsistencies */
    time_t tt = time(NULL);
    return Date.setCurrentDate(tt) && Time.setCurrentTime(tt);
}


const OFDate &OFDateTime::getDate() const
{
    return Date;
}


const OFTime &OFDateTime::getTime() const
{
    return Time;
}


OFBool OFDateTime::getISOFormattedDateTime(OFString &formattedDateTime,
                                           const OFBool showSeconds,
                                           const OFBool showFraction,
                                           const OFBool showTimeZone,
                                           const OFBool showDelimiter) const
{
    /* get formatted date first component */
    OFBool result = Date.getISOFormattedDate(formattedDateTime, showDelimiter);
    if (result)
    {
        /* ... then get the formatted time component */
        OFString timeString;
        if (Time.getISOFormattedTime(timeString, showSeconds, showFraction, showTimeZone, showDelimiter))
        {
            if (showDelimiter)
                formattedDateTime += " ";
            formattedDateTime += timeString;
        }
    } else {
        /* clear result variable in case of error */
        formattedDateTime.clear();
    }
    return result;
}


OFDateTime OFDateTime::getCurrentDateTime()
{
    /* create a date/time object with the current system date/time set */
    OFDateTime dateTime;
    /* this call might fail! */
    dateTime.setCurrentDateTime();
    /* return by-value */
    return dateTime;
}


ostream& operator<<(ostream& stream, const OFDateTime &dateTime)
{
    OFString string;
    /* print the given date and time in ISO format to the stream */
    if (dateTime.getISOFormattedDateTime(string))
        stream << string;
    return stream;
}
