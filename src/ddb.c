/*
 * Copyright (C) 2007. See COPYRIGHT.ddb in top-level directory.
 */

#include "dalecconf.h"

/***************************************************************************
 *---
 *--- The software in this file implements three heuristics for distributing
 *--- multidimensional arrays: ddb_h1 and ddb_h2 are fastest, ddb_ex does
 *--- an exhaustive search, see below for details.
 *---
 *--- To compile this file: cc ddb.c -lm
 *---
 *--- Author: Joel Malard
 *--- Address: Pacific Northwest National Laboratory
 *---          Battelle Boulevard, PO Box 999
 *---          Richland, WA 99352
 *---
 *--- Bug et al.: jm.malard@pnl.gov
 *---
 ***************************************************************************/

#if HAVE_STDIO_H
#   include <stdio.h>
#endif
#if HAVE_STDLIB_H
#   include <stdlib.h>
#endif
#if HAVE_MATH_H
#   include <math.h>
#endif

/*--
 *****************************************************************************
 *--
 *-- void ddb_h1 and ddb_h2 implement load-balancing heuristics
 *-- void ddb_ex implements an exhaustive search
 */
void ddb(ssize_t ndims, ssize_t ardims[], ssize_t npes, ssize_t blk[],
         ssize_t pedims[]);
void ddb_ex( ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold,
             ssize_t blk[], ssize_t pedims[]);
void ddb_h1( ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold,
             ssize_t blk[], ssize_t pedims[]);
void ddb_h2( ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold,
             ssize_t bias, ssize_t blk[], ssize_t pedims[]);

/*---------------------------------------------------------------------------
 *-- Arguments
 *--
 *-- The above three procedures have similar sequences of arguments
 *-- the only difference is that ddb_h2 takes an additional input argument
 *-- that induces the heuristic to possibly favor distributing the
 *-- right or the left axes of the data array.
 *--
 *-- ndims (input): number of dimensions in the data array and the
 *-- process grid. There is no provision for requesting process
 *-- with fewer dimensions as the data array.
 *--
 *-- ardims (input): extents of each dimension of the data array.
 *-- This array is of size ndims. destroyed.
 *--
 *-- npes (input): number of processes onto which the distribution
 *-- takes place.
 *--
 *-- threshold (input): minimum acceptable value of the load balance
 *-- ratio returned by ddb_ap(), see below.
 *--
 *-- bias (input to ddb_h2): when set to a positive value positive
 *-- the rightmost axes of the data array are preferentially
 *-- distributed, similarly when bias is negative. When bias is zero
 *-- the heuristic attempts to deal processes equally among the axes
 *-- of the data array.
 *--
 *-- blk (input/output): granularity of data mapping: The number of
 *-- consecutive elements assize_t each dimension of the array. Upon output:
 *-- extents of the local array assigned to the process
 *-- that is assigned the array element with lowest global indices, e.g. A[0].
 *-- The meaning of this array is not the same for ddb than for the underlying
 *-- load balancing subroutines. For the subroutine ddb, any non-positive value
 *-- in array blk is taken at face value. For example with ardim=[50,40],
 *-- blk = [3,-1] and npes = 40 the process grid that will be computed is:
 *-- pedims=[20,2] with 3x2 processes storing no data. This is compatible
 *-- with the semantics of the load-balancing subroutine in the 2D GA.
 *--
 *-- pedims(output): number of processes assize_t each dimension of the
 *-- data array.
 *--
 *--------------------------------------------------------------------------
 *--
 *-- Other prototypes
 *--
 *-- dd_ev evaluates the load balance ratio for the data distribution
 *-- specified by its argument list.
 *--
 *-- ddb_ap and dd_lk are specific to ddb_h1.
 */

void ddb_ap(ssize_t ndims, double qedims[], ssize_t ardims[], ssize_t pedims[], ssize_t npes, ssize_t npdivs, ssize_t pdivs[]);
double dd_ev(ssize_t ndims,ssize_t ardims[], ssize_t pedims[]);
ssize_t dd_lk(ssize_t * prt, ssize_t n, double key);
void dd_su(ssize_t ndims, ssize_t ardims[], ssize_t pedims[], ssize_t blk[]);

