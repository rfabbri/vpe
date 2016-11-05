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
 *  Purpose: Class for time functions (Source)
 *
 */


#include "osconfig.h"

#define INCLUDE_CSTDIO
#define INCLUDE_CTIME
#include "ofstdinc.h"

BEGIN_EXTERN_C
#ifndef HAVE_WINDOWS_H
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>     /* for struct timeval on Linux */
#endif

#ifndef HAVE_PROTOTYPE_GETTIMEOFDAY
 /* Ultrix has gettimeofday() but no prototype in the header files */
 int gettimeofday(struct timeval *tp, void *);
#endif
#endif
END_EXTERN_C

#ifdef HAVE_WINDOWS_H
#include <windows.h>      /* for Windows time functions */
#endif

#include "oftime.h"
#include "ofstd.h"

/*------------------*
 *  implementation  *
 *------------------*/

OFTime::OFTime()
  : Hour(0),
    Minute(0),
    Second(0),
    TimeZone(0)
{
}


OFTime::OFTime(const OFTime &timeVal)
  : Hour(timeVal.Hour),
    Minute(timeVal.Minute),
    Second(timeVal.Second),
    TimeZone(timeVal.TimeZone)
{
}


OFTime::OFTime(const unsigned int hour,
               const unsigned int minute,
               const double second,
               const double timeZone)
  : Hour(hour),
    Minute(minute),
    Second(second),
    TimeZone(timeZone)
{
}


OFTime::~OFTime()
{
}


OFTime &OFTime::operator=(const OFTime &timeVal)
{
    Hour = timeVal.Hour;
    Minute = timeVal.Minute;
    Second = timeVal.Second;
    TimeZone = timeVal.TimeZone;
    return *this;
}


OFBool OFTime::operator==(const OFTime &timeVal)
{
    return (getTimeInSeconds(OFTrue /*useTimeZone*/) == timeVal.getTimeInSeconds(OFTrue /*useTimeZone*/));
}


OFBool OFTime::operator!=(const OFTime &timeVal)
{
    return (getTimeInSeconds(OFTrue /*useTimeZone*/) != timeVal.getTimeInSeconds(OFTrue /*useTimeZone*/));
}


OFBool OFTime::operator<(const OFTime &timeVal)
{
    return (getTimeInSeconds(OFTrue /*useTimeZone*/) < timeVal.getTimeInSeconds(OFTrue /*useTimeZone*/));
}


OFBool OFTime::operator<=(const OFTime &timeVal)
{
    return (getTimeInSeconds(OFTrue /*useTimeZone*/) <= timeVal.getTimeInSeconds(OFTrue /*useTimeZone*/));
}


OFBool OFTime::operator>=(const OFTime &timeVal)
{
    return (getTimeInSeconds(OFTrue /*useTimeZone*/) >= timeVal.getTimeInSeconds(OFTrue /*useTimeZone*/));
}


OFBool OFTime::operator>(const OFTime &timeVal)
{
    return (getTimeInSeconds(OFTrue /*useTimeZone*/) > timeVal.getTimeInSeconds(OFTrue /*useTimeZone*/));
}


void OFTime::clear()
{
    Hour = 0;
    Minute = 0;
    Second = 0;
    TimeZone = 0;
}


OFBool OFTime::isValid() const
{
    /* check current time settings */
    return isTimeValid(Hour, Minute, Second, TimeZone);
}


OFBool OFTime::isTimeValid(const unsigned int hour,
                           const unsigned int minute,
                           const double second,
                           const double timeZone)
{
    /* check whether given time is valid */
    return (hour < 24) && (minute < 60) && (second >= 0) && (second < 60) && (timeZone >= -12) && (timeZone <= 12);
}


OFBool OFTime::setTime(const unsigned int hour,
                       const unsigned int minute,
                       const double second,
                       const double timeZone)
{
    OFBool status = OFFalse;
    /* only change if the new time is valid */
    if (isTimeValid(hour, minute, second, timeZone))
    {
        Hour = hour;
        Minute = minute;
        Second = second;
        TimeZone = timeZone;
        /* report that a new time has been set */
        status = OFTrue;
    }
    return status;
}


OFBool OFTime::setHour(const unsigned int hour)
{
    OFBool status = OFFalse;
    /* only change the currently stored value if the new hour is valid */
    if (isTimeValid(hour, Minute, Second, TimeZone))
    {
        Hour = hour;
        /* report that a new hour has been set */
        status = OFTrue;
    }
    return status;
}


OFBool OFTime::setMinute(const unsigned int minute)
{
    OFBool status = OFFalse;
    /* only change the currently stored value if the new minute is valid */
    if (isTimeValid(Hour, minute, Second, TimeZone))
    {
        Minute = minute;
        /* report that a new minute has been set */
        status = OFTrue;
    }
    return status;
}


OFBool OFTime::setSecond(const double second)
{
    OFBool status = OFFalse;
    /* only change the currently stored value if the new second is valid */
    if (isTimeValid(Hour, Minute, second, TimeZone))
    {
        Second = second;
        /* report that a new second has been set */
        status = OFTrue;
    }
    return status;
}


