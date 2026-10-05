# Internal hydraulic unit assumptions

This document inventories the fixed-unit assumptions in EPANET's hydraulic
implementation before the unit-independent hydraulics refactor.

The inventory is based on the `dev` source at commit `8bca02d` (2026-10-01).
The characterization and cross-unit equivalence tests added on
`feature/unit-independent-hydraulics` do not change production code, so this
inventory also describes the hydraulic source at the start of that branch.

## Scope and target

EPANET currently accepts several user-facing unit systems but converts most
hydraulic model data to a fixed internal basis before solving. In practice the
hydraulic engine assumes:

| Quantity | Current internal basis |
| --- | --- |
| head, elevation, length | ft |
| pipe diameter | ft |
| flow and demand | ft^3/s (cfs) |
| tank volume | ft^3 |
| velocity | ft/s |
| kinematic viscosity | ft^2/s |
| pressure settings used by the solver | ft of head |
| pump head/flow curves after validation | ft / cfs |

The goal of this project is **not** to remove unit handling from the INP,
Toolkit, reporting, energy, or water-quality interfaces. Those layers describe
physical quantities and must remain unit-aware.

The target is to introduce a clear boundary at which a dimensional hydraulic
problem is compiled into solver coordinates. The GGA and its component
equations operate on those scaled head/flow values instead of repeatedly
crossing back into EPANET's dimensional state. For compatibility, several
regularizers and convergence thresholds are still defined by their historical
ft/cfs values and are transformed once at this boundary. Results are converted
back to the physical EPANET representation before they are consumed by the
public API, reporting, energy, and water-quality code.

## Current conversion boundary

`src/input1.c` establishes the existing fixed internal basis:

- `initunits()` builds `Project::Ucf[]` as conversion factors from internal
  values to the selected user-facing units. For US flow units the head factor is
  `1.0` and the flow factor is based on cfs; for SI flow units the factors are
  conversions from ft/cfs to the requested units.
- `convertunits()` divides elevations, demands, pressure limits, tank geometry,
  pipe dimensions, valve settings, controls, and other input values by those
  factors. This leaves the in-memory hydraulic model in the fixed ft/cfs basis.
- `src/epanet.c`, `src/inpfile.c`, `src/output.c`, `src/report.c`, and
  `src/rules.c` use `Ucf[]` to translate between the internal representation and
  public/file/report values.

This boundary should remain compatible throughout the refactor. Initially, a
new solver-scaling boundary can be inserted *after* the existing conversion to
internal physical values. Moving or simplifying `Ucf[]` is a later cleanup, not
an initial requirement.

## Fixed-unit assumptions to migrate

### 1. Defaults and physical constants

**Files:** `src/input1.c`, `src/types.h`

- `HTOL = 0.0005` is documented and used as a default hydraulic head tolerance
  in ft.
- `QTOL = 0.0001` is documented and used as a default hydraulic flow tolerance
  in cfs.
- `VISCOS = 1.1e-5` is kinematic viscosity in ft^2/s. It participates directly
  in Darcy-Weisbach calculations.
- `DIFFUS = 1.3e-8` is diffusivity in ft^2/s. This belongs to water quality and
  should stay outside the hydraulic solver migration, but it confirms that the
  wider EPANET model remains dimensional.
- `MINPDIFF = 0.1` is defined in input pressure units before `convertunits()`
  converts `Pmin`/`Preq` to internal pressure head.
- `types.h` contains the ft/cfs-centered conversion constants (`GPMperCFS`,
  `LPSperCFS`, `MperFT`, `PSIperFT`, etc.). These are legitimate boundary
  constants and are not themselves candidates for deletion.

**Migration:** preserve public defaults exactly, then transform dimensional
head/flow tolerances into solver units at the solver boundary. Darcy-Weisbach
must receive consistently scaled viscosity or a dimensionless Reynolds-number
formulation.

### 2. Hydraulic state initialization

