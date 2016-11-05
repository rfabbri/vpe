/*
 *
 *  Copyright (C) 1997-2002, OFFIS
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
 *  Author:  Andrew Hewett
 *
 *  Purpose: A simple string class
 *
 */


/*
** A simple string class
** - for OFFIS projects when an ANSI string class is not always available
** - based on the ANSI-C++ specifications
** - this impementation is intended to be slow but reliable
** - it is known to be slow but is it reliable
*/


#include "osconfig.h"     /* include OS specific configuration first */
#include "ofstring.h"

#define INCLUDE_CCTYPE
#include "ofstdinc.h"

/*
** Constructors
*/

OFString::OFString()
    : theCString(NULL), theCapacity(0)
{
    reserve(1);
}

OFString::OFString(const OFString& str, size_t pos, size_t n)
    : theCString(NULL), theCapacity(0)
{
    this->assign(str, pos, n);
}

OFString::OFString (const char* s, size_t n)
    : theCString(NULL), theCapacity(0)
{
    if (s) {
        if (n == OFString_npos) {
            n = strlen(s);
        }
        reserve(n);
        strncpy(this->theCString, s, n);
        this->theCString[n] = '\0';
    } else {
        reserve(1);
    }
}

OFString::OFString (const char* s)
    : theCString(NULL), theCapacity(0)
{
    if (s) {
        reserve(strlen(s));
        strcpy(this->theCString, s);
    } else {
        reserve(1);
    }
}


OFString::OFString (size_t rep, char c)
    : theCString(NULL), theCapacity(0)
{
    reserve(rep);
    for (int i=0; i<(int)rep; i++) {
        this->theCString[i] = c;
    }
}

/*
** Destructor
*/

OFString::~OFString()
{
    if (theCString) {
        delete[] theCString;
        theCString = NULL;
    }
}

/*
** Operator =
*/

OFString&
OFString::operator= (const OFString& rhs)
{
    this->assign(rhs);
    return *this;
}

OFString&
OFString::operator= (const char* s)
{
    this->assign(s);
    return *this;
}

OFString&
OFString::operator= (char s)
{
    this->assign(1, s);
    return *this;
}

/*
** Operator +=
*/

OFString&
OFString::operator+= (const OFString& rhs)
{
    return this->append(rhs);
}

OFString&
OFString::operator+= (const char* s)
{
    return this->append(s);
}

OFString&
OFString::operator+= (char s)
{
    return this->append(1, s);
}

/*
** Append
*/

OFString&
OFString::append (const OFString& str, size_t pos, size_t n)
{
    OFString b(str, pos, n);
    this->reserve(this->size() + b.size());
    strcat(this->theCString, b.theCString);
    return *this;
}

OFString&
OFString::append (const char* s, size_t n)
{
    OFString str(s, n);
    return this->append(str);
}

OFString&
OFString::append (const char* s)
{
    OFString str(s);
    return this->append(str);
}

OFString&
OFString::append (size_t rep, char c)
{
    OFString str(rep, c);
    return this->append(str);
}

/*
** Assign
*/

OFString&
OFString::assign (const OFString& str, size_t pos, size_t n)
{
    OFSTRING_OUTOFRANGE(pos > str.size());
    size_t remain = (str.size() - pos);
    if ((n == OFString_npos) || (n > remain)) {
        n = remain;
    }
    if (n > 0) {
        this->reserve(n);
        strncpy(this->theCString, str.theCString + pos, n);
        this->theCString[n] = '\0';
    } else {
        this->reserve(1);
        /* assign an empty string */
        this->theCString[0] = '\0';
    }
    return *this;
}

OFString&
OFString::assign (const char* s, size_t n)
{
    OFString str(s, n);
    return this->assign(str);
}

OFString&
OFString::assign (const char* s)
{
    OFString str(s);
    return this->assign(str);
}

OFString&
OFString::assign (size_t rep, char c)
{
    OFString str(rep, c);
    return this->assign(str);
}

/*
** Insert
*/

