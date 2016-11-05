/*
 *
 *  Copyright (C) 1996-2002, OFFIS
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
 *  Module:  dcmimgle
 *
 *  Author:  Joerg Riesmeier
 *
 *  Purpose: DicomDocument (Header)
 *
 */


#ifndef __DIDOCU_H
#define __DIDOCU_H

#include "osconfig.h"
#include "dctypes.h"
#include "dcfilefo.h"
#include "dcxfer.h"
#include "diobjcou.h"

#define INCLUDE_CSTDDEF
#include "ofstdinc.h"


/*------------------------*
 *  forward declarations  *
 *------------------------*/

class OFString;
class DcmStack;
class DcmObject;
class DcmTagKey;
class DcmElement;
class DcmSequenceOfItems;


/*---------------------*
 *  class declaration  *
 *---------------------*/

/** Interface class to DICOM data management (dcmdata)
 */
class DiDocument
  : public DiObjectCounter
{
 public:

    /** constructor, open a DICOM file
     *
     ** @param  filename  the DICOM file
     *  @param  flags     configuration flags (only stored for later use)
     *  @param  fstart    first frame to be processed (only stored for later use)
     *  @param  fcount    number of frames (only stored for later use)
     */
    DiDocument(const char *filename,
               const unsigned long flags = 0,
               const unsigned long fstart = 0,
               const unsigned long fcount = 0);

    /** constructor, use a given DcmObject
     *
     ** @param  object  pointer to dicom data structures
     *  @param  xfer    transfer syntax
     *  @param  flags   configuration flags (only stored for later use)
     *  @param  fstart  first frame to be processed (only stored for later use)
     *  @param  fcount  number of frames (only stored for later use)
     */
    DiDocument(DcmObject *object,
               const E_TransferSyntax xfer,
               const unsigned long flags = 0,
               const unsigned long fstart = 0,
               const unsigned long fcount = 0);

    /** destructor
     */
    virtual ~DiDocument();

    /** check whether current document is valid
     *
     ** @return status, true if successful, false otherwise
     */
    inline int good() const
    {
        return Object != NULL;
    }

    /** get current DICOM object (dataset)
     *
     ** @return pointer to DICOM object
     */
    inline DcmObject *getDicomObject() const
    {
        return Object;
    }

    /** get first frame to be processed
     *
     ** @return first frame to be processed
     */
    inline unsigned long getFrameStart() const
    {
        return FrameStart;
    }

    /** get number of frames to be processed
     *
     ** @return number of frames to be processed
     */
    inline unsigned long getFrameCount() const
    {
        return FrameCount;
    }

    /** get configuration flags
     *
     ** @return configuration flags
     */
    inline unsigned long getFlags() const
    {
        return Flags;
    }

    /** get transfer syntax of the DICOM dataset
     *
     ** @return transfer syntax
     */
    inline E_TransferSyntax getTransferSyntax() const
    {
        return Xfer;
    }

    /** search for given tag
     *
     ** @param  tag  tag to search for
     *  @param  obj  element in the dataset where the search should start (default: root)
     *
     ** @return pointer to element if successful, NULL otherwise
     */
    DcmElement *search(const DcmTagKey &tag,
                       DcmObject *obj = NULL) const;

    /** search for given tag and put result(s) on a stack
     *
     ** @param  tag    tag to search for
     *  @param  stack  stack where the result(s) should be stored
     *
     ** @return status, true if successful, false otherwise
     */
    int search(const DcmTagKey &tag,
               DcmStack &pstack) const;


    /** get value multiplicity (VM) of given tag
     *
     ** @param  tag  tag to be searched
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getVM(const DcmTagKey &tag) const;

    /** get value of given tag (Uint16)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *  @param  item       element in the dataset where the search should start (default: root)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           Uint16 &returnVal,
                           const unsigned long pos = 0,
                           DcmObject *item = NULL) const;

    /** get value of given tag (Sint16)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           Sint16 &returnVal,
                           const unsigned long pos = 0) const;

    /** get value of given tag (Uint32)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           Uint32 &returnVal,
                           const unsigned long pos = 0) const;

    /** get value of given tag (Sint32)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           Sint32 &returnVal,
                           const unsigned long pos = 0) const;

    /** get value of given tag (double)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           double &returnVal,
                           const unsigned long pos = 0) const;

    /** get value of given tag (Uint16 array)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  item       element in the dataset where the search should start (default: root)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           const Uint16 *&returnVal,
                           DcmObject *item = NULL) const;

    /** get value of given tag (const char *)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  item       element in the dataset where the search should start (default: root)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           const char *&returnVal,
                           DcmObject *item = NULL) const;

    /** get value of given tag (OFString)
     *
     ** @param  tag        tag to search for
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *  @param  item       element in the dataset where the search should start (default: root)
     *
     ** @return VM if successful, 0 otherwise
     */
    unsigned long getValue(const DcmTagKey &tag,
                           OFString &returnVal,
                           const unsigned long pos = 0,
                           DcmObject *item = NULL) const;

    /** get sequence of given tag
     *
     ** @param  tag  tag to search for
     *  @param  seq  reference to the storage area for the resulting value
     *
     ** @return cardinality if successful, 0 otherwise
     */
    unsigned long getSequence(const DcmTagKey &tag,
                              DcmSequenceOfItems *&seq) const;

    /** get value of given element (Uint16)
     *
     ** @param  elem       element where the value is stored
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *
     ** @return VM if successful, 0 otherwise
     */
    static unsigned long getElemValue(const DcmElement *elem,
                                      Uint16 &returnVal,
                                      const unsigned long pos = 0);

    /** get value of given element (Uint16 array)
     *
     ** @param  elem       element where the value is stored
     *  @param  returnVal  reference to the storage area for the resulting value
     *
     ** @return VM if successful, 0 otherwise
     */
    static unsigned long getElemValue(const DcmElement *elem,
                                      const Uint16 *&returnVal);

    /** get value of given element (const char *)
     *
     ** @param  elem       element where the value is stored
     *  @param  returnVal  reference to the storage area for the resulting value
     *
     ** @return VM if successful, 0 otherwise
     */
    static unsigned long getElemValue(const DcmElement *elem,
                                      const char *&returnVal);

    /** get value of given element (OFString)
     *
     ** @param  elem       element where the value is stored
     *  @param  returnVal  reference to the storage area for the resulting value
     *  @param  pos        position in muti-valued elements (starting with 0)
     *
     ** @return VM if successful, 0 otherwise
     */
    static unsigned long getElemValue(const DcmElement *elem,
                                      OFString &returnVal,
                                      const unsigned long pos = 0);

 protected:

    /** convert pixel data to uncompressed representation (if required)
     */
    void convertPixelData();


 private:

    /// reference to DICOM dataset (in memory)
    DcmObject *Object;
    /// reference to DICOM fileformat (read from file, maybe NULL)
    DcmFileFormat *FileFormat;
    /// transfer syntax used for reading the dataset
    E_TransferSyntax Xfer;

    /// first frame to be processed
    unsigned long FrameStart;
    /// number of frames to be processed
    unsigned long FrameCount;

    /// configuration flags
    unsigned long Flags;

 // --- declarations to avoid compiler warnings

    DiDocument(const DiDocument &);
    DiDocument &operator=(const DiDocument &);
};


#endif
