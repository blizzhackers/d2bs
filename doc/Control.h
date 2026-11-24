/** This class represents out of game (OOG) Control s.
 *
 * A Control whose control is gone (the screen changed, or the game left the
 * menu) reads every property as undefined and ignores writes; getNext() then
 * returns false and click() and setText() do nothing.
 */
class Control
{
public:
	/** Gets the next Control from the linked list. This is done by finding the
	 * old Control based on type, location and size.
	 */
	Control getNext();

	/** Click Control in the center of the Control.
	 */
	void click();

	/** Click Control at the given location. This is equivalent with clicking at
	 * the given location.
	 *
	 * \param x The x coordinate of the point to click.
	 *
	 * \param y The y coordinate of the point to click.
	 */
	void click(uint32_t x, uint32_t y);

	/** Set the Control's text.
	 *
	 * \param text The text to set the Control's text field to.
	 *
	 * Throws "Invalid arguments" if no text is passed to a control that is still on screen.
	 */
	void setText(String text);

	/** Get the texts from a label Control. Only works for labels.
	 *
	 * \return An array of all the Control's texts.
	 */
	String[] getText();

	/** The text of the Control.
	 */
	String text;

	/** The x coordinate of the Control.
	 */
	double x;

	/** The y coordinate of the Control.
	 */
	double y;

	/** The xsize (width) of the Control.
	 */
	double xsize;

	/** The ysize (height) of the Control.
	 */
	double ysize;

	/** The state (?) of the Control.
	 *
	 * \todo Find a reference for these values.
	 */
	double state;

	/** Return whether or not the Control holds a password (starred out text).
	 */
	bool password;

	/** The type of control.
	 *
	 * 1 - TextBox
	 *
	 * 2 - Image
	 *
	 * 3 - Image2
	 *
	 * 4 - LabelBox
	 *
	 * 5 - ScrollBar
	 *
	 * 6 - Button
	 *
	 * 7 - List
	 *
	 * 8 - Timer
	 *
	 * 9 - Smack
	 *
	 * 10 - ProgressBar
	 *
	 * 11 - Popup
	 *
	 * 12 - AccountList
	 */
	double type;

	/** The position of the text cursor in an edit box. Reading it returns the current position, and setting it stores a new one. On any other control type it reads as undefined and ignores writes. */
	double cursorpos;

	/** Someting...
	 *
	 * \todo Determine what this is.
	 */
	double selectstart;

	/** Something...
	 *
	 * \todo Determine what this is.
	 */
	double selectend;

	/** Whether the Control is disabled or not.
	 */
	double disabled;
};
