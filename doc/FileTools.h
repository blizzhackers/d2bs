/** A collection of tools to do things like move files and determine whether
 * files exist.
 */
class FileTools
{
public:
	/** Remove (delete) a file.
	 *
	 * \param file The filename of the file to delete.
	 *
	 * \return true if the file was deleted, false if it could not be (for example it does not exist).
	 * An invalid file name still throws.
	 */
	static bool remove(String file);

	/** Rename a file.
	 *
	 * \param orig The original filename.
	 *
	 * \param newName The new filename. An existing file of that name is not replaced.
	 *
	 * \return true if the file was renamed, false otherwise. An invalid file name still throws.
	 */
	static bool rename(String orig, String newName);

	/** Copy a file byte for byte.
	 *
	 * \param orig The original filename.
	 *
	 * \param newName The filename of the new file.
	 *
	 * \param overwrite Replace newName if it already exists. Defaults to false, which leaves an existing file
	 * untouched and returns false.
	 *
	 * \return true if the copy was made, false otherwise (missing source, existing target without overwrite,
	 * copying a file onto itself, or a read/write failure, after which no partial file is left). With overwrite
	 * true, a copy that fails while reading or writing also leaves no newName (the old file was already
	 * replaced). An invalid file name still throws.
	 */
	static bool copy(String orig, String newName, bool overwrite = false);

	/** Determine if a file exists.
	 *
	 * \param file The filename of the file to check.
	 *
	 * \return Whether or not the file exists.
	 */
	static bool exists(String file);

	/** Read the contents of a file into a String.
	 *
	 * \param file The filename of the file to open.
	 *
	 * \return The contents of the file.
	 * Throws if the file is 256 MB or larger.
	 */
	static String readText(String file);

	/** Write the string representation (for standard files) or the binary
	 * representation (for binary files) of each of the arguments to the file.
	 *
	 * \param file The filename of the file to write to.
	 *
	 * \return Whether or not the operation was successful.
	 */
	bool writeText(String file, ...);

	/** Append the string representation (for standard files) or the binary
	 * representation (for binary files) of each of the arguments to the file.
	 *
	 * \param file The filename of the file to write to.
	 *
	 * \return Whether or not the operation was successful.
	 */
	bool appendText(String file, ...);
};