**Files:** `src/hydraul.c`, `src/validate.c`

- `QZERO = 1.e-6` is explicitly an equivalent-to-zero flow in cfs.
- `initlinkflow()` initializes non-pump open links to area times `1 ft/s`, so the
  resulting initial flow is in cfs.
- `constpowerpump()` initializes a constant-power pump with `Q0 = 1.0`, i.e.
  `1 cfs`.
- `inithyd()` initializes emitter and leakage trial flows with ordinary doubles
  whose magnitudes are meaningful only in the current flow basis.

**Migration:** calculate physical initial conditions first and scale them once
when creating/resetting solver state. Initial guesses should not encode a
particular flow unit.

### 3. Pipe resistance and minor-loss equations

**Files:** `src/input1.c`, `src/hydcoeffs.c`

`resistcoeff()` contains coefficients derived for the current ft/cfs basis:

- Hazen-Williams: `4.727`.
- Darcy-Weisbach: gravity appears as `32.2 ft/s^2` in the resistance expression.
- Chezy-Manning: `1.49` is the US customary Manning coefficient.

`convertunits()` transforms a dimensionless minor-loss coefficient into a
`Q^2` head-loss coefficient using `0.02517 / D^4`. The same `0.02517` basis is
used when `tcvcoeff()` converts a TCV setting into a loss coefficient.

`pipecoeff()` and `DWpipecoeff()` then evaluate head loss and derivatives using
`LinkFlow`, `R`, and `Km` in the fixed internal units. `P` and `Y` therefore
inherit units based on ft and cfs.

**Migration:** compile each pipe's physical law into coefficients expressed in
the solver representation. The GGA should consume only the compiled coefficient
and scaled flow. Unit-specific empirical constants must be confined to the
physical-law compilation boundary.

### 4. Darcy-Weisbach Reynolds-number path

**Files:** `src/hydcoeffs.c`, `src/types.h`, `src/input1.c`

- `Link::Kc` is converted from millifeet or mm to ft for Darcy-Weisbach.
- `Hydraul::Viscos` is stored in ft^2/s.
- `DWpipecoeff()` builds relative roughness and the viscosity/diameter term from
  these fixed-basis quantities before calling `frictionFactor()`.

The friction factor itself is dimensionless. `DWpipecoeff()` now scales the
viscosity-diameter term with the solver flow scale before evaluating Reynolds
number, while relative roughness remains dimensionless. `frictionFactor()`
therefore receives only solver-scaled flow-like input plus dimensionless
roughness, and its derivative is returned directly with respect to solver flow.

**Migration:** complete. Reynolds number and relative roughness no longer require
converting solver flow back to EPANET's legacy cfs representation.

### 5. Pump curves and constant-power pumps

**Files:** `src/validate.c`, `src/hydcoeffs.c`, `src/hydraul.c`

- Pump curve validation converts curve flow/head values to cfs/ft and stores
  `Q0`, `Qmax`, `H0`, `Hmax`, and `R` in that basis.
- `curvecoeff()` temporarily converts current cfs flow back to the original
  curve flow units via `Ucf[FLOW]`, interpolates the untransformed user curve,
  then converts the resulting head/slope back to ft/cfs.
- `constpowerpump()` uses the unit-dependent `8.814` conversion when creating
  its pump resistance and initializes at `1 cfs`.
- `getenergy()` uses ft and cfs head/flow and the corresponding `8.814` /
  horsepower conversion to compute kW.

The pump coefficient path compiles dimensional pump properties to solver
head/flow units at the GGA boundary. Custom pump and GPV curves are compiled
into solver-space flow breakpoints plus piecewise head intercept/slope
coefficients, so interpolation no longer round-trips through dimensional or
user units during an iteration. Whole-model compilation allocates these
solver-space segments only for pump-head and GPV headloss curves; unrelated
volume, efficiency, valve-position, and generic curves remain on the dimensional
side of the boundary. Constant-power and power-function pump
coefficients use the same generalized resistance scaling as other relations of
the form `H = R * Q^n`. Pump maximum-head status checks use the compiled
solver-head limit.

