// This is brl/bseg/bmrf/bmrf_curve_3d_builder.cxx
#include "bmrf_curve_3d_builder.h"
//:
// \file

#include "bmrf_curve_3d.h"
#include "bmrf_curvel_3d.h"
#include "bmrf_network.h"
#include "bmrf_node.h"
#include "bmrf_arc.h"
#include "bmrf_epi_seg.h"
#include "bmrf_epipole.h"
#include "bmrf_gamma_func.h"

#include <vcl_algorithm.h>
#include <vgl/vgl_point_2d.h>
#include <vnl/vnl_double_2.h>
#include <vnl/vnl_double_3.h>
#include <vnl/vnl_double_4.h>
#include <vnl/vnl_double_3x4.h>
#include <vnl/vnl_double_3x3.h>
#include <vnl/vnl_rotation_matrix.h>
#include <vnl/vnl_inverse.h>
#include <vnl/vnl_identity_3x3.h>
#include <vnl/algo/vnl_svd.h>


//: Constructor
bmrf_curve_3d_builder::bmrf_curve_3d_builder()
 : network_(NULL), min_alpha_(0.0), max_alpha_(0.0), bb_xform_(0.0)
{
}


//: Constructor
bmrf_curve_3d_builder::bmrf_curve_3d_builder(const bmrf_network_sptr& network)
 : network_(network), min_alpha_(0.0), max_alpha_(0.0), bb_xform_(0.0)
{
  // For our camera images are 1024 x 768
  // The focal length is 12.5mm / (6.25 um/pixel) = 2000 pixels
  double data[] = { 2000, 0,    512,
                    0,    2000, 384,
                    0,    0,    1 };
  vnl_double_3x3 K(data);

  // Use the identity camera by default
  vnl_double_3x4 C; C.set_identity();
  this->init_cameras(K*C);
}


//: Initialize the camera matrices
void
bmrf_curve_3d_builder::init_cameras(const vnl_double_3x4& C0, double scale)
{
  C_.clear();
  if (!network_)
    return;
  vcl_set<int> frames = network_->frame_numbers();
  const vgl_point_2d<double>& ep = network_->epipole(1).location();
  if (offsets_.empty())
    compute_camera_offsets();

  vnl_double_3x3 M = C0.extract(3,3);

  // compute the 3d direction unit vector
  vnl_double_3 e;
  e[0] = ep.x();
  e[1] = ep.y();
  e[2] = 1.0;
  vnl_double_3 dir = vnl_inverse(M)*e;
  direction_.set(dir[0],dir[1],dir[2]);
  normalize(direction_);

  // compute the cameras
  vcl_set<int>::const_iterator fitr = frames.begin();
  if (fitr != frames.end()){
    C_[*fitr] = C0;
    double dt = scale;
    vnl_double_3x4 Ef = C0;
    for ( ++fitr;  fitr != frames.end();  ++fitr ) {
      dt *= offsets_[*fitr];
      Ef.set_column(3,Ef.get_column(3) - dt*e);
      C_[*fitr] = Ef;
    }
  }
}


//: Compute the relative change in spacing between cameras
void
bmrf_curve_3d_builder::compute_camera_offsets()
{
  offsets_.clear();
  vcl_set<int> frames = network_->frame_numbers();

  for ( vcl_set<int>::const_iterator fitr = frames.begin();
        fitr != frames.end();  ++fitr ) {
    double cross_ratio_sum = 0.0;
    double cr_moment2 = 0.0;
    unsigned int count = 0;
    for ( vcl_set<bmrf_curve_3d_sptr>::const_iterator itr1 = curves_.begin();
          itr1 != curves_.end();  ++itr1)
    {
      for ( bmrf_curve_3d::const_iterator itr2 = (*itr1)->begin();
            itr2 != (*itr1)->end();  ++itr2)
      {
        bmrf_curvel_3d_sptr curvel = *itr2;
        double val = 0.0;
        if (curvel->cross_ratio(*fitr,val)){
          cross_ratio_sum += val;
          cr_moment2 += val*val;
          ++count;
        }
      }
    }

    double offset = 1.0;
    if (fitr == frames.begin())
      offset = 0.0;
    if (count>0){
      double avg_ratio = cross_ratio_sum/count;
      double var = 0.0;
      if (count>1)
        var = (cr_moment2 - avg_ratio*avg_ratio*count)/(count-1);
      vcl_cerr << "Frame "<< *fitr << ":\tmean = "<< avg_ratio << " \tstdev = " << vcl_sqrt(var) <<vcl_endl;
      offset = avg_ratio/(1.0 - avg_ratio);
    }
    offsets_[*fitr] = offset;
  }

  vcl_cerr << "All offsets\n";
  for ( vcl_set<int>::const_iterator fitr = frames.begin();
        fitr != frames.end();  ++fitr ) {
    vcl_cerr << offsets_[*fitr] << ' ';
  }
  vcl_cerr << vcl_endl;
}


