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
     setlinksetting(),
     resistance()-- all called from ENsetlinkvalue() in EPANET.C

  External functions called by this module are:
     createsparse() -- see SMATRIX.C
     freesparse()   -- see SMATRIX.C
     linsolve()     -- see SMATRIX.C
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
#include "hash.h"
#include "text.h"
#include "types.h"
#include "epanet2.h"
#include "funcs.h"
#define  EXTERN  extern
#include "vars.h"

#define   QZERO  1.e-6  /* Equivalent to zero flow */
#define   CBIG   1.e8   /* Big coefficient         */
#define   CSMALL 1.e-6  /* Small coefficient       */

/* Constants used for computing Darcy-Weisbach friction factor */
#define A1  0.314159265359e04  /* 1000*PI */
#define A2  0.157079632679e04  /* 500*PI  */
#define A3  0.502654824574e02  /* 16*PI   */
#define A4  6.283185307        /* 2*PI    */
#define A8  4.61841319859      /* 5.74*(PI/4)^.9 */
#define A9  -8.685889638e-01   /* -2/ln(10)      */
#define AA  -1.5634601348      /* -2*.9*2/ln(10) */
#define AB  3.28895476345e-03  /* 5.74/(4000^.9) */
#define AC  -5.14214965799e-03 /* AA*AB */

/*** Updated 3/1/01 ***/

/* Function to find flow coeffs. through open/closed valves */                 
void valvecoeff(EN_Project *pr, int k);                                                        


int  openhyd(EN_Project *pr)
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
void inithyd(EN_Project *pr, int initflag)
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
  
   /* Initialize tanks */
   for (i=1; i <= net->Ntanks; i++) {
     Stank *tank = &net->Tank[i];
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
     Slink *link = &net->Link[i];
   /* Initialize status and setting */
      hyd->LinkStatus[i] = link->Stat;
      hyd->LinkSetting[i] = link->Kc;

      /* Start active control valves in ACTIVE position */                     
      if (
           (link->Type == EN_PRV || link->Type == EN_PSV
            || link->Type == EN_FCV)                                            
            && (link->Kc != MISSING)
         ) hyd->LinkStatus[i] = ACTIVE;                                                      

/*** Updated 3/1/01 ***/
      /* Initialize flows if necessary */
      if (hyd->LinkStatus[i] <= CLOSED) hyd->LinkFlows[i] = QZERO;
      else if (ABS(hyd->LinkFlows[i]) <= QZERO || initflag > 0)
         initlinkflow(pr, i, hyd->LinkStatus[i], hyd->LinkSetting[i]);

      /* Save initial status */
      hyd->OldStat[i] = hyd->LinkStatus[i];
   }

   /* Reset pump energy usage */
   for (i=1; i <= net->Npumps; i++)
   {
     for (j=0; j<6; j++) { 
       net->Pump[i].Energy[j] = 0.0;
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


int   runhyd(EN_Project *pr, long *t)
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
   double relerr;                        /* Solution accuracy */
  
  hydraulics_t *hyd = &pr->hydraulics;
  time_options_t *time = &pr->time_options;
  report_options_t *rep = &pr->report;

   /* Find new demands & control actions */
   *t = time->Htime;
   demands(pr);
   controls(pr);

   /* Solve network hydraulic equations */
   errcode = netsolve(pr,&iter,&relerr);
   if (!errcode) {
      /* Report new status & save results */
     if (rep->Statflag) {
       writehydstat(pr,iter,relerr);
     }

     /* solution info */
     hyd->relativeError = relerr;
     hyd->iterations = iter;
     
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


int  nexthyd(EN_Project *pr, long *tstep)
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

   /* Update current time. */
   if (top->Htime < top->Dur)  /* More time remains */
   {
      top->Htime += hydstep;
     if (top->Htime >= top->Rtime) {
        top->Rtime += top->Rstep;
     }
   }
   else
   {
      top->Htime++;          /* Force completion of analysis */
      if (pr->quality.OpenQflag) {
        pr->quality.Qtime++; // force completion of wq analysis too
      }
   }
   *tstep = hydstep;
   return(errcode);
}
  

void  closehyd(EN_Project *pr)
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


int  allocmatrix(EN_Project *pr)
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
   hyd->EmitterFlows   = (double *) calloc(net->Nnodes+1,sizeof(double));
   s->P   = (double *) calloc(net->Nlinks+1,sizeof(double));
   s->Y   = (double *) calloc(net->Nlinks+1,sizeof(double));
   hyd->X_tmp   = (double *) calloc(MAX((net->Nnodes+1),(net->Nlinks+1)),sizeof(double));
   hyd->OldStat = (StatType *) calloc(net->Nlinks+net->Ntanks+1, sizeof(StatType));
   ERRCODE(MEMCHECK(s->Aii));
   ERRCODE(MEMCHECK(s->Aij));
   ERRCODE(MEMCHECK(s->F));
   ERRCODE(MEMCHECK(hyd->EmitterFlows));
   ERRCODE(MEMCHECK(s->P));
   ERRCODE(MEMCHECK(s->Y));
   ERRCODE(MEMCHECK(hyd->X_tmp));
   ERRCODE(MEMCHECK(hyd->OldStat));
   return(errcode);
}                               /* end of allocmatrix */


void  freematrix(EN_Project *pr)
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
   free(hyd->EmitterFlows);
   free(s->P);
   free(s->Y);
   free(hyd->X_tmp);
   free(hyd->OldStat);
}                               /* end of freematrix */


void  initlinkflow(EN_Project *pr, int i, char s, double k)
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
void  setlinkflow(EN_Project *pr, int k, double dh)
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
        hyd->LinkFlows[k] = interp(curve->Npts,curve->Y,curve->X,dh) * hyd->LinkSetting[k] / pr->Ucf[FLOW];
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


void  setlinkstatus(EN_Project *pr, int index, char value, StatType *s, double *k)
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


void  setlinksetting(EN_Project *pr, int index, double value, StatType *s, double *k)
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


void  resistance(EN_Project *pr, int k)
/*
**--------------------------------------------------------------------
**  Input:   k = link index                                                      
**  Output:  none                                                      
**  Purpose: computes link flow resistance      
**--------------------------------------------------------------------
*/
{
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  
   double e,d,L;
  
  Slink *link = &net->Link[k];
   link->R = CSMALL;
  
   
  switch (link->Type) {
      /* Link is a pipe. Compute resistance based on headloss formula. */
      /* Friction factor for D-W formula gets included during solution */
      /* process in pipecoeff() function.                              */
    case EN_CVPIPE:
    case EN_PIPE:
      e = link->Kc;                 /* Roughness coeff. */
      d = link->Diam;               /* Diameter */
      L = link->Len;                /* Length */
      switch(hyd->Formflag)
    {
      case HW:
        link->R = 4.727*L/pow(e,hyd->Hexp)/pow(d,4.871);
        break;
      case DW:
        link->R = L/2.0/32.2/d/SQR(PI*SQR(d)/4.0);
        break;
      case CM:
        link->R = SQR(4.0*e/(1.49*PI*d*d)) * pow((d/4.0),-1.333)*L;
    }
      break;
      
      /* Link is a pump. Use negligible resistance. */
    case EN_PUMP:
      link->R = CBIG;  //CSMALL;
      break;
    default:
      break;
  }
}


void  demands(EN_Project *pr)
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


