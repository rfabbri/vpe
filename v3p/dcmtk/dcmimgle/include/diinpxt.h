/*
 *
 *  Copyright (C) 1996-2001, OFFIS
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
 *  Purpose: DicomInputPixelTemplate (Header)
 *
 */


#ifndef __DIINPXT_H
#define __DIINPXT_H

#include "osconfig.h"
#include "dctypes.h"
#include "dcpixel.h"

#include "ofbmanip.h"

#include "diinpx.h"
#include "dipxrept.h"
#include "diutils.h"


/********************************************************************/


static inline Uint8 expandSign(const Uint8 Value,
                               const Uint8,
                               const Uint8)
{
    return Value;
}


static inline Uint16 expandSign(const Uint16 Value,
                                const Uint16,
                                const Uint16)
{
    return Value;
}


static inline Uint32 expandSign(const Uint32 Value,
                                const Uint32,
                                const Uint32)
{
    return Value;
}


static inline Sint8 expandSign(const Sint8 Value,
                               const Sint8 SignBit,
                               const Sint8 SignMask)
{
    return (Value & SignBit) ? (Value | SignMask) : Value;
}


static inline Sint16 expandSign(const Sint16 Value,
                                const Sint16 SignBit,
                                const Sint16 SignMask)
{
    return (Value & SignBit) ? (Value | SignMask) : Value;
}


static inline Sint32 expandSign(const Sint32 Value,
                                const Sint32 SignBit,
                                const Sint32 SignMask)
{
    return (Value & SignBit) ? (Value | SignMask) : Value;
}


/********************************************************************/


static Uint32 getPixelData(DcmPixelData *PixelData,
                           Uint8 *&pixel)
{
    PixelData->getUint8Array(pixel);
    return PixelData->getLength();
}


static Uint32 getPixelData(DcmPixelData *PixelData,
                           Uint16 *&pixel)
{
    PixelData->getUint16Array(pixel);
    return PixelData->getLength();
}


/*---------------------*
 *  class declaration  *
 *---------------------*/

/** Template class to convert DICOM pixel stream to intermediate representation
 */
