/*****************************************************************************
 *
 *  psi_rt.h
 *
 *  $Id$
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *  (c) 2012 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef PSI_RT_H
#define PSI_RT_H

#include "psi.h"
#include "map.h"

int psi_init_param_rt(psi_t * psi);
int psi_init_rho_rt(psi_t * psi, map_t * map);

#endif