/*---------------------------------------------------------------------------
 *--
 *-- Dependencies:
 *--
 *-- ddb: ddb_h2, ddb_ex
 *-- ddb_ex: dd_ev, dd_su
 *-- ddb_h1: ddb_ap, dd_ev, ddb_ex, dd_lk, dd_su  & -lm
 *-- ddb_h2: dd_ev, ddb_ex, dd_su
 *--
 ***************************************************************************/

/* The threshold for switching to an exhaustive search*/
/* UNUSED #define THRESHOLD -0.1 */

/************************************************************************
 *--
 *--  void ddb is a wrapper ontop of some load balancing heuristics. ddb
 *--  is called from within GA to compute process grids.
 *--  The first argument, ndims, is the number of
 *--  array dimensions. The resulting process grid also has ndims
 *--  dimensions but some of these can be degenerate.
 ************************************************************************/
void ddb(ssize_t ndims, ssize_t ardims[], ssize_t npes, ssize_t blk[], ssize_t pedims[])
{
    const double ddb_threshold = 0.1;
    const ssize_t ddb_bias = 0;
    ssize_t count = 0;

    ssize_t tp = (ssize_t)npes;

    /* count how many axes have <don't care> block values.*/
    for (ssize_t i=ndims-1; i>=0; i--) {
       if (blk[i]<=0){
          pedims[i] = -1;
          count += 1;
       } else {
          ssize_t sp = (ssize_t)(ardims[i]+blk[i]-1)/blk[i];
          if (sp>tp) {
             sp = tp; tp = 1;
             pedims[i] = (ssize_t)sp;
          } else {
             ssize_t j;
             for (j=sp; j<tp&&(tp%j!=0); j++);
             pedims[i] = (ssize_t)j;
             tp = tp / j;
          }
       }
    }

    if (count>0){
       ssize_t * tardim = calloc((ssize_t)count,sizeof(ssize_t));
       if (tardim==NULL) {
         fprintf(stderr,"ddb: Memory allocation failed\n");
         for (ssize_t i=0; i<ndims; i++) pedims[i] = 0;
         return;
       }
       ssize_t * tblk = calloc((ssize_t)count,sizeof(ssize_t));
       if (tblk==NULL) {
          fprintf(stderr,"ddb: Memory allocation failed\n");
          for (ssize_t i=0; i<ndims; i++) pedims[i] = 0;
          return;
       }
       ssize_t * tpedim = calloc((ssize_t)count,sizeof(ssize_t));
       if (tpedim==NULL) {
          fprintf(stderr,"ddb: Memory allocation failed\n");
          for (ssize_t i=0; i<ndims; i++) pedims[i] = 0;
          return;
       }

       for (ssize_t i=0;i<count;i++) {
           tblk[i] = 1;
       }
       for (ssize_t i=0,j=0;j<ndims;j++) {
          if (pedims[j]<0) tardim[i++] = ardims[j];
       }

       ddb_h2(count, tardim, tp, ddb_threshold, ddb_bias, tblk, tpedim);
       /* ddb_h1(count, tardim, tp, ddb_threshold, tblk, tpedim); */

       for (ssize_t i=0, j=0; j<ndims; j++)
          if (pedims[j]<0){
             blk[j] = (tardim[i]+tpedim[i]-1)/tpedim[i];
             i = i+1;
          }

       for (ssize_t i=0, j=0; j<ndims; j++) {
          if (pedims[j]<0) pedims[j] = tpedim[i++];
       }

       free(tpedim);
       free(tblk);
       free(tardim);

    } else {

       for (ssize_t j=0; j<ndims; j++)
          if (pedims[j]<0) pedims[j] = 1;

    }

}
/************************************************************************
 *--
 *--  void ddb_ex implements a naive data mapping of a multi-dimensional
 *--  array across a process Cartesian topology. ndims is the number of
 *--  array dimensions. The resulting process grid also has ndims 
 *--  dimensions but some of these can be degenerate.
 *--
 *--  Heuristic:   Let d be the number of dimensions of the data array.
 *--  Return that assignment p1, ..., pd that minimizes the communication
 *--  volume measure among those that maximizes the load balancing ratio
 *--  computed by dd_ev. 
 *--  The communication volume measure is the sum of
 *--  all monomials (ni/pi) of degree d-1.
 *--
 *--  ddb_ex returns as soon as it has found a process distribution whose
 *--  load balance ratio is at least as large as the value of threshold.
 *--
 *--  This procedure allocates storage for 3*ndims+npes integers.
 *--
 ************************************************************************/
