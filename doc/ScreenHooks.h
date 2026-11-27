/** \page shook ScreenHook documentation
 * \tableofcontents
 * \section opacity Explanation of opacity parameter.
 * A Box's `opacity` is the game's draw mode for the rectangle (default 0).
 * Only 0-7 are supported; the value is not range-checked, so use nothing
 * else. The effects below were measured in a 1.14d single-player game
 * running windowed (-w), where the game draws through its GDI renderer;
 * other video modes were not checked. Blended results are snapped to the
 * game's 256-color palette.
 *
 * | Mode | Effect |
 * |---|---|
 * | 0 | The color blended over what is underneath, about 25% color (the default) |
 * | 1 | The color blended over what is underneath, about 50% color |
 * | 2 | The color blended over what is underneath, about 75% color |
 * | 3 | Nothing visible: what is underneath is unchanged |
 * | 4 | Black: the color is ignored |
 * | 5 | The color, solid |
 * | 6 | The color is ignored and what is underneath is darkened through a palette table (some colors stay as they are) |
 * | 7 | The color, solid (the same as 5) |
 *
 */

/** The type of a click handler.
 *
 * \ingroup handlers
 *
 * \param button The button clicked.
 *
 * \param x The x coordinate clicked.
 *
 * \param y The y coordinate clicked.
 */
typedef ClickHandler (bool*)(int button, int x, int y);

/** The type of a hover handler. D2BS never calls a hover handler; it is kept
 * for scripts that read it back.
 *
 * \ingroup handlers
 *
 * \param x The x coordinate hovered.
 *
 * \param y The y coordinate hovered.
 */
typedef HoverHandler (void*)(int x, int y);
