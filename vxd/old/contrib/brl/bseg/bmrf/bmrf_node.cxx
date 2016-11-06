// This is brl/bseg/bmrf/bmrf_node.cxx
#include "bmrf_node.h"
//:
// \file

#include <bmrf/bmrf_arc.h>
#include <bmrf/bmrf_epi_transform.h>
#include <bmrf/bmrf_gamma_func.h>
#include <bmrf/bmrf_epi_seg_compare.h>
#include <vsl/vsl_binary_io.h>
#include <vbl/io/vbl_io_smart_ptr.h>
#include <vcl_algorithm.h>
#include <vcl_cmath.h>


//: Constructor
bmrf_node::bmrf_node( const bmrf_epi_seg_sptr& epi_seg, int frame_num,
                      double prob )
  : segment_(epi_seg), frame_num_(frame_num), probability_(prob),
    out_arcs_(), in_arcs_(), boundaries_(ALL+1, out_arcs_.end()), sizes_(ALL,0)
{
}

//: Copy constructor
bmrf_node::bmrf_node(bmrf_node const& n)
  : vbl_ref_count(),
    segment_(n.segment_), frame_num_(n.frame_num_), probability_(n.probability_),
    out_arcs_(n.out_arcs_), in_arcs_(n.in_arcs_), boundaries_(n.boundaries_), sizes_(n.sizes_)
{
}

//: Strip all of the arcs from this node
void
bmrf_node::strip()
{
  for (int t = 0; t<ALL; ++t)
    for (arc_iterator itr = boundaries_[t]; itr != boundaries_[t+1]; )
    {
      arc_iterator next_itr = itr; ++next_itr;
      remove_helper(itr, neighbor_type(t));
      itr = next_itr;
    }

  for (arc_iterator itr = in_arcs_.begin(); itr != in_arcs_.end(); ++itr)
    if ((*itr)->from_)
      (*(itr--))->from_->remove_neighbor(this, ALL);
}


//: Remove any arcs to or from NULL nodes
bool
bmrf_node::purge()
{
  bool retval = false;
  for (int t = 0; t<ALL; ++t)
  {
    for (arc_iterator itr = boundaries_[t]; itr != boundaries_[t+1]; ++itr) {
      if (!(*itr)->to_) {
        // adjust boundaries if necessary
        for (int t2=t; t2>=0 && (boundaries_[t2] == itr); --t2)
          ++boundaries_[t2];
        out_arcs_.erase(itr--); // remove the arc
        --sizes_[t]; // decrease count
        retval = true;
      }
    }
  }

  for (arc_iterator itr = in_arcs_.begin(); itr != in_arcs_.end(); ++itr) {
    if (!(*itr)->from_) {
      in_arcs_.erase(itr--);
      retval = true;
    }
  }

  // Invalidate the probability
  this->probability_ = -1.0;

  return retval;
}


//: Return the probability of this node
double
bmrf_node::probability()
{
  if (probability_ < 0.0)
    compute_probability();

  return probability_;
}


//: Return the gamma function of this node
bmrf_gamma_func_sptr
bmrf_node::gamma()
{
  if (!gamma_.ptr())
    compute_probability();

  return gamma_;
}


static bool
pair_dbl_arc_gt_cmp ( const vcl_pair<double,bmrf_node::arc_iterator>& lhs,
                      const vcl_pair<double,bmrf_node::arc_iterator>& rhs )
{
  return lhs.first > rhs.first;
}

//: Compute the conditional probability that this node is correct given its neighbors
void
bmrf_node::compute_probability()
{
  probability_ = 0.0;
  double sum = 0.0;
  for ( arc_iterator a_itr = this->begin(TIME);
        a_itr != this->end(TIME); ++a_itr )
  {
    // use the gamma function associated with the arc
    bmrf_gamma_func_sptr gamma_func = (*a_itr)->gamma_func();
    (*a_itr)->probability_ = this->probability(gamma_func, -1)
                            *this->probability(gamma_func, 1);

    sum += (*a_itr)->probability_;
    // select the probability of the best neighbor
    if ( (*a_itr)->probability_ > probability_ ) {
      probability_ = (*a_itr)->probability_;
      gamma_ = gamma_func;
    }
  }
  probability_ /= sum;
  // Normalize so probabilities sum to 1
  for ( arc_iterator a_itr = this->begin(TIME);
        a_itr != this->end(TIME); ++a_itr )
  {
    (*a_itr)->probability_ /= sum;
  }
}


