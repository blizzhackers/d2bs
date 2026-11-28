/** The class that represents the controlled unit. Technically, also called
 * Unit, but since C++ doesn't support class overloading, called it Me here
 * instead.
 */
class Me
{
public:
	/** Get the next unit that matches the originally given name or class id and
	 * originally specified mode.
	 *
	 * \return Whether another unit was found.
	 */
	bool getNext();

	/** Get the next unit that matches the given name and originally specified
	 * mode.
	 *
	 * \param szName The name to look for.
	 *
	 * \return Whether another unit was found.
	 */
	bool getNext(String szName);

	/** Get the next unit that matches the given class id and originally 
	 * specified mode.
	 *
	 * \param dwClassId The class id to look for.
	 *
	 * \return Whether another unit was found.
	 */
	bool getNext(int dwClassId);

	/** Get the next unit that matches the given name and mode.
	 *
	 * \param szName The name to look for.
	 *
	 * \param dwMode Either the mode being searched for, or a bitmask with bit
	 * 29 set and bits 0-28 corresponding to modes of units being searched for.
	 *
	 * \return Whether another unit was found.
	 */
	bool getNext(String szName, int dwMode);

	/** Get the next unit that matches the given class id and mode.
	 *
	 * \param dwClassId The class id to look for.
	 *
	 * \param dwMode Either the mode being searched for, or a bitmask with bit
	 * 29 set and bits 0-28 corresponding to modes of units being searched for.
	 *
	 * \return Whether another unit was found.
	 */
	bool getNext(int dwClassId, int dwMode);

	/** Cancel some form of interaction.
	 *
	 * If there is scrolling text, cancel it.
	 * If you are interacting with an NPC, stop interacting.
	 * If you have an item on your cursor, drop it.
	 * Otherwise call D2CLIENT_CloseInteract.
	 */
	void cancel();

	/** Try to repair.
	 *
	 * Need to be able to find unit that you're trying to repair with. That
	 * means the unit needs to still be in the server unit hash table.
	 *
	 * \return true if successful.
	 */
	bool repair();

	/** Use an NPC menu.
	 *
	 * Need to be able to find unit that you're trying to repair with. That
	 * means the unit needs to still be in the server unit hash table.
	 *
	 * \param menuItem Index of the menu item to click.
	 * 
	 * \return true if menu was found.
	 */
	bool useMenu(int menuItem);

	/** Interact with the unit.
	 *
	 * If the unit is an item in your inventory or stash, the call uses it as a
	 * right-click does: a potion is drunk, a town portal tome or scroll opens a
	 * portal outside town, and an identify tome or scroll gives the identify
	 * cursor. An item in the belt is used the same way. Either use returns
	 * false, whether or not the game acts on it. Any other unit, an item
	 * anywhere else included, gets a left-button-down click on its map
	 * position that targets it, the same kind of click clickMap(0, 0, unit)
	 * makes, and the call returns true. Called on your own character, or on a
	 * unit that has left the game, it does nothing and returns false. Outside
	 * a ready game it returns false (with a 'Game not ready' warning).
	 */
	bool interact();

	/** Interact with a waypoint.
	 *
	 * If the unit is an object, assume it's a waypoint, and take it.
	 *
	 * \param destId The area id of the destination to select.
	 */
	bool interact(int destId);

	/** Get an item from inventory by name, mode and nUnitId.
	 *
	 * \param name The name of the unit to look for.
	 *
	 * \param mode Either the mode being searched for, or a bitmask with bit
	 * 29 set and bits 0-28 corresponding to modes of units being searched
	 * for.
	 *
	 * \param nUnitId The unique id of the unit.
	 *
	 * \return The first item found that matches the description.
	 */
	Unit getItem(String name, uint32_t mode = (uint32_t) -1,
		uint32_t nUnitId = (uint32_t)-1);