OFString&
OFString::insert (size_t pos1, const OFString& str, size_t pos2, size_t n)
{
    OFString i(str, pos2, n);
    OFString a(*this, (size_t)0, pos1);
    OFString b(*this, pos1);
    return this->assign(a).append(i).append(b);
}

OFString&
OFString::insert (size_t pos, const char* s, size_t n)
{
    OFString str(s, n);
    return this->insert(pos, str);
}

OFString&
OFString::insert (size_t pos, const char* s)
{
    OFString str(s);
    return this->insert(pos, str);
}

OFString&
OFString::insert (size_t pos, size_t rep, char s)
{
    OFString str(rep, s);
    return this->insert(pos, str);
}

/*
** Erase
*/
OFString&
OFString::erase (size_t pos, size_t n)
{
    OFString a(*this, 0, pos);
    OFString b;
    if (n != OFString_npos) {
        b.assign(*this, pos + n, OFString_npos);
    }
    return this->assign(a).append(b);
}

/*
** Replace
*/

OFString&
OFString::replace (size_t pos1, size_t n1, const OFString& str,
                   size_t pos2, size_t n2)
{
    OFString a(*this, (size_t)0, pos1);
    OFString b;
    if ((n1 < OFString_npos) && ((pos1 + n1) < this->size())) {
        b.assign(*this, pos1 + n1, OFString_npos);
    }
    OFString i(str, pos2, n2);
    return this->assign(a).append(i).append(b);
}

OFString&
OFString::replace (size_t pos, size_t n, const char* s, size_t n2)
{
    OFString str(s, n2);
    return this->replace(pos, n, str);
}

OFString&
OFString::replace (size_t pos, size_t n, const char* s)
{
    OFString str(s);
    return this->replace(pos, n, str);
}

OFString&
OFString::replace (size_t pos, size_t n, size_t rep, char s)
{
    OFString str(rep, s);
    return this->replace(pos, n, str);
}


/*
** Data
*/

const char*
OFString::data () const
{
    return ((this->size() != 0)?(this->c_str()): "");
}


/*
** Resize
*/

void
OFString::resize (size_t n, char c)
{
    OFSTRING_LENGTHERROR(n == OFString_npos);

    reserve(n);
    size_t len = this->size();
    if (n <= len) {
        for (size_t i=n; i<len; i++) {
            this->theCString[i] = '\0';
        }
    } else {
        for (size_t i=len; i<n; i++) {
            this->theCString[i] = c;
        }
        this->theCString[n] = '\0';
    }
}

/*
** Reserve
*/

void
OFString::reserve (size_t res_arg)
{
    if (res_arg == OFString_npos) {
        res_arg = 0; /* let at least space for eos get reserved */
    }
    res_arg++; /* add space for eos */
    if (this->theCapacity < res_arg) {
        char* newstr = new char[res_arg];
        if (newstr) {
            for (size_t i=0; i<res_arg; i++) {
                newstr[i] = '\0';
            }
            this->theCapacity = res_arg - 1; /* not the eos */
            if (this->size() > 0) {
                strcpy(newstr, this->theCString);
            } else {
                newstr[0] = '\0';
            }
            char* oldstr = this->theCString;
            this->theCString = newstr;
            delete[] oldstr;
        } else {
            OFSTRING_MEMORYALLOCERROR(newstr);
        }
    }
}

/*
** Copy
*/

size_t
OFString::copy (char* s, size_t n, size_t pos) const
{
    OFString sub(this->substr(pos, n));
    size_t result = sub.size();
    strncpy(s, sub.theCString, result);
    return result;
}

/*
** Substr
*/

OFString
OFString::substr (size_t pos, size_t n) const
{
    OFString sub;
    return sub.assign(*this, pos, n);
}

/*
** Swap
*/

void
OFString::swap(OFString& s)
{
    char* tmpCString = s.theCString;
    s.theCString = this->theCString;
    this->theCString = tmpCString;

    size_t tmpCapacity = s.theCapacity;
    s.theCapacity = this->theCapacity;
    this->theCapacity = tmpCapacity;
}

