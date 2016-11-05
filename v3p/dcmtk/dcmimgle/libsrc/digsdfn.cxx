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
 *  Purpose: DicomGSDFunction (Source)
 *
 */


#include "osconfig.h"

#include "ofconsol.h"
#include "digsdfn.h"
#include "displint.h"

#include "ofstream.h"

#define INCLUDE_CMATH
#include "ofstdinc.h"


/*----------------------------*
 *  constant initializations  *
 *----------------------------*/

const unsigned int DiGSDFunction::GSDFCount = 1023;


/*----------------*
 *  constructors  *
 *----------------*/

DiGSDFunction::DiGSDFunction(const char *filename,
                             const E_DeviceType deviceType,
                             const signed int ord)
  : DiDisplayFunction(filename, deviceType, ord),
    JNDMin(0),
    JNDMax(0),
    GSDFValue(NULL),
    GSDFSpline(NULL)
{
    if (Valid)
        Valid = calculateGSDF() && calculateGSDFSpline() && calculateJNDBoundaries();
    if (!Valid)
    {
        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Errors))
        {
            ofConsole.lockCerr() << "ERROR: invalid DISPLAY file ... ignoring !" << endl;
            ofConsole.unlockCerr();
        }
    }
}


DiGSDFunction::DiGSDFunction(const double *val_tab,             // UNTESTED !!
                             const unsigned long count,
                             const Uint16 max,
                             const E_DeviceType deviceType,
                             const signed int ord)
  : DiDisplayFunction(val_tab, count, max, deviceType, ord),
    JNDMin(0),
    JNDMax(0),
    GSDFValue(NULL),
    GSDFSpline(NULL)
{
    if (Valid)
        Valid = calculateGSDF() && calculateGSDFSpline() && calculateJNDBoundaries();
    if (!Valid)
    {
        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Errors))
        {
            ofConsole.lockCerr() << "ERROR: invalid DISPLAY values ... ignoring !" << endl;
            ofConsole.unlockCerr();
        }
    }
}


DiGSDFunction::DiGSDFunction(const Uint16 *ddl_tab,             // UNTESTED !!
                             const double *val_tab,
                             const unsigned long count,
                             const Uint16 max,
                             const E_DeviceType deviceType,
                             const signed int ord)
  : DiDisplayFunction(ddl_tab, val_tab, count, max, deviceType, ord),
    JNDMin(0),
    JNDMax(0),
    GSDFValue(NULL),
    GSDFSpline(NULL)
{
    if (Valid)
        Valid = calculateGSDF() && calculateGSDFSpline() && calculateJNDBoundaries();
    if (!Valid)
    {
        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Errors))
        {
            ofConsole.lockCerr() << "ERROR: invalid DISPLAY values ... ignoring !" << endl;
            ofConsole.unlockCerr();
        }
    }
}


DiGSDFunction::DiGSDFunction(const double val_min,
                             const double val_max,
                             const unsigned long count,
                             const E_DeviceType deviceType,
                             const signed int ord)
  : DiDisplayFunction(val_min, val_max, count, deviceType, ord),
    JNDMin(0),
    JNDMax(0),
    GSDFValue(NULL),
    GSDFSpline(NULL)
{
    if (Valid)
        Valid = calculateGSDF() && calculateGSDFSpline() && calculateJNDBoundaries();
    if (!Valid)
    {
        if (DicomImageClass::checkDebugLevel(DicomImageClass::DL_Errors))
        {
            ofConsole.lockCerr() << "ERROR: invalid DISPLAY values ... ignoring !" << endl;
            ofConsole.unlockCerr();
        }
    }
}


/*--------------*
 *  destructor  *
 *--------------*/

DiGSDFunction::~DiGSDFunction()
{
    delete[] GSDFValue;
    delete[] GSDFSpline;
}


/********************************************************************/


