/*
 ******************************************************************************
 Project:      OWA EPANET
 Version:      2.2
 Module:       smatrix.c
 Description:  solves a sparse set of linear equations
 Authors:      see AUTHORS
 Copyright:    see AUTHORS
 License:      see LICENSE
 Last Updated: 05/15/2019
 ******************************************************************************
*/
/*
 This module contains the sparse matrix routines used to solve a network's
 hydraulic equations. The functions exported by this module are:
   createsparse() -- called from openhyd() in HYDRAUL.C
   freesparse()   -- called from closehyd() in HYDRAUL.C
   linsolve()     -- called from netsolve() in HYDRAUL.C
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <limits.h>

#include <time.h>  //For optional timer macros

#include "text.h"
#include "types.h"
#include "funcs.h"

// The multiple minimum degree re-ordering routine (see genmmd.c)
extern int genmmd(int *neqns, int *xadj, int *adjncy, int *invp, int *perm,
                  int *delta, int *dhead, int *qsize, int *llist, int *marker,
                  int *maxint, int *nofsub);

// Exported functions
int  createsparse(Project *);
void freesparse(Project *);
int  linsolve(Smatrix *, int);

// Local functions
static int     allocsmatrix(Smatrix *, int, int);
static int     alloclinsolve(Smatrix *, int);
static int     localadjlists(Network *, Smatrix *);
static int     paralink(Network *, Smatrix *, int, int, int k);
static void    xparalinks(Network *);
static int     reordernodes(Project *);
static int     factorize(Project *);
static int     growlist(Project *, int);
static int     newlink(Project *, Padjlist);
static int     linked(Network *, int, int);
static int     addlink(Network *, int, int, int);
static int     storesparse(Project *, int);
static int     sortsparse(Smatrix *, int);
static void    transpose(int, int *, int *, int *, int *,
                         int *, int *, int *);


/*************************************************************************
* Timer macros
**************************************************************************/
 //#define cleartimer(tmr) (tmr = 0.0)
 //#define starttimer(tmr) (tmr -= ((double) clock()/CLOCKS_PER_SEC));
 //#define stoptimer(tmr)  (tmr += ((double) clock()/CLOCKS_PER_SEC));
 //#define gettimer(tmr)   (tmr)


/*************************************************************************
* The following data type implements a timer
**************************************************************************/
// typedef double timer;
// timer    SmatrixTimer;


int  createsparse(Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: creates sparse representation of coeff. matrix
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Smatrix *sm = &pr->hydraul.smatrix;

    int errcode = 0;

//    cleartimer(SmatrixTimer);
//    starttimer(SmatrixTimer);

    // Allocate sparse matrix data structures
    errcode = allocsmatrix(sm, net->Nnodes, net->Nlinks);
    if (errcode) return errcode;

    // Build a local version of node-link adjacency lists
    // with parallel links removed
    errcode = localadjlists(net, sm);
    if (errcode) return errcode;

    // Re-order nodes to minimize number of non-zero coeffs.
    // in factorized solution matrix
    ERRCODE(reordernodes(pr));

    // Factorize solution matrix by updating adjacency lists
    // with non-zero connections due to fill-ins
    sm->Ncoeffs = net->Nlinks;
    ERRCODE(factorize(pr));

    // Allocate memory for sparse storage of positions of non-zero
    // coeffs. and store these positions in vector NZSUB
    ERRCODE(storesparse(pr, net->Njuncs));

    // Free memory used for local adjacency lists and sort
    // row indexes in NZSUB to optimize linsolve()
    freeadjlists(net);
    ERRCODE(sortsparse(sm, net->Njuncs));

    // Allocate memory used by linear eqn. solver
    ERRCODE(alloclinsolve(sm, net->Nnodes));

    // Re-build adjacency lists for future use
    ERRCODE(buildadjlists(net));
    return errcode;
}


