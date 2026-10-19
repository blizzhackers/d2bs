# Changelog

All notable changes to D2BS are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Versions up to 1.6.4 were never tagged; their history is in the git log.

## [Unreleased]

### Added

- **`clearAllTimers()`** cancels every timer of the calling script, including the one whose callback is running, and returns how many it cancelled.
- **Timer arguments.** `setTimeout(fn, delay, ...args)` and `setInterval(fn, delay, ...args)` pass their extra arguments to the callback.
- **Crash details in the log.** When the game crashes, the D2BS log records the exception with its registers and every thread's stack, and the game ends with the real exception code, so Windows Error Reporting can write a dump of it.
- **Halt dumps and packet logs.** When the game halts on one of its own error checks, D2BS writes a dump and a log of the last 512 game packets sent and received to its `logs` folder as `crash-<profile>-<time>-<pid>.dmp` and `.packets.txt`; a crash writes the packet log.
- **Bounded crash files.** The `logs` folder, shared by every profile, keeps the 5 newest dumps (a few hundred MB each) and 50 packet logs, and D2BS skips the dump when the disk has less than 2 GB free.
- **Debug symbols with every release.** `D2BS.pdb` ships beside `D2BS.dll`, so a crash or halt dump shows D2BS's own functions.
- **Build identity.** `version()`, the startup log line, the on-screen logo and the DLL's file properties name the commit the DLL was built from, for example `1.7.0U (7c036ba)`.

### Changed

- **Fast game exits.** Leaving a game takes a fraction of a second, even right after a run through crowded areas: D2BS lets the game load its graphics at full disk speed instead of a fixed 256 KB/s.
- **`setInterval(fn, 0)` repeats.** A zero period makes a repeating timer, like any other period; use `setTimeout()` for a single call.
- **One call at a time per interval.** An interval whose callback is still running, for example inside `delay()`, is not started again on top of it, and periods missed while the script was busy give one call.
- **Timers outlive `clearAllEvents()`.** Timers stop with `clearTimeout()`, `clearInterval()`, `clearAllTimers()` or the end of their script.
- **Safer dialog lines.** A line from an earlier `getDialogLines()` runs its handler only while its dialog is still the one open, and otherwise throws "That dialog is not currently clickable."

### Fixed

- **Reliable timers.** Each timer calls only its own callback, once per firing, and scripts can set and clear timers, their own or each other's, at the same time.
- **Any timer delay works.** `setTimeout()` and `setInterval()` accept fractional delays such as `1.5`, and a negative or NaN delay fires at once.
- **Safe slow listeners.** A key, chat, packet or screen-hook click listener that runs past its time limit (1 s for keys, 0.5 s for chat and packets, 3 s for clicks) lets the event through, and the game and the script carry on.
- **Steadier shopping, cubing and walking.** `me.cancel()`, `unit.useMenu()`, `unit.interact()`, `unit.move()`, `clickMap()`, dialog-line handlers, `transmute()` and the inventory setup on joining a game act between game frames, so bots that shop, gamble, cube and walk keep running.
- **Area lookups keep the game running.** `getArea()` and its properties such as `exits`, `getRoom()`, `getPresetUnit()`, `getPresetUnits()`, `getPath()` and `getCollision()` called while the game changes areas or leaves a game let the game carry on drawing.
- **Stable Act 5 wilderness.** Frigid Highlands, Arreat Plateau, Frozen Tundra and the other Act 5 outdoor areas build reliably in every game, whether the character walks in, takes a waypoint or a script looks up a room there with `getRoom()`.
- **Safe unit lookups.** Finding a unit by its id answers "not found" for a unit the game has just freed, as for any unit that's gone.
- **Dependable `getPlayerFlag()`.** It returns `true` or `false`, so `=== true` works, and it is safe to call while a game closes.
- **Missile casters.** A missile's `getParent()` returns the unit that cast it, or undefined when the caster is out of sight; `missile.owner` and `missile.ownertype` still give its id and type.
- **Reliable hover handlers.** A screen hook's hover handler gets exactly the cursor's x and y, and hovering for hours uses no extra memory.

[Unreleased]: https://github.com/blizzhackers/d2bs/compare/bf77a8f...HEAD