void ddb_ex( ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold,
             ssize_t blk[], ssize_t pedims[])
{
      ssize_t *tdims;
      ssize_t npdivs;
      ssize_t i, j, k;
      ssize_t bev;
      ssize_t pc, done;
      ssize_t *stack;
      ssize_t *tard;
      ssize_t r, cev;
      double clb, blb;

      /*- Quick exit -*/
      if (ndims==1) {
           pedims[0] = npes;
           blb = dd_ev(ndims,ardims,pedims);
           dd_su(1,ardims,pedims,blk);
           return;
      }

      /*- Reset array dimensions to reflect granularity -*/
      tard = calloc((ssize_t)ndims,sizeof(ssize_t));
      if (tard==NULL) {
         fprintf(stderr,"ddb_ex: Memory allocation failed\n");
         for (ssize_t i=0; i<ndims; i++) blk[i] = 0;
         return;
      }
      for (ssize_t i=0; i<ndims; i++) {
          if (blk[i]<1) blk[i] = 1;
      }
      for (ssize_t i=0; i<ndims; i++) {
          tard[i] = ardims[i] / blk[i];
      }
      for (ssize_t i=0; i<ndims; i++) {
          if (tard[i]<1) {
              tard[i] = 1; blk[i] = ardims[i];
          }
      }

      /*- Allocate memory for current solution -*/
      tdims = calloc((ssize_t)ndims,sizeof(ssize_t));
      if (tdims==NULL) {
         fprintf(stderr,"ddb_ex: Memory allocation failed\n");
         for (ssize_t i=0; i<ndims; i++) blk[i] = 0;
         return;
      }

      /*- Allocate memory to hold divisors of npes -*/
      npdivs = 1;
      for (i=2;i<=npes;i++) if (npes%i==0) npdivs += 1;
      ssize_t * pdivs = calloc((ssize_t)npdivs,sizeof(ssize_t));
      if (pdivs==NULL) {
         fprintf(stderr,"ddb_ex: Memory allocation failed\n");
         for (ssize_t i=0; i<ndims; i++) blk[i] = 0;
         free(tard);
         return;
      }

      /*- Allocate storage for the recursion stack -*/
      stack = calloc((ssize_t)ndims,sizeof(ssize_t));
      if (stack==NULL){
         fprintf(stderr,"%s: %s\n","ddb_ex",
             "memory allocation failed");
         for (ssize_t i=0; i<ndims; i++) blk[i] = 0;
         free(tard);
         free(pdivs);
         return;
      }

      /*- Find all divisors of npes -*/
      for (j=0,i=1;i<=npes;i++) if (npes%i==0) pdivs[j++] = i;

      /*- Pump priming the exhaustive search -*/
      blb = -1.0;
      bev = 1.0;
      for (ssize_t i=0; i<ndims; i++) bev *= tard[i];
      pedims[0]=npes; for (i=1;i<ndims;i++) pedims[i] = 1;
      tdims[0] = 0;
      stack[0] = npes;
      pc = 0;
      done = 0;

      /*-  Recursion loop -*/
      do {
         if (pc==ndims-1) {
	   /*- Set the number of processes for the last dimension -*/
           tdims[pc] = stack[pc];

	   /*- Evaluate current solution  -*/
            clb = dd_ev(ndims,tard,tdims);
            cev = 0;
            for (k=0; k<ndims; k++){
               r = 1;
               for (j=0; j<ndims; j++) {
                  if (j!=k) r = r*(tard[j]/tdims[j]);
               }
               cev = cev+r;
            }
            if (clb>blb || (clb==blb && cev<bev)) {
               for (j=0; j<ndims; j++) pedims[j] = tdims[j];
               blb = clb;
               bev = cev;
            }
            if (blb>threshold) break;
            tdims[pc] = 0;
            pc -= 1;
         } else {
           if ( tdims[pc] == stack[pc] ) {
	     /*- Backtrack when current array dimension has exhausted 
              *- all remaining processes
              */
              done = (pc==0);
              tdims[pc] = 0;
              pc -= 1;
           } else {
	     /*- Increment the number of processes assigned to the current
              *- array axis.
              */
              for (tdims[pc]+=1; stack[pc]%tdims[pc]!=0; tdims[pc]+=1);
              pc += 1;
              stack[pc] = npes;
              for (i=0;i<pc;i++) stack[pc] /= tdims[i];
              tdims[pc] = 0;
           }
         }
      } while(!done);

      dd_su(ndims,ardims,pedims,blk);

      free(tard);
      free(stack);
      free(tdims);
      free(pdivs);
}
/************************************************************************
 *--
 *-- void ddb_h1 estimates a good load balancing scheme
 *-- for a Cartesian process topology given the extents
 *-- assize_t all dimensions of an array. It does so by solving
 *-- exactly the continous problem and then finding a 'nearby'
 *-- approximation to the solution of the discrete problem.
 *--
 *-- If the value of objective function attained with this heuristic
 *-- is less than threshold then an exhaustive search is performed.
 *--
 *-- This procedure allocates storage for ndims ssize_ts and npes doubles and
 *-- may call ddb_ex.
 *--
 ************************************************************************/