int  allocsmatrix(Smatrix *sm, int Nnodes, int Nlinks)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: allocates memory for representing a sparse matrix
**--------------------------------------------------------------
*/
{
    int errcode = 0;

    // Memory for linear eqn. solver allocated in alloclinsolve().
    sm->Aij   = NULL;
    sm->Aii   = NULL;
    sm->F     = NULL;
    sm->temp  = NULL;
    sm->link  = NULL;
    sm->first = NULL;

    // Memory for representing sparse matrix data structure
    sm->Order  = (int *) calloc(Nnodes+1,  sizeof(int));
    sm->Row    = (int *) calloc(Nnodes+1,  sizeof(int));
    sm->Ndx    = (int *) calloc(Nlinks+1,  sizeof(int));
    ERRCODE(MEMCHECK(sm->Order));
    ERRCODE(MEMCHECK(sm->Row));
    ERRCODE(MEMCHECK(sm->Ndx));
    return errcode;
}


int  alloclinsolve(Smatrix *sm, int n)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: allocates memory used by linear eqn. solver.
**--------------------------------------------------------------
*/
{
    int errcode = 0;
    n = n + 1;    // All arrays are 1-based

    sm->Aij   = (double *)calloc(sm->Ncoeffs + 1, sizeof(double));
    sm->Aii   = (double *)calloc(n, sizeof(double));
    sm->F     = (double *)calloc(n, sizeof(double));
    sm->temp  = (double *)calloc(n, sizeof(double));
    sm->link  = (int *)calloc(n, sizeof(int));
    sm->first = (int *)calloc(n, sizeof(int));
    ERRCODE(MEMCHECK(sm->Aij));
    ERRCODE(MEMCHECK(sm->Aii));
    ERRCODE(MEMCHECK(sm->F));
    ERRCODE(MEMCHECK(sm->temp));
    ERRCODE(MEMCHECK(sm->link));
    ERRCODE(MEMCHECK(sm->first));
    return errcode;
}


void  freesparse(Project *pr)
/*
**----------------------------------------------------------------
** Input:   None
** Output:  None
** Purpose: Frees memory used for sparse matrix storage
**----------------------------------------------------------------
*/
{
    Smatrix *sm = &pr->hydraul.smatrix;

//    stoptimer(SmatrixTimer);
//    printf("\n");
//    printf("\n    Processing Time = %7.3f s", gettimer(SmatrixTimer));
//    printf("\n");

    FREE(sm->Order);
    FREE(sm->Row);
    FREE(sm->Ndx);
    FREE(sm->XLNZ);
    FREE(sm->NZSUB);
    FREE(sm->LNZ);

    FREE(sm->Aij);
    FREE(sm->Aii);
    FREE(sm->F);
    FREE(sm->temp);
    FREE(sm->link);
    FREE(sm->first);
}


int  localadjlists(Network *net, Smatrix *sm)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: builds linked list of non-parallel links adjacent to each node
**--------------------------------------------------------------
*/
{
    int    i, j, k;
    int    pmark = 0;     // parallel link marker
    int    errcode = 0;
    Padjlist  alink;

    // Create an array of adjacency lists
    freeadjlists(net);
    net->Adjlist = (Padjlist *)calloc(net->Nnodes + 1, sizeof(Padjlist));
    if (net->Adjlist == NULL) return 101;

    // For each link, update adjacency lists of its end nodes
    for (k = 1; k <= net->Nlinks; k++)
    {
        i = net->Link[k].N1;
        j = net->Link[k].N2;
        pmark = paralink(net, sm, i, j, k);  // Parallel link check

        // Include link in start node i's list
        alink = (struct Sadjlist *) malloc(sizeof(struct Sadjlist));
        if (alink == NULL) return(101);
        if (!pmark) alink->node = j;
        else        alink->node = 0;         // Parallel link marker
        alink->link = k;
        alink->next = net->Adjlist[i];
        net->Adjlist[i] = alink;

        // Include link in end node j's list
        alink = (struct Sadjlist *) malloc(sizeof(struct Sadjlist));
        if (alink == NULL) return(101);
        if (!pmark) alink->node = i;
        else        alink->node = 0;         // Parallel link marker
        alink->link = k;
        alink->next = net->Adjlist[j];
        net->Adjlist[j] = alink;
    }

    // Remove parallel links from adjacency lists
    xparalinks(net);
    return errcode;
}


