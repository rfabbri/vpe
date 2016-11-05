#include <bvxm/grid/bvxm_voxel_slab.hxx>
#include <bsta/bsta_attributes.h>
#include <bsta/bsta_gauss_sd3.h>
#include <bsta/io/bsta_io_attributes.h>
#include <bsta/io/bsta_io_gaussian_sphere.h>

typedef bsta_vsum_num_obs<bsta_gauss_sd3> gauss_type;
BVXM_VOXEL_SLAB_INSTANTIATE(gauss_type);
