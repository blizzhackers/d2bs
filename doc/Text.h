#include "ScreenHooks.h"

/** A Text screen hook
 *
 * A Text draws a string with one of the game's fonts on the game screen or
 * the automap. Create one with
 * `new Text(text, x, y, color, font, align, automap, click, hover)`; every
 * argument is optional (defaults "", 0, 0, 0, 0, 0, false, none, none). y is
 * the bottom of the first line. Each `\n` in the text starts a new
 * line above the previous one, so the first line is the bottom one and later
 * lines stack upward. Color codes in the text (the character 0xFF, written
 * `\xFF` in a script string, followed by `c` and a code character) change the
 * color from that point on; see `color`. A Text made by an out-of-game script
 * draws only at the menus; one made in game (or from the console) draws only
 * in game. With `automap` true (a constructor argument only), x and y are game
 * map coordinates (the units of unit.x and unit.y) and the Text is drawn only
 * while the automap is open; only align 0 then puts the text on its map
 * point, while 1 and 2 shift it sideways.
 *
 * Hooks of every script and every hook class are drawn in ascending `zorder`
 * (default 1), so a higher value draws on top. Set `visible` to hide or show
 * the Text, and call remove() to delete it. Keep a reference to every hook:
 * one whose object is garbage-collected is removed. When the script that made
 * a hook ends, its hooks leave the screen (with the default
 * `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that
 * ended normally keeps its hooks drawn, but not clickable, until the script
 * cache is flushed).
 *
 * Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `color`, `font` and `zorder` keep only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value is ignored.
 */
class Text
{
public:
	/** Create a Text hook with the given parameters.
	 *
	 * \param szText The string that the Text hook displays. A value that is not
	 * a string shows nothing (pass String(n) for a number).
	 *
	 * \param x The x coordinate of the text's anchor: its left edge (align 0),
	 * right edge (align 1) or centre (align 2).
	 *
	 * \param y The y coordinate of the bottom of the first line; further lines
	 * stack upward.
	 *
	 * \param color The text color, 0-12 (see \ref reference_textcolors).
	 *
	 * \param font The game font, 0-13 (see \ref reference_fonts). Values of 14
	 * or more are invalid and can crash the game.
	 *
	 * \param align The horizontal alignment.
	 *
	 * 0 - Left
	 *
	 * 1 - Right
	 *
	 * 2 - Center
	 *
	 * \param automap true to place the Text at game map coordinates and draw it
	 * only while the automap is open; false (the default) for screen
	 * coordinates. Only true or false is accepted.
	 *
	 * \param click A function to call when the Text is clicked; any other
	 * value sets no handler.
	 *
	 * \param hover A hover function for the Text, kept but never called; any
	 * other value sets no handler.
	 */
	Text(String szText = "", int x = 0, int y = 0, int color = 0, int font = 0,
		int align = 0, bool automap = false, ClickHandler click = null,
		HoverHandler hover = null);

	/** Remove the Text hook from the screen for good.
	 *
	 * The object stays and its click and hover handlers are released. Its
	 * properties then read undefined, writes to them are ignored, and calling
	 * remove() again does nothing. Returns undefined.
	 */
	void remove();

	/** The x coordinate of the Text.
	 *
	 * It is the text's left edge with align 0, its right edge with align 1 and
	 * its centre with align 2. A map coordinate when the Text was made with
	 * automap true. While x or y is -1 the Text is not drawn.
	 */
	int x;

	/** The y coordinate of the bottom of the Text's first line.
	 *
	 * Each further line (after a `\n`) is drawn above the previous one, so the
	 * block grows upward from y. A map coordinate when the Text was made with
	 * automap true. While x or y is -1 the Text is not drawn.
	 */
	int y;

	/** Whether or not the Text is visible.
	 *
	 * Defaults to true. A hidden Text is not drawn and gets no clicks. Only
	 * true or false is accepted.
	 */
	bool visible;

	/** The horizontal alignment.
	 *
	 * 0 - Left: x is the text's left edge
	 *
	 * 1 - Right: x is the text's right edge
	 *
	 * 2 - Center: x is the text's centre
	 *
	 * Other values are stored as given and draw as 0. With automap true only 0
	 * (or another value that draws as 0) puts the text on its map point; 1 and
	 * 2 shift it sideways.
	 */
	int align;

	/** The z-order of the Text (what it covers up and is covered by).
	 *
	 * Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so
	 * a higher value covers a lower one; hooks with equal values draw in no set
	 * order. A click reaches every hook under it, whatever its zorder.
	 */
	int zorder;

	/** The click handler that gets called when the Text is clicked. Assign null
	 * or undefined to remove it; a value that is not a function is ignored.
	 *
	 * It is called as click(button, x, y) when a mouse button is pressed over
	 * the Text: button 0 left, 1 right, 2 middle; x and y are window pixels.
	 * Only a boolean `true` return keeps the press from the game; the release
	 * always reaches the game. The handler runs when the owning script next
	 * processes events, and the game gets the press anyway if the handler has
	 * not answered within 3 seconds. A paused or ended script's hooks get no
	 * clicks.
	 *
	 * The Text gets clicks while its `visible` property is true, even when it
	 * is not drawn at the moment (other game state, automap closed, x or y of
	 * -1). The click area is the text's width at its aligned x, from y minus
	 * the height of all its lines up to y (edges excluded), on the raw x and y:
	 * the automap conversion is not applied.
	 */
	ClickHandler click;

	/** A hover handler for the Text. D2BS keeps it and returns it from `hover`,
	 * but never calls it. Assign null or undefined to remove it; a value that
	 * is not a function is ignored.
	 */
	HoverHandler hover;

	/** The color of the Text.
	 *
	 * 0-12 select the game's text colors; see \ref reference_textcolors for
	 * their names. Other values are not defined. Color codes in the text
	 * override it from that point on: the character 0xFF (written `\xFF` in a
	 * script string) followed by `c` and one of `0`-`9`, `:`, `;` or `<`
	 * selects colors 0-12; a code character after `<` selects 0, and one before
	 * `0` gives an undefined color.
	 */
	int color;

	/** The game font, 0-13.
	 *
	 * See \ref reference_fonts for the font names. Values of 14 or more are
	 * invalid: the game reads past its font table and can crash. D2BS does not
	 * check the value.
	 */
	int font;

	/** The string that the Text hook displays.
	 *
	 * Each `\n` starts a new line above the previous one (the first line is the
	 * bottom one), and color codes (see `color`) change the color from that
	 * point on. Assigning a value that is not a string is ignored.
	 */
	String text;
};