//: Prune neighbors with a probability below \p threshold
void
bmrf_node::prune_by_probability(double threshold, bool relative)
{
  // Compute a probability mass function
  vcl_vector<vcl_pair<double,arc_iterator> > pmf;
  for ( arc_iterator a_itr = this->begin(TIME);
        a_itr != this->end(TIME); ++a_itr )
  {
    // use the gamma function associated with the arc
    bmrf_gamma_func_sptr gamma_func = (*a_itr)->gamma_func();
    pmf.push_back(vcl_pair<double,arc_iterator>( this->probability(gamma_func,-1)
                                                 *this->probability(gamma_func,1), a_itr));
  }

  // Sort the results by probability
  vcl_sort(pmf.begin(), pmf.end(), pair_dbl_arc_gt_cmp);

  // if relative, modify the threshold relative to the maximum probability
  if ( relative )
    threshold *= pmf.front().first;

  // Remove arcs to neighbors that are below threshold
  vcl_vector<vcl_pair<double,arc_iterator> >::iterator p_itr = pmf.begin();
  while ( p_itr != pmf.end() && p_itr->first > threshold)  ++p_itr;
  for ( ; p_itr != pmf.end(); ++p_itr ) {
    remove_helper(p_itr->second, TIME);
  }
}


//: Prune neighbors with a gamma outside this range
void
bmrf_node::prune_by_gamma(double min_gamma, double max_gamma)
{
  for ( arc_iterator a_itr = this->begin(TIME);
        a_itr != this->end(TIME);)
  {
    double gamma = (*a_itr)->induced_gamma();

    arc_iterator next_itr = a_itr;
    ++next_itr;
    if ( gamma < min_gamma || gamma > max_gamma ){
      remove_helper(a_itr, TIME);
    }
    a_itr = next_itr;
  }
}


//: Prune directed arcs leaving only arcs to nodes which have arcs back to this node
void
bmrf_node::prune_directed()
{
  for (int t = 0; t<ALL; ++t){
    for (arc_iterator o_itr = boundaries_[t]; o_itr != boundaries_[t+1]; ) {
      bool found = false;
      for ( arc_iterator i_itr = in_arcs_.begin();
            i_itr != in_arcs_.end(); ++i_itr )
        if ( (*i_itr)->from_ == (*o_itr)->to_ ){
          found = true;
          break;
        }
      if (!found){
        arc_iterator next_itr = o_itr;
        ++next_itr;
        remove_helper(o_itr, neighbor_type(t));
        o_itr = next_itr;
      }
      else
        ++o_itr;
    }
  }
}


//: Calculate the error in similarity between this transformed by \p xform
double
bmrf_node::probability(const bmrf_gamma_func_sptr& gamma, int time_step)
{
  // precompute the transformed segment
  bmrf_epi_seg_sptr trans_seg = bmrf_epi_transform(this->epi_seg(), gamma, time_step);
  double prob = 0.0;
  double total_alpha = 0.0;
  for ( arc_iterator a_itr = this->begin(TIME); a_itr != this->end(TIME); ++a_itr ) {
    if ((*a_itr)->time_step() != time_step)
      continue;

    bmrf_node_sptr neighbor = (*a_itr)->to();
    double error = bmrf_match_error(trans_seg, neighbor->epi_seg());

    double alpha_range = (*a_itr)->max_alpha_ - (*a_itr)->min_alpha_;
    const double int_var = 0.001; // intensity variance
    prob += alpha_range * vcl_exp(-error/2.0 - (*a_itr)->avg_intensity_error_/(2.0*int_var));
    total_alpha += alpha_range;
  }

  // no neighbors found in this frame
  if (total_alpha == 0.0)
    return 1.0;

  return prob / total_alpha;
}


