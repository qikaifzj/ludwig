/*****************************************************************************
 *
 *  field_grad.c
 *
 *  Gradients (not just "grad" in the mathematical sense) of a field.
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2012-2016 The University of Edinburgh
 *
 *  Contributing authors:
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *  Alan Gray (alang@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#include <assert.h>
#include <stdlib.h>

#include "pe.h"
#include "coords.h"
#include "leesedwards.h"
#include "field_s.h"
#include "field_grad_s.h"
#include "targetDP.h"

static int field_grad_init(field_grad_t * obj);

/*****************************************************************************
 *
 *  field_grad_create
 *
 *****************************************************************************/

int field_grad_create(field_t * f, int level, field_grad_t ** pobj) {

  field_grad_t * obj =  (field_grad_t*) NULL;

  assert(f);
  assert(pobj);

  obj = (field_grad_t*) calloc(1, sizeof(field_grad_t));
  if (obj == NULL) fatal("calloc(field_grad_t) failed\n");

  obj->field = f;
  obj->level = level;

  field_nf(f, &obj->nf);
  assert(obj->nf > 0);

  field_grad_init(obj);
  *pobj = obj;

  return 0;
}

/*****************************************************************************
 *
 *  field_grad_init
 *
 *****************************************************************************/

static int field_grad_init(field_grad_t * obj) {

  int nsites;

  assert(obj);

  nsites = le_nsites();

  if (obj->level >= 2) {
    obj->grad = (double*) calloc(NVECTOR*obj->nf*nsites, sizeof(double));
    obj->delsq = (double*) calloc(obj->nf*nsites, sizeof(double));
    if (obj->grad == NULL) fatal("calloc(field_grad->grad) failed");
    if (obj->delsq == NULL) fatal("calloc(field_grad->delsq) failed");

 
    targetMalloc((void**) &(obj->tcopy),sizeof(field_grad_t));

    //allocate data space on target 
    double* tmpptr;
    field_grad_t* t_obj = obj->tcopy;

    targetCalloc((void**) &tmpptr,obj->nf*NVECTOR*nsites*sizeof(double));
    copyToTarget(&(t_obj->grad),&tmpptr,sizeof(double*)); 

    obj->t_grad=tmpptr; //DEPRECATED direct access to target data.
    

    targetCalloc((void**) &tmpptr,obj->nf*nsites*sizeof(double));
    copyToTarget(&(t_obj->delsq),&tmpptr,sizeof(double*)); 

    obj->t_delsq=tmpptr; //DEPRECATED direct access to target data.

    copyToTarget(&(t_obj->nf),&(obj->nf),sizeof(int)); //integer with number of field components

  }

  if (obj->level == 3) {
    obj->d_ab = (double*) calloc(NSYMM*obj->nf*nsites, sizeof(double));
    if (obj->d_ab == NULL) fatal("calloc(fieldgrad->d_ab) failed\n");
  }

  if (obj->level >= 4) {
    obj->grad_delsq = (double*) calloc(NVECTOR*obj->nf*nsites, sizeof(double));
    obj->delsq_delsq = (double*) calloc(obj->nf*nsites, sizeof(double));
    if (obj->grad_delsq == NULL) fatal("calloc(grad->grad_delsq) failed");
    if (obj->delsq_delsq == NULL) fatal("calloc(grad->delsq_delsq) failed");
  }

  return 0;
}

/*****************************************************************************
 *
 *  field_grad_set
 *
 *****************************************************************************/

int field_grad_set(field_grad_t * obj, grad_ft f2, grad_ft f4) {

  assert(obj);

  obj->d2 = f2;
  obj->d4 = f4;

  return 0;
}

/*****************************************************************************
 *
 *  field_grad_dab_set
 *
 *  The function may be NULL.
 *
 *****************************************************************************/

int field_grad_dab_set(field_grad_t * obj, dab_ft fdab) {

  assert(obj);

  obj->dab = fdab;

  return 0;
}

/*****************************************************************************
 *
 *  field_grad_free
 *
 *  The caller is responsible for releasing the resources associated
 *  with the field itself obj->field.
 *
 *****************************************************************************/

