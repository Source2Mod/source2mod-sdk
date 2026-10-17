/**
 * Source2Mod SourcePawn API: entities (ISource2ModEntities), with the entity
 * listener (created, spawned, deleted, parent changed) and the entity output
 * listener. Both take an optional designer name and an optional single
 * entity; what a script still has added when it unloads goes with it.
 */

#if defined _source2mod_entities_included
 #endinput
#endif
#define _source2mod_entities_included

#include <source2mod/types>
#include <source2mod/schema>
#include <source2mod/adt>

/** The entity listener's callbacks, one function each. */
typedef Source2ModEntityCallback = function void (CEntityInstance entity);
typedef Source2ModEntityParentCallback = function void (CEntityInstance entity, CEntityInstance newParent);

/**
 * An entity output firing, pre (post = false: before its targets get it) and
 * post. Pre may answer Action_Supersede: the output does not fire.
 */
typedef Source2ModEntityOutputCallback = function Source2ModAction (CEntityInstance caller, const char[] output,
                                                                    CEntityInstance activator, float delay, bool post);

methodmap Source2ModEntities
{
	/** The game rules, kept at hand by the core; null between maps. */
	public static native CCSGameRules GetGameRules();

	/** What the player's crosshair is on; NULL_ENTITY for nothing. */
	public static native CBaseEntity FindPickerEntity(CBasePlayerController player);

	/** The next entity of a class after `start` (NULL_ENTITY starts the search). */
	public static native CBaseEntity FindEntityByClassname(CEntityInstance start, const char[] className);

	/**
	 * The next entity with a targetname after `start` (NULL_ENTITY starts the
	 * search). searching / activator / caller resolve the special names
	 * "!self", "!activator", "!caller", "!player".
	 */
	public static native CBaseEntity FindEntityByName(CEntityInstance start, const char[] name, CEntityInstance searching = NULL_ENTITY,
	                                                  CEntityInstance activator = NULL_ENTITY, CEntityInstance caller = NULL_ENTITY);

	/** Every entity of a designer name ("prop_dynamic"), as an ArrayList of entities; close it. */
	public static native ArrayList FindAllEntitiesByDesignerName(const char[] designerName);

	/** Created, not spawned: set it up, then DispatchSpawn(). */
	public static native CBaseEntity CreateEntityByName(const char[] className);

	/** An input, as a map's I/O sends it. */
	public static native void AcceptInput(CEntityInstance target, const char[] input, CEntityInstance activator = NULL_ENTITY,
	                                      CEntityInstance caller = NULL_ENTITY, const char[] value = "");

	/** The same, queued: runs after `delay` seconds. */
	public static native void AddEntityIOEvent(CEntityInstance target, const char[] input, CEntityInstance activator = NULL_ENTITY,
	                                           CEntityInstance caller = NULL_ENTITY, const char[] value = "", float delay = 0.0);

	/**
	 * Entity events: created (keyvalues not applied, not spawned yet),
	 * spawned, deleted (the last call for it), parent changed. Pass
	 * INVALID_FUNCTION for what is not needed.
	 *
	 * @param designerName  Only entities of this designer name ("" any).
	 * @param entity        Only this entity (NULL_ENTITY any); dropped after its deleted call.
	 * @return The id RemoveEntityListener takes.
	 */
	public static native int AddEntityListener(Source2ModEntityCallback created = INVALID_FUNCTION,
	                                           Source2ModEntityCallback spawned = INVALID_FUNCTION,
	                                           Source2ModEntityCallback deleted = INVALID_FUNCTION,
	                                           Source2ModEntityParentCallback parentChanged = INVALID_FUNCTION,
	                                           const char[] designerName = "", CEntityInstance entity = NULL_ENTITY);

	public static native bool RemoveEntityListener(int id);

	/**
	 * Entity outputs, pre and post.
	 *
	 * @param designerName  Only outputs of entities of this designer name ("" any).
	 * @param output        Only this output ("OnTrigger"; "" any).
	 * @param entity        Only outputs of this entity (NULL_ENTITY any).
	 * @return The id RemoveEntityOutputListener takes.
	 */
	public static native int AddEntityOutputListener(Source2ModEntityOutputCallback callback, const char[] designerName = "",
	                                                 const char[] output = "", CEntityInstance entity = NULL_ENTITY);

	public static native bool RemoveEntityOutputListener(int id);
}