//: Set the network
void
bmrf_curve_3d_builder::set_network(const bmrf_network_sptr& network)
{
  network_ = network;
}


//: Return the constructed curves
vcl_set<bmrf_curve_3d_sptr>
bmrf_curve_3d_builder::curves() const
{
  return curves_;
}


//: Return the cameras used in the reconstruction
vcl_map<int,vnl_double_3x4>
bmrf_curve_3d_builder::cameras() const
{
  return C_;
}


//: Return the 3D direction of motion of the curves
vgl_vector_3d<double>
bmrf_curve_3d_builder::direction() const
{
  return direction_;
}


//: Return the bounding box transformation
vnl_double_4x4
bmrf_curve_3d_builder::bb_xform() const
{
  return bb_xform_;
}


bool bmrf_cmp_x(const vnl_double_3& rhs, const vnl_double_3& lhs)
{
  return rhs[0] < lhs[0];
}

bool bmrf_cmp_y(const vnl_double_3& rhs, const vnl_double_3& lhs)
{
  return rhs[1] < lhs[1];
}

bool bmrf_cmp_z(const vnl_double_3& rhs, const vnl_double_3& lhs)
{
  return rhs[2] < lhs[2];
}

//: Compute the bounding box aligned with vehicle direction
bool
bmrf_curve_3d_builder::compute_bounding_box(const float *inlier_fractions, bool align_ep)
{
  if (curves_.empty())
    return false;

  vnl_double_3x3 rot = vnl_identity_3x3();

  if ( align_ep )
  {
    vnl_double_3 base_axis(1.0, 0.0, 0.0);
    vnl_double_3 rot_axis(0.0, 1.0, 0.0);
    vnl_double_3 xz_proj(direction_.x(), 0.0, direction_.z());
    xz_proj.normalize();
    double ang = angle(xz_proj, base_axis);
    rot_axis *= ang;
    vnl_double_3x3 rot_y = vnl_rotation_matrix(rot_axis);

    rot_axis = vnl_double_3(0.0, 0.0, 1.0);
    vnl_double_3 xy_proj = rot_y * vnl_double_3(direction_.x(), direction_.y(), direction_.z());
    xy_proj.normalize();
    ang = angle(xy_proj, base_axis);
    rot_axis *= ang;
    vnl_double_3x3 rot_z = vnl_rotation_matrix(rot_axis);

    rot = rot_z*rot_y;
  }
  // rotate only about the z-axis
  else
  {
    vnl_double_3 x_axis(1.0, 0.0, 0.0);
    vnl_double_3 xy_proj(direction_.x(), direction_.y(), 0.0);
    xy_proj.normalize();
    double ang = angle(xy_proj, -x_axis);
    vcl_cerr << "bbox rotation angle: " <<ang << vcl_endl;
    vnl_double_3 rot_axis(0.0, 0.0, ang);
    rot = vnl_rotation_matrix(rot_axis);
  }

  vcl_vector<vnl_double_3> pts_z;

  for ( vcl_set<bmrf_curve_3d_sptr>::const_iterator itr1 = curves_.begin();
        itr1 != curves_.end();  ++itr1)
  {
    for ( bmrf_curve_3d::const_iterator itr2 = (*itr1)->begin();
          itr2 != (*itr1)->end();  ++itr2)
    {
      bmrf_curvel_3d_sptr curvel = *itr2;
      vnl_double_3 point(curvel->x(), curvel->y(), curvel->z());
      pts_z.push_back(rot*point);
    }
  }

  vcl_sort(pts_z.begin(), pts_z.end(), bmrf_cmp_z);
  vcl_vector<vnl_double_3> pts_x;

  // only consider points in x and y above 0.5 feet
  for ( vcl_vector<vnl_double_3>::const_iterator itr = pts_z.begin();
        itr != pts_z.end();  ++itr)
  {
    if ((*itr)[2] > 0.5) { pts_x.push_back(*itr); }
  }
  if (pts_x.empty()) { pts_x = pts_z; }

  vcl_vector<vnl_double_3> pts_y = pts_x;

  vcl_sort(pts_x.begin(), pts_x.end(), bmrf_cmp_x);
  vcl_sort(pts_y.begin(), pts_y.end(), bmrf_cmp_y);

  unsigned int min_ind_x = 0, min_ind_y = 0, min_ind_z = 0;
  unsigned int max_ind_x = pts_x.size()-1, max_ind_y = pts_y.size()-1, max_ind_z = pts_z.size()-1;
  if (inlier_fractions) {
    float fraction_out = (1.f - inlier_fractions[0])*.5f;
    min_ind_x = (unsigned int)(max_ind_x*fraction_out);
    max_ind_x = (unsigned int)(max_ind_x*(1.f-fraction_out));
    fraction_out = (1.f - inlier_fractions[1])*.5f;
    min_ind_y = (unsigned int)(max_ind_y*fraction_out);
    max_ind_y = (unsigned int)(max_ind_y*(1.f-fraction_out));
    fraction_out = (1.f - inlier_fractions[2])*.5f;
    min_ind_z = (unsigned int)(max_ind_z*fraction_out);
    max_ind_z = (unsigned int)(max_ind_z*(1.f-fraction_out));
  }

  vnl_double_3 min_point(pts_x[min_ind_x][0], pts_y[min_ind_y][1], pts_z[min_ind_z][2]);
  vnl_double_3 max_point(pts_x[max_ind_x][0], pts_y[max_ind_y][1], pts_z[max_ind_z][2]);

  vnl_double_3 diag_vector = max_point - min_point;
  vnl_vector_fixed<double,3> x_axis(0.0), y_axis(0.0), z_axis(0.0);
  x_axis[0] = diag_vector[0];
  y_axis[1] = diag_vector[1];
  z_axis[2] = diag_vector[2];

  vnl_double_3x3 inv_rot = rot.transpose();

  x_axis = inv_rot * x_axis;
  y_axis = inv_rot * y_axis;
  z_axis = inv_rot * z_axis;

  vnl_double_3 origin = inv_rot * min_point;

  bb_xform_.set_identity();
  bb_xform_(0,0)=x_axis[0]; bb_xform_(1,0)=x_axis[1]; bb_xform_(2,0)=x_axis[2];
  bb_xform_(0,1)=y_axis[0]; bb_xform_(1,1)=y_axis[1]; bb_xform_(2,1)=y_axis[2];
  bb_xform_(0,2)=z_axis[0]; bb_xform_(1,2)=z_axis[1]; bb_xform_(2,2)=z_axis[2];
  bb_xform_(0,3)=origin[0]; bb_xform_(1,3)=origin[1]; bb_xform_(2,3)=origin[2];

  return true;
}