void field_grad_free(field_grad_t * obj) {

  assert(obj);

  if (obj->grad) free(obj->grad);
  if (obj->delsq) free(obj->delsq);

  if (obj->tcopy) {

    //free data space on target 
    double* tmpptr;
    field_grad_t* t_obj = obj->tcopy;
    
    copyFromTarget(&tmpptr,&(t_obj->grad),sizeof(double*)); 
    targetFree(tmpptr);
    
    copyFromTarget(&tmpptr,&(t_obj->delsq),sizeof(double*)); 
    targetFree(tmpptr);
   
    //free target copy of structure 
    targetFree(obj->tcopy);
  }


  if (obj->grad_delsq) free(obj->grad_delsq);
  if (obj->delsq_delsq) free(obj->delsq_delsq);
  if (obj->d_ab) free(obj->d_ab);

  obj->field = NULL;
  free(obj);

  return;
}

/*****************************************************************************
 *
 *  field_grad_compute
 *
 *****************************************************************************/

int field_grad_compute(field_grad_t * obj) {

  assert(obj);
  assert(obj->d2);

  field_leesedwards(obj->field);


  //  obj->d2(obj->field->nf, obj->field->data, obj->grad, obj->delsq);

  obj->d2(obj->field->nf, obj->field->data,obj->field->t_data, obj->grad, obj->t_grad, obj->delsq, obj->t_delsq);

  if (obj->level == 3) {
    assert(obj->dab);
    obj->dab(obj->field->nf, obj->field->data, obj->d_ab);
  }

  if (obj->level >= 4) {
    assert(obj->d4);
    // obj->d4(obj->field->nf, obj->delsq, obj->grad_delsq, obj->delsq_delsq);
    //obj->d4(obj->field->nf, obj->field->data,obj->field->t_data, obj->grad, obj->t_grad, obj->delsq, obj->t_delsq, obj->field->siteMask, obj->field->t_siteMask);

    //TO DO TEMPORARY FIX to allow free energies which require del^4 phi - 
    // we are just using the existing t_* data structures 
    // for the higher order host structures - this needs properly sorted

    obj->d4(obj->field->nf, obj->delsq,obj->field->t_data,obj->grad_delsq, obj->t_grad, obj->delsq_delsq, obj->t_delsq);
    
    //void* dummy;
    //obj->d4(obj->field->nf, obj->delsq,dummy,obj->grad_delsq, dummy, obj->delsq_delsq, dummy, obj->field->siteMask, obj->field->t_siteMask);

  }

  return 0;
}

/*****************************************************************************
 *
 *  field_grad_scalar_grad
 *
 *****************************************************************************/

