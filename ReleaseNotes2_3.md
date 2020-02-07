>
## Release Notes for EPANET 2.3

This document describes the changes and updates that have been made in version 2.3 of EPANET.

 - The check for at least two nodes, one tank/reservoir and no unconnected junction nodes was moved from `EN_open` to `EN_openH` and `EN_openQ` so that partial network data files can be opened by the toolkit.
 - A `EN_setcurvetype` function was added to allow API clients to set a curve's type (e.g., `EN_PUMP_CURVE,` `EN_VOLUME_CURVE,` etc.).
 - A `EN_setvertex` function was added to allow API clients to change the coordinates of a link's vertex.
 - The index of a General Purpose Valve's (GPV's) head loss curve was added to the list of editable Link Properties using the symbolic constant name `EN_GPV_CURVE`.
 - The `EN_getlinkvalue` and `EN_setlinkvalue` functions were updated to get and set the value of `EN_GPV_CURVE`.
 - For `EN_CUSTOM` type pump curves the maximum head value is now extrapolated to the y-axis intercept instead of being based on the first curve data point. Similarly, the maximum flow value is extrapolated to the x-axis intercept. 
 - Status checking for a pump not able to deliver enough head has been replaced by adding a penalty term to the pump's operating curve that prevents it from having negative flow (i.e., from crossing the y-axis).
 - Status checking for Flow Control Valves has been eliminated by using a continuous head v. flow function. If the current flow is below the valve setting then the normal open head loss relation is used; otherwise a linear penalty function is applied to any flow in excess of the setting. Warnings are no longer issued when the valve operates fully opened at flows below the setting.
 - The maximum link head loss error convergence criterion is now evaluated using the most recently computed link flows instead of flows from the previous iteration.
 


