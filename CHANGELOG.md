# Changelog

All notable changes to D2BS are documented here. The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and versions follow [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Versions up to 1.6.4 have no section here; their history is in the git log, and 1.6.4 is tagged `v1.6.4`.

Each release candidate (1.7.0-rc.1, 1.7.0-rc.2, ...) has its own section, listing what changed since the release before it; the final release brings them together in one section.

## [Unreleased]

### Fixed

- **Steady preset lists.** `getPresetUnits()`, `getPresetUnit()` and `Room.getPresetUnits()` keep a room's preset list as it is however often they are called, and leave rooms that are already loaded as they are.
- **Tidy preset lookups.** `getPresetUnit()` unloads the room data it loads to find a unit, as `getPresetUnits()` does, so each lookup in a distant area releases the rooms it loaded.
- **Preset lookups across act changes.** When `getPresetUnit()` or `getPresetUnits()` finds an area, it finishes searching its rooms before an act change can unload them, so a lookup that overlaps an act change completes and the game carries on.
- **NPC message packets for other unit types.** The game keeps running when an NPC message packet (`0x27`) for a unit type other than a monster or an object arrives during an NPC's speech or with no NPC open, for example a packet a script passes to `getPacket()`, and you can talk to the NPC again afterwards.
- **Rooms a script keeps.** A `Room` from `getRoom()`, `getNearby()` or `getFirst()` is safe to use for as long as a script holds it, even after the game unloads its area or the player changes acts or games. Its `x`, `y`, `xsize`, `ysize`, `area`, `level`, `number` and `correcttomb` keep their values until `getNext()` moves it to the next room. Its methods find the room again wherever `getRoom()` could, in the room's own game and act, and when they can't, `getNext()` returns `false`, `getNearby()` an empty array, `getStat()` `null` and the other methods `undefined`, while `unitInRoom()` returns `false` for a unit in another room, act or game.
- **Objects built under memory pressure.** A script that runs out of memory while D2BS builds a unit, item, party, control, script, dialog line, directory or database object for it, a `getMouseCoords()` result, a `getPath()` point with the built-in reducers, or a `getStat(-2)` skill entry, stops at that call, as it would anywhere else, and the game keeps running. Afterwards, the next `area.exits` read returns the area's full exit list, a database whose `query()` failed this way still closes cleanly, and every script can still open files after a `File.open()` with file locking fails this way.
- **Sturdier room lookups.** The game keeps running when a room has no collision data: `Room.getStat()` returns `null` for its collision stats (9-16), and `getRoom()` passes over it when searching by position. `getRoom()` and `getRoom(0)` return `undefined` when the player's room isn't available. `Room.getStat()` and `Room.getPresetUnits()` unload every room they load for the lookup, and a script that runs out of memory during `getRoom()`, `getPresetUnit()`, `getPresetUnits()` or a `Room` method stops at that call, as it would anywhere else.
- **Accurate room details.** Preset units from `Room.getPresetUnits()` report their area in `level`, as those from `getPresetUnits()` do, and `Room.getStat(14)` returns the collision data's `posRoomY`, the y counterpart of `getStat(13)`.
- **Checked preset and room arguments.** `getPresetUnit()` and `getPresetUnits()` read their type and class id as whole numbers, converted the way `>>> 0` converts them, so `"2"` and `2.5` filter like `2`, `undefined`, `null`, `0`, `""` and `false` match any type or class, and a value that converts to `NaN`, such as `"obj"`, matches nothing. When converting an argument throws (a `valueOf()` that throws), the error reaches the script, for these and for `getRoom(x, y)` and `getRoom(area, x, y)`.
- **Pathing from several scripts at once.** Several scripts can call `getPath()` and `getCollision()` and read `area.exits` at the same time, each with its own search, and the game carries on after `getPath()` throws for an unknown reduction type.
- **Lean teleport paths.** A teleport `getPath()` call (reduction type 1) releases its teleport search data when it returns, so a bot can plan teleport paths all game long.
- **Safe database statements.** A script can read an `SQLite` database's `statements` alongside the statements `query()` returned, and close any of them in any order. Stepping a statement whose database is closed, with `next()`, `go()` or `skip()`, throws "The statement is closed".

## [1.8.0-rc.1] - 2026-11-20

### Added

- **Monster level stats.** `getBaseStat("monlvl", level, column)` reads the game's MonLvl table: the per-level defense, to-hit, life, damage and experience that monster stats are percentages of, for each difficulty, classic and expansion. The row is the monster level, and the columns are MonLvl.txt's value columns, for example `"L-DM(H)"`.
- **The whole character list at once.** On the expansion character select screen, `getCharacterList()` returns every character with its class, level and hardcore, expansion, ladder and expiry details, and `focusCharacter(name)` selects any of them by name and scrolls it into view, so a script can pick a character without paging through the list.
- **Hang reports.** When the game window stops responding for 5 seconds, D2BS writes what the game is stuck on to its log: the game thread's stack, the D2BS or game lock it appears to wait on and the thread that holds it, and for the first three hangs of a game process the other threads' stacks too. The last packets go to `logs\hang-<profile>-<time>-<pid>.packets.txt`. At the default wait the report starts reaching the log about 5 seconds into the hang, well before a bot manager's restart. The game keeps running, each thread paused only for the moment its stack is read, and a game that recovers logs how long it was stuck. With `HaltDumps=true`, a hang also writes a memory dump, one per Game.exe (Windows 8.1 or later), from a snapshot that pauses the game only for an instant. The `logs` folder keeps the 20 newest hang packet logs and the 2 newest hang dumps. Hang reports cover the game once D2BS has finished starting up and its timer has run. `HangReportSeconds` in `d2bs.ini` sets the wait, and 0 turns hang reports off.

### Fixed

- **Skipping NPC speeches.** The game keeps running when a script closes an NPC mid-speech, for example with `me.cancel()`, or leaves the game during one, and later speeches play out as usual.
- **Uninterrupted belt use.** The game keeps running as potions move down the belt.
- **Waypoint trips with a fallen mercenary.** The game keeps running when you take a waypoint back to the area where your mercenary died.
- **Safe `getBaseStat()` lookups.** `getBaseStat()` returns `undefined` for a table or column that doesn't exist, whether named or numbered, and for a row past the end of its table, so scripts can look up any value without risk to the game. Called with fewer than three arguments, it also returns `undefined`.
- **Reliable `getPacket()`.** Packets a script passes to `getPacket()` join the game's incoming packets reliably, even while the game is receiving its own. Outside a game, `getPacket()` returns `false` and passes nothing on.

## [1.7.0] - 2026-11-13

### Added

- **`clearAllTimers()`** cancels every timer of the calling script, including an interval whose callback is running, and returns how many it cancelled.
- **Timer arguments.** `setTimeout(fn, delay, ...args)` and `setInterval(fn, delay, ...args)` pass their extra arguments to the callback.
- **Crash details in the log.** When the game crashes, the D2BS log records the exception with its registers and every thread's stack, and the game ends with the real exception code, so Windows Error Reporting can write a dump of it. A stack overflow, which leaves no stack to work with, goes to Windows Error Reporting directly.
- **Packet logs.** When the game crashes (except from a stack overflow) or halts on one of its own error checks, D2BS writes the last 512 game packets sent and received to its `logs` folder as `crash-<profile>-<time>-<pid>.packets.txt`.
- **Halt dumps on request.** With `HaltDumps=true` in `d2bs.ini`, a halt also writes a memory dump beside its packet log, as `.dmp`. A dump can hold account details, so share it privately.
- **Bounded crash files.** The `logs` folder, shared by every profile, keeps at most 5 dumps (a few hundred MB each) and 50 packet logs, deleting the oldest first, and D2BS skips the dump when the disk has less than 2 GB free.
- **Debug symbols with every release.** `D2BS.pdb` ships beside `D2BS.dll`, so a crash or halt dump shows D2BS's own functions.
- **Build identity.** `version()`, the startup log line and the DLL's file properties name the commit the DLL was built from, for example `1.7.0U (7c036ba)`; the on-screen logo shows the version alone.

### Changed

- **Fast game exits.** Leaving a game takes a fraction of a second, even right after a run through crowded areas: D2BS lets the game load its graphics at full disk speed instead of a fixed 256 KB/s.
- **`setInterval(fn, 0)` repeats.** A zero period makes a repeating timer, like any other period; use `setTimeout()` for a single call.
- **One call at a time per interval.** An interval whose callback is still running, for example inside `delay()`, is not started again on top of it, and periods missed while the script was busy give one call.
- **Timers outlive `clearAllEvents()`.** Timers stop with `clearTimeout()`, `clearInterval()`, `clearAllTimers()` or the end of their script.
- **Safer dialog lines.** A line from an earlier `getDialogLines()` runs its handler only while it is one of the open dialog's lines, and otherwise throws "That dialog is not currently clickable."
- **Game actions don't wait for other scripts' sent-packet listeners.** A packet sent from inside a call that acts between game frames, such as `clickMap()`, `me.cancel()`, `clickItem()`, `item.shop()`, `transmute()` or `clickParty()`, reaches other scripts' `gamepacketsent` listeners without waiting for their answer, so they can't block it and the game keeps running; the calling script's own listeners still can.
- **Each listener once per event.** `addEventListener()` with a function already listening to that event leaves it as it is, as in a browser, so the function runs once per event and one `removeEventListener()` call removes it.
- **`getTickCount()` keeps counting.** It counts on past 4,294,967,295 ms (49.7 days of Windows uptime), so time differences in scripts stay right however long Windows has run, and `me.gamestarttime` uses the same clock. On Windows XP it still starts again from 0.

### Fixed

- **Reliable timers.** Each timer calls only its own callback, once per firing, and scripts can set and clear timers, their own or each other's, at the same time.
- **Any timer delay works.** `setTimeout()` and `setInterval()` accept fractional delays such as `1.5`, and a negative or NaN delay fires at once.
- **Scripts end with their game.** A stopped script, or one whose game has ended, ends even when that happens during one of its timer callbacks or event handlers, and frees its thread, memory and timers.
- **Safe slow listeners.** A key, chat, packet or screen-hook click listener that runs past its time limit (1 s for keys, 0.5 s for chat and packets, 3 s for clicks) lets the event through, and the game and the script carry on.
- **Steadier shopping, cubing and walking.** `me.cancel()`, `unit.useMenu()`, `unit.interact()` (except on an item the player holds, which only sends a packet), `unit.move()`, `clickMap()`, dialog-line handlers, `transmute()` and the inventory setup on joining a game act between game frames, so bots that shop, gamble, cube and walk keep running.
- **Area lookups keep the game running.** `getArea()` and its properties such as `exits`, `getRoom()`, `getPresetUnit()`, `getPresetUnits()`, `getPath()` and `getCollision()` called while the game changes areas or leaves a game let the game carry on drawing.
- **Stable Act 5 wilderness.** Frigid Highlands, Arreat Plateau, Frozen Tundra and the other Act 5 outdoor areas build reliably in every game, whether the character walks in, takes a waypoint or a script looks up a room there with `getRoom()`.
- **Long sessions keep running.** The game keeps loading monster and character graphics hour after hour, including while a bot crosses one crowded area after another.
- **Stable memory pools.** The game's memory pools stay consistent while several threads use them at once, so bots keep running game after game.
- **Safe with sound on.** Bots that play with sound keep running while the game starts and stops its music and other streamed sounds.
- **PCs that stay on for weeks.** Single-player games keep running however long Windows has been up.
- **Folder listings release their handles.** `dopen().getFiles()` and `getFolders()` close their folder search when done, so scripts can list folders as often as they like without adding to the game's handle count.
- **Safe unit lookups.** Finding a unit by its id answers "not found" for a unit the game has just freed, as for any unit that's gone.
- **Dependable `getPlayerFlag()`.** It returns `true` or `false`, so `=== true` works, answers for players anywhere in the game rather than only those nearby, and is safe to call while a game closes.
- **Safe party objects.** A `Party` object from `getParty()` follows its player: once that player leaves the game, or the game ends, its properties read `undefined`, `getNext()` returns `false` and `clickParty()` returns `false`, even when a later game gives another player the same id. `clickParty()` acts between game frames, `clickParty(player, 3)` leaves your party whichever player in the game you pass, and `getParty()` returns `undefined` for anything that doesn't name a player, such as a monster unit or `null`.
- **Missile casters.** A missile's `getParent()` returns the unit that cast it, or undefined when the caster is out of sight; `missile.owner` and `missile.ownertype` still give its id and type.
- **Independent Battle.net version checks.** Each game process uses its own temporary directory for the revision DLL, so profiles can verify their game version concurrently.
- **Large file writes.** `FileTools.writeText()`, `FileTools.appendText()` and `file.write()` save text of any size.
- **Binary number writes.** `file.write()` on a file opened in binary mode saves each number's value, with or without file locking.
- **Dependable file reads, seeks and closes.** `FileTools.readText()` throws when reading the file fails, a `File` whose `close()` throws is closed and can be reopened, `file.seek(n)` on a file opened with locking moves `n` bytes, and a binary `file.read(count)` returns 0 for each value past the end of the file and frees its memory when done.
- **Steady memory for busy bots.** Every event a listener receives and every `getPath()` call frees the memory it used, so bots that trade many chat, `copydata` and script messages keep a steady footprint over long runs.

[Unreleased]: https://github.com/blizzhackers/d2bs/compare/v1.8.0-rc.1...HEAD
[1.8.0-rc.1]: https://github.com/blizzhackers/d2bs/compare/v1.7.0...v1.8.0-rc.1
[1.7.0]: https://github.com/blizzhackers/d2bs/compare/v1.6.4...v1.7.0
