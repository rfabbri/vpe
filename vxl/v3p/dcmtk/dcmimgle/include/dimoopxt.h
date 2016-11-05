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
 *  Purpose: DicomMonoOutputPixelTemplate (Header)
 *
 */


#ifndef __DIMOOPXT_H
#define __DIMOOPXT_H

#include "osconfig.h"
#include "ofconsol.h"
#include "dctypes.h"

#include "dimoopx.h"
#include "diluptab.h"
#include "diovlay.h"
#include "dipxrept.h"
#include "diutils.h"
#include "didispfn.h"
#include "didislut.h"

#ifdef PASTEL_COLOR_OUTPUT
#include "dimcopxt.h"
#endif

#define INCLUDE_CMATH
#include "ofstdinc.h"


/*---------------------*
 *  class declaration  *
 *---------------------*/

/** Template class to create monochrome output data
 */
template<class T1, class T2, class T3>
class DiMonoOutputPixelTemplate
  : public DiMonoOutputPixel,
    public DiPixelRepresentationTemplate<T3>
{
 public:

    /** constructor
     *
     ** @param  buffer    storage area for the output pixel data (optional, maybe NULL)
     *  @param  pixel     pointer to intermediate pixel representation
     *  @param  overlays  array of overlay management objects
     *  @param  vlut      VOI LUT (optional, maybe NULL)
     *  @param  plut      presentation LUT (optional, maybe NULL)
     *  @param  disp      display function (optional, maybe NULL)
     *  @param  center    window center (optional, invalid if 'width' < 1)
     *  @param  width     window width (optional, invalid if < 1)
     *  @param  low       lowest pixel value for the output data (e.g. 0)
     *  @param  high      highest pixel value for the output data (e.g. 255)
     *  @param  columns   image's width (in pixels)
     *  @param  rows      image's height
     *  @param  frame     number of frame to be rendered
     *  @param  frames    total number of frames present in intermediate representation
     *  @param  pastel    flag indicating whether to use not only 'real' grayscale values (optional, experimental)
     */
    DiMonoOutputPixelTemplate(void *buffer,
                              const DiMonoPixel *pixel,
                              DiOverlay *overlays[2],
                              const DiLookupTable *vlut,
                              const DiLookupTable *plut,
                              DiDisplayFunction *disp,
                              const double center,
                              const double width,
                              const Uint32 low,
                              const Uint32 high,
                              const Uint16 columns,
                              const Uint16 rows,
                              const unsigned long frame,
#ifdef PASTEL_COLOR_OUTPUT
                              const unsigned long frames,
#else
                              const unsigned long /*frames*/,
#endif
                              const int pastel = 0)
      : DiMonoOutputPixel(pixel, (unsigned long)columns * (unsigned long)rows, frame, (unsigned long)fabs((double)(high - low))),
        Data(NULL),
        DeleteData(buffer == NULL),
        ColorData(NULL)
    {
        if ((pixel != NULL) && (Count > 0) && (FrameSize >= Count))
        {
            if (pastel)
#ifdef PASTEL_COLOR_OUTPUT
                color(buffer, pixel, frame, frames);
#else
            {
                ofConsole.lockCerr() << "WARNING: pastel color output not supported !" << endl;
                ofConsole.unlockCerr();
            }
#endif
            else
            {
                Data = (T3 *)buffer;
                if ((vlut != NULL) && (vlut->isValid()))            // valid VOI LUT ?
                    voilut(pixel, frame * FrameSize, vlut, plut, disp, (T3)low, (T3)high);
                else
                {
                    if (width < 1)                                  // no valid window according to supplement 33
                        nowindow(pixel, frame * FrameSize, plut, disp, (T3)low, (T3)high);
                    else
                        window(pixel, frame * FrameSize, plut, disp, center, width, (T3)low, (T3)high);
                }
                overlay(overlays, disp, columns, rows, frame);      // add (visible) overlay planes to output bitmap
            }
        }
    }

    /** destructor
     */
    virtual ~DiMonoOutputPixelTemplate()
    {
        if (DeleteData)
            delete[] Data;
        delete ColorData;
    }

    /** get integer representation of output data
     *
     ** @return integer representation
     */
    inline EP_Representation getRepresentation() const
    {
        return DiPixelRepresentationTemplate<T3>::getRepresentation();
    }

    /** get size of one pixel / item in the pixel array
     *
     ** @return item size
     */
    inline size_t getItemSize() const
    {
        return (ColorData != NULL) ? ColorData->getItemSize() : sizeof(T3);
    }

    /** get pointer to output pixel data
     *
     ** @return pointer to pixel data if sucessful, NULL otherwise
     */
    inline void *getData() const
    {
        return (ColorData != NULL) ? ColorData->getData() : (void *)Data;
    }

    /** remove reference to (internally handled) pixel data (abstract)
     */
    inline void removeDataReference()
    {
        Data = NULL;
        DeleteData = 0;
    }

    /** write pixel data of selected frame to PPM/ASCII file
     *
     ** @param  stream  open C++ output stream
     *
     ** @return status, true if successful, false otherwise
     */
    inline int writePPM(ostream &stream) const
    {
        if (Data != NULL)
        {
            register unsigned long i;
            for (i = 0; i < FrameSize; i++)
                stream << (unsigned long)Data[i] << " ";        // typecast to resolve problems with 'char'
            return 1;
        }
        if (ColorData != NULL)
            return ColorData->writePPM(stream);
        return 0;
    }

    /** write pixel data of selected frame to PPM/ASCII file
     *
     ** @param  stream  open C file stream
     *
     ** @return status, true if successful, false otherwise
     */
    inline int writePPM(FILE *stream) const
    {
        if (Data != NULL)
        {
            register unsigned long i;
            for (i = 0; i < FrameSize; i++)
                fprintf(stream, "%lu ", (unsigned long)Data[i]);
            return 1;
        }
        if (ColorData != NULL)
            return ColorData->writePPM(stream);
        return 0;
    }


 protected:

    /** examine which pixel values are actually used
     */
    inline void determineUsedValues()
    {
        if ((UsedValues == NULL) && (MaxValue > 0) && (MaxValue < MAX_TABLE_ENTRY_COUNT))
        {
            UsedValues = new Uint8[MaxValue + 1];
            if (UsedValues != NULL)
            {
                OFBitmanipTemplate<Uint8>::zeroMem(UsedValues, MaxValue + 1); // initialize array
                register const T3 *p = Data;
                register Uint8 *q = UsedValues;
                register unsigned long i;
                for (i = Count; i != 0; i--)
                    *(q + *(p++)) = 1;                                        // mark used entries
            }
        }
    }


 private:

    /** create a display LUT with the specified number of input bits
     *
     ** @param  dlut  reference to storage area where the display LUT should be stored
     *  @param  disp  pointer to object describing the current display function
     *  @param  bits  number of bits defining the input width of the display LUT
     */
    inline void createDisplayLUT(const DiDisplayLUT *&dlut,
                                 DiDisplayFunction *disp,
                                 const int bits)
    {
        if ((disp != NULL) && (disp->isValid()))
        {                                                                     // create Display LUT
            dlut = disp->getLookupTable(bits);
            if ((dlut != NULL) && (dlut->isValid()))                          // LUT is valid
            {
#ifdef DEBUG
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                {
                    ofConsole.lockCerr() << "INFO: using display transformation" << endl;
                    ofConsole.unlockCerr();
                }
#endif
            } else {
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Warnings))
                {
                    ofConsole.lockCerr() << "WARNING: can't create display LUT ... ignoring display transformation !" << endl;
                    ofConsole.unlockCerr();
                }
                dlut = NULL;
            }
        }
    }

    /** initialize an optimization LUT if the optimization criteria is fulfilled
     *
     ** @param  lut   reference to storage area where the optimization LUT should be stored
     *  @param  ocnt  number of entries for the optimization LUT
     */
    inline int initOptimizationLUT(T3 *&lut,
                                   const unsigned long ocnt)
    {
        int result = 0;
        if ((sizeof(T1) <= 2) && (Count > 3 * ocnt))                          // optimization criteria
        {                                                                     // use LUT for optimization
            lut = new T3[ocnt];
            if (lut != NULL)
            {
#ifdef DEBUG
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                {
                    ofConsole.lockCerr() << "INFO: using optimized routine with additional LUT" << endl;
                    ofConsole.unlockCerr();
                }
#endif
                result = 1;
            }
        }
        return result;
    }

