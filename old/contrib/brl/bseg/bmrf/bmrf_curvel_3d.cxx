// This is brl/bseg/bmrf/bmrf_curvel_3d.cxx
//:
// \file

#include "bmrf_curvel_3d.h"
#include "bmrf_node.h"
#include "bmrf_gamma_func.h"
#include "bmrf_epi_seg.h"
#include <vcl_algorithm.h>
#include <vsl/vsl_vector_io.h>
#include <vsl/vsl_map_io.h>
#include <vnl/io/vnl_io_matrix_fixed.h>

//: Constructor
bmrf_curvel_3d::bmrf_curvel_3d()
 : bugl_gaussian_point_3d<double>(),
   sum_gamma_(0.0), sum_sqr_gamma_(0.0), stats_valid_(false),
   num_projections_(0)
{
  this->set(0.0, 0.0, 0.0);
}

//: Copy constructor
bmrf_curvel_3d::bmrf_curvel_3d(bmrf_curvel_3d const& c)
 : bugl_gaussian_point_3d<double>(), vbl_ref_count(),
   sum_gamma_(c.sum_gamma_), sum_sqr_gamma_(c.sum_sqr_gamma_), stats_valid_(c.stats_valid_),
   num_projections_(c.num_projections_)
{
}

//: Constructor
bmrf_curvel_3d::bmrf_curvel_3d(vgl_point_3d<double> &p, vnl_double_3x3 &s)
 : bugl_gaussian_point_3d<double>(p, s),
   sum_gamma_(0.0), sum_sqr_gamma_(0.0), stats_valid_(false),
   num_projections_(0)
{
}

//: Constructor
bmrf_curvel_3d::bmrf_curvel_3d(double x, double y, double z, vnl_double_3x3 & s)
 : bugl_gaussian_point_3d<double>(x, y, z, s),
   sum_gamma_(0.0), sum_sqr_gamma_(0.0), stats_valid_(false),
   num_projections_(0)
{
}


//: Merge the other curvel into this curvel if there is no overlap
// \return false if merging is not possible
bool
bmrf_curvel_3d::merge(const bmrf_curvel_3d_sptr& other)
{
  unsigned int num_frames = vcl_max(this->projs_2d_.size(), other->projs_2d_.size());
  vcl_vector<vcl_pair<double,bmrf_node_sptr> > merged_projs_2d_(num_frames);
  unsigned int count = 0;
  for ( unsigned int f=0; f<num_frames; ++f )
  {
    bmrf_node_sptr n1 = NULL, n2 = NULL;
    if ( f<this->projs_2d_.size() )
      n1 = this->projs_2d_[f].second;
    if ( f<other->projs_2d_.size() )
      n2 = other->projs_2d_[f].second;

    if ( n1 && n2 )
      return false;

    if ( n1 ) {
      merged_projs_2d_[f] = this->projs_2d_[f];
      ++count;
    }
    else if ( n2 ) {
      merged_projs_2d_[f] = other->projs_2d_[f];
      ++count;
    }
  }
  num_projections_ = count;
  this->projs_2d_ = merged_projs_2d_;
  stats_valid_ = false;
  return true;
}


//: Set the projection of this curvel into \p frame as the segment in
//  \p node at the value \p alpha
void
bmrf_curvel_3d::set_proj_in_frame(unsigned int frame, double alpha, const bmrf_node_sptr& node)
{
  if (!node)
    return;

  if (frame >= projs_2d_.size())
    projs_2d_.resize(frame+1);


  if (!projs_2d_[frame].second)
    ++num_projections_;
  projs_2d_[frame] = vcl_pair<double, bmrf_node_sptr>(alpha, node);
  stats_valid_ = false;
}


//: Set the projection of this curvel into \p frame at an interpolated position
void
bmrf_curvel_3d::set_psuedo_point(unsigned int frame, const vnl_double_2& pos)
{
  if (frame >= projs_2d_.size())
    projs_2d_.resize(frame+1);

  pseudo_points_[frame] = pos;
}