**Migration:** hydraulic pump equations and head/flow curves are complete. The
legacy `8.814` factor remains only where constant pump power is converted into
the dimensional physical-model coefficient and in dimensional energy reporting;
neither use is part of the GGA.

### 6. Valves and valve status logic

**Files:** `src/input1.c`, `src/hydcoeffs.c`, `src/hydstatus.c`

- FCV settings are converted to cfs; PRV/PSV/PBV settings are converted to ft of
  pressure head.
- GPV curves are stored in original user units and converted in `curvecoeff()`.
- TCV loss conversion uses the ft/cfs-specific `0.02517` factor.
- PRV, PSV, FCV, CV, pump, and tank-link status decisions directly compare
  `NodeHead`, `LinkFlow`, valve settings, `Htol`, and `Qtol` in the current
  fixed basis.

**Migration:** complete. Dynamic valve settings are compiled when they change.
PRV/PSV target grades, PBV head settings, FCV flow settings, TCV/PCV losses, GPV
curves, minor-loss coefficients, and status tolerances are consumed in solver
coordinates. Status transitions remain covered by characterization and
scale-invariance tests.

### 7. Pressure-dependent demand and emitters

**Files:** `src/input1.c`, `src/hydcoeffs.c`, `src/hydsolver.c`

- `Pmin` and `Preq` are converted from selected pressure units to ft of head.
- Emitter coefficients are transformed in `convertunits()` into an inverted
  head-loss coefficient compatible with cfs and ft.
- `demandcoeffs()` / `demandheadloss()` form pressure-dependent demand head loss
  and gradients in ft and ft/cfs.
- `pdaconverged()` contains a separate hard-coded `QTOL = 0.0001 cfs`.
- `hydsolver.c` documents PDA intermediate values (`dp`, `dq`, `hloss`,
  `hgrad`, `dh`) explicitly as ft/cfs quantities.

**Migration:** complete. Emitter coefficients, PDA pressure ranges, node
elevations, the legacy PDA convergence tolerance, and smooth barrier parameters
are compiled into solver units at the GGA boundary. Emitter/PDA barrier
evaluation itself is solver-native; no dimensional conversion occurs in the
coefficient-assembly hot path.

### 8. Leakage model

**File:** `src/leakage.c`

Leakage has several especially important hidden assumptions:

- the orifice coefficient is constructed from SI-area input, gravity, and
  conversion to the current internal basis;
- pipe length is normalized by `100 ft` (`link->Len / 100.0`), reflecting the
  public leakage parameter definition;
- pressure head and leakage flows used by the GGA are ft and cfs;
- initial fixed/variable-area leakage trial flows use `0.001` in the current
  flow basis;
- `leakagehasconverged()` contains another hard-coded `0.0001 cfs` tolerance;
- leakage head-loss gradients are explicitly ft/cfs.

The public leakage definition may legitimately continue to refer to a leak area
per 100 units of pipe length according to EPANET's API/file contract. The
solver-side coefficient must nevertheless be compiled into solver-coordinate
form.

**Migration:** complete. Public leakage parameter interpretation remains
dimensional, while leakage equation coefficients, initial trial flows,
convergence tolerance, and barrier parameters are compiled into solver units.
The iterative leakage relation is solver-native.

### 9. Controls, rules, and tank state

**Files:** `src/input1.c`, `src/hydraul.c`, `src/hydsolver.c`, `src/rules.c`

- controls are converted to internal tank grades, pressure heads, flow settings,
  or valve settings during `convertunits()`;
- `hydsolver.c` compares control grades to `NodeHead` using `Htol`;
- `hydraul.c` uses dimensional tank heads, volumes, flows, and time to advance
  tank state and determine event times;
