#include "ScreenHooks.h"

/** This class represents a screen hook for a Image.
 *
 * Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. The `zorder` property and the `color` argument keep only the low 16 bits, so a `zorder` of -1 reads back 65535.
 *
 * \todo Explain (and understand) this better.
 * \todo Verify all the documentation in this class.
 */
class Image
{
public:
	/** Create an Image hook with the given parameters.
	 *
	 * \param szText The filename to load the image from, relative to the script folder. Only uncompressed 8-bit (256-color) bottom-up BMP files in the script folder load; a missing file or any other format leaves the Image blank, and a file fixed on disk loads the next time an Image or location names it. A name that has loaded once keeps that picture until Diablo II (Game.exe) exits, even if the file is rewritten on disk, and the same file under another spelling (letter case, .\, / instead of \) loads again as its own picture; a file that is deleted or cannot be opened (for example while another program holds it locked) leaves the Image blank and writes a line to the log. A name that leaves that folder (..\ or ../), contains any of ":?*<>|, or makes a path longer than 515 characters when joined to the script folder's path throws "Invalid image file path".
	 *
	 * \param x The x coordinate (left) of the Image.
	 *
	 * \param y The y coordinate (top) of the Image.
	 *
	 * \param color The color of the Image.
	 *
	 * See: http://forums.d2botnet.org/viewtopic.php?f=18&t=1000
	 *
	 * \param align The horizontal alignment.
	 *
	 * 0 - Left
	 *
	 * 1 - Right
	 *
	 * 2 - Center
	 *
	 * \param automap Whether the Image is in automap coordinate space (true) or
	 * screen coordinate space (false).
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

	/** Remove the Image from the screen and destroy the corresponding object.
	 */
	void remove();

	/** The x coordinate (left) of the Image.
	 */
	int x;

	/** The y coordinate (top) of the Image.
	 */
	int y;

	/** Whether or not the Image is visible.
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

	/** The z-order of the Image (what it covers up and is covered by).
	 */
	int zorder;

	/** The click handler that gets called when the Image is clicked. Assign
	 * null or undefined to remove it; a value that is not a function is
	 * ignored.
	 */
	ClickHandler click;

	/** A hover handler for the Image. D2BS keeps it and returns it from
	 * `hover`, but never calls it. Assign null or undefined to remove it; a
	 * value that is not a function is ignored.
	 */
	HoverHandler hover;

	/** Location of the file to load for display, relative to the script folder (an uncompressed 8-bit bottom-up BMP, as for new Image()). Setting a name that new Image() would refuse throws "Invalid image file path" and keeps the current file; a name that does not load leaves the Image blank.
	 */
	String location;
};
