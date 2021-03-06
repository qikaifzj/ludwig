/****************************************************************************
 *
 *  lc_droplet_rt.h
 *
 *  Run time initiliasation for the liquid crystal droplet free energy
 *
 *  Edinburgh Soft Matter and Statistical Physics Group
 *  and Edinburgh Parallel Computing Centre
 *
 *  (c) 2012-2016 The University of Edinburgh
 *
 ****************************************************************************/

#ifndef LC_DROPLET_RT_H
#define LC_DROPLET_RT_H

#include "pe.h"
#include "runtime.h"
#include "lc_droplet.h"

int fe_lc_droplet_run_time(pe_t * pe, rt_t * rt, fe_lc_droplet_t * fe);

#endif