	/** Get an item from inventory by classId, mode and nUnitId.
	 *
	 * \param classId The class id of the unit.
	 *
	 * \param mode Either the mode being searched for, or a bitmask with bit
	 * 29 set and bits 0-28 corresponding to modes of units being searched
	 * for.
	 *
	 * \param nUnitId The unique id of the unit.
	 *
	 * \return The first item found that matches the description.
	 */
	Unit getItem(uint32_t classId = (uint32_t)-1, uint32_t mode = (uint32_t)-1,
		uint32_t nUnitId = (uint32_t)-1);

	/** Get all items from inventory.
	 *
	 * \return An array of all the items from inventory; empty when the unit has no items, has left the game, or the method is called on something that is not a unit.
	 */
	Unit[] getItems();

	/** Get a player's merc.
	 *
	 * The unit being operated on should be a player.
	 *
	 * \return The merc unit, or null when none is found (the merc must be in a
	 * room the client has loaded) or the unit is not a player. Any argument is
	 * ignored: getMerc(true) returns the same unit or null, so test the result
	 * for truthiness. Outside a ready game it returns false (with a 'Game not
	 * ready' warning).
	 */
	Unit getMerc();

	/** Get the merc's HP.
	 *
	 * The reported HP is a percentage.
	 *
	 * \return The merc's HP percentage, 0 while the merc is dead, undefined when there is no merc, or false when the game is not ready.
	 */
	int getMercHP();

	/** Get skill name from hand.
	 *
	 * \param hand Hand to get the skill name off.
	 *
	 * 0 - Right hand
	 *
	 * 1 - Left hand
	 *
	 * \return The skill's in-game name in the client's language. It is not
	 * always a name setSkill() accepts; getSkill(2) and getSkill(3) give the
	 * ids. Returns false when the unit has left the game.
	 */
	String getSkill(int hand);

	/** Get skill id from hand.
	 *
	 * \param hand Hand to get the skill id off.
	 *
	 * 2 - Right hand
	 *
	 * 3 - Left hand
	 *
	 * \return Skill id.
	 */
	int getSkill(int hand);

	/** Get all skills.
	 *
	 * Gets all skill ids, along with corresponding base and total skill
	 * levels.
	 *
	 * \param dummy Should be 4.
	 *
	 * \return An array of arrays of integers. Inside the inner array the
	 * 0th index is the skill id, the 1st index is the base skill level and
	 * the 2nd index is the total skill level.
	 */
	int[][] getSkill(int dummy);

	/** Get the base or total level of a skill.
	 *
	 * With two arguments, only the unit's own (non-charged) entry for the
	 * skill is read. The total adds the bonuses the game applies to that
	 * skill: +all skills, +class skills, +skill tab, +single skill, +elemental
	 * skills, oskills and temporary bonuses such as the Skill shrine. Oskills
	 * add at most 3 to a player's own class skills. Skills that come from item
	 * charges get no bonuses. The result, base or total, is never below 0 and
	 * never above 99.
	 *
	 * Pass a boolean third argument to choose the entry: true reads the skill
	 * from an item with charges (the charge level), false reads the first
	 * entry with that id, charged or not.
	 *
	 * \param nSkillId The skill id. See \ref reference_skills.
	 *
	 * \param nExt 0 for the base level (the hard points put into the skill),
	 * any other integer (usually 1) for the total level.
	 *
	 * \returns The skill level. Returns false when the unit does not have the
	 * skill, when nSkillId or nExt is not an integer, when a third argument is
	 * passed that is not a boolean, or when the unit has left the game.
	 * Outside a ready game it returns false (with a 'Game not ready'
	 * warning).
	 */
	int getSkill(int nSkillId, int nExt);

	/** Gets the parent of a unit.
	 *
	 * This function is used for monster and item units.
	 *
	 * \return The parent unit.
	 */
	Unit getParent();

	/** Gets the parent of a unit.
	 *
	 * This function is used for object units.
	 *
	 * \return The parent's name.
	 */
	String getParent();

	/** Puts the string equivalent of message over the unit.
	 *
	 * Throws "Invalid arguments" if no message is passed for a unit that is still in the game; a null or
	 * undefined message shows nothing.
	 */
	void overhead(Object message);

