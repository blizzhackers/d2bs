/** Represents the items in the list in the Party screen. Also called a
 * RosterUnit.
 *
 * A Party object follows its player and reads the current values on every
 * access. Once that player leaves the game, or the game ends, its properties
 * read undefined, getNext() returns false and clickParty() returns false.
 */
class Party
{
public:
	/** The x coordinate of the location of the RosterUnit.
	 */
	int x;

	/** The y coordinate of the location of the RosterUnit.
	 */
	int y;

	/** The Area ID of the RosterUnit.
	 */
	int area;

	/** The unique id of the RosterUnit.
	 */
	double gid;

	/** The life of the RosterUnit as a percentage of its maximum life, 0 to
	 * 100.
	 *
	 * It is kept current only for members of your party; for other players it
	 * can be stale, and your own entry is not reliable either. The roster holds
	 * neither the player's absolute life nor their maximum life, so there is no
	 * maximum-life field. For your own character, use me.hp and me.hpmax.
	 */
	int life;

	/** The state of the party button for this player, as your character sees
	 * it.
	 *
	 * The value is a single state, not a set of bits:
	 *
	 * | Value | Meaning |
	 * |---|---|
	 * | 0 | The player is in no party and has no pending invite: clickParty(party, 2) invites them |
	 * | 1 | The player is already in a party (yours or another; compare partyid to tell which); your own entry also reads 1. clickParty(party, 2) returns false without acting |
	 * | 2 | The player has invited you: clickParty(party, 2) accepts and you join |
	 * | 4 | You have invited the player and they have not answered: clickParty(party, 2) cancels the invite |
	 *
	 * The game's client also accepts 3 (clicking acts like accepting an
	 * invite); 1.14d's own server never sends it, and whether a Battle.net
	 * realm does is unknown.
	 *
	 * clickParty(party, 2) also returns false for your own entry and for a
	 * member of your own party.
	 */
	int partyflag;

	/** The id of the party this player belongs to, or 65535 when they are in no
	 * party.
	 *
	 * The game numbers each party; two players are in the same party exactly
	 * when their partyid values are equal and not 65535. To list your party,
	 * compare each entry's partyid with your own (getParty(me).partyid). Your
	 * own entry reads 65535 as soon as you leave a party.
	 */
	int partyid;

	/** The name of the RosterUnit.
	 */
	String name;

	/** The class ID of the RosterUnit. See Unit::classId.
	 */
	int classid;

	/** The character level of the RosterUnit.
	 */
	int level;

	/** Move to the next RosterUnit.
	 *
	 * \return This object, now on the next player; or false at the end of the
	 * list, or when this object's player has left or its game has ended.
	 */
	Party getNext();
};
