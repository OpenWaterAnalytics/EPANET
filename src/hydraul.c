/*
*********************************************************************
                                                                   
HYDRAUL.C --  Hydraulic Simulator for EPANET Program         
                                                                   
VERSION:    2.00
DATE:       6/5/00
            9/7/00
            10/25/00
            12/29/00
            3/1/01
            11/19/01
            6/24/02
            8/15/07    (2.00.11)
            2/14/08    (2.00.12)
AUTHOR:     L. Rossman
            US EPA - NRMRL
                                                                   
  This module contains the network hydraulic simulator.            
  It simulates the network's hydraulic behavior over an            
  an extended period of time and writes its results to the         
  binary file HydFile.
                                             
  The entry points for this module are:
     openhyd()    -- called from ENopenH() in EPANET.C
     inithyd()    -- called from ENinitH() in EPANET.C
     runhyd()     -- called from ENrunH() in EPANET.C
     nexthyd()    -- called from ENnextH() in EPANET.C
     closehyd()   -- called from ENcloseH() in EPANET.C
     tankvolume() -- called from ENsetnodevalue() in EPANET.C
     setlinkstatus(),
     setlinksetting() -- all called from ENsetlinkvalue() in EPANET.C

  External functions called by this module are:
     createsparse() -- see SMATRIX.C
     freesparse()   -- see SMATRIX.C
     resistcoeff()  -- see HYDCOEFFS.C
     hydsolve()     -- see HYDSOLVER.C
     checkrules()   -- see RULES.C
     interp()       -- see EPANET.C
     savehyd()      -- see OUTPUT.C
     savehydstep()  -- see OUTPUT.C
     writehydstat() -- see REPORT.C
     writehyderr()  -- see REPORT.C
     writehydwarn() -- see REPORT.C
*******************************************************************
*/

#include <stdio.h>
#include <string.h>
#ifndef __APPLE__
#include <malloc.h>
#else
#include <stdlib.h>
#endif
#include <math.h>
#include "text.h"
#include "types.h"
#include "funcs.h"

#define   QZERO  1.e-6  /* Equivalent to zero flow */

// Local functions
int     allocmatrix(Project *pr);
void    freematrix(Project *pr);
void    initlinkflow(Project *pr, int, char, double);
void    setlinkflow(Project *pr, int, double);
void    demands(Project *pr);
int     controls(Project *pr);
long    timestep(Project *pr);
void    controltimestep(Project *pr, long *);
void    ruletimestep(Project *pr, long *);
void    addenergy(Project *pr, long);
void    tanklevels(Project *pr, long);


int  openhyd(Project *pr)
/*
 *--------------------------------------------------------------
 *  Input:   none     
 *  Output:  returns error code                                          
 *  Purpose: opens hydraulics solver system 
 *--------------------------------------------------------------
*/
{
    int  i;
    int  errcode = 0;
    ERRCODE(createsparse(pr));     /* See SMATRIX.C  */
    ERRCODE(allocmatrix(pr));      /* Allocate solution matrices */
    for (i=1; i <= pr->network.Nlinks; i++) {   /* Initialize flows */
        Slink *link = &pr->network.Link[i];
        initlinkflow(pr, i, link->Stat, link->Kc);
    }
    return(errcode);
}