/*
** Compare
*/

int
OFString::compare (const OFString& str) const
{
    int this_size = this->size();
    int str_size = str.size();
    int rlen = (this_size < str_size)?(this_size):(str_size);
    int result = strncmp(this->theCString, str.theCString, rlen);
    if (result == 0) {
        result = (this_size - str_size);
    }
    return result;
}

int
OFString::compare (size_t pos1, size_t n1, const OFString& str) const
{
    return OFString(*this, pos1, n1).compare(str);
}

int
OFString::compare (size_t pos1, size_t n1, const OFString& str,
                   size_t pos2, size_t n2) const
{
    return OFString(*this, pos1, n1).compare(OFString(str, pos2, n2));
}

int
OFString::compare (const char* s) const
{
    return this->compare(OFString(s));
}

int
OFString::compare (size_t pos1, size_t n1,
                   const char* s, size_t n2) const
{
    return OFString(*this, pos1, n1).compare(OFString(s, n2));
}

/*
** Find
*/

size_t
OFString::find (const OFString& pattern, size_t pos) const
{
    if (size() == 0 || pattern.size() == 0 || pos == OFString_npos) {
        return OFString_npos;
    }
    for (size_t i=pos; i<this->size(); i++) {
        /* is there enought space for the pattern? */
        if ((i + pattern.size()) > this->size()) {
            return OFString_npos;
        }
        int match = 1; /* assume there is a match */
        for (size_t j=0; (j<pattern.size()) && match; j++) {
            if (this->at(i+j) != pattern[j]) {
                match = 0;
            }
        }
        if (match) {
            return i;
        }
    }
    return OFString_npos;
}

size_t
OFString::find (const char* pattern, size_t pos, size_t n) const
{
    OFString str(pattern, n);
    return this->find(str, pos);
}

size_t
OFString::find (const char* pattern, size_t pos) const
{
    OFString str(pattern);
    return this->find(str, pos);
}

size_t
OFString::find (char pattern, size_t pos) const
{
    OFString str(1, pattern);
    return this->find(str, pos);
}

/*
** Rfind
*/

size_t
OFString::rfind (const OFString& pattern, size_t pos) const
{
    if (size() == 0 || pattern.size() == 0 || this->size()<pattern.size()) {
        return OFString_npos;
    }

    int above = ((this->size()-pattern.size()) < pos)?
                 (this->size()-pattern.size()):(pos);

    for (int i=above; i>=0; i--) {
        int match = 1; /* assume there is a match */
        for (size_t j=0; (j<pattern.size()) && match; j++) {
            if (this->at(i+j) != pattern[j]) {
                match = 0;
            }
        }
        if (match) {
            return i;
        }
    }
    return OFString_npos;
}

size_t
OFString::rfind (const char* pattern, size_t pos, size_t n) const
{
    OFString str(pattern, n);
    return this->rfind(str, pos);
}

size_t
OFString::rfind (const char* pattern, size_t pos) const
{
    OFString str(pattern);
    return this->rfind(str, pos);
}

size_t
OFString::rfind (char pattern, size_t pos) const
{
    OFString str(1, pattern);
    return this->rfind(str, pos);
}

/*
** Find_first_of
*/

size_t
OFString::find_first_of (const OFString& str, size_t pos) const
{
    if (size() == 0 || str.size() == 0 || pos == OFString_npos) {
        return OFString_npos;
    }
    for (size_t i=pos; i<this->size(); i++) {
        for (size_t j=0; j<str.size(); j++) {
            if (this->at(i) == str[j]) {
                return i;
            }
        }
    }
    return OFString_npos;
}

size_t
OFString::find_first_of (const char* s, size_t pos, size_t n) const
{
    OFString str(s, n);
    return this->find_first_of(str, pos);
}

size_t
OFString::find_first_of (const char* s, size_t pos) const
{
    OFString str(s);
    return this->find_first_of(str, pos);
}

size_t
OFString::find_first_of (char s, size_t pos) const
{
    OFString str(1, s);
    return this->find_first_of(str, pos);
}

