/*
 *   test_reent.cpp
 *
 *   Created: 8/30/2018
 *   Author: Michael E. Tryby
 *           US EPA - ORD/NRMRL
 *
 *   Multi-threading / reentrancy test for EPANET Toolkit API.
*/

#include <string>
#include <stdio.h>
#include <stdlib.h>

#include <boost/thread.hpp>

#include "epanet2_2.h"

#define NUM_THREADS 2

using namespace std;

void  epanet_thread(long i)
{
    int errorcode = 0;
    EN_Project ph;

    string prefix = "example_";
    string suffix = ".inp";
    string input = prefix + to_string(static_cast<long long>(i)) + suffix;

    suffix = ".rpt";
    string report = prefix + to_string(static_cast<long long>(i)) + suffix;

    suffix = ".out";
    string output = prefix + to_string(static_cast<long long>(i)) + suffix;

    printf("Thread #%ld starting EPANET ...\n", i);

    EN_createproject(&ph);
    errorcode = EN_runproject(ph, input.c_str(), report.c_str(), output.c_str(), NULL);
    EN_deleteproject(&ph);

    printf("Thread #%ld EPANET done. Status = %d\n", i, errorcode);
}

int main(int argc, char *argv[])
{
    long i;
    boost::thread *threads[NUM_THREADS];

    for (i = 0; i < NUM_THREADS; i++) {
        threads[i] = new boost::thread(epanet_thread, i);
        printf("Main: creating thread %ld.\n", i);
    }

    for (i = 0; i < NUM_THREADS; i++) {
        threads[i]->join();
        printf("Main: joining thread %ld.\n", i);
        delete threads[i];
    }

    printf("Main: program completed. Exiting.\n");
    return(0);
}
