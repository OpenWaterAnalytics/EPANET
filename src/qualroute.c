/*
*********************************************************************

QUALROUTE.C -- water quality routing module for the EPANET program

*********************************************************************
*/

#include <stdio.h>
#include <math.h>
#include "mempool.h"
#include "types.h"

// Macro to compute the volume of a link
#define LINKVOL(k) (0.785398 * net->Link[(k)].Len * SQR(net->Link[(k)].Diam))
// Macro to get link flow compatible with flow saved to hydraulics file
#define LINKFLOW(k) ((hyd->LinkStatus[k] <= CLOSED) ? 0.0 : hyd->LinkFlows[k])

// Exported Functions
int     buildilists(EN_Project *pr);
int     sortnodes(EN_Project *pr);
void    transport(EN_Project *pr, long);
void    initsegs(EN_Project *pr);
void    reversesegs(EN_Project *pr, int);
void    addseg(EN_Project *pr, int, double, double);

// Imported Functions
extern double  findsourcequal(EN_Project *pr, int, double, double, long);
extern void    reactpipes(EN_Project *pr, long);
extern void    reacttanks(EN_Project *pr, long);
extern double  mixtank(EN_Project *pr, int, double, double, double);

// Local Functions
static void    evalnodeinflow(EN_Project *pr, int, long, double *, double *);
static void    evalnodeoutflow(EN_Project *pr, int, double, long);
static double  findnodequal(EN_Project *pr, int, double, double, double, long);
static double  noflowqual(EN_Project *pr, int);
static void    updatemassbalance(EN_Project *pr, int, double, double, long);
static int     selectnonstacknode(EN_Project *pr, int, int *);


void transport(EN_Project *pr, long tstep)
/*
**--------------------------------------------------------------
**   Input:   tstep = length of current time step
**   Output:  none
**   Purpose: transports constituent mass through the pipe network
**            under a period of constant hydraulic conditions.
**--------------------------------------------------------------
*/
{
    int i, j, k, m, n;
    double volin, massin, volout, nodequal;

    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;
    EN_Network   *net = &pr->network;

    // React contents of each pipe and tank
    if (qual->Reactflag)
    {
        reactpipes(pr, tstep);
        reacttanks(pr, tstep);
    }

    // Analyze each node in topological order
    for (j = 1; j <= net->Nnodes; j++)
    {
        // ... index of node to be processed
        n = qual->SortedNodes[j];

        // ... zero out mass & flow volumes for this node
        volin = 0.0;
        massin = 0.0;
        volout = 0.0;

        // ... examine each link with flow into the node
        for (i = qual->IlistPtr[n]; i < qual->IlistPtr[n + 1]; i++)
        {
            // ... k is index of next link incident on node n
            k = qual->Ilist[i];

            // ... link has flow into node - add it to node's inflow
            //     (m is index of link's downstream node)
            m = net->Link[k].N2;
            if (qual->FlowDir[k] < 0) m = net->Link[k].N1;
            if (m == n)
            {
                evalnodeinflow(pr, k, tstep, &volin, &massin);
            }

            // ... link has flow out of node - add it to node's outflow
            else volout += fabs(LINKFLOW(k));
        }

        // ... if node is a junction, add on any external outflow (e.g., demands)
        if (net->Node[n].Type == EN_JUNCTION)
        {
            volout += MAX(0.0, hyd->NodeDemand[n]);
        }

        // ... convert from outflow rate to volume
        volout *= tstep;

        // ... find the concentration of flow leaving the node
        nodequal = findnodequal(pr, n, volin, massin, volout, tstep);

        // ... examine each link with flow out of the node
        for (i = qual->IlistPtr[n]; i < qual->IlistPtr[n + 1]; i++)
        {
            // ... link k incident on node n has upstream node m equal to n
            k = qual->Ilist[i];
            m = net->Link[k].N1;
            if (qual->FlowDir[k] < 0) m = net->Link[k].N2;
            if (m == n)
            {
                // ... send flow at new node concen. into link
                evalnodeoutflow(pr, k, nodequal, tstep);
            }
        }
        updatemassbalance(pr, n, massin, volout, tstep);
    }
}