/*
** Find_last_of
*/

size_t
OFString::find_last_of (const OFString& str, size_t pos) const
{
    if (this->size() == 0 || str.size() == 0) {
        return OFString_npos;
    }
    if (pos == OFString_npos || pos > this->size()) {
        pos = this->size();
    }
    for (int i=(int)pos-1; i>=0; i--) {
        for (size_t j=0; j<str.size(); j++) {
            if (this->at(i) == str[j]) {
                return i;
            }
        }
    }
    return OFString_npos;
}

size_t
OFString::find_last_of (const char* s, size_t pos, size_t n) const
{
    OFString str(s, n);
    return this->find_last_of(str, pos);
}

size_t
OFString::find_last_of (const char* s, size_t pos) const
{
    OFString str(s);
    return this->find_last_of(str, pos);
}

size_t
OFString::find_last_of (char s, size_t pos) const
{
    OFString str(1, s);
    return this->find_last_of(str, pos);
}

/*
** Find_first_not_of
*/

size_t
OFString::find_first_not_of (const OFString& str, size_t pos) const
{
    if (this->size() == 0 || str.size() == 0 || pos == OFString_npos) {
        return OFString_npos;
    }
    for (size_t i=pos; i<this->size(); i++) {
        for (size_t j=0; j<str.size(); j++) {
            if (this->at(i) != str[j]) {
                return i;
            }
        }
    }
    return OFString_npos;
}

size_t
OFString::find_first_not_of (const char* s, size_t pos, size_t n) const
{
    OFString str(s, n);
    return this->find_first_not_of(str, pos);
}

size_t
OFString::find_first_not_of (const char* s, size_t pos) const
{
    OFString str(s);
    return this->find_first_not_of(str, pos);
}

size_t
OFString::find_first_not_of (char s, size_t pos) const
{
    OFString str(1, s);
    return this->find_first_not_of(str, pos);
}

/*
** Find_last_not_of
*/

size_t
OFString::find_last_not_of (const OFString& str, size_t pos) const
{
    if (this->size() == 0 || str.size() == 0) {
        return OFString_npos;
    }
    if (pos == OFString_npos) {
        pos = this->size();
    }
    for (int i=(int)pos-1; i>=0; i--) {
        for (size_t j=0; j<str.size(); j++) {
            if (this->at(i) != str[j]) {
                return i;
            }
        }
    }
    return OFString_npos;
}

size_t
OFString::find_last_not_of (const char* s, size_t pos, size_t n) const
{
    OFString str(s, n);
    return this->find_last_not_of(str, pos);
}

size_t
OFString::find_last_not_of (const char* s, size_t pos) const
{
    OFString str(s);
    return this->find_last_not_of(str, pos);
}

size_t
OFString::find_last_not_of (char s, size_t pos) const
{
    OFString str(1, s);
    return this->find_last_not_of(str, pos);
}

/*
** Operator <<
*/

ostream& operator<< (ostream& o, const OFString& s)
{
    return o.write (s.c_str(), s.size());
}

/*
** Operator >>
*/
istream& operator>> (istream& i, OFString& s)
{
    s.resize(0);
    char c = '\0';
    size_t n = s.max_size();
    if (i.width() > 0) {
        n = i.width();
    }
    // skip white space before word
    i.get(c);
    while (i.good() && isspace(c)) {
        i.get(c);
    }
    // get the word
    while (i.good() && !isspace(c) && n--) {
        s += c;
        i.get(c);
    }
    if (isspace(c)) {
        i.putback(c);
    }
    i.width(0);
    return i;
}

/*
** Operator +
*/

OFString operator+ (const OFString& lhs, const OFString& rhs)
{
    OFString s(lhs);
    s += rhs;
    return s;
}

OFString operator+ (const char* lhs, const OFString& rhs)
{
    OFString s(lhs);
    s += rhs;
    return s;
}

OFString operator+ (char lhs, const OFString& rhs)
{
    OFString s(1, lhs);
    s += rhs;
    return s;
}

