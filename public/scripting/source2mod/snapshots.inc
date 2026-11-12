/**
 * Source2Mod SourcePawn API: snapshot inspector (ISource2ModSnapshots).
 *
 * What an entity sends to the clients, as the engine encodes it for a
 * snapshot: only its networked fields, components flattened in, values as they
 * come out of the encoders. That is the full state a client would get now,
 * before send proxies leave fields out for particular players.
 *
 * Each call encodes the entity once -- about what one entity in a snapshot
 * costs. Fine for commands and checks, not for every entity every tick.
 * The server console has the same as "s2m snapshot <entity index> [field | dump]".
 */

#if defined _source2mod_snapshots_included
 #endinput
#endif
#define _source2mod_snapshots_included

#include <source2mod/types>
#include <source2mod/schema>

/**
 * One field the entity sends.
 *
 * @param path   The packed field path
 * @param name   The network name ("m_pInGameMoneyServices.m_iAccount", "m_perRoundStats[0].m_iKills")
 * @param type   The field's type ("int32", "CHandle< CBasePlayerPawn >")
 * @param value  The value as the engine prints it ("16000", "Solman", "true")
 */
typeset SnapshotFieldCallback
{
	function void (int path, const char[] name, const char[] type, const char[] value, any data);
	function void (int path, const char[] name, const char[] type, const char[] value);
};

/** One line of the engine's own dump of the encoded entity. */
typeset SnapshotLineCallback
{
	function void (const char[] line, any data);
	function void (const char[] line);
};

methodmap Source2ModSnapshots
{
	/** Whether the engine's serializer interfaces are there. */
	public static native bool IsAvailable();

	/** The entity's network class ("CCSPlayerPawn"); false when it is not networked. */
	public static native bool GetNetworkClassName(CBaseEntity entity, char[] buffer, int maxlen);

	/**
	 * Encodes the entity now and calls back for every field that goes out.
	 *
	 * @return  The number of fields, -1 when the entity cannot be encoded
	 */
	public static native int GetFields(CBaseEntity entity, SnapshotFieldCallback callback, any data = 0);

	/**
	 * One field's encoded value by its network name, as GetFields() names it.
	 *
	 * @return  false when the entity does not send that field now
	 */
	public static native bool GetField(CBaseEntity entity, const char[] name, char[] value, int maxlen);

	/**
	 * The engine's own dump of the encoded entity, line by line.
	 *
	 * @return  The number of lines, -1 when the entity cannot be encoded
	 */
	public static native int Dump(CBaseEntity entity, SnapshotLineCallback callback, any data = 0);
}