/*** Updated 3/1/01 ***/
void inithyd(Project *pr, int initflag)
/*
**--------------------------------------------------------------
**  Input:   initflag > 0 if link flows should be re-initialized
**                    = 0 if not
**  Output:  none                                          
**  Purpose: initializes hydraulics solver system 
**--------------------------------------------------------------
*/
{
    int i,j;

   time_options_t *time = &pr->time_options;
   EN_Network *net = &pr->network;
   hydraulics_t *hyd = &pr->hydraulics;
   out_file_t *out = &pr->out_files;
   Stank *tank;
   Slink *link;
   Spump *pump;
  
    /* Initialize tanks */
    for (i=1; i <= net->Ntanks; i++) {
        tank = &net->Tank[i];
        tank->V = tank->V0;
        hyd->NodeHead[tank->Node] = tank->H0;
        hyd->NodeDemand[tank->Node] = 0.0;
        hyd->OldStat[net->Nlinks+i] = TEMPCLOSED;
    }

    /* Initialize emitter flows */
    memset(hyd->EmitterFlows,0,(net->Nnodes+1)*sizeof(double));
    for (i=1; i <= net->Njuncs; i++) {
        if (net->Node[i].Ke > 0.0) { 
            hyd->EmitterFlows[i] = 1.0;
        }
    }

    /* Initialize links */
    for (i=1; i <= net->Nlinks; i++) {
        link = &net->Link[i];
        
        /* Initialize status and setting */
        hyd->LinkStatus[i] = link->Stat;
        hyd->LinkSetting[i] = link->Kc;

        /* Compute flow resistance */
        resistcoeff(pr, i);

        /* Start active control valves in ACTIVE position */                     
        if (
            (link->Type == EN_PRV || link->Type == EN_PSV
            || link->Type == EN_FCV) && (link->Kc != MISSING)
        ) hyd->LinkStatus[i] = ACTIVE;                                                      

/*** Updated 3/1/01 ***/
       /* Initialize flows if necessary */
       if (hyd->LinkStatus[i] <= CLOSED) hyd->LinkFlows[i] = QZERO;
       else if (ABS(hyd->LinkFlows[i]) <= QZERO || initflag > 0)
           initlinkflow(pr, i, hyd->LinkStatus[i], hyd->LinkSetting[i]);

       /* Save initial status */
       hyd->OldStat[i] = hyd->LinkStatus[i];
    }

    /* Initialize pump energy usage */
    for (i=1; i <= net->Npumps; i++)
    {
        pump = &net->Pump[i];
        for (j = 0; j < MAX_ENERGY_STATS; j++) {
            pump->Energy[j] = 0.0;
        }
    }

    /* Re-position hydraulics file */
    if (pr->save_options.Saveflag) { 
        fseek(out->HydFile,out->HydOffset,SEEK_SET);
    }

/*** Updated 3/1/01 ***/
    /* Initialize current time */
    hyd->Haltflag = 0;
    time->Htime = 0;
    time->Hydstep = 0;
    time->Rtime = time->Rstep;
}


int   runhyd(Project *pr, long *t)
/*
**--------------------------------------------------------------
**  Input:   none     
**  Output:  t = pointer to current time (in seconds)
**  Returns: error code                                          
**  Purpose: solves network hydraulics in a single time period 
**--------------------------------------------------------------
*/
{
    int   iter;                          /* Iteration count   */
    int   errcode;                       /* Error code        */
    double relerr;                       /* Solution accuracy */
  
   hydraulics_t     *hyd = &pr->hydraulics;
   time_options_t   *time = &pr->time_options;
   report_options_t *rep = &pr->report;

    /* Find new demands & control actions */
    *t = time->Htime;
    demands(pr);
    controls(pr);

    /* Solve network hydraulic equations */
    errcode = hydsolve(pr,&iter,&relerr);

    if (!errcode) {
        /* Report new status & save results */
        if (rep->Statflag) {
            writehydstat(pr,iter,relerr);
        }
     
/*** Updated 3/1/01 ***/
        /* If system unbalanced and no extra trials */
        /* allowed, then activate the Haltflag.     */
        if (relerr > hyd->Hacc && hyd->ExtraIter == -1) {
            hyd->Haltflag = 1;
        }

        /* Report any warning conditions */
        if (!errcode) {
            errcode = writehydwarn(pr,iter,relerr);
        }
   }
   return(errcode);
}                               /* end of runhyd */


int  nexthyd(Project *pr, long *tstep)
/*
**--------------------------------------------------------------
**  Input:   none     
**  Output:  tstep = pointer to time step (in seconds)
**  Returns: error code                                          
**  Purpose: finds length of next time step & updates tank
**           levels and rule-based contol actions 
**--------------------------------------------------------------
*/
{
   long  hydstep;         /* Actual time step  */
   int   errcode = 0;     /* Error code        */
  
  hydraulics_t *hyd = &pr->hydraulics;
  time_options_t *top = &pr->time_options;

/*** Updated 3/1/01 ***/
   /* Save current results to hydraulics file and */
   /* force end of simulation if Haltflag is active */
  if (pr->save_options.Saveflag) {
     errcode = savehyd(pr,&top->Htime);
  }
  if (hyd->Haltflag) {
    top->Htime = top->Dur;
  }

   /* Compute next time step & update tank levels */
   *tstep = 0;
   hydstep = 0;
  if (top->Htime < top->Dur) {
     hydstep = timestep(pr);
  }
  if (pr->save_options.Saveflag) {
     errcode = savehydstep(pr,&hydstep);
  }

   /* Compute pumping energy */
  if (top->Dur == 0) {
     addenergy(pr,0);
  }
  else if (top->Htime < top->Dur) {
     addenergy(pr,hydstep);
  }

   /* More time remains - update current time. */
   if (top->Htime < top->Dur)
   {
      top->Htime += hydstep;
      if (!pr->quality.OpenQflag)
      {
        if (top->Htime >= top->Rtime) top->Rtime += top->Rstep;
      }
   }

   /* No more time remains - force completion of analysis. */
   else
   {
      top->Htime++;
      if (pr->quality.OpenQflag) pr->quality.Qtime++;
   }
   *tstep = hydstep;
   return(errcode);
}
  

