#include "ScreenHooks.h"

/** This class represents a screen hook for a Frame.
 *
 * A Frame is the game's decorative border (edge and corner pieces) drawn
 * around a rectangle. It has no color or opacity and does not fill its
 * inside; put a Box under it for a background. Create one with
 * `new Frame(x, y, xsize, ysize, align, automap, click, hover)`; every
 * argument is optional (defaults 0, 0, 0, 0, 0, false, none, none).
 * Coordinates are game-window pixels, x to the right and y down. A Frame
 * draws only in game, never at the menus. With `automap` true the Frame still
 * uses screen pixels, but shows only while the automap is open; `automap` is a
 * constructor argument only. The four corner pieces are always drawn, so a
 * Frame too small for its border (even `new Frame()` with its default size of
 * 0), or with a negative size, still draws its corners.
 *
 * Hooks of every script and every hook class are drawn in ascending `zorder`
 * (default 1), so a higher value draws on top. Set `visible` to hide or show
 * the Frame, and call remove() to delete it. Keep a reference to every hook:
 * one whose object is garbage-collected is removed. When the script that made
 * a hook ends, its hooks leave the screen (with the default
 * `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that
 * ended normally keeps its hooks drawn, but not clickable, until the script
 * cache is flushed).
 *
 * Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `zorder` keeps only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value is ignored.
 */
class Frame
{
public:
	/** Create a Frame hook with the given parameters.
	 *
	 * \param x The x coordinate of the Frame in window pixels: its left edge
	 * with align 0, its centre with align 2; with align 1 the Frame starts
	 * xsize/2 to the right of x.
	 *
	 * \param y The y coordinate (top) of the Frame, in window pixels.
	 *
	 * \param x2 The xsize (width) of the Frame. A width too small for the
	 * border, or a negative one, draws only the four corner pieces.
	 *
	 * \param y2 The ysize (height) of the Frame. A height too small for the
	 * border, or a negative one, draws only the four corner pieces.
	 *
	 * \param align The horizontal alignment.
	 *
	 * 0 - Left
	 *
	 * 1 - shifted right by half the width (not right-aligned)
	 *
	 * 2 - Center
	 *
	 * \param automap When true, the Frame shows only while the automap is open;
	 * its coordinates stay screen pixels. Only true or false is accepted.
	 *
	 * \param click A function to call when the Frame is clicked; any other
	 * value sets no handler.
	 *
	 * \param hover A hover function for the Frame, kept but never called; any
	 * other value sets no handler.
	 */
	Frame(int x = 0, int y = 0, int x2 = 0, int y2 = 0, int align = 0,
		bool automap = false, ClickHandler click = null,
		HoverHandler hover = null);

	/** Remove the Frame from the screen for good.
	 *
	 * The object stays and its click and hover handlers are released. Its
	 * properties then read undefined, writes to them are ignored, and calling
	 * remove() again does nothing. Returns undefined.
	 */
	void remove();

	/** The x coordinate of the Frame.
	 *
	 * In window pixels, even when the Frame was made with automap true. It is
	 * the left edge with align 0 and the centre with align 2; with align 1 the
	 * Frame starts xsize/2 to the right of it. While x or y is -1 the Frame is
	 * not drawn.
	 */
	int x;

	/** The y coordinate (top) of the Frame.
	 *
	 * In window pixels. While x or y is -1 the Frame is not drawn.
	 */
	int y;

	/** The xsize (width) of the Frame.
	 *
	 * In pixels. A width too small for the border, or a negative one, draws
	 * only the four corner pieces.
	 */
	int xsize;

	/** The ysize (height) of the Frame.
	 *
	 * In pixels. A height too small for the border, or a negative one, draws
	 * only the four corner pieces.
	 */
	int ysize;

	/** Whether or not the Frame is visible.
	 *
	 * Defaults to true. A hidden Frame is not drawn and gets no clicks. Only
	 * true or false is accepted.
	 */
	bool visible;

	/** The horizontal alignment.
	 *
	 * 0 - Left: the Frame starts at x
	 *
	 * 1 - the Frame is shifted right by half its width (not right-aligned)
	 *
	 * 2 - Center: the Frame is centred on x
	 *
	 * Other values are stored as given and draw as 0. The click area always
	 * starts at x, whatever the alignment.
	 */
	int align;

	/** The z-order of the Frame (what it covers up and is covered by).
	 *
	 * Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so
	 * a higher value covers a lower one; hooks with equal values draw in no set
	 * order. A click reaches every hook under it, whatever its zorder.
	 */
	int zorder;

	/** The click handler that gets called when the Frame is clicked. Assign
	 * null or undefined to remove it; a value that is not a function is
	 * ignored.
	 *
	 * It is called as click(button, x, y) when a mouse button is pressed over
	 * the Frame: button 0 left, 1 right, 2 middle; x and y are window pixels.
	 * Only a boolean `true` return keeps the press from the game; the release
	 * always reaches the game. The handler runs when the owning script next
	 * processes events, and the game gets the press anyway if the handler has
	 * not answered within 3 seconds. A paused or ended script's hooks get no
	 * clicks.
	 *
	 * The Frame gets clicks while its `visible` property is true, even when it
	 * is not drawn at the moment (at the menus, automap closed, x or y of -1).
	 * The click area runs from x to x + xsize and y to y + ysize (edges
	 * excluded) on the raw values: alignment is not applied.
	 */
	ClickHandler click;

	/** A hover handler for the Frame. D2BS keeps it and returns it from
	 * `hover`, but never calls it. Assign null or undefined to remove it; a
	 * value that is not a function is ignored.
	 */
	HoverHandler hover;
};
