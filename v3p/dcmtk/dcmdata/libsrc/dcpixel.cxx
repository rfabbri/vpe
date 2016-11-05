/*
 *
 *  Copyright (C) 1994-2001, OFFIS
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
 *  Purpose: class DcmPixelData
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcpixel.h"
#include "dccodec.h"
#include "dcpixseq.h"

//
// class DcmRepresentationEntry
//


class DcmRepresentationEntry
{
  friend class DcmPixelData;
 private:
    E_TransferSyntax repType;
    DcmRepresentationParameter * repParam;
    DcmPixelSequence * pixSeq;

    DcmRepresentationEntry(
        const DcmRepresentationEntry & oldEntry);
    DcmRepresentationEntry(
        const E_TransferSyntax rt,
        const DcmRepresentationParameter * rp,
        DcmPixelSequence * pixSeq);
    ~DcmRepresentationEntry();
    OFBool operator==(
        const DcmRepresentationEntry & x) const;
    OFBool operator!=(
        const DcmRepresentationEntry & x) const { return !(*this == x); }

 // --- declarations to avoid compiler warnings

    DcmRepresentationEntry &operator=(const DcmRepresentationEntry &);
};


DcmRepresentationEntry::DcmRepresentationEntry(
    const E_TransferSyntax rt,
    const DcmRepresentationParameter *rp,
    DcmPixelSequence * ps)
  : repType(rt),
    repParam(NULL),
    pixSeq(ps)
{
    if (rp)
        repParam = rp->clone();
}

DcmRepresentationEntry::DcmRepresentationEntry(
    const DcmRepresentationEntry & oldEntry)
  : repType(oldEntry.repType),
    repParam(NULL),
    pixSeq(NULL)
{
    if (oldEntry.repParam)
        repParam = oldEntry.repParam->clone();
    pixSeq = new DcmPixelSequence(*(oldEntry.pixSeq));
}

DcmRepresentationEntry::~DcmRepresentationEntry()
{
    if (repParam)
        delete repParam;
    if (pixSeq)
        delete pixSeq;
}

OFBool
DcmRepresentationEntry::operator==(const DcmRepresentationEntry & x) const
{
    return (repType == x.repType) &&
        ((x.repParam == NULL && repParam == NULL) ||
         ((x.repParam != NULL) && (repParam != NULL) && (*(x.repParam) == *repParam)));
}

//
// class DcmPixelData
//

// Constructors / Deconstructors

DcmPixelData::DcmPixelData(
    const DcmTag & tag,
    const Uint32 len)
  : DcmPolymorphOBOW(tag, len),
    repList(),
    repListEnd(),
    original(),
    current(),
    existUnencapsulated(OFFalse),
    unencapsulatedVR(EVR_UNKNOWN),
    pixelSeqForWrite(NULL)
{
    repListEnd = repList.end();
    current = original = repListEnd;
    if (Tag.getEVR() == EVR_ox) Tag.setVR(EVR_OW);
    unencapsulatedVR = Tag.getEVR();
    recalcVR();
}

DcmPixelData::DcmPixelData(
    const DcmPixelData & oldPixelData)
  : DcmPolymorphOBOW(oldPixelData),
    repList(),
    repListEnd(),
    original(),
    current(),
    existUnencapsulated(oldPixelData.existUnencapsulated),
    unencapsulatedVR(oldPixelData.unencapsulatedVR),
    pixelSeqForWrite(NULL)
{
    repListEnd = repList.end();
    original = repListEnd;
    current = original;
    recalcVR();
    DcmRepresentationListIterator oldEnd(oldPixelData.repList.end());
    for (DcmRepresentationListIterator it(oldPixelData.repList.begin());
         it != oldEnd;
         ++it)
    {
        DcmRepresentationEntry * repEnt = new DcmRepresentationEntry(**it);
        repList.push_back(repEnt);
        if (it == oldPixelData.original)
            original = --repList.end();

        if (it == oldPixelData.current)
        {
            current = --repList.end();
            recalcVR();
        }
    }
}


DcmPixelData::~DcmPixelData()
{
    for (DcmRepresentationListIterator it(repList.begin());
         it != repListEnd;
         ++it)
    {
        delete *it;
        *it = NULL;
    }
}

DcmPixelData &DcmPixelData::operator=(const DcmPixelData &obj)
{
  if (this != &obj)
  {
    DcmPolymorphOBOW::operator=(obj);
    existUnencapsulated = obj.existUnencapsulated;
    unencapsulatedVR = obj.unencapsulatedVR;
    pixelSeqForWrite = NULL;
    repList.clear();
    repListEnd = repList.end();
    original = repListEnd;
    current = original;
    recalcVR();
    DcmRepresentationListIterator oldEnd(obj.repList.end());
    DcmRepresentationListIterator it(obj.repList.begin());
    while (it != oldEnd)
    {
        DcmRepresentationEntry *repEnt = new DcmRepresentationEntry(**it);
        repList.push_back(repEnt);
        if (it == obj.original) original = --repList.end();
        if (it == current)
        {
            current = --repList.end();
            recalcVR();
        }
        ++it;
    }
  }
  return *this;
}

// methods in alphabetical order

Uint32
DcmPixelData::calcElementLength(
    const E_TransferSyntax xfer,
    const E_EncodingType enctype)
{
    DcmXfer xferSyn(xfer);
    errorFlag = EC_Normal;
    Uint32 elementLength = 0;

    if (xferSyn.isEncapsulated())
    {
        DcmRepresentationListIterator found;
        errorFlag =
            findConformingEncapsulatedRepresentation(xfer, NULL, found);
        if (errorFlag == EC_Normal)
            elementLength = (*found)->pixSeq->calcElementLength(xfer, enctype);
    }
    else if (existUnencapsulated)
        elementLength =  DcmPolymorphOBOW::calcElementLength(xfer, enctype);
    else
        errorFlag = EC_RepresentationNotFound;

    return elementLength;
}


OFBool
DcmPixelData::canChooseRepresentation(
    const E_TransferSyntax repType,
    const DcmRepresentationParameter * repParam)
{
    OFBool result = OFFalse;
    DcmXfer toType(repType);

    const DcmRepresentationEntry findEntry(repType, repParam, NULL);
    DcmRepresentationListIterator resultIt(repListEnd);
    if ((!toType.isEncapsulated() && existUnencapsulated) ||
        (toType.isEncapsulated() && findRepresentationEntry(findEntry, resultIt) == EC_Normal))
    {
        // representation found
        result = OFTrue;
    }
    else
    {
        // representation not found, check if we have a codec that can create the
        // desired representation.
        if (original == repListEnd)
        {
          result = DcmCodecList::canChangeCoding(EXS_LittleEndianExplicit, toType.getXfer());
        }
        else if (toType.isEncapsulated())
        {
          result = DcmCodecList::canChangeCoding(EXS_LittleEndianExplicit, toType.getXfer());

          if (!result)
          {
            // direct transcoding is not possible. Check if we can decode and then encode.
            result = canChooseRepresentation(EXS_LittleEndianExplicit, NULL);
            if (result) result = DcmCodecList::canChangeCoding(EXS_LittleEndianExplicit, toType.getXfer());
          }
        }
        else
        {
          result = DcmCodecList::canChangeCoding((*original)->repType, EXS_LittleEndianExplicit);
        }
    }
    return result;
}

OFBool
DcmPixelData::canWriteXfer(
    const E_TransferSyntax newXfer,
    const E_TransferSyntax /*oldXfer*/)
{
    DcmXfer newXferSyn(newXfer);
    DcmRepresentationListIterator found;
    OFBool result = !newXferSyn.isEncapsulated() && existUnencapsulated;

    if (!result && newXferSyn.isEncapsulated())
        result =
            findConformingEncapsulatedRepresentation(newXferSyn, NULL, found)
            == EC_Normal;
    return result;
}

