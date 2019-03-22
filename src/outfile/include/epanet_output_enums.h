/*
 *  epanet_output_enums.h - EPANET Output API enums
 *
 *  Created on: March 11, 2019
 *
 *      Author: Michael E. Tryby
 *              US EPA - ORD/NRMRL
 */


#ifndef EPANET_OUTPUT_ENUMS_H_
#define EPANET_OUTPUT_ENUMS_H_


typedef enum {
    ENR_node        = 1,
    ENR_link        = 2
} ENR_ElementType;

typedef enum {
    ENR_flowUnits   = 1,
    ENR_pressUnits  = 2,
    ENR_qualUnits   = 3
} ENR_Units;

typedef enum {
    ENR_CFS         = 0,
    ENR_GPM         = 1,
    ENR_MGD         = 2,
    ENR_IMGD        = 3,
    ENR_AFD         = 4,
    ENR_LPS         = 5,
    ENR_LPM         = 6,
    ENR_MLD         = 7,
    ENR_CMH         = 8,
    ENR_CMD         = 9
} ENR_FlowUnits;

typedef enum {
	ENR_PSI         = 0,
	ENR_MTR         = 1,
	ENR_KPA         = 2
} ENR_PressUnits;

typedef enum {
    ENR_NONE        = 0,
    ENR_MGL         = 1,
    ENR_UGL         = 2,
    ENR_HOURS       = 3,
    ENR_PRCNT       = 4
} ENR_QualUnits;

typedef enum {
    ENR_reportStart = 1,
    ENR_reportStep  = 2,
    ENR_simDuration = 3,
    ENR_numPeriods  = 4
}ENR_Time;

typedef enum {
    ENR_demand      = 1,
    ENR_head        = 2,
    ENR_pressure    = 3,
    ENR_quality     = 4
} ENR_NodeAttribute;

typedef enum {
    ENR_flow        = 1,
    ENR_velocity    = 2,
    ENR_headloss    = 3,
    ENR_avgQuality  = 4,
    ENR_status      = 5,
    ENR_setting     = 6,
    ENR_rxRate      = 7,
    ENR_frctnFctr   = 8
} ENR_LinkAttribute;


#endif /* EPANET_OUTPUT_ENUMS_H_ */