void  closehyd(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none     
**  Output:  returns error code                                          
**  Purpose: closes hydraulics solver system 
**--------------------------------------------------------------
*/
{
   freesparse(pr);           /* see SMATRIX.C */
   freematrix(pr);
}


int  allocmatrix(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns error code                                  
**  Purpose: allocates memory used for solution matrix coeffs.   
**--------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  
   int errcode = 0;
   s->Aii = (double *) calloc(net->Nnodes+1,sizeof(double));
   s->Aij = (double *) calloc(hyd->Ncoeffs+1,sizeof(double));
   s->F   = (double *) calloc(net->Nnodes+1,sizeof(double));
   hyd->DemandFlows = (double *)calloc(net->Nnodes + 1, sizeof(double));
   hyd->EmitterFlows   = (double *) calloc(net->Nnodes+1,sizeof(double));
   s->P   = (double *) calloc(net->Nlinks+1,sizeof(double));
   s->Y   = (double *) calloc(net->Nlinks+1,sizeof(double));
   hyd->X_tmp   = (double *) calloc(MAX((net->Nnodes+1),(net->Nlinks+1)),sizeof(double));
   hyd->OldStat = (StatType *) calloc(net->Nlinks+net->Ntanks+1, sizeof(StatType));
   ERRCODE(MEMCHECK(s->Aii));
   ERRCODE(MEMCHECK(s->Aij));
   ERRCODE(MEMCHECK(s->F));
   ERRCODE(MEMCHECK(hyd->DemandFlows));
   ERRCODE(MEMCHECK(hyd->EmitterFlows));
   ERRCODE(MEMCHECK(s->P));
   ERRCODE(MEMCHECK(s->Y));
   ERRCODE(MEMCHECK(hyd->X_tmp));
   ERRCODE(MEMCHECK(hyd->OldStat));
   return(errcode);
}                               /* end of allocmatrix */


void  freematrix(Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  none                                                
**  Purpose: frees memory used for solution matrix coeffs.       
**--------------------------------------------------------------
*/
{
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  
   free(s->Aii);
   free(s->Aij);
   free(s->F);
   free(hyd->DemandFlows);
   free(hyd->EmitterFlows);
   free(s->P);
   free(s->Y);
   free(hyd->X_tmp);
   free(hyd->OldStat);
}                               /* end of freematrix */


void  initlinkflow(Project *pr, int i, char s, double k)
/*
**--------------------------------------------------------------------
**  Input:   i = link index
**           s = link status
**           k = link setting (i.e., pump speed)
**  Output:  none                                                      
**  Purpose: sets initial flow in link to QZERO if link is closed,
**           to design flow for a pump, or to flow at velocity of
**           1 fps for other links.
**--------------------------------------------------------------------
*/
{
  hydraulics_t *hyd = &pr->hydraulics;
  EN_Network *n = &pr->network;
  Slink *link = &n->Link[i];
  
  if (s == CLOSED) {
    hyd->LinkFlows[i] = QZERO;
  }
  else if (link->Type == EN_PUMP) {
    hyd->LinkFlows[i] = k * n->Pump[findpump(n,i)].Q0;
  }
  else {
    hyd->LinkFlows[i] = PI * SQR(link->Diam)/4.0;
  }
}


/*** Updated 9/7/00 ***/
void  setlinkflow(Project *pr, int k, double dh)
/*
**--------------------------------------------------------------
**  Input:   k = link index
**           dh = headloss across link
**  Output:  none   
**  Purpose: sets flow in link based on current headloss                                              
**--------------------------------------------------------------
*/
{
  int   i,p;
  double h0;
  double  x,y;
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  Slink *link = &net->Link[k];
  Scurve *curve;
  
  switch (link->Type)
  {
    case EN_CVPIPE:
    case EN_PIPE:
      
      /* For Darcy-Weisbach formula: */
      /* use approx. inverse of formula. */
      if (hyd->Formflag == DW) {
        x = -log(hyd->LinkSetting[k]/3.7/link->Diam);
        y = sqrt(ABS(dh)/link->R/1.32547);
        hyd->LinkFlows[k] = x*y;
      }
      
      /* For Hazen-Williams or Manning formulas: */
      /* use inverse of formula. */
      else {
        x = ABS(dh)/link->R;
        y = 1.0/hyd->Hexp;
        hyd->LinkFlows[k] = pow(x,y);
      }
      
      /* Change sign of flow to match sign of headloss */
      if (dh < 0.0) {
        hyd->LinkFlows[k] = -hyd->LinkFlows[k];
      }
      
      break;
      
    case EN_PUMP:
      
      /* Convert headloss to pump head gain */
      dh = -dh;
      p = findpump(net,k);
      
      /* For custom pump curve, interpolate from curve */
      if (net->Pump[p].Ptype == CUSTOM)
      {
        dh = -dh * pr->Ucf[HEAD] / SQR(hyd->LinkSetting[k]);
        i = net->Pump[p].Hcurve;
        curve = &net->Curve[i];
        hyd->LinkFlows[k] = interp(curve->Npts,curve->Y,curve->X,dh) *
                            hyd->LinkSetting[k] / pr->Ucf[FLOW];
      }
      
      /* Otherwise use inverse of power curve */
      else
      {
        h0 = -SQR(hyd->LinkSetting[k])*net->Pump[p].H0;
        x = pow(hyd->LinkSetting[k],2.0 - net->Pump[p].N);
        x = ABS(h0-dh)/(net->Pump[p].R*x),
        y = 1.0/net->Pump[p].N;
        hyd->LinkFlows[k] = pow(x,y);
      }
      break;
    default:
      break;
  }
}


void  setlinkstatus(Project *pr, int index, char value, StatType *s, double *k)
/*----------------------------------------------------------------
**  Input:   index  = link index
**           value  = 0 (CLOSED) or 1 (OPEN)
**           s      = pointer to link status
**           k      = pointer to link setting
**  Output:  none
**  Purpose: sets link status to OPEN or CLOSED 
**----------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  Slink *link = &net->Link[index];
  EN_LinkType t = link->Type;
  
   /* Status set to open */
   if (value == 1) {
      /* Adjust link setting for pumps & valves */
     if (t == EN_PUMP) {
        *k = 1.0;
     }
     if (t > EN_PUMP &&  t != EN_GPV) {
        *k = MISSING;
     }
      /* Reset link flow if it was originally closed */
      *s = OPEN;
   }

   /* Status set to closed */ 
   else if (value == 0) {
      /* Adjust link setting for pumps & valves */
     if (t == EN_PUMP) {
       *k = 0.0;
     }
     if (t >  EN_PUMP &&  t != EN_GPV) {
        *k = MISSING;
     }
      /* Reset link flow if it was originally open */
      *s = CLOSED;
   }
}


void  setlinksetting(Project *pr, int index, double value, StatType *s, double *k)
/*----------------------------------------------------------------
**  Input:   index  = link index
**           value  = pump speed or valve setting
**           s      = pointer to link status
**           k      = pointer to link setting
**  Output:  none
**  Purpose: sets pump speed or valve setting, adjusting link
**           status and flow when necessary
**----------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  Slink *link = &net->Link[index];
  EN_LinkType t = link->Type;
  
   /* For a pump, status is OPEN if speed > 0, CLOSED otherwise */
   if (t == EN_PUMP)
   {
      *k = value;
      if (value > 0 && *s <= CLOSED) {
         *s = OPEN;
      }
      if (value == 0 && *s > CLOSED) {
          *s = CLOSED;
      }
   }

   /* For FCV, activate it */
   else if (t == EN_FCV) {
      *k = value;
      *s = ACTIVE;
   }

   /* Open closed control valve with fixed status (setting = MISSING) */
   else {
      if (*k == MISSING && *s <= CLOSED) {
         *s = OPEN;
      }
      *k = value;
   }
} 


