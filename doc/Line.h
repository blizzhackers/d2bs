#include "ScreenHooks.h"

/** A 2d line.
 *
 * A Line is a straight line from (x, y) to (x2, y2) in one palette color.
 * Create one with `new Line(x, y, x2, y2, color, automap, click, hover)`;
 * every argument is optional (defaults 0, 0, 0, 0, 0, false, none, none).
 * Coordinates are game-window pixels, x to the right and y down. With
 * `automap` true both ends are game map coordinates (the units of unit.x and
 * unit.y), and the Line shows only while the automap is open; `automap` is a
 * constructor argument only. A Line made by an out-of-game script draws only
 * at the menus; one made in game (or from the console) draws only in game. A
 * Line has no alignment and can never be clicked.
 *
 * Hooks of every script and every hook class are drawn in ascending `zorder`
 * (default 1), so a higher value draws on top. Set `visible` to hide or show
 * the Line, and call remove() to delete it. Keep a reference to every hook:
 * one whose object is garbage-collected is removed. When the script that made
 * a hook ends, its hooks leave the screen (with the default
 * `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that
 * ended normally keeps its hooks drawn until the script cache is flushed).
 *
 * Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `color` and `zorder` keep only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value is ignored.
 */
class Line
{
public:
	/** Create a Line hook with the given parameters.
	 *
	 * \param x The x coordinate of the first end of the Line.
	 *
	 * \param y The y coordinate of the first end of the Line.
	 *
	 * \param x2 The x coordinate of the second end of the Line.
	 *
	 * \param y2 The y coordinate of the second end of the Line.
	 *
	 * \param color The color of the Line: an index into the game's 256-color
	 * palette; only the low 8 bits are drawn.
	 *
	 * \param automap With true, all four coordinates are map coordinates and
	 * the Line shows only while the automap is open. Only true or false is
	 * accepted.
	 *
	 * \param click A click function for the Line, kept but never called (a
	 * click never hits a Line); any other value sets no handler.
	 *
	 * \param hover A hover function for the Line, kept but never called; any
	 * other value sets no handler.
	 */
	Line(int x = 0, int y = 0, int x2 = 0, int y2 = 0, int color = 0,
		bool automap = false, ClickHandler click = null,
		HoverHandler hover = null);

	/** Remove the Line from the screen for good.
	 *
	 * The object stays and its click and hover handlers are released. Its
	 * properties then read undefined, writes to them are ignored, and calling
	 * remove() again does nothing. Returns undefined.
	 */
	void remove();

	/** The first x coordinate of the Line.
	 *
	 * A map coordinate when the Line was made with automap true. While x or y
	 * is -1 the Line is not drawn (x2 and y2 of -1 do not hide it).
	 */
	int x;

	/** The first y coordinate of the Line.
	 *
	 * A map coordinate when the Line was made with automap true. While x or y
	 * is -1 the Line is not drawn.
	 */
	int y;

	/** The second x coordinate of the Line.
	 *
	 * An absolute coordinate, not a width; a map coordinate when the Line was
	 * made with automap true.
	 */
	int x2;

	/** The second y coordinate of the Line.
	 *
	 * An absolute coordinate, not a height; a map coordinate when the Line was
	 * made with automap true.
	 */
	int y2;

	/** Whether or not the Line is visible.
	 *
	 * Defaults to true. Only true or false is accepted.
	 */
	bool visible;

	/** The color of the Line.
	 *
	 * An index (0-255) into the game's 256-color palette. Only the low 8 bits
	 * are drawn, so 256 draws as 0, though the property reads back the low 16
	 * bits.
	 */
	int color;

	/** The z-order of the Line (what it covers up and is covered by).
	 *
	 * Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so
	 * a higher value covers a lower one; hooks with equal values draw in no set
	 * order.
	 */
	int zorder;

	/** The click handler for the Line. D2BS keeps it and returns it from
	 * `click`, but never calls it: a click never hits a Line. Assign null or
	 * undefined to remove it; a value that is not a function is ignored.
	 */
	ClickHandler click;

	/** A hover handler for the Line. D2BS keeps it and returns it from `hover`,
	 * but never calls it. Assign null or undefined to remove it; a value that
	 * is not a function is ignored.
	 */
	HoverHandler hover;
};
