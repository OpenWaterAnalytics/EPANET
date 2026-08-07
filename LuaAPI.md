# Lua scripting API

A network can carry a `[SCRIPT]` section holding Lua code that reads and changes the
model while it is being solved. Scripting is only available when EPANET is built with
`LUA_SCRIPTING` defined.

## The `[SCRIPT]` section

The section holds one Lua chunk, evaluated once when the project is opened, right after
the input file has been read. Its job is to define event handlers:

```
[SCRIPT]
function on_report()
    print("pressure at node 11: ", node("11").pressure)
end
```

Nothing has been solved when the chunk runs, so reading `pressure`, `flow` or any other
computed value at the top level returns whatever the network was initialised with. Put
that work in a handler.

Top-level code is still useful for constants and state the handlers share:

```
[SCRIPT]
local target = 30.0
local worst = 0.0

function on_report()
    local off = math.abs(node("J126").pressure - target)
    if off > worst then worst = off end
end

function on_close()
    print(string.format("worst deviation: %.3f", worst))
end
```

## Events

| Handler | When it runs |
| --- | --- |
| `on_open` | Once, when the hydraulic solver is initialised (`EN_initH`), before the first time step |
| `on_iteration` | After each time step converges, before its results are saved |
| `on_report` | After each time step is complete (`EN_runH`), with the step's final results in place |
| `on_close` | Once, when the hydraulic solver is closed (`EN_closeH`) |

All four are optional; a handler that is not defined is skipped.

`on_iteration` is the one that can change the outcome of the step it runs in. If a handler
changes the model, EPANET re-solves the step and calls `on_iteration` again, repeating
until the handler stops changing anything, up to a limit of 10 passes. This is what lets a
script act as a controller — see the PRV example below.

Two details matter when writing an `on_iteration` handler:

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
options().demand_multiplier = 1.2
print(times().duration)
```

`times()` values are whole seconds. Values written to them are rounded, not truncated.

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

Errors are written to the report file and never abort the simulation.

An error inside a handler cancels that one call, is reported as
`Lua script error in on_report: ...`, and the run continues with the next event. An error
in the chunk itself is reported as `Lua script error: ...` when the project opens; opening
still succeeds, but any handlers defined after the failing line will not exist, so the run
proceeds unscripted.

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

Every option is writable except `headloss_form`, which EPANET refuses to change while the
solver is open — which is always the case while a script runs.

| Property | | Property | | Property | |
| --- | --- | --- | --- | --- | --- |
| `trials` | rw | `global_efficiency` | rw | `specific_diffusivity` | rw |
| `accuracy` | rw | `global_price` | rw | `bulk_order` | rw |
| `tolerance` | rw | `global_pattern` | rw | `wall_order` | rw |
| `emitter_exponent` | rw | `demand_charge` | rw | `tank_order` | rw |
| `demand_multiplier` | rw | `specific_gravity` | rw | `concentration_limit` | rw |
| `head_error` | rw | `specific_viscosity` | rw | `demand_pattern` | rw |
| `flow_change` | rw | `unbalanced` | rw | `emitter_backflow` | rw |
| `check_frequency` | rw | `max_check` | rw | `pressure_units` | rw |
| `damp_limit` | rw | `status_report` | rw | `headloss_form` | r |

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
function on_report()
    print(string.format("t=%d p=%.2f q=%.3f",
                        times().hydraulic_time,
                        node("J126").pressure,
                        link("P1").flow))
end
```

### A pump controlled by tank level

```
[SCRIPT]
function on_iteration()
    local level = node("T1").tank_level
    local pump = link("PU1")

    if level > 5.2 then
        pump.status = 0
    elseif level < 3.5 then
        pump.status = 1
    end
end
```

The guards matter: writing a status the pump already holds is not a change, so the step
settles once the level is inside the band.

### A PRV holding a downstream setpoint

Adjusting the valve setting by the pressure error converges in one pass per step, because
the demand is fixed while the step is being solved.

```
[SCRIPT]
local target = 30.0

function on_iteration()
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
    options().demand_multiplier = 1.2
    print("duration (s):      ", times().duration)
    print("hydraulic step (s):", times().hydraulic_step)
end
```