void  demands(Project *pr)
/*
**--------------------------------------------------------------------
**  Input:   none                                                      
**  Output:  none                                                      
**  Purpose: computes demands at nodes during current time period      
**--------------------------------------------------------------------
*/
{
   int i,j,n;
   long k,p;
   double djunc, sum;
   Pdemand demand;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  time_options_t *top = &pr->time_options;

   /* Determine total elapsed number of pattern periods */
   p = (top->Htime + top->Pstart) / top->Pstep;

   /* Update demand at each node according to its assigned pattern */
   hyd->Dsystem = 0.0;          /* System-wide demand */
   for (i=1; i <= net->Njuncs; i++) {
      sum = 0.0;
      for (demand = net->Node[i].D; demand != NULL; demand = demand->next) {
         /*
            pattern period (k) = (elapsed periods) modulus
                                 (periods per pattern)
         */
         j = demand->Pat;
         k = p % (long) net->Pattern[j].Length;
         djunc = (demand->Base) * net->Pattern[j].F[k] * hyd->Dmult;
        if (djunc > 0.0) {
          hyd->Dsystem += djunc;
        }
         sum += djunc;
      }
      hyd->NodeDemand[i] = sum;

      // Initialize pressure dependent demand
      hyd->DemandFlows[i] = sum;
   }

   /* Update head at fixed grade nodes with time patterns. */
   for (n=1; n <= net->Ntanks; n++) {
     Stank *tank = &net->Tank[n];
      if (tank->A == 0.0) {
         j = tank->Pat;
         if (j > 0) {
            k = p % (long) net->Pattern[j].Length;
            i = tank->Node;
            hyd->NodeHead[i] = net->Node[i].El * net->Pattern[j].F[k];
         }
      }
   }

   /* Update status of pumps with utilization patterns */
   for (n=1; n <= net->Npumps; n++)
   {
     Spump *pump = &net->Pump[n];
      j = pump->Upat;
      if (j > 0) {
         i = pump->Link;
         k = p % (long) net->Pattern[j].Length;
         setlinksetting(pr, i, net->Pattern[j].F[k], &hyd->LinkStatus[i], &hyd->LinkSetting[i]);
      }
   }

}                        /* End of demands */


