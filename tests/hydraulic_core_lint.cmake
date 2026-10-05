# Build-time lint for the hydraulic solver-unit boundary.
# Keep these checks structural/token-based so harmless source formatting does
# not break the test suite.

if(NOT DEFINED SOURCE_DIR)
    message(FATAL_ERROR "SOURCE_DIR is required")
endif()

file(READ "${SOURCE_DIR}/src/hydsolver.c" HYDSOLVER)
file(READ "${SOURCE_DIR}/src/hydcoeffs.c" HYDCOEFFS)
file(READ "${SOURCE_DIR}/src/hydscale.c" HYDSCALE)
file(READ "${SOURCE_DIR}/src/hydstatus.c" HYDSTATUS)
file(READ "${SOURCE_DIR}/src/hydraul.c" HYDRAUL)
file(READ "${SOURCE_DIR}/src/leakage.c" LEAKAGE)

function(require_token variable token description)
    string(FIND "${${variable}}" "${token}" position)
    if(position EQUAL -1)
        message(FATAL_ERROR "${description}: missing '${token}'")
    endif()
endfunction()

function(forbid_token variable token description)
    string(FIND "${${variable}}" "${token}" position)
    if(NOT position EQUAL -1)
        message(FATAL_ERROR "${description}: found forbidden '${token}'")
    endif()
endfunction()

# Numerical core state must come from SolverState, not dimensional Hydraul data.
foreach(variable HYDSOLVER HYDCOEFFS HYDSTATUS)
    foreach(token
        "hyd->NodeHead"
        "hyd->NodeDemand"
        "hyd->FullDemand"
        "hyd->DemandFlow"
        "hyd->EmitterFlow"
        "hyd->LeakageFlow"
        "hyd->LinkFlow")
        forbid_token(${variable} "${token}" "${variable} dimensional-state access")
    endforeach()
endforeach()

# Fixed-unit constants stay on the model/compilation side of the boundary.
foreach(token "4.727" "32.2" "1.49" "0.02517" "8.814")
    forbid_token(HYDSOLVER "${token}" "hydsolver.c fixed-unit constant")
    forbid_token(HYDSTATUS "${token}" "hydstatus.c fixed-unit constant")
endforeach()
require_token(HYDCOEFFS "4.727" "Hazen-Williams dimensional coefficient")
require_token(HYDCOEFFS "32.2" "Darcy-Weisbach dimensional coefficient")
require_token(HYDCOEFFS "1.49" "Darcy-Weisbach dimensional coefficient")
forbid_token(HYDCOEFFS "0.02517" "dynamic valve loss must compile in hydscale.c")
require_token(HYDSCALE "0.02517" "TCV setting compilation")

# Global/node/static-link inputs are precompiled before entering GGA hot paths.
require_token(HYDSOLVER "hyd->SolverModel.NodeElevation" "compiled node elevation")
forbid_token(HYDSOLVER "hydheadtosolver(pr, hyd->Htol)" "Htol hot-path conversion")
forbid_token(HYDSOLVER "hydheadtosolver(pr, hyd->Pmin)" "Pmin hot-path conversion")
forbid_token(HYDSOLVER "hydflowtosolver(pr, hyd->FlowChangeLimit)" "flow-limit hot-path conversion")
require_token(HYDCOEFFS "hyd->SolverModel.LinkMinorLoss[k]" "compiled minor loss")
require_token(HYDCOEFFS "hyd->SolverModel.LinkResistance[k]" "compiled resistance")
require_token(HYDCOEFFS "hyd->SolverModel.LinkViscosityFlow[k]" "compiled viscosity flow")

# Dynamic settings and curves cross the dimensional boundary at mutation/model
# compilation points, not in nonlinear/status iteration code.
require_token(HYDSCALE "compilehydraulicsolversetting" "dynamic-setting compiler")
require_token(HYDRAUL "compilehydraulicsolversetting(pr, index)" "link mutation compiler")
require_token(HYDRAUL "compilehydraulicsolversetting(pr, k)" "control mutation compiler")
require_token(HYDSOLVER "compilehydraulicsolversetting(pr, k)" "solver control mutation compiler")
require_token(HYDCOEFFS "hyd->SolverModel.LinkSetting[k]" "compiled dynamic setting")
require_token(HYDCOEFFS "hyd->SolverModel.LinkDynamicLoss[k]" "compiled dynamic loss")
forbid_token(HYDCOEFFS "hydflowtosolver" "hydcoeffs.c dimensional conversion")
forbid_token(HYDCOEFFS "hydheadtosolver" "hydcoeffs.c dimensional conversion")
forbid_token(HYDCOEFFS "hydresistancetosolver" "hydcoeffs.c dimensional conversion")
forbid_token(HYDCOEFFS "hydconductancetosolver" "hydcoeffs.c dimensional conversion")
forbid_token(HYDCOEFFS "hydminorlosstosolver" "hydcoeffs.c dimensional conversion")
forbid_token(HYDCOEFFS "hydflowfromsolver" "hydcoeffs.c dimensional conversion")
forbid_token(HYDCOEFFS "hydheadfromsolver" "hydcoeffs.c dimensional conversion")
require_token(HYDCOEFFS "solvercurvecoeff" "solver-native curve interpolation")
require_token(HYDSCALE "compiled->X[j] = hydflowtosolver" "compiled curve breakpoints")
require_token(HYDSCALE "compiled->H0[j] = hydheadtosolver" "compiled curve intercepts")
require_token(HYDSCALE "compiled->R[j] = hydresistancetosolver" "compiled curve slopes")

# Status/solver code can convert only when publishing dimensional state or
# handing tank simulation state back to the compatibility side.
forbid_token(HYDSTATUS "hydheadtosolver" "hydstatus.c dimensional conversion")
forbid_token(HYDSTATUS "hydflowtosolver" "hydstatus.c dimensional conversion")
forbid_token(HYDSTATUS "hydresistancetosolver" "hydstatus.c dimensional conversion")
require_token(HYDSTATUS "hydflowfromsolver" "tank-status dimensional bridge")
require_token(HYDSTATUS "SolverModel.LinkPumpMaxHead" "compiled pump max head")
forbid_token(HYDSOLVER "hydheadtosolver" "hydsolver.c dimensional conversion")
forbid_token(HYDSOLVER "hydflowtosolver" "hydsolver.c dimensional conversion")
require_token(HYDSOLVER "SolverModel.ControlGrade" "compiled control grade")
require_token(HYDSOLVER "SolverModel.RelativeErrorFlowCutoff" "compiled low-flow cutoff")
require_token(HYDSCALE "NodeEmitterResistance" "compiled emitter resistance")
require_token(HYDSCALE "NodePdaMinGrade" "compiled PDA grade")
require_token(HYDSCALE "LinkPumpH0" "compiled pump intercept")
require_token(HYDSCALE "LinkPumpResistance" "compiled pump resistance")
require_token(HYDSCALE "LinkPumpMaxHead" "compiled pump maximum head")
require_token(HYDSCALE "ControlGrade" "compiled control grade")
require_token(HYDSCALE "BarrierGradient" "compiled barrier gradient")
require_token(HYDSCALE "RelativeErrorFlowCutoff" "compiled low-flow cutoff")
forbid_token(LEAKAGE "hydheadtosolver" "leakage.c hot-path head conversion")
forbid_token(LEAKAGE "hydflowfromsolver" "leakage.c hot-path flow conversion")
require_token(LEAKAGE "hydresistancetosolver" "leakage coefficient compilation")
require_token(LEAKAGE "hydflowtosolver" "leakage coefficient compilation")