DiDisplayLUT *DiGSDFunction::getDisplayLUT(unsigned long count)
{
    DiDisplayLUT *lut = NULL;
    if (count <= MAX_TABLE_ENTRY_COUNT)
    {
        if ((DeviceType == EDT_Printer) || (DeviceType == EDT_Scanner))
        {
            /* hardcopy: values are in optical density, first convert them to luminance */
            double *tmp_tab = convertODtoLumTable(LODValue, ValueCount);
            if (tmp_tab != NULL)
            {
                /* create new GSDF LUT */
                lut = new DiGSDFLUT(count, MaxDDLValue, DDLValue, tmp_tab, ValueCount, GSDFValue, GSDFSpline,
                    GSDFCount, JNDMin, JNDMax, AmbientLight, Illumination, (DeviceType == EDT_Scanner));
                /* delete temporary table */
                delete[] tmp_tab;
            }
        } else {
            /* softcopy: values are already in luminance */
            lut = new DiGSDFLUT(count, MaxDDLValue, DDLValue, LODValue, ValueCount, GSDFValue, GSDFSpline,
                GSDFCount, JNDMin, JNDMax, AmbientLight, Illumination, (DeviceType == EDT_Camera));
        }
    }
    return lut;
}


int DiGSDFunction::writeCurveData(const char *filename,
                                  const OFBool mode)
{
    if ((filename != NULL) && (strlen(filename) > 0))
    {
        ofstream file(filename);
        if (file)
        {
            const OFBool inverseLUT = (DeviceType == EDT_Scanner) || (DeviceType == EDT_Camera);
            /* comment header */
            file << "# Display function       : GSDF (DICOM Part 14)" << endl;
            if (DeviceType == EDT_Printer)
                file << "# Type of output device  : Printer (hardcopy)" << endl;
            else if (DeviceType == EDT_Scanner)
                file << "# Type of output device  : Scanner (hardcopy)" << endl;
            else if (DeviceType == EDT_Camera)
                file << "# Type of output device  : Camera (softcopy)" << endl;
            else
                file << "# Type of output device  : Monitor (softcopy)" << endl;
            file << "# Device driving levels  : " << ValueCount << endl;
            if ((DeviceType == EDT_Printer) || (DeviceType == EDT_Scanner))
                file << "# Illumination  [cd/m^2] : " << Illumination << endl;
            file << "# Ambient light [cd/m^2] : " << AmbientLight << endl;
            if ((DeviceType == EDT_Printer) || (DeviceType == EDT_Scanner))
            {
                file << "# Luminance w/o [cd/m^2] : " << convertODtoLum(MaxValue, OFFalse /*useAmb*/) << " - "
                                                      << convertODtoLum(MinValue, OFFalse /*useAmb*/) << endl;
                file << "# Optical density   [OD] : " << MinValue << " - " << MaxValue << endl;
            } else
                file << "# Luminance w/o [cd/m^2] : " << MinValue << " - " << MaxValue << endl;
            file << "# Barten JND index range : " << JNDMin << " - " << JNDMax << " (" << (JNDMax - JNDMin) << ")" << endl;
            file << "# Interpolation method   : ";
            if (getPolynomialOrder() > 0)
                file << "Curve fitting algorithm with order " << getPolynomialOrder() << endl << endl;
            else
                file << "Cubic spline interpolation" << endl << endl;
            /* print headings of the table */
            if (mode)
            {
                file << "# NB: values for CC, GSDF and PSC";
                if (inverseLUT)
                    file << "'";            // add ' to PSC
                file << " are specified in cd/m^2" << endl << endl;
                file << "DDL\tCC\tGSDF\tPSC";
                if (inverseLUT)
                    file << "'";            // add ' to PSC
                file << endl;
            } else {
                file << "# NB: values for CC and GSDF are specified in cd/m^2" << endl << endl;
                file << "DDL\tGSDF" << endl;
            }
            /* create GSDF LUT and write curve data to file */
            DiGSDFLUT *lut = NULL;
            if ((DeviceType == EDT_Printer) || (DeviceType == EDT_Scanner))
            {
                /* hardcopy: values are in optical density, first convert them to luminance */
                double *tmp_tab = convertODtoLumTable(LODValue, ValueCount, OFFalse /*useAmb*/);
                if (tmp_tab != NULL)
                {
                    lut = new DiGSDFLUT(ValueCount, MaxDDLValue, DDLValue, tmp_tab, ValueCount, GSDFValue,
                        GSDFSpline, GSDFCount, JNDMin, JNDMax, AmbientLight, Illumination, inverseLUT,
                        &file, mode);
                    /* delete temporary table */
                    delete[] tmp_tab;
                }
            } else {
                /* softcopy: values are already in luminance */
                lut = new DiGSDFLUT(ValueCount, MaxDDLValue, DDLValue, LODValue, ValueCount, GSDFValue,
                    GSDFSpline, GSDFCount, JNDMin, JNDMax, AmbientLight, Illumination, inverseLUT,
                    &file, mode);
            }
            int status = (lut != NULL) && (lut->isValid());
            delete lut;
            return status;
        }
    }
    return 0;
}