OFCondition
DcmPixelData::chooseRepresentation(
        const E_TransferSyntax repType,
        const DcmRepresentationParameter * repParam,
        DcmStack & pixelStack)
{
    OFCondition l_error = EC_CannotChangeRepresentation;
    DcmXfer toType(repType);

    const DcmRepresentationEntry findEntry(repType, repParam, NULL);
    DcmRepresentationListIterator result(repListEnd);
    if ((!toType.isEncapsulated() && existUnencapsulated) ||
        (toType.isEncapsulated() &&
         findRepresentationEntry(findEntry, result) == EC_Normal))
    {
        // representation found
        current = result;
        recalcVR();
        l_error = EC_Normal;
    }
    else
    {
        if (original == repListEnd)
            l_error = encode(EXS_LittleEndianExplicit, NULL, NULL,
                             toType, repParam, pixelStack);
        else if (toType.isEncapsulated())
            l_error = encode((*original)->repType, (*original)->repParam,
                             (*original)->pixSeq, toType, repParam, pixelStack);
        else
            l_error = decode((*original)->repType, (*original)->repParam,
                             (*original)->pixSeq, pixelStack);
    }
    return l_error;
}


void
DcmPixelData::clearRepresentationList(
    DcmRepresentationListIterator leaveInList)
{
    /* define iterators to go through all representations in the list */
    DcmRepresentationListIterator it(repList.begin());
    DcmRepresentationListIterator del;

    /* as long as we have not encountered the end of the */
    /* representation list, go through all representations */
    while (it != repListEnd)
    {
        /* if this representation shall not be left in the list */
        if (it != leaveInList)
        {
            /* delete representation and move it to the next representation */
            delete *it;
            del = it++;
            repList.erase(del);
        }
        /* else leave this representation in the list and just go to the next */
        else
            ++it;
    }
}