void ddb_h1(ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold, ssize_t blk[], ssize_t pedims[])
{
      ssize_t h, i, j, k;
      ssize_t npdivs;
      double t, q;
      double cb, ub, blb;

      if (ndims==1) {
         pedims[0] = npes;
         blb = dd_ev(ndims,ardims,pedims);
         dd_su(ndims,ardims,pedims,blk);
         return;
      }

      /*- Allocate memory to store the granularity -*/
      ssize_t * tard = calloc((ssize_t)ndims,sizeof(ssize_t));
      if (tard==NULL) {
         fprintf(stderr,"%s: %s\n","ddb_h1", "memory allocation failed");
         for (ssize_t i=0; i<ndims; i++) {
             blk[i] = 0;
         }
         return;
      }
      /*- Reset array dimensions to reflect granularity -*/
      for (ssize_t i=0; i<ndims; i++) {
          if (blk[i]<1) blk[i] = 1;
      }
      for (ssize_t i=0; i<ndims; i++) {
          tard[i] = ardims[i] / blk[i];
      }
      for (ssize_t i=0; i<ndims; i++) {
          if (tard[i]<1) {
              tard[i] = 1; blk[i] = ardims[i];
          }
      }

      /*- First solve the load balancing problem exactly in
       *- floating point arithmetic -*/
      double * qedims = calloc((ssize_t)ndims,sizeof(double));
      if (qedims==NULL) {
         fprintf(stderr,"%s: %s\n","ddb_h1", "memory allocation failed");
         for (ssize_t i=0; i<ndims; i++) {
             blk[i] = 0;
         }
         free(tard);
         return;
      }

      qedims[0] = (double) npes;
      for (i=1;i<ndims;i++) {
          qedims[0] /= (tard[i]/(double)tard[0]);
      }
      qedims[0] = pow(qedims[0],1.0/ndims);

      for (i=1;i<ndims;i++){
          qedims[i] = (tard[i]/(double)tard[0])*qedims[0];
      }

      /*- Set up the search for a integer approximation the floating point solution -*/
      npdivs = 1;
      for (i=2;i<=npes;i++) if (npes%i==0) npdivs += 1;
      ssize_t * pdivs = calloc((ssize_t)npdivs,sizeof(ssize_t));
      if (pdivs==NULL){
         fprintf(stderr,"%s: %s\n","split",
             "memory allocation failed");
         for (ssize_t i=0; i<ndims; i++) blk[i] = 0;
         free(tard);
         free(qedims);
         return;
      }

      /*- Compute the discreet approximation -*/
      for (j=0,i=1;i<=npes;i++) {
          if (npes%i==0) pdivs[j++] = i;
      }

      ddb_ap(ndims,qedims,tard,pedims,npes,npdivs,pdivs);

      free(qedims);
      free(pdivs);


      /*- Lookout for a permutation of the solution vector
       *- that would improve the initial solution -*/
      ssize_t * apdims = calloc((ssize_t)ndims,sizeof(ssize_t));
      if (apdims==NULL){
         fprintf(stderr,"%s: %s\n","split",
             "memory allocation failed");
         for (ssize_t i=0; i<ndims; i++) blk[i] = 0;
         free(tard);
         return;
      }

      ub = dd_ev(ndims,tard,pedims);
      for (k=0;k<ndims;k++) apdims[k] = pedims[k];

      do {
         for (k=0;k<ndims;k++) pedims[k] = apdims[k];
         blb = ub;

         /*- Find the worst distributed dimension -*/
         h = 0;
         q = (tard[0]<pedims[0]) ? pedims[0] : tard[0]%pedims[0];
         q /= (double)pedims[0];
         for (k=1;k<ndims;k++){
            t = (tard[k]<pedims[k]) ? pedims[k] : tard[k]%pedims[k];
            t /= (double) pedims[k];
            if (t>q) {
              h = k; q = t;
            }
         }

         /*- Swap elements of apdims to improve load balance */
         j = h;
         for (k=0;k<ndims;k++){
            if (k==h) continue;
            i = apdims[h]; apdims[h] = apdims[k]; apdims[k] = i;
            cb = dd_ev(ndims,tard,apdims);
            if (cb>ub) {
              j = k;
              ub = cb;
            }
            i = apdims[h]; apdims[h] = apdims[k]; apdims[k] = i;
         }
         if (j!=h){
            i = apdims[h]; apdims[h] = apdims[j]; apdims[j] = i;
         }
      } while (ub > blb);

      for (ssize_t i=0; i<ndims; i++) pedims[i] = apdims[i];
      blb = ub;
      free(apdims);

      /*- Do an exhaustive search is the heuristic returns a solution 
       *- whose load balance ratio is less than the given threshold. -*/
      if (blb<threshold) ddb_ex(ndims,tard,npes,threshold,blk,pedims);

      free(tard);

      dd_su(ndims,ardims,pedims,blk);

      return;
      }

