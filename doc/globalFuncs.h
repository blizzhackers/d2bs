/** \defgroup globalFunctions Global functions
 */

/** Get a unit by type, name, mode and nUnitId.
 *
 * \ingroup globalFunctions
 *
 * \param type The type of the unit. Set to -1 to get any unit. 100 gets the
 * cursor item, 101 gets the selected unit.
 *
 * \param name The name of the unit to look for.
 *
 * \param mode Either the mode being searched for, or a bitmask with bit 29 set
 * and bits 0-28 corresponding to modes of units being searched for.
 *
 * \param nUnitId The unique id of the unit.
 *
 * \return The first unit found that matches the description.
 */
Unit getUnit(int type = -1, String name = "", uint32_t mode = (uint32_t)-1,
	uint32_t nUnitId = (uint32_t)-1);

/** Get a unit by type, classId, mode and nUnitId.
 *
 * \ingroup globalFunctions
 *
 * \param type The type of the unit. Set to -1 to get any unit. 100 gets the
 * cursor item, 101 gets the selected unit.
 *
 * \param classId The class id of the unit.
 *
 * \param mode Either the mode being searched for, or a bitmask with bit 29 set
 * and bits 0-28 corresponding to modes of units being searched for.
 *
 * \param nUnitId The unique id of the unit.
 *
 * \return The first unit found that matches the description.
 */
Unit getUnit(int type = -1, uint32_t classId = (uint32_t)-1,
	uint32_t mode = (uint32_t)-1, uint32_t nUnitId = (uint32_t)-1);

/** Creates a path (walking or teleporting) from the source to the destination.
 * Returns an array of points that form a path from the source to the
 * destination. Takes Area ID(s), source and destination points, walking or
 * teleporting and range as parameters.
 *
 * \ingroup globalFunctions
 *
 * \param Area The Area ID to path through.
 *
 * \param srcX The source X coordinate.
 *
 * \param srcY The source Y coordinate.
 *
 * \param dstX The destination X coordinate.
 *
 * \param dstY The destination Y coordinate.
 *
 * \param reductionType The type of reduction: {0 - walking, 1 - teleport,
 * 	2 - none}
 *
 * \param Radius The distance between each point.
 *
 * \return The path as an array of objects with x and y properties.
 * Throws an error when the area is not in the act the player is in.
 */
object[] getPath(uint32_t Area, uint32_t srcX, uint32_t srcY, uint32_t dstX,
	int dstY, uint32_t reductionType = 0, uint32_t Radius = 20);

/** Creates a path (walking or teleporting) from the source to the destination.
 * Returns an array of points that form a path from the source to the
 * destination. Takes Area ID, source and destination points, walking or
 * teleporting, range and reduction or not as parameters.
 *
 * \ingroup globalFunctions
 *
 * \param Area The Area ID to path through.
 *
 * \param srcX The source X coordinate.
 *
 * \param srcY The source Y coordinate.
 *
 * \param dstX The destination X coordinate.
 *
 * \param dstY The destination Y coordinate.
 *
 * \param reductionType The type of reduction: {0 - walking, 1 - teleport,
 * 	2 - none, 3 - user specified}
 *
 * \param Radius The distance between each point. Reduction type 3 does not
 * 	use it, but it must be passed so the functions are the 8th to 10th
 * 	arguments.
 *
 * \param reject Called at most once for each point, with (x, y): for the
 * 	start and the end before the search, then for each neighbouring point the
 * 	search considers, except the end. A truthy return blocks the point. A
 * 	point outside the current act's levels is blocked without a call.
 *
 * \param reduce Called once for each point of the path found, in order, with
 * 	the point ({x, y}), its index and the array of all the points. A truthy
 * 	return keeps the point in the result.
 *
 * \param mutate Called with (x, y) for the start or the end when it is
 * 	blocked, before the search. Return [x, y] to move that point (fractions
 * 	are truncated); a return that isn't an object keeps it. A start or end
 * 	that is outside the current act's levels once mutate returns, moved
 * 	there or kept, has no path, so getPath returns an empty path without
 * 	calling reject or mutate again. An object whose elements 0 and 1 don't
 * 	convert to numbers in the 32-bit integer range makes getPath throw a
 * 	TypeError.
 *
 * Reduction type 3 needs all three functions, or getPath throws. They are
 * called with getPath's this (the global object for a plain call). An error
 * one of them throws, or the script being stopped while one runs, ends the
 * getPath call with it. When the script already holds the game lock as getPath
 * starts, for example in a gamepacketsent listener that runs inside clickMap(),
 * they run with the game paused and the D2BS log notes it once for that call;
 * otherwise they run without the game lock, so the game keeps running while one
 * waits, for example in delay(). They may call getCollision() and getPath().
 * Inside a listener the game is already waiting on, the game also stays paused
 * until getPath returns or the game stops waiting for that listener (after
 * 500 ms for a packet or chat blocker, 1 s for keyupblocker or keydownblocker,
 * 3 s for a hook's click handler). After each reject or mutate call that
 * returns normally, getPath checks that the player is still in the same game,
 * act and area; if not, it calls none of the three again and returns an empty
 * path. A trip away and back to the same area within one call may not be
 * noticed. reduce runs after the search, so the path it reduces is still
 * returned.
 *
 * \return The path as an array of objects with x and y properties.
 * Throws an error when the area is not in the act the player is in.
 */
object[] getPath(uint32_t Area, uint32_t srcX, uint32_t srcY, uint32_t dstX,
	int dstY, uint32_t reductionType, uint32_t Radius,
	bool (*reject)(int32_t x, int32_t y),
	bool (*reduce)(object curPt, int i, object[] pts),
	int32_t[] (*mutate)(int32_t x, int32_t y));

/** Get the collision flags at a given point in a given area.
 *
 * \ingroup globalFunctions
 *
 * \param nLevelId The level id of the area in question.
 *
 * \param nX The x coordinate of the point in question.
 *
 * \param nY The y coordinate of the point in question.
 *
 * \return The collision flags. See \ref reference_collision.
 * Throws an error when the area is not in the act the player is in. Returns
 * 65535 (all bits set) for a point that lies in no loaded room of the area, and
 * for barricade-tower spots in areas 74, 111, 112 and 117.
 */
unsigned short getCollision(uint32_t nLevelId, int32_t nX, int32_t nY);

/** Get the health of the controlled unit's merc, as a percentage.
 *
 * \ingroup globalFunctions
 *
 * \return The merc's HP percentage, 0 while the merc is dead, undefined when
 * there is no merc, or false when the game is not ready.
 */
int getMercHP();

/** Get the state of the in-game hand cursor.
 *
 * The value is the cursor's animation state, not a cursor image id:
 *
 * | Value | Meaning |
 * |---|---|
 * | 1 | Normal pointer. |
 * | 2 | The hand opening after 5 seconds without a mouse move or click. |
 * | 3 | The hand closing again. While an item is held, a mouse move gives 3, and a click can then give 5 and then 1. |
 * | 4 | The open hand idling. Also set when an item is put on the cursor, until the mouse moves. |
 * | 5 | The click animation while the mouse button is down (hand cursor only, not in states 6 and 7). |
 * | 6 | Item-target cursor, for example after using an identify scroll or tome. |
 * | 7 | Shop action cursor, after the NPC shop's Buy, Sell or Repair button is clicked. |
 *
 * The value does not reliably tell whether an item is held: use me.itemoncursor
 * for that. There is no game-ready check, so outside a game the call returns
 * whatever value the game last held.
 *
 * \ingroup globalFunctions
 *
 * \return The hand-cursor state.
 */
int getCursorType();

/** Get the hand-cursor state, or the item-panel mode when nType is 1.
 *
 * With nType 1 the call returns the mode shared by the NPC shop, player trade,
 * stash, Horadric Cube and submit-item panels. With any other value it returns
 * the hand-cursor state described for getCursorType(). The argument is
 * converted to an integer first, so "1" and 1.9 count as 1; a conversion that
 * throws ends the call with that exception. There is no game-ready check, so
 * outside a game the call returns whatever value the game last held.
 *
 * Item-panel modes (nType 1). Values 0, 1, 12 and 14 were seen in a live
 * game; the meanings of 5, 6, 7 and 19 are less certain than the others:
 *
 * | Value | Meaning |
 * |---|---|
 * | 0 | No item panel mode (cleared when a panel closes). |
 * | 1 | NPC shop open, no action selected. |
 * | 2 | Buy button active. |
 * | 3 | Sell button active. |
 * | 4 | Repair button active. |
 * | 5 | A buy, sell or repair of an item has been requested. |
 * | 6 | The transaction was sent; waiting for the server. |
 * | 7 | The NPC answered with a transaction result message; the mode then returns to 1. |
 * | 10 | Player trade: the trade request prompt is open. |
 * | 11 | Player trade: the trade window is open. |
 * | 12 | Stash open. |
 * | 13 | A stash sub-state; meaning unknown. It turns back to 12 when a gold-amount dialog opens. |
 * | 14 | Horadric Cube open. |
 * | 19 | Submit-item panel open for the Horadric Staff orifice (other uses of the submit-item panel were not checked). |
 *
 * \ingroup globalFunctions
 *
 * \param nType 1 - the item-panel mode, any other value - the hand-cursor
 * state.
 *
 * \return The item-panel mode or the hand-cursor state.
 */
