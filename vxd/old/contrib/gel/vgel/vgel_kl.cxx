// This is gel/vgel/vgel_kl.cxx
#include "vgel_kl.h"

#include <vxl_config.h>
#include <vil1/vil1_pixel.h>
#include <vil1/vil1_memory_image_of.h>
#include <vil1/vil1_image_as.h>
#include <vil/vil_image_view.h>
#include <vil/vil_image_resource.h>
#include <vil/vil_convert.h>
#include <vgel/vgel_multi_view_data.h>
#include <vtol/vtol_vertex_2d.h>
//#include <vidl_vil1/vidl_vil1_frame.h>
//#include <vidl_vil1/vidl_vil1_movie.h>

#include <vcl_deprecated.h>
#include <vcl_iostream.h>
#include <vcl_cassert.h>

vgel_kl::vgel_kl(const vgel_kl_params & params) : params_(params)
{
    prev_frame_ = NULL;
    seq_tc_ = NULL;
    fl_ = NULL;
}

vgel_kl::~vgel_kl()
{
    reset_prev_frame();
}

void vgel_kl::reset_prev_frame()
{
    if (prev_frame_)
    {
        delete prev_frame_;
        prev_frame_ = NULL;
    }
    if (seq_tc_)
    {
        KLTFreeTrackingContext(seq_tc_);
        seq_tc_ = NULL;
    }
    if (fl_)
    {
        KLTFreeFeatureList(fl_);
        fl_ = NULL;
    }
}

void vgel_kl::match_sequence(
    vil1_image&                      prev_img,
    vil1_image&                      cur_img,
    vgel_multi_view_data_vertex_sptr matches,
    bool                             use_persistent_features)
{
  VXL_DEPRECATED( "vgel_kl::match_sequence(vil1_image&, vil1_image&, vgel_multi_view_data_vertex_sptr,  bool)" );

  // Get the current image width & height.  (The KLT code
  // assumes these are constant for the duration of a
  // continuous tracking sequence!)
  int width = cur_img.width();
  int height = cur_img.height();
  KLT_PixelType * prev_img_gs = NULL;
  KLT_PixelType * cur_img_gs = convert_to_gs_image(cur_img);
  if (!prev_frame_) {
      assert (width == prev_img.width());
      assert (height == prev_img.height());
      prev_img_gs = convert_to_gs_image(prev_img);
  }
  match_sequence_base (prev_img_gs, cur_img_gs, width, height, matches, use_persistent_features);
}

void vgel_kl::match_sequence(
    vil_image_resource_sptr&         prev_img,
    vil_image_resource_sptr&         cur_img,
    vgel_multi_view_data_vertex_sptr matches,
    bool                             use_persistent_features)
{
  int width = (*cur_img).ni();
  int height = (*cur_img).nj();
  KLT_PixelType * prev_img_gs = NULL;
  KLT_PixelType * cur_img_gs = convert_to_gs_image(cur_img);
  if (!prev_frame_) {
      assert (width == int((*prev_img).ni()));
      assert (height == int((*prev_img).nj()));
      prev_img_gs = convert_to_gs_image(prev_img);
  }
  match_sequence_base (prev_img_gs, cur_img_gs, width, height, matches, use_persistent_features);
}

void vgel_kl::match_sequence(
    vil_image_view<vxl_byte>*        prev_img,
    vil_image_view<vxl_byte>*        cur_img,
    vgel_multi_view_data_vertex_sptr matches,
    bool                             use_persistent_features)
{
  int width = (*cur_img).ni();
  int height = (*cur_img).nj();
  KLT_PixelType * prev_img_gs = NULL;
  KLT_PixelType * cur_img_gs = convert_to_gs_image(cur_img);
  if (!prev_frame_) {
      assert (width == int((*prev_img).ni()));
      assert (height == int((*prev_img).nj()));
      prev_img_gs = convert_to_gs_image(prev_img);
  }
  match_sequence_base (prev_img_gs, cur_img_gs, width, height, matches, use_persistent_features);
}

