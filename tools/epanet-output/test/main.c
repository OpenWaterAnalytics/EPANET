/*
 * main.c
 *
 *  Created on: Aug 4, 2014
 *      Author: mtryby
 */
#include <stdlib.h>
#include <stdio.h>
#include "../src/outputapi.h"
#include "../src/messages.h"

int testGetNodeAttribute(char* path) {

	int i, length, error = 0;
	float* array = NULL;
	char* err_msg = NULL;
	ENR_Handle p_handle = NULL;

	error = ENR_init(&p_handle);

	ENR_clearError(p_handle);
	error = ENR_open(p_handle, path);

	// Test getNodeAttribute
	error = ENR_getNodeAttribute(p_handle, -1, ENR_quality, &array, &length);

	if (!error)
	{
		for (i = 0; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

    ENR_checkError(p_handle, &err_msg);
    printf("%s\n", err_msg);

    ENR_free((void*)&array);
    ENR_free((void*)&err_msg);

    ENR_close(&p_handle);

	return error;
}


int testGetLinkAttribute(char* path) {

	int i, length, error;
	float* array = NULL;
	ENR_Handle p_handle = NULL;

	ENR_init(&p_handle);
	ENR_open(p_handle, path);

	// Test getLinkAttribute
	error = ENR_getLinkAttribute(p_handle, 1, ENR_flow, &array ,&length);

	if (!error)
	{
		for (i = 0; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

    ENR_free((void*)&array);
    ENR_close(&p_handle);

	return error;
}

int testGetNodeResult(char* path) {

	int i, length, error;
	float* array = NULL;
	ENR_Handle p_handle = NULL;

	ENR_init(&p_handle);
	ENR_open(p_handle, path);

	// Test getNodeResult
	error = ENR_getNodeResult(p_handle, 1, 2, &array, &length);

	if (!error)
	{
		for (i = 0; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

    ENR_free((void*)&array);
    ENR_close(&p_handle);

	return error;
}

int testGetLinkResult(char* path) {

	int i, length, error;
	float* array = NULL;
	ENR_Handle p_handle = NULL;

	ENR_init(&p_handle);
	ENR_open(p_handle, path);

	// Test getLinkResult
	error = ENR_getLinkResult(p_handle, 24, 13, &array, &length);

	if (!error)
	{
		for (i = 0; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

    ENR_free((void*)&array);
    ENR_close(&p_handle);

	return error;
}

int testGetNodeSeries(char* path) {

	int i, length, simDur, rptStep, endPeriod, error;
	float* series = NULL;
	ENR_Handle p_handle = NULL;

	ENR_init(&p_handle);
	ENR_open(p_handle, path);

	// Test getNodeSeries
	error = ENR_getTimes(p_handle, ENR_simDuration, &simDur);
	error = ENR_getTimes(p_handle, ENR_reportStep, &rptStep);
	endPeriod = simDur/rptStep;

	error = ENR_getNodeSeries(p_handle, 2, ENR_pressure, 0, endPeriod, &series, &length);

	if (!error) {
		for (i = 0; i < length; i++)
			printf("%f\n", series[i]);
	}
	printf("\n");

    ENR_free((void*)&series);
    ENR_close(&p_handle);

    return error;
}

int testGetLinkSeries(char* path) {

	int i, length, simDur, rptStep, endPeriod, error;
	float* series = NULL;
	ENR_Handle p_handle = NULL;

	ENR_init(&p_handle);
	ENR_open(p_handle, path);

	// Test getNodeSeries
	error = ENR_getTimes(p_handle, ENR_simDuration, &simDur);
	error = ENR_getTimes(p_handle, ENR_reportStep, &rptStep);
	endPeriod = simDur/rptStep;

	//series = ENR_newOutValueSeries(p_handle, 6, 12, &length, &error);
	error = ENR_getLinkSeries(p_handle, 2, ENR_flow, 0, endPeriod, &series, &length);

	if (!error) {
		for (i = 0; i < length; i++)
			printf("%f\n", series[i]);
	}
	printf("\n");

    ENR_free((void*)&series);
    ENR_close(&p_handle);

    return error;

}

int testGetNetReacts(char* path) {

	int i, length, error;
	float* array = NULL;
	ENR_Handle p_handle = NULL;

	ENR_init(&p_handle);
	ENR_open(p_handle, path);

	// Test Network Reaction Summary
	error = ENR_getNetReacts(p_handle, &array, &length);

	if (!error) {
		for (i = 0; i < length; i++)
		printf("%f\n", array[i]);
	}
	printf("\n");

    ENR_free((void*)&array);
    ENR_close(&p_handle);

    return error;
}

int testGetEnergyUsage(char* path) {

	int i, length, linkIdx, error;
	float **arg4 = (float **) 0 ;
	int* arg5 = (int*) 0;
	float *temp4 = 0 ;
	int temp5 = 0;
	ENR_Handle p_handle = NULL;

	ENR_init(&p_handle);
	ENR_open(p_handle, path);

    arg4 = &temp4;
    arg5 = &temp5;
	// Test Pump Energy Usage Summary
	error = ENR_getEnergyUsage(p_handle, 1, &linkIdx, arg4, arg5);

	if (!error) {
	for (i = 0; i < *arg5; i++)
		printf("%f\n", temp4[i]);
	}
	printf("\n");

    ENR_free((void*)arg4);
    ENR_close(&p_handle);

    return error;

}

int main(int nargs, char** args)
{
	char path[MAXFNAME] = "M:\\net mydocuments\\EPA Projects\\EPAnet Examples\\net1.out";

	testGetNodeAttribute(path);

	testGetLinkAttribute(path);

	testGetNodeResult(path);

	testGetLinkResult(path);

	testGetNodeSeries(path);

	testGetLinkSeries(path);

	testGetEnergyUsage(path);

	testGetNetReacts(path);

	return 0;
}

