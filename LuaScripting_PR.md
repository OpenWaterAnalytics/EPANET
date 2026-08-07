# Lua scripting support

## Embedding the Lua engine

The original proof of concept downloaded the Lua engine directly and compiled it from source into a static library.

I have instead added the Lua engine to the repository as a single-header, embedded source file. This way, Lua support in EPANET stays stable even if the file hosting the Lua engine stops being accessible.

The amalgamation I have used is https://github.com/edubart/minilua.

Two new error codes have been added to `errors.dat`:
- `310` failed to initialize Lua scripting engine
- `311` Lua scripting engine not initialized

The Lua engine specifics are hidden inside the `src/lua` directory. An opaque type exposes the engine to the rest of the toolkit, so no implementation details of the Lua scripting backend leak out.

All Lua-related code is only compiled and linked if the compile flag macro `LUA_SCRIPTING` is defined. If it is not, all associated code is stripped, including from the data structures.

I’ve tried to make the minimum possible changes to attach the scripting engine to the EPANET simulation engine, and they are localized to a few call sites. Most of the complexity inside the `src/lua` directory is the interfacing between the Lua scripting engine and the `EN_xxx()` EPANET calls.

## Script lifecycle

In the PoC, the main body of the script was evaluated twice over: once at startup, and then once per hydraulic time step, after the iteration event had finished.

I have changed it so the script is evaluated once on startup, and every change to the model then has to be triggered through events.

With the other scheme the lifecycle was a bit hard to understand: some code in the main body of the script would run once per time step, and other code would run only on events. Now, all changes to the model have to be applied through event callbacks.

Also, instead of one global event handler for everything, there is a callback per event (`on_open`, `on_close`, `on_report`, `on_iteration`). I think this is more aligned with how other scripting engines work.

## Integration points in the engine

| Site | What it does |
| --- | --- |
| `input2.c` `newline()` | Forwards all `[SCRIPT]` lines to `luascript_addScriptLine()`. Blank lines are forwarded too, so line numbers in Lua error messages match the INP |
| `project.c` `openproject()` | `luascript_open()` creates the state and registers the API. `luascript_parseScript()` evaluates the chunk |
| `project.c` `freedata()` | `luascript_close()` to cleanup the engine |
| `epanet.c` `EN_initH()` | fires `on_open` |
| `epanet.c` `EN_runH()` | fires `on_report`, only once `runhyd()` has succeeded |
| `epanet.c` `EN_closeH()` | fires `on_close` |
| `hydraul.c` `runhyd()` | fires `on_iteration` after the step converges, and re-solves while the script keeps changing the model |
