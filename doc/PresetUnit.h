/** Represents a PresetUnit (a type of Unit that is available as soon as the
 * Area data is available).
 *
 * A preset unit's position is stored relative to its room. Its game map
 * coordinates are x: roomx * 5 + x and y: roomy * 5 + y. roomx and roomy are
 * the room's position in room tiles (5 map units each, the same values as
 * Room::x and Room::y), and x and y are the unit's offset in map units from the
 * room's top-left (minimum x and y) corner.
 */
class PresetUnit
{
public:
	/** The type ID. See Unit::type.
	 */
	int type;

	/** The X position of the room the PresetUnit is in, in room tiles of 5 map
	 * units (the same value as Room::x).
	 */
	int roomx;

	/** The Y position of the room the PresetUnit is in, in room tiles of 5 map
	 * units (the same value as Room::y).
	 */
	int roomy;

	/** The X offset in map units from the room's corner. Add roomx * 5 for the
	 * map X coordinate.
	 */
	int x;

	/** The Y offset in map units from the room's corner. Add roomy * 5 for the
	 * map Y coordinate.
	 */
	int y;

	/** The unique ID of the PresetUnit.
	 */
	int id;

	/** The Area ID of the Area the PresetUnit is in.
	 */
	int level;
};
