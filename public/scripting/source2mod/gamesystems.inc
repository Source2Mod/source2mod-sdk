/**
 * Source2Mod SourcePawn API: game systems (ISource2ModGameSystems).
 *
 * A game system gets the engine's per-session events: BuildGameSessionManifest
 * when a map's resources are gathered (precaching), SpawnGroupPrecache for a
 * spawn group's, GameFrameBoundary once a frame, and the rest below. The system
 * belongs to Source2Mod; a script hooks the events it wants and registers it,
 * and the engine only calls it for those. It goes when the script unloads.
 *
 * @code
 * IGameSystem g_System;
 *
 * public void OnPluginStart()
 * {
 *     g_System = new IGameSystem("MyPlugin");
 *     g_System.Hook(GameSystemEvent_BuildGameSessionManifest, OnBuildManifest);
 *     g_System.Register();
 * }
 *
 * void OnBuildManifest(IGameSystem system, GameSystemEvent event, GameSystemMessage msg)
 * {
 *     msg.AddResource("models/example/thing.vmdl");
 * }
 * @endcode
 */

#if defined _source2mod_gamesystems_included
 #endinput
#endif
#define _source2mod_gamesystems_included

#include <source2mod/schema>

/**
 * The events a game system can hook. The value is the event's IGameSystem
 * vtable slot, the id the engine dispatches it by.
 */
enum GameSystemEvent
{
	GameSystemEvent_GameInit = 3,
	GameSystemEvent_GameShutdown = 4,
	GameSystemEvent_GamePostInit = 5,
	GameSystemEvent_GamePreShutdown = 6,
	GameSystemEvent_BuildGameSessionManifest = 7,   /**< msg.AddResource() */
	GameSystemEvent_GameActivate = 8,
	GameSystemEvent_ClientFullySignedOn = 9,
	GameSystemEvent_ClientDisconnect = 10,
	GameSystemEvent_GameDeactivate = 12,
	GameSystemEvent_SpawnGroupPrecache = 13,        /**< msg.AddResource(), SpawnGroupHandle, GetSpawnGroupName() */
	GameSystemEvent_SpawnGroupUncache = 14,         /**< msg.SpawnGroupHandle, GetSpawnGroupName() */
	GameSystemEvent_PreSpawnGroupLoad = 15,
	GameSystemEvent_PostSpawnGroupLoad = 16,
	GameSystemEvent_PreSpawnGroupUnload = 17,
	GameSystemEvent_PostSpawnGroupUnload = 18,
	GameSystemEvent_ActiveSpawnGroupChanged = 19,
	GameSystemEvent_ServerPreEntityThink = 31,
	GameSystemEvent_ServerPostEntityThink = 32,
	GameSystemEvent_ServerPostPhysicsSimulate = 33,
	GameSystemEvent_ServerPreClientUpdate = 34,
	GameSystemEvent_ServerAdvanceTick = 35,
	GameSystemEvent_ServerGamePostSimulate = 37,
	GameSystemEvent_ServerPostAdvanceTick = 39,
	GameSystemEvent_GameFrameBoundary = 48,
	GameSystemEvent_SaveGame = 50,
	GameSystemEvent_RestoreGame = 51,
	GameSystemEvent_NewLevelPlayerConnect = 52,
	GameSystemEvent_WorldGroupCreated = 53,
	GameSystemEvent_WorldGroupDestroyed = 54,
	GameSystemEvent_ServerPrePackEntities = 58,
};

/**
 * The message of the event being handled. Only valid inside the handler it was
 * passed to; what it offers depends on the event.
 */
methodmap GameSystemMessage
{
	/**
	 * Adds a resource ("models/foo.vmdl") to the manifest being built:
	 * BuildGameSessionManifest (the map's, loaded on the server and every
	 * client) or SpawnGroupPrecache (that spawn group's).
	 *
	 * @error         Another event, or the message is no longer valid.
	 */
	public native void AddResource(const char[] path);

	/** The spawn group of SpawnGroupPrecache / SpawnGroupUncache. */
	property int SpawnGroupHandle
	{
		public native get();
	}

	/** The spawn group's name, for SpawnGroupPrecache / SpawnGroupUncache. */
	public native int GetSpawnGroupName(char[] buffer, int maxlen);
}

/**
 * A hooked event.
 *
 * @param system      The game system.
 * @param event       The event.
 * @param msg         Its message, valid inside this call only.
 */
typedef GameSystemEventHandler = function void (IGameSystem system, GameSystemEvent event, GameSystemMessage msg);

/**
 * A game system of this script's. delete = Destroy: it leaves the engine and
 * its handlers go with it; unloading the script does the same.
 */
methodmap IGameSystem < Handle
{
	/**
	 * Makes a game system, not registered yet.
	 *
	 * @param name    What it is called, for logs.
	 */
	public native IGameSystem(const char[] name);

	/**
	 * Adds a handler for an event. Hooking an event on a registered system
	 * takes effect right away.
	 *
	 * @return        The handler's id, for Unhook().
	 */
	public native int Hook(GameSystemEvent event, GameSystemEventHandler handler);

	/** Removes a handler by the id Hook() returned. */
	public native bool Unhook(int hookId);

	/**
	 * Hands the system to the engine: it gets its hooked events from now on,
	 * and on every map after.
	 *
	 * @return        False when game systems are unavailable on this game version.
	 */
	public native bool Register();

	/** Takes it out of the engine again; Register() brings it back. */
	public native bool Unregister();

	property bool IsRegistered
	{
		public native get();
	}
}

methodmap Source2ModGameSystems
{
	/** Whether game systems work on the running game version. */
	public static native bool IsAvailable();

	/**
	 * One of the engine's own game systems by its factory name
	 * ("SpawnGroupManagerGameSystem"); a null SchemaObject when there is none or
	 * its factory has no static instance.
	 */
	public static native SchemaObject FindByName(const char[] name);
}