void  evalnodeinflow(EN_Project *pr, int k, long tstep, double *volin,
    double *massin)
    /*
    **--------------------------------------------------------------
    **   Input:   k = link index
    **            tstep = quality routing time step
    **   Output:  volin = flow volume entering a node
    **            massin = constituent mass entering a node
    **   Purpose: adds the contribution of a link's outflow volume
    **            and constituent mass to the total inflow into its
    **            downstream node over a time step.
    **--------------------------------------------------------------
    */
{
    double q, v, vseg;
    Pseg seg;

    EN_Network *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t *qual = &pr->quality;

    // Get flow rate (q) and flow volume (v) through link
    q = LINKFLOW(k);
    v = fabs(q) * tstep;

    // Transport flow volume v from link's leading segments into downstream
    // node, removing segments once their full volume is consumed
    while (v > 0.0)
    {
        seg = qual->FirstSeg[k];
        if (!seg) break;

        // ... volume transported from first segment is smaller of
        //     remaining flow volume & segment volume
        vseg = seg->v;
        vseg = MIN(vseg, v);

        // ... update total volume & mass entering downstream node
        *volin += vseg;
        *massin += vseg * seg->c;

        // ... reduce remaining flow volume by amount transported
        v -= vseg;

        // ... if all of segment's volume was transferred
        if (v >= 0.0 && vseg >= seg->v)
        {
            // ... replace this leading segment with the one behind it
            qual->FirstSeg[k] = seg->prev;
            if (qual->FirstSeg[k] == NULL) qual->LastSeg[k] = NULL;

            // ... recycle the used up segment
            seg->prev = qual->FreeSeg;
            qual->FreeSeg = seg;
        }

        // ... otherwise just reduce this segment's volume
        else seg->v -= vseg;
    }
}


double  findnodequal(EN_Project *pr, int n, double volin,
    double massin, double volout, long tstep)
    /*
    **--------------------------------------------------------------
    **   Input:   n = node index
    **            volin = flow volume entering node
    **            massin = mass entering node
    **            volout = flow volume leaving node
    **            tstep = length of current time step
    **   Output:  returns water quality in a node's outflow
    **   Purpose: computes a node's new quality from its inflow
    **            volume and mass, including any source contribution.
    **--------------------------------------------------------------
    */
{
    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;

    // Node is a junction - update its water quality
    if (net->Node[n].Type == EN_JUNCTION)
    {
        // ... dilute inflow with any external negative demand
        volin -= MIN(0.0, hyd->NodeDemand[n]) * tstep;

        // ... new concen. is mass inflow / volume inflow
        if (volin > 0.0) qual->NodeQual[n] = massin / volin;

        // ... if no inflow adjust quality for reaction in connecting pipes
        else if (qual->Reactflag) qual->NodeQual[n] = noflowqual(pr, n);
    }

    // Node is a tank - use its mixing model to update its quality
    else if (net->Node[n].Type == EN_TANK)
    {
        qual->NodeQual[n] = mixtank(pr, n, volin, massin, volout);
    }

    // Add any external quality source onto node's concen.
    qual->SourceQual = 0.0;

    // For source tracing analysis find tracer added at source node
    if (qual->Qualflag == TRACE)
    {
        if (n == qual->TraceNode)
        {
            // ... quality added to network is difference between tracer
            //     concentration (100 mg/L) and current node quality
            if (net->Node[n].Type == EN_RESERVOIR) qual->SourceQual = 100.0;
            else qual->SourceQual = MAX(100.0 - qual->NodeQual[n], 0.0);
            qual->NodeQual[n] = 100.0;
        }
        return qual->NodeQual[n];
    }

    // Find quality contribued by any external chemical source
    else qual->SourceQual = findsourcequal(pr, n, volin, volout, tstep);
    if (qual->SourceQual == 0.0) return qual->NodeQual[n];

    // Combine source quality with node quality
    switch (net->Node[n].Type)
    {
    case EN_JUNCTION:
        qual->NodeQual[n] += qual->SourceQual;
        return qual->NodeQual[n];

    case EN_TANK:
        return qual->NodeQual[n] + qual->SourceQual;

    case EN_RESERVOIR:
        qual->NodeQual[n] = qual->SourceQual;
        return qual->SourceQual;
    }
    return qual->NodeQual[n];
}


