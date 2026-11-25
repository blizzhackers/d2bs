#include "ScreenHooks.h"

/** A Text screen hook
 *
 * Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `color`, `font` and `zorder` keep only the low 16 bits, so -1 reads back 65535.
 *
 * \todo Verify all the documentation in this class.
 */
class Text
{
public:
	/** Create a Text hook with the given parameters.
	 *
	 * \param szText The string that the Text hook displays.
	 *
	 * \param x The x coordinate (left) of the Text.
	 *
	 * \param y The y coordinate (top) of the Text.
	 *
	 * \param color The color of the Text.
	 *
	 * See: http://forums.d2botnet.org/viewtopic.php?f=18&t=1000
	 *
	 * \param font Something that determines the font.
	 *
	 * \param align The horizontal alignment.
	 *
	 * 0 - Left
	 *
	 * 1 - Right
	 *
	 * 2 - Center
	 *
	 * \param automap Whether the Text is in automap coordinate space (true) or
	 * screen coordinate space (false).
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

	/** Remove the Text hook from the screen and destroy the corresponding
	 * object.
	 */
	void remove();

	/** The x coordinate (left) of the Text.
	 */
	int x;

	/** The y coordinate (top) of the Text.
	 */
	int y;

	/** Whether or not the Text is visible.
	 */
	bool visible;

	/** The horizontal alignment.
	 *
	 * 0 - Left
	 *
	 * 1 - Right
	 *
	 * 2 - Center
	 */
	int align;

	/** The z-order of the Text (what it covers up and is covered by).
	 */
	int zorder;

	/** The click handler that gets called when the Text is clicked. Assign null
	 * or undefined to remove it; a value that is not a function is ignored.
	 */
	ClickHandler click;

	/** A hover handler for the Text. D2BS keeps it and returns it from `hover`,
	 * but never calls it. Assign null or undefined to remove it; a value that
	 * is not a function is ignored.
	 */
	HoverHandler hover;

	/** The color of the Text.
	 *
	 * See: http://forums.d2botnet.org/viewtopic.php?f=18&t=1000
	 */
	int color;

	/** Something that determines the font.
	 */
	int font;

	/** The string that the Text hook displays.
	 */
	String text;
};
