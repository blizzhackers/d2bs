#include "ScreenHooks.h"

/** A Box screen hook
 *
 * A Box is a filled rectangle in one palette color, drawn with the draw mode
 * set by `opacity` (see \ref opacity). Create one with
 * `new Box(x, y, xsize, ysize, color, opacity, align, automap, click, hover)`;
 * every argument is optional (defaults 0, 0, 0, 0, 0, 0, 0, false, none,
 * none). Coordinates are game-window pixels, x to the right and y down. A Box
 * made by an out-of-game script draws only at the menus; one made in game (or
 * from the console) draws only in game. With `automap` true, (x, y) and
 * (x + xsize, y + ysize) are game map coordinates (the units of unit.x and
 * unit.y), and the Box is the screen-aligned rectangle between those two
 * points on the automap: it shows only while the automap is open, and only
 * while xsize is greater than ysize and xsize + ysize is greater than 0
 * (otherwise nothing is drawn). `automap` is a constructor argument only;
 * there is no automap property.
 *
 * Hooks of every script and every hook class are drawn in ascending `zorder`
 * (default 1), so a higher value draws on top. Set `visible` to hide or show
 * the Box, and call remove() to delete it. Keep a reference to every hook: one
 * whose object is garbage-collected is removed. When the script that made a
 * hook ends, its hooks leave the screen (with the default `DisableCache=true`
 * in d2bs.ini; with `DisableCache=false` a script that ended normally keeps
 * its hooks drawn, but not clickable, until the script cache is flushed).
 *
 * Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `color`, `opacity` and `zorder` keep only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value (1 included) is ignored.
 */
class Box
{
public:
	/** Create a Box hook with the given parameters.
	 *
	 * \param x The x coordinate of the Box: its left edge with align 0, its
	 * centre with align 2; with align 1 the Box starts xsize/2 to the right of
	 * x. A map coordinate when automap is true.
	 *
	 * \param y The y coordinate (top) of the Box; a map coordinate when automap
	 * is true.
	 *
	 * \param x2 The xsize (width) of the Box. In screen pixels a zero or
	 * negative width draws nothing. With automap true it is a map-unit offset to
	 * the far corner, and the Box draws only when x2 is greater than y2 and
	 * x2 + y2 is greater than 0.
	 *
	 * \param y2 The ysize (height) of the Box. In screen pixels a zero or
	 * negative height draws nothing. With automap true it is a map-unit offset
	 * to the far corner.
	 *
	 * \param color The color of the Box: an index into the game's 256-color
	 * palette; only the low 8 bits are drawn.
	 *
	 * \param opacity The draw mode, 0-7 (default 0). See \ref opacity.
	 *
	 * \param align The horizontal alignment.
	 *
	 * 0 - Left
	 *
	 * 1 - shifted right by half the width (not right-aligned)
	 *
	 * 2 - Center
	 *
	 * \param automap true to place the Box at map coordinates on the automap,
	 * where it shows only while the automap is open (see the class text). Only
	 * true or false is accepted.
	 *
	 * \param click A function to call when the Box is clicked; any other value
	 * sets no handler.
	 *
	 * \param hover A hover function for the Box, kept but never called; any
	 * other value sets no handler.
	 */
	Box(int x = 0, int y = 0, int x2 = 0, int y2 = 0, int color = 0,
		int opacity = 0, int align = 0, bool automap = false,
		ClickHandler click = null, HoverHandler hover = null);

	/** Remove the Box from the screen for good.
	 *
	 * The object stays and its click and hover handlers are released. Its
	 * properties then read undefined, writes to them are ignored, and calling
	 * remove() again does nothing. Returns undefined.
	 */
	void remove();

	/** The x coordinate of the Box.
	 *
	 * It is the left edge with align 0 and the centre with align 2; with align
	 * 1 the Box starts xsize/2 to the right of it. A map coordinate when the
	 * Box was made with automap true. While x or y is -1 the Box is not drawn.
	 */
	int x;

	/** The y coordinate (top) of the Box.
	 *
	 * A map coordinate when the Box was made with automap true. While x or y is
	 * -1 the Box is not drawn.
	 */
	int y;

	/** The xsize (width) of the Box.
	 *
	 * In screen pixels a zero or negative width draws nothing. With automap
	 * true it is a map-unit offset (see the class text for when an automap Box
	 * draws).
	 */
	int xsize;

	/** The ysize (height) of the Box.
	 *
	 * In screen pixels a zero or negative height draws nothing. With automap
	 * true it is a map-unit offset (see the class text for when an automap Box
	 * draws).
	 */
	int ysize;

	/** Whether or not the Box is visible.
	 *
	 * Defaults to true. A hidden Box is not drawn and gets no clicks. Only true
	 * or false is accepted.
	 */
	bool visible;

	/** The horizontal alignment.
	 *
	 * 0 - Left: the Box starts at x
	 *
	 * 1 - the Box is shifted right by half its width (not right-aligned)
	 *
	 * 2 - Center: the Box is centred on x
	 *
	 * Other values are stored as given and draw as 0. The click area always
	 * starts at x, whatever the alignment.
	 */
	int align;

	/** The z-order of the Box (what it covers up and is covered by).
	 *
	 * Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so
	 * a higher value covers a lower one; hooks with equal values draw in no set
	 * order. A click reaches every hook under it, whatever its zorder.
	 */
	int zorder;

	/** The click handler that gets called when the Box is clicked. Assign null
	 * or undefined to remove it; a value that is not a function is ignored.
	 *
	 * It is called as click(button, x, y) when a mouse button is pressed over
	 * the Box: button 0 left, 1 right, 2 middle; x and y are window pixels.
	 * Only a boolean `true` return keeps the press from the game; the release
	 * always reaches the game. The handler runs when the owning script next
	 * processes events, and the game gets the press anyway if the handler has
	 * not answered within 3 seconds. A paused or ended script's hooks get no
	 * clicks.
	 *
	 * The Box gets clicks while its `visible` property is true, even when it is
	 * not drawn at the moment (other game state, automap closed, x or y of -1).
	 * The click area runs from x to x + xsize and y to y + ysize (edges
	 * excluded) on the raw values: alignment and the automap conversion are not
	 * applied.
	 */
	ClickHandler click;

	/** A hover handler for the Box. D2BS keeps it and returns it from `hover`,
	 * but never calls it. Assign null or undefined to remove it; a value that
	 * is not a function is ignored.
	 */
	HoverHandler hover;

	/** The color of the Box.
	 *
	 * An index (0-255) into the game's 256-color palette (0 is black). Only
	 * the low 8 bits are drawn, so 256 draws as 0, though the property reads
	 * back the low 16 bits.
	 */
	int color;

	/** The draw mode of the Box (default 0).
	 *
	 * Modes 0-2 blend the color with what is underneath; 3, 4 and 6 ignore the
	 * color (3 draws nothing, 4 draws black, 6 darkens what is underneath); 5
	 * and 7 draw the color solid. Only 0-7 are supported. See \ref opacity.
	 */
	int opacity;
};