double  noflowqual(EN_Project *pr, int n)
/*
**--------------------------------------------------------------
**   Input:   n = node index
**   Output:  quality for node n
**   Purpose: sets the quality for a junction node that has no
**            inflow to the average of the quality in its
**            adjoining link segments.
**   Note:    this function is only used for reactive substances.
**--------------------------------------------------------------
*/
{
    int i, k, inflow, kount = 0;
    double c = 0.0;
    FlowDirection dir;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;

    // Examine each link incident on the node
    for (i = qual->IlistPtr[n]; i < qual->IlistPtr[n + 1]; i++)
    {
        // ... index of an incident link
        k = qual->Ilist[i];
        dir = qual->FlowDir[k];

        // Node n is link's downstream node - add quality 
        // of link's first segment to average
        if (net->Link[k].N2 == n && dir >= 0) inflow = TRUE;
        else if (net->Link[k].N1 == n && dir < 0)  inflow = TRUE;
        else                                       inflow = FALSE;
        if (inflow == TRUE && qual->FirstSeg[k] != NULL)
        {
            c += qual->FirstSeg[k]->c;
            kount++;
        }

        // Node n is link's upstream node - add quality 
        // of link's last segment to average
        else if (inflow == FALSE && qual->LastSeg[k] != NULL)
        {
            c += qual->LastSeg[k]->c;
            kount++;
        }
    }
    if (kount > 0) c = c / (double)kount;
    return c;
}


void evalnodeoutflow(EN_Project *pr, int k, double c, long tstep)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**            c = quality from upstream node
**            tstep = time step
**   Output:  none
**   Purpose: releases flow volume and mass from the upstream
**            node of a link over a time step.
**--------------------------------------------------------------
*/
{
    double v;
    Pseg seg;

    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;

    // Find flow volume (v) released over time step
    v = fabs(LINKFLOW(k)) * tstep;
    if (v == 0.0) return;

    // Release flow and mass into upstream end of the link

    // ... case where link has a last (most upstream) segment
    seg = qual->LastSeg[k];
    if (seg)
    {
        // ... if node quality close to segment quality then mix
        //     the nodal outflow volume with the segment's volume
        if (fabs(seg->c - c) < qual->Ctol)
        {
            seg->c = (seg->c*seg->v + c*v) / (seg->v + v);
            seg->v += v;
        }

        // ... otherwise add a new segment at upstream end of link
        else addseg(pr, k, v, c);
    }

    // ... link has no segments so add one
    else addseg(pr, k, v, c);
}


void updatemassbalance(EN_Project *pr, int n, double massin,
    double volout, long tstep)
    /*
    **--------------------------------------------------------------
    **   Input:   n = node index
    **            massin = mass inflow to node
    **            volout = outflow volume from node
    **   Output:  none
    **   Purpose: Adds a node's external mass inflow and outflow
    **            over the current time step to the network's
    **            overall mass balance.
    **--------------------------------------------------------------
    */
{
    double masslost = 0.0,
        massadded = 0.0;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;


    switch (net->Node[n].Type)
    {
        // Junctions lose mass from outflow demand & gain it from source inflow
    case EN_JUNCTION:
        masslost = MAX(0.0, hyd->NodeDemand[n]) * tstep * qual->NodeQual[n];
        massadded = qual->SourceQual * volout;
        break;

        // Reservoirs add mass from quality source if specified or from a fixed
        // initial quality
    case EN_RESERVOIR:
        masslost = massin;
        if (qual->SourceQual > 0.0) massadded = qual->SourceQual * volout;
        else                        massadded = qual->NodeQual[n] * volout;
        break;

        // Tanks add mass only from external source inflow
    case EN_TANK:
        massadded = qual->SourceQual * volout;
        break;
    }
    qual->massbalance.outflow += masslost;
    qual->massbalance.inflow += massadded;
}



