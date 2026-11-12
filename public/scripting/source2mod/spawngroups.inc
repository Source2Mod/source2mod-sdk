/**
 * Source2Mod SourcePawn API: spawn groups (ISource2ModSpawnGroups).
 *
 * Another map, a prefab or a skybox loaded into the running map at an offset:
 * its world, its resources and -- unless told otherwise -- its entities. The
 * clients load it on their own. The world name is the map's path the way the
 * engine takes it: "de_mirage", "maps/prefabs/de_dust2/de_dust2_skybox",
 * "prefabs/misc/team_select"; it has to exist on the server and the clients.
 *
 * A plugin's groups are unloaded when it unloads; the engine unloads them all
 * when the map changes.
 */

#if defined _source2mod_spawngroups_included
 #endinput
#endif
#define _source2mod_spawngroups_included

#include <source2mod/types>
#include <source2mod/schema>

/** What Source2ModSpawnGroups.Load() takes as flags; combine with |. */
enum SpawnGroupLoadFlags
{
	SPAWNGROUP_LOAD_NONE = 0,
	/** Loads the world and its resources, but none of its entities. */
	SPAWNGROUP_LOAD_DONT_SPAWN_ENTITIES = (1 << 0),
	/** Spawns no entity of the map's own entity lump. */
	SPAWNGROUP_LOAD_SUPPRESS_MAP_ENTITIES = (1 << 1),
	/** Asks the engine to load it in one go; Load() still returns before it is ready: wait for the callback. */
	SPAWNGROUP_LOAD_BLOCK_UNTIL_LOADED = (1 << 2),
	/** Makes it the active spawn group once loaded. */
	SPAWNGROUP_LOAD_SET_ACTIVE = (1 << 3),
	/** Leaves the world_layer_base layer of the new world disabled. */
	SPAWNGROUP_LOAD_DISABLE_BASE_LAYER = (1 << 4),
	/** Does not load the world's streaming data (static props, lighting). */
	SPAWNGROUP_LOAD_NO_STREAMING_DATA = (1 << 5),
};

/**
 * Told once when a group is ready (loaded = true, its entities spawned) and once
 * when it is gone again (loaded = false: unloaded by someone else, or the map
 * changed). Not called for a group this plugin unloads itself.
 */
typeset SpawnGroupCallback
{
	function void (int group, bool loaded, any data);
	function void (int group, bool loaded);
};

methodmap Source2ModSpawnGroups
{
	/**
	 * Starts loading a spawn group into the running map.
	 *
	 * @param worldName       The world, see the top of this file
	 * @param origin          Where the world's origin goes (NULL_VECTOR: 0 0 0)
	 * @param angles          How it is turned (NULL_VECTOR: none)
	 * @param flags           SpawnGroupLoadFlags
	 * @param name            The name status lists and FindByName() looks up; "" for worldName
	 * @param callback        Told when it is ready and when it is gone
	 * @param data            Passed to the callback
	 * @param localNameFixup  Prefix for the group's own targetnames ("" for none)
	 * @return                The group, 0 when no map is running, the world does not exist
	 *                        (no maps/<world>.vpk or world.vwrld_c) or the engine refused it.
	 */
	public static native int Load(const char[] worldName, const float origin[3] = NULL_VECTOR, const float angles[3] = NULL_VECTOR,
		SpawnGroupLoadFlags flags = SPAWNGROUP_LOAD_NONE, const char[] name = "", SpawnGroupCallback callback = INVALID_FUNCTION,
		any data = 0, const char[] localNameFixup = "");

	/** Unloads a group with its entities; false when there is no such group. */
	public static native bool Unload(int group);

	/**
	 * A loaded or loading group by the name it was loaded under through
	 * Source2Mod, else by its world name (the map's own "de_dust2", its skybox,
	 * the engine's prefabs); 0 for none.
	 */
	public static native int FindByName(const char[] name);

	/** Whether the group is fully loaded, entities spawned. */
	public static native bool IsLoaded(int group);

	/** Whether the group is still on its way. */
	public static native bool IsLoading(int group);

	/** Makes the group the active one. */
	public static native bool MakeActive(int group);

	/** The group an entity belongs to; 0 for none. */
	public static native int GetEntitySpawnGroup(CBaseEntity entity);
}
