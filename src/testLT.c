/* Test file for epanet LemonTiger - Jinduan's version 
 The extension enables syncronized computation of hydraulics
 and water quality */

#include <stdio.h>
#include "types.h"
#include "vars.h"
#include "toolkit.h"
#include "lemontiger.h"


#ifdef CLE_LT

int main(int argc, char* argv[]) {
	int err = 0;   //error code
	long stime = 0;  //simulation time point, = t = Htime
	long step = 1;  //time to next time point, = tstep = hydstep
	long tleft = 0;  //time left in the simulation
	int id;   // some node id
	float value;   // some node/link value
	int TIME_A = 3600*10; 
	int TIME_B = 3600*20;  //two time points for testing


	/*  Asychronous solver (old epanet) */

	if (err=ENopen(argv[1], argv[2], "")) return err;
	ENgetnodeindex("184", &id);

	for (ENopenH(), ENinitH(1), step=1; 
		// must save intermediate results to disk (initH(1)), otherwise WQ solver won't execute
		step>0; ) {    

		ENrunH(&stime); ENnextH(&step);
		printf("stime = %d sec, step = %d sec.\n", stime, step);

		if (stime == TIME_A || stime == TIME_B) { // grab some results		
			ENgetnodevalue(id, EN_HEAD, &value);
			printf("Node 184's head = %f.\n", value);
		}			
	} 
	ENcloseH();

	printf("Reset time pointer and run WQ.\n");
	for (ENopenQ(), ENinitQ(0), step=1; step>0; ) { 
	/* this operation resets the internal time pointer (back to 0)*/
		ENrunQ(&stime); ENnextQ(&step);
		printf("stime = %d sec, step = %d sec.\n", stime, step);

		// grab some results
		if (stime == TIME_A || stime == TIME_B) {			
			ENgetnodevalue(id, EN_QUALITY, &value);
			printf("Node 184's quality = %f.\n", value);
		}
	}
	ENcloseQ();
	ENclose();


	/* Sychronous solver (LemonTiger) */
	
	if (err=ENopen(argv[1], argv[2], "")) return err;

	for (ENopeninitHQ(), tleft=Dur; tleft>0; ) {
		ENrunstepHQ(&stime, &tleft);
		printf("stime = %d sec, time left = %d sec.\n", stime, tleft);

		if (stime == TIME_A || stime == TIME_B) {
			ENgetnodevalue(id, EN_HEAD, &value);
			printf("Node 184's head = %f.\n", value);
			ENgetnodevalue(id, EN_QUALITY, &value);
			printf("Node 184's quality = %f.\n", value);
		}
	}
	ENcloseHQ();
	ENclose();
	

}


#endif