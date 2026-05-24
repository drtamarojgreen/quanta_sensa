# QuantaSensa SDD Restrictions

## Pattern Restrictions
- Prohibit empty catch blocks in C++ and empty except blocks in Python.
- Prohibit meaningless assertions (e.g., asserting string is not empty).
- All assertions must verify specific expected empirical values.
- No forbidden markers (T.O.D.O, S.T.U.B, p.l.a.c.e.h.o.l.d.e.r) in source code.
- **LLM Laziness Prohibition**: Using '1' or '0' as lazy substitutes for boolean values is prohibited. Numeric evidence must represent actual physical or logical measurements (counts, sizes, versions, timestamps, durations).

## Tool Restrictions
- Allowed tools: g++, python3, pytest, bash.
- Allowed libraries: std::thread, std::atomic, inotify (via FileWatcher), and standard C++17 headers.
- Any external tool usage must be discovered and verified before use.

## Architectural Restrictions
- **Single Action Principle**: The Python agent must perform exactly one atomic action per invocation.
- **Persistent Agent / Controller Separation**: The C++ controller must manage the lifecycle of the Python agent.
- **Pipe-Based Communication**: All events must be communicated via the designated named pipe `/tmp/prismquanta_events.pipe`.
- **Sip Principle**: Work must be performed in minimal increments (compile, run, observe, stop).
- **Dual-Ledger**: All work must be tracked in `sorrel_checkins.md` and `sorrel_checkouts.md`.

## Validation Restrictions
- All validation logic must reference real outputs (e.g., exit codes, file existence, pipe messages).
- Numeric Evidence Doctrine: All cards must produce numeric, machine-parseable observations.
- Self-certification using only boolean (pass/fail) or binary flags (1/0) is prohibited. Evidence must be descriptive.
- Green Syntax: Cards and structured communications must use TOOLS, PARAMETERS, and RESULTS blocks.