int  paralink(Network *net, Smatrix *sm, int i, int j, int k)
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
    for (alink = net->Adjlist[i]; alink != NULL; alink = alink->next)
    {
        // Link || to k (same end nodes)
        if (alink->node == j)
        {
            // Assign Ndx entry to this link
            sm->Ndx[k] = alink->link;
            return(1);
        }
    }
    // Ndx entry if link not parallel
    sm->Ndx[k] = k;
    return(0);
}


void  xparalinks(Network *net)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  none
** Purpose: removes parallel links from nodal adjacency lists
**--------------------------------------------------------------
*/
{
    int    i;
    Padjlist    alink,       // Current item in adjacency list
                blink;       // Previous item in adjacency list

    // Scan adjacency list of each node
    for (i = 1; i <= net->Nnodes; i++)
    {
        alink = net->Adjlist[i];               // First item in list
        blink = NULL;
        while (alink != NULL)
        {
            if (alink->node == 0)              // Parallel link marker found
            {
                if (blink == NULL)             // This holds at start of list
                {
                    net->Adjlist[i] = alink->next;
                    free(alink);                // Remove item from list
                    alink = net->Adjlist[i];
                }
                else                           // This holds for interior of list
                {
                    blink->next = alink->next;
                    free(alink);                // Remove item from list
                    alink = blink->next;
                }
            }
            else
            {
                blink = alink;                // Move to next item in list
                alink = alink->next;
            }
        }
    }
}


int   reordernodes(Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns 1 if successful, 0 if not
** Purpose: re-orders nodes to minimize # of non-zeros that
**          will appear in factorized solution matrix
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Smatrix *sm = &pr->hydraul.smatrix;

    int k, knode, m, njuncs, nlinks;
    int delta = -1;
    int nofsub = 0;
    int maxint = INT_MAX;   //defined in limits.h
    int errcode;
    Padjlist alink;

    // Local versions of node adjacency lists
    int *adjncy = NULL;
    int *xadj   = NULL;

    // Work arrays
    int *dhead = NULL;
    int *qsize = NULL;
    int *llist = NULL;
    int *marker = NULL;

    // Default ordering
    for (k = 1; k <= net->Nnodes; k++)
    {
        sm->Row[k] = k;
        sm->Order[k] = k;
    }
    njuncs = net->Njuncs;
    nlinks = net->Nlinks;

    // Allocate memory
    adjncy = (int *) calloc(2*nlinks+1, sizeof(int));
    xadj   = (int *) calloc(njuncs+2, sizeof(int));
    dhead  = (int *) calloc(njuncs+1, sizeof(int));
    qsize  = (int *) calloc(njuncs + 1, sizeof(int));
    llist  = (int *) calloc(njuncs + 1, sizeof(int));
    marker = (int *) calloc(njuncs + 1, sizeof(int));
    if (adjncy && xadj && dhead && qsize && llist && marker)
    {
        // Create local versions of node adjacency lists
        xadj[1] = 1;
        m = 1;
        for (k = 1; k <= njuncs; k++)
        {
            for (alink = net->Adjlist[k]; alink != NULL; alink = alink->next)
            {
                knode = alink->node;
                if (knode > 0 && knode <= njuncs)
                {
                    adjncy[m] = knode;
                    m++;
                }
            }
            xadj[k+1] = m;
        }

        // Generate a multiple minimum degree node re-ordering
        genmmd(&njuncs, xadj, adjncy, sm->Row, sm->Order, &delta,
               dhead, qsize, llist, marker, &maxint, &nofsub);
        errcode = 0;
    }
    else errcode = 101;  //insufficient memory

    // Free memory
    FREE(adjncy);
    FREE(xadj);
    FREE(dhead);
    FREE(qsize);
    FREE(llist);
    FREE(marker);
    return errcode;
}


