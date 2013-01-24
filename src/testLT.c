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
	int id, id2, id3;   // some node id
	float value;   // some node/link value
	int TIME_A = 3600*3; 
	int TIME_B = 3600*6;  //two time points for testing
	int TIME_C = 3600*10;


	/*  Asychronous solver (old epanet) */
	printf("*****Original EPANET results******\n");
	
	if (err=ENopen(argv[1], argv[2], "")) return err;
	ENgetnodeindex("184", &id);  // a node far away from water source
	ENgetlinkindex("101", &id2);  // a link close to the lake
	ENgetnodeindex("199", &id3);  // a node close to the lake (tracer point)
	
	for (ENopenH(), ENinitH(1), step=1; 
		// must save intermediate results to disk (initH(1)), otherwise WQ solver won't execute
		step>0; ENnextH(&step)) {    

		ENrunH(&stime);
		

		if (stime == TIME_A || stime == TIME_B || stime == TIME_C) { // grab some results
			printf("Hydraulic simulation time = %d sec, step = %d sec.\n", stime, step);

			ENgetnodevalue(id, EN_HEAD, &value);
			printf("Node 184's head = \t%f.\n", value);
			ENgetlinkvalue(id2, EN_FLOW, &value);
			printf("Link 101's flowrate = \t%f. \n", value);
			ENgetnodevalue(id3, EN_HEAD, &value);
			printf("Node 199's head = \t%f.\n", value);
		}			
	} 
	ENcloseH();

	printf("\nReset time pointer and run WQ.\n");
	for (step=1, ENopenQ(), ENinitQ(0); // this operation resets the internal time pointer (back to 0)
		 step>0; ENnextQ(&step)) { 
		ENrunQ(&stime);
		

		// grab some results
		if (stime == TIME_A || stime == TIME_B || stime == TIME_C) {
			printf("WQ simulation time = %d sec, step = %d sec.\n", stime, step);

			ENgetnodevalue(id, EN_QUALITY, &value);
			printf("Node 184's quality = \t%f.\n", value);
			ENgetnodevalue(id3, EN_QUALITY, &value);
			printf("Node 199's quality = \t%f.\n", value); 
		}
	}
	ENcloseQ();
	ENclose();
	

	/* Sychronous solver (LemonTiger) */
	printf("\n\n*****LemonTiger results******\n\n");

	if (err=ENopen(argv[1], argv[2], "")) return err;

	for (ENopeninitHQ(), tleft=Dur; tleft>0; ) {
		ENrunstepHQ(&stime, &tleft);
		
		

		if (stime == TIME_A || stime == TIME_B || stime == TIME_C) {
		//if (! (stime%1800)){
			printf("Simulation = %d sec, time left = %d sec.\n", stime, tleft);
			ENgetnodevalue(id, EN_HEAD, &value);
			printf("Node 184's head = \t%f.\n", value);
			
			ENgetnodevalue(id, EN_QUALITY, &value);
			printf("Node 184's quality = \t%f.\n", value);

			ENgetnodevalue(id3, EN_HEAD, &value);
			printf("Node 199's head = \t%f.\n", value);

			ENgetnodevalue(id3, EN_QUALITY, &value);
			printf("Node 199's quality = \t%f.\n", value);

			ENgetlinkvalue(id2, EN_FLOW, &value);
			printf("Link 101's flowrate = \t%f. \n", value);


			printf("\n");
		}
	}
	ENcloseHQ();
	ENclose();
	

}


#endif