//: Add \p node as a neighbor of type \p type
bool
bmrf_node::add_neighbor( const bmrf_node_sptr& node, neighbor_type type )
{
  if (!node || node == this || type == ALL)
    return false;

  // verify that this arc is not already present
  for (arc_iterator itr = boundaries_[type]; itr != boundaries_[type+1]; ++itr)
    if ((*itr)->to_ == node) return false;

  // add the arc
  bmrf_arc_sptr new_arc = new bmrf_arc(this, node);
  out_arcs_.insert(boundaries_[type+1], new_arc);
  node->in_arcs_.push_back(new_arc);
  ++sizes_[type];

  // adjust boundaries if necessary
  for (int t=type; t>=0 && (boundaries_[type+1] == boundaries_[t]); --t)
    --boundaries_[t];

  // Invalidate the probability
  this->probability_ = -1.0;

  return true;
}


//: Add an arc \p arc of type \p type
bool
bmrf_node::add_arc( const bmrf_arc_sptr& arc, neighbor_type type )
{
  if ( !arc.ptr() || arc->from_ != this || arc->to_ == this || type == ALL)
    return false;

  // verify that this arc is not already present
  for (arc_iterator itr = boundaries_[type]; itr != boundaries_[type+1]; ++itr)
    if ((*itr)->to_ == arc->to_) return false;

  out_arcs_.insert(boundaries_[type+1], arc);
  arc->to_->in_arcs_.push_back(arc);
  ++sizes_[type];

  // adjust boundaries if necessary
  for (int t=type; t>=0 && (boundaries_[type+1] == boundaries_[t]); --t)
    --boundaries_[t];

  // Invalidate the probability
  this->probability_ = -1.0;

  return true;
}


//: Remove \p node from the neighborhood
bool
bmrf_node::remove_neighbor( bmrf_node_sptr node, neighbor_type type )
{
  if (!node || node == this)
    return false;

  bool removed = false;
  int init_t = type;
  if (type == ALL) init_t=0;

  for (int t = init_t; t<ALL; ++t)
  {
    for (arc_iterator itr = boundaries_[t]; itr != boundaries_[t+1]; ++itr) {
      if ((*itr)->to_ == node) {
        arc_iterator prev_itr = itr; --prev_itr;
        remove_helper(itr, neighbor_type(t));
        itr = prev_itr;
        if (type != ALL) return true;
        removed = true;
      }
    }
  }

  return removed;
}


//: Remove the arc associated with the outgoing iterator
bool
bmrf_node::remove_helper( arc_iterator& a_itr, neighbor_type type)
{
  bmrf_arc_sptr arc = *a_itr;
  if ( arc->from_ != this )
    return false;

  // adjust boundaries if necessary
  for (int t=int(type); t>=0 && (boundaries_[t] == a_itr); --t)
    ++boundaries_[t];

  // find the pointer back from the other node
  arc_iterator back_itr = vcl_find( (*a_itr)->to_->in_arcs_.begin(),
                                    (*a_itr)->to_->in_arcs_.end(),
                                    *a_itr );

  // erase the pointer back from the other node
  if (back_itr != (*a_itr)->to_->in_arcs_.end())
      (*a_itr)->to_->in_arcs_.erase(back_itr);

  // erase the arc
  out_arcs_.erase(a_itr);

  // decrease count
  --sizes_[type];

  // Make these pointers NULL in case someone else still has
  // a pointer to this arc
  arc->to_ = NULL;
  arc->from_ = NULL;

  this->probability_ = -1.0;

  return true;
}


//: Returns an iterator to the beginning of the type \p type neighbors
bmrf_node::arc_iterator
bmrf_node::begin( neighbor_type type )
{
  if (type == ALL) return out_arcs_.begin();
  return boundaries_[type];
}


//: Returns an iterator to the end of the type \p type neighbors
bmrf_node::arc_iterator
bmrf_node::end( neighbor_type type )
{
  if (type == ALL) return out_arcs_.end();
  return boundaries_[type+1];
}


//: Returns the number of outgoing neighbors to this node of type \p type
int
bmrf_node::num_neighbors( neighbor_type type )
{
  if (type == ALL) return out_arcs_.size();
  return sizes_[type];
}


