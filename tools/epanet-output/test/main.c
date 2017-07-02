/*
 * main.c
 *
 *  Created on: Aug 4, 2014
 *      Author: mtryby
 */
#include <stdlib.h>
#include <stdio.h>
#include "../src/outputapi.h"



int testGetNodeAttribute(char *path) {

	int i, length, error;
	float *array;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test getNodeAttribute
	array  = ENR_newOutValueArray(enrapi, ENR_getAttribute, ENR_node, &length, &error);
	error = ENR_getNodeAttribute(enrapi, 1, ENR_quality, array);

	if (!error)
	{
		for (i = 1; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

	ENR_free(array);
    ENR_close(enrapi);

	return error;
}


int testGetLinkAttribute(char *path) {

	int i, length, error;
	float *array;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test getNodeAttribute
	array  = ENR_newOutValueArray(enrapi, ENR_getAttribute, ENR_link, &length, &error);
	error = ENR_getLinkAttribute(enrapi, 1, ENR_flow, array);

	if (!error)
	{
		for (i = 1; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

	ENR_free(array);
    ENR_close(enrapi);

	return error;
}

int testGetNodeResult(char *path) {

	int i, length, error;
	float *array;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test getNodeResult
	array  = ENR_newOutValueArray(enrapi, ENR_getResult, ENR_node, &length, &error);
	error = ENR_getNodeResult(enrapi, 1, 2, array);

	if (!error)
	{
		for (i = 1; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

	ENR_free(array);
    ENR_close(enrapi);

	return error;
}

int testGetLinkResult(char *path) {

	int i, length, error;
	float *array;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test getLinkResult
	array  = ENR_newOutValueArray(enrapi, ENR_getResult, ENR_link, &length, &error);
	error = ENR_getLinkResult(enrapi, 24, 13, array);

	if (!error)
	{
		for (i = 1; i < length; i++)
			printf("%f\n", array[i]);
	}
	printf("\n");

	ENR_free(array);
    ENR_close(enrapi);

	return error;
}

int testGetNodeSeries(char *path) {

	int i, length, simDur, rptStep, error;
	float *series;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test getNodeSeries
	error = ENR_getTimes(enrapi, ENR_simDuration, &simDur);
	error = ENR_getTimes(enrapi, ENR_reportStep, &rptStep);
	series = ENR_newOutValueSeries(enrapi, 0, simDur/rptStep, &length, &error);
	error = ENR_getNodeSeries(enrapi, 2, ENR_pressure, 0, length, series);

	if (!error) {
		for (i = 0; i < length; i++)
			printf("%f\n", series[i]);
	}
	printf("\n");

	ENR_free(series);
    ENR_close(enrapi);

    return error;
}

int testGetLinkSeries(char *path) {

	int i, length, simDur, rptStep, error;
	float *series;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test getNodeSeries
	error = ENR_getTimes(enrapi, ENR_simDuration, &simDur);
	error = ENR_getTimes(enrapi, ENR_reportStep, &rptStep);
	series = ENR_newOutValueSeries(enrapi, 6, 12, &length, &error);
	//error = ENR_getLinkSeries(enrapi, 2, ENR_flow, 0, length, series);

	if (!error) {
		for (i = 0; i < length; i++)
			printf("%f\n", series[i]);
	}
	printf("\n");

	ENR_free(series);
    ENR_close(enrapi);

    return error;

}

int testGetNetReacts(char *path) {

	int i, length, error;
	float *array;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test Network Reaction Summary
	array = ENR_newOutValueArray(enrapi, ENR_getReacts, 0, &length, &error);
	error = ENR_getNetReacts(enrapi, array);

	if (!error) {
		for (i = 1; i < length; i++)
		printf("%f\n", array[i]);
	}
	printf("\n");

    ENR_free(array);
    ENR_close(enrapi);

    return error;
}

int testGetEnergyUsage(char *path) {

	int i, length, linkIdx, error;
	float *array;

	ENResultsAPI *enrapi = ENR_init();
	ENR_open(enrapi, path);

	// Test Pump Energy Usage Summary
	array = ENR_newOutValueArray(enrapi, ENR_getEnergy, 0, &length, &error);
	error = ENR_getEnergyUsage(enrapi, 1, &linkIdx, array);

	if (!error) {
	for (i = 1; i < length; i++)
		printf("%f\n", array[i]);
	}
	printf("\n");

	ENR_free(array);
    ENR_close(enrapi);

    return error;

}

int main(int nargs, char **args)
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
}