/*---------------------------------------------------------------------------
 *--
 *-- void ddb_ap find an integer vector that is near in some sense
 *-- to a real valued vector
 *--
 *---------------------------------------------------------------------------*/
void ddb_ap(ssize_t ndims, double * qedims, ssize_t * ardims, ssize_t * pedims, ssize_t npes, ssize_t npdivs, ssize_t * pdivs)
{
    for (ssize_t idim=0; idim<ndims-1; idim++) {
       ssize_t g = dd_lk(pdivs,npdivs,qedims[idim]);
       ssize_t bq = pdivs[g] ;
       pedims[idim] = bq;
       npes /= bq;
       npes = (npes<1) ? 1 : npes;
       if (idim<ndims-2){
          qedims[idim+1] = (double) npes;
          for (ssize_t i=idim+2; i<ndims;i++) {
              qedims[idim+1] /= (ardims[i]/(double)ardims[idim+1]);
          }
          qedims[idim+1] = (double) pow(qedims[idim+1], 1.0/(double)(ndims-1-idim));
          for (ssize_t i=idim+2; i<ndims; i++) {
              qedims[i] = (ardims[i]/(double)ardims[idim+1]) * qedims[idim+1];
          }

          if (bq>1) {
              ssize_t k = 1;
              for (ssize_t i=g+1; i<npdivs; i++) {
                  if (pdivs[i]%bq==0) {
                      pdivs[k] = pdivs[i]/bq;
                      k += 1;
                  }
              }
              npdivs = k;
          }
       }
    }
    pedims[ndims-1] = npes;
}
/*---------------------------------------------------------------------------
 *--
 *-- ssize_t dd_lk find nearest match to key
 *--
 *---------------------------------------------------------------------------*/
