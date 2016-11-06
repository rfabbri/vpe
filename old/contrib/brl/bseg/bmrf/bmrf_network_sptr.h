// This is brl/bseg/bmrf/bmrf_network_sptr.h
#ifndef bmrf_network_sptr_h_
#define bmrf_network_sptr_h_
//:
// \file
// \brief A smart pointer to a bmrf_network.
// \author Matt Leotta, (mleotta@lems.brown.edu)
// \date January 16, 2004
//
// Include this file to use the smart pointer bmrf_network_sptr

class bmrf_network;

#include <vbl/vbl_smart_ptr.h>

typedef vbl_smart_ptr<bmrf_network> bmrf_network_sptr;

#endif // bmrf_network_sptr_h_