	/** Revive the character.
	 *
	 * BE CAREFUL! This function directly sends packets without checks. If you
	 * call this function and are not dead, you might get flagged/banned.
	 */
	void revive();

	/** Returns the item flags.
	 *
	 * \returns Item flags:
	 *
	 * 0x00000001 - Equipped
	 *
	 * 0x00000008 - In socket
	 *
	 * 0x00000010 - Identified
	 *
	 * 0x00000040 - Weapon/shield is in the active weapon switch
	 *
	 * 0x00000080 - Weapon/shield is in the inactive weapon switch
	 *
	 * 0x00000100 - Item is broken
	 *
	 * 0x00000400 - Full rejuv
	 *
	 * 0x00000800 - Socketed
	 *
	 * 0x00002000 - In the trade or gamble screen
	 *
	 * 0x00004000 - Not in a socket
	 *
	 * 0x00010000 - Is an ear
	 *
	 * 0x00020000 - A starting item (worth only 1 gold)
	 *
	 * 0x00200000 - Rune, or potion, or mephisto's soulstone
	 *
	 * 0x00400000 - Ethereal
	 *
	 * 0x00800000 - Is an item
	 *
	 * 0x01000000 - Personalized
	 *
	 * 0x04000000 - Runeword
	 *
	 * Source: http://subversion.assembla.com/svn/d2bs/scripts/YAMB/libs/YAMB/common/YAM-Common.dbl r1086
	 */
	int getFlags();

	/** Get a stat by stat id and sub index.
	 *
	 * Life, mana and stamina (stats 6-11) come back in whole points.
	 * Experience (stats 13, 29 and 30) is read unsigned, so values of 2^31 and
	 * above stay positive. On an item, stat 92 returns the item's level
	 * requirement for your character (the same value as the itemlevelreq
	 * property) and ignores nSubIndex. On any other unit, your own character
	 * included, stat 92 does not read a stat: the game's item level-requirement
	 * calculation runs on that unit and returns a number with no fixed
	 * meaning. Any other stat returns the game's total for that stat and
	 * subindex; when that total is 0, the value of that stat and subindex in
	 * the stat list that getStat(-1) reads (not the base stats getStat(-1)
	 * adds) is returned instead (0 if it is not there). Arguments that are not
	 * integers are converted to integers.
	 *
	 * \param nStat The stat id (the ID column of ItemStatCost.txt).
	 *
	 * \param nSubIndex A subindex to a certain type of stat. For instance a
	 * specific skill for the +1 to skill tab stat. Defaults to 0.
	 *
	 * \return The stat value. Returns false when no stat id is passed or the
	 * unit has left the game. Outside a ready game it returns false (with a
	 * 'Game not ready' warning).
	 */
	double getStat(int nStat, int nSubIndex = 0);

	/** Get an array of the unit's listed stats.
	 *
	 * The array holds the entries of the unit's stat list plus any of the
	 * unit's base stats not already in it. It is not every stat the unit
	 * has. Values are raw: life, mana and stamina are in 1/256 points, and
	 * values of 2^31 and above (such as high experience) come back negative.
	 *
	 * \param nStat Set to -1.
	 *
	 * \param nSubIndex Ignored.
	 *
	 * \return An array of arrays. The indices of the inner array are:
	 * 0 - stat id, 1 - subindex, 2 - value. Returns false when the unit has
	 * no such stat list, even if it has base stats. Outside a ready game it
	 * returns false (with a 'Game not ready' warning).
	 */
	int[][] getStat(int nStat, int nSubIndex = 0);

