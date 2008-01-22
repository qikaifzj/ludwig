/***************************************************************************
 *
 *  lattice.h
 *
 *  $Id: lattice.h,v 1.5 2006-12-20 16:56:57 kevin Exp $
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 ***************************************************************************/

#ifndef _LATTICE_H_
#define _LATTICE_H_

enum lattice_type { FLUID, SOLID, COLLOID, BOUNDARY };

void LATT_allocate_force(const int);
void LATT_allocate_phi(const int);
void latt_allocate_velocity(const int);
void set_force_at_site(const int, double []);
void add_force_at_site(const int, double []);
void get_velocity_at_lattice(const int, double []);

struct vector {double c[3];};

#endif /* _LATTICE_H_ */
