#ifndef vgl_homg_line_2d_h
#define  vgl_homg_line_2d_h
#ifdef __GNUC__
#pragma interface
#endif

// This is vxl/vgl/vgl_homg_line_2d.h

//:
// \file
// \author Don Hamilton, Peter Tu
//

#include <vcl_iostream.h>
//#include <vcl_algorithm.h> // for vcl_min
#include <vcl_cmath.h> // for vcl_abs

template <class Type>
class vgl_homg_point_2d;

template <class Type>
class vgl_line_2d;

//: Represents a homogeneous 2D line.
template <class Type>
class vgl_homg_line_2d {

  // PUBLIC INTERFACE--------------------------------------------------------
  
public:
 
  // Constructors/Initializers/Destructors-----------------------------------

  //: Default constructor (leaves line undefined)
  vgl_homg_line_2d () {}
  
  // Default copy constructor  
  // vgl_homg_line_2d (const vgl_homg_line_2d<Type>& that) { *this = that; }

//unimp  vgl_homg_line_2d<Type> (vgl_line_2d<Type> const& p);

  //: Construct from three Types.
  vgl_homg_line_2d (Type const& a_, Type const& b_, Type const& c_) { set(a_,b_,c_); }

  //: Construct from 3-vector.
  vgl_homg_line_2d (const Type v[3]) { set(v[0],v[1],v[2]); }

  //: Construct from two points (join)
  vgl_homg_line_2d (vgl_homg_point_2d<Type> const& p1, vgl_homg_point_2d<Type> const& p2);

  // Default destructor
  // ~vgl_homg_line_2d () {}

  // Default assignment operator
  // vgl_homg_line_2d<Type>& operator=(const vgl_homg_line_2d<Type>& that){
  //   set(that.a(),that.b(),that.c());
  //   return *this;
  // }

  // Data Access-------------------------------------------------------------

  //vcl_vector<Type> get_direction() const;
  //vcl_vector<Type> get_normal() const;

  Type dirx() const { return a(); }  // TODO
  Type diry() const { return b(); }  // TODO
  Type nx() const { return -b(); } // TODO
  Type ny() const { return a(); } // TODO

  Type a() const {return pos_[0];}
  Type b() const {return pos_[1];}
  Type c() const {return pos_[2];}

  //: Set a b c.
  void set (Type const& a_, Type const& b_, Type const& c_){
    pos_[0] = a_;
    pos_[1] = b_;
    pos_[2] = c_;
  }

  //: Return true iff the point is the point at infinity
  //    This version checks (min(|a|,|b|) < tol * c
  //win32 fails  bool ideal(Type tol) const { return vcl_min(vcl_abs(a()),vcl_abs(b())) < tol * vcl_abs(c()); }
  
  //:get two points on the line
  // These two points are normally the intersections
  // with the Y axis and X axis, respectively.  When the line is parallel to one
  // of these, the point with y=1 or x=1, resp. are taken.  When the line goes
  // through the origin, the second point is (b, -a, 1).  Finally, when the line
  // is the line at infinity, the returned points are (1,0,0) and (0,1,0).
  // Thus, whenever possible, the returned points are not at infinity.
  void get_two_points(vgl_homg_point_2d<Type> &p1, vgl_homg_point_2d<Type> &p2);

  // INTERNALS---------------------------------------------------------------

protected:
  //: the data associated with this line
  Type pos_[3];
};

//: Print line equation to stream
template <class Type>
vcl_ostream&  operator<<(vcl_ostream& s, const vgl_homg_line_2d<Type>& p) {
  return s << " <vgl_homg_line_2d "
           << p.a() << " x + " << p.b() << " y + "
           << p.c() << " z = 0>";
}

//: Load in line parameters from stream
template <class Type>
vcl_istream&  operator>>(vcl_istream& is,  vgl_homg_line_2d<Type>& p) {
  Type a,b,c;
  is >> a >> b >> c;
  p.set(a,b,c);
  return is;
}

#endif //  vgl_homg_line_2d_h
