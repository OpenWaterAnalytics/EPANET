
Release Notes for EPANET 2.2 (Draft)
============================

This document describes the changes and updates that have been made to version 2.2 of EPANET.

## Thread-Safe API Functions

A duplicate set of the version 2.1 API functions has been provided that allow multiple EPANET projects to be analyzed concurrently in a thread-safe manner. These functions maintain the same name as the original but use a `EN_` prefix instead of `EN`. In addition, the first argument to each of these functions is a pointer to an `EN_Project` structure that encapsulates the network data for the particular project being analyzed. For example, instead of writing:

`ENgetnodevalue(nodeIndex, EN_ELEVATION, &elev)`

one would use:

`EN_getnodevalue(pr, nodeIndex, EN_ELEVATION, &elev)`

where `pr` is the pointer to an `EN_Project`.

Two new functions have been added to the API to manage the creation and deletion of project pointers. `EN_createproject` creates a new project along with a pointer to it, while `EN_deleteproject` deletes a project. An example of using the thread-safe version of the API is shown below:
```
#include "epanet2.h"
int runEpanet(char *finp, char *frpt)
{
    EN_Project *pr = NULL;
    int err;
    err = EN_createproject(&pr);
    if (err) return err;
    err = EN_open(pr, finp, frpt, "");
    if (!err) err = EN_solveH(pr);
    if (!err) err = EN_report(pr);
    EN_close(pr);
    EN_deleteproject(pr);
    return err;
}
```

## Additional Convergence Parameters

Two new analysis options have been added to provide more rigorous convergence criteria for EPANET's hydraulic solver. In the API they are named `EN_HEADERROR` and `EN_FLOWCHANGE` while in the `[OPTIONS]` section of an EPANET input file they are named `HEADERROR` and `FLOWCHANGE`, respectively.

`EN_HEADERROR` is the maximum head loss error that any network link can have for hydraulic convergence to occur. A link's head loss error is the difference between the head loss found as a function of computed flow in the link (such as by the Hazen-Williams equation for a pipe) and the difference in computed heads for the link's end nodes. The units of this parameter are feet (or meters for SI units). The default value of 0 indicates that no head error limit applies.

`EN_FLOWCHANGE` is the largest change in flow that any network element (link, emitter, or pressure-dependent demand) can have for hydraulic convergence to occur. It is specified in whatever flow units the project is using. The default value of 0 indicates that no flow change limit applies.

These new parameters augment the current `EN_ACCURACY` option which always remains in effect. In addition, both `EN_HEADERROR` and `EN_FLOWCHANGE` can be used as parameters in the `ENgetstatistic` (or `EN_getstatistic`) function to retrieve their computed values (even when their option values are 0) after a hydraulic solution has been completed.  

## Improved Linear Solver Routine
EPANET's hydraulic solver requires solving a system of linear equations over a series of iterations until a set of convergence criteria are met. The coefficient matrix of this linear system is square and symmetric. It has a row for each network node and a non-zero off-diagonal coefficient for each link. The numerical effort needed to solve the linear system can be reduced if the nodes are re-ordered so that the non-zero coefficients cluster more tightly around the diagonal.

EPANET's original node re-ordering scheme has been replaced by the more powerful **Multiple Minimum Degree (MMD)** algorithm. On a series of eight networks ranging in size from 7,700 to 100,000 nodes **MMD** reduced the solution time for a single period (steady state) hydraulic analysis by an average of 58%.

## Pressure Dependent Demands
EPANET has always employed a Demand Driven Analysis (**DDA**) when modeling network hydraulics. Under this approach nodal demands at a given point in time are fixed values that must be delivered no matter what nodal heads and link flows are produced by a hydraulic solution. This can result in situations where required demands are satisfied at nodes that have negative pressures - a physical impossibility. 

To address this issue EPANET has been extended to use a Pressure Driven Analysis (**PDA**) if so desired. Under **PDA**, the demand *D* delivered at a node depends on the node's available pressure *P* according to:

*D = D<sub>f</sub> [ (P - P<sub>min</sub>) / (P<sub>req</sub> - P<sub>min</sub>) ]<sup>P<sub>exp</sub></sup>*

where *D<sub>f</sub>* is the full demand required, *P<sub>min</sub>* is the pressure below which demand is zero, *P<sub>req</sub>* is the pressure required to deliver the full required demand and *P<sub>exp</sub>* is an exponent. When *P < P<sub>min</sub>* demand is 0 and when *P > P<sub>req</sub>* demand equals *D<sub>f</sub>*.

To implement pressure driven analysis four new parameters have been added to the [OPTIONS] section of the EPANET input file:


| Parameter | Description  | Default |
|--|--|--|
| DEMAND MODEL | either DDA or PDA | DDA |
| MINIMUM PRESSURE | value for *P<sub>min</sub>* | 0
| REQUIRED PRESSURE | value for *P<sub>req</sub>* | 0
| PRESSURE EXPONENT | value for *P<sub>exp</sub>* | 0.5 |

These parameters can also be set and retrieved in code using the following API functions
```
int ENsetdemandmodel(int modelType, double pMin, double pReq, double pExp);
int ENgetdemandmodel(int *modelType, double *pMin, double *pReq, double *pExp);
```
for the legacy API and
```
int EN_setdemandmodel(EN_Project *pr, int modelType, double pMin, double pReq, double pExp);
int EN_getdemandmodel(EN_Project *pr, int *modelType, double *pMin, double *pReq, double *pExp);
```
for the thread-safe API. Some additional points regarding the new **PDA** option are:

 - If no DEMAND  MODEL and its parameters are specified then the analysis defaults to being demand driven (**DDA**).
 - This implementation of **PDA** assumes that the same parameters apply to all nodes in the network. Extending the framework to allow different parameters for specific nodes is straightforward to do but is left as a future feature to implement.
 - *P<sub>min</sub>* is allowed to equal to *P<sub>req</sub>*. This condition can be used to find a solution that results in the smallest amount of demand reductions needed to insure that no node delivers positive demand at a pressure below *P<sub>min</min>*.


## Code Changes

 - The header file `vars.h` containing global variables has been eliminated. Instead a number of new structures incorporating these variables has been added to `types.h`. These structures have been incorporated into the new `EN_Project` structure, also defined in `types.h`, which gets passed into each of the thread-safe API functions as a pointer.
 - Each of the legacy API functions now simply calls its thread-safe counterpart passing in a pointer to a default global`EN_Project` variable that is declared in `types.h`.
 -  Throughout all code modules, global variables that were previously accessed through `vars.h` are now accessed using the `EN_Project` pointer that is passed into the functions where the variables appear.
 - The exceedingly long `hydraul.c` file has been split into four separate files:
     - `hydraul.c` now contains just the code needed to initialize a hydraulic analysis, set demands and control actions at each time step, and determine the length of the next time step to take.
     - `hydsolver.c` implements EPANET's hydraulic solver at a single point in time.
     - `hydcoeffs.c` computes values of the matrix coefficients (derived from link head losses and their gradients) used by the hydraulic solver.
     - `hydstatus.c` checks for status changes in valves and pumps as requested by the hydraulic solver.
 - The Multiple Minimum Degree re-ordering algorithm appears in a new file named `genmmd.c`. This is 1990's legacy code that is readily available on the web and can be found in several linear equation solver libraries.

## General changes
 - Read and write demand categories names

## New API functions
|Function|Description|
|--|--|
|`ENaddlink`|  |
|`ENaddnode`|  |
|`ENgetcurvetype`|  |
|`ENgetdemandmodel`||
|`ENsetdemandmodel`||
|`ENsetflowunits`||
|`ENaddcontrol`||
|`ENsetdemandpattern`||
|`ENgetrule`||
|`ENsetrulepriority`||
|`ENgetpremise`||
|`ENsetpremise`||
|`ENsetpremiseindex`||
|`ENsetpremisestatus`||
|`ENsetpremisevalue`||
|`ENgettrueaction`||
|`ENsettrueaction`||
|`ENgetfalseaction`||
|`ENsetfalseaction`||
|`ENgetruleID`||
|`ENinit`||
|`ENsetheadcurveindex`||
|`ENsetlinktype`||
|`ENaddnode`||
|`ENaddlink`||
|`ENdeletelink`||
|`ENdeletenode`||
|`ENgetdemandname`||
|`ENsetdemandname`||

## API Extensions (additional definitions)
### Link value types:
- `EN_EFFICIENCY`
- `EN_HEADCURVE`
- `EN_EFFICIENCYCURVE`
- `EN_PRICEPATTERN`
- `EN_STATE`
- `EN_CONST_POWER`
- `EN_SPEED`
### Count types:
 - `EN_RULECOUNT`
### Head loss formula:
 - `EN_HW`
 - `EN_DW`
 - `EN_CM`
### Misc. options:
 - `EN_HEADERROR`
 - `EN_FLOWCHANGE`
 - `EN_DEMANDDEFPAT`
### Curve types:
 - `EN_V_CURVE`
 - `EN_P_CURVE`
 - `EN_E_CURVE`
 - `EN_H_CURVE`
 - `EN_G_CURVE`

## Authors contributing to this release:
 - List item