OFCondition
DcmPixelData::decode(
    const DcmXfer & fromType,
    const DcmRepresentationParameter * fromParam,
    DcmPixelSequence * fromPixSeq,
    DcmStack & pixelStack)
{
    if (existUnencapsulated) return EC_Normal;
    OFCondition l_error = DcmCodecList::decode(fromType, fromParam, fromPixSeq, *this, pixelStack);
    if (l_error.good())
    {
        existUnencapsulated = OFTrue;
        current = repListEnd;
        setVR(EVR_OW);
        recalcVR();
    }
    else
    {
        DcmPolymorphOBOW::putUint16Array(NULL,0);
        existUnencapsulated = OFFalse;
    }
    return l_error;
}


OFCondition
DcmPixelData::encode(
    const DcmXfer & fromType,
    const DcmRepresentationParameter * fromParam,
    DcmPixelSequence * fromPixSeq,
    const DcmXfer & toType,
    const DcmRepresentationParameter *toParam,
    DcmStack & pixelStack)
{
    OFCondition l_error = EC_CannotChangeRepresentation;
    if (toType.isEncapsulated())
    {
       DcmPixelSequence * toPixSeq = NULL;
       if (fromType.isEncapsulated())
       {
         l_error = DcmCodecList::encode(fromType.getXfer(), fromParam, fromPixSeq,
                   toType.getXfer(), toParam, toPixSeq, pixelStack);
       }
       else
       {
         Uint16 * pixelData;
         l_error = DcmPolymorphOBOW::getUint16Array(pixelData);
         Uint32 length = DcmPolymorphOBOW::getLength();
         if (l_error == EC_Normal)
         {
           l_error = DcmCodecList::encode(fromType.getXfer(), pixelData, length,
                     toType.getXfer(), toParam, toPixSeq, pixelStack);
         }
       }

       if (l_error.good())
       {
           current = insertRepresentationEntry(
             new DcmRepresentationEntry(toType.getXfer(), toParam, toPixSeq));
           recalcVR();
       } else delete toPixSeq;

       // if it was possible to convert one encapsulated syntax into
       // another directly try it using decoding and encoding!
       if (l_error.bad() && fromType.isEncapsulated())
       {
           l_error = decode(fromType, fromParam, fromPixSeq, pixelStack);
           if (l_error.good()) l_error = encode(EXS_LittleEndianExplicit, NULL, NULL, toType, toParam, pixelStack);
       }
    }
    return l_error;
}

OFCondition
DcmPixelData::findRepresentationEntry(
    const DcmRepresentationEntry & findEntry,
    DcmRepresentationListIterator & result)
{
    result = repList.begin();
    while (result != repListEnd &&
           (*result)->repType < findEntry.repType)
        ++result;

    DcmRepresentationListIterator it(result);

    while (it != repListEnd && **it != findEntry)
        ++it;
    if (it == repListEnd || **it != findEntry)
        return EC_RepresentationNotFound;
    else
    {
        result = it;
        return EC_Normal;
    }
}


