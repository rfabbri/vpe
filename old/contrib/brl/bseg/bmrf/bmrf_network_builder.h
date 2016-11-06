// This is brl/bseg/bmrf/bmrf_network_builder.h
#ifndef bmrf_network_builder_h_
#define bmrf_network_builder_h_
//:
// \file
// \brief A builder class that generates the MRF network
// \author J. L. Mundy
// \date January 18, 2004
//
// The builder generates bmrf_epi_seg(s) from a series of frames and adds
// them to the network.

#include <vil/vil_image_view.h>
#include <vcl_vector.h>
#include <vcl_set.h>
#include <vcl_map.h>
#include <vnl/vnl_double_3.h>
#include <vgl/vgl_point_2d.h>
#include <vdgl/vdgl_digital_curve_sptr.h>
#include <vtol/vtol_edge_2d_sptr.h>
#include <bmrf/bmrf_network_builder_params.h>
#include <bmrf/bmrf_epi_point_sptr.h>
#include <bmrf/bmrf_epi_seg_sptr.h>
#include <bmrf/bmrf_node_sptr.h>
#include <bmrf/bmrf_network_sptr.h>


class bmrf_network_builder : public bmrf_network_builder_params
{
 public:
  //Constructors/destructor
  bmrf_network_builder(bmrf_network_builder_params& tp);

  ~bmrf_network_builder();

  //Accessors

  //: get the completed network
  bmrf_network_sptr network();

  //Mutators

  //:initialize the builder
  void init();

  //: specify the image for the current frame
  void set_image(vil_image_view<float> const& image);

  //: set the edge segmentation for the current frame
  void set_edges(int frame, vcl_vector<vtol_edge_2d_sptr> const & edges);

  //Utility Methods

  //:convert image coordinates to epipolar coordinates
  void epi_coords(const double u, const double v,
                  double& alpha, double& s) const;

  //:convert epipolar coordinates to image coordinates
  // \note if \p u or \p v are outside the image return false.
  bool image_coords(const double a, const double s,
                    double& u, double& v) const;


  //: the main process method
  bool build();


 protected:
  float scan_interval(float xs, float ys, float xe, float ye, int& npix);
  bool scan_intensity_info(bmrf_epi_point_sptr const &pm1,
                           bmrf_epi_point_sptr p,
                           bmrf_epi_point_sptr const &pp1);
  bool inside_epipolar_wedge(vdgl_digital_curve_sptr const& dc);
  bool extract_alpha_segments(vdgl_digital_curve_sptr const & dc,
                              vcl_vector<bmrf_epi_seg_sptr>& epi_segs);
  bool compute_segments();
  bool intensity_candidates(const bmrf_epi_seg_sptr& seg,
                            vcl_set<bmrf_epi_seg_sptr>& left_cand,
                            vcl_set<bmrf_epi_seg_sptr>& right_cand) const;
  double radius(const double s) const;
  double find_left_s(const double a, const double s,
                     vcl_set<bmrf_epi_seg_sptr> const& cand) const;
  double find_right_s(const double a, const double s,
                      vcl_set<bmrf_epi_seg_sptr> const& cand) const;
  double ds(const double s) const;

  double scan_interval(const double a, const double sl, const double s) const;
  double scan_left(double a, double s,
                   vcl_set<bmrf_epi_seg_sptr> const& left_cand,
                   double& ds) const;
  double scan_right(double a,double s,
                    vcl_set<bmrf_epi_seg_sptr> const& right_cand,
                    double& ds) const;

  bool fill_intensity_values(bmrf_epi_seg_sptr& seg);
  bool set_intensity_info();
  bool add_frame_nodes();
  bool time_neighbors(bmrf_node_sptr const& node,
                      vcl_set<bmrf_node_sptr>& neighbors) const;
  bool assign_neighbors();

  //members
  //: network building status flags
  bool network_valid_;

  //:the current frame index
  int frame_;

  //:the increment in image column dimension
  double du_;

  //:the increment in epipolar angle
  double da_;

  //:the minimum alpha bound
  double alpha_min_;

  //:the alpha scale factor to map alpha onto [0, 1]
  double alpha_inv_;

  //:the maximum distance from the epipole in the image
  double smax_;

  //:the epipole
  vgl_point_2d<double> epi_;

  //:the homogeneous coordinates of the upper epipolar wedge limit
  vnl_double_3 upper_wedge_line_;

  //:the homogeneous coordinates of the lower epipolar wedge limit
  vnl_double_3 lower_wedge_line_;

  //:the image for the current frame
  vil_image_view<float> image_;

  //:the edges for current frame
  vcl_vector<vtol_edge_2d_sptr> edges_;

  //:the network under construction
  bmrf_network_sptr network_;

  //:temporary arrays for building the intensity information
  vcl_vector<bmrf_epi_seg_sptr> epi_segs_;

  //:map s ranges to nodes
  vcl_multimap<double,bmrf_node_sptr> s_node_map_;

  //:map s ranges to nodes for the previous frame
  vcl_multimap<double,bmrf_node_sptr> prev_s_node_map_;
};

#endif // bmrf_network_builder_h_
