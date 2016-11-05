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


#ifndef __OFCOND_H
#define __OFCOND_H

#include "osconfig.h"
#include "oftypes.h"   /* for class OFBool */
#include "ofstring.h"  /* for class OFString */


/** this enumeration describes the return status of an operation.
 */
enum OFStatus
{
  /// no error, operation has completed successfully
  OF_ok,

  /// operation has not completed successfully
  OF_error,

  /// application failure
  OF_failure
};


/** abstract base class for condition codes
 */
class OFConditionBase
{
 public:

  /// default constructor
  OFConditionBase()
  {
  }

  /// copy constructor
  OFConditionBase(const OFConditionBase& /* arg */)
  {
  }

  /// destructor
  virtual ~OFConditionBase()
  {
  }

  /** this method returns a pointer to a OFConditionBase object containing a clone
   *  of this object. If deletable() is true, the clone must be a deep copy allocated
   *  on the heap.  If deletable() is false, the clone should be a pointer to this.
   *  @return clone of this object, either deep copy or alias.
   */
  virtual const OFConditionBase *clone() const = 0;

  /** returns a combined code and module for this object.
   *  code is lower 16 bits, module is upper 16 bits
   */
  virtual unsigned long codeAndModule() const = 0;

  /// returns the status for this object.
  virtual OFStatus status() const = 0;

  /// returns the error message text for this object.
  virtual const char *text() const = 0;

  /** checks if this object is deletable, e.g. all instances
   *  of this class are allocated on the heap.
   *  @return true if deletable, false otherwise
   */
  virtual OFBool deletable() const = 0;

  /// returns the module identifier for this object.
  unsigned short module() const
  {
    return (unsigned short)((codeAndModule() >> 16) & 0xFFFF);
  }

  /// returns the status code identifier for this object.
  unsigned short code() const
  {
    return (unsigned short)(codeAndModule() & 0xFFFF);
  }

  /** comparison operator.
   *  Compares status, code and module but not error text.
   *  @param arg argument to compare to
   *  @return true if equal, false otherwise
   */
  OFBool operator==(const OFConditionBase& arg) const
  {
    return (status() == arg.status()) && (codeAndModule() == arg.codeAndModule());
  }

  /** comparison operator, not equal.
   *  Compares status, code and module but not error text.
   *  @param arg argument to compare to
   *  @return false if equal, true otherwise
   */
  OFBool operator!=(const OFConditionBase& arg) const
  {
    return (status() != arg.status()) || (code() != arg.code()) || (module() != arg.module());
  }

 private:

  // private unimplemented copy assignment operator
  OFConditionBase& operator=(const OFConditionBase& arg);
};


/** this class is used to declare global condition constants.
 *  OFError instances may keep multiple aliased pointers to an instance of
 *  this class. Therefore, instances should be global constants.
 */
class OFConditionConst: public OFConditionBase
{
 public:

  /** constructor.
   *  @param aModule module identifier. 0 is reserved for global codes,
   *    other constants are defined elsewhere.
   *  @param aCode status code that is unique for each module
   *  @param aStatus condition status enum
   *  @param aText error text.  The text is not copied, so the pointer must
   *    remain valid for the full lifetime of this object.
   */
  OFConditionConst(unsigned short aModule, unsigned short aCode, OFStatus aStatus, const char *aText)
  : OFConditionBase()
  , theCodeAndModule((unsigned long)aCode | ((unsigned long)aModule << 16))
  , theStatus(aStatus)
  , theText(aText)
  {
  }

  /// copy constructor
  OFConditionConst(const OFConditionConst& arg)
  : OFConditionBase(arg)
  , theCodeAndModule(arg.theCodeAndModule)
  , theStatus(arg.theStatus)
  , theText(arg.theText)
  {
  }

  /// destructor
  virtual ~OFConditionConst()
  {
  }

  /** this method returns a pointer to a OFConditionBase object containing a clone
   *  of this object. In this case, deletable() is false and clone just returns a pointer to this.
   *  @return alias of this object
   */
  virtual const OFConditionBase *clone() const;

  /** returns a combined code and module for this object.
   *  code is lower 16 bits, module is upper 16 bits
   */
  virtual unsigned long codeAndModule() const;

  /// returns the status for this object.
  virtual OFStatus status() const;

  /// returns the error message text for this object.
  virtual const char *text() const;

  /** checks if this object is deletable, e.g. all instances
   *  of this class are allocated on the heap.
   *  @return always false for this class
   */
  virtual OFBool deletable() const;

 private:

  /// private undefined copy assignment operator
  OFConditionConst& operator=(const OFConditionConst& arg);

  /// code/module identification. Code is lower 16 bits, module is upper 16 bits
  unsigned long theCodeAndModule;

  /// status
  OFStatus theStatus;

  /// condition description
  const char *theText;
};


/** this class is used to declare condition codes with
 *  user defined error messages.
 */
class OFConditionString: public OFConditionBase
{
 public:

  /** constructor.
   *  @param aModule module identifier. 0 is reserved for global codes,
   *    other constants are defined elsewhere.
   *  @param aCode status code that is unique for each module
   *  @param aStatus condition status enum
   *  @param aText error text. The text is copied.
   */
  OFConditionString(unsigned short aModule, unsigned short aCode, OFStatus aStatus, const char *aText)
  : OFConditionBase()
  , theCodeAndModule((unsigned long)aCode | ((unsigned long)aModule << 16))
  , theStatus(aStatus)
  , theText()
  {
    if (aText) theText = aText;
  }