int  controls(Project *pr)
/*
**---------------------------------------------------------------------
**  Input:   none                                                   
**  Output:  number of links whose setting changes                  
**  Purpose: implements simple controls based on time or tank levels  
**---------------------------------------------------------------------
*/
{
   int   i, k, n, reset, setsum;
   double h, vplus;
   double v1, v2;
   double k1, k2;
   char  s1, s2;
   Slink *link;
   Scontrol *control;  

  EN_Network *net = &pr->network;
  time_options_t *top = &pr->time_options;
  hydraulics_t *hyd = &pr->hydraulics;

   /* Examine each control statement */
   setsum = 0;
   for (i=1; i <= net->Ncontrols; i++)
   {
      control = &net->Control[i];
      /* Make sure that link is defined */
      reset = 0;
      if ( (k = control->Link) <= 0) {
        continue;
      }
      link = &net->Link[k];
      /* Link is controlled by tank level */
      if ((n = control->Node) > 0 && n > net->Njuncs)
      {
         h = hyd->NodeHead[n];
         vplus = ABS(hyd->NodeDemand[n]);
         v1 = tankvolume(pr,n - net->Njuncs,h);
         v2 = tankvolume(pr,n - net->Njuncs, control->Grade);
         if (control->Type == LOWLEVEL && v1 <= v2 + vplus)
            reset = 1;
         if (control->Type == HILEVEL && v1 >= v2 - vplus)
            reset = 1;
      }

      /* Link is time-controlled */
      if (control->Type == TIMER)
      {
          if (control->Time == top->Htime) reset = 1;
      }

      /* Link is time-of-day controlled */
      if (control->Type == TIMEOFDAY)
      {
        if ((top->Htime + top->Tstart) % SECperDAY == control->Time) {
          reset = 1;
        }
      }

      /* Update link status & pump speed or valve setting */
      if (reset == 1)
      {
        if (hyd->LinkStatus[k] <= CLOSED) {
          s1 = CLOSED;
        }
        else {
          s1 = OPEN;
        }
         s2 = control->Status;
         k1 = hyd->LinkSetting[k];
         k2 = k1;
         if (link->Type > EN_PIPE) {
           k2 = control->Setting;
         }
         if (s1 != s2 || k1 != k2) {
            hyd->LinkStatus[k] = s2;
            hyd->LinkSetting[k] = k2;
           if (pr->report.Statflag) {
              writecontrolaction(pr,k,i);
           }
            setsum++;
         }
      }   
   }
   return(setsum);
}                        /* End of controls */


long  timestep(Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns time step until next change in hydraulics   
**  Purpose: computes time step to advance hydraulic simulation  
**----------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  time_options_t *time = &pr->time_options;
  
  
  long   n,t,tstep;
  
  /* Normal time step is hydraulic time step */
  tstep = time->Hstep;
  
  /* Revise time step based on time until next demand period */
  n = ((time->Htime + time->Pstart) / time->Pstep) + 1;   /* Next pattern period   */
  t = n * time->Pstep - time->Htime;              /* Time till next period */
  if (t > 0 && t < tstep) {
    tstep = t;
  }
  
  /* Revise time step based on time until next reporting period */
  t = time->Rtime - time->Htime;
  if (t > 0 && t < tstep) {
    tstep = t;
  }
  
  /* Revise time step based on smallest time to fill or drain a tank */
  tanktimestep(pr,&tstep);
  
  /* Revise time step based on smallest time to activate a control */
  controltimestep(pr,&tstep);
  
  /* Evaluate rule-based controls (which will also update tank levels) */
  if (net->Nrules > 0) {
    ruletimestep(pr,&tstep);
  }
  else {
    tanklevels(pr,tstep);
  }
  return(tstep);
}


