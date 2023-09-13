>
## Release Notes for EPANET 2.3

This document describes the changes and updates that have been made in version 2.3 of EPANET.

 - The check for at least two nodes, one tank/reservoir and no unconnected junction nodes was moved from `EN_open` to `EN_openH` and `EN_openQ` so that partial network data files could be opened by the toolkit.
 - A `EN_setcurvetype` function was added to allow API clients to set a curve's type (e.g., `EN_PUMP_CURVE,` `EN_VOLUME_CURVE,` etc.).
 - A `EN_setvertex` function was added to allow API clients to change the coordinates of a single link vertex.
 - The index of a General Purpose Valve's (GPV's) head loss curve was added to the list of editable Link Properties using the symbolic constant name `EN_GPV_CURVE`.
 - The `EN_getlinkvalue` and `EN_setlinkvalue` functions were updated to get and set the value of `EN_GPV_CURVE`.
 - Negative pressure values for `EN_SETTING` are now permitted in the `EN_setlinkvalue` function. 
 - The `EN_STARTTIME` parameter was added into the `EN_settimeparam` function.
 - A `EN_DEMANDPATTERN` parameter was added as the index of the default time pattern used by demands with no specific pattern assigned. It can be set or retrieved with the `EN_setoption` and `EN_getoption` functions, respectively, and is saved to file when the `EN_saveinpfile` function is called.
 - The `EN_getaveragepatternvalue` function will now accept a pattern index of zero which represents the constant pattern assigned to junction demands by default.
 - The adjustment of a tank's minimum volume (`Vmin`) when its parameters are changed using `EN_setnodevalue` or `EN_settankdata` has been corrected. 
 - A pump whose status is set to CLOSED in the input file now also has its speed setting set to zero which allows a simple pressure control activate the pump correctly.
 - A failure to raise an error condition for a non-positve pipe roughness in the input file has been fixed.
 - The calculation of head loss gradient for low flow conditions was corrected.
 - Improved updating and convergence tests were added to pressure dependent demand analysis.
 - Improved checks to prevent outflow from empty tanks or inflow to full (non-overflow) tanks, including the case where a link is connected to a pair of tanks, were added.
 - The CI regression test protocol was modified by:
   - changing the absolute tolerance used to compare the closeness of test results to benchmark values from 0 to 0.0001
   - dropping the "correct decimal digits" test 
   - dropping the check for identical status report content since it prevents accepting code changes that produce more accurate solutions in fewer iterations.
 - A possible loss of network connectivity when evaluating a Pressure Sustaining Valve was prevented.
 - Having the implied loss coefficient for an active Flow Control Valve be less than its fully opened value was prevented.
 - A new type of valve, a Positional Control Valve (PCV), was added that uses a valve characteristic curve to relate its loss coefficient to its fraction open setting. 
 - A new set of functions have been added to get information about upcoming time step events. Users will now see what type of event is going to cause the end of a time step to occur. See `ENtimetonextevent` and `EN_timetonextevent`.
 - A new set of functions have been added to allow users to set a reporting callback function. The user-supplied function will recieve all output normally directed to the report file.
 - A `EN_EMITBACKFLOW` option was added that either allows emitters to have reverse flow through them (the default) or not.
 - An incorrect tank elevation value set using `EN_settankdata` with SI units has been fixed.
 - The `EN_INITSETTING` option in function `EN_getlinkvalue` will now return `EN_MISSING` for a valve whose initial status is fixed to `EN_OPEN` or `EN_CLOSED`.
 - The functions `EN_getnodevalue` and `EN_getlinkvalue` now include the options `EN_NODE_INCONTROL` and `EN_LINK_INCONTROL` to determine whether a node or link appears in any simple or rule-based control.
 - An error is no longer raised when a minor loss coefficient of zero is assigned in `EN_setlinkvalue(ph, index, EN_MINORLOSS, 0)`.
 - The incorrect display of unconnected nodes has been fixed.
 - A header file for binding C# to the Toolkit has been added.
 - A new error code `263 - node is not a tank` is returned when `EN_settankdata` or `EN_setnodevalue` attempts to set a tank-only parameter for a non-tank node.
 - The function `EN_saveinpfile` was corrected for simple controls on GPV's by saving their status instead of the index of their head loss curve.
 - Support was added for Conan dependency manager.
 - The internal Qualflag variable is now adjusted when an EPANET input file has a QUALITY option not equal to NONE and simulation duration of zero.
 - Support was added for cubic meters per second (`EN_CMS`) flow units.
 - An EPANET input file with a simple timer control that has more than 9 input tokens no longer results in an incorrect hour setting.
 - Errors in node and link vertex coordinates are now ignored when reading an EPANET input file.
 - Only non-zero demands are now included in the `[DEMANDS]` section of the input file produced by `EN_saveinpfile`.
 - `EN_SET_CLOSED` and `EN_SET_OPEN` constants were added that can be used with `EN_setcontrol` to fix the status of pipes and valves to completely closed or completely open.
 - `EN_EMITTERFLOW` can now be used with `EN_getnodevalue` to retrieve a node's emitter flow rate.
 - `EN_STATUS_REPORT` can now be used with `EN_getoption` and `EN_setoption` to get or set the type of status report that EPANET will generate (`EN_NO_REPORT`, `EN_NORMAL_REPORT` or `EN_FULL_REPORT`).  
 - A possible parser error that could result in a Trace Node ID in an input file not being recognized was fixed.
 - Additional API functions for enabling/disabling controls and rules were added.
 