OFCondition
DcmPixelData::findConformingEncapsulatedRepresentation(
    const DcmXfer & repTypeSyn,
    const DcmRepresentationParameter * repParam,
    DcmRepresentationListIterator & result)
{
    E_TransferSyntax repType = repTypeSyn.getXfer();
    result = repListEnd;
    OFCondition l_error = EC_RepresentationNotFound;
    // we are looking for an encapsulated representation
    // of this pixel data element which meets both
    // the transfer syntax and (if given) the representation
    // parameter (i.e. quality factor for lossy JPEG).
    if (repTypeSyn.isEncapsulated())
    {
        // first we check the current (active) representation if any.
        if ((current != repListEnd) && ((*current)->repType == repType) &&
            ((repParam==NULL) || (((*current)->repParam != NULL)&&(*(*current)->repParam == *repParam))))
        {
            result = current;
            l_error = EC_Normal;
        }
        else
        {
            // now we check all representations
            DcmRepresentationListIterator it(repList.begin());
            OFBool found = OFFalse;
            while (!found && (it != repListEnd))
            {
              if ((*it)->repType == repType)
              {
                if ((repParam == NULL) || (((*it)->repParam != NULL)&&(*(*it)->repParam == *repParam)))
                {
                  // repParam is NULL or matches the one we are comparing with
                  found = OFTrue;
                  result = it;
                  l_error = EC_Normal;
                } else ++it;
              } else ++it;
            }
        }
    }
    return l_error;
}


OFCondition
DcmPixelData::getEncapsulatedRepresentation(
    const E_TransferSyntax repType,
    const DcmRepresentationParameter * repParam,
    DcmPixelSequence * & pixSeq)
{
    DcmRepresentationListIterator found;
    DcmRepresentationEntry findEntry(repType, repParam, NULL);
    if (findRepresentationEntry(findEntry, found) == EC_Normal)
    {
        pixSeq = (*found)->pixSeq;
        return EC_Normal;
    }
    else
        return EC_RepresentationNotFound;
}


OFBool
DcmPixelData::hasRepresentation(
    const E_TransferSyntax repType,
    const DcmRepresentationParameter * repParam)
{
    DcmXfer repTypeSyn(repType);
    DcmRepresentationListIterator found;
    if (!repTypeSyn.isEncapsulated() && existUnencapsulated)
        return OFTrue;
    else if (repTypeSyn.isEncapsulated())
        return findConformingEncapsulatedRepresentation(
            repTypeSyn, repParam, found) != EC_Normal;
    else
        return OFFalse;
}


Uint32
DcmPixelData::getLength(const E_TransferSyntax xfer,
                        const E_EncodingType enctype)
{
    DcmXfer xferSyn(xfer);
    errorFlag = EC_Normal;
    Uint32 valueLength = 0;

    if (xferSyn.isEncapsulated())
    {
        DcmRepresentationListIterator foundEntry;
        errorFlag = findConformingEncapsulatedRepresentation(
            xferSyn, NULL, foundEntry);
        if (errorFlag == EC_Normal)
            valueLength = (*foundEntry)->pixSeq->getLength(xfer, enctype);
    }
    else if (existUnencapsulated)
        valueLength =  DcmPolymorphOBOW::getLength(xfer, enctype);
    else
        errorFlag = EC_RepresentationNotFound;

    return valueLength;
}


void
DcmPixelData::getCurrentRepresentationKey(
    E_TransferSyntax & repType,
    const DcmRepresentationParameter * & repParam)
{
    if (current != repListEnd)
    {
        repType = (*current)->repType;
        repParam = (*current)->repParam;
    }
    else
    {
        repType = EXS_LittleEndianExplicit;
        repParam = NULL;
    }
}

void
DcmPixelData::getOriginalRepresentationKey(
    E_TransferSyntax & repType,
    const DcmRepresentationParameter * & repParam)
{
    if (original != repListEnd)
    {
        repType = (*original)->repType;
        repParam = (*original)->repParam;
    }
    else
    {
        repType = EXS_LittleEndianExplicit;
        repParam = NULL;
    }
}

