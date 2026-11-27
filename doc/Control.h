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

	/** The Control's display state: its flag word (see disabled) minus 2, read
	 * as an unsigned number.
	 *
	 * Common reads: 3 = shown and enabled, 2 = shown but greyed, 11 or 10 = the
	 * same on the popup layer. A hidden control reads 1 or 0 (7 or 6 on the
	 * popup layer), or 4294967295 or 4294967294 when its flag word is below 2.
	 *
	 * The flag word is a set of bits: 0x01 enabled (takes clicks; a button
	 * without it is drawn greyed), 0x04 shown (drawn and clickable), 0x08 popup
	 * layer (drawn after every other control; popups have it), and 0x02, a
	 * transient bit the game sets while a button is being released.
	 *
	 * Setting 0-3 replaces the whole flag word with state + 2: 3 shows and
	 * enables the control, 2 shows it greyed, and 1 and 0 hide it. Setting it
	 * also removes the popup-layer bit. States 0 and 1 also set a transient bit
	 * the game uses while a button is being released, with effects that are not
	 * fully known, so prefer 2 and 3. Any other integer throws "Invalid state
	 * value", and a non-integer value is ignored.
	 *
	 * Reads undefined once the control is gone.
	 */
	double state;

	/** Return whether or not the Control holds a password (starred out text).
	 */
	bool password;

	/** The type of control.
	 *
	 * | Value | Type | Notes |
	 * |---|---|---|
	 * | 1 | Edit box | Single-line text input (account, password, game name); its text is writable |
	 * | 2 | Image | Image or picture |
	 * | 3 | (unused) | Never constructed in 1.14d |
	 * | 4 | Text box | Multi-line text or label box; read it with getText() |
	 * | 5 | Scroll bar | |
	 * | 6 | Button | |
	 * | 7 | List | List box |
	 * | 8 | Timer | Invisible timer control |
	 * | 9 | Video | Smacker video player |
	 * | 10 | Progress bar | |
	 * | 11 | Popup | |
	 * | 12 | Account list | |
	 */
	double type;

	/** The position of the text cursor in an edit box. Reading it returns the current position, and setting it stores a new one. On any other control type it reads as undefined and ignores writes. */
	double cursorpos;

	/** On an edit box, the character index of the selection's moving end.
	 *
	 * It follows the cursor while the selection is extended with Shift or the
	 * mouse. Despite its name it is not where the selection began; that is
	 * selectend. The selection is the text between selectstart and selectend,
	 * in whichever order they fall; nothing is selected when selectend is
	 * 4294967295 or equals selectstart. It reads 0 once the selection is
	 * cleared (moving the cursor with the keyboard without Shift clears it), so
	 * it is meaningful only while a selection exists.
	 *
	 * Read-only. On other control types the value has no selection meaning.
	 * Reads undefined once the control is gone.
	 */
	double selectstart;

	/** On an edit box, the character index where the current selection began
	 * (its fixed end).
	 *
	 * Reads 4294967295 when no selection has been started. Nothing is selected
	 * either when selectend equals selectstart. The selection is the text
	 * between selectend and selectstart, in whichever order they fall. Select
	 * all gives selectend 0 and selectstart equal to the text length.
	 *
	 * Read-only. On other control types the value has no selection meaning.
	 * Reads undefined once the control is gone.
	 */
	double selectend;

	/** Whether the Control is disabled or not.
	 */
	double disabled;
};
