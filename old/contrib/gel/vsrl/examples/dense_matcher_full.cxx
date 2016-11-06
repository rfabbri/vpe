// This is gel/vsrl/examples/dense_matcher_full.cxx
//:
// \file
// \brief Dense matcher

#include <vcl_iostream.h>
#include <vsrl/vsrl_stereo_dense_matcher.h>
#include <vsrl/vsrl_parameters.h>

#include <vil1/vil1_image.h>
#include <vil1/vil1_load.h>

int main(int argc, char ** argv)
{
  if (argc<2) {
    vcl_cerr << argv[0] << " im1 im2 disparity_file [params_file]\n";
    return 0;
  }

  // create the stereo dense matcher
  vil1_image image1 = vil1_load(argv[1]);
  if (!image1) {
    vcl_cerr << "Error loading " << argv[1] << ". Exiting.\n";
    return 0;
  }
  vil1_image image2 = vil1_load(argv[2]);
  if (!image2) {
    vcl_cerr << "Error loading " << argv[2] << ". Exiting.\n";
    return 0;
  }

  // set the parameters of the dense matcher
  vsrl_parameters *params = vsrl_parameters::instance();

  if (argc>=5) {
    // we want to set some parameters based from a data file
    params->load(argv[4]);
  }

  vsrl_stereo_dense_matcher matcher(image1,image2);

  // set the correlation range
  matcher.set_correlation_range(params->correlation_range);

  // perform a full dynamic program
  matcher.execute();

  // save the disparity results
  matcher.write_disparity_image(argv[3]);

  return 0;
}