int buildilists(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns an error code
**   Purpose: Creates link incidence lists for each node stored
**            in compact form.
**--------------------------------------------------------------
*/
{
    int i, j, k, n, n1, n2;
    int *degree = NULL;

    quality_t    *qual = &pr->quality;
    EN_Network   *net = &pr->network;

    // Allocate an array to count # links incident on each node
    n = net->Nnodes + 1;
    degree = (int *)calloc(n, sizeof(int));
    if (degree == NULL) return 101;

    // Count # links incident on each node
    for (k = 1; k <= net->Nlinks; k++)
    {
        degree[net->Link[k].N1]++;
        degree[net->Link[k].N2]++;
    }

    // Use incidence counts to determine start position of
    // each node's incidence list in Xilist
    qual->IlistPtr[1] = 1;
    for (i = 1; i <= n; i++)
    {
        qual->IlistPtr[i + 1] = qual->IlistPtr[i] + degree[i];
    }

    // Add each link to the incidence lists of its start & end nodes
    for (i = 1; i <= net->Nnodes; i++) degree[i] = 0;
    for (k = 1; k <= net->Nlinks; k++)
    {
        // j is index of next unused location in link's start node list
        n1 = net->Link[k].N1;
        j = qual->IlistPtr[n1] + degree[n1];
        qual->Ilist[j] = k;
        degree[n1]++;

        // Repeat same for end node
        n2 = net->Link[k].N2;
        j = qual->IlistPtr[n2] + degree[n2];
        qual->Ilist[j] = k;
        degree[n2]++;
    }
    free(degree);

    /*//////// QA CHECK
    for (i = 1; i <= net->Nnodes; i++)
    {
    printf("\nNode %s: ", net->Node[i].ID);
    for (j = qual->IlistPtr[i]; j < qual->IlistPtr[i + 1]; j++)
    {
    printf("  %s,", net->Link[qual->Ilist[j]].ID);
    }
    }
    */
    return 0;
}



int sortnodes(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  returns an error code
**   Purpose: topologically sorts nodes from upstream to downstream.
**   Note:    links with negligible flow are ignored since they can
**            create spurious cycles that cause the sort to fail.
**--------------------------------------------------------------
*/
{
    int i, j, k, n;
    int *indegree = NULL;;
    int *stack = NULL;
    int stacksize = 0;
    int numsorted = 0;
    int errcode = 0;
    FlowDirection dir;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;

    // Allocate an array to count # links with inflow to each node
    // and for a stack to hold nodes waiting to be processed
    indegree = (int *)calloc(net->Nnodes + 1, sizeof(int));
    stack = (int *)calloc(net->Nnodes + 1, sizeof(int));
    if (indegree && stack)
    {
        // Count links with "non-negligible" inflow to each node
        for (k = 1; k <= net->Nlinks; k++)
        {
            dir = qual->FlowDir[k];
            if (dir == POSITIVE) n = net->Link[k].N2;
            else if (dir == NEGATIVE) n = net->Link[k].N1;
            else continue;
            indegree[n]++;
        }

        // Place nodes with no inflow onto a stack
        for (i = 1; i <= net->Nnodes; i++)
        {
            if (indegree[i] == 0)
            {
                stacksize++;
                stack[stacksize] = i;
            }
        }

        // Examine each node on the stack until none are left
        while (numsorted < net->Nnodes)
        {
            // ... if stack is empty then a cycle exists
            if (stacksize == 0)
            {
                //  ... add a non-sorted node connected to a sorted one to stack
                j = selectnonstacknode(pr, numsorted, indegree);
                if (j == 0) break;  // This shouldn't happen.
                indegree[j] = 0;
                stacksize++;
                stack[stacksize] = j;
            }

            // ... make the last node added to the stack the next
            //     in sorted order & remove it from the stack
            i = stack[stacksize];
            stacksize--;
            numsorted++;
            qual->SortedNodes[numsorted] = i;

            // ... for each outflow link from this node reduce the in-degree
            //     of its downstream node
            for (j = qual->IlistPtr[i]; j < qual->IlistPtr[i + 1]; j++)
            {
                // ... k is the index of the next link incident on node i
                k = qual->Ilist[j];

                // ... skip link if flow is negligible
                if (qual->FlowDir[k] == 0) continue;

                // ... link has flow out of node (downstream node n not equal to i)
                n = net->Link[k].N2;
                if (qual->FlowDir[k] < 0) n = net->Link[k].N1;

                // ... reduce degree of node n
                if (n != i && indegree[n] > 0)
                {
                    indegree[n]--;

                    // ... no more degree left so add node n to stack
                    if (indegree[n] == 0)
                    {
                        stacksize++;
                        stack[stacksize] = n;
                    }
                }
            }
        }
    }
    else errcode = 101;
    if (numsorted < net->Nnodes) errcode = 120;
    FREE(indegree);
    FREE(stack);
    /*
    /////////////////// QA CHECK
    snprintf(pr->Msg, MAXMSG, "\n\nSorted Nodes:");
    writeline(pr, pr->Msg);
    for (i = 1; i <= net->Nnodes; i++)
    {
    j = qual->SortedNodes[i];
    snprintf(pr->Msg, MAXMSG, "%s", net->Node[j].ID);
    writeline(pr, pr->Msg);
    }
    //printf("\n");
    //system("pause");
    /////////////////
    */
    return errcode;
}


int selectnonstacknode(EN_Project *pr, int numsorted, int *indegree)
/*
**--------------------------------------------------------------
**   Input:   numsorted = number of nodes that have been sorted
**            indegree = number of inflow links to each node
**   Output:  returns a node index
**   Purpose: selects a next node for sorting when a cycle exists.
**--------------------------------------------------------------
*/
{
    int i, j, k, m, n;

    quality_t    *qual = &pr->quality;
    EN_Network   *net = &pr->network;

    // Examine each sorted node in last in - first out order
    for (i = numsorted; i > 0; i--)
    {
        // For each link connected to the sorted node
        m = qual->SortedNodes[i];
        for (j = qual->IlistPtr[m]; j < qual->IlistPtr[m + 1]; j++)
        {
            // ... k is index of next link incident on node m
            k = qual->Ilist[j];

            // ... n is the node of link k opposite to node m
            n = net->Link[k].N2;
            if (n == m) n = net->Link[k].N1;

            // ... select node n if it still has inflow links
            if (indegree[n] > 0) return n;
        }
    }

    // If no node was selected by the above process then return the
    // first node that still has inflow links remaining
    for (i = 1; i <= net->Nnodes; i++)
    {
        if (indegree[i] > 0) return i;
    }

    // If all else fails return 0 indicating that no node was selected
    return 0;
}


void initsegs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none
**   Output:  none
**   Purpose: initializes water quality volume segments in each
**            pipe and tank.
**--------------------------------------------------------------
*/
{
    int j, k;
    double c, v, v1;

    EN_Network   *net = &pr->network;
    hydraulics_t *hyd = &pr->hydraulics;
    quality_t    *qual = &pr->quality;

    // Add one segment with assigned downstream node quality to each pipe
    for (k = 1; k <= net->Nlinks; k++)
    {
        qual->FirstSeg[k] = NULL;
        qual->LastSeg[k] = NULL;
        if (net->Link[k].Type == EN_PIPE)
        {
            v = LINKVOL(k);
            j = net->Link[k].N2;
            c = qual->NodeQual[j];
            addseg(pr, k, v, c);
        }
    }

    // Initialize segments in tanks
    for (j = 1; j <= net->Ntanks; j++)
    {
        // Skip reservoirs
        if (net->Tank[j].A == 0.0) continue;

        // Establish initial tank quality & volume
        k = net->Tank[j].Node;
        c = net->Node[k].C0;
        v = net->Tank[j].V0;

        // Create one volume segment for entire tank
        k = net->Nlinks + j;
        qual->FirstSeg[k] = NULL;
        qual->LastSeg[k] = NULL;
        addseg(pr, k, v, c);

        // Create a 2nd segment for the 2-compartment tank model
        if (net->Tank[j].MixModel == MIX2)
        {
            // ... mixing zone segment
            v1 = MAX(0, v - net->Tank[j].V1max);
            qual->FirstSeg[k]->v = v1;

            // ... stagnant zone segment
            v = v - v1;
            addseg(pr, k, v, c);
        }
    }
}


void reversesegs(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none
**   Purpose: re-orients a link's segments when flow reverses.
**--------------------------------------------------------------
*/
{
    Pseg   seg, nseg, pseg;
    quality_t *qual = &pr->quality;

    seg = qual->FirstSeg[k];
    qual->FirstSeg[k] = qual->LastSeg[k];
    qual->LastSeg[k] = seg;
    pseg = NULL;
    while (seg != NULL)
    {
        nseg = seg->prev;
        seg->prev = pseg;
        pseg = seg;
        seg = nseg;
    }
}


void addseg(EN_Project *pr, int k, double v, double c)
/*
**-------------------------------------------------------------
**   Input:   k = segment chain index
**            v = segment volume
**            c = segment quality
**   Output:  none
**   Purpose: adds a segment to the start of a link
**            upstream of its current last segment.
**-------------------------------------------------------------
*/
{
    Pseg seg;
    quality_t *qual = &pr->quality;

    // Grab the next free segment from the segment pool if available
    if (qual->FreeSeg != NULL)
    {
        seg = qual->FreeSeg;
        qual->FreeSeg = seg->prev;
    }

    // Otherwise allocate a new segment
    else
    {
        seg = (struct Sseg *) mempool_alloc(qual->SegPool, sizeof(struct Sseg));
        if (seg == NULL)
        {
            qual->OutOfMemory = TRUE;
            return;
        }
    }

    // Assign volume and quality to the segment
    seg->v = v;
    seg->c = c;

    // Add the new segment to the end of the segment chain
    seg->prev = NULL;
    if (qual->FirstSeg[k] == NULL) qual->FirstSeg[k] = seg;
    if (qual->LastSeg[k] != NULL)  qual->LastSeg[k]->prev = seg;
    qual->LastSeg[k] = seg;
}