OFBool OFTime::setTimeZone(const double timeZone)
{
    OFBool status = OFFalse;
    /* only change the currently stored value if the new time zone is valid */
    if (isTimeValid(Hour, Minute, Second, timeZone))
    {
        TimeZone = timeZone;
        /* report that a new time zone has been set */
        status = OFTrue;
    }
    return status;
}


OFBool OFTime::setTimeZone(const signed int hour,
                           const unsigned int minute)
{
    /* convert hour and minute values to one time zone value */
    const double timeZone = (hour < 0) ? (double)hour - (double)minute / 60 : (double)hour + (double)minute / 60;
    /* only change the currently stored value if the new time zone is valid */
    return setTimeZone(timeZone);
}


OFBool OFTime::setTimeInSeconds(const double seconds,
                                const double timeZone,
                                const OFBool normalize)
{
    OFBool status = OFFalse;
    /* only change if the new time is valid */
    if (normalize || ((seconds >= 0) && (seconds < 86400)))
    {
        /* first normalize the value first to the valid range of [0.0,86400.0[ */
        const double normalSeconds = (normalize) ? seconds + (signed long)(seconds / 86400) * 86400 : seconds;
        /* compute time from given number of seconds since "00:00:00" */
        const unsigned int newHour = (unsigned int)(normalSeconds / 3600);
        const unsigned int newMinute = (unsigned int)((normalSeconds - (double)newHour * 3600) / 60);
        const double newSecond = normalSeconds - (double)newHour * 3600 - (double)newMinute * 60;
        status = setTime(newHour, newMinute, newSecond, timeZone);
    }
    return status;
}


OFBool OFTime::setTimeInHours(const double hours,
                              const double timeZone,
                              const OFBool normalize)
{
    OFBool status = OFFalse;
    /* only change if the new time is valid */
    if (normalize || ((hours >= 0) && (hours < 24)))
    {
        /* first normalize the value to the valid range of [0.0,24.0[ */
        const double normalHours = (normalize) ? hours + (signed long)(hours / 24) * 24 : hours;
        /* compute time from given number of hours since "00:00:00" */
        const unsigned int newHour = (unsigned int)normalHours;
        const unsigned int newMinute = (unsigned int)((normalHours - (double)newHour) * 60);
        const double newSecond = (normalHours - (double)newHour) * 3600 - (double)newMinute * 60;
        status = setTime(newHour, newMinute, newSecond, timeZone);
    }
    return status;
}


OFBool OFTime::setCurrentTime()
{
    /* get the current system time and call the "real" function */
    return setCurrentTime(time(NULL));
}


OFBool OFTime::setCurrentTime(const time_t &tt)
{
    OFBool status = OFFalse;
#if defined(_REENTRANT) && !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__hpux)
    // use localtime_r instead of localtime
    struct tm ltBuf;
    struct tm *lt = &ltBuf;
    localtime_r(&tt, lt);
#else
    struct tm *lt = localtime(&tt);
#endif
    if (lt != NULL)
    {
        /* store retrieved time */
        Hour = lt->tm_hour;
        Minute = lt->tm_min;
        Second = lt->tm_sec;
#if defined(_REENTRANT) && !defined(_WIN32) && !defined(__CYGWIN__) && !defined(__hpux)
        // use gmtime_r instead of gmtime
        struct tm gtBuf;
        struct tm *gt = &gtBuf;
        gmtime_r(&tt, gt);
#else
        // avoid overwriting of local time structure by calling gmtime()
        struct tm ltBuf = *lt;
        lt = &ltBuf;
        struct tm *gt = gmtime(&tt);
#endif
        if (gt != NULL)
        {
            /* retrieve and store the time zone */
            TimeZone = (lt->tm_hour - gt->tm_hour) + (double)(lt->tm_min - gt->tm_min) / 60;
        } else {
            /* could not retrieve the time zone */
            TimeZone = 0;
        }
#ifdef HAVE_WINDOWS_H
        /* Windows: no microseconds available, use milliseconds instead */
        SYSTEMTIME timebuf;
        GetSystemTime(&timebuf);
        Second += (double)timebuf.wMilliseconds / 1000;
#else /* Unix */
        struct timeval tv;
        if (gettimeofday(&tv, NULL) == 0)
            Second += (double)tv.tv_usec / 1000000;
#endif
        /* report that current system time has been set */
        status = OFTrue;
    }
    return status;
}


unsigned int OFTime::getHour() const
{
    return Hour;
}


unsigned int OFTime::getMinute() const
{
    return Minute;
}


double OFTime::getSecond() const
{
    return Second;
}


unsigned int OFTime::getIntSecond() const
{
    /* return integral value of seconds */
    return (unsigned int)Second;
}


unsigned int OFTime::getMilliSecond() const
{
    return (unsigned int)((Second - (unsigned int)Second) * 1000);
}


unsigned int OFTime::getMicroSecond() const
{
    return (unsigned int)((Second - (unsigned int)Second) * 1000000);
}


