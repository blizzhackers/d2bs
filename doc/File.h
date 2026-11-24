/** This class handles the opening, reading and writing of files.
 */
class File
{
public:
	/** Close the current file and then return it.
	 *
	 * \return The same File object.
	 */
	File close();

	/** Reopen the file, equivalent to calling open with the same parameters
	 * as were used the first time.
	 *
	 * \return The same File object.
	 */
	File reopen();

	/** Read count chars from the file. Used when the file was not opened in
	 * binary mode.
	 *
	 * \param count The number of characters to read.
	 *
	 * \return The characters that were read.
	 * Throws if the file is not open (it was closed).
	 */
	String read(int count);

	/** Read count 32-bit integers from the file. Used when the file was opened in
	 * binary mode.
	 *
	 * \param count The number of 4-byte little-endian integers to read.
	 *
	 * \return One integer when count is 1 (0 at the end of the file), otherwise an
	 * array of the integers read, which is shorter than count at the end of the
	 * file. Binary write stores integers as 4 bytes but other numbers as 8-byte
	 * doubles and booleans as 1 byte, so only integers read back unchanged.
	 * Throws if the file is not open (it was closed).
	 */
	int[] read(int count);

	/** Read a line from the file and return it.
	 *
	 * \return The line.
	 * Throws if the file is not open (it was closed).
	 */
	String readLine();

	/** Read all the lines from the file into a String array.
	 *
	 * \return An array of all the lines in the file.
	 * Throws if the file is not open (it was closed).
	 */
	String[] readAllLines();

	/** Read the entire file and return as a String. Includes a seek to start
	 * from the beginning of the file, regardless of where the current position
	 * was.
	 *
	 * \return The entire file in a String.
	 * Throws if the file is 256 MB or larger.
	 * Throws if the file is not open (it was closed).
	 */
	String readAll();

	/** Write the string representation (for standard files) or the binary
	 * representation (for binary files) of each of the arguments to the File.
	 *
	 * \return The File.
	 */
	File write(...);

	/** Seek n bytes from the current location.
	 *
	 * \param n Number of bytes to move..
	 *
	 * \return The File.
	 */
	File seek(int n);

	/** Seek n bytes or lines (depending on isLines) from the current location
	 * or the start of file (depending on fromStart).
	 *
	 * \param n Number of bytes or lines to move.
	 *
	 * \param isLines Whether n is a number of lines, or just bytes.
	 *
	 * \param fromStart Whether to start from the start of file, or the current
	 * location.
	 *
	 * \return The File.
	 */
	File seek(int n, bool isLines, bool fromStart = false);

	/** Flush the buffers for the File.
	 *
	 * \return The File.
	 */
	File flush();

	/** Seek back to the beginning of the File.
	 *
	 * \return The File.
	 */
	File reset();

	/** Seek to the end of the File.
	 *
	 * \return The File.
	 */
	File end();

	/** Open a file with the given settings.
	 *
	 * Uses defaults: binary = false, autoflush = false, lockFile = false.
	 *
	 * \param file The filename.
	 *
	 * \param mode The mode of the file (read/write/append). Use FILE_READ,
	 * FILE_WRITE and FILE_APPEND.
	 * FILE_READ corresponds to a C mode of "r".
	 * FILE_WRITE corresponds to a C mode of "w+".
	 * FILE_APPEND corresponds to a C mode of "a+".
	 *
	 * \return The File object just created.
	 */
	static File open(String file, int mode);

	/** Open a file with the given settings.
	 *
	 * \param file The filename.
	 *
	 * \param mode The mode of the file (read/write/append). Use FILE_READ,
	 * FILE_WRITE and FILE_APPEND.
	 * FILE_READ corresponds to a C mode of "r".
	 * FILE_WRITE corresponds to a C mode of "w+".
	 * FILE_APPEND corresponds to a C mode of "a+".
	 *
	 * \param binary Whether to open the file in text or binary mode. If opening
	 * in text mode a "t" is appended to the C mode, otherwise a "b" is
	 * appended.
	 *
	 * \param autoflush Whether to flush after each write.
	 *
	 * \param lockFile Whether or not to lock the file.
	 *
	 * \return The File object just created.
	 */
	static File open(String file, int mode, bool binary = false,
		bool autoflush = false, bool lockFile = false);

	/** Whether or not the file can be read from. Based on whether the File is
	 * open, whether it is at the end of file, and whether there is an error
	 * pending.
	 */
	bool readable;

	/** Whether or not the file is writeable. Based on whether the File is open,
	 * whether there is an error pending and whether the File was opened for
	 * writing or not.
	 */
	bool writeable;

	/** Whether or not the file is seekable. Based on whether the File is open
	 * and whether there is an error pending.
	 */
	bool seekable;

	/** The mode the file was opened with. Compare with FILE_READ, FILE_WRITE
	 * and FILE_APPEND.
	 */
	int mode;

	/** Whether the file was opened as a binary file or not.
	 */
	bool binaryMode;

	/** The file length.
	 */
	int length;

	/** The relative path to the File (the filename/path given for opening).
	 */
	String path;

	/** The position in the File (where the next read or write will occur).
	 */
	int position;

	/** Whether or not the File is at end of file.
	 */
	bool eof;

	/** The last accessed time of the File.
	 */
	int accessed;

	/** The creation time of the File.
	 */
	int created;

	/** The last modified time of the File.
	 */
	int modified;

	/** Whether or not to autoflush the File.
	 */
	bool autoflush;
};