//: Build the curves
bool
bmrf_curve_3d_builder::build(int min_prj, int min_len, double trim_thresh)
{
  if (!network_)
    return false;

  unsigned int num_frames = network_->num_frames();
  if (num_frames < 2)
    return false;

  find_alpha_bounds();

  vcl_set<bmrf_curve_3d_sptr> growing_curves;

  // Build an initial set of curves
  vcl_set<bmrf_curvel_3d_sptr> empty_set;
  vcl_set<bmrf_curvel_3d_sptr> init_curvels = build_curvels(empty_set, min_alpha_);
  for ( vcl_set<bmrf_curvel_3d_sptr>::iterator itr = init_curvels.begin();
        itr != init_curvels.end(); ++itr)
  {
    bmrf_curve_3d_sptr new_curve = new bmrf_curve_3d;
    new_curve->push_back(*itr);
    curves_.insert(new_curve);
    growing_curves.insert(new_curve);
  }

  // Sweep through alpha
  double da = 0.001; // step size in alpha
  for ( double alpha = min_alpha_+da; alpha < max_alpha_; alpha += da ) {
    vcl_cout << "percent complete : " << (alpha - min_alpha_)/(max_alpha_-min_alpha_)*100.0<< vcl_endl;
    vcl_set<bmrf_curvel_3d_sptr> curvels = extend_curves(growing_curves, alpha);
    // find all curvels
    vcl_set<bmrf_curvel_3d_sptr> new_curvels = build_curvels(curvels, alpha);
    this->append_curvels(new_curvels, growing_curves, min_prj);
  }

  // Clean up curves
  for ( vcl_set<bmrf_curve_3d_sptr >::iterator itr = curves_.begin();
        itr != curves_.end();)
  {
    vcl_set<bmrf_curve_3d_sptr >::iterator next_itr = itr;
    ++next_itr;

    (*itr)->fill_gaps( network_->frame_numbers(), da);
    (*itr)->interp_gaps( network_->frame_numbers() );
    (*itr)->stat_trim( trim_thresh );
    if ( (*itr)->size() < (unsigned int)min_len )
      curves_.erase(itr);
    itr = next_itr;
  }

  return true;
}


