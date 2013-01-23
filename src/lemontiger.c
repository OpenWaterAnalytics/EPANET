#include "types.h"
#include "vars.h"
#include "funcs.h"
#include "toolkit.h"
#include "lemontiger.h"

extern char OutOfMemory;


int ENopeninitHQ() {
	int errcode = 0;
	Statflag = FALSE; //disable status report

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

int ENrunstepHQ(long* pstime /* Simulation time pointer */
				, long* ptleft /* Time left in the simulation*/) {
   long    hydtime;       /* Hydraulic solution time */
   long    hydstep;       /* Hydraulic time step     */
   int     errcode = 0;
   long    dt, hstep, tstep;

   /* Update reported simulation time */
   *pstime = Qtime;

   /* if needed, push forward hydraulic simulation */
   if (Qtime == Htime)
   {
	   if ( (errcode = runhyd(&hydtime))  ||
			(errcode = nexthyd(&hydstep))
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

   /* run WQ simulation similar to stepqual() */
   tstep = Qstep;

   do
   {
      dt = tstep;
      hstep = Htime - Qtime;
      if (hstep < dt)
      {
         dt = hstep;
         if (Qualflag != NONE) transport(dt);
         Qtime += dt;
         
		 /* if needed, push forward hydraulic simulation */
		 if ( (errcode = runhyd(&hydtime))  ||
			(errcode = nexthyd(&hydstep))
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
      }
      else
      {
         if (Qualflag != NONE) transport(dt);
         Qtime += dt;
      }
      tstep -= dt;
      if (OutOfMemory) errcode = 101;
   }  while (!errcode && tstep > 0);

   *ptleft = Dur - Qtime;
   if (!errcode && Saveflag && *ptleft == 0) errcode = savefinaloutput();
   return(errcode);
}

int ENcloseHQ()  {
	int errcode = 0;
	if ( (errcode = ENcloseQ()) ||  (errcode = ENcloseH()) )
		return errcode;
	return errcode;
}


	
	