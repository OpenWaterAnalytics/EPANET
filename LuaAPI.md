# Lua scripting API

A network can carry a `[SCRIPT]` section holding Lua code that reads and changes the
model while it is being solved. Scripting is only available when EPANET is built with
`LUA_SCRIPTING` defined.

## The `[SCRIPT]` section

The section holds a Lua script, compiled once when the project is opened, right after the
input file has been read. There are two ways to write the control code that runs during
the simulation:

- Explicit handler, defining `on_hydraulic_step`. 

```
[SCRIPT]
local target = 30.0

function on_hydraulic_step()
    local off = node("J126").pressure - target
    if math.abs(off) > 0.01 then
        link("V1").setting = link("V1").setting - off
    end
end
```

- Through global scope evaluation. Leave `on_hydraulic_step` undefined and write the control code straight into the script.

The whole script is then re-evaluated on every solver pass:

```
[SCRIPT]
local target = 30.0

local off = node("J126").pressure - target
if math.abs(off) > 0.01 then
    link("V1").setting = link("V1").setting - off
end
```

Because the script re-runs, its top-level `local`s are rebuilt from scratch every pass.
Consider locals scratch variables, not global state. Anything that has to survive from one pass to the
next must be a global:

```
[SCRIPT]
passes = (passes or 0) + 1        -- global: counts up across the run
local level = node("2").head      -- local: recomputed every pass
```

### Not both

The two are alternatives, and which one is in effect is decided by whether
`on_hydraulic_step` exists. Defining it *and* putting control code at the top level does
not run both: the handler wins and the top-level code runs only once, at load.

The other three handlers are unaffected and can be used with either mode. Note that in
global scope mode they are re-created on every pass, so state they keep in a top-level
`local` is reset each time; use a global for that too.

### The load-time evaluation

Either way, the script is evaluated once when the project opens, which is what defines the
handlers. In global scope mode that means the control code also runs once at this point,
against a network that has not been solved yet: `pressure`, `flow` and the other computed
properties still hold their initial values, and `times().hydraulic_time` is 0.

Writes made during this evaluation are applied but never trigger a re-solve. Usually it is
harmless — a controller simply computes one result from the initial state and is corrected
on the first real pass — but a script that would divide by a solved value, or that logs,
should expect this extra run.

## Events

| Handler | When it runs |
| --- | --- |
| `on_open` | Once, when the hydraulic solver is initialised (`EN_initH`), before the first time step |
| `on_hydraulic_step` | After each time step converges, before its results are saved |
| `on_hydraulics_solved` | After each time step's results have been saved (`EN_nextH`), with those results still in place |
| `on_close` | Once, when the hydraulic solver is closed (`EN_closeH`) |

All four are optional; a handler that is not defined is skipped. Leaving
`on_hydraulic_step` out is what selects global scope mode, described above; the other three
are simply skipped when absent.

`on_hydraulic_step` — or, in global scope mode, the script that stands in for it — is the one
that can change the outcome of the step it runs in. If it changes the model, EPANET re-solves
the step and runs it again, repeating until it stops changing anything, up to a limit of 10
passes. This is what lets a script act as a controller — see the examples below.

If the limit is reached, the step is saved as it stands and the report file gets:

```
WARNING: Lua script still changing the network after 10 re-solves at hour 4.00
```

Two details matter when writing control code, in either mode:

- A write only counts as a change if it actually alters the stored value. Assigning a
  property the value it already holds does not trigger another pass, so a handler that
  re-applies the same setting settles instead of looping to the cap.
- A handler that changes something on every single pass will hit the 10-pass cap and the
  step will be saved as it stands. Converge on a value, or guard the write.

## Functions

### `node(id)`, `link(id)`

Return an object whose properties are the node's or link's values. Properties are always
numbers, in the project's own units, and match what `EN_getnodevalue` and
`EN_getlinkvalue` return. Enumerated properties (`status`, `valve_type`, `mix_model`, …)
use the same numeric codes as the toolkit's `EN_` constants.

```lua
local p = node("J126").pressure
link("V1").setting = p - 5
```

An unknown id raises `node not found: <id>`.

### `options()`, `times()`

The project-wide analysis options and time parameters. They take no id:

```lua
print(options().demand_multiplier)
times().report_step = 900
```

Options are read only. Time parameters can be written.
Their values are whole seconds, and values written to them are rounded, not truncated.

### `curve(id)`

Returns a curve's points as an array of `{x, y}` pairs:

```lua
for i, point in ipairs(curve("PU2")) do
    print(string.format("point %d: %g, %g", i, point[1], point[2]))
end
```

The array is a snapshot — changing it does not change the curve. An unknown id raises
`curve not found: <id>`.

### `print(...)`

Writes a line to the project's report file. Arguments are separated by tabs. This replaces
Lua's standard `print`, which would otherwise write to stdout.

## Errors

A script that does not **compile** is a fatal input error. `EN_open` fails with error 312,
the project is left closed, and the report file names the line Lua objected to:

```
Lua script error while parsing: [string "function on_hydraulic_step()..."]:5: ')' expected near 'elevation'
Error 312: failed to parse Lua script
```

Line numbers count from the first line of the `[SCRIPT]` section, blank lines included.

A **runtime** error is fatal as well. The toolkit call the script was running under fails
with error 313 and the run stops there, rather than carrying on with a script that raised:

```
Lua script error in on_hydraulic_step: [string "local target_pressure = 90.0..."]:30: link not found: NOPE
Error 313: failed to execute Lua script
```

Reading or writing a property that does not exist, or writing a read-only one, raises an
error naming it: `unknown node property: presure`,
`link property is read only: valve_type`.

## Property reference