ssize_t dd_lk(ssize_t *prt, ssize_t n, double key)
{
      if (n==1) return 0;

      ssize_t k;
      if (n<=5) {
        k = 0;
        double km = key-prt[0];
        if (km<0.0) km = -km;
        for (ssize_t i=1; i<n; i++) {
           double kz = key-prt[i];
           if (kz<0.0) kz = -kz;
           if (kz<km) {
              km = kz;
              k = i;
           }
        }
        return k;
      }
      ssize_t lw = 0;
      ssize_t hgh = n-1;

      if (lw==hgh) {
         return lw;
      }
      do {
         ssize_t md = (lw+hgh)/2;
         if (key>prt[md]) {
            lw = md+1;
         } else {
            hgh = md;
         }
      } while (lw<hgh);

      double kz = prt[lw];
      double u = key-prt[lw];
      if (u<0.0) u = -u;
      ssize_t h = lw;

      if (lw>0) {
        double v = key-prt[lw-1];
        if (v<0.0) v = -v;
        if (v<u) {
          u = v;
          h = lw-1;
        }
      }
      if (lw<n-1) {
        double v = key-prt[lw+1];
        if (v<0.0) v = -v;
        if (v<u) {
          u = v;
          h = lw+1;
        }
      }
      return h ;
}
/************************************************************************
 *--
 *-- void ddb_h2 lists all prime divisors of the number of
 *-- processes and distribute these among the array dimensions.
 *-- If the value of objective function attained with this heuristic
 *-- is less than threshold then an exhaustive search is performed.
 *-- The argument bias directs the search of ddb_h2. When bias is
 *-- positive the rightmost axes of the data array are preferentially
 *-- distributed, similarly when bias is negative. When bias is zero
 *-- the heuristic attempts to deal processes equally among the axes
 *-- of the data array.
 *--
 *-- ddb_h2 allocates storage for ndims+npes integers and may call ddb_ex.
 *--
 ************************************************************************/