//: Reconstruct the 3D curves from the curvel chains
void
bmrf_curve_3d_builder::reconstruct(float sigma)
{
  for ( vcl_set<bmrf_curve_3d_sptr >::iterator itr = curves_.begin();
        itr != curves_.end();  ++itr)
  {
    (*itr)->reconstruct(C_, sigma);
  }
}


//: Determine the alpha bounds from the network
void
bmrf_curve_3d_builder::find_alpha_bounds()
{
  min_alpha_ = 4.0;  // greater than pi
  max_alpha_ = -4.0; // less than -pi
  for ( bmrf_network::seg_node_map::const_iterator itr = network_->begin();
        itr != network_->end();  ++itr )
  {
    min_alpha_ = vcl_min(min_alpha_, itr->first->min_alpha());
    max_alpha_ = vcl_max(max_alpha_, itr->first->max_alpha());
  }
}


//: For sorting matches by arc probability
static bool
bmrf_arc_prob_cmp( const bmrf_arc_sptr& left_arc,
                   const bmrf_arc_sptr& right_arc )
{
  return left_arc->probability() < right_arc->probability();
}


//: Build curvels by linking across time through probable arcs
vcl_set<bmrf_curvel_3d_sptr>
bmrf_curve_3d_builder::build_curvels(vcl_set<bmrf_curvel_3d_sptr>& all_curvels, 
                                     double alpha) const
{
  vcl_set<int> frames = network_->frame_numbers();

  // find all arcs at alpha
  vcl_vector<bmrf_arc_sptr> all_arcs = this->find_arcs_at(alpha);
  // sort by probability
  vcl_sort(all_arcs.begin(), all_arcs.end(), bmrf_arc_prob_cmp);

  typedef vcl_map<bmrf_node_sptr, bmrf_curvel_3d_sptr> node_curvel_map;
  node_curvel_map node_map;

  for ( vcl_set<bmrf_curvel_3d_sptr>::const_iterator c_itr = all_curvels.begin();
        c_itr != all_curvels.end();  ++c_itr )
  {
    if ( (*c_itr)->num_projections() == 0 )
      continue;
    for ( vcl_set<int>::const_iterator fitr = frames.begin();
          fitr != frames.end();  ++fitr ) {
      bmrf_node_sptr update_node = (*c_itr)->node_at_frame(*fitr);
      if ( update_node )
        node_map[update_node] = *c_itr;
    }
  }

  vcl_set<bmrf_curvel_3d_sptr> new_curvels;

  while ( !all_arcs.empty() )
  {
    bmrf_arc_sptr curr_arc = all_arcs.back();
    all_arcs.pop_back();

    bmrf_node_sptr from_node = curr_arc->from();
    bmrf_node_sptr to_node = curr_arc->to();

    node_curvel_map::iterator itr1 = node_map.find(from_node);
    node_curvel_map::iterator itr2 = node_map.find(to_node);
    if ( itr1 == node_map.end() )
    {
      if ( itr2 == node_map.end() ) {
        bmrf_curvel_3d_sptr curr_curvel = new bmrf_curvel_3d;
        curr_curvel->set_proj_in_frame(from_node->frame_num(), alpha, from_node);
        curr_curvel->set_proj_in_frame(to_node->frame_num(), alpha, to_node);
        node_map[from_node] = curr_curvel;
        node_map[to_node] = curr_curvel;
        new_curvels.insert(curr_curvel);
      }
      else {
        bmrf_curvel_3d_sptr curr_curvel = itr2->second;
        bmrf_node_sptr other_node = curr_curvel->node_at_frame(from_node->frame_num());
        if ( !other_node ) {
          curr_curvel->set_proj_in_frame(from_node->frame_num(), alpha, from_node);
          node_map[from_node] = curr_curvel;
        }
      }
    }
    else
    {
      if ( itr2 == node_map.end() ){
        bmrf_curvel_3d_sptr curr_curvel = itr1->second;
        if ( !curr_curvel->node_at_frame(to_node->frame_num()) ){
          curr_curvel->set_proj_in_frame(to_node->frame_num(), alpha, to_node);
          node_map[to_node] = curr_curvel;
        }
      }
      else
      {
        bmrf_curvel_3d_sptr curvel_from = itr1->second;
        bmrf_curvel_3d_sptr curvel_to = itr2->second;
        if ( curvel_from->merge(curvel_to) ) {
          if (new_curvels.erase(curvel_to) == 0)
            all_curvels.erase(curvel_to);
          for ( vcl_set<int>::const_iterator fitr = frames.begin();
                fitr != frames.end();  ++fitr ) {
            bmrf_node_sptr update_node = curvel_to->node_at_frame(*fitr);
            if ( update_node )
              node_map[update_node] = curvel_from;
          }
        }
      }
    }
  }
  return new_curvels;
}