int  tanktimestep(Project *pr, long *tstep)
/*
**-----------------------------------------------------------------
**  Input:   *tstep = current time step                                                
**  Output:  *tstep = modified current time step   
**  Purpose: revises time step based on shortest time to fill or
**           drain a tank  
**-----------------------------------------------------------------
*/
{
  int    i,n, tankIdx = 0;
  double  h,q,v;
  long   t;
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  
  /* (D[n] is net flow rate into (+) or out of (-) tank at node n) */
  for (i=1; i <= net->Ntanks; i++)
  {
    Stank *tank = &net->Tank[i];
    if (tank->A == 0.0) {
      continue;           /* Skip reservoirs     */
    }
    n = tank->Node;
    h = hyd->NodeHead[n];                                 /* Current tank grade  */
    q = hyd->NodeDemand[n];                                 /* Flow into tank      */
    if (ABS(q) <= QZERO) {
      continue;
    }
    if (q > 0.0 && h < tank->Hmax) {
      v = tank->Vmax - tank->V;          /* Volume to fill      */
    }
    else if (q < 0.0 && h > tank->Hmin) {
      v = tank->Vmin - tank->V;          /* Volume to drain (-) */
    }
    else {
      continue;
    }
    t = (long)ROUND(v/q);                     /* Time to fill/drain  */
    if (t > 0 && t < *tstep) {
      *tstep = t;
      tankIdx = n;
    }
  }
  return tankIdx;
}


void  controltimestep(Project *pr, long *tstep)
/*
**------------------------------------------------------------------
**  Input:   *tstep = current time step                                                
**  Output:  *tstep = modified current time step   
**  Purpose: revises time step based on shortest time to activate
**           a simple control  
**------------------------------------------------------------------
*/
{
   int   i,j,k,n;
   double h,q,v;
   long  t,t1,t2;
   Slink *link;
   Scontrol *control;

  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  
   for (i=1; i <= net->Ncontrols; i++)
   {
      t = 0;
      control = &net->Control[i];
      if ( (n = control->Node) > 0)           /* Node control:       */
      {
        if ((j = n - net->Njuncs) <= 0) {
           continue;     /* Node is a tank      */
        }
         h = hyd->NodeHead[n];                              /* Current tank grade  */
         q = hyd->NodeDemand[n];                              /* Flow into tank      */
        if (ABS(q) <= QZERO) {
          continue;
        }
         if
         ( (h < control->Grade &&
            control->Type == HILEVEL &&       /* Tank below hi level */
            q > 0.0)                            /* & is filling        */
         || (h > control->Grade &&
             control->Type == LOWLEVEL &&     /* Tank above low level */
             q < 0.0)                           /* & is emptying        */
         )
         {                                      /* Time to reach level  */
            v = tankvolume(pr, j, control->Grade) - net->Tank[j].V;
            t = (long)ROUND(v/q);
         }
      }

      if (control->Type == TIMER)             /* Time control:        */
      {              
         if (control->Time > pr->time_options.Htime)
             t = control->Time - pr->time_options.Htime;
      }

      if (control->Type == TIMEOFDAY)         /* Time-of-day control: */
      {
         t1 = (pr->time_options.Htime + pr->time_options.Tstart) % SECperDAY;
         t2 = control->Time;
         if (t2 >= t1)
           t = t2 - t1;
         else
           t = SECperDAY - t1 + t2;
      }

      if (t > 0 && t < *tstep)               /* Revise time step     */
      {
         /* Check if rule actually changes link status or setting */
         k = control->Link;
         link = &net->Link[k];
         if ( (link->Type > EN_PIPE && hyd->LinkSetting[k] != control->Setting)
               || (hyd->LinkStatus[k] != control->Status) ) {
            *tstep = t;
         }
      }
   }
}                        /* End of timestep */


