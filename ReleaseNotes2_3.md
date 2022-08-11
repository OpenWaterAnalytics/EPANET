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
 - The `EN_getaveragepatternvalue` function will now accept a pattern index of 0 which represents the constant pattern assigned to junction demands by default.
 - The adjustment of a tank's minimum volume (`Vmin`) when its parameters are changed using `EN_setnodevalue` or `EN_settankdata` has been corrected. 
 - A pump whose status is set to CLOSED in the input file now also has its speed setting set to 0 which fixes having a simple pressure control activate the pump correctly.
 - A failure to raise an error condition for a pipe roughness <= 0 in the input file has been fixed.
 - The calculation of head loss gradient for low flow conditions was corrected.
 - Improved updating and convergence tests were added to pressure dependent demand analysis.
 - Improved checks to prevent outflow from empty tanks or inflow to full (non-overflow) tanks. *(Still needs more work).*
 - The CI regression test protocol was modified by:
   - changing the absolute tolerance used to compare the closeness of test results to benchmark values from 0 to 0.0001
   - dropping the "correct decimal digits" test 
   - dropping the check for identical status report content since it prevents accepting code changes that produce more accurate solutions in fewer iterations.
 - A possible loss of network connectivity when evaluating a Pressure Sustaining Valve was prevented.
 - Having the implied loss coefficient for an active Flow Control Valve be less than its fully opened value was prevented.
 
 