//: extend all curves to the next alpha
vcl_set<bmrf_curvel_3d_sptr>
bmrf_curve_3d_builder::extend_curves( vcl_set<bmrf_curve_3d_sptr>& growing_curves,
                                      double alpha )
{
  vcl_set<int> frames = network_->frame_numbers();
  vcl_set<bmrf_curvel_3d_sptr> new_curvels;

  for ( vcl_set<bmrf_curve_3d_sptr>::const_iterator itr = growing_curves.begin();
        itr != growing_curves.end();  ++itr )
  {
    bmrf_curve_3d::reverse_iterator c_itr = (*itr)->rbegin();

    bmrf_curvel_3d_sptr curvel = new bmrf_curvel_3d;
    new_curvels.insert(curvel);
    (*itr)->push_back(curvel);

    for ( vcl_set<int>::const_iterator fitr = frames.begin();
          fitr != frames.end();  ++fitr ) {
      bmrf_node_sptr node = (*c_itr)->node_at_frame(*fitr);
      if ( !node )
        continue;
      bool prev_valid = true;
      for (unsigned int c=0; c<2; ++c){
        if ( --c_itr == (*itr)->rend() || node != (*c_itr)->node_at_frame(*fitr)){
          prev_valid = false;
          break;
        }
      }
      if ( !prev_valid )
        continue;

      if ((node->epi_seg()->min_alpha() <= alpha) &&
          (node->epi_seg()->max_alpha() >= alpha))
        curvel->set_proj_in_frame(*fitr, alpha, node);
    }
  }
  return new_curvels;
}


//: Find all curves that intersect \p alpha in \p frame
vcl_vector<bmrf_arc_sptr>
bmrf_curve_3d_builder::find_arcs_at(double alpha) const
{
  vcl_vector<bmrf_arc_sptr> matches;
  for ( bmrf_network::seg_node_map::const_iterator itr = network_->begin();
        itr != network_->end();  ++itr )
  {
    if ((itr->first->min_alpha() <= alpha) &&
        (itr->first->max_alpha() >= alpha))
    {
      for ( bmrf_node::arc_iterator a_itr = itr->second->begin(bmrf_node::TIME);
            a_itr != itr->second->end(bmrf_node::TIME); ++a_itr )
      {
        bmrf_epi_seg_sptr other_seg = (*a_itr)->to()->epi_seg();
        if ((other_seg->min_alpha() <= alpha) &&
            (other_seg->max_alpha() >= alpha))
        {
          matches.push_back(*a_itr);
        }
      }
    }
  }
  return matches;
}


//: Reconstruct the 3d location of a curvel from its projections
void
bmrf_curve_3d_builder::reconstruct_point(bmrf_curvel_3d_sptr curvel) const
{
  unsigned int nviews = curvel->num_projections(true);

  vcl_cout << "reconstructing from " << nviews << " views" << vcl_endl;

  vnl_matrix<double> A(2*nviews, 4, 0.0);

  unsigned int v = 0;
  for ( vcl_map<int,vnl_double_3x4>::const_iterator C_itr = C_.begin();
        C_itr != C_.end();  ++C_itr ) {
    const int f = C_itr->first;
    const vnl_double_3x4 cam = C_itr->second;
    vnl_double_2 pos;
    if ( curvel->pos_in_frame(f,pos) ) {
      for (unsigned int i=0; i<4; i++) {
        A[2*v  ][i] = pos[0]*cam[2][i] - cam[0][i];
        A[2*v+1][i] = pos[1]*cam[2][i] - cam[1][i];
      }
      ++v;
    }
  }
  vnl_svd<double> svd_solver(A);
  vnl_double_4 p = svd_solver.nullvector();

  curvel->set(p[0]/p[3], p[1]/p[3], p[2]/p[3]);
}