	/** Get the unit's stats as an array indexed by stat id.
	 *
	 * Each element is an array indexed by subindex that holds the value, with
	 * life, mana and stamina (stats 6-11) in whole points. Other values are
	 * raw, so values of 2^31 and above (such as high experience) come back
	 * negative. The values come from the unit's base stats, its stat totals
	 * (the values the single-stat form reads) and the stat list that
	 * getStat(-1) reads, in that order; where more than one holds the same
	 * stat and subindex, the last one wins, so a value here is not always the
	 * one the single-stat form returns.
	 *
	 * A stat that carries a skill (for example a charged skill or a
	 * chance-to-cast skill such as one cast on striking) holds an object
	 * {skill, level} instead, for skill ids 8 and up; lower skill ids come
	 * back as ordinary entries. A charged skill also gets charges and
	 * maxcharges, except on an item with a maximum of 1 charge, or of 2
	 * charges with none left. When the same stat id occurs more than once,
	 * the element is an array of such objects. Each entry comes back once, so
	 * a stat with a single skill holds a bare object, not an array; check an
	 * element with Array.isArray. When the stat also holds an ordinary entry
	 * for a lower skill id, the element is a subindex array instead, with the
	 * skill objects after its last index. Oskill mods (stat 97) are ordinary
	 * entries: element 97 is indexed by skill id and holds the bonus level.
	 *
	 * \param nStat Set to -2.
	 *
	 * \param nSubIndex Ignored.
	 *
	 * \return A sparse array indexed by stat id. Outside a ready game it
	 * returns false (with a 'Game not ready' warning).
	 */
	Object[] getStat(int nStat, int nSubIndex = 0);

	/** Return whether or not the unit has a given state.
	 *
	 * Objects, items and tiles always return false: the game's state check
	 * answers false for them.
	 *
	 * \param nState The state id: the id column of States.txt, 0-183. State
	 * 184 (uberminion) exists in 1.14d but getState rejects it and returns
	 * false.
	 *
	 * Throws "Invalid arguments" if no state id is passed. Returns false when
	 * the unit has left the game, or when nState is not an integer or is out
	 * of range. Outside a ready game it returns false (with a 'Game not
	 * ready' warning).
	 *
	 * \return Whether or not the unit has the state.
	 */
	bool getState(int nState);

	/** Determine whether or not a unit has a given enchant.
	 *
	 * \param nEnchant Enchantment id.
	 *
	 * \return Whether or not the unit has the enchantment.
	 */
	bool getEnchant(int nEnchant);

	/** Shop with a given npc, either buying or selling.
	 *
	 * \param dwMode What to do with the item. 1 - Sell, 2 - Buy, 6 - ?
	 *
	 * \return Whether or not the shop went through.
	 */
	bool shop(int dwMode);

	/** Set the skill on the given hand to be skill with name skillName.
	 *
	 * Asks the game to put the skill on the hand, then waits up to about one
	 * second for the hand to show it. When the hand already shows the skill,
	 * the call returns true at once. The call always acts on your own
	 * character, whichever unit it is called on.
	 *
	 * The name is matched case-insensitively against D2BS's built-in English
	 * skill-name table, not the game's localized strings. "Shadow Master" and
	 * "Phoenix Strike" are not recognized by name; pass their ids (279 and
	 * 280) instead.
	 *
	 * \param skillName Name of the skill to put up.
	 *
	 * \param nHand Hand to put the skill on: 0 - right, any other integer -
	 * left.
	 *
	 * \return true once the hand shows the skill. Returns false when the
	 * skill name is unknown, nHand is missing or not an integer, your
	 * character does not have the skill (total level 0), you are not in a
	 * game, or the hand does not show the skill in time.
	 */
	bool setSkill(String skillName, int nHand);

	/** Set the skill on the given hand to be skill with id nSkillId.
	 *
	 * Asks the game to put the skill on the hand, then waits up to about one
	 * second for the hand to show it. When the hand already shows the skill,
	 * the call returns true at once. The call always acts on your own
	 * character, whichever unit it is called on.
	 *
	 * \param nSkillId Id of the skill to put up. See \ref reference_skills.
	 *
	 * \param nHand Hand to put the skill on: 0 - right, any other integer -
	 * left.
	 *
	 * \return true once the hand shows the skill. Returns false when the
	 * first argument is neither a string nor an integer, nHand is missing or
	 * not an integer, your character does not have the skill (total level
	 * 0), you are not in a game, or the hand does not show the skill in time.
	 */
	bool setSkill(int nSkillId, int nHand);

