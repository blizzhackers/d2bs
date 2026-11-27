#include "ScreenHooks.h"

/** This class represents a screen hook for an Image.
 *
 * An Image draws a picture from an 8-bit BMP file on the game screen or the
 * automap. Create one with
 * `new Image(file, x, y, color, align, automap, click, hover)`; every argument
 * is optional (defaults: no file, 0, 0, 0, 0, false, none, none; without a
 * file the Image stays blank). The BMP's own palette is ignored: each pixel is drawn as that entry of the
 * game's current palette, pixels of index 0 are transparent, and pixels of
 * index 255 take the palette index given by `color`. The picture is centred
 * vertically on y; `align` puts x at its left edge (0), right edge (1) or
 * centre (2). An Image made by an out-of-game script draws only at the menus;
 * one made in game (or from the console) draws only in game. With `automap`
 * true (a constructor argument only), x and y are game map coordinates (the
 * units of unit.x and unit.y) and the Image is drawn only while the automap is
 * open; only align 2 then puts the picture on its map point, while 0 and 1
 * shift it sideways.
 *
 * Hooks of every script and every hook class are drawn in ascending `zorder`
 * (default 1), so a higher value draws on top. Set `visible` to hide or show
 * the Image, and call remove() to delete it. Keep a reference to every hook:
 * one whose object is garbage-collected is removed. When the script that made
 * a hook ends, its hooks leave the screen (with the default
 * `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that
 * ended normally keeps its hooks drawn, but not clickable, until the script
 * cache is flushed).
 *
 * Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `zorder` keeps only the low 16 bits, so a `zorder` of -1 reads back 65535; `color` is a palette index and only its low 8 bits are used. `automap` and `visible` take only true or false; any other value is ignored.
 */
class Image
{
public:
	/** Create an Image hook with the given parameters.
	 *
	 * \param szText The filename to load the image from, relative to the script folder. Only uncompressed 8-bit (256-color) bottom-up BMP files in the script folder load; a missing file or any other format leaves the Image blank, and a file fixed on disk loads the next time an Image or location names it. A name that has loaded once keeps that picture until Diablo II (Game.exe) exits, even if the file is rewritten on disk, and the same file under another spelling (letter case, .\, / instead of \) loads again as its own picture; a file that is deleted or cannot be opened (for example while another program holds it locked) leaves the Image blank and writes a line to the log. A name that leaves that folder (..\ or ../), contains any of ":?*<>|, or makes a path longer than 515 characters when joined to the script folder's path throws "Invalid image file path". A value that is not a string, or no file at all, counts as an empty name, which loads nothing, leaves the Image blank and writes a line to the log.
	 *
	 * \param x The x coordinate of the Image's anchor: its left edge (align 0),
	 * right edge (align 1) or centre (align 2).
	 *
	 * \param y The y coordinate of the Image's vertical centre.
	 *
	 * \param color A game palette index (0-255; only the low 8 bits are used)
	 * for the BMP's pixels of index 255. Pixels of index 0 are transparent, and
	 * every other index is drawn as that entry of the game's current palette.
	 *
	 * \param align The horizontal alignment.
	 *
	 * 0 - Left
	 *
	 * 1 - Right
	 *
	 * 2 - Center
	 *
	 * \param automap true to place the Image at game map coordinates and draw it
	 * only while the automap is open; false (the default) for screen
	 * coordinates. Only true or false is accepted.
	 *
	 * \param click A function to call when the Image is clicked; any other
	 * value sets no handler.
	 *
	 * \param hover A hover function for the Image, kept but never called; any
	 * other value sets no handler.
	 */
	Image(String szText = "", int x = 0, int y = 0, int color = 0,
		int align = 0, bool automap = false, ClickHandler click = null,
		HoverHandler hover = null);

	/** Remove the Image from the screen for good.
	 *
	 * The object stays and its click and hover handlers are released. Its
	 * properties then read undefined, writes to them are ignored, and calling
	 * remove() again does nothing. Returns undefined.
	 */
	void remove();

	/** The x coordinate of the Image.
	 *
	 * It is the picture's left edge with align 0, its right edge with align 1
	 * and its centre with align 2. A map coordinate when the Image was made
	 * with automap true. While x or y is -1 the Image is not drawn.
	 */
	int x;

	/** The y coordinate of the Image's vertical centre.
	 *
	 * A map coordinate when the Image was made with automap true. While x or y
	 * is -1 the Image is not drawn.
	 */
	int y;

	/** Whether or not the Image is visible.
	 *
	 * Defaults to true. A hidden Image is not drawn and gets no clicks. Only
	 * true or false is accepted.
	 */
	bool visible;

	/** The horizontal alignment.
	 *
	 * 0 - Left: x is the picture's left edge
	 *
	 * 1 - Right: x is the picture's right edge
	 *
	 * 2 - Center: x is the picture's centre
	 *
	 * Other values are stored as given and draw as 2. With automap true only 2
	 * (or another value that draws as 2) puts the picture on its map point; 0
	 * and 1 shift it sideways.
	 */
	int align;

	/** The z-order of the Image (what it covers up and is covered by).
	 *
	 * Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so
	 * a higher value covers a lower one; hooks with equal values draw in no set
	 * order. A click reaches every hook under it, whatever its zorder.
	 */
	int zorder;

	/** The click handler that gets called when the Image is clicked. Assign
	 * null or undefined to remove it; a value that is not a function is
	 * ignored.
	 *
	 * It is called as click(button, x, y) when a mouse button is pressed over
	 * the Image: button 0 left, 1 right, 2 middle; x and y are window pixels.
	 * Only a boolean `true` return keeps the press from the game; the release
	 * always reaches the game. The handler runs when the owning script next
	 * processes events, and the game gets the press anyway if the handler has
	 * not answered within 3 seconds. A paused or ended script's hooks get no
	 * clicks.
	 *
	 * The Image gets clicks while its `visible` property is true, even when it
	 * is not drawn at the moment (other game state, automap closed, x or y of
	 * -1). The click area uses the raw x and y (the automap conversion is not
	 * applied) and spans y - height/2 to y + height/2. With align 1 and 2 it
	 * matches the picture; with align 0 it is one picture-width to the right of
	 * the picture. An Image whose file did not load (a blank Image) gets no
	 * clicks.
	 */
	ClickHandler click;

	/** A hover handler for the Image. D2BS keeps it and returns it from
	 * `hover`, but never calls it. Assign null or undefined to remove it; a
	 * value that is not a function is ignored.
	 */
	HoverHandler hover;

	/** Location of the file to load for display.
	 *
	 * It is relative to the script folder (an uncompressed 8-bit bottom-up BMP,
	 * as for new Image()). Setting a name that new Image() would refuse throws
	 * "Invalid image file path" and keeps the current file; a name that does
	 * not load leaves the Image blank. Reading it returns the name as it was
	 * given; assigning a value that is not a string is ignored.
	 */
	String location;
};
