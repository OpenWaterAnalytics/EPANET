Release Notes for EPANET 2.2 (Draft)
============================
This document describes the changes and updates that have been made in version 2.2 of EPANET.

## Thread-Safe API Functions

A duplicate set of API functions has been provided that allow multiple EPANET projects to be analyzed concurrently in a thread-safe manner. These functions maintain the same name as the original but use a `EN_` prefix instead of `EN`. In addition, the first argument to each of these functions is a handle that identifies the particular project being analyzed. For example, instead of writing:

`ENgetnodevalue(nodeIndex, EN_ELEVATION, &elev)`

one would use:

`EN_getnodevalue(ph, nodeIndex, EN_ELEVATION, &elev)`

where `ph` is the handle assigned to the project. 

Two new functions have been added to the API to manage the creation and deletion of project handles. `EN_createproject` creates a new project along with its handle, while `EN_deleteproject` deletes a project. An example of using the thread-safe version of the API is shown below:
```
#include "epanet2_2.h"
int runEpanet(char *finp, char *frpt)
{
    EN_Project ph = 0;
    int err;
    err = EN_createproject(&ph);
    if (err) return err;
    err = EN_open(ph, finp, frpt, "");
    if (!err) err = EN_solveH(ph);
    if (!err) err = EN_report(ph);
    EN_close(ph);
    EN_deleteproject(&ph);
    return err;
}
```
Prototypes of the thread-safe functions appear in the `epanet2_2.h` header file while `epanet2.h` contains prototypes of the legacy-style API functions. The enumerated constants used with both types of functions have been moved to `epanet2_enums.h`.

## Network Building By Code

API users now have the ability to build a complete EPANET network model using just function calls, without the need to open an EPANET-formatted input file. All types of network objects can be created and have their properties set using these calls, including both simple and rule-based controls.  Here is an example of building a simple 2-node, 1-pipe network just through code:
```
#include "epanet2_2.h"
int buildandrunEpanet(char *frpt)
{
    EN_Project ph = 0;
    int err;
    err = EN_createproject(&ph);
    if (err) return err;
    EN_init(ph, frpt, "", EN_GPM, EN_HW);
    EN_addnode(ph, "J1, EN_JUNCTION);
    EN_setjuncdata(ph, 1, 710, 500, "");  //elev, demand
    EN_addnode(ph, "R1", EN_RESERVOIR);
    EN_setnodevalue(ph, 2, EN_ELEVATION, 800);
    EN_addlink(ph, "P1", EN_PIPE, "R1", "J1");
    EN_setpipedata(ph, 1, 5280, 14, 100, 0); // length, diam, C-factor
    EN_setreport(ph, "NODES ALL");
    err = EN_solveH(ph);
    if (!err) err = EN_report(ph);
    EN_close(ph);
    EN_deleteproject(&ph);
    return err;
}
```
Instead of using `EN_open` to load data from a file, `EN_init` is used to initialize a project with the names of its report and binary files, and its flow units and head loss formula. The legacy-style API has a complementary set of functions for building networks from code. 

## Additional Convergence Parameters

Two new analysis options have been added to provide more rigorous convergence criteria for EPANET's hydraulic solver. In the API they are named `EN_HEADERROR` and `EN_FLOWCHANGE` while in the `[OPTIONS]` section of an EPANET input file they are named `HEADERROR` and `FLOWCHANGE`, respectively.

`EN_HEADERROR` is the maximum head loss error that any network link can have for hydraulic convergence to occur. A link's head loss error is the difference between the head loss found as a function of computed flow in the link (such as by the Hazen-Williams equation for a pipe) and the difference in computed heads for the link's end nodes. The units of this parameter are feet (or meters for SI units). The default value of 0 indicates that no head error limit applies.

`EN_FLOWCHANGE` is the largest change in flow that any network element (link, emitter, or pressure-dependent demand) can have for hydraulic convergence to occur. It is specified in whatever flow units the project is using. The default value of 0 indicates that no flow change limit applies.

These new parameters augment the current `EN_ACCURACY` option which always remains in effect. In addition, both `EN_HEADERROR` and `EN_FLOWCHANGE` can be used as parameters in the `ENgetstatistic` (or `EN_getstatistic`) function to retrieve their computed values (even when their option values are 0) after a hydraulic solution has been completed.  

## Improved Linear Solver Routine

EPANET's hydraulic solver requires solving a system of linear equations over a series of iterations until a set of convergence criteria are met. The coefficient matrix of this linear system is square and symmetric. It has a row for each network node and a non-zero off-diagonal coefficient for each link. The numerical effort needed to solve the linear system can be reduced if the nodes are re-ordered so that the non-zero coefficients cluster more tightly around the diagonal.