void  ruletimestep(Project *pr, long *tstep)
/*
**--------------------------------------------------------------
**  Input:   *tstep = current time step (sec)                            
**  Output:  *tstep = modified time step                                               
**  Purpose: updates next time step by checking if any rules
**           will fire before then; also updates tank levels.                               
**--------------------------------------------------------------
*/
{
   long tnow,      /* Start of time interval for rule evaluation */
        tmax,      /* End of time interval for rule evaluation   */
        dt,        /* Normal time increment for rule evaluation  */
        dt1;       /* Actual time increment for rule evaluation  */

  EN_Network *net = &pr->network;
  time_options_t *time = &pr->time_options;
  
   /* Find interval of time for rule evaluation */
   tnow = pr->time_options.Htime;
   tmax = tnow + *tstep;

   /* If no rules, then time increment equals current time step */
   if (net->Nrules == 0) {
      dt = *tstep;
      dt1 = dt;
   }

   /* Otherwise, time increment equals rule evaluation time step and */
   /* first actual increment equals time until next even multiple of */
   /* Rulestep occurs. */
   else
   {
      dt = pr->time_options.Rulestep;
      dt1 = pr->time_options.Rulestep - (tnow % pr->time_options.Rulestep);
   }

   /* Make sure time increment is no larger than current time step */
   dt = MIN(dt, *tstep);
   dt1 = MIN(dt1, *tstep);
  if (dt1 == 0) {
    dt1 = dt;
  }

   /* Step through time, updating tank levels, until either  */
   /* a rule fires or we reach the end of evaluation period. */
   /*
   ** Note: we are updating the global simulation time (Htime)
   **       here because it is used by functions in RULES.C
   **       to evaluate rules when checkrules() is called.
   **       It is restored to its original value after the
   **       rule evaluation process is completed (see below).
   **       Also note that dt1 will equal dt after the first
   **       time increment is taken.
   */
   do {
      pr->time_options.Htime += dt1;               /* Update simulation clock */
      tanklevels(pr,dt1);            /* Find new tank levels    */
     if (checkrules(pr,dt1)) {
       break; /* Stop if rules fire      */
     }
      dt = MIN(dt, tmax - time->Htime); /* Update time increment   */
      dt1 = dt;                   /* Update actual increment */
   }  while (dt > 0);             /* Stop if no time left    */

   /* Compute an updated simulation time step (*tstep) */
   /* and return simulation time to its original value */
   *tstep = pr->time_options.Htime - tnow;
   pr->time_options.Htime = tnow;
}
   

void  addenergy(Project *pr, long hstep)
/*
**-------------------------------------------------------------
**  Input:   hstep = time step (sec)                            
**  Output:  none                                               
**  Purpose: accumulates pump energy usage                               
**-------------------------------------------------------------
*/
{
    int   i,j,k;
    long  m,n;
    double c0,c,            /* Energy cost (cost/kwh) */
          f0,               /* Energy cost factor */
          dt,               /* Time interval (hr) */
          e,                /* Pump efficiency (fraction) */
          q,                /* Pump flow (cfs) */
          p,                /* Pump energy (kw) */
          psum = 0.0;       /* Total energy (kw) */
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  Spump *pump;

   /* Determine current time interval in hours */
  if (pr->time_options.Dur == 0) {
     dt = 1.0;
  }
  else if (pr->time_options.Htime < pr->time_options.Dur) {
     dt = (double) hstep / 3600.0;
  }
  else {
     dt = 0.0;
  }
  
  if (dt == 0.0) {
     return;
  }
  n = (pr->time_options.Htime + pr->time_options.Pstart) / pr->time_options.Pstep;

   /* Compute default energy cost at current time */
   c0 = hyd->Ecost;
   f0 = 1.0;
   if (hyd->Epat > 0)
   {
      m = n % (long)net->Pattern[hyd->Epat].Length;
      f0 = net->Pattern[hyd->Epat].F[m];
   }

   /* Examine each pump */
   for (j=1; j <= net->Npumps; j++)
   {
      /* Skip closed pumps */
      pump = &net->Pump[j];
      k = pump->Link;
      if (hyd->LinkStatus[k] <= CLOSED) {
        continue;
      }
      q = MAX(QZERO, ABS(hyd->LinkFlows[k]));

      /* Find pump-specific energy cost */
      if (pump->Ecost > 0.0) {
        c = pump->Ecost;
     }
     else {
       c = c0;
     }

      if ( (i = pump->Epat) > 0) {
          m = n % (long)net->Pattern[i].Length;
          c *= net->Pattern[i].F[m];
      }
      else {
        c *= f0;
      }

      /* Find pump energy & efficiency */
      getenergy(pr,k,&p,&e);
      psum += p;

      /* Update pump's cumulative statistics */
      pump->Energy[PCNT_ONLINE] += dt;
      pump->Energy[PCNT_EFFIC] += e*dt;
      pump->Energy[KWH_PER_FLOW] += p/q*dt;
      pump->Energy[TOTAL_KWH] += p*dt;
      pump->Energy[MAX_KW] = MAX(pump->Energy[MAX_KW],p);
      pump->Energy[TOTAL_COST] += c*p*dt;
   }

   /* Update maximum kw value */
   hyd->Emax = MAX(hyd->Emax,psum);
}                       /* End of pumpenergy */


