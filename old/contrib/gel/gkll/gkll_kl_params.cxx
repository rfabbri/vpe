// This is gel/gkll/gkll_kl_params.cxx
#include "gkll_kl_params.h"

gkll_kl_params::gkll_kl_params()
{
    set_defaults();
}

gkll_kl_params::~gkll_kl_params()
{
}

gkll_kl_params::gkll_kl_params(const gkll_kl_params & params)
{
    // A faire
    numpoints =params.numpoints;
    search_range = params.search_range ;

    mindist = 10;
    window_width = 7;
    window_height = 7;
    min_eigenvalue = 1;
    min_determinant = (float)0.01;
    min_displacement = (float)0.1;
    max_iterations = 10;
    max_residue = 10.0;
    grad_sigma = 1.0;
    smooth_sigma_fact = (float)0.1;
    pyramid_sigma_fact = (float)0.9;
    replaceLostPoints = true;
    sequentialMode = false;
    smoothBeforeSelecting = true;
    writeInternalImages = false;
    nSkippedPixels = 0;
    verbose = true;
}


void gkll_kl_params::set_defaults()
{
  numpoints = 100;
  search_range = 15;

  mindist = 10;
  window_width = 7;
  window_height = 7;
  min_eigenvalue = 1;
  min_determinant = (float)0.01;
  min_displacement = (float)0.1;
  max_iterations = 10;
  max_residue = 10.0;
  grad_sigma = 1.0;
  smooth_sigma_fact = (float)0.1;
  pyramid_sigma_fact = (float)0.9;
  replaceLostPoints = true;
  sequentialMode = false;
  smoothBeforeSelecting = true;
  writeInternalImages = false;
  nSkippedPixels = 0;
  verbose = true;
}