	/** Set the skill on the given hand to be skill with name skillName.
	 *
	 * Asks the game to put the skill on the hand, then waits up to about one
	 * second for the hand to show it. When the hand already shows the skill,
	 * the call returns true at once. The call always acts on your own
	 * character, whichever unit it is called on.
	 *
	 * The name is matched case-insensitively against D2BS's built-in English
	 * skill-name table, not the game's localized strings. "Shadow Master" and
	 * "Phoenix Strike" are not recognized by name; pass their ids (279 and
	 * 280) instead.
	 *
	 * \param skillName Name of the skill to put up.
	 *
	 * \param nHand Hand to put the skill on: 0 - right, any other integer -
	 * left.
	 *
	 * \param item An item unit: the hand gets that item's charged skill. A
	 * third argument that is not an item unit is ignored, and the skill is
	 * set as a normal skill. The item is not checked to be yours.
	 *
	 * \return true once the hand shows the skill. Returns false when the
	 * skill name is unknown, nHand is missing or not an integer, your
	 * character does not have the skill (total level 0), you are not in a
	 * game, or the hand does not show the skill in time.
	 */
	bool setSkill(String skillName, int nHand, Unit item);

	/** Set the skill on the given hand to be skill with id nSkillId.
	 *
	 * Asks the game to put the skill on the hand, then waits up to about one
	 * second for the hand to show it. When the hand already shows the skill,
	 * the call returns true at once. The call always acts on your own
	 * character, whichever unit it is called on.
	 *
	 * \param nSkillId Id of the skill to put up. See \ref reference_skills.
	 *
	 * \param nHand Hand to put the skill on: 0 - right, any other integer -
	 * left.
	 *
	 * \param item An item unit: the hand gets that item's charged skill. A
	 * third argument that is not an item unit is ignored, and the skill is
	 * set as a normal skill. The item is not checked to be yours.
	 *
	 * \return true once the hand shows the skill. Returns false when the
	 * first argument is neither a string nor an integer, nHand is missing or
	 * not an integer, your character does not have the skill (total level
	 * 0), you are not in a game, or the hand does not show the skill in time.
	 */
	bool setSkill(int nSkillId, int nHand, Unit item);

	/** Move to the given location.
	 *
	 * \param x The x location.
	 * 
	 * \param y The y location.
	 */
	void move(int x, int y);

	/** Move to this unit.
	 */
	void move();

	/** Get the quest flag for a quest specified by act and quest number.
	 *
	 * \param nAct The act of the quest.
	 *
	 * \param nQuest The quest number.
	 *
	 * \return The quest flag for the specified quest, or undefined when act and
	 * quest are not both integers.
	 */
	int getQuest(int nAct, int nQuest);

	/** Get the number of minions of a certain type.
	 *
	 * \param nType The type of the minions.
	 *
	 * \return The number of minions of the specified type.
	 */
	int getMinionCount(int nType);

	/** Get price to repair this unit at the current npc given by nNpcClassId.
	 *
	 * \param nNpcClassId The class id of the npc to get the price for repair
	 * at.
	 *
	 * \return The price to repair the given unit.
	 */
	int getRepairCost(int nNpcClassId = 154);

	/** Get the cost to do something (buy, sell, repair) with the given item, at
	 * the given npc.
	 *
	 * \param nMode What to do: 0 - buy, 1 - sell, 2 - repair.
	 *
	 * \param npc The npc to check the price with.
	 *
	 * \return The price.
	 */
	int getItemCost(int nMode, Unit npc);

	/** Get the cost to do something (buy, sell, repair) with the given item, at
	 * the given npc, in the given difficulty.
	 *
	 * \param nMode What to do: 0 - buy, 1 - sell, 2 - repair.
	 *
	 * \param npc The npc to check the price with.
	 *
	 * \param nDifficulty The difficulty to check the price in.
	 *
	 * \return The price.
	 */
	int getItemCost(int nMode, Unit npc, int nDifficulty);

	/** Get the cost to do something (buy, sell, repair) with the given item, at
	 * the given npc.
	 *
	 * \param nMode What to do: 0 - buy, 1 - sell, 2 - repair.
	 *
	 * \param nNpcClassId The class id of the npc to check the price with.
	 *
	 * \return The price.
	 */
	int getItemCost(int nMode, int nNpcClassId = 154);