void vgel_kl::match_sequence_base(
    KLT_PixelType *                  prev_img_gs,
    KLT_PixelType *                  cur_img_gs,
    int                              width,
    int                              height,
    vgel_multi_view_data_vertex_sptr matches,
    bool                             use_persistent_features)
{
    // Get the current image as a linear buffer
    KLT_PixelType*    cur_frame = cur_img_gs;

    if (use_persistent_features)
    {
        // If there's no feature list yet...
        if (!fl_)
        {
            // Allocate a feature list for the frame
            fl_ = KLTCreateFeatureList(params_.numpoints);
        }
    }
    else
    {
        // Allocate a feature list for the frame
        fl_ = KLTCreateFeatureList(params_.numpoints);
    }

    // If there's no previous frame yet...
    if (!prev_frame_)
    {
        // Get the previous image as a linear buffer
        prev_frame_ = prev_img_gs;

        // Create & initialize a tracking context for the sequence
        if (seq_tc_ != NULL)
        {
            KLTFreeTrackingContext(seq_tc_);
        }
        seq_tc_ = KLTCreateTrackingContext();
        set_tracking_context(seq_tc_);
        seq_tc_->sequentialMode = TRUE;

        if (use_persistent_features)
        {
            // Get some features from the first image
            KLTSelectGoodFeatures(seq_tc_, prev_frame_, width, height, fl_);
        }
    }

    if (!use_persistent_features)
    {
        KLTReplaceLostFeatures(seq_tc_, prev_frame_, width, height, fl_);
    }

    // Create a feature table for the previous & current frames
    KLT_FeatureTable    ft = KLTCreateFeatureTable(2, params_.numpoints);

    // Store the previous frame's feature list
    KLTStoreFeatureList(fl_, ft, 0);

    // Track the points
    KLTTrackFeatures(seq_tc_, prev_frame_, cur_frame, width, height, fl_);

    if (use_persistent_features)
    {
        // Restore lost features
        if (params_.replaceLostPoints)
        {
            KLTReplaceLostFeatures(seq_tc_, cur_frame, width, height, fl_);
        }
    }

    // Store the current frame's feature list
    KLTStoreFeatureList(fl_, ft, 1);

    if (matches)
    {
        // Go through the feature table and extract the matches
        matches_from_feature_table(ft, matches);
    }

    // We're done with the feature table
    KLTFreeFeatureTable(ft);

    if (use_persistent_features)
    {
        // Reset the val fields on the features to a positive value
        // for the next call to matches_from_feature_table()
        for (int pointnum = 0; pointnum < fl_->nFeatures; pointnum++)
        {
            fl_->feature[pointnum]->val = 1;
        }
    }
    else
    {
        // We're done with the feature list
        KLTFreeFeatureList(fl_);
        fl_ = NULL;
    }

    // We're done with the previous frame data -- save
    // current frame data as previous data for next frame
    delete prev_frame_;
    prev_frame_ = cur_frame;
}

void vgel_kl::match_sequence(
    vcl_vector<vil1_image> &         image_list,
    vgel_multi_view_data_vertex_sptr matches)
{
  VXL_DEPRECATED( "vgel_kl::match_sequence(vcl_vector<vil1_image> &, vgel_multi_view_data_vertex_sptr)" );

  vcl_vector<KLT_PixelType *> image_list_gs(image_list.size());
  int width = image_list[0].width();
  int height = image_list[0].height();
  for (unsigned n=0; n<image_list.size(); n++) {
      assert (width == image_list[n].width());
      assert (height == image_list[n].height());
      image_list_gs[n] = convert_to_gs_image(image_list[n]);
  }
  match_sequence_base (image_list_gs, width, height, matches);
}

void vgel_kl::match_sequence(
    vcl_vector<vil_image_resource_sptr> & image_list,
    vgel_multi_view_data_vertex_sptr      matches)
{
  vcl_vector<KLT_PixelType *> image_list_gs(image_list.size());
  int width = (*image_list[0]).ni();
  int height = (*image_list[0]).nj();
  for (unsigned n=0; n<image_list.size(); n++) {
      assert (width == int((*image_list[1]).ni()));
      assert (height == int((*image_list[1]).nj()));
      image_list_gs[n] = convert_to_gs_image(image_list[n]);
  }
  match_sequence_base (image_list_gs, width, height, matches);
}