- tank volume curves are deliberately stored in original user units and
  `tankvolume()` / `tankgrade()` cross the `Ucf[]` boundary to interpolate them;
- rule evaluation converts internal physical values back to user-facing values
  before comparisons/actions.

**Migration:** do not dimensionless-ize the event scheduler or rule language as
part of this project. Keep simulation state dimensional. `hydsolve()` loads tank
head/net inflow into `SolverState` through the hydraulic scaling boundary and
publishes them back before timestep, control, and rule evaluation. Tank volume
logic consumes only dimensional state. Controls checked *inside* hydraulic
iterations use scaled comparison values.

### 10. Convergence and numerical scale assumptions

**Files:** `src/input1.c`, `src/hydcoeffs.c`, `src/hydsolver.c`,
`src/hydstatus.c`, `src/leakage.c`, `src/types.h`

The following are not all physical unit constants, but their numerical meaning
depends on the present scale of head/flow and therefore must be audited during
normalization:

- `Htol`, `Qtol`, `FlowChangeLimit`, and `HeadErrorLimit`;
- `RQtol = 1e-7`, used as a minimum hydraulic gradient/resistance;
- `QZERO = 1e-6 cfs`;
- `CSMALL = 1e-6` and `CBIG = 1e8` for open/closed-link surrogate resistance;
- barrier functions in `hydcoeffs.c` and `leakage.c` using `1e9` and `1e-6`;
- global `TINY = 1e-6` when it is applied to hydraulic flow/curve quantities;
- the separate `0.0001 cfs` PDA and leakage convergence tests.

**Migration:** preserve the legacy dimensional meaning of hydraulic
regularizers, then map them into solver head/flow, head/flow-gradient, or
flow/head-conductance units according to how each one is used. Dimensionless
uses (for example `TINY` when comparing a pump exponent to 1.0) remain
unscaled. The legacy relative-flow convergence metric has a special low-flow
branch where the numeric `Hacc` value also acts as an internal-flow cutoff;
that cutoff is compiled into solver flow units and the fallback correction is
mapped back to dimensional flow before comparison, so changing `FlowScale`
cannot change the stopping decision. Raw `CSMALL`/`CBIG` values are still valid
while constructing dimensional model-side resistance data; numerical GGA uses
must be scaled.

### 11. Solver matrix quantities

**Files:** `src/hydcoeffs.c`, `src/hydsolver.c`, `src/types.h`

The sparse linear algebra is structurally unit-agnostic, but the assembled
values are not currently dimensionless:

- `NodeHead` is ft;
- `LinkFlow`, `Xflow`, and demand/emitter/leakage flows are cfs;
- `P` is inverse head-loss gradient (cfs/ft for ordinary link equations);
- `Y` is a flow correction term (cfs);
- matrix right-hand sides and diagonal/off-diagonal coefficients inherit these
  scales.

`smatrix.c` itself does not need to know physical units. The migration should
therefore happen before matrix assembly, allowing the sparse solver to remain a
plain numerical component.

### 12. Reporting and public diagnostics

**Files:** `src/hydsolver.c`, `src/epanet.c`, `src/output.c`, `src/report.c`

- `reporthydbal()` converts maximum flow/head errors from the current internal
  values through `Ucf[FLOW]` and `Ucf[HEAD]`.
- Toolkit getters/setters similarly assume hydraulic state is stored in the
  legacy physical basis and translate at the API boundary.

**Migration:** after the numerical solve is normalized, convert diagnostics and
results back to EPANET's dimensional physical state before existing public
conversion/report code sees them. Public values and file formats must not
change.

## Explicit non-goals for the hydraulic-core refactor

The following remain dimensional and unit-aware unless separately redesigned:

- INP parsing and writing;
- Toolkit input/output units;
- report and binary-output units;
- rule-language quantities;
- simulation clock and event scheduling;
- water-quality transport/reaction calculations;
- energy and cost reporting.