double OFTime::getTimeZone() const
{
    return TimeZone;
}


double OFTime::getTimeInSeconds(const OFBool useTimeZone) const
{
    return getTimeInSeconds(Hour, Minute, Second, (useTimeZone) ? TimeZone : 0);
}


double OFTime::getTimeInHours(const OFBool useTimeZone) const
{
    return getTimeInHours(Hour, Minute, Second, (useTimeZone) ? TimeZone : 0);
}


double OFTime::getTimeInSeconds(const unsigned int hour,
                                const unsigned int minute,
                                const double second,
                                const double timeZone,
                                const OFBool normalize)
{
    /* compute number of seconds since 00:00:00 */
    double result = ((double)(hour - timeZone) * 60 + (double)minute) * 60 + second;
    /* normalize the result to the range [0.0,86400.0[ */
    if (normalize)
        result -= (unsigned long)(result / 86400) * 86400;
    return result;
}


double OFTime::getTimeInHours(const unsigned int hour,
                              const unsigned int minute,
                              const double second,
                              const double timeZone,
                              const OFBool normalize)
{
    /* compute number of hours since 00:00:00 (incl. fraction of hours) */
    double result = (double)hour - timeZone + ((double)minute + second / 60) / 60;
    /* normalize the result to the range [0.0,24.0[ */
    if (normalize)
        result -= (unsigned long)(result / 24) * 24;
    return result;
}


OFTime OFTime::getCoordinatedUniversalTime() const
{
    /* create a new time object */
    OFTime timeVal;
    /* convert time to UTC */
    timeVal.setTimeInHours(getTimeInHours(OFTrue /*useTimeZone*/), 0 /*timeZone*/);
    /* return by-value */
    return timeVal;
}


OFTime OFTime::getLocalTime() const
{
    /* create a new time object */
    OFTime timeVal;
    const double localTimeZone = getLocalTimeZone();
    /* convert time to local time */
    if (TimeZone != localTimeZone)
        timeVal.setTimeInHours(getTimeInHours(OFTrue /*useTimeZone*/) + localTimeZone, localTimeZone);
    else
    {
        /* same time zone, return currently stored time */
        timeVal = *this;
    }
    /* return by-value */
    return timeVal;
}


OFBool OFTime::getISOFormattedTime(OFString &formattedTime,
                                   const OFBool showSeconds,
                                   const OFBool showFraction,
                                   const OFBool showTimeZone,
                                   const OFBool showDelimiter) const
{
    OFBool status = OFFalse;
    /* check for valid time first */
    if (isValid())
    {
        char buf[32];
        /* format: HH:MM */
        if (showDelimiter)
            sprintf(buf, "%02u:%02u", Hour, Minute);
        /* format: HHMM */
        else
            sprintf(buf, "%02u%02u", Hour, Minute);
        if (showSeconds)
        {
            if (showFraction)
            {
                char buf2[12];
                OFStandard::ftoa(buf2, sizeof(buf2), Second,
                  OFStandard::ftoa_format_f | OFStandard::ftoa_zeropad, 9, 6);

                if (showDelimiter)
                    strcat(buf, ":");  /* format: HH:MM:SS.FFFFFF */
                strcat(buf, buf2);
            } else {
                /* format: HH:MM:SS*/
                if (showDelimiter)
                    sprintf(strchr(buf, 0), ":%02u", (unsigned int)Second);
                /* format: HHMMSS */
                else
                    sprintf(strchr(buf, 0), "%02u", (unsigned int)Second);
            }
        }
        if (showTimeZone)
        {
            /* convert time zone from hours and fraction of hours to hours and minutes */
            const char zoneSign = (TimeZone < 0) ? '-' : '+';
            const double zoneAbs = (TimeZone < 0) ? -TimeZone : TimeZone;
            const unsigned int zoneHour = (unsigned int)zoneAbs;
            const unsigned int zoneMinute = (unsigned int)((zoneAbs - zoneHour) * 60);
            /* format: ...+HH:MM or -HH:MM */
            if (showDelimiter)
                sprintf(strchr(buf, 0), "%c%02u:%02u", zoneSign, zoneHour, zoneMinute);
            /* format: ...+HHMM or -HHMM */
            else
                sprintf(strchr(buf, 0), "%c%02u%02u",  zoneSign, zoneHour, zoneMinute);
        }
        formattedTime = buf;
        status = OFTrue;
    }
    return status;
}


OFTime OFTime::getCurrentTime()
{
    /* create a time object with the current system time set */
    OFTime timeVal;
    /* this call might fail! */
    timeVal.setCurrentTime();
    /* return by-value */
    return timeVal;
}


double OFTime::getLocalTimeZone()
{
    double result = 0;
    /* determine local time zone */
    OFTime timeVal;
    if (timeVal.setCurrentTime())
        result = timeVal.getTimeZone();
    return result;
}


ostream& operator<<(ostream& stream, const OFTime &timeVal)
{
    OFString string;
    /* print the given time in ISO format to the stream */
    if (timeVal.getISOFormattedTime(string))
        stream << string;
    return stream;
}