void  getenergy(Project *pr, int k, double *kw, double *eff)
/*
**----------------------------------------------------------------
**  Input:   k    = link index                         
**  Output:  *kw  = kwatt energy used
**           *eff = efficiency (pumps only)
**  Purpose: computes flow energy associated with link k                                           
**----------------------------------------------------------------
*/
{
   int    i,       // efficiency curve index
          j;       // pump index
   double dh,      // head across pump (ft)
          q,       // flow through pump (cfs)
          e;       // pump efficiency
   double q4eff;   // flow at nominal pump speed of 1.0
   double speed;   // current speed setting
   Scurve *curve;
   
  EN_Network   *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  Slink *link = &net->Link[k];
  
/*** Updated 6/24/02 ***/
   /* No energy if link is closed */
   if (hyd->LinkStatus[k] <= CLOSED)
   {
      *kw = 0.0;
      *eff = 0.0;
      return;
   }
/*** End of update ***/

   /* Determine flow and head difference */
   q = ABS(hyd->LinkFlows[k]);
   dh = ABS(hyd->NodeHead[link->N1] - hyd->NodeHead[link->N2]);

   /* For pumps, find effic. at current flow */
   if (link->Type == EN_PUMP)
   {
      j = findpump(net,k);
      e = hyd->Epump;
      speed = hyd->LinkSetting[k];
      if ( (i = net->Pump[j].Ecurve) > 0)
      {
         q4eff = q / speed * pr->Ucf[FLOW];
         curve = &net->Curve[i];
         e = interp(curve->Npts,curve->X, curve->Y, q4eff);
         /* Sarbu and Borza pump speed adjustment */
         e = 100.0 - ((100.0-e) * pow(1.0/speed, 0.1));
      }
      e = MIN(e, 100.0);
      e = MAX(e, 1.0);
      e /= 100.0;
   }
   else e = 1.0;

   /* Compute energy */
   *kw = dh * q * hyd->SpGrav / 8.814 / e * KWperHP;
   *eff = e;
}


void  tanklevels(Project *pr, long tstep)
/*
**----------------------------------------------------------------
**  Input:   tstep = current time step                         
**  Output:  none                                                
**  Purpose: computes new water levels in tanks after current    
**           time step                                           
**----------------------------------------------------------------
*/
{
  int   i,n;
  double dv;
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  
  for (i=1; i<=net->Ntanks; i++)
  {
    Stank *tank = &net->Tank[i];
    /* Skip reservoirs */
    if (tank->A == 0.0) {
      continue;
    }
    
    /* Update the tank's volume & water elevation */
    n = tank->Node;
    dv = hyd->NodeDemand[n] * tstep;
    tank->V += dv;
    
    /*** Updated 6/24/02 ***/
    /* Check if tank full/empty within next second */
    if (tank->V + hyd->NodeDemand[n] >= tank->Vmax) {
      tank->V = tank->Vmax;
    }
    else if (tank->V - hyd->NodeDemand[n] <= tank->Vmin) {
      tank->V = tank->Vmin;
    }
    hyd->NodeHead[n] = tankgrade(pr, i, tank->V);
  }
}                       /* End of tanklevels */


double  tankvolume(Project *pr, int i, double h)
/*
**--------------------------------------------------------------------
**  Input:   i = tank index                                         
**           h = water elevation in tank                                
**  Output:  returns water volume in tank                           
**  Purpose: finds water volume in tank i corresponding to elev. h. 
**--------------------------------------------------------------------
*/
{
  int j;
  EN_Network *net = &pr->network;
  Stank *tank = &net->Tank[i];
  Scurve *curve;

  /* Use level*area if no volume curve */
  j = tank->Vcurve;
  if (j == 0) {
    return(tank->Vmin + (h - tank->Hmin) * tank->A);
  }
  
  /* If curve exists, interpolate on h to find volume v */
  /* remembering that volume curve is in original units.*/
  else {
    curve = &net->Curve[j];
    return(interp(curve->Npts, curve->X, curve->Y,
                 (h - net->Node[tank->Node].El) *
                 pr->Ucf[HEAD]) / pr->Ucf[VOLUME]);
  }
  
}                       /* End of tankvolume */


double  tankgrade(Project *pr, int i, double v)
/*
**-------------------------------------------------------------------
**  Input:   i = tank index                                         
**           v = volume in tank                                     
**  Output:  returns water level in tank                            
**  Purpose: finds water level in tank i corresponding to volume v. 
**-------------------------------------------------------------------
*/
{
  int j;
  EN_Network *net = &pr->network;
  Stank *tank = &net->Tank[i];
  /* Use area if no volume curve */
  j = tank->Vcurve;
  if (j == 0) {
    return(tank->Hmin + (v - tank->Vmin) / tank->A);
  }
  
  /* If curve exists, interpolate on volume (originally the Y-variable */
  /* but used here as the X-variable) to find new level above bottom.  */
  /* Remember that volume curve is stored in original units.           */
  else {
    Scurve *curve = &net->Curve[j];
    return(net->Node[tank->Node].El + interp(curve->Npts, curve->Y, curve->X, v * pr->Ucf[VOLUME]) / pr->Ucf[HEAD]);
  }
  
}                        /* End of tankgrade */


/****************  END OF HYDRAUL.C  ***************/