void vgel_kl::match_sequence_base(
    vcl_vector<KLT_PixelType *> &    image_list_gs,
    int                              width,
    int                              height,
    vgel_multi_view_data_vertex_sptr matches)
{
  // Uses the KL tracker to track points through an image
  int nFeatures = params_.numpoints;
  int nFrames = image_list_gs.size();

  // If there are no frames in this movie, then skip
  if (nFrames < 1) return;

  // Set up the context
  KLT_TrackingContext tc = KLTCreateTrackingContext();
  KLT_FeatureList     fl = KLTCreateFeatureList(nFeatures);
  KLT_FeatureTable    ft = KLTCreateFeatureTable(nFrames, nFeatures);

  // Apply the defaults
  set_tracking_context (tc);
  tc->sequentialMode = TRUE;

  // Now, get the imagery into a linear buffer
  KLT_PixelType* img1=image_list_gs[0];

  // Get some features from the first image
  KLTSelectGoodFeatures(tc, img1, width, height, fl);
  KLTStoreFeatureList(fl, ft, 0);

  for (int i=1; i<nFrames; i++)
  {
    KLT_PixelType* img2=image_list_gs[i];

    // Track the points
    KLTTrackFeatures(tc, img1, img2, width, height, fl);

    // Restore lost features
    if (params_.replaceLostPoints)
      KLTReplaceLostFeatures(tc, img2, width, height, fl);

    // Store the values
    KLTStoreFeatureList(fl, ft, i);

    // MPP 9/24/2003
    // We're done with the image -- plug memory leak!
    delete img2;
  }

  // MPP 9/24/2003
  // We're done with the image & tracking context -- plug memory leaks!
  delete img1;
  KLTFreeTrackingContext(tc);

  // Go through the feature table and extract the matches
  matches_from_feature_table(ft, matches);

  // MPP 9/24/2003
  // We're done with the feature list & feature table -- plug memory leaks!
  KLTFreeFeatureList(fl);
  KLTFreeFeatureTable(ft);
}

void vgel_kl::matches_from_feature_table(KLT_FeatureTable                 ft,
                                         vgel_multi_view_data_vertex_sptr matches)
{
    int matchnum = -1;
    int pointnum;
    int viewnum;

    for (pointnum = 0; pointnum < ft->nFeatures; pointnum++)
    {
        for (viewnum = 0; viewnum < ft->nFrames; viewnum++)
        {
            // Get the current feature
            KLT_Feature feat = ft->feature[pointnum][viewnum];

            // Get the components of this feature
            float   x = feat->x;
            float   y = feat->y;

//            printf("ft->feature[%03d][%d] = (%09.6f %09.6f %-05d)\n",
//                    pointnum, viewnum, x, y, feat->val);

            // Test to see if this is the continuation of a sequence
            // - then put in the table
            if (feat->val == 0)
            {
                vtol_vertex_2d_sptr vertex = new vtol_vertex_2d(x, y);
//                printf("\tmatches->set1(%d, %-04d, (%9.6f, %9.6f))\n",
//                       viewnum, matchnum, x, y);
                matches->set(viewnum, matchnum, vertex);
            }
            // Otherwise, this is the start of a sequence
            else if (feat->val > 0)
            {
                // Must test to see if the next value is zero
                // - otherwise this is a 1-frame sequence
                if ((viewnum < ft->nFrames - 1) &&
                    (ft->feature[pointnum][viewnum + 1]->val == 0))
                {
                    // This is a new match
                    matchnum++;

                    // Store it
                    vtol_vertex_2d_sptr vertex = new vtol_vertex_2d(x, y);
//                    printf("\tmatches->set2(%d, %-04d, (%9.6f, %9.6f))\n",
//                           viewnum, matchnum, x, y);
                    matches->set(viewnum, matchnum, vertex);
                }
            }
        }
    }

#if 0
    // Finally, renumber the matches
    matches->renumber();
#endif
}

// Removed until old vidl dependency is resolved.
//void vgel_kl::match_sequence(vidl_vil1_movie_sptr             movie,
//                             vgel_multi_view_data_vertex_sptr matches)
//{
//  VXL_DEPRECATED( "vgel_kl::match_sequence(vidl_vil1_movie_sptr, vgel_multi_view_data_vertex_sptr)" );
//
//  vcl_vector<vil1_image> image_list;
//  for (vidl_vil1_movie::frame_iterator pframe = movie->first();
//       pframe <= movie->last();
//       ++pframe)
//  {
//    vil1_image im = vil1_image(pframe->get_image());
//    image_list.push_back(im);
//  }
//  match_sequence(image_list,matches);
//}

vcl_vector<vtol_vertex_2d_sptr>* vgel_kl::extract_points(vil1_image & image)
{
  VXL_DEPRECATED( "vgel_kl::extract_points(vil1_image &)" );

  int width=image.width();
  int height=image.height();
  KLT_PixelType* image_gs=convert_to_gs_image(image);
  return extract_points_base (image_gs, width, height);
}