int getCursorType(int nType);

/** Get skill ID by name.
 *
 * \ingroup globalFunctions
 *
 * \param skillName The skill name.
 *
 * \return The skill ID (0 is Attack), or undefined when the name is not a known skill or the argument is not a string. The name is matched case-insensitively.
 */
int getSkillByName(String skillName);

/** Get skill name by ID.
 *
 * \ingroup globalFunctions
 *
 * \param skillId The skill ID.
 *
 * \return The skill name.
 */
String getSkillById(int skillId);

/** Get the String in the current locale that corresponds to the given id.
 *
 * \ingroup globalFunctions
 *
 * \param localeId The id of the String.
 *
 * \return The String in the current locale.
 */
String getLocaleString(uint16_t localeId);

/** Get the width and height of the given text in the given font.
 *
 * \ingroup globalFunctions
 *
 * \param string The string to get the size of.
 *
 * \param font The font to use.
 *
 * \param asObject False to return as an array.
 *
 * \return The width and height of the text. 0 - width, 1 - height.
 */
int[] getTextSize(String string, int font, bool asObject = false);

/** Get the width and height of the given text in the given font.
 *
 * \ingroup globalFunctions
 *
 * \param string The string to get the size of.
 *
 * \param font The font to use.
 *
 * \param asObject True to return as an object.
 *
 * \return The width and height of the text. Has properties .width and .height.
 */
Object getTextSize(String string, int font, bool asObject);

/** Get the priority of the current thread.
 *
 * \ingroup globalFunctions
 *
 * \return The priority.
 */
int getThreadPriority();

/** Get whether or not a UI flag is set.
 *
 * \ingroup globalFunctions
 *
 * Valid ids are 0 to 37. See \ref reference_uiflags for what each id tracks.
 *
 * \warning Any other id (38 or more, or a negative id) makes the game exit
 * immediately. D2BS does not check the range.
 *
 * \param nUIId The ID of the UI in question, 0 to 37.
 *
 * \return true when the panel or state is open or active, false when it is not.
 * Returns undefined when the argument is missing or is not an integer (1.5 or a
 * string), and false (with a 'Game not ready' warning) outside a ready game.
 * While a game is loading the call can first wait for it to become ready.
 */
bool getUIFlag(int nUIId);

/** Get the current player-trade state.
 *
 * Modes 0 and 2 return the same number:
 *
 * | Value | Meaning |
 * |---|---|
 * | 0 | No trade. |
 * | 1 | You asked another player to trade and are waiting for the answer (a box with only a Cancel button shows). |
 * | 2 | Another player asked to trade with you and the accept/decline prompt is showing. |
 * | 3 | The trade window is open, or about to open, and you have not accepted. |
 * | 5 | The trade window is open and the other player has accepted; you have not. |
 * | 7 | You have accepted. Also used for 30 seconds after a trade request was declined or cancelled (by you, or automatically by the game), with no trade window open; the state then returns to 0. |
 *
 * The game declines new trade requests on its own whenever the state is not 0.
 *
 * \ingroup globalFunctions
 *
 * \param nMode 0 or 2 - the trade state. Both return the same value.
 *
 * \return The trade state. An integer mode other than 0, 1 and 2 returns false.
 * With no argument the call returns false at once; with a non-integer argument
 * it waits for the game, then returns false. Outside a ready game it returns
 * false (with a 'Game not ready' warning).
 */
int getTradeInfo(int nMode);

/** With nMode 1, meant to get the name of the player you are trading with; it
 * always returns null.
 *
 * \ingroup globalFunctions
 *
 * \param nMode 1.
 *
 * \return null.
 */
String getTradeInfo(int nMode);

/** Get whether the controlled unit has the given waypoint.
 *
 * \ingroup globalFunctions
 *
 * \param nWaypointId The area id of the waypoint in question.
 *
 * \return Whether or not the controlled unit has the waypoint.
 */
bool getWaypoint(int nWaypointId);

/** Get the current or first context.
 *
 * \ingroup globalFunctions
 *
 * \param currentScript Whether to get the current script (true) or the first
 * (false).
 *
 * \return A D2BSScript object for the request script.
 */
D2BSScript getScript(bool currentScript);

/** Get a script by thread id.
 *
 * \ingroup globalFunctions
 *
 * \param threadId The thread id of the script to get.
 *
 * \return A D2BSScript object for the script from the given thread.
 */
D2BSScript getScript(int threadId);

/** Get a script by filename.
 *
 * \ingroup globalFunctions
 *
 * \param name The filename of the script to find.
 *
 * \return A D2BSScript object for the script.
 */
D2BSScript getScript(String name);

/** Get the first script.
 *
 * \ingroup globalFunctions
 *
 * \return A D2BSScript representing the first script in the runtime.
 */
D2BSScript getScript();

/** Get the first room in area given by level id.
 *
 * \ingroup globalFunctions
 *
 * \param levelId The id of the area to get the room from, or 0 to get the room
 * the controlled unit is in. Any number or numeric string is read as an area id;
 * undefined, null and values that convert to NaN read as 0.
 *
 * \return The first room in the area, or the controlled unit's room for 0.
 * Waits while an area or act loads; undefined when the area is not in the
 * player's act, and can be undefined when an act change, a move between rooms
 * or the end of the game begins as the call runs. Throws "Get Room Game not
 * ready" out of game, when that wait times out, or at once while the game is
 * busy in a blocking listener or while your script holds the game lock.
 */
Room getRoom(uint32_t levelId);

/** Get the room that the given point is in from the given level id.
 *
 * \ingroup globalFunctions
 *
 * \param levelId The level id to look in.
 *
 * \param x The x coordinate to find a room for.
 *
 * \param y The y coordinate to find a room for.
 *
 * \return The room at the given point. Throws "Get Room Game not ready" out of
 * game, when the wait for an area or act load times out, or at once while the
 * game is busy in a blocking listener or while your script holds the game lock.
 */
Room getRoom(uint32_t levelId, int x, int y);

/** Get the room that the given point is in.
 *
 * \ingroup globalFunctions
 *
 * \param x The x coordinate to find a room for.
 *
 * \param y The y coordinate to find a room for.
 *
 * \return The room at the given point. Throws "Get Room Game not ready" out of
 * game, when the wait for an area or act load times out, or at once while the
 * game is busy in a blocking listener or while your script holds the game lock.
 */
Room getRoom(int x, int y);

/** Get the first room in the current area.
 *
 * \ingroup globalFunctions
 *
 * \return The first room in the current area; can be undefined when an act
 * change begins as the call runs. Throws "Get Room Game not ready" out of game, when
 * the wait for an area or act load times out, or at once while the game is busy
 * in a blocking listener or while your script holds the game lock.
 */
Room getRoom();

/** Get the first party.
 *
 * \ingroup globalFunctions
 *
 * \return The first party.
 */
Party getParty();

/** Get the party line for the player with the given name
 *
 * \ingroup globalFunctions
 *
 * \param name The name of the player to search for
 *
 * \return The party line
 */
Party getParty(String name);

/** Get the party line for the player with the given player id
 *
 * \ingroup globalFunctions
 *
 * \param playerId
 *
 * \return The party line
 */
Party getParty(uint32_t playerId);

/** Get the party line that corresponds to the given unit
 *
 * \ingroup globalFunctions
 *
 * \param player A player Unit; any other unit gives undefined.
 *
 * \return The party line
 */
Party getParty(Unit player);

/** Get the first PresetUnit of the given type and class id.
 *
 * \ingroup globalFunctions
 *
 * \param levelId The area id of the level to look for a PresetUnit in.
 *
 * \param nType The type of the PresetUnit. See getUnit for type codes. 0 or omitted matches any type, and a value that
 * converts to NaN matches none.
 *
 * \param nClassId The class id to find. See:
 * http://forums.d2botnet.org/viewtopic.php?f=18&t=986 and
 * http://forums.d2botnet.org/viewtopic.php?f=18&t=985 . 0 or omitted matches any class id, and a value that converts
 * to NaN matches none.
 *
 * \return The first PresetUnit found of type nType and class ID nClassId.
 */
PresetUnit getPresetUnit(uint32_t levelId, int nType = 0, int nClassId = 0);

/** Get an array of PresetUnit s of the given type and class id.
 *
 * \ingroup globalFunctions
 *
 * \param levelId The area id of the level to look for PresetUnit s in.
 *
 * \param nType The type of the PresetUnit. See getUnit for type codes. 0 or omitted matches any type, and a value that
 * converts to NaN matches none.
 *
 * \param nClassId The class id to find. See:
 * http://forums.d2botnet.org/viewtopic.php?f=18&t=986 and
 * http://forums.d2botnet.org/viewtopic.php?f=18&t=985 . 0 or omitted matches any class id, and a value that converts
 * to NaN matches none.
 *
 * \return An array of PresetUnit s found of type nType and class ID nClassId.
 */
PresetUnit[] getPresetUnits(uint32_t levelId, int nType = 0, int nClassId = 0);

/** Get the Area where the controlled unit currently resides.
 *
 * \ingroup globalFunctions
 *
 * \return An Area object for the current object.
 */
Area getArea();

/** Get the Area for area ID nArea.
 *
 * \ingroup globalFunctions
 *
 * \param nArea The area ID.
 *
 * \return An Area object.
 */
