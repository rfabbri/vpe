//-*- c++ -*-------------------------------------------------------------------
#ifndef ByteConvertImage_h_
#define ByteConvertImage_h_
#ifdef __GNUC__
#pragma interface
#endif
//
// Class : ByteConvertImage
//
// .SECTION Description
//    ByteConvertImage is a class that awf hasn't documented properly. FIXME
//
// .NAME        ByteConvertImage - Undocumented class FIXME
// .LIBRARY     oxp
// .HEADER	oxl Package
// .INCLUDE     oxp/ByteConvertImage.h
// .FILE        ByteConvertImage.h
// .FILE        ByteConvertImage.cxx
// .SECTION Author
//     Andrew W. Fitzgibbon, Oxford RRG, 22 Dec 98
//
//-----------------------------------------------------------------------------

#include <vcl/vcl_iostream.h>

#include <vil/vil_memory_image_of.h>
#include <vil/vil_byte.h>

struct ByteConvertImage : public vil_memory_image_of<vil_byte> {
  typedef vil_memory_image_of<vil_byte> base;

  ByteConvertImage(vil_memory_image_of<float> const& in, bool ignore_zero = false);
  ByteConvertImage(vil_memory_image_of<float> const& in, float min, float max);
  ByteConvertImage(vil_memory_image_of<double> const& in, bool ignore_zero = false);
  ByteConvertImage(vil_memory_image_of<double> const& in, double min, double max);

  void print(ostream&) const;
  
private:
  bool ignore_zero_;
  float min_;
  float max_;
  void filter(vil_memory_image_of<float> const&);
  void filter(vil_memory_image_of<double> const&);
};

#endif   // DO NOT ADD CODE AFTER THIS LINE! END OF DEFINITION FOR CLASS ByteConvertImage.