  /// copy constructor
  OFConditionString(const OFConditionString& arg)
  : OFConditionBase(arg)
  , theCodeAndModule(arg.theCodeAndModule)
  , theStatus(arg.theStatus)
  , theText(arg.theText)
  {
  }

  /// destructor
  virtual ~OFConditionString()
  {
  }

  /** this method returns a pointer to a OFConditionBase object containing a clone
   *  of this object. The clone is a deep copy allocated on the heap.
   *  @return deep copy of this object
   */
  virtual const OFConditionBase *clone() const;

  /** returns a combined code and module for this object.
   *  code is lower 16 bits, module is upper 16 bits
   */
  virtual unsigned long codeAndModule() const;

  /// returns the status for this object.
  virtual OFStatus status() const;

  /// returns the error message text for this object.
  virtual const char *text() const;

  /** checks if this object is deletable, e.g. all instances
   *  of this class are allocated on the heap.
   *  @return true if deletable, false otherwise
   */
  virtual OFBool deletable() const;

 private:
  /// private undefined copy assignment operator
  OFConditionString& operator=(const OFConditionString& arg);

  /// code/module identification. Code is lower 16 bits, module is upper 16 bits
  unsigned long theCodeAndModule;

  /// status
  OFStatus theStatus;

  /// condition description
  OFString theText;
};


// global constant used by OFCondition default constructor.
extern const OFConditionConst ECC_Normal;


/* General purpose class for condition codes. Objects of this class can be
 * efficiently passed by value since they only contain a single pointer and
 * no virtual methods. The condition code is maintained by the object of class
 * OFConditionBase pointed to.
 */
class OFCondition
{
 public:

  /** constructor for condition code with user-defined error text
   *  @param base pointer to error base, which must be allocated on the heap.
   *     The object pointed to is deleted upon destruction of this object.
   *     Pointer must not be NULL.
   */
  OFCondition(OFConditionString *base)
  : theCondition(base)
  {
    assert(theCondition);
  }

  /** constructor for condition code with global const condition object
   *  @param base reference to condition base, which must be guaranteed
   *     to exist for the lifetime of this (and every derived) object
   *     since it is only referenced but not copied.
   */
#ifdef OFCONDITION_STRICT_MODE
  // in strict mode OFCondition has no default constructor.
  OFCondition(const OFConditionConst& base)
#else
  OFCondition(const OFConditionConst& base = ECC_Normal)
#endif
  : theCondition(&base)
  {
    assert(theCondition);
  }

  /// copy constructor
  OFCondition(const OFCondition& arg)
  : theCondition(arg.theCondition->clone())
  {
    assert(theCondition);
  }

  /// destructor
  ~OFCondition()
  {
    if (theCondition->deletable())
    {
      delete (OFConditionBase *) theCondition; // cast away const
    }
  }

  /// copy assignment operator
  OFCondition& operator=(const OFCondition& arg)
  {
    if (&arg != this)
    {
      if (theCondition->deletable())
      {
        delete (OFConditionBase *) theCondition; // cast away const
      }
      theCondition = arg.theCondition->clone();
      assert(theCondition);
    }
    return *this;
  }

  /// returns the module identifier for this object.
  inline unsigned short module() const
  {
    return theCondition->module();
  }

  /// returns the status code identifier for this object.
  inline unsigned short code() const
  {
    return theCondition->code();
  }

  /// returns the status for this object.
  inline OFStatus status() const
  {
    return theCondition->status();
  }

  /// returns the error message text for this object.
  inline const char *text() const
  {
    return theCondition->text();
  }

  /// returns true if status is OK
  inline OFBool good() const
  {
    OFStatus s = theCondition->status();
    return s == OF_ok;
  }

  /// returns true if status is not OK, i. e. error or failure
  inline OFBool bad() const
  {
    OFStatus s = theCondition->status();
    return s != OF_ok;
  }

#ifdef OFCONDITION_IMPLICIT_BOOL_CONVERSION
  /* Implicit conversion from OFCondition to bool might
   * not always be a good idea since it can hide unwanted constructs.
   * Therefore, we disable this operator by default.
   */

  /** conversion operator to bool.
   *  @return true if status is OK, false otherwise
   */
  inline operator OFBool() const
  {
    return good();
  }
#endif

  /** comparison operator. Compares status, code and module
   *  but not error text.
   *  @param arg error to compare to
   *  @return true if equal, false otherwise
   */
  inline OFBool operator==(const OFCondition& arg) const
  {
    return *theCondition == *arg.theCondition;
  }

  /** comparison operator. Compares status, code and module
   *  but not error text.
   *  @param arg error to compare to
   *  @return true if equal, false otherwise
   */
  inline OFBool operator!=(const OFCondition& arg) const
  {
    return *theCondition != *arg.theCondition;
  }

 private:

  /// pointer to the condition base object
  const OFConditionBase *theCondition;
};


/*  global condition constants.
 *  All constants defined here use module number 0 which is reserved for
 *  global definitions. Other constants are defined elsewhere.
 */

/// condition constant: successful completion
extern const OFCondition EC_Normal;

/// condition constant: error, function called with illegal parameters
extern const OFCondition EC_IllegalParameter;

/// condition constant: failure, memory exhausted
extern const OFCondition EC_MemoryExhausted;


/** this macro is a shortcut for creating user-specific error messages.
 */
#define makeOFCondition(A, B, C, D) OFCondition(new OFConditionString((A), (B), (C), (D)))


#endif