int DiGSDFunction::setAmbientLightValue(const double value)
{
    int status = DiDisplayFunction::setAmbientLightValue(value);
    if (status)
        Valid = calculateJNDBoundaries();       // check validity
    return status;
}


int DiGSDFunction::setIlluminationValue(const double value)
{
    int status = DiDisplayFunction::setIlluminationValue(value);
    if (status && ((DeviceType == EDT_Printer) || (DeviceType == EDT_Scanner)))
        Valid = calculateJNDBoundaries();       // check validity
    return status;
}


/********************************************************************/


int DiGSDFunction::calculateGSDF()
{
    GSDFValue = new double[GSDFCount];
    if (GSDFValue != NULL)
    {
        /*
         *  algorithm taken from DICOM part 14: Grayscale Standard Display Function (GSDF)
         */
        const double a = -1.3011877;
        const double b = -2.5840191e-2;
        const double c =  8.0242636e-2;
        const double d = -1.0320229e-1;
        const double e =  1.3646699e-1;
        const double f =  2.8745620e-2;
        const double g = -2.5468404e-2;
        const double h = -3.1978977e-3;
        const double k =  1.2992634e-4;
        const double m =  1.3635334e-3;
        register unsigned int i;
        register double ln;
        register double ln2;
        register double ln3;
        register double ln4;
        for (i = 0; i < GSDFCount; i++)
        {
            ln = log((double)(i + 1));
            ln2 = ln * ln;
            ln3 = ln2 * ln;
            ln4 = ln3 * ln;
            GSDFValue[i] = pow(10.0, (a + c*ln + e*ln2 + g*ln3 + m*ln4) / (1 + b*ln + d*ln2 + f*ln3 + h*ln4 + k*(ln4*ln)));
        }
        return 1;
    }
    return 0;
}


int DiGSDFunction::calculateGSDFSpline()
{
    int status = 0;
    if ((GSDFValue != NULL) && (GSDFCount > 0))
    {
        GSDFSpline = new double[GSDFCount];
        unsigned int *jidx = new unsigned int[GSDFCount];
        if ((GSDFSpline != NULL) && (jidx != NULL))
        {
            register unsigned int i;
            register unsigned int *p = jidx;
            for (i = 1; i <= GSDFCount; i++)
                *(p++) = i;
            status = DiCubicSpline<unsigned int, double>::Function(jidx, GSDFValue, GSDFCount, GSDFSpline);
        }
        delete[] jidx;
    }
    return status;
}


int DiGSDFunction::calculateJNDBoundaries()
{
    if ((LODValue != NULL) && (ValueCount > 0))
    {
        if ((DeviceType == EDT_Printer) || (DeviceType == EDT_Scanner))
        {
            /* hardcopy device (printer/scanner), values are in OD */
            JNDMin = getJNDIndex(convertODtoLum(MaxValue));
            JNDMax = getJNDIndex(convertODtoLum(MinValue));
        } else {
            /* softcopy device (monitor/camera), values are in cd/m^2 */
            JNDMin = getJNDIndex(MinValue + AmbientLight);
            JNDMax = getJNDIndex(MaxValue + AmbientLight);
        }
        return (JNDMin >= 0) && (JNDMax >= 0);
    }
    return 0;
}


double DiGSDFunction::getJNDIndex(const double lum)
{
    if (lum > 0.0)
    {
        /*
         *  algorithm taken from DICOM part 14: Grayscale Standard Display Function (GSDF)
         */
        const double a = 71.498068;
        const double b = 94.593053;
        const double c = 41.912053;
        const double d =  9.8247004;
        const double e =  0.28175407;
        const double f = -1.1878455;
        const double g = -0.18014349;
        const double h =  0.14710899;
        const double i = -0.017046845;
        double lg10[8];
        lg10[0] = log10(lum);
        register unsigned int j;
        for (j = 0; j < 7; j++)                         // reduce number of multiplications
            lg10[j + 1] = lg10[j] * lg10[0];
        return a + b*lg10[0] + c*lg10[1] + d*lg10[2] + e*lg10[3] + f*lg10[4] + g*lg10[5] + h*lg10[6] + i*lg10[7];
    }
    return -1;
}
