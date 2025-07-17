>
## Release Notes for EPANET 2.3

This document describes the changes and updates that have been made in version 2.3 of EPANET.

### New Features

 - A `EN_setcurvetype` function was added to allow API clients to set a curve's type (e.g., `EN_PUMP_CURVE,` `EN_VOLUME_CURVE,` etc.).

 - A `EN_setvertex` function was added to allow API clients to change the coordinates of a single link vertex.

- Support has been added for FAVAD (Fixed And Variable Area Discharge) modeling of pipe leaks:
   - A new `[LEAKAGE]` section has been added to the input file format where each line contains the ID name of a pipe, its leak area in sq. mm per 100 length units, and its leak expansion rate in sq. mm per unit of pressure head.
   - `EN_LEAK_AREA` and `EN_LEAK_EXPAN` can be used with the functions `EN_getlinkvalue` and `EN_setlinkvalue` to retrieve and assign values for a pipe's leak area and expansion properties.
   - `EN_LINK_LEAKAGE` can be used with `EN_getlinkvalue` to retrieve a pipe's leakage rate at a given point in time.
   - `EN_LEAKAGEFLOW` can be used with `EN_getnodevalue` to retrieve the leakage demand generated at a node from all its connecting pipes at a given point in time.
   - `EN_LEAKAGELOSS` can be used with `EN_getstatistic` to retrieve the total leakage loss in the system at a given point in time as a percentage of total flow entering the system.

 - Support has been added for reading the `[TAGS]` section of an EPANET input file. In addition:
   - A newly added `EN_settag` function will assign a Tag to a node or link.
   - A newly added `EN_gettag` function will retrieve a node or link's Tag.
   - The existing `EN_saveinpfile` will include saving all node and link tags to file.
 
 - A new Flow Balance Report has been added to the end of a simulation run's Status Report that lists the various components of the system's total inflow and outflow over the simulation period. It also displays the ratio of outflow to inflow as a check on flow continuity.

 - A new type of valve, a Positional Control Valve (PCV), was added. It uses a valve characteristic curve to relate its loss coefficient to a percentage open setting (parameter - `EN_PCV`).

 - `EN_VALVE_CURVE` can now be used with the `EN_getcurvetype` and `EN_setcurvetype` to get or set the valve position curve.

 - `EN_VALVE_TYPE` can now be used with `EN_getlinkvalue` and `EN_setlinkvalue` to get and set a valve's type. This is the preferred way to change just a valve's type rather than using `EN_setlinktype`.

 - A new set of functions has been added to get information about upcoming time step events. Users will now see what type of event is going to cause the end of a time step to occur. See `EN_timetonextevent`.

 - A new set of functions has been added to allow users to set a reporting callback function. The user-supplied function will receive all output normally directed to the report file.

 - A `EN_EMITBACKFLOW` option was added that either allows emitters to have reverse flow through them (the default) or not.

 - The functions `EN_getnodevalue` and `EN_getlinkvalue` now include the options `EN_NODE_INCONTROL` and `EN_LINK_INCONTROL` to determine whether a node or link appears in any simple or rule-based control.

 - `EN_SET_CLOSED` and `EN_SET_OPEN` constants were added that can be used with `EN_setcontrol` to fix the status of pipes and valves to completely closed or completely open.

 - `EN_STATUS_REPORT` can now be used with `EN_getoption` and `EN_setoption` to get or set the type of status report that EPANET will generate (`EN_NO_REPORT`, `EN_NORMAL_REPORT` or `EN_FULL_REPORT`).  

 - `EN_PRESS_UNITS` can now be used with `EN_getoption` and `EN_setoption` to get or set a project's pressure units. The choices are EN_PSI, EN_KPA, EN_METERS, EN_BAR, or EN_FEET.

 - Pressure units have been decoupled from the flow unit system, allowing them to be set independently to support mixed-unit conventions (e.g., using LPS for flow and PSI for pressure). 

 - The following constants can be used with EN_getnodevalue to retrieve the components of a node's total demand at a given point in time:
   - `EN_FULLDEMAND` - the consumer demand requested
   - `EN_DEMANDFLOW` - the consumer demand delivered
   - `EN_DEMANDDEFICIT` - the difference between the consumer demand requested and delivered
   - `EN_EMITTERFLOW` - the node's emitter flow
   - `EN_LEAKAGEFLOW` - the node's leakage flow
   - `EN_DEMAND` - the sum of the node's consumer demand, emitter flow, and leakage flow

 - Additional API functions for enabling/disabling controls and rules were added (`EN_getcontrolenabled`, `EN_setcontrolenabled`, `EN_getruleenabled`, `EN_setruleenabled`). A new keyword `DISABLED` is added to the end of a control or rule statement in an EPANET input file to indicate that the control is disabled.

 - The `EN_openX` function has been added to enable the opening of input files with formatting errors through the API. This allows users to continue using toolkit functions even when such errors are present.

 - The `EN_getnodesvalues` and `EN_getlinksvalues` were added to retrieve a property value for all nodes or links in the network.

 - Support was added for cubic meters per second (`EN_CMS`) flow units.

 - A header file for binding C# to the Toolkit has been added.

 - Support was added for Conan dependency manager.

