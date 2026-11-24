/** Class for dealing with an SQLite database.
 */
class SQLite
{
public:
	/** Create a database object which can be used to open SQLite databases and
	 * execute queries.
	 *
	 * \param path The path to the database to open.
	 *
	 * \param autoOpen Whether to open the database immediately (default true; only a boolean is read). A failed
	 * open throws "Could not open database: " followed by SQLite's message.
	 */
	SQLite(String path = ":memory:", bool autoOpen = true);

	/** Execute a query on the database.
	 *
	 * \param queryString The query string to execute.
	 *
	 * \return true
	 */
	bool execute(String queryString);

	/** Prepare sql on the database and bind the remaining arguments to its parameters in order: strings as text,
	 * numbers as integers or reals, true/false as 1/0, and null or undefined as NULL.
	 *
	 * \param sql The SQL to prepare.
	 *
	 * \return The resulting DBStatement. Throws when sql does not prepare or holds no statement, when an argument
	 * is an object or a function, or when there are more arguments than sql has parameters (an explicit undefined
	 * counts as an argument).
	 */
	DBStatement query(String sql, ...);

	/** Open the database. Opening an open database does nothing. After close(), it opens the same path again;
	 * statements from before the close stay closed, and a ":memory:" database starts empty.
	 *
	 * \return true. Throws "Could not open database: " followed by SQLite's message when SQLite cannot open the path.
	 */
	bool open();

	/** Close the database and every statement from query(). Closing a closed database does nothing.
	 *
	 * \return true. Throws "Could not close database: " followed by SQLite's message when SQLite cannot close it.
	 */
	bool close();

	/** Get the path used to open the database.
	 */
	String path;

	/** Get the statements from query() that are still open, from index 0. A statement finished by go(), closed with
	 * close() or closed by closing the database is left out.
	 */
	DBStatement[] statements;

	/** Whether or not the database is open.
	 */
	bool isOpen;

	/** Get the row id of the last inserted row (exact up to 2^53), or 0 when the database is not open.
	 */
	double lastRowId;

	/** Get the number of rows modified/inserted/deleted by the last statement, or 0 when the database is not open.
	 */
	double changes;
};
