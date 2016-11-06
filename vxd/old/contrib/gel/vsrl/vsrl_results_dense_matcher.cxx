// This is gel/vsrl/vsrl_results_dense_matcher.cxx
#include "vsrl_results_dense_matcher.h"
#include <vsrl/vsrl_parameters.h>


// constructor
vsrl_results_dense_matcher::vsrl_results_dense_matcher(const vil1_image &image1, const vil1_image &disparity):
  vsrl_dense_matcher(image1),
  disparity_(disparity)
{
  correlation_range_= vsrl_parameters::instance()->correlation_range; // probably 10
}

// destructor
vsrl_results_dense_matcher::~vsrl_results_dense_matcher()
{
}

// get the disparity
int vsrl_results_dense_matcher::get_disparity(int x, int y)
{
  int disp;

  disp = disparity_(x,y);

  if (!disp){
    // it looks like there was a null assignment
    return 0-1000;
  }

  disp = disp - correlation_range_ -1;

  return disp;
}


int vsrl_results_dense_matcher::get_assignment(int x, int y)
{
  // we want to get the assignment for pixel x y

  int disp = this->get_disparity(x,y);

  if (disp > 0-999){
    return x+disp;
  }

  // looks like no valid disparity

  return 0-1000;
}
