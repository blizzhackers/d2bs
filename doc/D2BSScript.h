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

	/** Pause the script. Messages sent to it with send(), and its
	 * setTimeout/setInterval callbacks, wait until resume() (or, in game, run
	 * while it waits inside a game call such as a skill change); each pending
	 * timer runs once then, however many periods passed. Game events that
	 * arrive while it is paused are not delivered.
	 */
	void pause();

	/** Resume the script.
	 */
	void resume();

	/** Wait for the script to finish. Returns at once when it has already
	 * ended. Throws an error when called on the calling script itself.
	 */
	void join();

	/** Stop the script. D2BS prints a line such as "Script default.dbj ended"
	 * once for each script other than the console, when it finishes or is
	 * stopped (a script that fails to start prints "failed to start" instead);
	 * stop() on a script that has already ended prints no second line.
	 */
	void stop();

	/** Send some values to the script. Has the effect of calling any
	 * listeners to scriptmsg with the parameters provided. The message reaches
	 * a script that is running or paused and has a scriptmsg listener, or one
	 * that has not started yet (just loaded, or waiting for the copy it
	 * replaces to end). A script that has not started usually gets the message
	 * at its first delay() or event, so a listener its top level adds before
	 * then receives it; an engine interrupt during its top level (a garbage
	 * collection, for example) can deliver it earlier, before such a listener
	 * exists. A paused script gets it once it resumes (or, in game, while it
	 * waits inside a game call such as a skill change).
	 * Nothing is copied or sent when the script has ended or is stopping, or
	 * has started (running or paused) without a scriptmsg listener. A value
	 * that can't be copied (a function, or a game object such as a Unit)
	 * throws an error and nothing is sent.
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
