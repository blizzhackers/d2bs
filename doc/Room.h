/** Represents a "Room" in the game. A Room is a rectangular piece of the map
 * data. Its methods reach the room's lists, such as its PresetUnit s.
 *
 * A Room keeps its area, position and size, so its properties keep their
 * values while a script holds it, until getNext() moves it to the next room.
 * Its methods look the room up again on every call, in the game the room came
 * from and the player's current act, wherever getRoom() could find it. When
 * they can't, getNext() returns false, getNearby() returns an empty array,
 * getStat() returns null and the other methods return undefined. unitInRoom()
 * compares the unit's room with this one instead: it returns false when the
 * unit is in another room, act or game, and undefined when the unit can't be
 * found or an act change is still loading.
 */
class Room
{
public:
	/** Move to the next Room.
	 *
	 * \return Whether there was another Room to move to or not.
	 */
	bool getNext();

	/** Reveal the Room on the map.
	 *
	 * \return Whether or not the Room was successfully revealed.
	 */
	bool reveal();

	/** Get the PresetUnit s from the current Room.
	 *
	 * \param nType The type of Unit to get, or 0 for any. See Unit::type. A
	 * number or numeric string converts as `nType >>> 0` does; a result of 0,
	 * which a value that converts to NaN also gives, matches any.
	 *
	 * \param nClass The class id of the Unit (s) to find, or 0 for any. See
	 * Unit::classId. A number or numeric string converts as `nClass >>> 0` does;
	 * a result of 0, which a value that converts to NaN also gives, matches any.
	 *
	 * \return An array of all the PresetUnit s from the current Room.
	 */
	PresetUnit[] getPresetUnits(int nType = 0, int nClass = 0);

	/** Get the collision data for the Room.
	 *
	 * The array holds one entry per map row, each an array with one value per
	 * map column: result[dy][dx] is the subtile at map offset dx in x and dy in
	 * y from the room's corner (the corner with the smallest x and y). That is
	 * the subtile at map coordinates (x * 5 + dx, y * 5 + dy), where x and y
	 * are this Room's x and y. A room whose data is not loaded is loaded for
	 * the call and unloaded again.
	 *
	 * Each value is the game's 16-bit collision mask for that subtile, a set of
	 * flag bits. D2BS's own pathing treats 0x1 as blocking walking and 0x2 as
	 * blocking line of sight; the names of the other bits are conventional
	 * names, not verified behavior. See \ref reference_collision.
	 *
	 * \return An array with the collision data. The first dimension is the y
	 * coordinate, the second is the x coordinate. Returns undefined when the
	 * game is not ready, when the room is no longer in your current act or
	 * game, or when it has no collision data.
	 */
	int[][] getCollision();

	/** Get the "nearby" Rooms from pRoom2Near member.
	 *
	 * \return An array of the nearby Rooms.
	 */
	Room[] getNearby();

	/** Get some property of the Room.
	 *
	 * \param nStat Which property to get (any number or numeric string, made a
	 * 32-bit integer as `nStat | 0` does, so 4.5 reads stat 4):
	 *
	 * 0 - xStart (room 1)
	 *
	 * 1 - yStart (room 1)
	 *
	 * 2 - xSize (room 1)
	 *
	 * 3 - ySize (room 1)
	 *
	 * 4 - posX (room 2)
	 *
	 * 5 - posY (room 2)
	 *
	 * 6 - sizeX (room 2)
	 *
	 * 7 - sizeY (room 2)
	 *
	 * 9 - posGameX (coll)
	 *
	 * 10 - posGameY (coll)
	 *
	 * 11 - sizeGameX (coll)
	 *
	 * 12 - sizeGameY (coll)
	 *
	 * 13 - posRoomX (coll)
	 *
	 * 14 - posRoomY (coll)
	 *
	 * 15 - sizeRoomX (coll)
	 *
	 * 16 - sizeRoomY (coll)
	 *
	 * \return The stat value, or null with no argument, undefined or null, a value
	 * that converts to NaN, an unlisted stat, when the room's data can't be
	 * loaded, or for stats 9 to 16 when the room has no collision map.
	 */
	int getStat(int nStat);

	/** Get the first Room of the level this Room is on.
	 *
	 * \return The first Room.
	 */
	Room getFirst();

	/** Determine whether or not the Unit is in the Room.
	 *
	 * \param unit The unit to check.
	 *
	 * \return Whether or not the Unit is in the Room.
	 */
	bool unitInRoom(Unit unit);

	/** The LvlPrest.txt row (Def column) of the preset map the Room was built
	 * from.
	 *
	 * This includes the pieces of randomly generated dungeons and caves. A large
	 * preset map is cut into several rooms, which all read the same number. It
	 * is -1 for rooms of outdoor areas that were not built from a preset map;
	 * an outdoor area can also hold rooms built from a preset map, so not every
	 * room of an outdoor area reads -1.
	 */
	int number;

	/** Get the x coordinate of the Room.
	 */
	int x;

	/** Get the y coordinate of the Room.
	 */
	int y;

	/** Get the x size (width) of the Room.
	 */
	int xsize;

	/** Get the y size (height) of the Room.
	 */
	int ysize;

	/** No implementation.
	 */
	int subnumber;

	/** Get the Area ID for the area which the Room is in.
	 */
	int area;

	/** Get the Area ID for the area which the Room is in.
	 */
	int level;

	/** The Area ID (66 to 72) of the true Tal Rasha's Tomb in the current game,
	 * the one with the Orifice leading to Duriel's Lair.
	 *
	 * The game picks it from the act's map seed, so every Room in Act II reads
	 * the same value. It is undefined on rooms in other acts. See
	 * \ref reference_areas.
	 */
	int correcttomb;
};
