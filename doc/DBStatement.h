/** A prepared SQL statement on an SQLite database.
 *
 * SQLite.query(sql, ...) creates one, binding any extra arguments to its parameters in order, and SQLite.statements
 * lists the ones still open; a DBStatement cannot be constructed directly (new DBStatement() throws "Invalid
 * Operation"). query() prepares only the first SQL statement in the string, and throws "Statement has no effect" for
 * an empty one.
 *
 * Step through the result rows with next(), which returns true while a row is ready and false at the end, and read
 * the current row with getObject(), getColumnValue() or getColumnName(). Calling next() again after it returned
 * false runs the statement again from its first row. Use go() instead to run a statement such as an INSERT or UPDATE
 * once: it returns true when the statement ran to completion, and false when its step produced a row (a SELECT run
 * with go() returns false when it finds a row and true when it finds none; either way the statement is closed after
 * that one step). bind() sets a parameter (1-based index, or a name including its prefix such as ":id"), skip()
 * steps over rows and reset() rewinds the statement so it can run again.
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
	 * \return true when the statement ran to completion; false when its step produced a row.
	 */
	bool go();

	/** Call sqlite3_step on the statement.
	 *
	 * Moves to the next row of the results; ready tells whether there is one.
	 *
	 * \return true when a row is ready; false at the end of the results. A further call after false runs the
	 * statement again from its first row.
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

	/** Bind the given data to the given parameter.
	 *
	 * \param colnum The parameter to bind to: its 1-based number, or its name including the prefix (for example
	 * ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".
	 *
	 * \param val The data to bind.
	 *
	 * \return true when the value was bound. Throws when colnum is out of range, when val is an object or a function,
	 * or when the statement has been stepped with next() or skip() since it was created or last reset().
	 */
	bool bind(int colnum, double val);

	/** Bind the given data to the given parameter.
	 *
	 * \param colnum The parameter to bind to: its 1-based number, or its name including the prefix (for example
	 * ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".
	 *
	 * \param val The data to bind.
	 *
	 * \return true when the value was bound. Throws when colnum is out of range, when val is an object or a function,
	 * or when the statement has been stepped with next() or skip() since it was created or last reset().
	 */
	bool bind(int colnum, int val);

	/** Bind the given data to the given parameter.
	 *
	 * \param colnum The parameter to bind to: its 1-based number, or its name including the prefix (for example
	 * ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".
	 *
	 * \param val The data to bind.
	 *
	 * \return true when the value was bound. Throws when colnum is out of range, when val is an object or a function,
	 * or when the statement has been stepped with next() or skip() since it was created or last reset().
	 */
	bool bind(int colnum, String val);

	/** Bind the given boolean to the given parameter as the integer 1 (true) or 0 (false). A bind() of null or
	 * undefined binds NULL.
	 *
	 * \param colnum The parameter to bind to: its 1-based number, or its name including the prefix (for example
	 * ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".
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
