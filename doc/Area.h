/** Represent an Area of the map. Reading a property throws an error while the
 * area is not in the act the player is in, for example after a waypoint to
 * another act, or when the game isn't ready (out of game, or in a blocking
 * listener while an area loads).
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