//: Match the \p curvels to the ends of the \p growing_curves set of lists
void
bmrf_curve_3d_builder::append_curvels(vcl_set<bmrf_curvel_3d_sptr>& curvels,
                                      vcl_set<bmrf_curve_3d_sptr>& growing_curves,
                                      int min_prj)
{
  vcl_set<bmrf_curve_3d_sptr> grown_curves;

  typedef vcl_pair<bmrf_curvel_3d_sptr, bmrf_curvel_3d_sptr> append_match;
  vcl_vector<vcl_pair<double, append_match> > matches;
  for ( vcl_set<bmrf_curve_3d_sptr>::iterator g_itr = growing_curves.begin();
        g_itr != growing_curves.end();  ++g_itr )
  {
    typedef vcl_set<bmrf_curvel_3d_sptr>::iterator curvel_iterator;

    bmrf_curvel_3d_sptr end_curvel = (*g_itr)->back();
    bmrf_curvel_3d_sptr prev_curvel = *(++((*g_itr)->rbegin()));

    for ( curvel_iterator c_itr = curvels.begin();
          c_itr != curvels.end();  ++c_itr )
    {
      double align = this->append_correct((*c_itr), prev_curvel);
      if ( align > 0 ) {
        matches.push_back(vcl_pair<double, append_match>(align, append_match(end_curvel, *c_itr)));
      }
    }
  }

  vcl_sort(matches.begin(), matches.end());

  while ( !matches.empty() )
  {
    bmrf_curvel_3d_sptr base_curvel = matches.back().second.first;
    bmrf_curvel_3d_sptr merge_curvel = matches.back().second.second;
    matches.pop_back();
    vcl_set<bmrf_curvel_3d_sptr>::iterator c_itr = curvels.find(merge_curvel);
    if ( c_itr == curvels.end() )
      continue;
    if ( base_curvel->merge(merge_curvel) )
      curvels.erase(c_itr);
  }

  for ( vcl_set<bmrf_curve_3d_sptr>::iterator g_itr = growing_curves.begin();
        g_itr != growing_curves.end();  ++g_itr )
  {
    if ( (*g_itr)->back()->num_projections() < min_prj )
      (*g_itr)->pop_back();
    else
      grown_curves.insert(*g_itr);
  }

  // make new curves for the unmatched curvels
  for ( vcl_set<bmrf_curvel_3d_sptr>::iterator c_itr = curvels.begin();
        c_itr != curvels.end();  ++c_itr )
  {
    if ( (*c_itr)->num_projections() < min_prj)
      continue;
    bmrf_curve_3d_sptr new_curve = new bmrf_curve_3d;
    new_curve->push_back(*c_itr);
    curves_.insert(new_curve);
    grown_curves.insert(new_curve);
  }

  growing_curves = grown_curves;
}


//: Return a measure (0.0 to 1.0) of how well \p new_c matches \p prev_c
double
bmrf_curve_3d_builder::append_correct( const bmrf_curvel_3d_sptr& new_c,
                                       const bmrf_curvel_3d_sptr& prev_c ) const
{
  vcl_set<int> frames = network_->frame_numbers();
  unsigned int total_overlap = 0;
  unsigned int total_cover = 0;
  unsigned int total_equal = 0;
  for ( vcl_set<int>::const_iterator fitr = frames.begin();
        fitr != frames.end();  ++fitr )
  {
    bmrf_node_sptr p_node = prev_c->node_at_frame(*fitr);
    bmrf_node_sptr n_node = new_c->node_at_frame(*fitr);
    if ( p_node || n_node )
      ++total_cover;
    if ( p_node && n_node ) {
      ++total_overlap;
      if ( p_node == n_node )
        ++total_equal;
    }
  }

  if (total_overlap == 0)
    return 0.0;

  // Rate according to percentage of overlapping curvels that match
  // Use the percentage of curves that overlap as a tie-breaker
  double equal_ratio = double(total_equal)/double(total_overlap);
  double overlap_ratio = double(total_overlap)/double(total_cover);
  return equal_ratio - (1.0 - overlap_ratio)/double(frames.size());
}