//: Returns the 2d position of this curvel in \p frame by reference
bool
bmrf_curvel_3d::pos_in_frame(unsigned int frame, vnl_double_2& pos) const
{
  if ( frame >= projs_2d_.size() )
    return false;
  double alpha = projs_2d_[frame].first;
  bmrf_node_sptr node = projs_2d_[frame].second;
  if ( node && node->epi_seg()) {
    pos[0] = node->epi_seg()->x(alpha);
    pos[1] = node->epi_seg()->y(alpha);
    return true;
  }
  vcl_map<unsigned int, vnl_double_2>::const_iterator p_itr = pseudo_points_.find(frame);
  if ( p_itr != pseudo_points_.end() ) {
    pos = p_itr->second;
    return true;
  }

  return false;
}


//: Returns the invariant cross ratio
bool
bmrf_curvel_3d::cross_ratio(unsigned int frame, double& ratio) const
{
  if ( frame >= projs_2d_.size() || frame < 2 )
    return false;

  double s[3];
  for (unsigned int i=0; i<3; ++i) {
    double alpha = projs_2d_[frame+i-2].first;
    bmrf_node_sptr node = projs_2d_[frame+i-2].second;
    if ( node && node->epi_seg())
      s[i] = node->epi_seg()->s(alpha);
    else
      return false;
  }
  ratio = s[0]*(s[1]-s[2])/(s[1]*(s[0]-s[2]));
  return true;
}


//: Returns the smart pointer to the node at the projection into \p frame
bmrf_node_sptr
bmrf_curvel_3d::node_at_frame(unsigned int frame) const
{
  if ( frame >= projs_2d_.size() )
    return NULL;
  return projs_2d_[frame].second;
}


//: Returns the alpha value at the projection into \p frame
double
bmrf_curvel_3d::alpha_at_frame(unsigned int frame) const
{
  if ( frame >= projs_2d_.size() )
    return 0.0;
  return projs_2d_[frame].first;
}


//: Return true if \p a projection of this curvel lies on \p node
bool
bmrf_curvel_3d::is_projection(const bmrf_node_sptr& node) const
{
  for ( vcl_vector<vcl_pair<double,bmrf_node_sptr> >::const_iterator itr = projs_2d_.begin();
        itr != projs_2d_.end();  ++itr )
  {
    if (node == itr->second)
      return true;
  }
  return false;
}


//: Return the number of projections available
int
bmrf_curvel_3d::num_projections(bool include_pseudo) const
{
  if (include_pseudo)
    return num_projections_ + pseudo_points_.size();

  return num_projections_;
}


//: Return the average gamma value relative to frame 0
double
bmrf_curvel_3d::gamma_avg()
{
  if (!stats_valid_)
    compute_statistics();
  return sum_gamma_ / (num_projections_*(num_projections_-1)/2);
}


//: Return the standard deviation of the gamma values
double
bmrf_curvel_3d::gamma_std()
{
  if (!stats_valid_)
    compute_statistics();
  int num_samples = (num_projections_*(num_projections_-1)/2);
  double avg_sqr_gamma = sum_sqr_gamma_ / (num_samples-1);
  double avg_gamma = sum_gamma_ / num_samples;
  double var = avg_sqr_gamma - avg_gamma*sum_gamma_/(num_samples-1);
  return vcl_sqrt(var);
}


//: Return the average s value projected into \p frame
double
bmrf_curvel_3d::s_avg(unsigned int frame)
{
  double gamma = this->gamma_avg();
  int count = 0;
  double s_sum = 0.0;
  for (unsigned int f=0; f<projs_2d_.size(); ++f) {
    if ( (frame == f) || !projs_2d_[f].second)
      continue;
    double s = projs_2d_[f].second->epi_seg()->s(projs_2d_[f].first);
    s_sum += s * (1.0 - gamma*f) / (1.0 - gamma*frame);
    ++count;
  }
  return s_sum / count;
}


//: Compute the gamma statistics on the current projections
void
bmrf_curvel_3d::compute_statistics()
{
  sum_gamma_ = 0.0;
  sum_sqr_gamma_ = 0.0;
  int base_frame = -1; // map all gamma's to this frame
  for (unsigned int ind1=0; ind1<projs_2d_.size(); ++ind1)
  {
    if (!projs_2d_[ind1].second)
      continue;
    if (base_frame == -1)
      base_frame = ind1;
    double s1 = projs_2d_[ind1].second->epi_seg()->s(projs_2d_[ind1].first);
    for (unsigned int ind2=ind1+1; ind2<projs_2d_.size(); ++ind2) {
      if (!projs_2d_[ind2].second)
        continue;
      double s2 = projs_2d_[ind2].second->epi_seg()->s(projs_2d_[ind2].first);
      double gamma = 1.0 / (((int(ind2) - int(ind1)) / (1.0 - s1/s2)) + double(ind1-base_frame));
      sum_gamma_ += gamma;
      sum_sqr_gamma_ += gamma*gamma;
    }
  }
  stats_valid_ = true;
}