	/** Get the cost to do something (buy, sell, repair) with the given item, at
	 * the given npc, in the given difficulty.
	 *
	 * \param nMode What to do: 0 - buy, 1 - sell, 2 - repair.
	 *
	 * \param nNpcClassId The class id of the npc to check the price with.
	 *
	 * \param nDifficulty The difficulty to check the price in.
	 *
	 * \return The price.
	 */
	int getItemCost(int nMode, int nNpcClassId, int nDifficulty);

	/** The type of the unit.
	 *
	 * 0 - Player
	 *
	 * 1 - NPC
	 *
	 * 2 - Object
	 *
	 * 3 - Missile
	 *
	 * 4 - Item
	 *
	 * 5 - Warp
	 * 
	 * Source: botNET
	 */
	int type;

	/** The class id of the object.
	 *
	 * See: http://forums.d2botnet.org/viewtopic.php?f=18&t=1002
	 *
	 * http://forums.d2botnet.org/viewtopic.php?f=18&t=986 and
	 *
	 * http://forums.d2botnet.org/viewtopic.php?f=18&t=985
	 */
	int classId;

	/** The mode of the unit.
	 *
	 * See: http://forums.d2botnet.org/viewtopic.php?f=18&t=988
	 */
	int mode;

	/** The name of the unit.
	 */
	String name;

	/** The seed used to create the map.
	 */
	int mapid;

	/** The act where the unit is currently located.
	 */
	int act;

	/** The unit unique id. Referred to simply as the unit id. Used along with
	 * the unit type to uniquely identify the unit.
	 */
	int gid;

	/** The x location of the unit.
	 */
	int x;

	/** The y location of the unit.
	 */
	int y;

	/** The target x location of the unit. Where the game is taking the unit.
	 */
	int targetx;

	/** The target y location of the unit. Where the game is taking the unit.
	 */
	int targety;

	/** The id of the area (level) the unit is located in.
	 *
	 * The value is the Id column of Levels.txt, for example 1 = Rogue
	 * Encampment, 108 = The Chaos Sanctuary (Chaos Sanctum in Levels.txt),
	 * 109 = Harrogath. See \ref reference_areas.
	 *
	 * Undefined when the game has no room for the unit.
	 */
	int area;

	/** The unit's current health.
	 */
	int hp;

	/** The unit's maximum health.
	 */
	int hpmax;

	/** The unit's current mana.
	 */
	int mp;

	/** The unit's maximum mana.
	 */
	int mpmax;

	/** The unit's current stamina.
	 */
	int stamina;

	/** The unit's maximum stamina.
	 */
	int staminamax;

	/** The character level. The level that determines stat points, skill
	 * levels, etc.
	 */
	int charlvl;

	/** The number of items in inventory.
	 */
	int itemcount;

	/** The unit id of the owner of the unit.
	 */
	double owner;

	/** The type of owner.
	 *
	 * \ref type
	 */
	int ownertype;

	/** The special type of the unit. Represents things like unique, minion,
	 * boss, etc.
	 *
	 * 1 - "Normal" Boss
	 * 2 - Champion
	 * 4 - Boss
	 * 8 - Minion
	 */
	int spectype;

	/** The direction the unit is facing, from 0 to 63.
	 *
	 * 0 faces straight down the screen and the value increases clockwise:
	 * 16 faces left (west), 32 up (north) and 48 right (east). 8, 24, 40 and
	 * 56 lie between them along the game's map axes (down-left, up-left,
	 * up-right and down-right).
	 *
	 * A player or monster turns a step at a time, so while it turns this
	 * value can trail the direction it is moving in.
	 *
	 * Undefined when the unit has no path or its path is not in a room. For
	 * objects and items the value is currently not their facing: it can be
	 * undefined or an arbitrary number.
	 */
	int direction;

	/** The three digit code for an item.
	 *
	 * See: http://forums.d2botnet.org/viewtopic.php?f=18&t=991
	 */
	String code;

