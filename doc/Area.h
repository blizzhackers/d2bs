/** Represent an Area of the map. Reading a property waits while an area or
 * act loads. It throws an error while the area is not in the act the player
 * is in (for example after a waypoint to another act) or the game cannot
 * build its rooms, and when the game isn't ready: out of game, when the wait
 * for an area or act load times out, or at once while the game is busy in a
 * blocking listener or while your script holds the game lock.
 */
class Area
{
public:
	/** Get an array of the Area's exits. This includes arrays where you can
	 * walk between two areas and warp Unit s.
	 */
	Exit[] exits;

	/** The name of the Area.
	 */
	String name;

	/** The x coordinate of the left side of the map.
	 */
	int x;

	/** The x size (width) of the map.
	 */
	int xsize;

	/** The y coordinate of the bottom (smallest y) of the map.
	 */
	int y;

	/** The y size (height) of the map.
	 */
	int ysize;

	/** The area id. See Unit::area.
	 */
	int id;
};