void
bmrf_curvel_3d::show_stats() const
{
  int base_frame = -1; // map all gamma's to this frame
  for (unsigned int ind1=0; ind1<projs_2d_.size(); ++ind1)
  {
    if (!projs_2d_[ind1].second)
      continue;
    if (base_frame == -1)
      base_frame = ind1;
    double s1 = projs_2d_[ind1].second->epi_seg()->s(projs_2d_[ind1].first);
    for (unsigned int ind2=ind1+1; ind2<projs_2d_.size(); ++ind2) {
      if (!projs_2d_[ind2].second)
        continue;
      double s2 = projs_2d_[ind2].second->epi_seg()->s(projs_2d_[ind2].first);
      double gamma = 1.0 / (((int(ind2) - int(ind1)) / (1.0 - s1/s2)) + double(ind1-base_frame));
      vcl_cout << " frames " << ind1 << ',' << ind2 << " gamma=" << gamma << vcl_endl;
    }
  }
}


//: Binary save self to stream.
void
bmrf_curvel_3d::b_write( vsl_b_ostream& os ) const
{
  vsl_b_write(os, version());

  vsl_b_write(os, this->x());
  vsl_b_write(os, this->y());
  vsl_b_write(os, this->z());
  vsl_b_write(os, this->projs_2d_);
  vsl_b_write(os, this->pseudo_points_);
  vsl_b_write(os, this->proj_error_);
}


//: Binary load self from stream.
void
bmrf_curvel_3d::b_read( vsl_b_istream& is )
{
  if (!is) return;

  short ver;
  vsl_b_read(is, ver);
  switch (ver)
  {
   case 1:
   {
    double x, y, z;
    vsl_b_read(is, x);
    vsl_b_read(is, y);
    vsl_b_read(is, z);
    this->set(x,y,z);
    vsl_b_read(is, this->projs_2d_);
    vsl_b_read(is, this->pseudo_points_);
    vsl_b_read(is, this->proj_error_);
    this->stats_valid_ = false;

    for (unsigned int i=0; i<this->projs_2d_.size(); ++i)
      if ( this->projs_2d_[i].second )
        ++num_projections_;

    break;
   }

   default:
    vcl_cerr << "I/O ERROR: bmrf_curvel_3d::b_read(vsl_b_istream&)\n"
             << "           Unknown version number "<< ver << '\n';
    is.is().clear(vcl_ios::badbit); // Set an unrecoverable IO error on stream
    return;
  }
}


//: Return IO version number;
short
bmrf_curvel_3d::version() const
{
  return 1;
}


//: Print an ascii summary to the stream
void
bmrf_curvel_3d::print_summary( vcl_ostream& os ) const
{
  os << "num_proj=" << this->num_projections_;
}

//-----------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------

//: Binary save bmrf_curvel_3d to stream.
void
vsl_b_write(vsl_b_ostream &os, const bmrf_curvel_3d* c)
{
  if (c==0) {
    vsl_b_write(os, false); // Indicate null pointer stored
  }
  else {
    vsl_b_write(os,true); // Indicate non-null pointer stored
    c->b_write(os);
  }
}


//: Binary load bmrf_curvel_3d from stream.
void
vsl_b_read(vsl_b_istream &is, bmrf_curvel_3d* &c)
{
  delete c;
  bool not_null_ptr;
  vsl_b_read(is, not_null_ptr);
  if (not_null_ptr) {
    c = new bmrf_curvel_3d();
    c->b_read(is);
  }
  else
    c = 0;
}


//: Print an ASCII summary to the stream
void
vsl_print_summary(vcl_ostream &os, const bmrf_curvel_3d* c)
{
  os << "bmrf_curvel_3d{ ";
  c->print_summary(os);
  os << " }";
}

