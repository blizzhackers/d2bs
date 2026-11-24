/** A class describing ...
 *
 * \todo Get a better description (and understanding).
 *
 * The statement is closed after close(), after a go() that returns, and after the database's close(): every method
 * except close() throws "The statement is closed", sql reads undefined and ready reads false. When go()'s step
 * reports an error, go() throws and the statement stays open, not ready.
 */
class DBStatement
{
public:
	/** An object with property names from the column names and property values
	 * coming from the data.
	 * The same object is returned until the statement moves to another row or closes.
	 *
	 * \return The row object, or null when the statement is not ready.
	 */
	Object getObject();

	/** Get the column count.
	 *
	 * \return The column count.
	 */
	int getColumnCount();

	/** Get the value from the column specified by index.
	 *
	 * \param colIndex The index of the column to get data from.
	 *
	 * Throws "Invalid column index" if colIndex is not a column of the statement.
	 *
	 * \return The data.
	 */
	double getColumnValue(int colIndex);

	/** Get the value from the column specified by index.
	 *
	 * \param colIndex The index of the column to get data from.
	 *
	 * Throws "Invalid column index" if colIndex is not a column of the statement.
	 *
	 * \return The data.
	 */
	String getColumnValue(int colIndex);

	/** Get the name of the column at the given index.
	 *
	 * \param colIndex The index of the column to get data from.
	 *
	 * Throws "Invalid column index" if colIndex is not a column of the statement.
	 *
	 * \return The column name.
	 */
	getColumnName(int colIndex);

	/** Call sqlite3_step and close_db_stmt on the statement.
	 *
	 * Runs the statement once and closes it: ready is false afterwards and the statement can't be stepped again.
	 * When the step reports an error, go() throws and the statement stays open, not ready.
	 *
	 * \return Whether the result is SQLITE_DONE.
	 */
	bool go();

	/** Call sqlite3_step on the statement.
	 *
	 * Moves to the next row of the results; ready tells whether there is one.
	 *
	 * \return Whether the result is SQLITE_ROW.
	 */
	bool next();

	/** Skip a certain number of rows in the results.
	 * Stops at the end of the results; ready is true when it stopped on a row. A count of 0 or less skips nothing.
	 *
	 * \param rows The number of rows to skip.
	 *
	 * \return The number of rows actually skipped, which is less than rows at the end of the results.
	 */
	int skip(int rows);

	/** Call sqlite3_reset.
	 *
	 * Starts the results again from the first row; ready is false until next() or skip() lands on a row.
	 *
	 * \return true. After a failed next(), skip() or go() it throws that step's error; the statement is still rewound
	 * and not ready.
	 */
	bool reset();

	/** Close the statement.
	 *
	 * \return true, also when the statement was already closed.
	 */
	bool close();

	/** Bind the given data to the given column.
	 *
	 * \param colnum The column number to bind to.
	 *
	 * \param val The data to bind.
	 *
	 * \return true when the value was bound. Throws when colnum is out of range, when val is an object or a function,
	 * or when the statement has been stepped with next() or skip() since it was created or last reset().
	 */
	bool bind(int colnum, double val);

	/** Bind the given data to the given column.
	 *
	 * \param colnum The column number to bind to.
	 *
	 * \param val The data to bind.
	 *
	 * \return true when the value was bound. Throws when colnum is out of range, when val is an object or a function,
	 * or when the statement has been stepped with next() or skip() since it was created or last reset().
	 */
	bool bind(int colnum, int val);

	/** Bind the given data to the given column.
	 *
	 * \param colnum The column number to bind to.
	 *
	 * \param val The data to bind.
	 *
	 * \return true when the value was bound. Throws when colnum is out of range, when val is an object or a function,
	 * or when the statement has been stepped with next() or skip() since it was created or last reset().
	 */
	bool bind(int colnum, String val);

	/** Bind the given boolean to the given column as the integer 1 (true) or 0 (false). A bind() of null or
	 * undefined binds NULL.
	 *
	 * \param colnum The column number to bind to.
	 *
	 * \param val The data to bind.
	 *
	 * \return true when the value was bound. Throws when colnum is out of range, when val is an object or a function,
	 * or when the statement has been stepped with next() or skip() since it was created or last reset().
	 */
	bool bind(int colnum, bool val);

	/** The statement string, or undefined once the statement is closed. */
	String sql;

	/** Whether a row can be read now; false once the statement is closed. */
	bool ready;
};