int  controls(EN_Project *pr)
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
  
  EN_Network *net = &pr->network;
  time_options_t *top = &pr->time_options;
  hydraulics_t *hyd = &pr->hydraulics;

   /* Examine each control statement */
   setsum = 0;
   for (i=1; i <= net->Ncontrols; i++)
   {
     Scontrol *control = &net->Control[i];
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


long  timestep(EN_Project *pr)
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


int  tanktimestep(EN_Project *pr, long *tstep)
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


void  controltimestep(EN_Project *pr, long *tstep)
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


void  ruletimestep(EN_Project *pr, long *tstep)
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
   

void  addenergy(EN_Project *pr, long hstep)
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
    double c0,c,             /* Energy cost (cost/kwh) */
          f0,               /* Energy cost factor */
          dt,               /* Time interval (hr) */
          e,                /* Pump efficiency (fraction) */
          q,                /* Pump flow (cfs) */
          p,                /* Pump energy (kw) */
          psum = 0.0;       /* Total energy (kw) */
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;

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
     Spump *pump = &net->Pump[j];
      /* Skip closed pumps */
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
      pump->Energy[0] += dt;            /* Time on-line */
      pump->Energy[1] += e*dt;          /* Effic.-hrs   */
      pump->Energy[2] += p/q*dt;        /* kw/cfs-hrs   */
      pump->Energy[3] += p*dt;          /* kw-hrs       */
      pump->Energy[4] = MAX(pump->Energy[4],p);
      pump->Energy[5] += c*p*dt;        /* cost-hrs.    */
   }

   /* Update maximum kw value */
   hyd->Emax = MAX(hyd->Emax,psum);
}                       /* End of pumpenergy */


