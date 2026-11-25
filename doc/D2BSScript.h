/** Represents a script (a thread), created by either loading a file, or typing
 * something in the console.
 */
class D2BSScript
{
public:
	/** Get the next script.
	 *
	 * \return Whether there was another script or not.
	 */
	bool getNext();

	/** Pause the script.
	 */
	void pause();

	/** Resume the script.
	 */
	void resume();

	/** Wait for the script to finish. Returns at once when it has already
	 * ended. Throws an error when called on the calling script itself.
	 */
	void join();

	/** Stop the script.
	 */
	void stop();

	/** Send some values to the script. Has the effect of calling any
	 * listeners to scriptmsg with the parameters provided. A value that can't
	 * be copied (a function, or a game object such as a Unit) throws an error
	 * and nothing is sent.
	 */
	void send(...);

	/** The relative filename of the script. If the script is from the console
	 * the filename will be reported as "Command Line".
	 */
	String name;

	/** The type of script.
	 *
	 * 0 - InGame
	 *
	 * 1 - OutOfGame or Command
	 */
	int type;

	/** Whether or not the script is running. False before the script has
	 * started (while it compiles and, after a reload, while it waits up to
	 * half a second for the old copy to end), while it is paused, after
	 * stop(), and after it has ended. A stopped or ended script stays listed
	 * until its thread ends (with DisableCache=false, until the cache is
	 * flushed or the file is loaded again). Once it is no longer listed, its
	 * name is an empty string, running stays false, its other members are
	 * undefined and its methods do nothing.
	 */
	bool running;

	/** The thread id.
	 */
	int threadid;
};