int field_grad_scalar_grad(field_grad_t * obj, int index, double grad[3]) {

  int ia;

  assert(obj);
  assert(obj->nf == 1);
  assert(grad);
#ifndef OLD_SHIT
  for (ia = 0; ia < NVECTOR; ia++) {
    grad[ia] = obj->grad[addr_rank1(le_nsites(), NVECTOR, index, ia)];
  }
#else
  for (ia = 0; ia < 3; ia++) {
    grad[ia] = obj->grad[NVECTOR*index + ia];
  }
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_scalar_delsq
 *
 *****************************************************************************/

int field_grad_scalar_delsq(field_grad_t * obj, int index, double * delsq) {

  assert(obj);
  assert(obj->nf == 1);
  assert(delsq);

#ifndef OLD_SHIT
  *delsq = obj->delsq[addr_rank1(le_nsites(), 1, index, 0)];
#else
  *delsq = obj->delsq[index];
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_scalar_grad_delsq
 *
 *****************************************************************************/

int field_grad_scalar_grad_delsq(field_grad_t * obj, int index,
				 double grad[3]) {
  int ia;

  assert(obj);
  assert(obj->nf == 1);
  assert(grad);

#ifndef OLD_SHIT
  for (ia = 0; ia < NVECTOR; ia++) {
    grad[ia] = obj->grad_delsq[addr_rank1(le_nsites(), NVECTOR, index, ia)];
  }
#else
  for (ia = 0; ia < 3; ia++) {
    grad[ia] = obj->grad_delsq[NVECTOR*index + ia];
  }
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_scalar_delsq_delsq
 *
 *****************************************************************************/

int field_grad_scalar_delsq_delsq(field_grad_t * obj, int index, double * dd) {

  assert(obj);
  assert(obj->nf == 1);
  assert(dd);

#ifndef OLD_SHIT
  *dd = obj->delsq_delsq[addr_rank1(le_nsites(), 1, index, 0)];
#else
  *dd = obj->delsq_delsq[index];
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_scalar_dab
 *
 *  Return tensor d_a d_b for scalar field
 *
 *****************************************************************************/

int field_grad_scalar_dab(field_grad_t * obj, int index, double dab[3][3]) {

  int nsites;

  assert(obj);
  assert(obj->nf == 1);

  nsites = coords_nsites();

#ifndef OLD_SHIT
  dab[X][X] = obj->d_ab[addr_dab(nsites, index, XX)];
  dab[X][Y] = obj->d_ab[addr_dab(nsites, index, XY)];
  dab[X][Z] = obj->d_ab[addr_dab(nsites, index, XZ)];
  dab[Y][X] = dab[X][Y];
  dab[Y][Y] = obj->d_ab[addr_dab(nsites, index, YY)];
  dab[Y][Z] = obj->d_ab[addr_dab(nsites, index, YZ)];
  dab[Z][X] = dab[X][Z];
  dab[Z][Y] = dab[Y][Z];
  dab[Z][Z] = obj->d_ab[addr_dab(nsites, index, ZZ)];
#else
  dab[X][X] = obj->d_ab[FLDADR(nsites,NSYMM,index,XX)];
  dab[X][Y] = obj->d_ab[FLDADR(nsites,NSYMM,index,XY)];
  dab[X][Z] = obj->d_ab[FLDADR(nsites,NSYMM,index,XZ)];
  dab[Y][X] = dab[X][Y];
  dab[Y][Y] = obj->d_ab[FLDADR(nsites,NSYMM,index,YY)];
  dab[Y][Z] = obj->d_ab[FLDADR(nsites,NSYMM,index,YZ)];
  dab[Z][X] = dab[X][Z];
  dab[Z][Y] = dab[Y][Z];
  dab[Z][Z] = obj->d_ab[FLDADR(nsites,NSYMM,index,ZZ)];
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_vector_grad
 *
 *  Return the gradient tensor for vector field dp[ia][ib] = d_a p_b.
 *
 *****************************************************************************/

int field_grad_vector_grad(field_grad_t * obj, int index, double dp[3][3]) {

  int ia, ib;

  assert(obj);
  assert(obj->nf == NVECTOR);
  assert(dp);

#ifndef OLD_SHIT
  for (ia = 0; ia < NVECTOR; ia++) {
    for (ib = 0; ib < obj->nf; ib++) {
      dp[ia][ib] = obj->grad[addr_rank2(coords_nsites(), obj->nf, NVECTOR, index, ib, ia)];
    }
  }
#else
  for (ia = 0; ia < 3; ia++) {
    for (ib = 0; ib < 3; ib++) {
      dp[ia][ib] = obj->grad[NVECTOR*(obj->nf*index + ib) + ia];
    }
  }
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_vector_delsq
 *
 *****************************************************************************/

int field_grad_vector_delsq(field_grad_t * obj, int index, double delsq[3]) {

  int ia;

  assert(obj);
  assert(obj->nf == NVECTOR);
  assert(delsq);

#ifndef OLD_SHIT
  for (ia = 0; ia < NVECTOR; ia++) {
    delsq[ia] = obj->delsq[addr_rank1(coords_nsites(), NVECTOR, index, ia)];
  }
#else
  for (ia = 0; ia < 3; ia++) {
    delsq[ia] = obj->delsq[NVECTOR*index + ia];
  }
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_tensor_grad
 *
 *  Expand and return the rank 3 gradient of rank 2 tensor q.
 *
 *****************************************************************************/

int field_grad_tensor_grad(field_grad_t * obj, int index, double dq[3][3][3]) {

  int ia;
  int nsites;

  assert(obj);
  assert(obj->nf == NQAB);
  assert(dq);

  nsites = coords_nsites();

#ifndef OLD_SHIT
  for (ia = 0; ia < NVECTOR; ia++) {
    dq[ia][X][X] = obj->grad[addr_rank2(nsites, NQAB, 3, index, XX, ia)];
    dq[ia][X][Y] = obj->grad[addr_rank2(nsites, NQAB, 3, index, XY, ia)];
    dq[ia][X][Z] = obj->grad[addr_rank2(nsites, NQAB, 3, index, XZ, ia)];
    dq[ia][Y][X] = dq[ia][X][Y];
    dq[ia][Y][Y] = obj->grad[addr_rank2(nsites, NQAB, 3, index, YY, ia)];
    dq[ia][Y][Z] = obj->grad[addr_rank2(nsites, NQAB, 3, index, YZ, ia)];
    dq[ia][Z][X] = dq[ia][X][Z];
    dq[ia][Z][Y] = dq[ia][Y][Z];
    dq[ia][Z][Z] = 0.0 - dq[ia][X][X] - dq[ia][Y][Y];
  }
#else
  for (ia = 0; ia < NVECTOR; ia++) {
    dq[ia][X][X] = obj->grad[FGRDADR(nsites,NQAB,index,XX,ia)];
    dq[ia][X][Y] = obj->grad[FGRDADR(nsites,NQAB,index,XY,ia)];
    dq[ia][X][Z] = obj->grad[FGRDADR(nsites,NQAB,index,XZ,ia)];
    dq[ia][Y][X] = dq[ia][X][Y];
    dq[ia][Y][Y] = obj->grad[FGRDADR(nsites,NQAB,index,YY,ia)];
    dq[ia][Y][Z] = obj->grad[FGRDADR(nsites,NQAB,index,YZ,ia)];
    dq[ia][Z][X] = dq[ia][X][Z];
    dq[ia][Z][Y] = dq[ia][Y][Z];
    dq[ia][Z][Z] = 0.0 - dq[ia][X][X] - dq[ia][Y][Y];
  }
#endif
  return 0;
}

/*****************************************************************************
 *
 *  field_grad_tensor_delsq
 *
 *  Expand and return Laplacian for tensor field.
 *
 *****************************************************************************/

int field_grad_tensor_delsq(field_grad_t * obj, int index, double dsq[3][3]) {

  int nsites;

  assert(obj);
  assert(obj->nf == NQAB);
  assert(dsq);

  nsites = coords_nsites();

#ifndef OLD_SHIT
  dsq[X][X] = obj->delsq[addr_rank1(nsites, NQAB, index, XX)];
  dsq[X][Y] = obj->delsq[addr_rank1(nsites, NQAB, index, XY)];
  dsq[X][Z] = obj->delsq[addr_rank1(nsites, NQAB, index, XZ)];
  dsq[Y][X] = dsq[X][Y];
  dsq[Y][Y] = obj->delsq[addr_rank1(nsites, NQAB, index, YY)];
  dsq[Y][Z] = obj->delsq[addr_rank1(nsites, NQAB, index, YZ)];
  dsq[Z][X] = dsq[X][Z];
  dsq[Z][Y] = dsq[Y][Z];
  dsq[Z][Z] = 0.0 - dsq[X][X] - dsq[Y][Y];
#else
  dsq[X][X] = obj->delsq[FLDADR(nsites,NQAB,index,XX)];
  dsq[X][Y] = obj->delsq[FLDADR(nsites,NQAB,index,XY)];
  dsq[X][Z] = obj->delsq[FLDADR(nsites,NQAB,index,XZ)];
  dsq[Y][X] = dsq[X][Y];
  dsq[Y][Y] = obj->delsq[FLDADR(nsites,NQAB,index,YY)];
  dsq[Y][Z] = obj->delsq[FLDADR(nsites,NQAB,index,YZ)];
  dsq[Z][X] = dsq[X][Z];
  dsq[Z][Y] = dsq[Y][Z];
  dsq[Z][Z] = 0.0 - dsq[X][X] - dsq[Y][Y];
#endif
  return 0;
}