DcmRepresentationListIterator
DcmPixelData::insertRepresentationEntry(
    DcmRepresentationEntry * repEntry)
{
    DcmRepresentationListIterator insertedEntry;
    DcmRepresentationListIterator result;
    if (findRepresentationEntry(*repEntry, result).good())
    {
        // this type of representation entry was already present in the list
        if (repEntry != *result)
        {
          insertedEntry = repList.insert(result, repEntry);

          // delete old entry from representation list
          delete *result;
          repList.erase(result);
        }
    }
    else
        insertedEntry = repList.insert(result,repEntry);
    return insertedEntry;
}

void
DcmPixelData::print(ostream &out,
                    const size_t flags,
                    const int level,
                    const char *pixelFileName,
                    size_t *pixelCounter)
{
    if (current == repListEnd)
        printPixel(out, flags, level, pixelFileName, pixelCounter);
    else
        (*current)->pixSeq->print(out, flags, level, pixelFileName, pixelCounter);
}

OFCondition
DcmPixelData::putUint16Array(
    const Uint16 * wordValue,
    const unsigned long length)
{
    // clear RepresentationList
    clearRepresentationList(repListEnd);
    OFCondition l_error = DcmPolymorphOBOW::putUint16Array(wordValue, length);
    original = current = repListEnd;
    recalcVR();
    existUnencapsulated = OFTrue;
    return l_error;
}

OFCondition
DcmPixelData::putUint8Array(
    const Uint8 * byteValue,
    const unsigned long length)
{
    // clear RepresentationList
    clearRepresentationList(repListEnd);
    OFCondition l_error = DcmPolymorphOBOW::putUint8Array(byteValue, length);
    original = current = repListEnd;
    recalcVR();
    existUnencapsulated = OFTrue;
    return l_error;
}

void
DcmPixelData::putOriginalRepresentation(
    const E_TransferSyntax repType,
    const DcmRepresentationParameter * repParam,
    DcmPixelSequence * pixSeq)
{
    // delete RepresentationList
    clearRepresentationList(repListEnd);
    // delete unencapsulated representation
    DcmPolymorphOBOW::putUint16Array(NULL,0);
    existUnencapsulated = OFFalse;
    // insert new Representation
    current = original = insertRepresentationEntry(
        new DcmRepresentationEntry(repType, repParam, pixSeq));
    recalcVR();
}

OFCondition
DcmPixelData::read(
    DcmInputStream & inStream,
    const E_TransferSyntax ixfer,
    const E_GrpLenEncoding glenc,
    const Uint32 maxReadLength)
{
    /* if this element's transfer state shows ERW_notInitialized, this is an illegal call */
    if (fTransferState == ERW_notInitialized)
        errorFlag = EC_IllegalCall;
    else
    {
        /* if this is not an illegal call, go ahead */

        /* if the transfer state is ERW_init, we need to prepare the reading of the pixel */
        /* data from the stream: remove all representations from the representation list. */
        if (fTransferState == ERW_init)
            clearRepresentationList(repListEnd);

        /* create a DcmXfer object based on the transfer syntax which was passed */
        DcmXfer ixferSyn(ixfer);

        /* determine if the pixel data is captured in native (uncompressed) or encapsulated
         * (compressed) format.  We only derive this information from the length field
         * which is set to undefined length for encapsulated data because even in
         * compressed transfer syntaxes the Icon Image Sequence may contain an
         * uncompressed image.
         */
        if (Length == DCM_UndefinedLength)
        {
            /* the pixel data is captured in encapsulated (compressed) format */

            /* if the transfer state is ERW_init, we need to prepare */
            /* the reading of the pixel data from the stream. */
            if (fTransferState == ERW_init)
            {
                current = insertRepresentationEntry(
                    new DcmRepresentationEntry(
                        ixfer, NULL, new DcmPixelSequence(Tag, Length)));
                recalcVR();
                original = current;
                existUnencapsulated = OFFalse;
                fTransferState = ERW_inWork;

                if (! ixferSyn.isEncapsulated())
                {
                  /* Special case: we have encountered a compressed image
                   * although we're decoding an uncompressed transfer syntax.
                   * This could be a compressed image stored without meta-header.
                   * For now, we just accept the data element; however, any attempt
                   * to write the dataset will fail because no suitable decoder
                   * is known.
                   */
                }
            }

            /* conduct the reading process */
            errorFlag =
                (*current)->pixSeq->read(inStream, ixfer, glenc, maxReadLength);

            /* if the errorFlag equals EC_Normal, all pixel data has been */
            /* read; hence, the transfer state has to be set to ERW_ready */
            if (errorFlag == EC_Normal)
                fTransferState = ERW_ready;
        }
        else
        {
            /* the pixel data is captured in native (uncompressed) format */

            /* if the transfer state is ERW_init, we need to prepare */
            /* the reading of the pixel data from the stream. */
            if (fTransferState == ERW_init)
            {
                current = original = repListEnd;
                unencapsulatedVR = Tag.getEVR();
                recalcVR();
                existUnencapsulated = OFTrue;

                if (ixferSyn.isEncapsulated())
                {
                  /* Special case: we have encountered an uncompressed image
                   * although we're decoding an encapsulated transfer syntax.
                   * This is probably an icon image.
                   */
                }
            }

            /* conduct the reading process */
            errorFlag =
                DcmPolymorphOBOW::read(inStream, ixfer, glenc, maxReadLength);
        }
    }

    /* return result value */
    return errorFlag;
}