#ifdef PASTEL_COLOR_OUTPUT
    void color(void *buffer,                               // create true color pastel image
               const DiMonoPixel *inter,
               const unsigned long frame,
               const unsigned long frames)
    {
        ColorData = new DiMonoColorOutputPixelTemplate<T1, T3>(buffer, inter, frame, frames);
        if (ColorData != NULL)
        {
            ofConsole.lockCout() << "COLOR" << endl;
            ofConsole.unlockCout();
        }
    }
#endif

    /** apply the currently active VOI LUT to the output data
     *
     ** @param  inter  pointer to intermediate pixel representation
     *  @param  start  offset of the first pixel to be processed
     *  @param  vlut   VOI LUT
     *  @param  plut   presentation LUT (optional, maybe NULL)
     *  @param  disp   display function (optional, maybe NULL)
     *  @param  low    lowest pixel value for the output data (e.g. 0)
     *  @param  high   highest pixel value for the output data (e.g. 255)
     */
    void voilut(const DiMonoPixel *inter,
                const Uint32 start,
                const DiLookupTable *vlut,
                const DiLookupTable *plut,
                DiDisplayFunction *disp,
                const T3 low,
                const T3 high)
    {
        const T1 *pixel = (const T1 *)inter->getData();
        if ((pixel != NULL) && (vlut != NULL))
        {
            if (Data == NULL)
                Data = new T3[FrameSize];
            if (Data != NULL)
            {
#ifdef DEBUG
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                {
                    ofConsole.lockCerr() << "INFO: using VOI routine 'voilut()'" << endl;
                    ofConsole.unlockCerr();
                }
#endif
                const DiDisplayLUT *dlut = NULL;
                const double minvalue = vlut->getMinValue();
                const double outrange = (double)high - (double)low + 1;
                register unsigned long i;
                if (minvalue == vlut->getMaxValue())                                    // LUT has only one entry or all entries are equal
                {
                    T3 value;
                    if ((plut != NULL) && (plut->isValid()))                            // has presentation LUT
                    {
#ifdef DEBUG
                        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                        {
                            ofConsole.lockCerr() << "INFO: using presentation LUT transformation" << endl;
                            ofConsole.unlockCerr();
                        }
#endif
                        createDisplayLUT(dlut, disp, plut->getBits());
                        const Uint32 value2 = (Uint32)((minvalue / (double)vlut->getAbsMaxRange()) * plut->getCount());
                        if (dlut != NULL)                                               // perform display transformation
                        {
                            /* UNTESTED !! */
                            if (low > high)                                             // invers
                                value = (T3)dlut->getValue((Uint16)(plut->getAbsMaxRange() - plut->getValue(value2) - 1));
                            else                                                        // normal
                                value = (T3)dlut->getValue((Uint16)plut->getValue(value2));
                        } else {                                                        // don't use display: invalid or absent
                            value = (T3)((double)low + (double)plut->getValue(value2) * outrange / (double)plut->getAbsMaxRange());
                        }
                    } else {                                                            // has no presentation LUT
                        createDisplayLUT(dlut, disp, vlut->getBits());
                        if (dlut != NULL)                                               // perform display transformation
                        {
                            /* UNTESTED !! */
                            if (low > high)                                             // invers
                                value = (T3)dlut->getValue((Uint16)(vlut->getAbsMaxRange() - minvalue - 1));
                            else                                                        // normal
                                value = (T3)dlut->getValue((Uint16)minvalue);
                        } else                                                          // don't use display: invalid or absent
                            value = (T3)((double)low + (minvalue / (double)vlut->getAbsMaxRange()) * outrange);
                    }
                    OFBitmanipTemplate<T3>::setMem(Data, value, Count);                 // set output pixels to LUT value
                }
                else
                {
                    register T2 value = 0;
                    const T2 absmin = (T2)inter->getAbsMinimum();
                    const T2 firstentry = vlut->getFirstEntry(value);                   // choose signed/unsigned method
                    const T2 lastentry = vlut->getLastEntry(value);
                    const unsigned long ocnt = (unsigned long)inter->getAbsMaxRange();  // number of LUT entries
                    register const T1 *p = pixel + start;
                    register T3 *q = Data;
                    T3 *lut = NULL;
                    if ((plut != NULL) && (plut->isValid()))                            // has presentation LUT
                    {
#ifdef DEBUG
                        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                        {
                            ofConsole.lockCerr() << "INFO: using presentation LUT transformation" << endl;
                            ofConsole.unlockCerr();
                        }
#endif
                        createDisplayLUT(dlut, disp, plut->getBits());
                        register Uint32 value2;                                         // presentation LUT is always unsigned
                        const Uint32 pcnt = plut->getCount();
                        const double gradient1 = (double)pcnt / (double)vlut->getAbsMaxRange();
                        const Uint32 firstvalue = (Uint32)((double)vlut->getFirstValue() * gradient1);
                        const Uint32 lastvalue = (Uint32)((double)vlut->getLastValue() * gradient1);
                        if (initOptimizationLUT(lut, ocnt))
                        {                                                                 // use LUT for optimization
                            q = lut;
                            if (dlut != NULL)                                             // perform display transformation
                            {
                                if (low > high)                                           // inverse
                                {
                                    const Uint16 maxvalue = (Uint16)(plut->getAbsMaxRange() - 1);
                                    for (i = 0; i < ocnt; i++)
                                    {
                                        value = (T2)i + absmin;
                                        if (value <= firstentry)
                                            value2 = firstvalue;
                                        else if (value >= lastentry)
                                            value2 = lastvalue;
                                        else
                                            value2 = (Uint32)((double)vlut->getValue(value) * gradient1);
                                        *(q++) = (T3)dlut->getValue((Uint16)(maxvalue - plut->getValue(value2)));
                                    }
                                } else {                                                  // normal
                                    for (i = 0; i < ocnt; i++)
                                    {
                                        value = (T2)i + absmin;
                                        if (value <= firstentry)
                                            value2 = firstvalue;
                                        else if (value >= lastentry)
                                            value2 = lastvalue;
                                        else
                                            value2 = (Uint32)((double)vlut->getValue(value) * gradient1);
                                        *(q++) = (T3)dlut->getValue(plut->getValue(value2));
                                    }
                                }
                            } else {                                                      // don't use display: invalid or absent
                                const double gradient2 = outrange / (double)plut->getAbsMaxRange();
                                for (i = 0; i < ocnt; i++)
                                {
                                    value = (T2)i + absmin;
                                    if (value <= firstentry)
                                        value2 = firstvalue;
                                    else if (value >= lastentry)
                                        value2 = lastvalue;
                                    else
                                        value2 = (Uint32)((double)vlut->getValue(value) * gradient1);
                                    *(q++) = (T3)((double)low + (double)plut->getValue(value2) * gradient2);
                                }
                            }
                            const T3 *lut0 = lut - (T2)inter->getAbsMinimum();            // points to 'zero' entry
                            q = Data;
                            for (i = Count; i != 0; i--)                                  // apply LUT
                                *(q++) = *(lut0 + (*(p++)));
                        }
                        if (lut == NULL)                                                  // use "normal" transformation
                        {
                            if (dlut != NULL)                                             // perform display transformation
                            {
                                if (low > high)                                           // inverse
                                {
                                    const Uint16 maxvalue = (Uint16)(Uint16)(vlut->getAbsMaxRange() - 1);
                                    for (i = Count; i != 0; i--)
                                    {
                                        value = (T2)(*(p++));                             // pixel value
                                        if (value <= firstentry)
                                            value2 = firstvalue;
                                        else if (value >= lastentry)
                                            value2 = lastvalue;
                                        else
                                            value2 = (Uint32)((double)vlut->getValue(value) * gradient1);
                                        *(q++) = (T3)dlut->getValue((Uint16)(maxvalue - plut->getValue(value2)));
                                    }
                                } else {                                                  // normal
                                    for (i = Count; i != 0; i--)
                                    {
                                        value = (T2)(*(p++));                             // pixel value
                                        if (value <= firstentry)
                                            value2 = firstvalue;
                                        else if (value >= lastentry)
                                            value2 = lastvalue;
                                        else
                                            value2 = (Uint32)((double)vlut->getValue(value) * gradient1);
                                        *(q++) = (T3)dlut->getValue(plut->getValue(value2));
                                    }
                                }
                            } else {                                                      // don't use display: invalid or absent
                                const double gradient2 = outrange / (double)plut->getAbsMaxRange();
                                for (i = Count; i != 0; i--)
                                {
                                    value = (T2)(*(p++));                                 // pixel value
                                    if (value <= firstentry)
                                        value2 = firstvalue;
                                    else if (value >= lastentry)
                                        value2 = lastvalue;
                                    else
                                        value2 = (Uint32)((double)vlut->getValue(value) * gradient1);
                                    *(q++) = (T3)((double)low + (double)plut->getValue(value2) * gradient2);
                                }
                            }
                        }
                    }
                    else
                    {                                                              // has no presentation LUT
                        createDisplayLUT(dlut, disp, vlut->getBits());
                        const double gradient = outrange / (double)vlut->getAbsMaxRange();
                        const T3 firstvalue = (T3)((double)low + (double)vlut->getFirstValue() * gradient);
                        const T3 lastvalue = (T3)((double)low + (double)vlut->getLastValue() * gradient);
                        if (initOptimizationLUT(lut, ocnt))
                        {                                                                 // use LUT for optimization
                            q = lut;
                            if (dlut != NULL)                                             // perform display transformation
                            {
                                if (low > high)                                           // inverse
                                {
                                    const Uint16 maxvalue = (Uint16)(Uint16)(vlut->getAbsMaxRange() - 1);
                                    for (i = 0; i < ocnt; i++)
                                    {
                                        value = (T2)i + absmin;
                                        if (value < firstentry)
                                            value = firstentry;
                                        else if (value > lastentry)
                                            value = lastentry;
                                        *(q++) = (T3)dlut->getValue((Uint16)(maxvalue - vlut->getValue(value)));
                                    }
                                } else {                                                  // normal
                                    for (i = 0; i < ocnt; i++)
                                    {
                                        value = (T2)i + absmin;
                                        if (value < firstentry)
                                            value = firstentry;
                                        else if (value > lastentry)
                                            value = lastentry;
                                        *(q++) = (T3)dlut->getValue(vlut->getValue(value));
                                    }
                                }
                            } else {                                                      // don't use display: invalid or absent
                                for (i = 0; i < ocnt; i++)                                // calculating LUT entries
                                {
                                    value = (T2)i + absmin;
                                    if (value <= firstentry)
                                        *(q++) = firstvalue;
                                    else if (value >= lastentry)
                                        *(q++) = lastvalue;
                                    else
                                        *(q++) = (T3)((double)low + (double)vlut->getValue(value) * gradient);
                                }
                            }
                            const T3 *lut0 = lut - (T2)inter->getAbsMinimum();            // points to 'zero' entry
                            q = Data;
                            for (i = Count; i != 0; i--)                                  // apply LUT
                                *(q++) = *(lut0 + (*(p++)));
                        }
                        if (lut == NULL)                                                  // use "normal" transformation
                        {
                            if (dlut != NULL)                                             // perform display transformation
                            {
                                if (low > high)                                           // inverse
                                {
                                    const Uint16 maxvalue = (Uint16)(Uint16)(vlut->getAbsMaxRange() - 1);
                                    for (i = Count; i != 0; i--)
                                    {
                                        value = (T2)(*(p++));
                                        if (value < firstentry)
                                            value = firstentry;
                                        else if (value > lastentry)
                                            value = lastentry;
                                        *(q++) = (T3)dlut->getValue((Uint16)(maxvalue - vlut->getValue(value)));
                                    }
                                } else {                                                  // normal
                                    for (i = Count; i != 0; i--)
                                    {
                                        value = (T2)(*(p++));
                                        if (value < firstentry)
                                            value = firstentry;
                                        else if (value > lastentry)
                                            value = lastentry;
                                        *(q++) = (T3)dlut->getValue(vlut->getValue(value));
                                    }
                                }
                            } else {                                                      // don't use display: invalid or absent
                                for (i = 0; i < Count; i++)
                                {
                                    value = (T2)(*(p++));
                                    if (value <= firstentry)
                                        *(q++) = firstvalue;
                                    else if (value >= lastentry)
                                        *(q++) = lastvalue;
                                    else
                                        *(q++) = (T3)((double)low + (double)vlut->getValue(value) * gradient);
                                }
                            }
                        }
                    }
                    delete[] lut;
                }
                if (Count < FrameSize)
                    OFBitmanipTemplate<T3>::zeroMem(Data + Count, FrameSize - Count);     // set remaining pixels of frame to zero
            }
        } else
            Data = NULL;
    }

    /** perform linear scaling to the output data (no windowing)
     *
     ** @param  inter  pointer to intermediate pixel representation
     *  @param  start  offset of the first pixel to be processed
     *  @param  plut   presentation LUT (optional, maybe NULL)
     *  @param  disp   display function (optional, maybe NULL)
     *  @param  low    lowest pixel value for the output data (e.g. 0)
     *  @param  high   highest pixel value for the output data (e.g. 255)
     */
    void nowindow(const DiMonoPixel *inter,
                  const Uint32 start,
                  const DiLookupTable *plut,
                  DiDisplayFunction *disp,
                  const T3 low,
                  const T3 high)
    {
        const DiDisplayLUT *dlut = NULL;
        const T1 *pixel = (const T1 *)inter->getData();
        if (pixel != NULL)
        {
            if (Data == NULL)                                                         // create new output buffer
                Data = new T3[FrameSize];
            if (Data != NULL)
            {
#ifdef DEBUG
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                {
                    ofConsole.lockCerr() << "INFO: using VOI routine 'nowindow()'" << endl;
                    ofConsole.unlockCerr();
                }
#endif
                const double absmin = inter->getAbsMinimum();
                const double absmax = inter->getAbsMaximum();
                const double outrange = (double)high - (double)low + 1;
                const unsigned long ocnt = (unsigned long)inter->getAbsMaxRange();     // number of LUT entries
                register const T1 *p = pixel + start;
                register T3 *q = Data;
                register unsigned long i;
                T3 *lut = NULL;
                if ((plut != NULL) && (plut->isValid()))                              // has presentation LUT
                {
#ifdef DEBUG
                    if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                    {
                        ofConsole.lockCerr() << "INFO: using presentation LUT transformation" << endl;
                        ofConsole.unlockCerr();
                    }
#endif
                    createDisplayLUT(dlut, disp, plut->getBits());
                    register Uint32 value;                                            // presentation LUT is always unsigned
                    const double gradient1 = (double)plut->getCount() / (inter->getAbsMaxRange());
                    const double gradient2 = outrange / (double)plut->getAbsMaxRange();
                    if (initOptimizationLUT(lut, ocnt))
                    {                                                                 // use LUT for optimization
                        q = lut;
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            if (low > high)                                           // inverse
                            {
                                const Uint16 maxvalue = (Uint16)(Uint16)(plut->getAbsMaxRange() - 1);
                                for (i = 0; i < ocnt; i++)
                                {
                                    value = (Uint32)((double)i * gradient1);
                                    *(q++) = (T3)dlut->getValue((Uint16)(maxvalue - plut->getValue(value)));
                                }
                            } else {                                                  // normal
                                for (i = 0; i < ocnt; i++)
                                {
                                    value = (Uint32)((double)i * gradient1);
                                    *(q++) = (T3)dlut->getValue(plut->getValue(value));
                                }
                            }
                        } else {                                                      // don't use display: invalid or absent
                            for (i = 0; i < ocnt; i++)
                            {
                                value = (Uint32)((double)i * gradient1);
                                *(q++) = (T3)((double)low + (double)plut->getValue(value) * gradient2);
                            }
                        }
                        const T3 *lut0 = lut - (T2)inter->getAbsMinimum();            // points to 'zero' entry
                        q = Data;
                        for (i = Count; i != 0; i--)                                  // apply LUT
                            *(q++) = *(lut0 + (*(p++)));
                    }
                    if (lut == NULL)                                                  // use "normal" transformation
                    {
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            if (low > high)                                           // inverse
                            {
                                const Uint16 maxvalue = (Uint16)(Uint16)(plut->getAbsMaxRange() - 1);
                                for (i = Count; i != 0; i--)
                                {
                                    value = (Uint32)(((double)(*(p++)) - absmin) * gradient1);
                                    *(q++) = (T3)dlut->getValue((Uint16)(maxvalue - plut->getValue(value)));
                                }
                            } else {                                                  // normal
                                for (i = Count; i != 0; i--)
                                {
                                    value = (Uint32)(((double)(*(p++)) - absmin) * gradient1);
                                    *(q++) = (T3)dlut->getValue(plut->getValue(value));
                                }
                            }
                        } else {                                                      // don't use display: invalid or absent
                            for (i = Count; i != 0; i--)
                            {
                                value = (Uint32)(((double)(*(p++)) - absmin) * gradient1);
                                *(q++) = (T3)((double)low + (double)plut->getValue(value) * gradient2);
                            }
                        }
                    }
                }
                else
                {                                                              // has no presentation LUT
                    createDisplayLUT(dlut, disp, inter->getBits());
                    register const double gradient = outrange / (inter->getAbsMaxRange());
                    if (initOptimizationLUT(lut, ocnt))
                    {                                                                 // use LUT for optimization
                        q = lut;
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            if (low > high)                                           // inverse
                            {
                                for (i = ocnt; i != 0; i--)                           // calculating LUT entries
                                    *(q++) = (T3)dlut->getValue((Uint16)(i - 1));
                            } else {                                                  // normal
                                for (i = 0; i < ocnt; i++)                            // calculating LUT entries
                                    *(q++) = (T3)dlut->getValue((Uint16)i);
                            }
                        } else {                                                      // don't use display: invalid or absent
                            for (i = 0; i < ocnt; i++)                                // calculating LUT entries
                                *(q++) = (T3)((double)low + (double)i * gradient);
                        }
                        const T3 *lut0 = lut - (T2)inter->getAbsMinimum();            // points to 'zero' entry
                        q = Data;
                        for (i = Count; i != 0; i--)                                  // apply LUT
                            *(q++) = *(lut0 + (*(p++)));
                    }
                    if (lut == NULL)                                                  // use "normal" transformation
                    {
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            if (low > high)                                           // inverse
                            {
                                for (i = Count; i != 0; i--)
                                    *(q++) = (T3)dlut->getValue((Uint16)(absmax - ((double)*(p++) - absmin)));
                            } else {                                                  // normal
                                for (i = Count; i != 0; i--)
                                    *(q++) = (T3)dlut->getValue((Uint16)((double)*(p++) - absmin));
                            }
                        } else {                                                      // don't use display: invalid or absent
                            for (i = Count; i != 0; i--)
                                *(q++) = (T3)((double)low + ((double)(*(p++)) - absmin) * gradient);
                        }
                    }
                }
                delete[] lut;
                if (Count < FrameSize)
                    OFBitmanipTemplate<T3>::zeroMem(Data + Count, FrameSize - Count); // set remaining pixels of frame to zero
            }
        } else
            Data = NULL;
    }


    /** apply the currently active VOI window to the output data
     *
     ** @param  inter   pointer to intermediate pixel representation
     *  @param  start   offset of the first pixel to be processed
     *  @param  plut    presentation LUT (optional, maybe NULL)
     *  @param  disp    display function (optional, maybe NULL)
     *  @param  center  window center
     *  @param  width   window width (>= 1)
     *  @param  low     lowest pixel value for the output data (e.g. 0)
     *  @param  high    highest pixel value for the output data (e.g. 255)
     */
    void window(const DiMonoPixel *inter,
                const Uint32 start,
                const DiLookupTable *plut,
                DiDisplayFunction *disp,
                const double center,
                const double width,
                const T3 low,
                const T3 high)
    {
        const T1 *pixel = (const T1 *)inter->getData();
        if (pixel != NULL)
        {
            if (Data == NULL)
                Data = new T3[FrameSize];                                             // create new output buffer
            if (Data != NULL)
            {
#ifdef DEBUG
                if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                {
                    ofConsole.lockCerr() << "INFO: using VOI routine 'window()'" << endl;
                    ofConsole.unlockCerr();
                }
#endif
                const DiDisplayLUT *dlut = NULL;
                const double absmin = inter->getAbsMinimum();
                const double width_1 = width - 1;
                const double leftBorder = center - 0.5 - width_1 / 2;                 // window borders, according to supplement 33
                const double rightBorder = center - 0.5 + width_1 / 2;
                const double outrange = (double)high - (double)low;                   // output range
                const unsigned long ocnt = (unsigned long)inter->getAbsMaxRange();    // number of LUT entries
                register const T1 *p = pixel + start;
                register T3 *q = Data;
                register unsigned long i;
                register double value;
                T3 *lut = NULL;
                if ((plut != NULL) && (plut->isValid()))                              // has presentation LUT
                {
#ifdef DEBUG
                    if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Informationals))
                    {
                        ofConsole.lockCerr() << "INFO: using presentation LUT transformation" << endl;
                        ofConsole.unlockCerr();
                    }
#endif
                    createDisplayLUT(dlut, disp, plut->getBits());
                    register Uint32 value2;                                           // presentation LUT is always unsigned
                    const Uint32 pcnt = plut->getCount();
                    const double plutmax_1 = (double)plut->getAbsMaxRange() - 1;
                    const double gradient1 = (width_1 == 0) ? 0 : (double)(pcnt - 1) / width_1;
                    if (initOptimizationLUT(lut, ocnt))
                    {                                                                 // use LUT for optimization
                        q = lut;
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            const double maxvalue = (double)(dlut->getCount() - 1);
                            const double offset = (low > high) ? maxvalue : 0;
                            const double gradient2 = (low > high) ? (-maxvalue / plutmax_1) : (maxvalue / plutmax_1);
                            for (i = 0; i < ocnt; i++)
                            {
                                value = (double)i + absmin;                           // pixel value
                                if (value <= leftBorder)
                                    value2 = 0;                                       // first LUT index
                                else if (value > rightBorder)
                                    value2 = pcnt - 1;                                // last LUT index
                                else
                                    value2 = (Uint32)((value - leftBorder) * gradient1);
                                *(q++) = (T3)dlut->getValue((Uint16)(offset + (double)plut->getValue(value2) * gradient2));
                            }
                        } else {                                                      // don't use display: invalid or absent
                            const double gradient2 = outrange / plutmax_1;
                            for (i = 0; i < ocnt; i++)
                            {
                                value = (double)i + absmin;                           // pixel value
                                if (value <= leftBorder)
                                    value2 = 0;                                       // first LUT index
                                else if (value > rightBorder)
                                    value2 = pcnt - 1;                                // last LUT index
                                else
                                    value2 = (Uint32)((value - leftBorder) * gradient1);
                                *(q++) = (T3)((double)low + (double)plut->getValue(value2) * gradient2);
                            }
                        }
                        const T3 *lut0 = lut - (T2)absmin;                            // points to 'zero' entry
                        q = Data;
                        for (i = Count; i != 0; i--)                                  // apply LUT
                            *(q++) = *(lut0 + (*(p++)));
                    }
                    if (lut == NULL)                                                  // use "normal" transformation
                    {
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            const double maxvalue = (double)(dlut->getCount() - 1);
                            const double offset = (low > high) ? maxvalue : 0;
                            const double gradient2 = (low > high) ? (-maxvalue / plutmax_1) : (maxvalue / plutmax_1);
                            for (i = Count; i != 0; i--)
                            {
                                value = (double)*(p++);                               // pixel value
                                if (value <= leftBorder)
                                    value2 = 0;                                       // first LUT index
                                else if (value > rightBorder)
                                    value2 = pcnt - 1;                                // last LUT index
                                else
                                    value2 = (Uint32)((value - leftBorder) * gradient1);
                                *(q++) = (T3)dlut->getValue((Uint16)(offset + (double)plut->getValue(value2) * gradient2));
                            }
                        } else {                                                      // don't use display: invalid or absent
                            const double gradient2 = outrange / plutmax_1;
                            for (i = Count; i != 0; i--)
                            {
                                value = (double)*(p++);                               // pixel value
                                if (value <= leftBorder)
                                    value2 = 0;                                       // first LUT index
                                else if (value > rightBorder)
                                    value2 = pcnt - 1;                                // last LUT index
                                else
                                    value2 = (Uint32)((value - leftBorder) * gradient1);
                                *(q++) = (T3)((double)low + (double)plut->getValue(value2) * gradient2);
                            }
                        }
                    }
                } else {                                                              // has no presentation LUT
                    createDisplayLUT(dlut, disp, bitsof(T1));
                    if (initOptimizationLUT(lut, ocnt))
                    {                                                                 // use LUT for optimization
                        q = lut;
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            const double maxvalue = (double)(dlut->getCount() - 1);
                            const double offset = (low > high) ? maxvalue : 0;
                            const double gradient = (width_1 == 0) ? 0 : ((low > high) ? (-maxvalue / width_1) : (maxvalue / width_1));
                            for (i = 0; i < ocnt; i++)                                // calculating LUT entries
                            {
                                value = (double)i + absmin - leftBorder;
                                if (value < 0)                                               // left border
                                    value = 0;
                                else if (value > width_1)                                    // right border
                                    value = width_1;
                                *(q++) = (T3)dlut->getValue((Uint16)(offset + value * gradient));   // calculate value
                            }
                        } else {                                                       // don't use display: invalid or absent
                            const double offset = (width_1 == 0) ? 0 : (high - ((center - 0.5) / width_1 + 0.5) * outrange);
                            const double gradient = (width_1 == 0) ? 0 : outrange / width_1;
                            for (i = 0; i < ocnt; i++)                                 // calculating LUT entries
                            {
                                value = (double)i + absmin;
                                if (value <= leftBorder)
                                    *(q++) = low;                                            // black/white
                                else if (value > rightBorder)
                                    *(q++) = high;                                           // white/black
                                else
                                    *(q++) = (T3)(offset + value * gradient);                // gray value
                            }
                        }
                        const T3 *lut0 = lut - (T2)absmin;                            // points to 'zero' entry
                        q = Data;
                        for (i = Count; i != 0; i--)                                  // apply LUT
                            *(q++) = *(lut0 + (*(p++)));
                    }
                    if (lut == NULL)                                                  // use "normal" transformation
                    {
                        if (dlut != NULL)                                             // perform display transformation
                        {
                            const double maxvalue = (double)(dlut->getCount() - 1);
                            const double offset = (low > high) ? maxvalue : 0;
                            const double gradient = (width_1 == 0) ? 0 : ((low > high) ? (-maxvalue / width_1) : (maxvalue / width_1));
                            for (i = Count; i != 0; i--)                              // calculating LUT entries
                            {
                                value = (double)*(p++) - leftBorder;
                                if (value < 0)                                               // left border
                                    value = 0;
                                else if (value > width_1)                                    // right border
                                    value = width_1;
                                *(q++) = (T3)dlut->getValue((Uint16)(offset + value * gradient));   // calculate value
                            }
                        } else {                                                      // don't use display: invalid or absent
                            const double offset = (width_1 == 0) ? 0 : (high - ((center - 0.5) / width_1 + 0.5) * outrange);
                            const double gradient = (width_1 == 0) ? 0 : outrange / width_1;
                            for (i = Count; i != 0; i--)
                            {
                                value = (double)*(p++);
                                if (value <= leftBorder)
                                    *(q++) = low;                                            // black/white
                                else if (value > rightBorder)
                                    *(q++) = high;                                           // white/black
                                else
                                    *(q++) = (T3)(offset + value * gradient);                // gray value
                            }
                        }
                    }
                }
                delete[] lut;
                if (Count < FrameSize)
                    OFBitmanipTemplate<T3>::zeroMem(Data + Count, FrameSize - Count);        // set remaining pixels of frame to zero
            }
        } else
            Data = NULL;
    }


    /** apply the currently active overlay planes to the output data
     *
     ** @param  overlays  array of overlay management objects
     *  @param  disp      display function (optional, maybe NULL)
     *  @param  columns   image's width (in pixels)
     *  @param  frame     number of frame to be rendered
     */
    void overlay(DiOverlay *overlays[2],
                 DiDisplayFunction *disp,
                 const Uint16 columns,
                 const Uint16 rows,
                 const unsigned long frame)
    {
        if ((Data != NULL) && (overlays != NULL))
        {
            for (unsigned int j = 0; j < 2; j++)
            {
                if (overlays[j] != NULL)
                {
                    const signed long left_pos = overlays[j]->getLeft();
                    const signed long top_pos = overlays[j]->getTop();
                    register DiOverlayPlane *plane;
                    for (unsigned int i = 0; i < overlays[j]->getCount(); i++)
                    {
                        plane = overlays[j]->getPlane(i);
                        if ((plane != NULL) && plane->isVisible() && plane->reset(frame))
                        {
                            register T3 *q;
                            register Uint16 x;
                            register Uint16 y;
                            const Uint16 xmin = (plane->getLeft(left_pos) > 0) ? plane->getLeft(left_pos) : 0;
                            const Uint16 ymin = (plane->getTop(top_pos) > 0) ? plane->getTop(top_pos) : 0;
                            const Uint16 xmax = (plane->getRight(left_pos) < columns) ? plane->getRight(left_pos) : columns;
                            const Uint16 ymax = (plane->getBottom(top_pos) < rows) ? plane->getBottom(top_pos) : rows;
                            const T3 maxvalue = (T3)DicomImageClass::maxval(bitsof(T3));
                            switch (plane->getMode())
                            {
                                case EMO_Replace:
                                {
                                    const T3 fore = (T3)(plane->getForeground() * maxvalue);
                                    for (y = ymin; y < ymax; y++)
                                    {
                                        plane->setStart((Uint16)(left_pos + xmin), (Uint16)(top_pos + y));
                                        q = Data + (unsigned long)y * (unsigned long)columns + (unsigned long)xmin;
                                        for (x = xmin; x < xmax; x++, q++)
                                        {
                                            if (plane->getNextBit())
                                                *q = fore;
                                        }
                                    }
                                    break;
                                }
                                case EMO_ThresholdReplace:
                                {
                                    const T3 fore = (T3)(plane->getForeground() * maxvalue);
                                    const T3 thresh = (T3)(plane->getThreshold() * maxvalue);
                                    for (y = ymin; y < ymax; y++)
                                    {
                                        plane->setStart((Uint16)(left_pos + xmin), (Uint16)(top_pos + y));
                                        q = Data + (unsigned long)y * (unsigned long)columns + (unsigned long)xmin;
                                        for (x = xmin; x < xmax; x++, q++)
                                        {
                                            if (plane->getNextBit())
                                                *q = (*q <= thresh) ? fore : 1;
                                        }
                                    }
                                    break;
                                }
                                case EMO_Complement:
                                {
                                    const T3 thresh = (T3)DicomImageClass::maxval(bitsof(T3) / 2);
                                    for (y = ymin; y < ymax; y++)
                                    {
                                        plane->setStart((Uint16)(left_pos + xmin), (Uint16)(top_pos + y));
                                        q = Data + (unsigned long)y * (unsigned long)columns + (unsigned long)xmin;
                                        for (x = xmin; x < xmax; x++, q++)
                                        {
                                            if (plane->getNextBit())
                                                *q = (*q <= thresh) ? maxvalue : 0;
                                        }
                                    }
                                    break;
                                }
                                case EMO_RegionOfInterest:
                                {
                                    const int dim = bitsof(T3) / 2;
                                    for (y = ymin; y < ymax; y++)
                                    {
                                        plane->setStart((Uint16)(left_pos + xmin), (Uint16)(top_pos + y));
                                        q = Data + (unsigned long)y * (unsigned long)columns + (unsigned long)xmin;
                                        for (x = xmin; x < xmax; x++, q++)
                                        {
                                            if (!plane->getNextBit())
                                                *q = *q >> dim;
                                        }
                                    }
                                    break;
                                }
                                case EMO_BitmapShutter:
                                {
                                    register T3 fore = (T3)((double)maxvalue * (double)plane->getPValue() / (double)DicomImageClass::maxval(WIDTH_OF_PVALUES));
                                    if ((disp != NULL) && (disp->isValid()))
                                    {
                                        const DiDisplayLUT *dlut = disp->getLookupTable(WIDTH_OF_PVALUES);
                                        if ((dlut != NULL) && (dlut->isValid()))
                                            fore = (T3)dlut->getValue(plane->getPValue());
                                    }
                                    for (y = ymin; y < ymax; y++)
                                    {
                                        plane->setStart((Uint16)(left_pos + xmin), (Uint16)(top_pos + y));
                                        q = Data + (unsigned long)y * (unsigned long)columns + (unsigned long)xmin;
                                        for (x = xmin; x < xmax; x++, q++)
                                        {
                                            if (plane->getNextBit())
                                                *q = fore;
                                        }
                                    }
                                    break;
                                }
                                default: /* e.g. EMO_Default */
                                    if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Warnings))
                                    {
                                        ofConsole.lockCerr() << "WARNING: unhandled overlay mode (" << (int)plane->getMode() << ") !" << endl;
                                        ofConsole.unlockCerr();
                                    }
                            }
                        }
                    }
                }
            }
        }
    }


    /// pointer to the storage area where the output data should be stored
    T3 *Data;
    /// flag indicating whether the output data buffer should be deleted in the destructor
    int DeleteData;

#ifdef PASTEL_COLOR_OUTPUT
    DiMonoColorOutputPixelTemplate<T1, T3> *ColorData;
#else
    DiMonoOutputPixel *ColorData;
#endif

 // --- declarations to avoid compiler warnings

    DiMonoOutputPixelTemplate(const DiMonoOutputPixelTemplate<T1,T2,T3> &);
    DiMonoOutputPixelTemplate<T1,T2,T3> &operator=(const DiMonoOutputPixelTemplate<T1,T2,T3> &);
};


#endif
