#include "types.h"
#include "vars.h"
#include "funcs.h"
#include "toolkit.h"

extern char OutOfMemory;
extern int Haltflag;


int DLLEXPORT ENopeninitHQ() {
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

long timestepLT();
/* computes the length of the time step to next hydraulic simulation, but don't 
    update tank volumne and tank levels. During a sync HQ simulation,
	nextqual() will update the tank vols */

int  nexthydLT(long *tstep);
/* finds length of next time step but don't save
    results to hydraulics file. ignore reporting functions. */

void updateTanklevels();  
//Prior to running hydraulic simulation, update the tank levels.


/*!
 \fn int ENrunnexHQ( long* simTimePtr, long* timeStepPtr )
 \brief equivalent of ENnextQ, hydraulic solver is called on-demand
 \param simTimePtr Simulation time (output variable).
 \param timeStepPtr Time to next time step boundary (output variable).
 \return on error, an error code 
 */
int DLLEXPORT ENrunnextHQ(long* simTimePtr, long* timeStepPtr) {
  /* The lemonTiger equivalent of ENnextQ, hydraulic solver is called on-demand*/
  long    hydtime;       /* Hydraulic solution time */
  long    hydstep;       /* Hydraulic time step     */
  int     errcode = 0;
  
  /* if needed, push forward hydraulic simulation, similar to runqual() */
  if (Qtime == Htime)
  {
    if ( (errcode = runhyd(&hydtime)) || (errcode = nexthydLT(&hydstep)) ) {
      return errcode;
    }
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
  *simTimePtr = Htime;
  hydstep = Htime - Qtime;
  
  /* Perform water quality routing over this time step */
  if (Qualflag != NONE && hydstep > 0) transport(hydstep);
  
  updateTanklevels();
  /* Update current time */
  if (OutOfMemory) errcode = 101;
  if (!errcode) *timeStepPtr = hydstep;
  Qtime += hydstep;
  
  /* Save final output if no more time steps */
  if (!errcode && Saveflag && *timeStepPtr == 0) errcode = savefinaloutput();
  return(errcode);
  
}

int DLLEXPORT ENrunstepHQ(long* pstime /* Simulation time pointer */
				,long* ptleft /* Time left in the simulation*/) {

/* The LemonTiger equivalence of ENstepQ,  hydraulic solver is called on-demand */

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


int DLLEXPORT ENcloseHQ()  {
	int errcode = 0;
	if ( (errcode = ENcloseQ()) ||  (errcode = ENcloseH()) )
		return errcode;
	return errcode;
}




long  timestepLT(void) {
/* computes time step to advance hydraulic simulation, but don't 
    update tank levels. Instead, let nextqual() do the job. */
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


int  nexthydLT(long *tstep) {
/* finds length of next time step but don't updates tank volumnes and tank
    levels and rule-based contol actions. don't save
    results to hydraulics file. don't consider Report time. */
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

  
void updateTanklevels() {  //Prior to doing hydraulic simulation, update the tank levels
	int   i,n;
   for (i=1; i<=Ntanks; i++)   {
      /* Skip reservoirs */
      if (Tank[i].A == 0.0) continue;

	  n = Tank[i].Node;
      /* Check if tank full/empty within next second */
      if (Tank[i].V + D[n] >= Tank[i].Vmax) Tank[i].V = Tank[i].Vmax;
      if (Tank[i].V - D[n] <= Tank[i].Vmin) Tank[i].V = Tank[i].Vmin;

      H[n] = tankgrade(i,Tank[i].V);
   }
}


	
	