vcl_vector<vtol_vertex_2d_sptr>* vgel_kl::extract_points(vil_image_resource_sptr & image)
{
  int width=(*image).ni();
  int height=(*image).nj();
  KLT_PixelType* image_gs=convert_to_gs_image(image);
  return extract_points_base (image_gs, width, height);
}

vcl_vector<vtol_vertex_2d_sptr>* vgel_kl::extract_points_base(KLT_PixelType * image_gs, int width, int height)
{
  vcl_cerr << "Beginning points extraction\n";

  KLT_PixelType* img1=image_gs;

  // Now, run the extractor
  int nFeatures = params_.numpoints;

  vcl_cerr << "Setting up the context...\n";
  // Set up the context
  KLT_TrackingContext tc = KLTCreateTrackingContext();

  // Set the default values
  set_tracking_context (tc);

  // KLTPrintTrackingContext(tc);

  // Set up structure to hold the features.
  vcl_cerr << "Setting up structure to hold the features...\n";
  KLT_FeatureList fl = KLTCreateFeatureList(nFeatures);

  // Extract the features
  vcl_cerr << "Extracting the features...\n";
  KLTSelectGoodFeatures(tc, img1, width, height, fl);

  // Make an IUPointGroup to hold the values
  vcl_vector<vtol_vertex_2d_sptr> *grp = new vcl_vector<vtol_vertex_2d_sptr>();

  for (int i=0 ; i< fl->nFeatures ; i++)
  {
    // Change the point into an IUPoint -- offset by ilow, jlow
    float x = fl->feature[i]->x;
    float y = fl->feature[i]->y;
    //HomgPoint2D *point = new HomgPoint2D(x, y,1.0)
    vtol_vertex_2d_sptr point=new vtol_vertex_2d(x,y);
    // Put the point in the backup list
    grp->push_back(point);
  }

  // !!
  // We probably need to delete the feature list

  // Return the group
  return grp;
}

KLT_PixelType* vgel_kl::convert_to_gs_image(vil1_image &image)
{
  // this method is private, so no need to mark it deprecated

  if (vil1_pixel_format(image)==VIL1_RGB_BYTE)
  {
    vcl_cerr << "Converting image to grey scale...\n";

    int w=image.width();
    int h=image.height();
    KLT_PixelType* tab_mono=new KLT_PixelType[w*h];
    vcl_cerr << "width: " <<w<< "  height: "<<h<< "  pixel type: byte\n";

    vil1_memory_image_of<vxl_byte> ima_mono;
    ima_mono.resize(w,h);

    vil1_image_as_byte(image).get_section(ima_mono.get_buffer(), 0, 0, w, h);
    vxl_byte* p=ima_mono.get_buffer();

    for (int j=0;j<h;j++)
      for (int i=0;i<w;i++)
        tab_mono[j*w+i]=(KLT_PixelType)p[j*w+i];
    return tab_mono;
  }
  else if (vil1_pixel_format(image)==VIL1_BYTE)
  {
    int w=image.width();
    int h=image.height();
    KLT_PixelType* tab_mono=new KLT_PixelType[w*h];
    vcl_cerr << "width: " <<w<< "  height: "<<h<< "  pixel type: byte\n";

    vil1_memory_image_of<vxl_byte> ima_mono;
    ima_mono.resize(w,h);

    vil1_image_as_byte(image).get_section(ima_mono.get_buffer(), 0, 0, w, h);
    vxl_byte* p=ima_mono.get_buffer();

    for (int j=0;j<h;j++)
      for (int i=0;i<w;i++)
        tab_mono[j*w+i]=(KLT_PixelType)p[j*w+i];

    return tab_mono;
  }
  else if (vil1_pixel_format(image)==VIL1_UINT16)
  {
    int w=image.width();
    int h=image.height();
    KLT_PixelType* tab_mono=new KLT_PixelType[w*h];
    vcl_cerr << "width: " <<w<< "  height: "<<h<< "  pixel type: uint_16\n";

    vil1_memory_image_of<vxl_uint_16> ima_mono;
    ima_mono.resize(w,h);

    vil1_image_as_uint16(image).get_section(ima_mono.get_buffer(), 0, 0, w, h);
    vxl_uint_16* p=ima_mono.get_buffer();

    for (int j=0;j<h;j++)
      for (int i=0;i<w;i++)
        tab_mono[j*w+i]=(KLT_PixelType)p[j*w+i];

    return tab_mono;
  }
  else if (vil1_pixel_format(image)==VIL1_RGB_UINT16)
  {
    int w=image.width();
    int h=image.height();
    KLT_PixelType* tab_mono=new KLT_PixelType[w*h];
    vcl_cerr << "width: " <<w<< "  height: "<<h<< "  pixel type: uint_16\n";

    vil1_memory_image_of<vxl_uint_16> ima_mono;
    ima_mono.resize(w,h);

    vil1_image_as_uint16(image).get_section(ima_mono.get_buffer(), 0, 0, w, h);
    vxl_uint_16* p=ima_mono.get_buffer();

    for (int j=0;j<h;j++)
      for (int i=0;i<w;i++)
        tab_mono[j*w+i]=(KLT_PixelType)p[j*w+i];

    return tab_mono;
  }
  else
  {
    vcl_cerr << "Error: Cannot convert pixel type: "
             << vil1_print(vil1_pixel_format(image)) << vcl_endl;
    return NULL;
  }
}