Water quality is especially important: `quality.c` and `qualreact.c` currently
consume physical hydraulic flow, pipe size, velocity, and volume. The hydraulic
solver must therefore publish dimensional results before quality advances.


## Final hydraulic solver-unit contract

The hydraulic refactor leaves EPANET's model and public interfaces dimensional
while giving the Global Gradient Algorithm an independent numerical head/flow
representation. The final contract is:

1. `Hydraul.NodeHead`, `Hydraul.LinkFlow`, demands, settings, tolerances, tank
   state, and physical model coefficients remain in EPANET's dimensional
   internal basis for compatibility with the Toolkit, reporting, energy, water
   quality, controls/rules, and event scheduling.
2. `ShydScale` defines `H_internal = H_solver * Head` and
   `Q_internal = Q_solver * Flow`. The scale is numerical conditioning state,
   not a public-unit conversion.
3. `Hydraul.SolverModel` is the compiled numerical model. Static coefficients,
   node grades, regularization constants, curve segments, and dynamic link
   settings are transformed at model/mutation boundaries and consumed directly
   by the GGA.
4. `Hydraul.SolverState` is the exclusive head/flow/demand state used while the
   GGA is iterating. `loadhydraulicsolverstate()` imports only state the GGA
   consumes; `savehydraulicsolverstate()` publishes the solved dimensional
   state before non-solver subsystems run.
5. `hydcoeffs.c` may construct dimensional physical-law coefficients using
   legacy-basis constants such as the HW/DW/CM factors, but matrix/nonlinear
   coefficient assembly must consume their compiled `SolverModel` forms. TCV
   setting conversion belongs to the solver-model compilation boundary, not to
   iterative coefficient assembly.
6. Remaining `hyd*tosolver()` / `hyd*fromsolver()` calls are explicit boundary
   work: model compilation, leakage initialization, tank-state crossings, and
   publication of dimensional convergence diagnostics. They are not part of the
   GGA coefficient/matrix hot path.
7. The sparse matrix solver is numerical only. Its assembled quantities inherit
   solver head/flow scales, not EPANET's ft/cfs basis.

The contract is guarded from both directions. `test_hydraulic_solver_scaling`
checks production and forced solver scales, nonlinear components, live Toolkit
updates, pump/GPV curves, pressure valves, energy, and quality consumers.
`test_hydraulic_core_unit_contract` statically rejects dimensional state access,
fixed-unit constants, and conversion-helper calls in the GGA hot paths.
`test_hydraulic_performance_guardrails` freezes deterministic hydraulic
signatures and iteration budgets for Net1, Net2, Net3, and Grid20. All are part
of `test_toolkit`.

### Production scaling policy

The production solver no longer uses the legacy `Head = 1`, `Flow = 1` mapping.
When hydraulics are opened, `inithydraulicscaling()` derives characteristic
magnitudes from the validated dimensional network and rounds them down to powers
of ten. This keeps typical GGA unknowns near order one while making the selected
scale stable against small parsing/conversion roundoff.

The characteristic flow magnitude is the largest of the total absolute base
junction demand, finite pump operating flows, and FCV settings. The characteristic
head magnitude is the largest relevant absolute grade/head from nodes, tanks, PDA
pressure limits, finite pump heads, pressure-control valves, and simple controls.
For constant-power pumps, which have no finite maximum head, the pump head at the
characteristic network flow is included instead. Empty or degenerate models fall
back to `1.0` until a meaningful dimensional magnitude exists.

These scale values are a numerical preconditioner only. They do not depend on the
selected public flow-unit system and do not change model storage, Toolkit units,
reports, energy, quality, tank integration, or event timing. The scale is fixed
for a hydraulic-open session and is recomputed the next time hydraulics are opened,
so model edits made before `EN_openH()` are reflected in the numerical scaling.

### Compatibility and cost review