void ddb_h2(ssize_t ndims, ssize_t ardims[], ssize_t npes, double threshold, ssize_t bias, ssize_t blk[], ssize_t pedims[])
{
      /*- Allocate memory to store the granularity -*/
      ssize_t * tard = calloc((ssize_t)ndims,sizeof(ssize_t));
      if (tard==NULL){
         fprintf(stderr,"%s: %s\n","ddb_h2", "memory allocation failed");
         for (ssize_t i=0; i<ndims; i++) {
             blk[i] = 0;
         }
         return;
      }

      /*- Reset array dimensions to reflect granularity -*/
      for (ssize_t i=0; i<ndims; i++) {
          if (blk[i]<1) blk[i] = 1;
      }

      for (ssize_t i=0; i<ndims; i++) {
          tard[i] = (ardims[i] + blk[i]-1)/blk[i];
      }

      for (ssize_t i=0; i<ndims; i++) {
          if (tard[i]<1) {
             tard[i] = 1;
             blk[i] = ardims[i];
          }
      }

      /*- Allocate storage to old all divisors of npes -*/
      ssize_t npdivs = 1;
      for (ssize_t i=2; i<=npes; i++) {
          if (npes%i==0) npdivs += 1;
      }
      ssize_t * pdivs = calloc((ssize_t)npdivs,sizeof(ssize_t));
      if (pdivs==NULL){
         fprintf(stderr,"%s: %s\n","split", "memory allocation failed");
         for (ssize_t i=0; i<ndims; i++) {
             blk[i] = 0;
         }
         free(tard);
         return;
      }

      /*- Find all divisors of npes -*/
      for (ssize_t j=0, i=1; i<=npes; i++) {
          if (npes%i==0) pdivs[j++] = i;
      }

      /*- Find all prime divisors of npes (with repetitions) -*/
      if (npdivs>1) {
         ssize_t k = 1;
         do {
            ssize_t h = k+1;
            for (ssize_t j=h; j<npdivs; j++) {
               if (pdivs[j]%pdivs[k]==0) {
                  pdivs[h++] = pdivs[j]/pdivs[k];
               }
            }
            npdivs = h;
            k = k+1;
         } while(k<npdivs);
      }

      /*- Set istart and istep -*/
      ssize_t istep = 1;
      ssize_t istart = 0;
      if (bias>0) {
          istep = -1;
          istart = ndims-1;
      }
      /*- Set pedims -*/
      for (ssize_t j=0;j<ndims;j++) {
          pedims[j] = 1.0;
      }
      for (ssize_t k=npdivs-1;k>=1;k--) {
         ssize_t p0 = pdivs[k];
         ssize_t h = istart;
         double q = (tard[istart]<p0*pedims[istart]) ? 1.1 : (tard[istart]%(p0*pedims[istart]))/(double) tard[istart];
         for (ssize_t j=1;j<ndims;j++){
            ssize_t ilook = (istart+istep*j)%ndims;
            double w = (tard[ilook]<p0*pedims[ilook]) ? 1.1 : (tard[ilook]%(p0*pedims[ilook]))/(double) tard[ilook];
            if (w<q) {
                q = w;
                h = ilook;
            }
         }
         pedims[h] *= p0;
         if (bias==0) istart = (istart+1)%ndims;
      }
      free(pdivs);

      double ub = dd_ev(ndims,tard,pedims);

      /*- Do an exhaustive search is the heuristic returns a solution
       *- whose load balance ratio is less than the given threshold. -*/
      if (ub<threshold) {
         ddb_ex(ndims, tard, npes, threshold, blk, pedims);
      }

      dd_su(ndims,ardims,pedims,blk);

      for (ssize_t i=0; i<ndims; i++) {
          if (pedims[i]>0){
             blk[i] = (tard[i]+pedims[i]-1)/pedims[i];
          } else {
             fprintf(stderr,"process dimension is zero: ddb_h2");
             abort();
          }
      }

      free(tard);
      return;
}

/****************************************************************************
 *--
 *--  double dd_ev evaluates the load balancing ratio as follows:
 *--
 *--  Let n1, n2 ... nd be the extents of the array dimensions
 *--  Let p1, p2 ... pd be the numbers of processes across
 *--      the corresponding dimensions of the process grid
 *--  Let there be npes processes available
 *--
 *--  Load balancing measure = (n1/p1)*...*(nd/pd)*npes
 *--                           ------------------------
 *--                                 n1*n2*...*nd
 *--  The communication volume measure is the sum of
 *--  all monomials (ni/pi) of degree d-1.
 *--
 ****************************************************************************/
double dd_ev(ssize_t ndims,ssize_t ardims[], ssize_t pedims[])
{
    double q = 1.0;
    double t = 1.0;
    for (ssize_t k=0; k<ndims; k++){
        q = (ardims[k]/pedims[k])*pedims[k];
        t = t*(q/(double)ardims[k]);
    }
    return t;
}

/****************************************************************************
 *--
 *--  void dd_su computes the extents of the local block corresponding
 *--  to the element with least global indices, e.g. A[0][0][0].
 *--
 ****************************************************************************/
void dd_su(ssize_t ndims, ssize_t ardims[], ssize_t pedims[], ssize_t blk[])
{
    for (ssize_t i=0; i<ndims; i++) {
         blk[i] = ardims[i]/pedims[i];
         if (blk[i]<1) blk[i] = 1;
    }
}