void
DcmPixelData::removeAllButCurrentRepresentations()
{
    clearRepresentationList(current);
    if (current != repListEnd && existUnencapsulated)
    {
        DcmPolymorphOBOW::putUint16Array(NULL,0);
        existUnencapsulated = OFFalse;
    }
    original = current;
}


void
DcmPixelData::removeAllButOriginalRepresentations()
{
    clearRepresentationList(original);
    if (original != repListEnd && existUnencapsulated)
    {
        DcmPolymorphOBOW::putUint16Array(NULL,0);
        existUnencapsulated = OFFalse;
    }
    current = original;
    recalcVR();
}

OFCondition
DcmPixelData::removeOriginalRepresentation(
    const E_TransferSyntax repType,
    const DcmRepresentationParameter * repParam)
{
    OFCondition l_error = EC_Normal;
    DcmXfer repTypeSyn(repType);

    if (!repTypeSyn.isEncapsulated())
    {
        if (original != repListEnd)
        {
            if (current == original)
            {
                current = repListEnd;
                recalcVR();
            }
            repList.erase(original);
            original = repListEnd;
        }
        else
            l_error = EC_IllegalCall;
    }
    else
    {
        DcmRepresentationListIterator result;
        DcmRepresentationEntry findEntry(repType, repParam, NULL);
        if (findRepresentationEntry(findEntry, result) == EC_Normal)
        {
            if (result != original)
            {
                if (current == original)
                {
                    current = result;
                    recalcVR();
                }
                if (original == repListEnd)
                {
                    DcmPolymorphOBOW::putUint16Array(NULL, 0);
                    existUnencapsulated = OFFalse;
                }
                else
                    repList.erase(original);
                original = result;
            }
            else
                l_error = EC_IllegalCall;
        }
        else
            l_error = EC_RepresentationNotFound;
    }
    return l_error;
}

OFCondition
DcmPixelData::removeRepresentation(
    const E_TransferSyntax repType,
    const DcmRepresentationParameter * repParam)
{
    OFCondition l_error = EC_Normal;
    DcmXfer repTypeSyn(repType);

    if (!repTypeSyn.isEncapsulated())
    {
        if (original != repListEnd && existUnencapsulated)
        {
            DcmPolymorphOBOW::putUint16Array(NULL, 0);
            existUnencapsulated = OFFalse;
        }
        else
            l_error = EC_CannotChangeRepresentation;
    }
    else
    {
        DcmRepresentationListIterator result;
        DcmRepresentationEntry findEntry(repType, repParam, NULL);
        if (findRepresentationEntry(findEntry, result) == EC_Normal)
        {
            if (original != result)
                repList.erase(result);
            else l_error = EC_CannotChangeRepresentation;
        }
        else
            l_error = EC_RepresentationNotFound;
    }
    return l_error;
}


OFCondition
DcmPixelData::setCurrentRepresentationParameter(
    const DcmRepresentationParameter * repParam)
{
    if (current != repListEnd)
    {
        if (repParam == NULL)
            (*current)->repParam = NULL;
        else
            (*current)->repParam = repParam->clone();
        return EC_Normal;
    }
    return EC_RepresentationNotFound;
}

