/*
*******************************************************************

SMATRIX.C -- Sparse matrix routines for EPANET program.

VERSION:    2.00
DATE:       5/8/00
AUTHOR:     L. Rossman
            US EPA - NRMRL

This module contains the sparse matrix routines used to solve
a network's hydraulic equations. The entry points into this
module are:
   createsparse() -- called from openhyd() in HYDRAUL.C
   freesparse()   -- called from closehyd() in HYDRAUL.C
   linsolve()     -- called from netsolve() in HYDRAUL.C

Createsparse() does the following:
   1. for each node, builds an adjacency list that identifies
      all links connected to the node (see buildlists())
   2. re-orders the network's nodes to minimize the number
      of non-zero entries in the hydraulic solution matrix
      (see reorder())
   3. converts the adjacency lists into a compact scheme
      for storing the non-zero coeffs. in the lower diagonal
      portion of the solution matrix (see storesparse())
Freesparse() frees the memory used for the sparse matrix.
Linsolve() solves the linearized system of hydraulic equations.

********************************************************************
*/

#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <math.h>
#include "epanet2.h"
#include "funcs.h"
#include "hash.h"
#include "text.h"
#include "types.h"
#define EXTERN extern
#include "vars.h"

int createsparse(EN_Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: creates sparse representation of coeff. matrix
**--------------------------------------------------------------
*/
{
  int errcode = 0;
  EN_Network *n = &pr->network;
  solver_t *s = &pr->hydraulics.solver;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;

  /* Allocate data structures */
  ERRCODE(allocsparse(pr));

  if (errcode) {
    return (errcode);
  }

  /* Build node-link adjacency lists with parallel links removed. */
  s->Degree = (int *)calloc(n->Nnodes + 1, sizeof(int));
  ERRCODE(MEMCHECK(s->Degree));
  ERRCODE(buildlists(pr, TRUE));
  if (!errcode) {
    xparalinks(pr);  /* Remove parallel links */
    countdegree(pr); /* Find degree of each junction */
  }                  /* (= # of adjacent links)  */

  /* Re-order nodes to minimize number of non-zero coeffs.    */
  /* in factorized solution matrix. At same time, adjacency   */
  /* list is updated with links representing non-zero coeffs. */
  hyd->Ncoeffs = n->Nlinks;
  ERRCODE(reordernodes(pr));

  /* Allocate memory for sparse storage of positions of non-zero */
  /* coeffs. and store these positions in vector NZSUB. */
  ERRCODE(storesparse(pr, net->Njuncs));

  /* Free memory used for adjacency lists and sort */
  /* row indexes in NZSUB to optimize linsolve().  */
  if (!errcode) {
    freelists(pr);
  }
  ERRCODE(ordersparse(hyd, net->Njuncs));

  /* Re-build adjacency lists without removing parallel */
  /* links for use in future connectivity checking.     */
  ERRCODE(buildlists(pr, FALSE));

  /* Free allocated memory */
  free(s->Degree);
  return (errcode);
} /* End of createsparse */

int allocsparse(EN_Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: allocates memory for indexing the solution matrix
**--------------------------------------------------------------
*/
{
  EN_Network *n = &pr->network;
  solver_t *s = &pr->hydraulics.solver;

  int errcode = 0;
  n->Adjlist = (Padjlist *)calloc(n->Nnodes + 1, sizeof(Padjlist));
  s->Order = (int *)calloc(n->Nnodes + 1, sizeof(int));
  s->Row = (int *)calloc(n->Nnodes + 1, sizeof(int));
  s->Ndx = (int *)calloc(n->Nlinks + 1, sizeof(int));
  ERRCODE(MEMCHECK(n->Adjlist));
  ERRCODE(MEMCHECK(s->Order));
  ERRCODE(MEMCHECK(s->Row));
  ERRCODE(MEMCHECK(s->Ndx));
  return (errcode);
}

void freesparse(EN_Project *pr)
/*
**----------------------------------------------------------------
** Input:   None
** Output:  None
** Purpose: Frees memory used for sparse matrix storage
**----------------------------------------------------------------
*/
{
  EN_Network *n = &pr->network;
  solver_t *s = &pr->hydraulics.solver;

  freelists(pr);
  free(n->Adjlist);
  free(s->Order);
  free(s->Row);
  free(s->Ndx);
  free(s->XLNZ);
  free(s->NZSUB);
  free(s->LNZ);
} /* End of freesparse */

int buildlists(EN_Project *pr, int paraflag)
/*
**--------------------------------------------------------------
** Input:   paraflag = TRUE if list marks parallel links
** Output:  returns error code
** Purpose: builds linked list of links adjacent to each node
**--------------------------------------------------------------
*/
{
  int i, j, k;
  int pmark = 0;
  int errcode = 0;
  Padjlist alink;

  EN_Network *n = &pr->network;

  /* For each link, update adjacency lists of its end nodes */
  for (k = 1; k <= n->Nlinks; k++) {
    i = n->Link[k].N1;
    j = n->Link[k].N2;
    if (paraflag) {
      pmark = paralink(pr, i, j, k); /* Parallel link check */
    }

    /* Include link in start node i's list */
    alink = (struct Sadjlist *)malloc(sizeof(struct Sadjlist));
    if (alink == NULL) return (101);
    if (!pmark)
      alink->node = j;
    else
      alink->node = 0; /* Parallel link marker */
    alink->link = k;
    alink->next = n->Adjlist[i];
    n->Adjlist[i] = alink;

    /* Include link in end node j's list */
    alink = (struct Sadjlist *)malloc(sizeof(struct Sadjlist));
    if (alink == NULL) return (101);
    if (!pmark)
      alink->node = i;
    else
      alink->node = 0; /* Parallel link marker */
    alink->link = k;
    alink->next = n->Adjlist[j];
    n->Adjlist[j] = alink;
  }
  return (errcode);
} /* End of buildlists */

int paralink(EN_Project *pr, int i, int j, int k)
/*
**--------------------------------------------------------------
** Input:   i = index of start node of link
**          j = index of end node of link
**          k = link index
** Output:  returns 1 if link k parallels another link, else 0
** Purpose: checks for parallel links between nodes i and j
**
**--------------------------------------------------------------
*/
{
  Padjlist alink;
  for (alink = pr->network.Adjlist[i]; alink != NULL; alink = alink->next) {
    if (alink->node == j) /* Link || to k (same end nodes) */
    {
      pr->hydraulics.solver.Ndx[k] =
          alink->link; /* Assign Ndx entry to this link */
      return (1);
    }
  }
  pr->hydraulics.solver.Ndx[k] = k; /* Ndx entry if link not parallel */
  return (0);
} /* End of paralink */

void xparalinks(EN_Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  none
** Purpose: removes parallel links from nodal adjacency lists
**--------------------------------------------------------------
*/
{
  int i;
  Padjlist alink, /* Current item in adjacency list */
      blink;      /* Previous item in adjacency list */
  EN_Network *n = &pr->network;

  /* Scan adjacency list of each node */
  for (i = 1; i <= n->Nnodes; i++) {
    alink = n->Adjlist[i]; /* First item in list */
    blink = NULL;
    while (alink != NULL) {
      if (alink->node == 0) /* Parallel link marker found */
      {
        if (blink == NULL) /* This holds at start of list */
        {
          n->Adjlist[i] = alink->next;
          free(alink); /* Remove item from list */
          alink = n->Adjlist[i];
        } else /* This holds for interior of list */
        {
          blink->next = alink->next;
          free(alink); /* Remove item from list */
          alink = blink->next;
        }
      } else {
        blink = alink; /* Move to next item in list */
        alink = alink->next;
      }
    }
  }
} /* End of xparalinks */

void freelists(EN_Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  none
** Purpose: frees memory used for nodal adjacency lists
**--------------------------------------------------------------
*/
{
  int i;
  Padjlist alink;
  EN_Network *n = &pr->network;

  for (i = 0; i <= n->Nnodes; i++) {
    for (alink = n->Adjlist[i]; alink != NULL; alink = n->Adjlist[i]) {
      n->Adjlist[i] = alink->next;
      free(alink);
    }
  }
} /* End of freelists */

void countdegree(EN_Project *pr)
/*
**----------------------------------------------------------------
** Input:   none
** Output:  none
** Purpose: counts number of nodes directly connected to each node
**----------------------------------------------------------------
*/
{
  int i;
  Padjlist alink;
  EN_Network *n = &pr->network;
  memset(pr->hydraulics.solver.Degree, 0, (n->Nnodes + 1) * sizeof(int));

  /* NOTE: For purposes of node re-ordering, Tanks (nodes with  */
  /*       indexes above Njuncs) have zero degree of adjacency. */

  for (i = 1; i <= n->Njuncs; i++) {
    for (alink = n->Adjlist[i]; alink != NULL; alink = alink->next) {
      if (alink->node > 0) {
        pr->hydraulics.solver.Degree[i]++;
      }
    }
  }
}

int reordernodes(EN_Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns 1 if successful, 0 if not
** Purpose: re-orders nodes to minimize # of non-zeros that
**          will appear in factorized solution matrix
**--------------------------------------------------------------
*/
{
  int k, knode, m, n;
  EN_Network *net = &pr->network;
  solver_t *s = &pr->hydraulics.solver;

  for (k = 1; k <= net->Nnodes; k++) {
    s->Row[k] = k;
    s->Order[k] = k;
  }
  n = net->Njuncs;
  for (k = 1; k <= n; k++) /* Examine each junction    */
  {
    m = mindegree(s, k, n); /* Node with lowest degree  */
    knode = s->Order[m];    /* Node's index             */
    if (!growlist(pr, knode)) {
      return (101); /* Augment adjacency list   */
    }
    s->Order[m] = s->Order[k]; /* Switch order of nodes    */
    s->Order[k] = knode;
    s->Degree[knode] = 0; /* In-activate node         */
  }
  for (k = 1; k <= n; k++) { /* Assign nodes to rows of  */
    s->Row[s->Order[k]] = k; /*   coeff. matrix          */
  }
  return (0);
} /* End of reordernodes */

int mindegree(solver_t *s, int k, int n)
/*
**--------------------------------------------------------------
** Input:   k = first node in list of active nodes
**          n = total number of junction nodes
** Output:  returns node index with fewest direct connections
** Purpose: finds active node with fewest direct connections
**--------------------------------------------------------------
*/
{
  int i, m;
  int min = n, imin = n;

  for (i = k; i <= n; i++) {
    m = s->Degree[s->Order[i]];
    if (m < min) {
      min = m;
      imin = i;
    }
  }
  return (imin);
} /* End of mindegree */

int growlist(EN_Project *pr, int knode)
/*
**--------------------------------------------------------------
** Input:   knode = node index
** Output:  returns 1 if successful, 0 if not
** Purpose: creates new entries in knode's adjacency list for
**          all unlinked pairs of active nodes that are
**          adjacent to knode
**--------------------------------------------------------------
*/
{
  int node;
  Padjlist alink;
  EN_Network *n = &pr->network;
  solver_t *s = &pr->hydraulics.solver;

  /* Iterate through all nodes connected to knode */
  for (alink = n->Adjlist[knode]; alink != NULL; alink = alink->next) {
    node = alink->node;      /* End node of connecting link  */
    if (s->Degree[node] > 0) /* End node is active           */
    {
      s->Degree[node]--;         /* Reduce degree of adjacency   */
      if (!newlink(pr, alink)) { /* Add to adjacency list        */
        return (0);
      }
    }
  }
  return (1);
} /* End of growlist */

int newlink(EN_Project *pr, Padjlist alink)
/*
**--------------------------------------------------------------
** Input:   alink = element of node's adjacency list
** Output:  returns 1 if successful, 0 if not
** Purpose: links end of current adjacent link to end nodes of
**          all links that follow it on adjacency list
**--------------------------------------------------------------
*/
{
  int inode, jnode;
  Padjlist blink;
  EN_Network *n = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &pr->hydraulics.solver;

  /* Scan all entries in adjacency list that follow anode. */
  inode = alink->node; /* End node of connection to anode */
  for (blink = alink->next; blink != NULL; blink = blink->next) {
    jnode = blink->node; /* End node of next connection */

    /* If jnode still active, and inode not connected to jnode, */
    /* then add a new connection between inode and jnode.       */
    if (s->Degree[jnode] > 0) /* jnode still active */
    {
      if (!linked(n, inode, jnode)) { /* inode not linked to jnode */
        /* Since new connection represents a non-zero coeff. */
        /* in the solution matrix, update the coeff. count.  */
        hyd->Ncoeffs++;

        /* Update adjacency lists for inode & jnode to */
        /* reflect the new connection.                 */
        if (!addlink(n, inode, jnode, hyd->Ncoeffs)) {
          return (0);
        }
        if (!addlink(n, jnode, inode, hyd->Ncoeffs)) {
          return (0);
        }
        s->Degree[inode]++;
        s->Degree[jnode]++;
      }
    }
  }
  return (1);
} /* End of newlink */

int linked(EN_Network *n, int i, int j)
/*
**--------------------------------------------------------------
** Input:   i = node index
**          j = node index
** Output:  returns 1 if nodes i and j are linked, 0 if not
** Purpose: checks if nodes i and j are already linked.
**--------------------------------------------------------------
*/
{
  Padjlist alink;
  for (alink = n->Adjlist[i]; alink != NULL; alink = alink->next) {
    if (alink->node == j) {
      return (1);
    }
  }
  return (0);
} /* End of linked */

int addlink(EN_Network *net, int i, int j, int n)
/*
**--------------------------------------------------------------
** Input:   i = node index
**          j = node index
**          n = link index
** Output:  returns 1 if successful, 0 if not
** Purpose: augments node i's adjacency list with node j
**--------------------------------------------------------------
*/
{
  Padjlist alink;
  alink = (struct Sadjlist *)malloc(sizeof(struct Sadjlist));
  if (alink == NULL) return (0);
  alink->node = j;
  alink->link = n;
  alink->next = net->Adjlist[i];
  net->Adjlist[i] = alink;
  return (1);
} /* End of addlink */

int storesparse(EN_Project *pr, int n)
/*
**--------------------------------------------------------------
** Input:   n = number of rows in solution matrix
** Output:  returns error code
** Purpose: stores row indexes of non-zeros of each column of
**          lower triangular portion of factorized matrix
**--------------------------------------------------------------
*/
{
  Padjlist alink;
  int i, ii, j, k, l, m;
  int errcode = 0;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &pr->hydraulics.solver;

  /* Allocate sparse matrix storage */
  s->XLNZ = (int *)calloc(n + 2, sizeof(int));
  s->NZSUB = (int *)calloc(hyd->Ncoeffs + 2, sizeof(int));
  s->LNZ = (int *)calloc(hyd->Ncoeffs + 2, sizeof(int));
  ERRCODE(MEMCHECK(s->XLNZ));
  ERRCODE(MEMCHECK(s->NZSUB));
  ERRCODE(MEMCHECK(s->LNZ));
  if (errcode) {
    return (errcode);
  }

  /* Generate row index pointers for each column of matrix */
  k = 0;
  s->XLNZ[1] = 1;
  for (i = 1; i <= n; i++) { /* column */
    m = 0;
    ii = s->Order[i];
    for (alink = net->Adjlist[ii]; alink != NULL; alink = alink->next) {
      j = s->Row[alink->node]; /* row */
      l = alink->link;
      if (j > i && j <= n) {
        m++;
        k++;
        s->NZSUB[k] = j;
        s->LNZ[k] = l;
      }
    }
    s->XLNZ[i + 1] = s->XLNZ[i] + m;
  }
  return (errcode);
} /* End of storesparse */

int ordersparse(hydraulics_t *h, int n)
/*
**--------------------------------------------------------------
** Input:   n = number of rows in solution matrix
** Output:  returns eror code
** Purpose: puts row indexes in ascending order in NZSUB
**--------------------------------------------------------------
*/
{
  int i, k;
  int *xlnzt, *nzsubt, *lnzt, *nzt;
  int errcode = 0;
  solver_t *s = &h->solver;

  xlnzt = (int *)calloc(n + 2, sizeof(int));
  nzsubt = (int *)calloc(h->Ncoeffs + 2, sizeof(int));
  lnzt = (int *)calloc(h->Ncoeffs + 2, sizeof(int));
  nzt = (int *)calloc(n + 2, sizeof(int));
  ERRCODE(MEMCHECK(xlnzt));
  ERRCODE(MEMCHECK(nzsubt));
  ERRCODE(MEMCHECK(lnzt));
  ERRCODE(MEMCHECK(nzt));
  if (!errcode) {
    /* Count # non-zeros in each row */
    for (i = 1; i <= n; i++) {
      nzt[i] = 0;
    }
    for (i = 1; i <= n; i++) {
      for (k = s->XLNZ[i]; k < s->XLNZ[i + 1]; k++) nzt[s->NZSUB[k]]++;
    }
    xlnzt[1] = 1;
    for (i = 1; i <= n; i++) xlnzt[i + 1] = xlnzt[i] + nzt[i];

    /* Transpose matrix twice to order column indexes */
    transpose(n, s->XLNZ, s->NZSUB, s->LNZ, xlnzt, nzsubt, lnzt, nzt);
    transpose(n, xlnzt, nzsubt, lnzt, s->XLNZ, s->NZSUB, s->LNZ, nzt);
  }

  /* Reclaim memory */
  free(xlnzt);
  free(nzsubt);
  free(lnzt);
  free(nzt);
  return (errcode);
} /* End of ordersparse */

void transpose(int n, int *il, int *jl, int *xl, int *ilt, int *jlt, int *xlt,
               int *nzt)
/*
**---------------------------------------------------------------------
** Input:   n = matrix order
**          il,jl,xl = sparse storage scheme for original matrix
**          nzt = work array
** Output:  ilt,jlt,xlt = sparse storage scheme for transposed matrix
** Purpose: Determines sparse storage scheme for transpose of a matrix
**---------------------------------------------------------------------
*/
{
  int i, j, k, kk;

  for (i = 1; i <= n; i++) nzt[i] = 0;
  for (i = 1; i <= n; i++) {
    for (k = il[i]; k < il[i + 1]; k++) {
      j = jl[k];
      kk = ilt[j] + nzt[j];
      jlt[kk] = i;
      xlt[kk] = xl[k];
      nzt[j]++;
    }
  }
} /* End of transpose */

int linsolve(solver_t *s, int n)
/*
**--------------------------------------------------------------
** Input:   s    = solver struct
            n    = number of equations
** Output:  s->F    = solution values
**          returns 0 if solution found, or index of
**          equation causing system to be ill-conditioned
** Purpose: solves sparse symmetric system of linear
**          equations using Cholesky factorization
**
** NOTE:   This procedure assumes that the solution matrix has
**         been symbolically factorized with the positions of
**         the lower triangular, off-diagonal, non-zero coeffs.
**         stored in the following integer arrays:
**            XLNZ  (start position of each column in NZSUB)
**            NZSUB (row index of each non-zero in each column)
**            LNZ   (position of each NZSUB entry in Aij array)
**
**  This procedure has been adapted from subroutines GSFCT and
**  GSSLV in the book "Computer Solution of Large Sparse
**  Positive Definite Systems" by A. George and J. W-H Liu
**  (Prentice-Hall, 1981).
**--------------------------------------------------------------
*/
{
  double *Aii = s->Aii;
  double *Aij = s->Aij;
  double *B = s->F;
  int *LNZ = s->LNZ;
  int *XLNZ = s->XLNZ;
  int *NZSUB = s->NZSUB;

  int *link, *first;
  int i, istop, istrt, isub, j, k, kfirst, newk;
  int errcode = 0;
  double bj, diagj, ljk;
  double *temp;

  temp = (double *)calloc(n + 1, sizeof(double));
  link = (int *)calloc(n + 1, sizeof(int));
  first = (int *)calloc(n + 1, sizeof(int));
  ERRCODE(MEMCHECK(temp));
  ERRCODE(MEMCHECK(link));
  ERRCODE(MEMCHECK(first));
  if (errcode) {
    errcode = -errcode;
    goto ENDLINSOLVE;
  }
  memset(temp, 0, (n + 1) * sizeof(double));
  memset(link, 0, (n + 1) * sizeof(int));

  /* Begin numerical factorization of matrix A into L */
  /*   Compute column L(*,j) for j = 1,...n */
  for (j = 1; j <= n; j++) {
    /* For each column L(*,k) that affects L(*,j): */
    diagj = 0.0;
    newk = link[j];
    k = newk;
    while (k != 0) {
      /* Outer product modification of L(*,j) by  */
      /* L(*,k) starting at first[k] of L(*,k).   */
      newk = link[k];
      kfirst = first[k];
      ljk = Aij[LNZ[kfirst]];
      diagj += ljk * ljk;
      istrt = kfirst + 1;
      istop = XLNZ[k + 1] - 1;
      if (istop >= istrt) {
        /* Before modification, update vectors 'first' */
        /* and 'link' for future modification steps.   */
        first[k] = istrt;
        isub = NZSUB[istrt];
        link[k] = link[isub];
        link[isub] = k;

        /* The actual mod is saved in vector 'temp'. */
        for (i = istrt; i <= istop; i++) {
          isub = NZSUB[i];
          temp[isub] += Aij[LNZ[i]] * ljk;
        }
      }
      k = newk;
    }

    /* Apply the modifications accumulated */
    /* in 'temp' to column L(*,j).         */
    diagj = Aii[j] - diagj;
    if (diagj <= 0.0) /* Check for ill-conditioning */
    {
      errcode = j;
      goto ENDLINSOLVE;
    }
    diagj = sqrt(diagj);
    Aii[j] = diagj;
    istrt = XLNZ[j];
    istop = XLNZ[j + 1] - 1;
    if (istop >= istrt) {
      first[j] = istrt;
      isub = NZSUB[istrt];
      link[j] = link[isub];
      link[isub] = j;
      for (i = istrt; i <= istop; i++) {
        isub = NZSUB[i];
        bj = (Aij[LNZ[i]] - temp[isub]) / diagj;
        Aij[LNZ[i]] = bj;
        temp[isub] = 0.0;
      }
    }
  } /* next j */

  /* Foward substitution */
  for (j = 1; j <= n; j++) {
    bj = B[j] / Aii[j];
    B[j] = bj;
    istrt = XLNZ[j];
    istop = XLNZ[j + 1] - 1;
    if (istop >= istrt) {
      for (i = istrt; i <= istop; i++) {
        isub = NZSUB[i];
        B[isub] -= Aij[LNZ[i]] * bj;
      }
    }
  }

  /* Backward substitution */
  for (j = n; j >= 1; j--) {
    bj = B[j];
    istrt = XLNZ[j];
    istop = XLNZ[j + 1] - 1;
    if (istop >= istrt) {
      for (i = istrt; i <= istop; i++) {
        isub = NZSUB[i];
        bj -= Aij[LNZ[i]] * B[isub];
      }
    }
    B[j] = bj / Aii[j];
  }

ENDLINSOLVE:
  free(temp);
  free(link);
  free(first);
  return (errcode);
} /* End of linsolve */

/************************ END OF SMATRIX.C ************************/