int factorize(Project *pr)
/*
**--------------------------------------------------------------
** Input:   none
** Output:  returns error code
** Purpose: symbolically factorizes the solution matrix in
**          terms of its adjacency lists
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Smatrix *sm = &pr->hydraul.smatrix;

    int k, knode;
    int errcode = 0;
    Padjlist alink;

    // Find degree of each junction node
    sm->Degree = (int *)calloc(net->Nnodes + 1, sizeof(int));
    if (sm->Degree == NULL) return 101;

    // NOTE: For purposes of node re-ordering, Tanks (nodes with
    //       indexes above Njuncs) have zero degree of adjacency.

    for (k = 1; k <= net->Njuncs; k++)
    {
        for (alink = net->Adjlist[k]; alink != NULL; alink = alink->next)
        {
            if (alink->node > 0) sm->Degree[k]++;
        }
    }

    // Augment each junction's adjacency list to account for
    // new connections created when solution matrix is solved.
    // NOTE: Only junctions (indexes <= Njuncs) appear in solution matrix.
    for (k = 1; k <= net->Njuncs; k++)          // Examine each junction
    {
        knode = sm->Order[k];                   // Re-ordered index
        if (!growlist(pr, knode))               // Augment adjacency list
        {
            errcode = 101;
            break;
        }
        sm->Degree[knode] = 0;                  // In-activate node
    }
    free(sm->Degree);
    return errcode;
}


int  growlist(Project *pr, int knode)
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
    Network *net = &pr->network;
    Smatrix *sm = &pr->hydraul.smatrix;

    int node;
    Padjlist alink;

    // Iterate through all nodes connected to knode
    for (alink = net->Adjlist[knode]; alink != NULL; alink = alink -> next)
    {
        node = alink->node;                   // End node of connecting link
        if (node > 0 && sm->Degree[node] > 0) // End node is active
        {
            sm->Degree[node]--;           // Reduce degree of adjacency
            if (!newlink(pr, alink))      // Add to adjacency list
            {
                return 0;
            }
        }
  }
  return 1;
}


int  newlink(Project *pr, Padjlist alink)
/*
**--------------------------------------------------------------
** Input:   alink = element of node's adjacency list
** Output:  returns 1 if successful, 0 if not
** Purpose: links end of current adjacent link to end nodes of
**          all links that follow it on adjacency list
**--------------------------------------------------------------
*/
{
    Network *net = &pr->network;
    Smatrix *sm = &pr->hydraul.smatrix;

    int inode, jnode;
    Padjlist blink;

    // Scan all entries in adjacency list that follow anode.
    inode = alink->node;             // End node of connection to anode
    for (blink = alink->next; blink != NULL; blink = blink->next)
    {
        jnode = blink->node;          // End node of next connection

        // If jnode still active, and inode not connected to jnode,
        // then add a new connection between inode and jnode.
        if (jnode > 0 && sm->Degree[jnode] > 0)  // jnode still active
        {
            if (!linked(net, inode, jnode))      // inode not linked to jnode
            {
                // Since new connection represents a non-zero coeff.
                // in the solution matrix, update the coeff. count.
                sm->Ncoeffs++;

                // Update adjacency lists for inode & jnode to
                // reflect the new connection.
                if (!addlink(net, inode, jnode, sm->Ncoeffs)) return 0;
                if (!addlink(net, jnode, inode, sm->Ncoeffs)) return 0;
                sm->Degree[inode]++;
                sm->Degree[jnode]++;
            }
        }
    }
    return 1;
}


int  linked(Network *net, int i, int j)
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
    for (alink = net->Adjlist[i]; alink != NULL; alink = alink->next)
    {
        if (alink->node == j) return 1;
    }
    return 0;
}


int  addlink(Network *net, int i, int j, int n)
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
    alink = (struct Sadjlist *) malloc(sizeof(struct Sadjlist));
    if (alink == NULL) return 0;
    alink->node = j;
    alink->link = n;
    alink->next = net->Adjlist[i];
    net->Adjlist[i] = alink;
    return 1;
}