//: Binary save self to stream.
void
bmrf_node::b_write( vsl_b_ostream& os ) const
{
  vsl_b_write(os, version());

  vsl_b_write(os, this->segment_);
  vsl_b_write(os, this->frame_num_);
  vsl_b_write(os, this->probability_);

  // write the number of nodes of each type
  for (int t=0; t<ALL; ++t)
    vsl_b_write(os, sizes_[t]);

  // write all the outgoing arcs
  for (vcl_list<bmrf_arc_sptr>::const_iterator itr = out_arcs_.begin();
       itr != out_arcs_.end(); ++itr) {
    vsl_b_write(os, *itr);  // Save the arc
  }

  // write the number of incoming arcs
  vsl_b_write(os, (unsigned int) in_arcs_.size());
  // write all the incoming arcs
  for (vcl_list<bmrf_arc_sptr>::const_iterator itr = in_arcs_.begin();
       itr != in_arcs_.end(); ++itr) {
    vsl_b_write(os, *itr);  // Save the arc
  }
}


//: Binary load self from stream.
void
bmrf_node::b_read( vsl_b_istream& is )
{
  if (!is) return;

  short ver;
  vsl_b_read(is, ver);
  switch (ver)
  {
   case 1:
   {
    vsl_b_read(is, this->segment_);
    vsl_b_read(is, this->frame_num_);
    vsl_b_read(is, this->probability_);

    out_arcs_.clear();
    boundaries_.clear();
    boundaries_.resize(ALL+1, out_arcs_.end());

    int num_neighbors = 0;
    for (int t=0; t<ALL; ++t) {
      vsl_b_read(is, sizes_[t]);
      num_neighbors += sizes_[t];
    }
    int type = 0;
    int b_loc = sizes_[0];
    for (int n=0; n<num_neighbors; ++n) {
      bmrf_arc_sptr arc_ptr;
      vsl_b_read(is, arc_ptr);
      arc_ptr->from_ = this;
      if (arc_ptr->to_)
        arc_ptr->time_init();
      out_arcs_.push_back(arc_ptr);

      while (type < ALL && n == b_loc) {
        boundaries_[++type] = out_arcs_.end();
        --boundaries_[type];
        b_loc += sizes_[type];
      }
    }
    boundaries_[0] = out_arcs_.begin();

    unsigned int num_incoming;
    vsl_b_read(is, num_incoming);
    for (unsigned int n=0; n<num_incoming; ++n) {
      bmrf_arc_sptr arc_ptr;
      vsl_b_read(is, arc_ptr);
      arc_ptr->to_ = this;
      if (arc_ptr->from_)
        arc_ptr->time_init();
      in_arcs_.push_back(arc_ptr);
    }
    break;
   }

   default:
    vcl_cerr << "I/O ERROR: bmrf_node::b_read(vsl_b_istream&)\n"
             << "           Unknown version number "<< ver << '\n';
    is.is().clear(vcl_ios::badbit); // Set an unrecoverable IO error on stream
    return;
  }
}


//: Return IO version number;
short
bmrf_node::version() const
{
  return 1;
}


//: Print an ascii summary to the stream
void
bmrf_node::print_summary( vcl_ostream& os ) const
{
  os << "pr=" << probability_ << ", frame=" << frame_num_;
}


//-----------------------------------------------------------------------------------------
// External functions
//-----------------------------------------------------------------------------------------

//: Binary save bmrf_node to stream.
void
vsl_b_write(vsl_b_ostream &os, const bmrf_node* n)
{
  if (n==0) {
    vsl_b_write(os, false); // Indicate null pointer stored
  }
  else{
    vsl_b_write(os,true); // Indicate non-null pointer stored
    n->b_write(os);
  }
}


//: Binary load bmrf_node from stream.
void
vsl_b_read(vsl_b_istream &is, bmrf_node* &n)
{
  delete n;
  bool not_null_ptr;
  vsl_b_read(is, not_null_ptr);
  if (not_null_ptr) {
    n = new bmrf_node();
    n->b_read(is);
  }
  else
    n = 0;
}


//: Print an ASCII summary to the stream
void
vsl_print_summary(vcl_ostream &os, const bmrf_node* n)
{
  os << "bmrf_node{ ";
  n->print_summary(os);
  os << " }";
}

