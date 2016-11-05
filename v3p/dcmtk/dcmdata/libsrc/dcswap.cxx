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
 *  Purpose: byte order functions
 *
 */

#include "osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcswap.h"

OFCondition swapIfNecessary(const E_ByteOrder newByteOrder,
                            const E_ByteOrder oldByteOrder,
                            void * value, const Uint32 byteLength,
                            const size_t valWidth)
    /*
     * This function swaps byteLength bytes in value if newByteOrder and oldByteOrder
     * differ from each other. In case bytes have to be swapped, these bytes are seperated
     * in valWidth elements which will be swapped seperately.
     *
     * Parameters:
     *   newByteOrder - [in] The new byte ordering (little or big endian).
     *   oldByteOrder - [in] The current old byte ordering (little or big endian).
     *   value        - [in] Array that contains the actual bytes which might have to be swapped.
     *   byteLength   - [in] Length of the above array.
     *   valWidth     - [in] Specifies how many bytes shall be treated together as one element.
     */
{
    /* if the two byte orderings are unknown this is an illegal call */
    if (oldByteOrder != EBO_unknown && newByteOrder != EBO_unknown )
    {
        /* and if they differ from each other and valWidth is not 1 */
        if (oldByteOrder != newByteOrder && valWidth != 1)
        {
            /* in case the array length equals valWidth and only 2 or 4 bytes have to be swapped */
            /* we can swiftly swap these bytes by calling the corresponding functions. If this is */
            /* not the case we have to call a more sophisticated function. */
            if (byteLength == valWidth)
            {
                if (valWidth == 2)
                    swap2Bytes((Uint8 *)value);
                else if (valWidth == 4)
                    swap4Bytes((Uint8 *)value);
                else
                    swapBytes(value, byteLength, valWidth);
            }
            else
                swapBytes(value, byteLength, valWidth);
        }
        return EC_Normal;
    }
    return EC_IllegalCall;
}


void swapBytes(void * value, const Uint32 byteLength,
                           const size_t valWidth)
    /*
     * This function swaps byteLength bytes in value. These bytes are seperated
     * in valWidth elements which will be swapped seperately.
     *
     * Parameters:
     *   value        - [in] Array that contains the actual bytes which might have to be swapped.
     *   byteLength   - [in] Length of the above array.
     *   valWidth     - [in] Specifies how many bytes shall be treated together as one element.
     */
{
    /* use register (if available) to increase speed */
    register Uint8 save;

    /* in case valWidth equals 2, swap correspondingly */
    if (valWidth == 2)
    {
        register Uint8 * first = &((Uint8*)value)[0];
        register Uint8 * second = &((Uint8*)value)[1];
        register Uint32 times = byteLength/2;
        while (times--)
        {
            save = *first;
            *first = *second;
            *second = save;
            first +=2;
            second +=2;
        }
    }
    /* if valWidth is greater than 2, swap correspondingly */
    else if (valWidth > 2)
    {
        register size_t i;
        const size_t halfWidth = valWidth/2;
        const size_t offset = valWidth-1;
        register Uint8 *start;
        register Uint8 *end;

        Uint32 times = byteLength/valWidth;
        Uint8  *base = (Uint8 *)value;

        while (times--)
        {
            i = halfWidth;
            start = base;
            end = base+offset;
            while (i--)
            {
                save = *start;
                *start++ = *end;
                *end-- = save;
            }
            base += valWidth;
        }
    }
}


Uint16 swapShort(const Uint16 toSwap)
{
        Uint8 * swapped = (Uint8 *)&toSwap;
        swap2Bytes(swapped);
        return * ((Uint16*)swapped);
}