template<class T1, class T2>
class DiInputPixelTemplate
  : public DiInputPixel,
    public DiPixelRepresentationTemplate<T2>
{
 public:

    /** constructor
     *
     ** @param  pixel   pointer to DICOM dataset element containing the pixel data
     *  @param  alloc   number of bits allocated for each pixel
     *  @param  stored  number of bits stored for each pixel
     *  @param  high    position of bigh bit within bits allocated
     *  @param  start   start position of pixel data to be processed
     *  @param  count   number of pixels to be processed
     */
    DiInputPixelTemplate(/*const*/ DcmPixelData *pixel,
                         const Uint16 alloc,
                         const Uint16 stored,
                         const Uint16 high,
                         const unsigned long start,
                         const unsigned long count)
      : DiInputPixel(stored, start, count),
        Data(NULL)
    {
        MinValue[0] = 0;
        MinValue[1] = 0;
        MaxValue[0] = 0;
        MaxValue[1] = 0;
        if (this->isSigned())
        {
            AbsMinimum = -(double)DicomImageClass::maxval(Bits - 1, 0);
            AbsMaximum = (double)DicomImageClass::maxval(Bits - 1);
        } else {
            AbsMinimum = 0;
            AbsMaximum = (double)DicomImageClass::maxval(Bits);
        }
        if (pixel != NULL)
            convert(pixel, alloc, stored, high);
        if ((PixelCount == 0) || (PixelStart + PixelCount > Count))         // check for corrupt pixel length
            PixelCount = Count - PixelStart;
    }

    /** destructor
     */
    virtual ~DiInputPixelTemplate()
    {
        delete[] Data;
    }

    /** determine minimum and maximum pixel value
     *
     ** @return status, true if successful, false otherwise
     */
    int determineMinMax()
    {
        if (Data != NULL)
        {
            register T2 *p = Data;
            register unsigned long i;
            const unsigned long ocnt = (unsigned long)getAbsMaxRange();
            Uint8 *lut = NULL;
            if ((sizeof(T2) <= 2) && (Count > 3 * ocnt))               // optimization criteria
            {
                lut = new Uint8[ocnt];
                if (lut != NULL)
                {
                    OFBitmanipTemplate<Uint8>::zeroMem(lut, ocnt);
                    register Uint8 *q = lut - (T2)getAbsMinimum();
                    for (i = Count; i != 0; i--)                       // fill lookup table
                        *(q + *(p++)) = 1;
                    q = lut;
                    for (i = 0; i < ocnt; i++)                         // search for minimum
                    {
                        if (*(q++) != 0)
                        {
                            MinValue[0] = (T2)((double)i + getAbsMinimum());
                            break;
                        }
                    }
                    q = lut + ocnt;
                    for (i = ocnt; i != 0; i--)                        // search for maximum
                    {
                        if (*(--q) != 0)
                        {
                            MaxValue[0] = (T2)((double)(i - 1) + getAbsMinimum());
                            break;
                        }
                    }
                    if (Count >= PixelCount)                           // use global min/max value
                    {
                        MinValue[1] = MinValue[0];
                        MaxValue[1] = MaxValue[0];
                    } else {                                           // calculate min/max for selected range
                        OFBitmanipTemplate<Uint8>::zeroMem(lut, ocnt);
                        p = Data + PixelStart;
                        q = lut - (T2)getAbsMinimum();
                        for (i = PixelCount; i != 0; i--)                  // fill lookup table
                            *(q + *(p++)) = 1;
                        q = lut;
                        for (i = 0; i < ocnt; i++)                         // search for minimum
                        {
                            if (*(q++) != 0)
                            {
                                MinValue[1] = (T2)((double)i + getAbsMinimum());
                                break;
                            }
                        }
                        q = lut + ocnt;
                        for (i = ocnt; i != 0; i--)                         // search for maximum
                        {
                            if (*(--q) != 0)
                            {
                                MaxValue[1] = (T2)((double)(i - 1) + getAbsMinimum());
                                break;
                            }
                        }
                    }
                }
            }
            if (lut == NULL)                                           // use conventional method
            {
                register T2 value = *p;
                MinValue[0] = value;
                MaxValue[0] = value;
                for (i = Count; i > 1; i--)
                {
                    value = *(++p);
                    if (value < MinValue[0])
                        MinValue[0] = value;
                    else if (value > MaxValue[0])
                        MaxValue[0] = value;
                }
                if (Count <= PixelCount)                               // use global min/max value
                {
                    MinValue[1] = MinValue[0];
                    MaxValue[1] = MaxValue[0];
                } else {                                               // calculate min/max for selected range
                    p = Data + PixelStart;
                    value = *p;
                    MinValue[1] = value;
                    MaxValue[1] = value;
                    for (i = PixelCount; i > 1; i--)
                    {
                        value = *(++p);
                        if (value < MinValue[1])
                            MinValue[1] = value;
                        else if (value > MaxValue[1])
                            MaxValue[1] = value;
                    }
                }
            }
            delete[] lut;
            return 1;
        }
        return 0;
    }

    /** get pixel representation
     *
     ** @return pixel representation
     */
    inline EP_Representation getRepresentation() const
    {
        return DiPixelRepresentationTemplate<T2>::getRepresentation();
    }

    /** get pointer to input pixel data
     *
     ** @return pointer to input pixel data
     */
    inline void *getData() const
    {
        return (void *)Data;
    }

    /** remove reference to (internally handled) pixel data
     */
    inline void removeDataReference()
    {
        Data = NULL;
    }

    /** get minimum pixel value
     *
     ** @param  idx  specifies whether to return the global minimum (0) or
     *               the minimum of the selected pixel range (1, see PixelStart/Range)
     *
     ** @return minimum pixel value
     */
    inline double getMinValue(const int idx) const
    {
        return (idx == 0) ? (double)MinValue[0] : (double)MinValue[1];
    }

    /** get maximum pixel value
     *
     ** @param  idx  specifies whether to return the global maximum (0) or
     *               the maximum of the selected pixel range (1, see PixelStart/Range)
     *
     ** @return maximum pixel value
     */
    inline double getMaxValue(const int idx) const
    {
        return (idx == 0) ? (double)MaxValue[0] : (double)MaxValue[1];
    }


 private:

    /** convert pixel data from DICOM dataset to input representation
     *
     ** @param  pixelData      pointer to DICOM dataset element containing the pixel data
     *  @param  bitsAllocated  number of bits allocated for each pixel
     *  @param  bitsStored     number of bits stored for each pixel
     *  @param  highBit        position of bigh bit within bits allocated
     */
    void convert(/*const*/ DcmPixelData *pixelData,
                 const Uint16 bitsAllocated,
                 const Uint16 bitsStored,
                 const Uint16 highBit)
    {
        const Uint16 bitsof_T1 = bitsof(T1);
        const Uint16 bitsof_T2 = bitsof(T2);
        T1 *pixel;
        const Uint32 length_Bytes = getPixelData(pixelData, pixel);
        const Uint32 length_T1 = length_Bytes / sizeof(T1);
        Count = ((length_Bytes * 8) + bitsAllocated - 1) / bitsAllocated;
        register unsigned long i;
        Data = new T2[Count];
        if (Data != NULL)
        {
#ifdef DEBUG
            if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
            {
                ofConsole.lockCerr() << bitsAllocated << " " << bitsStored << " " << highBit << " " << this->isSigned() << endl;
                ofConsole.unlockCerr();
            }
#endif
            register const T1 *p = pixel;
            register T2 *q = Data;
            if (bitsof_T1 == bitsAllocated)                                             // case 1: equal 8/16 bit
            {
                if (bitsStored == bitsAllocated)
                {
#ifdef DEBUG
                    if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                    {
                        ofConsole.lockCerr() << "convert pixelData: case 1a (single copy)" << endl;
                        ofConsole.unlockCerr();
                    }
#endif
                    for (i = Count; i != 0; i--)
                        *(q++) = (T2)*(p++);
                }
                else /* bitsStored < bitsAllocated */
                {
                    register T1 mask = 0;
                    for (i = 0; i < bitsStored; i++)
                        mask |= (T1)(1 << i);
                    const T2 sign = 1 << (bitsStored - 1);
                    T2 smask = 0;
                    for (i = bitsStored; i < bitsof_T2; i++)
                        smask |= (T2)(1 << i);
                    const Uint16 shift = highBit + 1 - bitsStored;
                    if (shift == 0)
                    {
#ifdef DEBUG
                        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                        {
                            ofConsole.lockCerr() << "convert pixelData: case 1b (mask & sign)" << endl;
                            ofConsole.unlockCerr();
                        }
#endif
                        for (i = length_T1; i != 0; i--)
                            *(q++) = expandSign((T2)(*(p++) & mask), sign, smask);
                    }
                    else /* shift > 0 */
                    {
#ifdef DEBUG
                        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                        {
                            ofConsole.lockCerr() << "convert pixelData: case 1c (shift & mask & sign)" << endl;
                            ofConsole.unlockCerr();
                        }
#endif
                        for (i = length_T1; i != 0; i--)
                            *(q++) = expandSign((T2)((*(p++) >> shift) & mask), sign, smask);
                    }
                }
            }
            else if ((bitsof_T1 > bitsAllocated) && (bitsof_T1 % bitsAllocated == 0))   // case 2: divisor of 8/16 bit
            {
                const Uint16 times = bitsof_T1 / bitsAllocated;
                register T1 mask = 0;
                for (i = 0; i < bitsStored; i++)
                    mask |= (T1)(1 << i);
                register Uint16 j;
                register T1 value;
                if ((bitsStored == bitsAllocated) && (bitsStored == bitsof_T2))
                {
                    if (times == 2)
                    {
#ifdef DEBUG
                        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                        {
                            ofConsole.lockCerr() << "convert pixelData: case 2a (simple mask)" << endl;
                            ofConsole.unlockCerr();
                        }
#endif
                        for (i = length_T1; i != 0; i--, p++)
                        {
                            *(q++) = (T2)(*p & mask);
                            *(q++) = (T2)(*p >> bitsAllocated);
                        }
                    }
                    else
                    {
#ifdef DEBUG
                        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                        {
                            ofConsole.lockCerr() << "convert pixelData: case 2b (mask)" << endl;
                            ofConsole.unlockCerr();
                        }
#endif
                        for (i = length_T1; i != 0; i--)
                        {
                            value = *(p++);
                            for (j = times; j != 0; j--)
                            {
                                *(q++) = (T2)(value & mask);
                                value >>= bitsAllocated;
                            }
                        }
                    }
                }
                else
                {
#ifdef DEBUG
                    if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                    {
                        ofConsole.lockCerr() << "convert pixelData: case 2c (shift & mask & sign)" << endl;
                        ofConsole.unlockCerr();
                    }
#endif
                    const T2 sign = 1 << (bitsStored - 1);
                    T2 smask = 0;
                    for (i = bitsStored; i < bitsof_T2; i++)
                        smask |= (T2)(1 << i);
                    const Uint16 shift = highBit + 1 - bitsStored;
                    for (i = length_T1; i != 0; i--)
                    {
                        value = *(p++) >> shift;
                        for (j = times; j != 0; j--)
                        {
                            *(q++) = expandSign((T2)(value & mask), sign, smask);
                            value >>= bitsAllocated;
                        }
                    }
                }
            }
            else if ((bitsof_T1 < bitsAllocated) && (bitsAllocated % bitsof_T1 == 0)    // case 3: multiplicant of 8/16
                && (bitsStored == bitsAllocated))
            {
#ifdef DEBUG
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                {
                    ofConsole.lockCerr() << "convert pixelData: case 3 (multi copy)" << endl;
                    ofConsole.unlockCerr();
                }
#endif
                const Uint16 times = bitsAllocated / bitsof_T1;
                register Uint16 j;
                register Uint16 shift;
                register T2 value;
                for (i = length_T1; i != 0; i--)
                {
                    shift = 0;
                    value = (T2)*(p++);
                    for (j = times; j > 1; j--, i--)
                    {
                        shift += bitsof_T1;
                        value |= (T2)*(p++) << shift;
                    }
                    *(q++) = value;
                }
            }
            else                                                                        // case 4: anything else
            {
#ifdef DEBUG
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                {
                    ofConsole.lockCerr() << "convert pixelData: case 4 (general)" << endl;
                    ofConsole.unlockCerr();
                }
#endif
                register T2 value = 0;
                register Uint16 bits = 0;
                register Uint32 skip = highBit + 1 - bitsStored;
                register Uint32 times;
                T1 mask[bitsof_T1];
                mask[0] = 1;
                for (i = 1; i < bitsof_T1; i++)
                    mask[i] = (mask[i - 1] << 1) | 1;
                T2 smask = 0;
                for (i = bitsStored; i < bitsof_T2; i++)
                    smask |= (T2)(1 << i);
                const T2 sign = 1 << (bitsStored - 1);
                const Uint32 gap = bitsAllocated - bitsStored;
                i = 0;
                while (i < length_T1)
                {
                    if (skip < bitsof_T1)
                    {
                        if (skip + bitsStored - bits < bitsof_T1)       // -++- --++
                        {
                            value |= ((T2)((*p >> skip) & mask[bitsStored - bits - 1]) << bits);
                            skip += bitsStored - bits + gap;
                            bits = bitsStored;
                        }
                        else                                            // ++-- ++++
                        {
                            value |= ((T2)((*p >> skip) & mask[bitsof_T1 - skip - 1]) << bits);
                            bits += bitsof_T1 - (Uint16)skip;
                            skip = (bits == bitsStored) ? gap : 0;
                            i++;
                            p++;
                        }
                        if (bits == bitsStored)
                        {
                            *(q++) = expandSign(value, sign, smask);
                            value = 0;
                            bits = 0;
                        }
                    }
                    else
                    {
                        times = skip / bitsof_T1;
                        i += times;
                        p += times;
                        skip -= times * bitsof_T1;
                    }
                }
            }
        }
    }

    /// pointer to pixel data
    T2 *Data;

    /// minimum pixel value ([0] = global, [1] = selected pixel range)
    T2 MinValue[2];
    /// maximum pixel value ([0] = global, [1] = selected pixel range)
    T2 MaxValue[2];

 // --- declarations to avoid compiler warnings

    DiInputPixelTemplate(const DiInputPixelTemplate<T1,T2> &);
    DiInputPixelTemplate<T1,T2> &operator=(const DiInputPixelTemplate<T1,T2> &);
};


#endif