KLT_PixelType* vgel_kl::convert_to_gs_image(vil_image_resource_sptr &image)
{
  // view of the image with the right type
  vil_image_view<KLT_PixelType> imgv = vil_convert_cast (KLT_PixelType(), image->get_view());

  // view of image converted to gray
  vil_image_view<KLT_PixelType> imgg;
  // only convert to gray if we have to
  if (imgv.nplanes() == 3)
      vil_convert_planes_to_grey (imgv, imgg);
  else
      imgg = imgv;
  if (!imgg) vil_convert_cast(imgv,imgg); // if all else fails, try this.

  assert ( 1 == imgg.nplanes() );

  KLT_PixelType* tab_mono=new KLT_PixelType[ imgg.ni() * imgg.nj() ];

  // view of the image using memory organized for the underlying library
  vil_image_view<KLT_PixelType> imgc (tab_mono,
                                      imgg.ni(), imgg.nj(), 1,
                                      1, imgg.ni(), imgg.ni() * imgg.nj());

  vil_copy_deep (imgg, imgc);

  return tab_mono;
}


KLT_PixelType* vgel_kl::convert_to_gs_image(vil_image_view<vxl_byte>* image)
{
  // view of the image
  vil_image_view<vxl_byte> imgv = *image;

  // view of image converted to gray
  vil_image_view<KLT_PixelType> imgg;
  // only convert to gray if we have to
  if (imgv.nplanes() == 3)
    vil_convert_planes_to_grey (imgv, imgg);
  else
    imgg = imgv;
  if (!imgg) vil_convert_cast(imgv,imgg); // if all else fails, try this.

  assert ( 1 == imgg.nplanes() );

  KLT_PixelType* tab_mono=new KLT_PixelType[ imgg.ni() * imgg.nj() ];

  // view of the image using memory organized for the underlying library
  vil_image_view<KLT_PixelType> imgc (tab_mono,
                                      imgg.ni(), imgg.nj(), 1,
                                      1, imgg.ni(), imgg.ni() * imgg.nj());

  vil_copy_deep (imgg, imgc);

  return tab_mono;
}

void vgel_kl::set_tracking_context( KLT_TrackingContext tc)
{
  // Set values to values derived from the parameters
  tc->mindist               = params_.mindist;
  tc->window_width          = params_.window_width;
  tc->window_height         = params_.window_height;
  tc->sequentialMode        = params_.sequentialMode;
  tc->smoothBeforeSelecting = params_.smoothBeforeSelecting;
  tc->writeInternalImages   = params_.writeInternalImages;
  tc->min_eigenvalue        = params_.min_eigenvalue;
  tc->min_determinant       = params_.min_determinant;
  tc->max_iterations        = params_.max_iterations;
  tc->min_displacement      = params_.min_displacement;
  tc->max_residue           = params_.max_residue;
  tc->grad_sigma            = params_.grad_sigma;
  tc->smooth_sigma_fact     = params_.smooth_sigma_fact;
  tc->pyramid_sigma_fact    = params_.pyramid_sigma_fact;
  tc->nSkippedPixels        = params_.nSkippedPixels;

  // klt functions to complete the setup
  KLTChangeTCPyramid (tc, params_.search_range); //set nPyramidLevels and subsampling
  KLTUpdateTCBorder  (tc); //set borderx and bordery
}