EPANET's original node re-ordering scheme has been replaced by the more powerful **Multiple Minimum Degree (MMD)** algorithm. On a series of eight networks ranging in size from 7,700 to 100,000 nodes **MMD** reduced the solution time for a single period (steady state) hydraulic analysis by an average of 58%.

## Improved Handling of Near-Zero Flows

EPANET's hydraulic solver can generate an ill-conditioned solution matrix when pipe flows approach zero unless some adjustment is made (i.e., as a pipe's flow approaches 0 its head loss gradient also approaches 0 causing its reciprocal, which is used to form the solution matrix's coefficients, to approach infinity). EPANET 2.0 used an arbitrary cutoff on head loss gradient to prevent it from becoming 0. This approach doesn't allow a pipe to follow any head loss v. flow relation in the region below the cutoff and can produce incorrect solutions for some networks (see [Estrada et al., 2009](https://ascelibrary.org/doi/full/10.1061/%28ASCE%29IR.1943-4774.0000100)).

The hydraulic solver has been modified to use a linear head loss v. flow relation for flows approaching zero. For the Darcy-Weisbach equation, the linear Hagen-Poiseuille formula is used for laminar flow where the Reynolds Number is <= 2000. For the Hazen-Williams and Chezy-Manning equations, a flow limit `Qa` is established for each pipe, equal to the flow that produces the EPANET 2 gradient cutoff. For flows below this a linear head loss relation is used between 0 and the head loss at `Qa` and the gradient always equals the cutoff. EPANET 2.2 is now able to correctly solve the examples presented in Estrada et al. (2009) as well as those in [Gorev et al., (2013)](https://ascelibrary.org/doi/10.1061/%28ASCE%29HY.1943-7900.0000694) and [Elhay and Simpson (2011)](https://ascelibrary.org/doi/10.1061/%28ASCE%29HY.1943-7900.0000411).

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
int EN_setdemandmodel(EN_Project ph, int modelType, double pMin, double pReq, double pExp);
int EN_getdemandmodel(EN_Project ph, int *modelType, double *pMin, double *pReq, double *pExp);
```
for the thread-safe API. Some additional points regarding the new **PDA** option are:

 - If no DEMAND  MODEL and its parameters are specified then the analysis defaults to being demand driven (**DDA**).
 - This implementation of **PDA** assumes that the same parameters apply to all nodes in the network. Extending the framework to allow different parameters for specific nodes is left as a future feature to implement.
 - *P<sub>min</sub>* is allowed to equal to *P<sub>req</sub>*. This condition can be used to find a solution that results in the smallest amount of demand reductions needed to insure that no node delivers positive demand at a pressure below *P<sub>min</min>*.

## Improved Water Quality Mass Balance

As described by [Davis et al. (2018)](https://www.drink-water-eng-sci.net/11/25/2018/dwes-11-25-2018.pdf) EPANET's water quality simulations can result in some significant mass balance errors when modeling short term mass injections (errors are much smaller for continuous source flows). The entire water quality engine has been re-written to eliminate these errors. It still uses the Lagrangian Time Driven transport method but now analyzes each network node in topologically sorted order rather than in arbitrary order.

A Mass Balance Report now appears the end of a simulation's Status Report that lists the various components (inflow, outflow, reaction) that comprise the network's overall mass balance. In addition `EN_MASSBALANCE` can be used as a parameter in the `ENgetstatistic` (or `EN_getstatistic`) function to retrieve the Mass Balance Ratio (Total Outflow Mass / Total Inflow Mass) at any point during a water quality simulation.

Mass balance ratio (MBR) results for two of the networks analyzed by Davis et al. (2018) are shown in the following table. MBR-2.0 is for EPANET 2.0.012 as reported by Davis et al. while MBR-2.2 is for the re-written quality engine.

| Network | Time Step (s)  | MBR-2.0  | MBR-2.2 |
|--|--|--|--|
| N2 | 900  | 16.63 | 1.00 |
|       | 300 | 23.45 | 1.00 |
|       |  60 |  6.49  |  1.00 |
| N4 | 900 | 0.09 | 1.00 |
|  | 300 | 0.70 | 1.00 |
|  | 60 | 0.98 | 1.00 |

Both network files are available [here](https://doi.org/10.23719/1375314).

## New API functions
|Function|Description|
|--|--|
|`EN_createproject` | Creates a new EPANET project |
|`EN_deleteproject` | Deletes an EPANET project |
|`EN_init`|Initializes an EPANET project|
|`EN_setflowunits`|Sets the project's flow units|
|`EN_addnode`|Adds a new node to a project|
|`EN_addlink`|Adds a new link to a project|
|`EN_addcontrol`|Adds a new simple control to a project|
|`EN_addrule`|Adds a new control rule to a project|
|`EN_deletenode`|Deletes a node from the project|
|`EN_deletelink`|Deletes a link from the project|
|`EN_deletepattern`|Deletes a time pattern from the project|
|`EN_deletecurve`|Deletes a data curve from the project|
|`EN_deletecontrol`|Deletes a simple control from the project|
|`EN_deleterule`|Deletes a rule-based control from the project|
|`EN_setnodeid`|Changes the ID name for a node|
|`EN_setjuncdata` |Sets values for a junction's parameters |
|`EN_settankdata` |Sets values for a tank's parameters| 
|`EN_setlinkid`|Changes the ID name for a link|
|`EN_setlinknodes`|Sets a link's start- and end-nodes|
|`EN_setlinktype`|Changes the type of a specific link|
|`EN_setpipedata`|Sets values for a pipe's parameters|
|`EN_getdemandmodel`|Retrieves the type of demand model in use |
|`EN_setdemandmodel`|Sets the type of demand model to use|
|`EN_getdemandname`|Gets the name of a node's demand category|
|`EN_setdemandname`|Sets the name of a node's demand category|
|`EN_setdemandpattern`|Assigns a time pattern to a node's demand category |
|`EN_setpatternid`|Changes the ID name of a time pattern|
|`EN_setcurveid`|Changes the ID name of a data curve|
|`EN_getcurvetype`|Gets a curve's type|
|`EN_setheadcurveindex`|Sets the index of a head curve used by a pump |
|`EN_getrule`|Gets the number of elements in a rule-based control |
|`EN_getruleid` | Gets the name assigned to a rule-based control |
|`EN_getpremise`|Gets the contents of a premise in a rule-based control|
|`EN_setpremise`|Sets the contents of a premise in a rule-based control|
|`EN_setpremiseindex`|Sets the index of an object in a premise of a rule-based control|
|`EN_setpremisestatus`|Sets the status of an object in a premise of a rule-based control|
|`EN_setpremisevalue`|Sets the value of a property in a premise of a rule-based control|
|`EN_getthenaction`|Gets the contents of a THEN action in a rule-based control|
|`EN_setthenaction`|Sets the contents of a THEN action in a rule-based control|
|`EN_getelseaction`|Gets the contents of an ELSE action in a rule-based control|
|`EN_setelseaction`|Sets the contents of an ELSE action in a rule-based control|
|`EN_setrulepriority`|Sets the priority of a rule-based control|
|`EN_gettitle` |Gets a project's title |
|`EN_settitle` |Sets a project's title |
|`EN_clearreport` |Clears the contents of a project's report file |
|`EN_copyreport` | Copies the contents of a project's report file |
In addition to these new functions, a tank's volume curve `EN_VOLCURVE` can be set using `EN_setnodevalue` and `EN_setlinkvalue` can now be used to set the following pump properties:
 - `EN_PUMP_POWER` (constant power rating)
 - `EN_PUMP_HCURVE` (head characteristic curve)
 - `EN_PUMP_ECURVE` (efficiency curve)
 - `EN_PUMP_ECOST` (average energy price)
 - `EN_PUMP_EPAT` (energy pricing pattern)
 - `EN_LINKPATTERN` (speed setting pattern)
 
Access to the following global energy options have been added to  `EN_getoption` and `EN_setoption`:
 - `EN_GLOBALEFFIC` (global pump efficiency)
 - `EN_GLOBALPRICE` (global average energy price per kW-Hour)
 - `EN_GLOBALPATTERN`	(global energy price pattern)
 - `EN_DEMANDCHARGE` (price per maximum kW of energy consumption)

## New API Constants

### Link value types:
- `EN_PUMP_STATE`
- `EN_PUMP_EFFIC`
- `EN_PUMP_POWER`
- `EN_PUMP_HCURVE`
- `EN_PUMP_ECURVE`
- `EN_PUMP_ECOST`
- `EN_PUMP_EPAT`

### Count types:
 - `EN_RULECOUNT`

### Head loss formula:
 - `EN_HW`
 - `EN_DW`
 - `EN_CM`

### Hydraulic option types:
 - `EN_HEADERROR`
 - `EN_FLOWCHANGE`
 - `EN_DEFDEMANDPAT`
 - `EN_HEADLOSSFORM`
 - `EN_GLOBALEFFIC`
 - `EN_GLOBALPRICE`
 - `EN_GLOBALPATTERN`
 - `EN_DEMANDCHARGE`

### Simulation statistic types:
 - `EN_MAXHEADERROR`
 - `EN_MAXFLOWCHANGE`
 - `EN_MASSBALANCE`

### Action code types:
 - `EN_UNCONDITIONAL`
 - `EN_CONDITIONAL`

### Curve types:
 - `EN_VOLUME_CURVE`
 - `EN_PUMP_CURVE`
 - `EN_EFFIC_CURVE`
 - `EN_HLOSS_CURVE`
 - `EN_GENERIC_CURVE`

### Demand model types:
 - `EN_DDA`
 - `EN_PDA`
 
## Authors contributing to this release:
 - List item