The production policy is regression-tested against the former `Head = 1`,
`Flow = 1` path on Net1, Net2, and Net3 over their full hydraulic event
sequences. Event times and link statuses must match exactly; dimensional node
heads, demands, and link flows must remain within tight floating-point
tolerances. Separate cross-unit tests verify equivalent physical results for all
supported public flow-unit systems.

`SolverState` adds six `double` arrays per node (head, total node demand, full
demand, delivered demand, emitter flow, and leakage flow) plus one `double`
array per link for flow. `SolverModel` adds compiled node/link arrays and
solver-space curve data. The boundary load intentionally omits incoming
`NodeDemand` and fixed-grade-node junction-only flow components because the GGA
does not consume them; publication still restores the dimensional state needed
by downstream EPANET subsystems.

The opt-in `benchmark_hydraulics` target characterizes the remaining runtime
cost without making wall-clock timing a CI gate. Profiling after the migration
shows the dominant runtime remains sparse linear solution and GGA coefficient
assembly. The explicit state load/save and model compilation are measurable but
small boundary costs; avoiding them entirely would require a persistent
solver-native simulation state and a much broader dirty-state protocol, which is
intentionally outside this compatibility-focused refactor.

## Migration checklist

The implementation is complete only when all items below are satisfied.

- [x] Introduce an explicit hydraulic solver scaling/context object.
- [x] Separate dimensional physical hydraulic state from numerical solver state.
- [x] Scale/reset all initial head and flow guesses.
- [x] Compile HW, DW, CM, and minor-loss pipe equations into solver units.
- [x] Make Reynolds-number/relative-roughness evaluation unit-independent.
- [x] Compile pump equations and pump curves into solver units.
- [x] Compile valve equations/settings/curves into solver units.
- [x] Compile emitter and PDA equations into solver units.
- [x] Compile leakage equations into solver units without changing public leak
      parameter semantics.
- [x] Scale status/control comparisons that occur inside hydraulic iterations.
- [x] Replace dimensional/hard-coded convergence thresholds inside the solver.
- [x] Classify and normalize numerical regularization constants (`RQtol`,
      `CSMALL`, `CBIG`, barriers, hydraulic uses of `TINY`).
- [x] Keep tank/event simulation state dimensional and define its solver boundary.
- [x] Keep energy calculations dimensional and define their solver boundary.
- [x] Keep water-quality calculations dimensional and define their solver boundary.
- [x] Return hydraulic results/diagnostics to the legacy physical representation
      before Toolkit/report/output consumers use them.
- [x] Pass hydraulic characterization tests unchanged.
- [x] Pass cross-unit equivalence tests for all supported flow-unit systems.
- [x] Add stress tests for very small/large hydraulic scales.
- [x] Audit `hydsolver.c`, `hydcoeffs.c`, and `hydstatus.c` for remaining fixed
      ft/cfs assumptions.
- [x] Document the final solver-unit contract and add a CI guard against
      reintroducing fixed-unit assumptions into the numerical core.
- [x] Enable model-derived production solver scaling instead of the legacy
      `Head = 1`, `Flow = 1` mapping.
- [x] Add deterministic performance guardrails plus an opt-in wall-clock
      benchmark and profile the final solver boundary costs against `dev`.

## Suggested migration order

To keep every commit reviewable and bisectable, migrate in this order:

1. scaling/context abstraction with legacy-equivalent scaling;
2. physical vs. numerical solver state;
3. initialization;
4. pipes and Darcy-Weisbach inputs;
5. pumps and curves;
6. valves;
7. emitters, PDA, and leakage;
8. convergence and numerical regularization;
9. in-iteration controls/status checks;
10. tank/energy/quality boundaries;
11. enable non-trivial unit-invariant scaling;
12. remove obsolete fixed-unit assumptions from the hydraulic core.

At each stage, the repository should build and all existing tests plus the new
characterization/equivalence tests should pass.