	/** The magic prefix of the item.
	 */
	String prefix;

	/** The magic suffix of the item.
	 */
	String suffix;

	/** The id code for the magic prefix.
	 */
	int prefixnum;

	/** The id code for the magic suffix.
	 */
	int suffixnum;

	/** The full name of an item.
	 */
	String fname;

	/** The item quality.
	 *
	 * 1 - Low quality
	 *
	 * 2 - Normal
	 *
	 * 3 - Superior
	 *
	 * 4 - Magic
	 *
	 * 5 - Set
	 *
	 * 6 - Rare
	 *
	 * 7 - Unique
	 *
	 * 8 - Crafted
	 *
	 * Source: botNET
	 */
	int quality;

	/** Which kind of inventory grid holds the item.
	 *
	 * 0 - Not in any inventory grid (on the ground or on the cursor)
	 *
	 * 1 - A storage page (inventory, cube, stash, trade or shop)
	 *
	 * 2 - The belt
	 *
	 * 3 - Equipped (body locations 1-10)
	 *
	 * 4 - Equipped in a weapon-switch slot (body locations 11 and 12)
	 *
	 * Use location to tell storage pages apart. An item linked into an
	 * inventory without a grid cell also reads 1, so node 1 alone does not
	 * prove the item sits on a storage page.
	 *
	 * Undefined for units that are not items, so
	 * me.node is always undefined.
	 */
	int node;

	/** The location of the item (ground, inventory, stash, etc.).
	 *
	 * 0 - Ground
	 *
	 * 1 - Equipped
	 *
	 * 2 - Belt
	 *
	 * 3 - Inventory
	 *
	 * 4 - Store
	 *
	 * 5 - Trade
	 *
	 * 6 - Cube
	 *
	 * 7 - Stash
	 */
	int location;

	/** The x size of the item.
	 */
	int sizex;

	/** The y size of the item.
	 */
	int sizey;

	/** The type of the item.
	 *
	 * The base item's primary type: its row index in ItemTypes.txt, counting
	 * data rows from 0 and skipping the "Expansion" separator row (for example
	 * 2 = shield, 4 = gold, 10 = ring, 12 = amulet, 58 = jewel, 74 = rune).
	 * Class-specific bases report their own type, so an auric shield is not 2.
	 * See \ref reference_itemtypes.
	 *
	 * Undefined for units that are not items, so
	 * me.itemType is always undefined.
	 */
	int itemType;

	/** The description of the item.
	 */
	String description;

	/** The equipped location of the item.
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
	int bodylocation;

	/** Item level. Used for things like item stat rolling.
	 */
	int ilvl;

	/** Whether the controlled character is in the always run mode or not.
	 *
	 * 0 - Walk unless directed to run
	 * 1 - Always run
	 */
	int runwalk;

	/** Set the weapon switch.
	 */
	int weaponswitch;

	/** The account name used to log on to the account.
	 */
	String account;

	/** The character name.
	 */
	String charname;

	/** The difficulty of the game.
	 *
	 * 0 - Normal
	 *
	 * 1 - Nightmare
	 *
	 * 2 - Hell
	 */
	int diff;

	/** The maximum difficulty available. See Me::diff for details.
	 */
	int maxdiff;

	/** The name of the game.
	 */
	String gamename;

	/** The password of the game.
	 */
	String gamepassword;

	/** A string representation of the IPv4 address of the game server.
	 */
	String gameserverip;

	/** When the character entered the current game, as a getTickCount() value. For a game D2BS was loaded into, it is
	 * when D2BS first saw that game; before the first game, when D2BS loaded; at the menu after a game, 0.
	 */
	double gamestarttime;

	/** The game type, expansion or not.
	 *
	 * 0 - Classic
	 *
	 * 1 - Expansion (Lord of Destruction)
	 *
	 * It reads 0 at the menus, early while a game loads and while a game is
	 * left; it can take its real value shortly before gameReady turns true,
	 * so read it once gameReady is true. Read-only.
	 */
	int gametype;

	/** Whether or not there is an item on the cursor.
	 */
	bool itemoncursor;

