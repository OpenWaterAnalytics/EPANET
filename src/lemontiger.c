#include "types.h"
#include "vars.h"
#include "funcs.h"
#include "toolkit.h"
#include "lemontiger.h"

extern char OutOfMemory;
extern int Haltflag;


int ENopeninitHQ() {
	int errcode = 0;

	if (Hstep % Qstep) {
		errcode = 401;
		errmsg(errcode);
		return errcode;
	}

	Statflag = TRUE; //disable status report

	if (errcode = ENopenH()) return errcode;

	// Open WQ solver, but don't check SaveHflag as in ENopenQ()
	ERRCODE(openqual());
    if (!errcode) OpenQflag = TRUE;
    else {
		errmsg(errcode);
		return errcode;
	}

	if (errcode = ENinitH(1)) return errcode;
	if (errcode = ENinitQ(0)) return errcode;

	Rtime = Rstep; //use ENinitH()'s setup
	return errcode;
}

long  timestepLT(void)
/*
**----------------------------------------------------------------
**  Input:   none                                                
**  Output:  returns time step until next change in hydraulics   
**  Purpose: computes time step to advance hydraulic simulation, but don't update tank levels
**			 Let nextqual() to do the job.
**----------------------------------------------------------------
*/
{
   long   n,t,tstep;

   /* Normal time step is hydraulic time step */
   tstep = Hstep;

   /* Revise time step based on time until next demand period */
   n = ((Htime+Pstart)/Pstep) + 1;   /* Next pattern period   */
   t = n*Pstep - Htime;              /* Time till next period */
   if (t > 0 && t < tstep) tstep = t;

   /* Revise time step based on time until next reporting period */
   t = Rtime - Htime;
   if (t > 0 && t < tstep) tstep = t;

   /* Revise time step based on smallest time to fill or drain a tank */
   tanktimestep(&tstep);

   /* Revise time step based on smallest time to activate a control */
   controltimestep(&tstep);

   /* Evaluate rule-based controls (which will also update tank levels) */
   if (Nrules > 0) ruletimestep(&tstep);
   
   return(tstep);
}

int  nexthydLT(long *tstep)
/*
**--------------------------------------------------------------
**  Input:   none     
**  Output:  tstep = pointer to time step (in seconds)
**  Returns: error code                                          
**  Purpose: finds length of next time step & updates tank
**           levels and rule-based contol actions. don't save
**			 results to hydraulics file. don't consider Report time.
**--------------------------------------------------------------
*/
{
   long  hydstep;         /* Actual time step  */
   int   errcode = 0;     /* Error code        */

   if (Haltflag) Htime = Dur;

   /* Compute next time step & update tank levels */
   *tstep = 0;
   hydstep = 0;
   if (Htime < Dur) hydstep = timestepLT();

   /* Compute pumping energy */
   if (Dur == 0) addenergy(0);
   else if (Htime < Dur) addenergy(hydstep);

   /* Update current time. */
   if (Htime < Dur)  /* More time remains */
   {
      Htime += hydstep;
   }
   else
   {
      Htime++;          /* Force completion of analysis */
   }
   *tstep = hydstep;
   return(errcode);
}

  
void updateTanklevels() {
	int   i,n;

   for (i=1; i<=Ntanks; i++)
   {
	  
      /* Skip reservoirs */
      if (Tank[i].A == 0.0) continue;

	  n = Tank[i].Node;
      /* Check if tank full/empty within next second */
      if (Tank[i].V + D[n] >= Tank[i].Vmax) Tank[i].V = Tank[i].Vmax;
      if (Tank[i].V - D[n] <= Tank[i].Vmin) Tank[i].V = Tank[i].Vmin;

      H[n] = tankgrade(i,Tank[i].V);
   }
}

int ENrunstepHQ(long* pstime /* Simulation time pointer */
				, long* ptleft /* Time left in the simulation*/) {
   long    hydtime;       /* Hydraulic solution time */
   long    hydstep;       /* Hydraulic time step     */
   int     errcode = 0;
   long    dt, hstep, tstep;

   /* if needed, push forward hydraulic simulation, similar to runqual() */
   if (Qtime == Htime)
   {
	   if ( (errcode = runhyd(&hydtime))  ||
			(errcode = nexthydLT(&hydstep))
			) return errcode;
	      /* If simulating WQ: */
	   if (Qualflag != NONE && Qtime < Dur) {

			/* Compute reaction rate coeffs. */
			if (Reactflag && Qualflag != AGE) ratecoeffs();

			/* Initialize pipe segments (at time 0) or  */
			/* else re-orient segments if flow reverses.*/
			if (Qtime == 0) initsegs();
			else            reorientsegs();
		}
        Htime = hydtime + hydstep;
   }

   /* run WQ simulation, similar to stepqual() */
   tstep = Qstep;

   do {
      dt = tstep;
      hstep = Htime - Qtime;
      if (hstep < dt) {/* Htime is closer */
         dt = hstep;
         if (Qualflag != NONE) transport(dt);
         Qtime += dt;
         
		 updateTanklevels();
		 /* if needed, push forward hydraulic simulation */
		 if ( (errcode = runhyd(&hydtime))  ||
			(errcode = nexthydLT(&hydstep))
			) return errcode;
		 if (Qualflag != NONE && Qtime < Dur) {

			/* Compute reaction rate coeffs. */
			if (Reactflag && Qualflag != AGE) ratecoeffs();

			/* Initialize pipe segments (at time 0) or  */
			/* else re-orient segments if flow reverses.*/
			if (Qtime == 0) initsegs();
			else            reorientsegs();
		}
		 Htime = hydtime + hydstep;
         Qtime = hydtime;

      } else { /* Qtime is closer */
         if (Qualflag != NONE) transport(dt);
         Qtime += dt;
      }
      tstep -= dt;
      if (OutOfMemory) errcode = 101;
   }  while (!errcode && tstep > 0); /*do it until Qstep is elapsed.*/

   *ptleft = Dur - Qtime;
   if (!errcode && Saveflag && *ptleft == 0) errcode = savefinaloutput();

   /* if needed, push forward hydraulic simulation again, so that hyd and wq states are consistent. */
   if (Qtime == Htime && Htime < Dur)    {
	   updateTanklevels();
	   if ( (errcode = runhyd(&hydtime))  ||
			(errcode = nexthydLT(&hydstep))
			) return errcode;
	     //  If simulating WQ: 
	   if (Qualflag != NONE && Qtime < Dur) {

			// Compute reaction rate coeffs. 
			if (Reactflag && Qualflag != AGE) ratecoeffs();

			// Initialize pipe segments (at time 0) or  
			// else re-orient segments if flow reverses.
			if (Qtime == 0) initsegs();
			else            reorientsegs();
		}
        Htime = hydtime + hydstep;
   }
   

   /* Update reported simulation time */
   *pstime = Qtime;

   return(errcode);
}

int ENcloseHQ()  {
	int errcode = 0;
	if ( (errcode = ENcloseQ()) ||  (errcode = ENcloseH()) )
		return errcode;
	return errcode;
}


	
	