Area getArea(int32_t nArea);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
String getBaseStat(String szTableName, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
String getBaseStat(int32_t nBaseStat, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
String getBaseStat(String szTableName, int32_t nClassId, int32_t nStat);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
String getBaseStat(int32_t nBaseStat, int32_t nClassId, int32_t nStat);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
int getBaseStat(String szTableName, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
int getBaseStat(int32_t nBaseStat, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
int getBaseStat(String szTableName, int32_t nClassId, int32_t nStat);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
int getBaseStat(int32_t nBaseStat, int32_t nClassId, int32_t nStat);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
bool getBaseStat(String szTableName, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
bool getBaseStat(int32_t nBaseStat, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
bool getBaseStat(String szTableName, int32_t nClassId, int32_t nStat);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
bool getBaseStat(int32_t nBaseStat, int32_t nClassId, int32_t nStat);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
double getBaseStat(String szTableName, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * name.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param szStatName The stat name.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
double getBaseStat(int32_t nBaseStat, int32_t nClassId, String szStatName);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param szTableName The name of the table to look in.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
double getBaseStat(String szTableName, int32_t nClassId, int32_t nStat);

/** Get the base stat from the given table with the given class ID and stat
 * ID.
 *
 * \ingroup globalFunctions
 *
 * \param nBaseStat The stat to look up.
 *
 * \param nClassId The class ID of the ...
 *
 * \param nStat The stat ID.
 *
 * Tables, by index (either the name or the index selects a table): 0 items,
 * 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2,
 * 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp,
 * 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets,
 * 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable,
 * 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the
 * columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level
 * column is not stored, since the row is the level. In experience, row 0 holds
 * each class's maximum level, row L (from 1 to the maximum level) the
 * experience that reaches level L, and the row after that the experience cap.
 *
 * \return The stat, or undefined when the table, row or column does not
 * exist.
 */
double getBaseStat(int32_t nBaseStat, int32_t nClassId, int32_t nStat);

/** Get the control specified by type, location and size. -1 indicates parameter
 * not to be matched against.
 *
 * Any negative value, and any argument that is not an integer (such as the
 * string "6" or 6.5), also skips that field silently. Leaving out trailing
 * arguments skips them too, so with no arguments the call returns the first
 * control on the screen. Controls exist only at the out-of-game menus. Pass at
 * most five arguments: an integer in a sixth or later argument is written past
 * the end of D2BS's argument storage and can crash the game.
 *
 * nType is the control's type, the same value Control.type returns:
 *
 * | Type | Name (D2BS source) | Control |
 * |---|---|---|
 * | 1 | CONTROL_EDITBOX | Single-line text input (account, password, game name). |
 * | 2 | CONTROL_IMAGE | Image. |
 * | 3 | CONTROL_UNUSED | Never constructed in 1.14d; matches nothing. |
 * | 4 | CONTROL_TEXTBOX | Multi-line text box or label (read with getText). |
 * | 5 | CONTROL_SCROLLBAR | Scroll bar. |
 * | 6 | CONTROL_BUTTON | Button. |
 * | 7 | CONTROL_LIST | List box. |
 * | 8 | CONTROL_TIMER | Timer (invisible). |
 * | 9 | CONTROL_SMACK | Video player. |
 * | 10 | CONTROL_PROGRESSBAR | Progress bar. |
 * | 11 | CONTROL_POPUP | Popup. |
 * | 12 | CONTROL_ACCOUNTLIST | Account list. |
 *
 * \ingroup globalFunctions
 *
 * \param nType The type of control (see the table above).
 *
 * \param nX The x coordinate of the control to find.
 *
 * \param nY The y coordinate of the control to find.
 *
 * \param nXSize The width of the control.
 *
 * \param nYSize The height of the control.
 *
 * \return A Control object for the first control that matches every given
 * field, or undefined when none matches or when the client is not at the
 * out-of-game menus. Throws 'Failed to build control!' only when the Control
 * object cannot be created.
 */
Control getControl(int32_t nType = -1, int32_t nX = -1, int32_t nY = -1,
	int32_t nXSize = -1, int32_t nYSize = -1);

/** Get the relation between two players. It reads the party roster, so it answers for any player in the game, near
 * this one or not.
 *
 * \ingroup globalFunctions
 *
 * nFlag is a bit mask; combine bits with | to test several at once. The bits are the party screen's per-player
 * buttons, set by clicking them (or with clickParty) and also by the server's relation updates:
 *
 * | Bit | Name | Meaning |
 * |---|---|---|
 * | 1 | loot | The loot button (allow corpse looting); available only in hardcore games (clickParty mode 0). |
 * | 2 | ignore | The ignore toggle next to squelch, as kolbot names it (clickParty mode 4). |
 * | 4 | squelch | The squelch toggle (clickParty mode 5). |
 * | 8 | hostile | Hostile (clickParty mode 1). The game lets you declare hostility only toward a player of level 9 or more. |
 *
 * The flags read are the ones the first player holds toward the second. Do not rely on
 * getPlayerFlag(me.gid, other.gid, 8) to tell whether the other player has declared hostility toward you.
 *
 * \param nFirstUnitId The party-roster id (me.gid or a Party's gid) of the player holding the relation.
 *
 * \param nSecondUnitId The party-roster id of the player the relation is toward.
 *
 * \param nFlag The relation bits to test (see the table above).
 *
 * \return true when any bit of nFlag is set, otherwise false: also when both ids are the same, when the first
 * player is not in the party roster, when not exactly three numbers are passed, and when the game closes meanwhile.
 * Outside a ready game it returns false (with a 'Game not ready' warning).
 */
bool getPlayerFlag(int nFirstUnitId, int nSecondUnitId, int nFlag);

/** Get the tick count. Milliseconds since Windows started (GetTickCount64); it keeps counting past 2^32 (49.7 days),
 * except on Windows XP.
 *
 * \ingroup globalFunctions
 *
 * \return The tick count.
 */
double getTickCount();

/** Get the Unit that the controlled Unit is currently interacting with.
 *
 * \ingroup globalFunctions
 *
 * \return The Unit currently being interacted with.
 */
Unit getInteractedNPC();

/** Print the String representation of the arguments to the console.
 *
 * \ingroup globalFunctions
 */
void print(...);

/** Delay for nDelay milliseconds. This is done by suspending the context,
 * sleeping, then resuming the context for short (<50 ms) delays. For longer
 * delays the context is suspended, 50ms are slept away, the context is resumed,
 * sequence repeats until time slept is greater then nDelay ms.
 *
 * \ingroup globalFunctions
 *
 * \param nDelay Milliseconds to sleep.
 */
void delay(uint32_t nDelay);

/** Load file and start a new thread that runs its main function. File taken
 * from scriptDir\\file. All remaining parameters are copied to the new
 * script's main function; a value that can't be copied (a function, or a
 * game object such as a Unit) throws an error and starts nothing. Loading a
 * script that is already running ends the running copy; the new copy starts
 * once that copy has ended, or after half a second (at once when a script
 * loads its own file), and getScript() and getScripts() then list only the
 * new copy.
 *
 * \ingroup globalFunctions
 *
 * \param file The filename of the script.
 *
 * \return true when the script's thread started, false when the file was not
 * found or the thread could not start. A syntax error in the file still ends
 * that script on its own thread after load() returned true; the calling script
 * and the game keep running. A missing, empty, non-string or too-long file
 * name throws an error.
 */
bool load(String file, ...);

/** Determine whether a file has been included yet or not. File taken from
 * scriptDir\\libs\\file
 *
 * \ingroup globalFunctions
 *
 * \param file Filename of the file that might have been included.
 *
 * Throws if file is not a string.
 *
 * \return Whether or not the file has been included.
 */
bool isIncluded(String file);

/** Include file from the scriptDir\\libs directory.
 *
 * \ingroup globalFunctions
 *
 * \param file Filename of the file to include.
 *
 * Throws if file is not a string.
 *
 * \return Whether the inclusion was successful.
 */
bool include(String file);

/** Conditionally stop current script.
 *
 * \ingroup globalFunctions
 *
 * \param stop 1 if current script should be stopped.
 */
void stop(int stop = 0);

/** Conditionnaly stop current script.
 *
 * \ingroup globalFunctions
 *
 * \param stop True if current script should be stopped.
 */
void stop(bool stop);

/** Return a random number between low and high. Draws from the operating system's
 * random generator (the C runtime's rand_s), in and out of game; it never touches
 * the game's own random seed and never waits for the game.
 *
 * \ingroup globalFunctions
 *
 * \param low The lowest possible integer to return.
 *
 * \param high The highest possible integer to return.
 *
 * \return A random integer in the range [low,high].
 */
int rand(int32_t low, int32_t high);

/** Reveal the player's current level on the automap.
 *
 * \ingroup globalFunctions
 *
 * \param drawPresets True to also draw preset icons (shrines, special NPCs, level names). Optional; read only
 * when it is the only argument, and false when omitted or not a boolean.
 *
 * \return True once the player's level was walked and revealed (a room the game
 * cannot load stays hidden), false when the game is not ready.
 */
bool revealLevel(bool drawPresets = false);

/** Copy txt to the clipboard as Unicode text.
 *
 * \ingroup globalFunctions
 *
 * \param txt Text to copy to the clipboard. Throws when it is missing or not a string.
 *
 * \return true when the text was placed on the clipboard, false otherwise.
 */
bool copy(String txt);

/** Get the text from the clipboard.
 *
 * \ingroup globalFunctions
 *
 * \return The text on the clipboard, or an empty string when it holds no text.
 */
String paste();

/** Send data to another window by means of the WM_COPYDATA message. Uses
 * FindWindow to get the HWND and SendMessage to send the message.
 *
 * \ingroup globalFunctions
 *
 * \param windowClassName The class name of the receiving window.
 *
 * \param windowName The window name of the receiving window.
 *
 * \param nModeId Data to be sent as the dwData member of the copy data.
 *
 * \param data Data to be send as the lpData member of the copy data.
 *
 * \return The result of SendMessage.
 */
int sendCopyData(String windowClassName, String windowName, int32_t nModeId,
	String data);

/** Send data to another window by means of the WM_COPYDATA message. Uses
 * SendMessage to send the message to hWnd.
 *
 * \ingroup globalFunctions
 *
 * \param hWnd The handle to the receiving window.
 *
 * \param dummy Ignored.
 *
 * \param nModeId Data to be sent as the dwData member of the copy data.
 *
 * \param data Data to be send as the lpData member of the copy data.
 *
 * \return The result of SendMessage.
 */
int sendCopyData(HWND hWnd, String dummy, int32_t nModeId, String data);

/** Send data to a DDE server by means of DdeClientTransaction.
 *
 * \ingroup globalFunctions
 *
 * \param mode The mode of transaction to use.
 *
 * 0 - Do command given by pszItem with an XTYP_REQUEST and get the resulting
 * data.
 *
 * 1 - Send pszData to pszItem with an XTYP_POKE.
 *
 * 2 - Send pszData as an XTYP_EXECUTE command.
 *
 * \param pszDDEServer The service name of the DDE server to connect to.
 *
 * \param pszTopic The topic with which to start the connection.
 *
 * \param pszItem The item to request (mode 0) or poke (mode 1).
 *
 * \param pszData The data to poke (mode 1) or execute (mode 2).
 *
 * Arguments after the server are optional. mode must be an integer; sendDDE throws when mode or the server is missing.
 *
 * \return Result data (at most 255 bytes) if mode was 0, otherwise false. A server that is not running gives ""
 * in mode 0. Apart from the argument errors, it throws only when DDE itself cannot start.
 */
String sendDDE(int32_t mode, String pszDDEServer, String pszTopic,
	String pszItem, String pszData);

/** Get whether or not a key is pressed. Returns the value of GetAsyncKeyState.
 *
 * \ingroup globalFunctions
 *
 * \param vKey The virtual key code for the key in question. See:
 * http://msdn.microsoft.com/en-us/library/dd375731%28v=VS.85%29.aspx
 *
 * \return Whether not the key is pressed.
 */
bool keystate(int vKey);

/** Add an event handler to the list of listeners for the given event. A function already listening to that event is
 * not added again, so it runs once per event and one removeEventListener() call removes it. Listeners run in the
 * order they were added. A listener removed while an event is being delivered (with removeEventListener(),
 * clearEvent() or clearAllEvents(), also from inside a listener) is not called for that event if it has not run
 * yet (unless it is added again before its turn), and one added meanwhile runs from the next event. An error a
 * listener throws is printed when it is thrown, the event's other listeners still run, and the error never blocks
 * the event or reaches the rest of the script. Once the script is stopped (or, for an in-game script, its game
 * ends), no further listener of the event runs.
 *
 * \ingroup globalFunctions
 *
 * \param event The name of the event. Can be:
 *
 * melife (uint32_t dwLife)
 *
 * memana (uint32_t dwMana)
 *
 * keyup (uint32_t key)
 *
 * keydown (uint32_t key)
 *
 * bool keyupblocker (uint32_t key) - Returns true to block event from Diablo
 * 	II. Blocking a key-up whose key-down reached the game leaves that key held
 * 	down in the game until a later key-up for it passes.
 *
 * bool keydownblocker (uint32_t key) - Returns true to block event from Diablo
 * 	II
 *
 * playerassign (uint32_t unitId)
 *
 * mouseclick (uint32_t button, uint32_t x, uint32_t y, bool bUp)
 *
 * mousemove (uint32_t x, uint32_t y)
 *
 * scriptmsg (...)
 *
 * golddrop (uint32_t gId, uint32_t Mode)
 *
 * chatmsg (String lpszNick, String lpszMsg)
 *
 * bool chatmsgblocker (String lpszNick, String lpszMsg) - Returns true to block
 * 	event from Diablo II
 *
 * whispermsg (String lpszNick, String lpszMsg)
 *
 * bool whispermsgblocker (String lpszNick, String lpszMsg) - Returns true to
 * block event from Diablo II
 *
 * copydata (int32_t dwMode, String lpszMsg)
 *
 * itemaction (uint32_t gId, uint32_t mode, String code, Bool global)
 *
 * gameevent (BYTE mode, DWORD param1, DWORD param2, String name1, String name2)
 *
 * From http://www.blizzhackers.cc/viewtopic.php?t=392307 Dark_Mage-
 *
 * mode:
 *
 * 0x00 - "%Name1(%Name2) dropped due to time out."
 * 0x01 - "%Name1(%Name2) dropped due to errors."
 * 0x02 - "%Name1(%Name2) joined our world. Diablo's minions grow stronger."
 * 0x03 - "%Name1(%Name2) left our world. Diablo's minions weaken."
 * 0x04 - "%Name1 is not in the game."
 * 0x05 - "%Name1 is not logged in."
 * 
 * 0x06 - "%Name1 was Slain by %Name2"
 *    BYTE Param2 = Unit Type of Slayer (0x00 = Player, 0x01 = NPC)
 *        if Type = Player, %Name2 = Slayer Character Name & DWORD Param1 = Slayer Character Type
 *        if Type = NPC, DWORD Param1 = Monster Id Code from MPQ (points to string for %Name2)
 *        if Type = NPC & Monster is Unique, %Name2 = Unique Monster Id
 * 
 * 0x07 - Player Relations (Bottom Left Text)
 *    DWORD Param1 = Player Id
 *    Player Id = Pointer to Character Name
 *    BYTE Param2 = Action Taken
 *       Actions:
 *       0x01 - "%Player permits you to loot his corpse."
 *       0x02 - "%Player permits you to loot her corpse."
 *       0x03 - "%Player has declared hostility towards you."
 *       0x04 - "%Player is no longer hostile towards you."
 *       0x05 - "%Player invites you to ally against the forces of evil."
 *       0x06 - "%Player has canceled party invite."
 *       0x07 - "%Player has joined your party to fight the forces of evil."
 *       0x08 - "You are now allied with %Player."
 *       0x09 - "%Player has left your party."
 *       0x0a - "%Player no longer allows you to access his corpse."
 *       0x0b - "%Player no longer allows you to access her corpse."
 * 
 * 0x08 - "%Name1 is busy."
 * 0x09 - "You must wait a short time to trade with that player."
 * 
 * 0x0a - "%Name1 has items in his box."
 *    if %Name1 = 0x00, "You have items in your box."
 * 
 * 0x0b - Guild master kicked, from an unused guild feature (the English text is blank). No known server sends it.
 * 0x0c - Not handled by the game client; meaning unknown. No known server sends it.
 * 0x0d - "%Name1 is not listening to you."
 * 0x0e - Received on 'Not enough mana' speech.
 * 0x0f - "Realm going down in %Param1 minutes."
 * 0x10 - "You must wait a short time to declare hostility with that player."
 * 0x11 - "%Param1 Stones of Jordan Sold to Merchants"
 * 0x12 - "Diablo Walks the Earth"
 *
 * A listener whose result can block its event (keyupblocker, keydownblocker,
 * chatmsgblocker, chatinputblocker, whispermsgblocker, gamepacket,
 * gamepacketsent, realmpacket, and a hook's click handler) runs while the game,
 * or the script that sent the packet, may be waiting for its answer. A call in
 * it that needs a ready game returns its "Game not ready" result at once while
 * the game is busy (an area or act loading, or the moment the player moves
 * between rooms), rather than waiting. A gamepacketsent listener for a packet
 * its own script sent runs inside that send instead; its calls wait as usual
 * unless the send was made while the script holds the game lock or from inside
 * a listener whose result can block its event, when they get "Game not ready"
 * at once.
 *
 * \param eventHandler A function with the signature listed above that handles
 * the event.
 *
 * Throws "Invalid arguments" if fewer than two arguments are passed.
 */
void addEventListener(String event, Function eventHandler);

/** Removes a previously added event from the listener list.
 *
 * \ingroup globalFunctions
 *
 * \param event The event name (the one used to add the event).
 *
 * \param eventHandler The handler to remove.
 *
 * Throws "Invalid arguments" if fewer than two arguments are passed.
 */
void removeEventListener(String event, Function eventHandler);

/** Clear the event handler list for given event.
 *
 * \ingroup globalFunctions
 *
 * \param event The name of the event to clear the handler list for.
 *
 * Throws "Invalid arguments" if no event is passed.
 */
void clearEvent(String event);

/** Clear all event handlers (from all events). Timers keep running: they stop with clearTimeout(), clearInterval(),
 * clearAllTimers() or the end of their script.
 *
 * \ingroup globalFunctions
 */
void clearAllEvents();

/** Call a function once, after a delay. An error the callback throws is reported when it happens, as one in main()
 * is.
 *
 * \ingroup globalFunctions
 *
 * \param callback The function to call.
 *
 * \param delay The delay in milliseconds; a fraction is cut to whole milliseconds, and a negative or NaN delay calls
 * it at once.
 *
 * \param ... Extra arguments, passed on to the callback.
 *
 * \return The timer's id, a whole number from 1 up, for clearTimeout().
 *
 * Throws an error the script can catch when the callback is not a function, when the delay is missing or not a
 * number, or when Windows could not create or set the timer.
 */
int setTimeout(Function callback, double delay, ...);

/** Call a function every period until the timer is cleared or its script ends. A period of 0 (or a negative or NaN
 * one) repeats every millisecond. A call is not started while the previous one is still running (for example inside
 * delay()), and periods missed while the script was busy give one call. An error the callback throws is reported and
 * the interval keeps running; clear it to stop it.
 *
 * \ingroup globalFunctions
 *
 * \param callback The function to call.
 *
 * \param period The period in milliseconds; a fraction is cut to whole milliseconds.
 *
 * \param ... Extra arguments, passed on to the callback.
 *
 * \return The timer's id, a whole number from 1 up, for clearInterval().
 *
 * Throws an error the script can catch when the callback is not a function, when the period is missing or not a
 * number, or when Windows could not create or set the timer.
 */
int setInterval(Function callback, double period, ...);

/** Cancel a timer by id. clearTimeout() and clearInterval() are the same function: either cancels a timer of either
 * kind, and of any script. With no id, undefined or null it does nothing; a fraction is cut to a whole number (1.5
 * cancels timer 1); a number that is no timer's id (0, or a timer already cancelled) cancels nothing; any other
 * value throws an error the script can catch.
 *
 * \ingroup globalFunctions
 *
 * \param id The id setTimeout() or setInterval() returned.
 */
void clearTimeout(int id);

/** Cancel a timer by id; the same function as clearTimeout(), with the same handling of undefined, null, fractions
 * and other values.
 *
 * \ingroup globalFunctions
 *
 * \param id The id setTimeout() or setInterval() returned.
 */
void clearInterval(int id);

/** Cancel every timer of the calling script, including an interval whose callback is running.
 *
 * \ingroup globalFunctions
 *
 * \return How many timers it cancelled.
 */
int clearAllTimers();

/** Get whether or not the JSOPTION_STRICT flag is set.
 *
 * \ingroup globalFunctions
 *
 * \return Whether or not the strict flag is set.
 */
bool js_strict();

/** Set or clear the JSOPTION_STRICT flag.
 *
 * \ingroup globalFunctions
 *
 * \param setStrictFlag The new JSOPTION_STRICT state.
 */
void js_strict(bool setStrictFlag);

/** Get the D2BS version string.
 *
 * \ingroup globalFunctions
 *
 * \return The D2BS version string.
 */
String version();

/** Print the D2BS version string.
 *
 * \ingroup globalFunctions
 */
void version(int a);

/** Send a message to every running script that has a scriptmsg listener, the
 * calling script included. D2BS calls each such script's scriptmsg listeners
 * with copies of the values passed in. The values are copied once, so getters
 * on them run once however many scripts listen, and not at all when none does.
 * When a script listens, a value that can't be copied (a function, or a game
 * object such as a Unit) throws an error and no script gets the message. A
 * paused script, or one that has not started yet, does not get it;
 * script.send() reaches those.
 *
 * \ingroup globalFunctions
 */
void scriptBroadcast(...);

/** Get the sqlite version string (from sqlite3_version).
 *
 * \ingroup globalFunctions
 *
 * \return The sqlite version string.
 */
String sqlite_version();

/** Return the number of bytes of memory SQLite currently holds.
 *
 * \ingroup globalFunctions
 *
 * \return The memory SQLite holds, in bytes (0 before SQLite is first used).
 */
double sqlite_memusage();

/** Get the Folder object for the directory scriptPath\\name, creating the folder when it is missing (its parent
 * must exist).
 *
 * \ingroup globalFunctions
 *
 * \param name The name of the Folder to open, relative to the script folder. It can't leave the script folder or
 * make a path longer than Windows allows.
 *
 * \return A Folder object representing the Folder name, or undefined when no name, or more than one argument, is
 * passed. A name that isn't a string or isn't a valid folder name throws an error.
 */
Folder dopen(String name);

/** Log the String representation of the arguments.
 *
 * \ingroup globalFunctions
 */
void debugLog(...);

/** Show the console.
 *
 * \ingroup globalFunctions
 */
void showConsole();

/** Hide the console.
 *
 * \ingroup globalFunctions
 */
void hideConsole();

/** Loads the current profile (set by the -profile launch argument, a D2Bot
 * copydata message, or an earlier login(profile)) from d2bs.ini and gets to the
 * lobby (for battle.net chars) or into a game (for single player characters).
 *
 * \ingroup globalFunctions
 *
 * \throw String May throw one of the following:
 *
 * "invalid character name" - If it fails to select the character.
 *
 * "Failed to click the Single button?" - If it fails to click the single player
 * button.
 *
 * "Failed to click the 'Battle.net' button?" - If it fails to click the
 * battle.net button.
 *
 * "Failed to click the 'Other Multiplayer' button?" - If it fails to click the
 * Other Multiplayer button.
 *
 * "Failed to click the 'Open Battle.net' button?" - If it fails to click the
 * Open battle.net button.
 *
 * "Failed to click the 'TCP/IP' button?" - If it fails to click the TCP/IP
 * button.
 *
 * "Failed to click the 'Host Game' button?" - If it fails to click the Host
 * Game (TCP/IP) button.
 *
 * "Failed to click the 'Join Game' button?" - If it fails to click the Join
 * Game (TCP/IP) button.
 *
 * "Failed to click the OK button" - If it fails to click the OK button (TCP/IP
 * enter IP address).
 *
 * "Failed to find the 'Host IP Address' text-edit box." - If it can't find the
 * host IP address (TCP/IP) text box.
 *
 * "Could not get the IP address from the profile in the d2bs.ini file." - If
 * the user failed to specify the IP address in the d2bs.ini file.
 *
 * "Failed to click the exit button?" - If it is login and fails to click the
 * exit button.
 *
 * "Failed to set the 'Username' text-edit box.' - If it can't find the username
 * control.
 *
 * "Failed to set the 'Password' text-edit box." - If it can't find the password
 * control.
 *
 * "Failed to click the 'Log in' button?" - If it can't click the login button.
 *
 * "Failed to click the 'Normal Difficulty' button?" - If it can't click the
 * normal button in single player game creation.
 *
 * "Failed to click the 'Nightmare Difficulty' button?" - If it can't click the
 * nightmare button in single player game creation.
 *
 * "Failed to click the 'Hell Difficulty' button?" - If it can't click the hell
 * button in single player game creation.
 *
 * "Invalid single player difficulty level specified!" - If the difficulty level
 * is invalid.
 *
 * "Unable to connect" - If the unable to connect screen is shown.
 *
 * "CD-Key in use" - If the cdkey in use screen is shown.
 *
 * "Bad account or password" - If the username/password wrong screen is shown.
 *
 * "Realm Down" - If the realm down message is shown.
 *
 * "Unhandled login location" - If the game is in an unsupported location.
 */
void login();

/** Loads the profile from d2bs.ini and gets to the lobby (for battle.net chars)
 * or into a game (for single player characters).
 *
 * \ingroup globalFunctions
 *
 * \param profile The profile to load.
 *
 * \throw String May throw one of the following:
 *
 * "invalid character name" - If it fails to select the character.
 *
 * "Failed to click the Single button?" - If it fails to click the single player
 * button.
 *
 * "Failed to click the 'Battle.net' button?" - If it fails to click the
 * battle.net button.
 *
 * "Failed to click the 'Other Multiplayer' button?" - If it fails to click the
 * Other Multiplayer button.
 *
 * "Failed to click the 'Open Battle.net' button?" - If it fails to click the
 * Open battle.net button.
 *
 * "Failed to click the exit button?" - If it is login and fails to click the
 * exit button.
 *
 * "Failed to set the 'Username' text-edit box.' - If it can't find the username
 * control.
 *
 * "Failed to set the 'Password' text-edit box." - If it can't find the password
 * control.
 *
 * "Failed to click the 'Log in' button?" - If it can't click the login button.
 *
 * "Failed to click the 'Normal Difficulty' button?" - If it can't click the
 * normal button in single player game creation.
 *
 * "Failed to click the 'Nightmare Difficulty' button?" - If it can't click the
 * nightmare button in single player game creation.
 *
 * "Failed to click the 'Hell Difficulty' button?" - If it can't click the hell
 * button in single player game creation.
 *
 * "Invalid single player difficulty level specified!" - If the difficulty level
 * is invalid.
 *
 * "Unable to connect" - If the unable to connect screen is shown.
 *
 * "CD-Key in use" - If the cdkey in use screen is shown.
 *
 * "Bad account or password" - If the username/password wrong screen is shown.
 *
 * "Realm Down" - If the realm down message is shown.
 *
 * "Unhandled login location" - If the game is in an unsupported location.
 */
void login(String profile);

/** Select the character from the given profile. Keeps all details except the
 * character name from old profile.
 *
 * \ingroup globalFunctions
 *
 * \param profile The profile to get the character name from.
 *
 * \return Whether or not character swap was successful.
 */
bool selectCharacter(String profile);

/** Get every character on the character select screen in one call, in the order the screen lists them.
 *
 * Only the expansion character select screen is supported. On a realm the list holds the characters the realm sent:
 * when an account has more, the screen's "more characters" row is left out.
 *
 * \ingroup globalFunctions
 *
 * \return false when getLocation() does not report 12 (only the expansion character select screen does, and a popup
 * such as the difficulty choice changes the location), a realm is still sending its list, or D2BS is still starting
 * up. A screen with no characters reports 42, as it can while a list is still loading. Otherwise an array with one
 * object per character: name, realm (empty for single player), classid (0 amazon to 6 assassin), level, hardcore,
 * expansion, ladder, dead (a hardcore character that died and can't be played) and expires (the realm expiry as Unix
 * time in seconds, 0 when there is none, as in single player).
 */
Object[] getCharacterList();

/** Select a character on the character select screen by name and scroll the list to show it.
 *
 * Selecting does not log in: click OK or press Enter after it returns. A character slot click or arrow key in between
 * changes the selection again. Only the expansion character select screen is supported.
 *
 * \ingroup globalFunctions
 *
 * \param name The character name, not case sensitive. Anything but a string throws.
 *
 * \return true when the character is selected; false when it is not in the list, getLocation() does not report 12, a
 * realm is still sending its list, a character is being deleted, a popup such as the delete confirmation or the
 * gateway list is open, or D2BS is still starting up.
 */
bool focusCharacter(String name);

/** Create a game with the given name and password in the given difficulty.
 *
 * \ingroup globalFunctions
 *
 * \param name The game name.
 *
 * \param pass The game password.
 *
 * \param diff The game difficulty. 0 - normal, 1 - nightmare, 2 - hell, 3 - 
 * hardest difficulty available.
 */
void createGame(String name, String pass = "", int32_t diff = 3);

/** Join a game with the given name and password.
 *
 * \ingroup globalFunctions
 *
 * \param name The game name.
 *
 * \param pass The game password.
 */
void joinGame(String name, String pass = "");

/** Create a profile with the given profile name, mode, gateway, username,
 * password, character name and single player difficulty.
 *
 * \ingroup globalFunctions
 *
 * \param profile The profile name.
 *
 * \param mode The mode (single player/battle.net/etc).
 *
 * \param gateway The realm gateway.
 *
 * \param username Account username.
 *
 * \param password Account password.
 *
 * \param charname Character name (case sensitive).
 *
 * \param spDifficulty The difficulty to create games for a single player
 * character.
 */
void addProfile(String profile, String mode, String gateway, String username,
	String password, String charname, int spDifficulty = 3);

/** Get the current OOG location.
 *
 * D2BS detects the out-of-game screen by the buttons and text boxes it shows.
 * The values (0 to 44) are listed in \ref reference_locations. A popup the
 * detection knows reports the popup; an unknown popup can report the screen
 * under it, or 0.
 *
 * \ingroup globalFunctions
 *
 * \return The current OOG location, 0 when on the menus but no known screen
 * matched, or null while not on the out-of-game menus: in a game, while a game
 * loads, and in the moments between screens when no controls exist.
 */
int getLocation();

/** Load an mpq file.
 *
 * \ingroup globalFunctions
 *
 * \param path The mpq file to load
 *
 * Throws if path is not a string.
 */
void loadMpq(String path);

/** Submit the item on the cursor to the open screen (like the add sockets
 * screen).
 *
 * \ingroup globalFunctions
 *
 * \return Whether or not it was successful.
 */
bool submitItem();

/** Get the mouse coordinates (in screen or map space) and return as an array.
 *
 * \ingroup globalFunctions
 *
 * \param nFlag Whether to return in map space (true) or screen space (false).
 *
 * \param nReturn Whether to return as an array or as an Object, false for array
 * format.
 *
 * \return The mouse coordinates in an array. 0 - x, 1 - y.
 */
int[] getMouseCoords(bool nFlag = false, bool nReturn = false);

/** Get the mouse coordinates (in screen or map space) and return as an Object.
 *
 * \ingroup globalFunctions
 *
 * \param nFlag Whether to return in map space (true) or screen space (false).
 *
 * \param nReturn Whether to return as an array or as an Object, true for Object
 * format.
 *
 * \return The mouse coordinates in an array. Has properties .x and .y.
 */
Object getMouseCoords(bool nFlag, bool nReturn);

/** Copy the cached data associated with given unit. This is things like type
 * and unit id.
 *
 * The copy is a new Unit object for the same game unit. It holds other's unit
 * id, type and getUnit/getNext search filters (and, for an item reached through
 * getItem, its owner). It is not a snapshot: like the original, it reads live
 * game data on every access and returns false or undefined once the unit has
 * left the game. The difference is that getNext() moves only the object it is
 * called on, so a copy stays on its unit while the original keeps iterating.
 *
 * Use it to keep the units found in a getUnit/getNext loop, for example
 * list.push(copyUnit(unit)). Pushing unit itself stores the same object every
 * time, so every entry ends up on the unit the loop finished on.
 *
 * \ingroup globalFunctions
 *
 * \param other The Unit to copy.
 *
 * \return The new copy of the unit, or undefined when other is missing or is
 * not a Unit. Throws 'Couldn't copy unit' only when the new object cannot be
 * created.
 */
Unit copyUnit(Unit other);

/** Click on the map with the given click type, with or without Shift held, at
 * the given unit.
 *
 * nClickType picks the mouse button and phase; a normal click is a down
 * followed by an up:
 *
 * 0 - Left button down
 *
 * 1 - Left button held
 *
 * 2 - Left button up
 *
 * 3 - Right button down
 *
 * 4 - Right button held
 *
 * 5 - Right button up
 *
 * Any other value (a negative one wraps to a large value) does nothing.
 *
 * The click lands on the unit's position and targets that unit, for example to
 * attack it, talk to it or pick it up. The game ignores the click while UI flag
 * 0x09 (the Esc menu) is set.
 *
 * \ingroup globalFunctions
 *
 * \param nClickType The click type to do (see above).
 *
 * \param nShift Whether or not to be holding down shift. 0 - Don't hold shift,
 * any other integer - hold shift. With shift held, D2BS also adds the flag that
 * the game's Always Run setting adds; the game applies it only while the
 * character has stamina.
 *
 * \param toClick The unit to click on.
 *
 * \return true once the click is handed to the game, even when the game ignores
 * it (a click type above 5, the Esc menu open, or a click the game is still
 * handling). false when fewer than three arguments are given, the arguments
 * match neither form (a non-integer number such as 1.5 included), toClick is
 * not a Unit or no longer exists, or the player is not in a game or the game
 * does not become ready in time (with a 'Game not ready' warning). It does not
 * throw.
 */
bool clickMap(uint16_t nClickType, uint16_t nShift, Unit toClick);

/** Click on the map with the given click type, with or without Shift held, at
 * the given unit.
 *
 * nClickType and the click itself work as in the first clickMap overload.
 *
 * \ingroup globalFunctions
 *
 * \param nClickType The click type to do: 0 left down, 1 left held, 2 left up,
 * 3 right down, 4 right held, 5 right up.
 *
 * \param nShift Whether or not to be holding down shift. With shift held, D2BS
 * also adds the flag that the game's Always Run setting adds; the game applies
 * it only while the character has stamina.
 *
 * \param toClick The unit to click on.
 *
 * \return true once the click is handed to the game, even when the game ignores
 * it. false when the arguments are invalid, toClick is not a Unit or no longer
 * exists, or the player is not in a game (with a 'Game not ready' warning). It
 * does not throw.
 */
bool clickMap(uint16_t nClickType, bool nShift, Unit toClick);

/** Click on the map with the given click type, with or without Shift held, at
 * the given point.
 *
 * nClickType works as in the first clickMap overload. nX and nY are game map
 * coordinates (the same as unit.x and unit.y), not screen pixels, and the point
 * does not need to be on screen. Use the Unit form to interact with a unit.
 *
 * \ingroup globalFunctions
 *
 * \param nClickType The click type to do: 0 left down, 1 left held, 2 left up,
 * 3 right down, 4 right held, 5 right up.
 *
 * \param nShift Whether or not to be holding down shift. 0 - Don't hold shift,
 * any other integer - hold shift. With shift held, D2BS also adds the flag that
 * the game's Always Run setting adds; the game applies it only while the
 * character has stamina.
 *
 * \param nX The x coordinate to click at (map coordinates; negative values
 * wrap).
 *
 * \param nY The y coordinate to click at (map coordinates; negative values
 * wrap).
 *
 * \return true once the click is handed to the game, even when the game ignores
 * it (a click type above 5, the Esc menu open, or a click the game is still
 * handling). false when fewer than three arguments are given, the arguments
 * match neither form (a non-integer number such as 1.5 included), or the player
 * is not in a game or the game does not become ready in time (with a 'Game not
 * ready' warning). It does not throw.
 */
bool clickMap(uint16_t nClickType, uint16_t nShift, uint16_t nX, uint16_t nY);

/** Click on the map with the given click type, with or without Shift held, at
 * the given point.
 *
 * nClickType works as in the first clickMap overload, and nX and nY as in the
 * third.
 *
 * \ingroup globalFunctions
 *
 * \param nClickType The click type to do: 0 left down, 1 left held, 2 left up,
 * 3 right down, 4 right held, 5 right up.
 *
 * \param nShift Whether or not to be holding down shift. With shift held, D2BS
 * also adds the flag that the game's Always Run setting adds; the game applies
 * it only while the character has stamina.
 *
 * \param nX The x coordinate to click at (map coordinates).
 *
 * \param nY The y coordinate to click at (map coordinates).
 *
 * \return true once the click is handed to the game, even when the game ignores
 * it. false when the arguments are invalid or the player is not in a game (with
 * a 'Game not ready' warning). It does not throw.
 */
bool clickMap(uint16_t nClickType, bool nShift, uint16_t nX, uint16_t nY);

/** Read the state of the player-trade Accept button, or press it.
 *
 * action is converted to a number first, so "2" counts as 2 and true as 1. A
 * value that converts to NaN (a non-numeric string, undefined), 0, or any
 * number other than 1, 2 and 3 presses the button exactly as acceptTrade()
 * does.
 *
 * \ingroup globalFunctions
 *
 * \param action What action to perform.
 *
 * 1 - Get whether you have pressed Accept (the trade screen's own Accept flag).
 *
 * 2 - Get the trade state number (the same value as getTradeInfo(0)).
 *
 * 3 - Get whether the accept check is red (the server has disabled the Accept
 * button).
 *
 * \return The value specified by action: true or false for 1 and 3, a number
 * for 2. Outside a ready game it returns false (with a 'Game not ready'
 * warning). A value whose conversion throws ends the call with that exception.
 */
bool acceptTrade(int action);

/** Press your Accept button in a player trade: accept the trade, or withdraw
 * your acceptance if you have already accepted.
 *
 * It acts like a click on the trade screen's Accept button. When acceptTrade(1)
 * is false it accepts (getTradeInfo(0) becomes 7). Otherwise it withdraws your
 * acceptance: the state returns to 3 and the trade window stays open; it does
 * not cancel the trade.
 *
 * It acts only when getTradeInfo(0) is 3, 5 or 7. State 7 is also the
 * 30-second pause after a trade request was declined or cancelled, with no
 * trade window open. A call in that pause sends an accept (or an un-accept, if
 * acceptTrade(1) is still true) with no trade open; the state then stays at 7
 * (or 3) instead of returning to 0, so the game keeps declining new trade
 * requests until the server ends the trade. Check that the trade window is open before calling it.
 *
 * \ingroup globalFunctions
 *
 * \return true when the button was pressed or released; false, with nothing
 * done, while the accept check is red (acceptTrade(3) is true). Throws
 * "Invalid parameter passed to acceptTrade!" when getTradeInfo(0) is not 3, 5
 * or 7, although the call has no parameter at fault. Outside a ready game it
 * returns false (with a 'Game not ready' warning).
 */
bool acceptTrade();

/** Hit the OK button to open the trade window.
 *
 * Throws an error if client not in a proper state to click ok.
 *
 * \ingroup globalFunctions
 */
void tradeOk();

/** Get the lines of dialog of whatever you're talking to.
 *
 * Each object has a text property which gives the text and a selectable
 * property which is a boolean as to whether you can click on that line or not.
 *
 * \ingroup globalFunctions
 *
 * \return An array of objects of the lines, or undefined if no dialog up.
 */
object[] getDialogLines();

/** Beep with beep ID nBeepId.
 *
 * \ingroup globalFunctions
 *
 * \param nBeepId ID of the beep to make. See:
 * http://msdn.microsoft.com/en-us/library/ms680356%28VS.85%29.aspx .
 */
void beep(int nBeepId = 0);

/** Click an item.
 *
 * \ingroup globalFunctions
 *
 * \param item The item to click on.
 */
void clickItem(Unit item);

/** Click on a body location with a certain click type.
 *
 * \ingroup globalFunctions
 *
 * \param nClickType The click type. {0 - left click, 4 - left click merc body locations}
 *
 * \param nBodyLoc The body location.
 *
 * 0 - Not equipped
 *
 * 1 - Helmet
 *
 * 2 - Amulet
 *
 * 3 - Armor
 *
 * 4 - Right hand slot 1
 *
 * 5 - Left hand slot 1
 *
 * 6 - Right ring
 *
 * 7 - Left ring
 *
 * 8 - Belt
 *
 * 9 - Boots
 *
 * 10 - Gloves
 *
 * 11 - Right hand slot 2
 *
 * 12 - Left hand slot 2
 */
void clickItem(int nClickType, int nBodyLoc);

/** Click on a given item with the given click type.
 *
 * \ingroup globalFunctions
 *
 * \param nClickType The click type. {0 - left click, 1 - right click,
 * 	4 - left click merc location}
 *
 * \param item The item to click.
 *
 * \param nClickType
 */
void clickItem(int nClickType, Unit item);

/** Click an item location with the given click type.
 *
 * \ingroup globalFunctions
 *
 * \param nClickType The click type. {0 - left click, 1 - right click,
 * 	2 - shift left click, 4 - left click merc location}
 *
 * \param nX The x coordinate of the location to click.
 *
 * \param nY The y coordinate of the location to click.
 *
 * \param nLoc The game location of the item.
 *
 * 2 - Belt
 *
 * 3 - Inventory
 *
 * 4 - Player trade
 *
 * 6 - Cube
 *
 * 7 - Stash
 */
void clickItem(int nClickType, int nX, int nY, int nLoc);

/** Get the euclidean distance from me to a.
 *
 * \ingroup globalFunctions
 *
 * \param a The second Unit.
 *
 * \return The euclidean distance from a.
 */
double getDistance(Unit a);

/** Get the euclidean distance from a to b.
 *
 * \ingroup globalFunctions
 *
 * \param a The first Unit.
 *
 * \param b The second Unit.
 *
 * \return The euclidean distance from a to b.
 */
double getDistance(Unit a, Unit b);

/** Get the euclidean distance from a to b.
 *
 * \ingroup globalFunctions
 *
 * \param a The first Unit, or any object with x and y.
 *
 * \param bx The x coordinate of point b.
 *
 * \param by The y coordinate of point b.
 *
 * \return The euclidean distance from a to b, or undefined when a Unit argument has left the game.
 */
double getDistance(Unit a, int32_t bx, int32_t by);

/** Get the euclidean distance from me to a.
 *
 * \ingroup globalFunctions
 *
 * \param ax The x coordinate of point a.
 *
 * \param ay The y coordinate of point a.
 *
 * \return The euclidean distance from me to a.
 */
double getDistance(int32_t ax, int32_t ay);

/** Get the euclidean distance from a to b.
 *
 * \ingroup globalFunctions
 *
 * \param ax The x coordinate of point a.
 *
 * \param ay The y coordinate of point a.
 *
 * \param b The second Unit, or any object with x and y.
 *
 * \return The euclidean distance from a to b, or undefined when a Unit argument has left the game.
 */
double getDistance(int32_t ax, int32_t ay, Unit b);

/** Get the euclidean distance from a to b.
 *
 * \ingroup globalFunctions
 *
 * \param ax The x coordinate of point a.
 *
 * \param ay The y coordinate of point a.
 *
 * \param bx The x coordinate of point b.
 *
 * \param by The y coordinate of point b.
 *
 * \return The euclidean distance from a to b.
 */
double getDistance(int32_t ax, int32_t ay, int32_t bx, int32_t by);

/** Drop, offer in a trade, deposit or withdraw gold.
 *
 * nMode picks what happens to nGold:
 *
 * | Mode | Action |
 * |---|---|
 * | 0, 1 | Drop nGold gold on the ground (1 is the default). |
 * | 2 | Set the gold you offer in the trade window, capped at the gold you carry; a negative nGold offers all of it. If the other player has already accepted, the offer is not changed: the call sends the same request as un-pressing Accept instead, and the trade state returns to 3. |
 * | 3 | Move nGold from your inventory into the stash (with the stash open). |
 * | 4 | Move nGold from the stash into your inventory (with the stash open). |
 *
 * Any other mode does nothing, and an amount of 0 does nothing except in mode
 * 2. The call checks neither that a trade or the stash is open nor the amount
 * in modes 0, 3 and 4. It first closes the game's own gold dialog; if one was
 * open, the amount typed in it is used instead of nGold.
 *
 * It returns undefined; outside a ready game it returns false (with a 'Game not
 * ready' warning).
 *
 * \ingroup globalFunctions
 *
 * \param nGold The amount of gold to move around. Must be an integer in the
 * 32-bit range; any other value (1.5, 1e10, the string "100") is replaced by
 * 0.
 *
 * \param nMode What to do with the gold (see the table above). Must be an
 * integer; any other value is replaced by 1.
 */
void gold(int nGold = 0, int nMode = 1);

/** Check if two units collide.
 *
 * \ingroup globalFunctions
 *
 * \param a Unit a.
 *
 * \param b Unit b.
 *
 * \param nBitMask Bit mask to check collision of a and b with.
 *
 * \return The collision result for nBitMask, or nBitMask itself (reads as
 * blocked) when either unit has left the game; undefined for wrong arguments.
 */
int checkCollision(Unit a, Unit b, int nBitMask);

/** Play d2 sound by id.
 *
 * \ingroup globalFunctions
 *
 * \return true
 */
bool playSound(int nSoundId);

/** Quit the game.
 *
 * \ingroup globalFunctions
 */
void quit();

/** Quit Diablo II. Allows the core to shutdown, than terminates the process.
 *
 * \ingroup globalFunctions
 */
void quitGame();

/** Say the string equivalent of the each of the arguments.
 *
 * \ingroup globalFunctions
 *
 * \return true if at least one parameter was sent in.
 */
bool say(...);

/** Click one of the buttons in the party screen for the given player.
 *
 * \ingroup globalFunctions
 *
 * \param player The player who's button you wish to click.
 *
 * \param nMode Which button to click.
 *
 * 0 - Allow loot corpse
 *
 * 1 - Hostile player
 *
 * 2 - Join party
 *
 * 3 - Leave party (any player still in the game can be passed in)
 *
 * \return true if the button was clicked; false if it doesn't apply, or once
 * the player has left or the game has ended.
 */
bool clickParty(Party player, int nMode);

/** Get which weapon switch is being used.
 *
 * \ingroup globalFunctions
 *
 * \param dummy Any int other than 0.
 *
 * \return Which weapon switch is active.
 */
int weaponSwitch(int32_t dummy);

/** Switch the weapons.
 *
 * \ingroup globalFunctions
 *
 * \param dummy 0 to use this mode.
 *
 * \return true
 */
bool weaponSwitch(int32_t dummy = 0);

/** Hit the transmute button.
 *
 * \ingroup globalFunctions
 */
void transmute();

/** Use a stat point.
 *
 * The points are spent one at a time, half a second apart, and the script
 * waits meanwhile. If you have fewer unspent stat points than count, nothing is
 * spent; count 0 (or a negative count) spends nothing.
 *
 * BE CAREFUL! The call does not check that statType is valid. statType is
 * converted to a 16-bit number, so a non-numeric value such as "dexterity"
 * becomes 0 (strength) and -1 becomes 65535. The game's own stat button uses
 * the high byte of this value to spend extra points in one packet, so a value
 * above 255 asks for more points than count. Use only 0 to 3; an invalid stat
 * might get you flagged/banned.
 *
 * It returns undefined; outside a ready game it returns false (with a 'Game not
 * ready' warning).
 *
 * \ingroup globalFunctions
 *
 * \param statType The type of stat to add the points to: 0 - strength, 1 -
 * energy, 2 - dexterity, 3 - vitality (the same ids getStat uses).
 *
 * \param count The number of points to add
 */
void useStatPoint(uint16_t statType, uint32_t count = 1);

/** Use a skill point
 *
 * The points are spent one at a time, half a second apart, and the script
 * waits meanwhile. If you have fewer unspent skill points than count, nothing
 * is spent; count 0 (or a negative count) spends nothing.
 *
 * BE CAREFUL! The call does not check that your character can learn the skill.
 * The game's own skill tree checks the required level and the prerequisite
 * skills before it spends a point; this call skips those checks.
 * skill is converted to a 16-bit number, so a non-numeric value becomes 0
 * (Attack). An invalid skill might get you flagged/banned.
 *
 * It returns undefined; outside a ready game it returns false (with a 'Game not
 * ready' warning).
 *
 * \ingroup globalFunctions
 *
 * \param skill The skill to add the points to: a skill id, the same number
 * getSkill uses. See \ref reference_skills. Only class skills can be learned:
 * Amazon 6-35, Sorceress 36-65, Necromancer 66-95, Paladin 96-125, Barbarian
 * 126-155, Druid 221-250, Assassin 251-280, each only by its own class.
 *
 * \param count The number points to spend
 */
void useSkillPoint(uint16_t skill, uint32_t count = 1);

/** Take a screenshot
 *
 * Performs the action that pressing print screen (by default) would do
 *
 * \ingroup globalFunctions
 */
void takeScreenshot();

/** Move NPC to a new location.
 *
 * \warning This is an exploit function.
 *
 * \param npc NPC to move.
 *
 * \param x X coord to move to.
 *
 * \param y Y coord to move to.
 *
 * \return true once the move request is sent; false with a warning when EnableUnsupported is off or the game
 * is not ready. Throws if fewer than three arguments are passed or npc is not a monster Unit. x and y are converted
 * like any JS number.
 */
bool moveNPC(Unit npc, uint32_t x, uint32_t y);

/** Pass packets to the client as if the server had sent them.
 * Takes either one ArrayBuffer holding one or more whole packets, or pairs of arguments: size, value, size, value,
 * ... where each size (1 to 4) is the number of bytes its value (an integer) adds to the packet, little endian.
 * Every packet must be one the game can read, 1 to 516 bytes long and complete. An empty ArrayBuffer is refused.
 * Pass a typed array's `.buffer`, not the typed array; the whole buffer is used, so for a typed array that covers
 * only part of its buffer pass `view.buffer.slice(view.byteOffset, view.byteOffset + view.byteLength)`. The packets
 * are checked together: if one fails, none is passed on.
 *
 * \warning This is an exploit function.
 *
 * \return true when the packets were passed to the game's incoming packet queue; false outside a game, and false
 * with a warning when the input is refused. A value that throws while it is converted stops the call with its error.
 */
bool getPacket(...);

/** Send a packet to the server.
 * Takes either one ArrayBuffer holding the packet, or pairs of arguments: size, value, size, value, ... where each
 * size (1 to 4) is the number of bytes its value (an integer) adds to the packet, little endian. The packet must be
 * 1 to 516 bytes long. Pass a typed array's `.buffer`, not the typed array; the whole buffer is used, so for a typed
 * array that covers only part of its buffer pass
 * `view.buffer.slice(view.byteOffset, view.byteOffset + view.byteLength)`.
 *
 * \warning This is an exploit function.
 *
 * \return true when the game took the packet; false when it did not (outside a game, for example, or a
 * `gamepacketsent` listener blocked it, with no warning), and false with a warning when the input is refused.
 * A value that throws while it is converted stops the call with its error.
 */
bool sendPacket(...);

/** Get the external IP by going to
 * "http://automation.whatismyip.com/n09230945.asp" and parsing the result.
 */
String getIP();

/** Convert a point from screen coordinates to automap coordinates.
 *
 * \ingroup globalFunctions
 *
 * \param arg An object with x and y parameters specifying the point.
 *
 * \return A new object with x and y, containing the automap coordinate. The argument is not changed.
 */
Object screenToAutomap(Object arg);

/** Convert a point from screen coordinates to automap coordinates.
 *
 * \ingroup globalFunctions
 *
 * \param ix The x coordinate.
 *
 * \param iy The y coordinate.
 *
 * \return An object with x and y, containing the automap coordinate.
 */
Object screenToAutomap(int ix, int iy);

/** Convert a point from automap coordinates to screen coordinates.
 *
 * \ingroup globalFunctions
 *
 * \param arg An object with x and y parameters specifying the point.
 *
 * \return A new object with x and y, containing the screen coordinate. The argument is not changed.
 */
Object automapToScreen(Object arg);

/** Convert a point from automap coordinates to screen coordinates.
 *
 * \ingroup globalFunctions
 *
 * \param ix The x coordinate.
 *
 * \param iy The y coordinate.
 *
 * \return An object with x and y, containing the screen coordinate.
 */
Object automapToScreen(int ix, int iy);

/** Takes the md5 hash.
 *
 * \ingroup globalFunctions
 *
 * \param str The string to hash.
 *
 * \return The md5 hash of the string.
 */
String md5(String str);

/** Takes the sha1 hash.
 *
 * \ingroup globalFunctions
 *
 * \param str The string to hash.
 *
 * \return The sha1 hash of the string.
 */
String sha1(String str);

/** Takes the sha256 hash.
 *
 * \ingroup globalFunctions
 *
 * \param str The string to hash.
 *
 * \return The sha256 hash of the string.
 */
String sha256(String str);

/** Takes the sha384 hash.
 *
 * \ingroup globalFunctions
 *
 * \param str The string to hash.
 *
 * \return The sha384 hash of the string.
 */
String sha384(String str);

/** Takes the sha512 hash.
 *
 * \ingroup globalFunctions
 *
 * \param str The string to hash.
 *
 * \return The sha512 hash of the string.
 */
String sha512(String str);

/** Take the md5 hash of a file.
 *
 * \ingroup globalFunctions
 *
 * \param file Filename relative to script path.
 *
 * \return The md5 hash of the file, or null when the file can't be read.
 */
String md5_file(String file);

/** Take the sha1 hash of a file.
 *
 * \ingroup globalFunctions
 *
 * \param file Filename relative to script path.
 *
 * \return The sha1 hash of the file, or null when the file can't be read.
 */
String sha1_file(String file);

/** Take the sha256 hash of a file.
 *
 * \ingroup globalFunctions
 *
 * \param file Filename relative to script path.
 *
 * \return The sha256 hash of the file, or null when the file can't be read.
 */
String sha256_file(String file);

/** Take the sha384 hash of a file.
 *
 * \ingroup globalFunctions
 *
 * \param file Filename relative to script path.
 *
 * \return The sha384 hash of the file, or null when the file can't be read.
 */
String sha384_file(String file);

/** Take the sha512 hash of a file.
 *
 * \ingroup globalFunctions
 *
 * \param file Filename relative to script path.
 *
 * \return The sha512 hash of the file, or null when the file can't be read.
 */
String sha512_file(String file);