### Feature Updates

 - The check for at least two nodes, one tank/reservoir and no unconnected junction nodes was moved from `EN_open` to `EN_openH` and `EN_openQ` so that partial network data files could be opened by the toolkit.

 - The indices of a General Purpose Valve (GPV) and a Positional Control Valve (PCV) were added to the list of editable Link Properties using the symbolic constant names `EN_GPV_CURVE` and `EN_PCV_CURVE`, respectively.

 - The `EN_getlinkvalue` and `EN_setlinkvalue` functions were updated to get and set the values of `EN_GPV_CURVE` and `EN_PCV_CURVE`.

 - Negative pressure values for `EN_SETTING` are now permitted in the `EN_setlinkvalue` function. 

 - The `EN_STARTTIME` parameter was added into the `EN_settimeparam` function.

 - A `EN_DEMANDPATTERN` parameter was added as the index of the default time pattern used by demands with no specific pattern assigned. It can be set or retrieved with the `EN_setoption` and `EN_getoption` functions, respectively, and is saved to the file when the `EN_saveinpfile` function is called.

 - The `EN_getaveragepatternvalue` function will now accept a pattern index of zero which represents the constant pattern assigned to junction demands by default.

 - Improved updating and convergence tests were added to pressure-dependent demand analysis.

 - Improved checks to prevent outflow from empty tanks or inflow to full (non-overflow) tanks, including the case where a link is connected to a pair of tanks, were added.

 - The `EN_INITSETTING` option in function `EN_setlinkvalue` will now save the setting value so that if a new simulation is begun or if  `EN_saveinpfile` is called the saved initial setting will remain in effect rather than whatever setting a simulation may have ended with.

 - A new error code `263 - node is not a tank` is returned when `EN_settankdata` or `EN_setnodevalue` attempts to set a tank-only parameter for a non-tank node.

 - Errors in node and link vertex coordinates are now ignored when reading an EPANET input file.

 - Only non-zero demands are now included in the `[DEMANDS]` section of the input file produced by `EN_saveinpfile`.

 - Setting the demand multiplier within the `[DEMANDS]` section of INP has been depreciated, please use `DEMAND MULTIPLIER` inside `[OPTIONS]` instead.

 - Continuous barrier functions were added to constrain emitter flows to allowable values.

 - The CI regression test protocol was modified by:
   - changing the absolute tolerance used to compare the closeness of test results to benchmark values from 0 to 0.0001
   - dropping the "correct decimal digits" test 
   - dropping the check for identical status report content since it prevents accepting code changes that produce more accurate solutions in fewer iterations.

### Bug Fixes

 - The adjustment of a tank's minimum volume (`Vmin`) when its parameters are changed using `EN_setnodevalue` or `EN_settankdata` has been corrected. 

 - A pump whose status is set to CLOSED in the input file now also has its speed setting set to zero which allows a simple pressure control to activate the pump correctly.

 - A failure to raise an error condition for a non-positive pipe roughness in the input file has been fixed.

 - The calculation of head loss gradient for low flow conditions was corrected.

 - A possible loss of network connectivity when evaluating a Pressure Sustaining Valve was prevented.

 - Having the implied loss coefficient for an active Flow Control Valve be less than its fully opened value was prevented.

 - An incorrect tank elevation value set using `EN_settankdata` with SI units has been fixed.

 - An error is no longer raised when a minor loss coefficient of zero is assigned in `EN_setlinkvalue(ph, index, EN_MINORLOSS, 0)`.

 - The incorrect display of unconnected nodes has been fixed.

 - The function `EN_saveinpfile` was corrected for simple controls on GPV's by saving their status instead of the index of their head loss curve.

 - The internal Qualflag variable is now adjusted when an EPANET input file has a QUALITY option not equal to NONE and a simulation duration of zero.

 - An EPANET input file with simple timer control that has more than 9 input tokens no longer results in an incorrect hour setting.

 - A possible parser error that could result in a Trace Node ID in an input file not being recognized was fixed.

 - Updated the internal function `getclosedlink` in report.c to use a loop instead of recursion to prevent a stack overflow during the analysis of very large disconnections.

 - Fixed a bug in EN_setnodevalue with EN_EMITTER option that could cause NaN results.

 - A failure to close a temporary hydraulics file between successive simulations of an opened project was fixed.
 
 - Corrupting the index of a water quality Trace Node when adding or deleting a node was fixed.