### `node(id)`

| Property | | Property | | Property | |
| --- | --- | --- | --- | --- | --- |
| `elevation` | rw | `mix_model` | rw | `can_overflow` | rw |
| `base_demand` | rw | `tank_diameter` | rw | `demand` | r |
| `pattern` | rw | `min_volume` | rw | `head` | r |
| `emitter` | rw | `volume_curve` | rw | `pressure` | r |
| `init_quality` | rw | `min_level` | rw | `quality` | r |
| `source_quality` | rw | `max_level` | rw | `source_mass` | r |
| `source_pattern` | rw | `mix_fraction` | rw | `init_volume` | r |
| `source_type` | rw | `bulk_coeff` | rw | `mix_zone_volume` | r |
| `tank_level` | rw | `tank_volume` | r | `max_volume` | r |
| `demand_deficit` | r | `in_control` | r | `emitter_flow` | r |
| `leakage_flow` | r | `demand_flow` | r | `full_demand` | r |

`tank_level` is the tank's *initial* level and stays fixed for the whole run. For the level
a tank is currently at, use `head - elevation`, or `tank_volume` for the volume that
corresponds to it.

### `link(id)`

| Property | | Property | | Property | |
| --- | --- | --- | --- | --- | --- |
| `diameter` | rw | `status` | rw | `flow` | r |
| `length` | rw | `setting` | rw | `velocity` | r |
| `roughness` | rw | `pattern` | rw | `headloss` | r |
| `minor_loss` | rw | `pump_power` | rw | `energy` | r |
| `init_status` | rw | `pump_hcurve` | rw | `quality` | r |
| `init_setting` | rw | `pump_ecurve` | rw | `pump_state` | r |
| `bulk_coeff` | rw | `pump_ecost` | rw | `pump_efficiency` | r |
| `wall_coeff` | rw | `pump_epattern` | rw | `in_control` | r |
| `gpv_curve` | rw | `leak_area` | rw | `leakage` | r |
| `pcv_curve` | rw | `leak_expansion` | rw | `valve_type` | r |

### `options()`

Every option is read only. They configure the run, and a script only ever sees a run that is
already under way, so writing one raises `options property is read only: <name>`.

| Property | | Property | | Property | |
| --- | --- | --- | --- | --- | --- |
| `trials` | r | `global_efficiency` | r | `specific_diffusivity` | r |
| `accuracy` | r | `global_price` | r | `bulk_order` | r |
| `tolerance` | r | `global_pattern` | r | `wall_order` | r |
| `emitter_exponent` | r | `demand_charge` | r | `tank_order` | r |
| `demand_multiplier` | r | `specific_gravity` | r | `concentration_limit` | r |
| `head_error` | r | `specific_viscosity` | r | `demand_pattern` | r |
| `flow_change` | r | `unbalanced` | r | `emitter_backflow` | r |
| `check_frequency` | r | `max_check` | r | `pressure_units` | r |
| `damp_limit` | r | `status_report` | r | `headloss_form` | r |

### `times()`

| Property | | Property | | Property | |
| --- | --- | --- | --- | --- | --- |
| `duration` | rw | `report_start` | rw | `quality_time` | rw |
| `hydraulic_step` | rw | `rule_step` | rw | `periods` | r |
| `quality_step` | rw | `statistic` | rw | `halt_flag` | r |
| `pattern_step` | rw | `start_time` | rw | `next_event` | r |
| `pattern_start` | rw | `hydraulic_time` | rw | `next_event_tank` | r |
| `report_step` | rw | | | | |

## Examples

### Logging

```
[SCRIPT]
function on_hydraulics_solved()
    print(string.format("t=%d p=%.2f q=%.3f",
                        times().hydraulic_time,
                        node("J126").pressure,
                        link("P1").flow))
end
```

### A pump controlled by tank level

The live level of a tank is `head - elevation`. Do not use `tank_level` for this: that
property is the tank's *initial* level and does not change while the run proceeds.

Written as a handler:

```
[SCRIPT]
function on_hydraulic_step()
    local tank = node("2")
    local pump = link("9")
    local level = tank.head - tank.elevation

    if level > 140 then
        pump.status = 0
    elseif level < 110 then
        pump.status = 1
    end
end
```

The same control, written at global scope. The body is identical; all that changed is that
it is no longer wrapped in a function:

```
[SCRIPT]
local tank = node("2")
local pump = link("9")
local level = tank.head - tank.elevation

if level > 140 then
    pump.status = 0
elseif level < 110 then
    pump.status = 1
end
```

Both drive the pump identically. The handler form is the better fit once the script needs
state that outlives a pass — a pump start counter, a minimum-level record — because a
top-level `local` will hold it. The global scope form suits a control law like this one,
which is a pure function of the current state and keeps nothing.

The guards matter in either form: writing a status the pump already holds is not a change,
so the step settles once the level is inside the band. Without the band — say a bare
`pump.status = level > 140 and 0 or 1` recomputed from a level that the switching itself
moves — the step would flip back and forth until it hit the 10-pass cap.

### A PRV holding a downstream setpoint

Adjusting the valve setting by the pressure error converges in one pass per step, because
the demand is fixed while the step is being solved.

```
[SCRIPT]
local target = 30.0

function on_hydraulic_step()
    local off = node("J126").pressure - target

    if math.abs(off) > 0.01 then
        link("V1").setting = link("V1").setting - off
    end
end
```

### Setting up the run

```
[SCRIPT]
function on_open()
    times().report_step = 900
    print("demand multiplier: ", options().demand_multiplier)
    print("duration (s):      ", times().duration)
    print("hydraulic step (s):", times().hydraulic_step)
end
```