OFString operator+ (const OFString& lhs, const char* rhs)
{
    OFString s(lhs);
    s += rhs;
    return s;
}

OFString operator+ (const OFString& lhs, char rhs)
{
    OFString s(lhs);
    s += rhs;
    return s;
}

/*
** Operator ==
*/

OFBool operator== (const OFString& lhs, const OFString& rhs)
{
    return (lhs.compare(rhs) == 0)?(OFTrue):(OFFalse);
}

OFBool operator== (const char* lhs, const OFString& rhs)
{
    OFString slhs(lhs);
    return (slhs == rhs);
}

OFBool operator== (char lhs, const OFString& rhs)
{
    OFString slhs(1, lhs);
    return (slhs == rhs);
}

OFBool operator== (const OFString& lhs, const char* rhs)
{
    OFString srhs(rhs);
    return (lhs == srhs);
}

OFBool operator== (const OFString& lhs, char rhs)
{
    OFString srhs(1, rhs);
    return (lhs == srhs);
}

/*
** Operator <
*/

OFBool operator< (const OFString& lhs, const OFString& rhs)
{
    return (lhs.compare(rhs) < 0)?(OFTrue):(OFFalse);
}

OFBool operator< (const char* lhs, const OFString& rhs)
{
    OFString slhs(lhs);
    return (slhs < rhs);
}

OFBool operator< (char lhs, const OFString& rhs)
{
    OFString slhs(1, lhs);
    return (slhs < rhs);
}

OFBool operator< (const OFString& lhs, const char* rhs)
{
    OFString srhs(rhs);
    return (lhs < srhs);
}

OFBool operator< (const OFString& lhs, char rhs)
{
    OFString srhs(1, rhs);
    return (lhs < srhs);
}

/*
** Operator <=
*/

OFBool operator<= (const OFString& lhs, const OFString& rhs)
{
    return (!(rhs < lhs));
}

OFBool operator<= (const char* lhs, const OFString& rhs)
{
    return (!(rhs < lhs));
}

OFBool operator<= (char lhs, const OFString& rhs)
{
    return (!(rhs < lhs));
}

OFBool operator<= (const OFString& lhs, const char* rhs)
{
    return (!(rhs < lhs));
}

OFBool operator<= (const OFString& lhs, char rhs)
{
    return (!(rhs < lhs));
}

/*
** Operator !=
*/

OFBool operator!= (const OFString& lhs, const OFString& rhs)
{
    return (!(lhs == rhs));
}

OFBool operator!= (const char* lhs, const OFString& rhs)
{
    return (!(lhs == rhs));
}

OFBool operator!= (char lhs, const OFString& rhs)
{
    return (!(lhs == rhs));
}

OFBool operator!= (const OFString& lhs, const char* rhs)
{
    return (!(lhs == rhs));
}

OFBool operator!= (const OFString& lhs, char rhs)
{
    return (!(lhs == rhs));
}

/*
** Operator >
*/

OFBool operator> (const OFString& lhs, const OFString& rhs)
{
    return (rhs < lhs);
}

OFBool operator> (const char* lhs, const OFString& rhs)
{
    return (rhs < lhs);
}

OFBool operator> (char lhs, const OFString& rhs)
{
    return (rhs < lhs);
}

OFBool operator> (const OFString& lhs, const char* rhs)
{
    return (rhs < lhs);
}

OFBool operator> (const OFString& lhs, char rhs)
{
    return (rhs < lhs);
}

/*
** Operator >=
*/

OFBool operator>= (const OFString& lhs, const OFString& rhs)
{
    return (!(lhs < rhs));
}

OFBool operator>= (const char* lhs, const OFString& rhs)
{
    return (!(lhs < rhs));
}

OFBool operator>= (char lhs, const OFString& rhs)
{
    return (!(lhs < rhs));
}

OFBool operator>= (const OFString& lhs, const char* rhs)
{
    return (!(lhs < rhs));
}

OFBool operator>= (const OFString& lhs, char rhs)
{
    return (!(lhs < rhs));
}
