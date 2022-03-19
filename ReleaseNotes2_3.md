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
 - The calculation of head loss gradient for low flow conditions was corrected.
 - Improved updating and convergence tests were added to pressure dependent demand analysis.
 - <more>
 