int  storesparse(Project *pr, int n)
/*
**--------------------------------------------------------------
** Input:   n = number of rows in solution matrix
** Output:  returns error code
** Purpose: stores row indexes of non-zeros of each column of
**          lower triangular portion of factorized matrix
**--------------------------------------------------------------
*/
{
    Network  *net = &pr->network;
    Smatrix  *sm = &pr->hydraul.smatrix;

    int i, ii, j, k, l, m;
    int errcode = 0;
    Padjlist alink;

    // Allocate sparse matrix storage
    sm->XLNZ  = (int *) calloc(n+2, sizeof(int));
    sm->NZSUB = (int *) calloc(sm->Ncoeffs+2, sizeof(int));
    sm->LNZ   = (int *) calloc(sm->Ncoeffs+2, sizeof(int));
    ERRCODE(MEMCHECK(sm->XLNZ));
    ERRCODE(MEMCHECK(sm->NZSUB));
    ERRCODE(MEMCHECK(sm->LNZ));
    if (errcode) return errcode;

    // Generate row index pointers for each column of matrix
    k = 0;
    sm->XLNZ[1] = 1;
    for (i = 1; i <= n; i++)            // column
    {
        m = 0;
        ii = sm->Order[i];
        for (alink = net->Adjlist[ii]; alink != NULL; alink = alink->next)
        {
            if (alink->node == 0) continue;
            j = sm->Row[alink->node];    // row
            l = alink->link;
            if (j > i && j <= n)
            {
                m++;
                k++;
                sm->NZSUB[k] = j;
                sm->LNZ[k] = l;
            }
        }
        sm->XLNZ[i+1] = sm->XLNZ[i] + m;
    }
    return errcode;
}


int  sortsparse(Smatrix *sm, int n)
/*
**--------------------------------------------------------------
** Input:   n = number of rows in solution matrix
** Output:  returns eror code
** Purpose: puts row indexes in ascending order in NZSUB
**--------------------------------------------------------------
*/
{
    int  i, k;
    int  *xlnzt, *nzsubt, *lnzt, *nzt;
    int  errcode = 0;

    int *LNZ = sm->LNZ;
    int *XLNZ = sm->XLNZ;
    int *NZSUB = sm->NZSUB;

    xlnzt  = (int *) calloc(n+2, sizeof(int));
    nzsubt = (int *) calloc(sm->Ncoeffs+2, sizeof(int));
    lnzt   = (int *) calloc(sm->Ncoeffs+2, sizeof(int));
    nzt    = (int *) calloc(n+2, sizeof(int));
    ERRCODE(MEMCHECK(xlnzt));
    ERRCODE(MEMCHECK(nzsubt));
    ERRCODE(MEMCHECK(lnzt));
    ERRCODE(MEMCHECK(nzt));
    if (!errcode)
    {
        // Count # non-zeros in each row
        for (i = 1; i <= n; i++) nzt[i] = 0;
        for (i = 1; i <= n; i++)
        {
            for (k = XLNZ[i]; k < XLNZ[i+1]; k++) nzt[NZSUB[k]]++;
        }
        xlnzt[1] = 1;
        for (i = 1; i <= n; i++) xlnzt[i+1] = xlnzt[i] + nzt[i];

        // Transpose matrix twice to order column indexes
        transpose(n, XLNZ, NZSUB, LNZ, xlnzt, nzsubt, lnzt, nzt);
        transpose(n, xlnzt, nzsubt, lnzt, XLNZ, NZSUB, LNZ, nzt);
    }

    // Reclaim memory
    free(xlnzt);
    free(nzsubt);
    free(lnzt);
    free(nzt);
    return errcode;
}