OFCondition
DcmPixelData::setVR(DcmEVR vr)
{
    unencapsulatedVR = vr;
    return DcmPolymorphOBOW::setVR(vr);
}

void
DcmPixelData::transferEnd()
{
    DcmPolymorphOBOW::transferEnd();
    for (DcmRepresentationListIterator it(repList.begin());
         it != repListEnd;
         ++it)
        (*it)->pixSeq->transferEnd();
}

void
DcmPixelData::transferInit()
{
    DcmPolymorphOBOW::transferInit();
    for (DcmRepresentationListIterator it(repList.begin());
         it != repListEnd;
         ++it)
        (*it)->pixSeq->transferInit();
}

OFCondition DcmPixelData::write(
    DcmOutputStream & outStream,
    const E_TransferSyntax oxfer,
    const E_EncodingType enctype)
{
  errorFlag = EC_Normal;
  if (fTransferState == ERW_notInitialized) errorFlag = EC_IllegalCall;
  else
  {
    DcmXfer xferSyn(oxfer);
    if (xferSyn.isEncapsulated())
    {
      if (fTransferState == ERW_init)
      {
        DcmRepresentationListIterator found;
        errorFlag = findConformingEncapsulatedRepresentation(xferSyn, NULL, found);
        if (errorFlag == EC_Normal)
        {
          current = found;
          recalcVR();
          pixelSeqForWrite = (*found)->pixSeq;
          fTransferState = ERW_inWork;
        }
      }
      if (errorFlag == EC_Normal && pixelSeqForWrite) errorFlag = pixelSeqForWrite->write(outStream, oxfer, enctype);
      if (errorFlag == EC_Normal) fTransferState = ERW_ready;
    }
    else if (existUnencapsulated)
    {
      current = repListEnd;
      recalcVR();
      errorFlag = DcmPolymorphOBOW::write(outStream, oxfer, enctype);
    }
    else if (getValue() == NULL)
    {
      errorFlag = DcmPolymorphOBOW::write(outStream, oxfer, enctype);
    } else errorFlag = EC_RepresentationNotFound;
  }
  return errorFlag;
}

OFCondition DcmPixelData::writeXML(
    ostream &out,
    const size_t flags)
{
    if (current == repListEnd)
    {
        errorFlag = DcmPolymorphOBOW::writeXML(out, flags);
    } else {
        /* pixel sequence (encapsulated data) */
        errorFlag = (*current)->pixSeq->writeXML(out, flags);
    }
    return errorFlag;
}

OFCondition DcmPixelData::writeSignatureFormat(
    DcmOutputStream & outStream,
    const E_TransferSyntax oxfer,
    const E_EncodingType enctype)
{
  errorFlag = EC_Normal;
  if (fTransferState == ERW_notInitialized) errorFlag = EC_IllegalCall;
  else if (Tag.isSignable())
  {
    DcmXfer xferSyn(oxfer);
    if (xferSyn.isEncapsulated())
    {
      if (fTransferState == ERW_init)
      {
        DcmRepresentationListIterator found;
        errorFlag = findConformingEncapsulatedRepresentation(xferSyn, NULL, found);
        if (errorFlag == EC_Normal)
        {
          current = found;
          recalcVR();
          pixelSeqForWrite = (*found)->pixSeq;
          fTransferState = ERW_inWork;
        }
      }
      if (errorFlag == EC_Normal && pixelSeqForWrite) errorFlag = pixelSeqForWrite->writeSignatureFormat(outStream, oxfer, enctype);
      if (errorFlag == EC_Normal) fTransferState = ERW_ready;
    }
    else if (existUnencapsulated)
    {
      current = repListEnd;
      recalcVR();
      errorFlag = DcmPolymorphOBOW::writeSignatureFormat(outStream, oxfer, enctype);
    }
    else if (getValue() == NULL)
    {
      errorFlag = DcmPolymorphOBOW::writeSignatureFormat(outStream, oxfer, enctype);
    } else errorFlag = EC_RepresentationNotFound;
  } else errorFlag = EC_Normal;
  return errorFlag;
}

OFCondition DcmPixelData::loadAllDataIntoMemory(void)
{
    if (current == repListEnd)
        return DcmElement::loadAllDataIntoMemory();
    else
        return (*current)->pixSeq->loadAllDataIntoMemory();
}