void  getenergy(EN_Project *pr, int k, double *kw, double *eff)
/*
**----------------------------------------------------------------
**  Input:   k    = link index                         
**  Output:  *kw  = kwatt energy used
**           *eff = efficiency (pumps only)
**  Purpose: computes flow energy associated with link k                                           
**----------------------------------------------------------------
*/
{
   int   i,j;
   double dh, q, e;
   double q4eff; //q4eff=flow at nominal speed to compute efficiency
   Scurve *curve;
   
  EN_Network *net = &pr->network;
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
      if ( (i = net->Pump[j].Ecurve) > 0)
      { 
         q4eff = q / hyd->LinkSetting[k];
         curve = &net->Curve[i];
         e = interp(curve->Npts,curve->X, curve->Y, q4eff * pr->Ucf[FLOW]);
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


void  tanklevels(EN_Project *pr, long tstep)
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


double  tankvolume(EN_Project *pr, int i, double h)
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
  /* Use level*area if no volume curve */
  j = tank->Vcurve;
  if (j == 0) {
    return(tank->Vmin + (h - tank->Hmin) * tank->A);
  }
  
  /* If curve exists, interpolate on h to find volume v */
  /* remembering that volume curve is in original units.*/
  else {
    Scurve *curve = &net->Curve[j];
    return(interp(curve->Npts, curve->X, curve->Y, (h - net->Node[tank->Node].El) * pr->Ucf[HEAD]) / pr->Ucf[VOLUME]);
  }
  
}                       /* End of tankvolume */


double  tankgrade(EN_Project *pr, int i, double v)
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


int  netsolve(EN_Project *pr, int *iter, double *relerr)
/*
**-------------------------------------------------------------------
**  Input:   none                                                
**  Output:  *iter   = # of iterations to reach solution         
**           *relerr = convergence error in solution             
**           returns error code                                  
**  Purpose: solves network nodal equations for heads and flows    
**           using Todini's Gradient algorithm                   
**
*** Updated 9/7/00 *** 
*** Updated 2.00.11 ***
*** Updated 2.00.12 ***                                                            
**  Notes:   Status checks on CVs, pumps and pipes to tanks are made
**           every hyd->CheckFreq iteration, up until MaxCheck iterations
**           are reached. Status checks on control valves are made
**           every iteration if DampLimit = 0 or only when the
**           convergence error is at or below DampLimit. If DampLimit
**           is > 0 then future computed flow changes are only 60% of
**           their full value. A complete status check on all links
**           is made when convergence is achieved. If convergence is
**           not achieved in hyd->MaxIter trials and hyd->ExtraIter > 0 then
**           another hyd->ExtraIter trials are made with no status changes
**           made to any links and a warning message is generated.
**                                                             
**   This procedure calls linsolve() which appears in SMATRIX.C. 
**-------------------------------------------------------------------
*/
{
  int    i;                     /* Node index */
  int    errcode = 0;           /* Node causing solution error */
  int    nextcheck;             /* Next status check trial */
  int    maxtrials;             /* Max. trials for convergence */
  double newerr;                /* New convergence error */
  int    valveChange;           /* Valve status change flag */
  int    statChange;
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *sol = &hyd->solver;
  report_options_t *rep = &pr->report;
  
  /* Initialize status checking & relaxation factor */
  nextcheck = hyd->CheckFreq;
  hyd->RelaxFactor = 1.0;
  
  /* Repeat iterations until convergence or trial limit is exceeded. */
  /* (hyd->ExtraIter used to increase trials in case of status cycling.)  */
  if (pr->report.Statflag == FULL) {
    writerelerr(pr,0,0);
  }
  maxtrials = hyd->MaxIter;
  if (hyd->ExtraIter > 0) {
    maxtrials += hyd->ExtraIter;
  }
  *iter = 1;
  while (*iter <= maxtrials) {
    /*
     ** Compute coefficient matrices A & F and solve A*H = F
     ** where H = heads, A = Jacobian coeffs. derived from
     ** head loss gradients, & F = flow correction terms.
     ** Solution for H is returned in F from call to linsolve().
     */
    newcoeffs(pr);
    errcode = linsolve(&hyd->solver,net->Njuncs);
    
    /* Take action depending on error code */
    if (errcode < 0) {
      break;    /* Memory allocation problem */
    }
    if (errcode > 0) { /* Ill-conditioning problem */
      /* If control valve causing problem, fix its status & continue, */
      /* otherwise end the iterations with no solution.               */
      if (badvalve(pr,sol->Order[errcode])) {
        continue;
      }
      else break;
    }
    
    /* Update current solution. */
    /* (Row[i] = row of solution matrix corresponding to node i). */
    for (i=1; i<=net->Njuncs; i++) {
      hyd->NodeHead[i] = sol->F[sol->Row[i]];   /* Update heads */
    }
    newerr = newflows(pr);                          /* Update flows */
    *relerr = newerr;
    
    /* Write convergence error to status report if called for */
    if (rep->Statflag == FULL) {
      writerelerr(pr, *iter,*relerr);
    }
    
    /* Apply solution damping & check for change in valve status */
    hyd->RelaxFactor = 1.0;
    valveChange = FALSE;
    if ( hyd->DampLimit > 0.0 ) {
      if( *relerr <= hyd->DampLimit ) {
        hyd->RelaxFactor = 0.6;
        valveChange = valvestatus(pr);
      }
    }
    else {
      valveChange = valvestatus(pr);
    }
    
    /* Check for convergence */
    if (*relerr <= hyd->Hacc) {
      /* We have convergence. Quit if we are into extra iterations. */
      if (*iter > hyd->MaxIter) {
        break;
      }
      
      /* Quit if no status changes occur. */
      statChange = FALSE;
      if (valveChange) {
        statChange = TRUE;
      }
      if (linkstatus(pr)) {
        statChange = TRUE;
      }
      if (pswitch(pr)) {
        statChange = TRUE;
      }
      if (!statChange) {
        break;
      }
      
      /* We have a status change so continue the iterations */
      nextcheck = *iter + hyd->CheckFreq;
    }
    
    /* No convergence yet. See if its time for a periodic status */
    /* check  on pumps, CV's, and pipes connected to tanks.      */
    else if (*iter <= hyd->MaxCheck && *iter == nextcheck)
    {
      linkstatus(pr);
      nextcheck += hyd->CheckFreq;
    }
    (*iter)++;
  }
  
  /* Iterations ended. Report any errors. */
  if (errcode < 0) errcode = 101;      /* Memory allocation error */
  else if (errcode > 0)
  {
    writehyderr(pr, sol->Order[errcode]);      /* Ill-conditioned eqns. error */
    errcode = 110;
  }
  
  /* Add any emitter flows to junction demands */
  for (i=1; i<=net->Njuncs; i++) { 
    hyd->NodeDemand[i] += hyd->EmitterFlows[i];
  }
  return(errcode);
}                        /* End of netsolve */


int  badvalve(EN_Project *pr, int n)
/*
 **-----------------------------------------------------------------
**  Input:   n = node index
**  Output:  returns 1 if node n belongs to an active control valve,
**           0 otherwise  
**  Purpose: determines if a node belongs to an active control valve
**           whose setting causes an inconsistent set of eqns. If so,
**           the valve status is fixed open and a warning condition
**           is generated.
**-----------------------------------------------------------------
*/
{
   int i,k,n1,n2;
   Slink *link;
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  time_options_t *time = &pr->time_options;
  
   for (i=1; i <= net->Nvalves; i++) {
      k = net->Valve[i].Link;
      link = &net->Link[k];
      n1 = link->N1;
      n2 = link->N2;
      if (n == n1 || n == n2) {
        EN_LinkType t = link->Type;
         if (t == EN_PRV || t == EN_PSV || t == EN_FCV) {
            if (hyd->LinkStatus[k] == ACTIVE) {
               if (rep->Statflag == FULL) {
                  sprintf(pr->Msg,FMT61,clocktime(rep->Atime,time->Htime),link->ID);
                  writeline(pr, pr->Msg);
               }
              if (link->Type == EN_FCV) {
                hyd->LinkStatus[k] = XFCV;
              }
              else {
                hyd->LinkStatus[k] = XPRESSURE;
              }
               return(1);
            }
         }
         return(0);
      }
   }
   return(0);
}
   

int  valvestatus(EN_Project *pr)
/*
**-----------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns 1 if any pressure or flow control valve                   
**           changes status, 0 otherwise                                        
**  Purpose: updates status for PRVs & PSVs whose status                       
**           is not fixed to OPEN/CLOSED
**-----------------------------------------------------------------
*/
{
   int   change = FALSE,            /* Status change flag      */
         i,k,                       /* Valve & link indexes    */
         n1,n2;                     /* Start & end nodes       */
   StatType  status;                         /* Valve status settings   */
   double hset;                     /* Valve head setting      */
   Slink *link;
   
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  
   for (i=1; i <= net->Nvalves; i++)                   /* Examine each valve   */
   {
      k = net->Valve[i].Link;                        /* Link index of valve  */
      link = &net->Link[k];
     if (hyd->LinkSetting[k] == MISSING) {
       continue;            /* Valve status fixed   */
     }
      n1 = link->N1;                          /* Start & end nodes    */
      n2 = link->N2;
      status  = hyd->LinkStatus[k];                                /* Save current status  */

//      if (s != CLOSED                           /* No change if flow is */  
//      && ABS(hyd->LinkFlows[k]) < hyd->Qtol) continue;            /* negligible.          */  

      switch (link->Type)                     /* Evaluate new status: */
      {
         case EN_PRV:
          hset = net->Node[n2].El + hyd->LinkSetting[k];
          hyd->LinkStatus[k] = prvstatus(pr,k,status,hset,hyd->NodeHead[n1],hyd->NodeHead[n2]);
          break;
         case EN_PSV:
          hset = net->Node[n1].El + hyd->LinkSetting[k];
          hyd->LinkStatus[k] = psvstatus(pr,k,status,hset,hyd->NodeHead[n1],hyd->NodeHead[n2]);
          break;

////  FCV status checks moved back into the linkstatus() function ////           
//         case FCV:  S[k] = fcvstatus(k,s,hyd->NodeHead[n1],hyd->NodeHead[n2]);                         
//                    break;                                                     

         default:
          continue;
      }

/*** Updated 9/7/00 ***/
      /* Do not reset flow in valve if its status changes. */
      /* This strategy improves convergence. */

      /* Check for status change */
      if (status != hyd->LinkStatus[k])
      {
        if (rep->Statflag == FULL) {
          writestatchange(pr, k,status,hyd->LinkStatus[k]);
        }
         change = TRUE;
      }
   }
   return(change);
}                       /* End of valvestatus() */


/*** Updated 9/7/00 ***/
int  linkstatus(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns 1 if any link changes status, 0 otherwise   
**  Purpose: determines new status for pumps, CVs, FCVs & pipes                
**           to tanks.                                              
**--------------------------------------------------------------
*/
{
   int   change = FALSE,             /* Status change flag      */
         k,                          /* Link index              */
         n1,                         /* Start node index        */
         n2;                         /* End node index          */
   double dh;                        /* Head difference         */
   StatType  status;                     /* Current status          */
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;
  
   /* Examine each Slink */
   for (k=1; k <= net->Nlinks; k++)
   {
     Slink *link = &net->Link[k];
      n1 = link->N1;
      n2 = link->N2;
      dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];

      /* Re-open temporarily closed links (status = XHEAD or TEMPCLOSED) */
      status = hyd->LinkStatus[k];
     if (status == XHEAD || status == TEMPCLOSED) {
       hyd->LinkStatus[k] = OPEN;
     }

      /* Check for status changes in CVs and pumps */
     if (link->Type == EN_CVPIPE) {
        hyd->LinkStatus[k] = cvstatus(pr,hyd->LinkStatus[k],dh,hyd->LinkFlows[k]);
     }
     if (link->Type == EN_PUMP && hyd->LinkStatus[k] >= OPEN && hyd->LinkSetting[k] > 0.0) {                  
         hyd->LinkStatus[k] = pumpstatus(pr,k,-dh);
     }

      /* Check for status changes in non-fixed FCVs */
     if (link->Type == EN_FCV && hyd->LinkSetting[k] != MISSING) {                             //
         hyd->LinkStatus[k] = fcvstatus(pr,k,status,hyd->NodeHead[n1],hyd->NodeHead[n2]);                               //
     }

      /* Check for flow into (out of) full (empty) tanks */
     if (n1 > net->Njuncs || n2 > net->Njuncs) {
        tankstatus(pr,k,n1,n2);
     }

      /* Note change in link status; do not revise link flow */                
      if (status != hyd->LinkStatus[k]) {
         change = TRUE;
        if (rep->Statflag == FULL) {
          writestatchange(pr,k,status,hyd->LinkStatus[k]);
        }

         //if (S[k] <= CLOSED) hyd->LinkFlows[k] = QZERO;                                   
         //else setlinkflow(k, dh);                                            
      }
   }
   return(change);
}                        /* End of linkstatus */


StatType  cvstatus(EN_Project *pr, StatType s, double dh, double q)
/*
**--------------------------------------------------
**  Input:   s  = current status
**           dh = headloss
**           q  = flow
**  Output:  returns new link status                 
**  Purpose: updates status of a check valve.        
**--------------------------------------------------
*/
{
  hydraulics_t *hyd = &pr->hydraulics;
  
   /* Prevent reverse flow through CVs */
   if (ABS(dh) > hyd->Htol)
   {
     if (dh < -hyd->Htol) {
       return(CLOSED);
     }
     else if (q < -hyd->Qtol) {
       return(CLOSED);
     }
     else {
       return(OPEN);
     }
   }
   else
   {
     if (q < -hyd->Qtol) {
       return(CLOSED);
     }
     else {
       return(s);
     }
   }
}


StatType  pumpstatus(EN_Project *pr, int k, double dh)
/*
**--------------------------------------------------
**  Input:   k  = link index                         
**           dh = head gain
**  Output:  returns new pump status                 
**  Purpose: updates status of an open pump.               
**--------------------------------------------------
*/
{
  int   p;
  double hmax;
  hydraulics_t *hyd = &pr->hydraulics;
  EN_Network *net = &pr->network;
  
  /* Prevent reverse flow through pump */
  p = findpump(net,k);
  if (net->Pump[p].Ptype == CONST_HP) {
     hmax = BIG;
  }
  else {
    hmax = SQR(hyd->LinkSetting[k]) * net->Pump[p].Hmax;
  }
  if (dh > hmax + hyd->Htol) {
     return(XHEAD);
  }

/*** Flow higher than pump curve no longer results in a status change ***/     
   /* Check if pump cannot deliver flow */                                     
   //if (hyd->LinkFlows[k] > K[k]*Pump[p].Qmax + hyd->Qtol) return(XFLOW);                       
   return(OPEN);
}


StatType  prvstatus(EN_Project *pr, int k, StatType s, double hset, double h1, double h2)
/*
**-----------------------------------------------------------
**  Input:   k    = link index                                
**           s    = current status                            
**           hset = valve head setting                        
**           h1   = head at upstream node                     
**           h2   = head at downstream node                   
**  Output:  returns new valve status                         
**  Purpose: updates status of a pressure reducing valve.     
**-----------------------------------------------------------
*/
{
  StatType  status;     /* New valve status */
  double hml;        /* Minor headloss   */
  hydraulics_t *hyd = &pr->hydraulics;
  
  double htol = hyd->Htol;
  Slink *link = &pr->network.Link[k];
  
  status = s;
  if (hyd->LinkSetting[k] == MISSING) 
    return(status);       /* Status fixed by user */
  hml = link->Km*SQR(hyd->LinkFlows[k]); /* Head loss when open  */
  
  /*** Status rules below have changed. ***/
  
  switch (s)
  {
    case ACTIVE:
      if (hyd->LinkFlows[k] < -hyd->Qtol) {
        status = CLOSED;
      }
      else if (h1-hml < hset-htol) {
        status = OPEN;                           
      }
      else {
        status = ACTIVE;
      }
      break;
    case OPEN:
      if (hyd->LinkFlows[k] < -hyd->Qtol) {
        status = CLOSED;
      }
      else if (h2 >= hset+htol) {
        status = ACTIVE;                         
      }
      else {
        status = OPEN;
      }
      break;
    case CLOSED:
      if ( h1 >= hset+htol && h2 < hset-htol) {
        status = ACTIVE;                         
      }
      else if (h1 < hset-htol && h1 > h2+htol) {
        status = OPEN;                           
      }
      else {
        status = CLOSED;
      }
      break;
    case XPRESSURE:
      if (hyd->LinkFlows[k] < -hyd->Qtol) {
        status = CLOSED;
      }
      break;
    default:
      break;
  }
  return(status);
}


StatType  psvstatus(EN_Project *pr, int k, StatType s, double hset, double h1, double h2)
/*
**-----------------------------------------------------------
**  Input:   k    = link index                                
**           s    = current status                            
**           hset = valve head setting                        
**           h1   = head at upstream node                     
**           h2   = head at downstream node                   
**  Output:  returns new valve status                         
**  Purpose: updates status of a pressure sustaining valve.   
**-----------------------------------------------------------
*/
{
  StatType  status;       /* New valve status */
  double hml;          /* Minor headloss   */
  hydraulics_t *hyd = &pr->hydraulics;
  
  double htol = hyd->Htol;
  
  Slink *link = &pr->network.Link[k];
  
  status = s;
  if (hyd->LinkSetting[k] == MISSING) {
    return(status);       /* Status fixed by user */
  }
  hml = link->Km*SQR(hyd->LinkFlows[k]);                /* Head loss when open  */
  
  /*** Status rules below have changed. ***/                                     
  
  switch (s)
  {
    case ACTIVE:
      if (hyd->LinkFlows[k] < -hyd->Qtol) {
        status = CLOSED;
      }
      else if (h2+hml > hset+htol) {
        status = OPEN;                           
      }
      else {
        status = ACTIVE;
      }
      break;
    case OPEN:
      if (hyd->LinkFlows[k] < -hyd->Qtol) {
        status = CLOSED;
      }
      else if (h1 < hset-htol) {
        status = ACTIVE;                         
      }
      else {
        status = OPEN;
      }
      break;
    case CLOSED:
      if (h2 > hset+htol && h1 > h2+htol) {
        status = OPEN;                           
      }
      else if (h1 >= hset+htol && h1 > h2+htol) {
        status = ACTIVE;                         
      }
      else {
        status = CLOSED;
      }
      break;
    case XPRESSURE:
      if (hyd->LinkFlows[k] < -hyd->Qtol) {
        status = CLOSED;
      }
      break;
    default:
      break;
  }
  return(status);
}


StatType  fcvstatus(EN_Project *pr, int k, StatType s, double h1, double h2)
/*
**-----------------------------------------------------------
**  Input:   k    = link index                                
**           s    = current status                            
**           h1   = head at upstream node                     
**           h2   = head at downstream node                   
**  Output:  returns new valve status                         
**  Purpose: updates status of a flow control valve.          
**                                                          
**    Valve status changes to XFCV if flow reversal.       
**    If current status is XFCV and current flow is        
**    above setting, then valve becomes active.             
**    If current status is XFCV, and current flow          
**    positive but still below valve setting, then          
**    status remains same.                                  
**-----------------------------------------------------------
*/
{
   StatType  status;        /* New valve status */
   hydraulics_t *hyd = &pr->hydraulics;
  
   status = s;
   if (h1 - h2 < -hyd->Htol) {
     status = XFCV;
   }
   else if ( hyd->LinkFlows[k] < -hyd->Qtol ) {
     status = XFCV;                          
   }
   else if (s == XFCV && hyd->LinkFlows[k] >= hyd->LinkSetting[k]) {
     status = ACTIVE;
   }
   return(status);
}


/*** Updated 9/7/00 ***/
/*** Updated 11/19/01 ***/
void  tankstatus(EN_Project *pr, int k, int n1, int n2)
/*
**----------------------------------------------------------------
**  Input:   k  = link index                                     
**           n1 = start node of link
**           n2 = end node of link                              
**  Output:  none                                                
**  Purpose: closes link flowing into full or out of empty tank  
**----------------------------------------------------------------
*/
{
   int   i,n;
   double h,q;
   Stank *tank;
   
  hydraulics_t *hyd = &pr->hydraulics;
  EN_Network *net = &pr->network;
  Slink *link = &net->Link[k];
  
   /* Make node n1 be the tank */
   q = hyd->LinkFlows[k];
   i = n1 - net->Njuncs;
   if (i <= 0) {
      i = n2 - net->Njuncs;
     if (i <= 0) {
       return;
     }
      n = n1;
      n1 = n2;
      n2 = n;
      q = -q;
   }
   h = hyd->NodeHead[n1] - hyd->NodeHead[n2];
   tank = &net->Tank[i];
   /* Skip reservoirs & closed links */
  if (tank->A == 0.0 || hyd->LinkStatus[k] <= CLOSED) {
    return;
  }

   /* If tank full, then prevent flow into it */
   if (hyd->NodeHead[n1] >= tank->Hmax - hyd->Htol)
   {

      /* Case 1: Link is a pump discharging into tank */
      if ( link->Type == EN_PUMP ) {
         if (link->N2 == n1)
           hyd->LinkStatus[k] = TEMPCLOSED;
      }

      /* Case 2: Downstream head > tank head */
      /* (i.e., an open outflow check valve would close) */
      else if (cvstatus(pr, OPEN, h, q) == CLOSED) {
        hyd->LinkStatus[k] = TEMPCLOSED;
      }
   }

   /* If tank empty, then prevent flow out of it */
   if (hyd->NodeHead[n1] <= tank->Hmin + hyd->Htol) {

      /* Case 1: Link is a pump discharging from tank */
      if ( link->Type == EN_PUMP) {
        if (link->N1 == n1) {
          hyd->LinkStatus[k] = TEMPCLOSED;
        }
      }

      /* Case 2: Tank head > downstream head */
      /* (i.e., a closed outflow check valve would open) */
      else if (cvstatus(pr, CLOSED, h, q) == OPEN) {
        hyd->LinkStatus[k] = TEMPCLOSED;
      }
   }
}                        /* End of tankstatus */


int  pswitch(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns 1 if status of any link changes, 0 if not   
**  Purpose: adjusts settings of links controlled by junction    
**           pressures after a hydraulic solution is found       
**--------------------------------------------------------------
*/
{
   int   i,                 /* Control statement index */
         k,                 /* Link being controlled */
         n,                 /* Node controlling Slink */
         reset,             /* Flag on control conditions */
         change,            /* Flag for status or setting change */
         anychange = 0;     /* Flag for 1 or more changes */
   char  s;                 /* Current link status */
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  report_options_t *rep = &pr->report;

   /* Check each control statement */
   for (i=1; i <= net->Ncontrols; i++)
   {
     reset = 0;
     if ( (k = net->Control[i].Link) <= 0) {
       continue;
     }

      /* Determine if control based on a junction, not a tank */
      if ( (n = net->Control[i].Node) > 0 && n <= net->Njuncs) {
         /* Determine if control conditions are satisfied */
         if (net->Control[i].Type == LOWLEVEL
             && hyd->NodeHead[n] <= net->Control[i].Grade + hyd->Htol ) {
             reset = 1;
         }
         if (net->Control[i].Type == HILEVEL
             && hyd->NodeHead[n] >= net->Control[i].Grade - hyd->Htol ) {
             reset = 1;
         }
      }

      /* Determine if control forces a status or setting change */
      if (reset == 1)
      {
        Slink *link = &net->Link[k];
         change = 0;
         s = hyd->LinkStatus[k];
         if (link->Type == EN_PIPE) {
           if (s != net->Control[i].Status) {
             change = 1;
           }
         }
         if (link->Type == EN_PUMP) {
           if (hyd->LinkSetting[k] != net->Control[i].Setting) {
             change = 1;
           }
         }
         if (link->Type >= EN_PRV) {
           if (hyd->LinkSetting[k] != net->Control[i].Setting) {
             change = 1;
           }
            else if (hyd->LinkSetting[k] == MISSING && s != net->Control[i].Status) {
              change = 1;
            }
         }

         /* If a change occurs, update status & setting */
         if (change) {
            hyd->LinkStatus[k] = net->Control[i].Status;
           if (link->Type > EN_PIPE) {
             hyd->LinkSetting[k] = net->Control[i].Setting;
           }
           if (rep->Statflag == FULL) {
              writestatchange(pr, k,s,hyd->LinkStatus[k]);
           }

            /* Re-set flow if status has changed */
//            if (S[k] != s) initlinkflow(k, S[k], K[k]);
            anychange = 1;
         }
      }
   }
   return(anychange);
}                        /* End of pswitch */


double newflows(EN_Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns solution convergence error                  
**  Purpose: updates link flows after new nodal heads computed   
**----------------------------------------------------------------
*/
{
   double  dh,                    /* Link head loss       */
           dq;                    /* Link flow change     */
   double  dqsum,                 /* Network flow change  */
           qsum;                  /* Network total flow   */
   int   k, n, n1, n2;
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;

   /* Initialize net inflows (i.e., demands) at tanks */
  for (n = net->Njuncs + 1; n <= net->Nnodes; n++) {
    hyd->NodeDemand[n] = 0.0;
  }

   /* Initialize sum of flows & corrections */
   qsum  = 0.0;
   dqsum = 0.0;

   /* Update flows in all links */
   for (k=1; k<=net->Nlinks; k++)
   {
     Slink *link = &net->Link[k];
      /*
      ** Apply flow update formula:                   
      **   dq = Y - P*(new head loss)                 
      **    P = 1/(dh/dq)                             
      **    Y = P*(head loss based on current flow)   
      ** where P & Y were computed in newcoeffs().   
      */

      n1 = link->N1;
      n2 = link->N2;
      dh = hyd->NodeHead[n1] - hyd->NodeHead[n2];
      dq = s->Y[k] - s->P[k] * dh;

      /* Adjust flow change by the relaxation factor */                        
      dq *= hyd->RelaxFactor;                                                       

      /* Prevent flow in constant HP pumps from going negative */
      if (link->Type == EN_PUMP) {
        n = findpump(net,k);
        if (net->Pump[n].Ptype == CONST_HP && dq > hyd->LinkFlows[k]) {
          dq = hyd->LinkFlows[k]/2.0;
        }
      }
      hyd->LinkFlows[k] -= dq;

      /* Update sum of absolute flows & flow corrections */
      qsum += ABS(hyd->LinkFlows[k]);
      dqsum += ABS(dq);

      /* Update net flows to tanks */
      if ( hyd->LinkStatus[k] > CLOSED )                                                     
      {
        if (n1 > net->Njuncs) {
          hyd->NodeDemand[n1] -= hyd->LinkFlows[k];
        }
        if (n2 > net->Njuncs) {
          hyd->NodeDemand[n2] += hyd->LinkFlows[k];
        }
      }

   }

   /* Update emitter flows */
   for (k=1; k<=net->Njuncs; k++)
   {
     if (net->Node[k].Ke == 0.0) {
       continue;
     }
      dq = emitflowchange(pr,k);
      hyd->EmitterFlows[k] -= dq;
      qsum += ABS(hyd->EmitterFlows[k]);
      dqsum += ABS(dq);
   }

   /* Return ratio of total flow corrections to total flow */
  if (qsum > hyd->Hacc) {
    return(dqsum/qsum);
  }
  else {
    return(dqsum);
  }
}                        /* End of newflows */


void   newcoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**  Input:   none                                                
**  Output:  none                                                
**  Purpose: computes coefficients of linearized network eqns.   
**--------------------------------------------------------------
*/
{
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  EN_Network *n = &pr->network;
  
   memset(s->Aii,0,(n->Nnodes+1)*sizeof(double));   /* Reset coeffs. to 0 */
   memset(s->Aij,0,(hyd->Ncoeffs+1)*sizeof(double));
   memset(s->F,0,(n->Nnodes+1)*sizeof(double));
   memset(hyd->X_tmp,0,(n->Nnodes+1)*sizeof(double));
   memset(s->P,0,(n->Nlinks+1)*sizeof(double));
   memset(s->Y,0,(n->Nlinks+1)*sizeof(double));
   linkcoeffs(pr);                            /* Compute link coeffs.  */
   emittercoeffs(pr);                         /* Compute emitter coeffs.*/
   nodecoeffs(pr);                            /* Compute node coeffs.  */
   valvecoeffs(pr);                           /* Compute valve coeffs. */
}                        /* End of newcoeffs */


void  linkcoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none                                                
**   Output:  none                                                
**   Purpose: computes solution matrix coefficients for links     
**--------------------------------------------------------------
*/
{
   int   k,n1,n2;
  
  EN_Network *net = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  EN_Network *n = &pr->network;

   /* Examine each link of network */
   for (k=1; k<=net->Nlinks; k++)
   {
      Slink *link = &net->Link[k];
      n1 = link->N1;           /* Start node of Slink */
      n2 = link->N2;           /* End node of link   */

      /* Compute P[k] = 1 / (dh/dQ) and Y[k] = h * P[k]   */
      /* for each link k (where h = link head loss).      */
      /* FCVs, PRVs, and PSVs with non-fixed status       */
      /* are analyzed later.                              */

      switch (link->Type)
      {
         case EN_CVPIPE:
         case EN_PIPE:
          pipecoeff(pr, k);
          break;
         case EN_PUMP:
          pumpcoeff(pr, k);
          break;
         case EN_PBV:
          pbvcoeff(pr, k);
          break;
         case EN_TCV:
          tcvcoeff(pr, k);
          break;
         case EN_GPV:
          gpvcoeff(pr, k);
          break;
         case EN_FCV:
         case EN_PRV:
         case EN_PSV:   /* If valve status fixed then treat as pipe, otherwise ignore the valve for now. */
          if (hyd->LinkSetting[k] == MISSING) {
            valvecoeff(pr, k);  //pipecoeff(k);      
          }
          else {
            continue;
          }
          break;
         default:
          continue;
      }                                         

      /* Update net nodal inflows (X), solution matrix (A) and RHS array (F) */
      /* (Use covention that flow out of node is (-), flow into node is (+)) */
      hyd->X_tmp[n1] -= hyd->LinkFlows[k];
      hyd->X_tmp[n2] += hyd->LinkFlows[k];
      s->Aij[s->Ndx[k]] -= s->P[k];              /* Off-diagonal coeff. */
      if (n1 <= n->Njuncs)                 /* Node n1 is junction */
      {
         s->Aii[s->Row[n1]] += s->P[k];          /* Diagonal coeff. */
         s->F[s->Row[n1]] += s->Y[k];            /* RHS coeff.      */
      }
      else {
        s->F[s->Row[n2]] += (s->P[k]*hyd->NodeHead[n1]);  /* Node n1 is a tank   */
      }
      if (n2 <= n->Njuncs) { /* Node n2 is junction */
         s->Aii[s->Row[n2]] += s->P[k];          /* Diagonal coeff. */
         s->F[s->Row[n2]] -= s->Y[k];            /* RHS coeff.      */
      }
      else {
        s->F[s->Row[n1]] += (s->P[k] * hyd->NodeHead[n2]); /* Node n2 is a tank   */
      }
   }
}                        /* End of linkcoeffs */


void  nodecoeffs(EN_Project *pr)
/*
**----------------------------------------------------------------
**  Input:   none                                                
**  Output:  none                                                
**  Purpose: completes calculation of nodal flow imbalance (X)   
**           & flow correction (F) arrays                        
**----------------------------------------------------------------
*/
{
   int   i;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  EN_Network *n = &pr->network;

   /* For junction nodes, subtract demand flow from net */
   /* flow imbalance & add imbalance to RHS array F.    */
   for (i=1; i <= n->Njuncs; i++)
   {
      hyd->X_tmp[i] -= hyd->NodeDemand[i];
      s->F[s->Row[i]] += hyd->X_tmp[i];
   }
}                        /* End of nodecoeffs */


void  valvecoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none                                                
**   Output:  none                                                
**   Purpose: computes matrix coeffs. for PRVs, PSVs & FCVs       
**            whose status is not fixed to OPEN/CLOSED            
**--------------------------------------------------------------
*/
{
   int i,k,n1,n2;

  hydraulics_t *hyd = &pr->hydraulics;
  EN_Network *n = &pr->network;
  Slink *link;
  Svalve *valve;
  
   for (i=1; i<=n->Nvalves; i++)                   /* Examine each valve   */
   {
     valve = &n->Valve[i];
     k = valve->Link;                        /* Link index of valve  */
     link = &n->Link[k];
     if (hyd->LinkSetting[k] == MISSING) {
       continue;            /* Valve status fixed   */
     }
      n1 = link->N1;                          /* Start & end nodes    */
      n2 = link->N2; 
      switch (link->Type)                     /* Call valve-specific  */
      {                                         /*   function           */
         case EN_PRV:
          prvcoeff(pr, k,n1,n2);
          break;
         case EN_PSV:
          psvcoeff(pr, k,n1,n2);
          break;
         case EN_FCV:
          fcvcoeff(pr, k,n1,n2);
          break;
         default:   continue;
      }
   }
}                        /* End of valvecoeffs */


void  emittercoeffs(EN_Project *pr)
/*
**--------------------------------------------------------------
**   Input:   none                                                
**   Output:  none                                                
**   Purpose: computes matrix coeffs. for emitters
**
**   Note: Emitters consist of a fictitious pipe connected to
**         a fictitious reservoir whose elevation equals that
**         of the junction. The headloss through this pipe is
**         Ke*(Flow)^hyd->Qexp, where Ke = emitter headloss coeff.
**--------------------------------------------------------------
*/
{
   int   i;
   double  ke;
   double  p;
   double  q;
   double  y;
   double  z;
  
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  EN_Network *n = &pr->network;
  
   for (i=1; i <= n->Njuncs; i++)
   {
     Snode *node = &n->Node[i];
     if (node->Ke == 0.0) {
       continue;
     }
      ke = MAX(CSMALL, node->Ke);
      q = hyd->EmitterFlows[i];
      z = ke * pow(ABS(q),hyd->Qexp);
      p = hyd->Qexp * z / ABS(q);
     if (p < hyd->RQtol) {
       p = 1.0 / hyd->RQtol;
     }
     else {
       p = 1.0 / p;
     }
      y = SGN(q)*z*p;
      s->Aii[s->Row[i]] += p;
      s->F[s->Row[i]] += y + p * node->El;
      hyd->X_tmp[i] -= q;
   }
}


double  emitflowchange(EN_Project *pr, int i)
/*
**--------------------------------------------------------------
**   Input:   i = node index
**   Output:  returns change in flow at an emitter node                                                
**   Purpose: computes flow change at an emitter node
**--------------------------------------------------------------
*/
{
  double ke, p;
  hydraulics_t *hyd = &pr->hydraulics;
  EN_Network *n = &pr->network;
  Snode *node = &n->Node[i];
  
  ke = MAX(CSMALL, node->Ke);
  p = hyd->Qexp * ke * pow(ABS(hyd->EmitterFlows[i]),(hyd->Qexp - 1.0));
  if (p < hyd->RQtol) {
      p = 1/hyd->RQtol;
  }
  else {
      p = 1.0/p;
  }
  return(hyd->EmitterFlows[i] / hyd->Qexp - p * (hyd->NodeHead[i] - node->El));
}


void  pipecoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index                                      
**   Output:  none                                                
**  Purpose:  computes P & Y coefficients for pipe k              
**                                                              
**    P = inverse head loss gradient = 1/(dh/dQ)                
**    Y = flow correction term = h*P                            
**--------------------------------------------------------------
*/
{
   double  hpipe,     /* Normal head loss          */
         hml,       /* Minor head loss           */
         ml,        /* Minor loss coeff.         */
         p,         /* q*(dh/dq)                 */
         q,         /* Abs. value of flow        */
         r,         /* Resistance coeff.         */
         r1,        /* Total resistance factor   */
         f,         /* D-W friction factor       */
         dfdq;      /* Derivative of fric. fact. */
  
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  Slink *link = &pr->network.Link[k];

   /* For closed pipe use headloss formula: h = CBIG*q */
   if (hyd->LinkStatus[k] <= CLOSED)
   {
      s->P[k] = 1.0/CBIG;
      s->Y[k] = hyd->LinkFlows[k];
      return;
   }

   /* Evaluate headloss coefficients */
   q = ABS(hyd->LinkFlows[k]);                         /* Absolute flow       */
   ml = link->Km;                       /* Minor loss coeff.   */
   r = link->R;                         /* Resistance coeff.   */
   f = 1.0;                               /* D-W friction factor */
   if (hyd->Formflag == DW) f = DWcoeff(pr,k,&dfdq);   
   r1 = f*r+ml;
 
   /* Use large P coefficient for small flow resistance product */
   if (r1*q < hyd->RQtol)
   {
      s->P[k] = 1.0/hyd->RQtol;
      s->Y[k] = hyd->LinkFlows[k]/hyd->Hexp;
      return;
   }

   /* Compute P and Y coefficients */
   if (hyd->Formflag == DW)                  /* D-W eqn. */
   {
      hpipe = r1*SQR(q);                /* Total head loss */
      p = 2.0*r1*q;                     /* |dh/dQ| */
     /* + dfdq*r*q*q;*/                 /* Ignore df/dQ term */
      p = 1.0/p;
      s->P[k] = p;
      s->Y[k] = SGN(hyd->LinkFlows[k])*hpipe*p;
   }
   else                                 /* H-W or C-M eqn.   */
   {
      hpipe = r*pow(q,hyd->Hexp);            /* Friction head loss  */
      p = hyd->Hexp*hpipe;                   /* Q*dh(friction)/dQ   */
      if (ml > 0.0)
      {
         hml = ml*q*q;                  /* Minor head loss   */
         p += 2.0*hml;                  /* Q*dh(Total)/dQ    */
      }
      else  hml = 0.0;
      p = hyd->LinkFlows[k]/p;                       /* 1 / (dh/dQ) */
      s->P[k] = ABS(p);
      s->Y[k] = p*(hpipe + hml);
   }
}                        /* End of pipecoeff */


double DWcoeff(EN_Project *pr, int k, double *dfdq)
/*
**--------------------------------------------------------------
**   Input:   k = link index                                      
**   Output:  returns Darcy-Weisbach friction factor              
**   Purpose: computes Darcy-Weisbach friction factor             
**                                                              
**    Uses interpolating polynomials developed by               
**    E. Dunlop for transition flow from 2000 < Re < 4000.      
**
**   df/dq term is ignored as it slows convergence rate.
**--------------------------------------------------------------
*/
{
   double q,             /* Abs. value of flow */
          f;             /* Friction factor    */
   double x1,x2,x3,x4,
          y1,y2,y3,
          fa,fb,r;
   double s,w;
  
  hydraulics_t *hyd = &pr->hydraulics;
  Slink *link = &pr->network.Link[k];

   *dfdq = 0.0;
   if (link->Type > EN_PIPE)
     return(1.0); /* Only apply to pipes */
   q = ABS(hyd->LinkFlows[k]);
   s = hyd->Viscos * link->Diam;
   w = q/s;                       /* w = Re(Pi/4) */
   if (w >= A1)                   /* Re >= 4000; Colebrook Formula */
   {
      y1 = A8/pow(w,0.9);
      y2 = link->Kc/(3.7*link->Diam) + y1;
      y3 = A9*log(y2);
      f = 1.0/SQR(y3);
      /*  *dfdq = (2.0+AA*y1/(y2*y3))*f; */   /* df/dq */
   }
   else if (w > A2)              /* Re > 2000; Interpolation formula */
   {
      y2 = link->Kc/(3.7*link->Diam) + AB;
      y3 = A9*log(y2);
      fa = 1.0/SQR(y3);
      fb = (2.0+AC/(y2*y3))*fa;
      r = w/A2;
      x1 = 7.0*fa - fb;
      x2 = 0.128 - 17.0*fa + 2.5*fb;
      x3 = -0.128 + 13.0*fa - (fb+fb);
      x4 = r*(0.032 - 3.0*fa + 0.5*fb);
      f = x1 + r*(x2 + r*(x3+x4));
      /*  *dfdq = (x1 + x1 + r*(3.0*x2 + r*(4.0*x3 + 5.0*x4)));  */
   }
   else if (w > A4)              /* Laminar flow: Hagen-Poiseuille Formula */
   {
      f = A3*s/q;
      /*  *dfdq = A3*s; */
   }
   else
   {
      f = 8.0;
      *dfdq = 0.0;
   }
   return(f);
}                        /* End of DWcoeff */


/*** Updated 10/25/00 ***/
/*** Updated 12/29/00 ***/
void  pumpcoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index                                      
**   Output:  none                                                
**   Purpose: computes P & Y coeffs. for pump in link k           
**--------------------------------------------------------------
*/
{
   int   p;         /* Pump index             */
   double h0,       /* Shutoff head           */
         q,         /* Abs. value of flow     */
         r,         /* Flow resistance coeff. */
         n;         /* Flow exponent coeff.   */
  hydraulics_t *hyd;
  solver_t *s;
  double setting;
  Spump *pump;
  
  hyd = &pr->hydraulics;
  s = &hyd->solver;

   setting = hyd->LinkSetting[k];
  
   /* Use high resistance pipe if pump closed or cannot deliver head */
   if (hyd->LinkStatus[k] <= CLOSED || setting == 0.0) {
      s->P[k] = 1.0/CBIG;
      s->Y[k] = hyd->LinkFlows[k];
      return;
   }

   q = ABS(hyd->LinkFlows[k]);
   q = MAX(q,TINY);
   p = findpump(&pr->network,k);

   pump = &pr->network.Pump[p];
   /* Get pump curve coefficients for custom pump curve. */
   if (pump->Ptype == CUSTOM)
   {
      /* Find intercept (h0) & slope (r) of pump curve    */
      /* line segment which contains speed-adjusted flow. */
      curvecoeff(pr,pump->Hcurve, q/setting, &h0, &r);

      /* Determine head loss coefficients. */
      pump->H0 = -h0;
      pump->R  = -r;
      pump->N  = 1.0;
   }

   /* Adjust head loss coefficients for pump speed. */
   h0 = SQR(setting) * pump->H0;
   n  = pump->N;
   r  = pump->R * pow(setting,2.0-n);
   if (n != 1.0) {
     r = n * r * pow(q,n-1.0);
   }

   /* Compute inverse headloss gradient (P) and flow correction factor (Y) */
   s->P[k] = 1.0/MAX(r,hyd->RQtol);
   s->Y[k] = hyd->LinkFlows[k]/n + s->P[k]*h0;
}                        /* End of pumpcoeff */


/*** Updated 10/25/00 ***/
/*** Updated 12/29/00 ***/
void  curvecoeff(EN_Project *p, int i, double q, double *h0, double *r)
/*
**-------------------------------------------------------------------
**   Input:   i   = curve index                                        
**            q   = flow rate
**   Output:  *h0  = head at zero flow (y-intercept)                   
**            *r  = dHead/dFlow (slope)                                
**   Purpose: computes intercept and slope of head v. flow curve       
**            at current flow.                                         
**-------------------------------------------------------------------
*/
{
   int   k1, k2, npts;
   double *x, *y;
   Scurve *curve;
    
   /* Remember that curve is stored in untransformed units */
   q *= p->Ucf[FLOW];
   curve = &p->network.Curve[i];
   x = curve->X;           /* x = flow */
   y = curve->Y;           /* y = head */
   npts = curve->Npts;

   /* Find linear segment of curve that brackets flow q */
   k2 = 0;
   while (k2 < npts && x[k2] < q)
     k2++;
  
   if (k2 == 0)
     k2++;
  
   else if (k2 == npts)
     k2--;
  
   k1 = k2 - 1;

   /* Compute slope and intercept of this segment */
   *r  = (y[k2]-y[k1]) / (x[k2]-x[k1]);
   *h0 = y[k1] - (*r)*x[k1];

   /* Convert units */
   *h0 = (*h0) / p->Ucf[HEAD];
   *r  = (*r)*p->Ucf[FLOW]/p->Ucf[HEAD];
}                       /* End of curvecoeff */


void  gpvcoeff(EN_Project *p, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index
**   Output:  none                                                
**   Purpose: computes P & Y coeffs. for general purpose valve   
**--------------------------------------------------------------
*/
{
   double h0,        /* Headloss curve intercept */
          q,         /* Abs. value of flow       */
          r;         /* Flow resistance coeff.   */

  hydraulics_t *hyd = &p->hydraulics;
  solver_t *s = &hyd->solver;
  
/*** Updated 9/7/00 ***/
   /* Treat as a pipe if valve closed */
   if (hyd->LinkStatus[k] == CLOSED) {
     valvecoeff(p,k); //pipecoeff(k);                          
   }
   /* Otherwise utilize headloss curve   */
   /* whose index is stored in K */
   else {
      /* Find slope & intercept of headloss curve. */
      q = ABS(hyd->LinkFlows[k]);
      q = MAX(q,TINY);

/*** Updated 10/25/00 ***/
/*** Updated 12/29/00 ***/
      curvecoeff(p,(int)ROUND(hyd->LinkSetting[k]),q,&h0,&r);

      /* Compute inverse headloss gradient (P) */
      /* and flow correction factor (Y).       */
      s->P[k] = 1.0 / MAX(r,hyd->RQtol);
      s->Y[k] = s->P[k]*(h0 + r*q) * SGN(hyd->LinkFlows[k]);                                        
   }
}
 

void  pbvcoeff(EN_Project *p, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index                                      
**   Output:  none                                                
**   Purpose: computes P & Y coeffs. for pressure breaker valve   
**--------------------------------------------------------------
*/
{
  
   Slink *link = &p->network.Link[k];
   hydraulics_t *hyd = &p->hydraulics;
   solver_t *s = &hyd->solver;
  
   /* If valve fixed OPEN or CLOSED then treat as a pipe */
   if (hyd->LinkSetting[k] == MISSING || hyd->LinkSetting[k] == 0.0) {
     valvecoeff(p,k);  //pipecoeff(k);         
   }

   /* If valve is active */
   else {
      /* Treat as a pipe if minor loss > valve setting */
      if (link->Km * SQR(hyd->LinkFlows[k]) > hyd->LinkSetting[k]) {
        valvecoeff(p,k);  //pipecoeff(k);         
      }
      /* Otherwise force headloss across valve to be equal to setting */
      else {
         s->P[k] = CBIG;
         s->Y[k] = hyd->LinkSetting[k] * CBIG;
      }
   }
}                        /* End of pbvcoeff */


void  tcvcoeff(EN_Project *p, int k)
/*
**--------------------------------------------------------------
**   Input:   k = link index                                      
**   Output:  none                                                
**   Purpose: computes P & Y coeffs. for throttle control valve   
**--------------------------------------------------------------
*/
{
   double km;
   Slink *link = &p->network.Link[k];
   hydraulics_t *hyd = &p->hydraulics;
  
   /* Save original loss coeff. for open valve */
   km = link->Km;

   /* If valve not fixed OPEN or CLOSED, compute its loss coeff. */
   if (hyd->LinkSetting[k] != MISSING) {
     link->Km = 0.02517 * hyd->LinkSetting[k] / (SQR(link->Diam)*SQR(link->Diam));
   }

   /* Then apply usual pipe formulas */
   valvecoeff(p,k);  //pipecoeff(k);                                             

   /* Restore original loss coeff. */
   link->Km = km;
}                        /* End of tcvcoeff */


void  prvcoeff(EN_Project *p, int k, int n1, int n2)
/*
**--------------------------------------------------------------
**   Input:   k    = link index                                   
**            n1   = upstream node of valve                       
**            n2   = downstream node of valve                       
**   Output:  none                                                
**   Purpose: computes solution matrix coeffs. for pressure       
**            reducing valves                                     
**--------------------------------------------------------------
*/
{
   int   i,j;                       /* Rows of solution matrix */
   double hset;                      /* Valve head setting      */
   hydraulics_t *hyd = &p->hydraulics;
   solver_t *s = &hyd->solver;
  
   i = s->Row[n1];                    /* Matrix rows of nodes    */
   j = s->Row[n2];
   hset = p->network.Node[n2].El + hyd->LinkSetting[k];     /* Valve setting           */

   if (hyd->LinkStatus[k] == ACTIVE)
   {
      /*
         Set coeffs. to force head at downstream 
         node equal to valve setting & force flow (when updated in       
         newflows()) equal to flow imbalance at downstream node. 
      */
      s->P[k] = 0.0;
      s->Y[k] = hyd->LinkFlows[k] + hyd->X_tmp[n2];       /* Force flow balance   */
      s->F[j] += (hset * CBIG);       /* Force head = hset    */
      s->Aii[j] += CBIG;            /*   at downstream node */
      if (hyd->X_tmp[n2] < 0.0) {
        s->F[i] += hyd->X_tmp[n2];
      }
      return;
   }

   /* 
      For OPEN, CLOSED, or XPRESSURE valve
      compute matrix coeffs. using the valvecoeff() function.                  
   */
   valvecoeff(p,k);  /*pipecoeff(k);*/                                           
   s->Aij[s->Ndx[k]] -= s->P[k];
   s->Aii[i] += s->P[k];
   s->Aii[j] += s->P[k];
   s->F[i] += (s->Y[k] - hyd->LinkFlows[k]);
   s->F[j] -= (s->Y[k] - hyd->LinkFlows[k]);
}                        /* End of prvcoeff */


void  psvcoeff(EN_Project *p, int k, int n1, int n2)
/*
**--------------------------------------------------------------
**   Input:   k    = link index                                   
**            n1   = upstream node of valve                       
**            n2   = downstream node of valve                       
**   Output:  none                                                
**   Purpose: computes solution matrix coeffs. for pressure       
**            sustaining valve                                    
**--------------------------------------------------------------
*/
{
   int   i,j;                       /* Rows of solution matrix */
   double hset;                      /* Valve head setting      */
  
  hydraulics_t *hyd = &p->hydraulics;
  solver_t *s = &hyd->solver;
  
   i = s->Row[n1];                    /* Matrix rows of nodes    */
   j = s->Row[n2];
   hset = p->network.Node[n1].El + hyd->LinkSetting[k];     /* Valve setting           */

   if (hyd->LinkStatus[k] == ACTIVE)
   {
      /*
         Set coeffs. to force head at upstream 
         node equal to valve setting & force flow (when updated in       
         newflows()) equal to flow imbalance at upstream node. 
      */
      s->P[k] = 0.0;
      s->Y[k] = hyd->LinkFlows[k] - hyd->X_tmp[n1];              /* Force flow balance   */
      s->F[i] += (hset*CBIG);              /* Force head = hset    */
      s->Aii[i] += CBIG;                   /*   at upstream node   */
      if (hyd->X_tmp[n1] > 0.0) {
        s->F[j] += hyd->X_tmp[n1];
      }
      return;
   }

   /* 
      For OPEN, CLOSED, or XPRESSURE valve
      compute matrix coeffs. using the valvecoeff() function.                  
   */
   valvecoeff(p,k);  /*pipecoeff(k);*/                                           
   s->Aij[s->Ndx[k]] -= s->P[k];
   s->Aii[i] += s->P[k];
   s->Aii[j] += s->P[k];
   s->F[i] += (s->Y[k] - hyd->LinkFlows[k]);
   s->F[j] -= (s->Y[k] - hyd->LinkFlows[k]);
}                        /* End of psvcoeff */


void  fcvcoeff(EN_Project *p, int k, int n1, int n2)
/*
**--------------------------------------------------------------
**   Input:   k    = link index                                   
**            n1   = upstream node of valve                       
**            n2   = downstream node of valve                       
**   Output:  none                                                
**   Purpose: computes solution matrix coeffs. for flow control   
**            valve                                               
**--------------------------------------------------------------
*/
{
   int   i,j;                   /* Rows in solution matrix */
   double q;                     /* Valve flow setting      */
  
  hydraulics_t *hyd = &p->hydraulics;
  solver_t *s = &hyd->solver;
  
   q = hyd->LinkSetting[k];
   i = hyd->solver.Row[n1];
   j = hyd->solver.Row[n2];

   /*
      If valve active, break network at valve and treat  
      flow setting as external demand at upstream node   
      and external supply at downstream node.            
   */
   if (hyd->LinkStatus[k] == ACTIVE)
   {
      hyd->X_tmp[n1] -= q;
      s->F[i] -= q;
      hyd->X_tmp[n2] += q;
      s->F[j] += q;
      /*P[k] = 0.0;*/
      s->P[k] = 1.0/CBIG;                                                         
      s->Aij[s->Ndx[k]] -= s->P[k];                                                     
      s->Aii[i] += s->P[k];                                                          
      s->Aii[j] += s->P[k];                                                          
      s->Y[k] = hyd->LinkFlows[k] - q;
   }
   /*
     Otherwise treat valve as an open pipe
   */
   else 
   {
      valvecoeff(p,k);  //pipecoeff(k);                                          
      s->Aij[s->Ndx[k]] -= s->P[k];
      s->Aii[i] += s->P[k];
      s->Aii[j] += s->P[k];
      s->F[i] += (s->Y[k] - hyd->LinkFlows[k]);
      s->F[j] -= (s->Y[k] - hyd->LinkFlows[k]);
   }
}                        /* End of fcvcoeff */


/*** New function added. ***/                                                  
void valvecoeff(EN_Project *pr, int k)
/*
**--------------------------------------------------------------
**   Input:   k    = link index                                   
**   Output:  none                                                
**   Purpose: computes solution matrix coeffs. for a completely
**            open, closed, or throttled control valve.                                               
**--------------------------------------------------------------
*/
{
   double p;
  
  EN_Network *n = &pr->network;
  hydraulics_t *hyd = &pr->hydraulics;
  solver_t *s = &hyd->solver;
  
  Slink *link = &n->Link[k];
  
  double flow = hyd->LinkFlows[k];
  
   // Valve is closed. Use a very small matrix coeff.
   if (hyd->LinkStatus[k] <= CLOSED)
   {
      s->P[k] = 1.0/CBIG;
      s->Y[k] = flow;
      return;
   }

   // Account for any minor headloss through the valve
   if (link->Km > 0.0)
   {
      p = 2.0 * link->Km * fabs(flow);
      if ( p < hyd->RQtol ) {
        p = hyd->RQtol;
      }
      s->P[k] = 1.0 / p;
      s->Y[k] = flow / 2.0;
   }
   else
   {
      s->P[k] = 1.0 / hyd->RQtol;
      s->Y[k] = flow;
   }
}

/****************  END OF HYDRAUL.C  ***************/