void  transpose(int n, int *il, int *jl, int *xl, int *ilt, int *jlt,
                int *xlt, int *nzt)
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
    int  i, j, k, kk;

    for (i = 1; i <= n; i++) nzt[i] = 0;
    for (i = 1; i <= n; i++)
    {
        for (k = il[i]; k < il[i+1]; k++)
        {
            j = jl[k];
            kk = ilt[j] + nzt[j];
            jlt[kk] = i;
            xlt[kk] = xl[k];
            nzt[j]++;
        }
    }
}


int  linsolve(Smatrix *sm, int n)
/*
**--------------------------------------------------------------
** Input:   sm   = sparse matrix struct
            n    = number of equations
** Output:  sm->F = solution values
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
    double *Aii  = sm->Aii;
    double *Aij  = sm->Aij;
    double *B    = sm->F;
    double *temp = sm->temp;
    int *LNZ     = sm->LNZ;
    int *XLNZ    = sm->XLNZ;
    int *NZSUB   = sm->NZSUB;
    int *link    = sm->link;
    int *first   = sm->first;

    int    i, istop, istrt, isub, j, k, kfirst, newk;
    double bj, diagj, ljk;

    memset(temp,  0, (n + 1) * sizeof(double));
    memset(link,  0, (n + 1) * sizeof(int));
    memset(first, 0, (n + 1) * sizeof(int));

   // Begin numerical factorization of matrix A into L
   //   Compute column L(*,j) for j = 1,...n
   for (j = 1; j <= n; j++)
   {
      // For each column L(*,k) that affects L(*,j):
      diagj = 0.0;
      newk = link[j];
      k = newk;
      while (k != 0)
      {
         // Outer product modification of L(*,j) by
         // L(*,k) starting at first[k] of L(*,k)
         newk = link[k];
         kfirst = first[k];
         ljk = Aij[LNZ[kfirst]];
         diagj += ljk*ljk;
         istrt = kfirst + 1;
         istop = XLNZ[k+1] - 1;
         if (istop >= istrt)
         {

	     // Before modification, update vectors 'first'
	     // and 'link' for future modification steps
            first[k] = istrt;
            isub = NZSUB[istrt];
            link[k] = link[isub];
            link[isub] = k;

	    // The actual mod is saved in vector 'temp'
            for (i = istrt; i <= istop; i++)
            {
               isub = NZSUB[i];
               temp[isub] += Aij[LNZ[i]]*ljk;
            }
         }
         k = newk;
      }

      // Apply the modifications accumulated
      // in 'temp' to column L(*,j)
      diagj = Aii[j] - diagj;
      if (diagj <= 0.0)        // Check for ill-conditioning
      {
         return j;
      }
      diagj = sqrt(diagj);
      Aii[j] = diagj;
      istrt = XLNZ[j];
      istop = XLNZ[j+1] - 1;
      if (istop >= istrt)
      {
         first[j] = istrt;
         isub = NZSUB[istrt];
         link[j] = link[isub];
         link[isub] = j;
         for (i = istrt; i <= istop; i++)
         {
            isub = NZSUB[i];
            bj = (Aij[LNZ[i]] - temp[isub])/diagj;
            Aij[LNZ[i]] = bj;
            temp[isub] = 0.0;
         }
      }
   }      // next j

   // Foward substitution
   for (j = 1; j <= n; j++)
   {
      bj = B[j]/Aii[j];
      B[j] = bj;
      istrt = XLNZ[j];
      istop = XLNZ[j+1] - 1;
      if (istop >= istrt)
      {
         for (i = istrt; i <= istop; i++)
         {
            isub = NZSUB[i];
            B[isub] -= Aij[LNZ[i]]*bj;
         }
      }
   }

   // Backward substitution
   for (j = n; j >= 1; j--)
   {
      bj = B[j];
      istrt = XLNZ[j];
      istop = XLNZ[j+1] - 1;
      if (istop >= istrt)
      {
         for (i = istrt; i <= istop; i++)
         {
            isub = NZSUB[i];
            bj -= Aij[LNZ[i]]*B[isub];
         }
      }
      B[j] = bj/Aii[j];
   }
   return 0;
}