	/** Whether or not the game is a ladder game.
	 */
	bool ladder;

	/** The ping in milliseconds.
	 */
	int ping;

	/** The frame rate.
	 */
	int fps;

	/** Whether or not the gametype is hardcore.
	 */
	int playertype;

	/** The realm name.
	 */
	String realm;

	/** The realm name, normally the same value as me.realm.
	 *
	 * It is not an abbreviation: it holds the name of the Battle.net realm
	 * the client logged in to, for example "Europe". Empty in single player
	 * and TCP/IP; undefined if the client has no Battle.net data. Read-only.
	 */
	String realmshort;

	/** The cost to revive the merc.
	 */
	int mercrevivecost;

	/** Whether the controlled character is in the always run mode or not.
	 *
	 * 0 - Walk unless directed to run
	 * 1 - Always run
	 */
	int runwalk;

	/** The health at or below which to chicken, outside town; 0 turns it off.
	 */
	int chickenhp;

	/** The mana at or below which to chicken, outside town; 0 turns it off.
	 */
	int chickenmp;

	/** Whether to chicken if another player hostiles the controlled character.
	 */
	bool quitonhostile;

	/** Whether keyboard input to the game window is blocked.
	 *
	 * While true, keyboard input to the game window is discarded before it
	 * reaches the game, the D2BS console (including the Home key) or
	 * keydown/keyup event listeners. Alt+F4 still works. Keystrokes sent with
	 * sendKey() still reach the game. Defaults to false and is shared by all
	 * scripts. Only true or false is accepted, so other values are
	 * ignored. login() sets it to true while it runs and to false when it
	 * returns, whatever a script set before.
	 */
	bool blockKeys;

	/** Whether mouse input to the game window is blocked.
	 *
	 * While true, mouse input to the game window is discarded before the game
	 * handles it, and no mouse click/move events or screen-hook clicks are
	 * delivered; getMouseCoords() does not follow the real cursor meanwhile.
	 * Clicks on the title bar (and the left or top border) still pass
	 * through. Defaults to false and is shared by all scripts. Only true or
	 * false is accepted, so other values are ignored. login() sets it to true
	 * while it runs and to false when it returns, whatever a script set
	 * before.
	 */
	bool blockMouse;

	/** Whether or not the game window is in the game.
	 */
	bool gameReady;

	/** The profile currently being used.
	 */
	String profile;

	/** Whether the game is set to not pick up items or not.
	 */
	bool nopickup;

	/** The process id of Diablo II.
	 */
	double pid;

	/** The game's resolution mode.
	 *
	 * A mode number, not a pixel size:
	 *
	 * 0 - 640x480 (in a game)
	 *
	 * 1 - 800x600 (at the out-of-game menus)
	 *
	 * 2 - 800x600 (in a game)
	 *
	 * The game also maps 3 to 1344x700, but nothing in stock 1.14d is known to
	 * set it. Compare against 0 to detect 640x480 in a game; at the menus it
	 * reads 1, so a test for 2 there fails. Read-only.
	 */
	int screensize;

	/** The window title.
	 */
	String windowtitle;

	/** True if the game is not at menu.
	 *
	 * Unlike gameReady, it is also true while a game is loading, during act
	 * changes and on the loading screens between the menus and a game. Use
	 * gameReady to know the game can be acted on. Read-only.
	 */
	bool ingame;

	/** Whether a script error makes the client leave the current game.
	 *
	 * When true, any script error (an uncaught exception or a compile error,
	 * not a warning) in any script, the console included, makes the client
	 * leave the current game and return to the menus; the in-game scripts
	 * end with the game as on any game exit. The error is still logged and
	 * printed. It has no effect at the menus or while the game is loading.
	 * The process keeps running. The default comes from QuitOnError in
	 * d2bs.ini (false if absent; a value starting with t, T or 1 is true).
	 * Only true or false is accepted, so other values such as 1 are ignored.
	 */
	bool quitonerror;

	/** The max time in milliseconds to stay in game before the game auto-quits.
	 */
	int maxgametime;
};
