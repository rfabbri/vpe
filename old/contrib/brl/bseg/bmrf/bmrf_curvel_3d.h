// This is brl/bseg/bmrf/bmrf_curvel_3d.h
#ifndef bmrf_curvel_3d_h_
#define bmrf_curvel_3d_h_
//:
// \file
// \brief An uncertain 3D curve element
// \author Matt Leotta, (mleotta@lems.brown.edu)
// \date March 23, 2004

#include <bmrf/bmrf_node_sptr.h>
#include <bbas/bugl/bugl_gaussian_point_3d.h>
#include <vgl/vgl_point_3d.h>
#include <vnl/vnl_double_3x3.h>
#include <vnl/vnl_double_2.h>
#include <vbl/vbl_ref_count.h>
#include <vsl/vsl_binary_io.h>
#include <vcl_vector.h>
#include <vcl_map.h>
#include <vcl_utility.h>
#include <vcl_iosfwd.h> // for std::ostream

#include "bmrf_curvel_3d_sptr.h"

//: An uncertain 3D curve element
class bmrf_curvel_3d : public bugl_gaussian_point_3d<double>, public vbl_ref_count
{
 public:
  //: Constructor
  bmrf_curvel_3d();
  //: Copy constructor
  bmrf_curvel_3d(bmrf_curvel_3d const& c);
  //: Constructor
  bmrf_curvel_3d(vgl_point_3d<double> &p, vnl_double_3x3 &s);
  //: Constructor
  bmrf_curvel_3d(double x, double y, double z, vnl_double_3x3 & s);

  //: Destructor
  ~bmrf_curvel_3d() {}

  //: Merge the other curvel into this curvel if there is no overlap
  // \return false if merging is not possible
  bool merge(const bmrf_curvel_3d_sptr& other);

  //: Set the projection of this curvel into \p frame as the segment in
  //  \p node at the value \p alpha
  void set_proj_in_frame(unsigned int frame, double alpha, const bmrf_node_sptr& node);

  //: Set the projection of this curvel into \p frame at an interpolated position
  void set_psuedo_point(unsigned int frame, const vnl_double_2& pos);

  //: Returns the 2d position of this curvel in \p frame by reference
  // \retval true if a correspondence exists at this frame
  // \retval false if no correspondence exists at this frame
  bool pos_in_frame(unsigned int frame, vnl_double_2& pos) const;

  //: Returns the invariant cross ratio
  //  computed with the epipole and points from frame-1 to frame+1
  // \retval true if all required points exist (ratio returned by reference)
  // \retval false if one or more required points do not exist
  bool cross_ratio(unsigned int frame, double& ratio) const;

  //: Returns the smart pointer to the node at the projection into \p frame
  bmrf_node_sptr node_at_frame(unsigned int frame) const;

  //: Returns the alpha value at the projection into \p frame
  double alpha_at_frame(unsigned int frame) const;

  //: Return true if a projection of this curvel lies on \p node
  bool is_projection(const bmrf_node_sptr& node) const;

  //: Return the number of projections available
  int num_projections(bool include_pseudo = false) const;

  //: Return the projection error
  double proj_error() const { return proj_error_; }

  //: Return the average gamma value relative to frame 0
  double gamma_avg();

  //: Return the standard deviation of the gamma values
  double gamma_std();

  //: Return the average s value projected into \p frame
  double s_avg(unsigned int frame);

  //: Set the projection error
  void set_proj_error(double error) { proj_error_ = error; }

  void show_stats() const;

  //: Binary save self to stream.
  void b_write(vsl_b_ostream &os) const;

  //: Binary load self from stream.
  void b_read(vsl_b_istream &is);

  //: Return IO version number;
  short version() const;

  //: Print an ascii summary to the stream
  void print_summary(vcl_ostream &os) const;

 protected:
  //: Compute the gamma statistics on the current projections
  void compute_statistics();

  //: A vector of alpha/node pairs which represent the projection of this curvel into image i.
  vcl_vector<vcl_pair<double,bmrf_node_sptr> > projs_2d_;

  //: A map of projections that have been interpolated and do not belong to a node
  vcl_map<unsigned int, vnl_double_2> pseudo_points_;

  //: The sum of gamma values of all projections relative to the frame 0
  double sum_gamma_;

  //: The sum of the squared gamma values relative to frame 0;
  double sum_sqr_gamma_;

  //: Indicates that the statistics are valid
  bool stats_valid_;

  //: The number of valid projections
  unsigned int num_projections_;

  //: The error in the projection;
  double proj_error_;
};


//: Binary save bmrf_curvel_3d* to stream.
void vsl_b_write(vsl_b_ostream &os, const bmrf_curvel_3d* n);

//: Binary load bmrf_curvel_3d* from stream.
void vsl_b_read(vsl_b_istream &is, bmrf_curvel_3d* &n);

//: Print an ASCII summary to the stream
void vsl_print_summary(vcl_ostream &os, const bmrf_curvel_3d* n);

#endif // bmrf_curvel_3d_h_
