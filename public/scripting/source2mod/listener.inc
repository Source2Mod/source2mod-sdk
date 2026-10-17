/**
 * Source2Mod SourcePawn API: ISource2ModListener as forwards.
 *
 * Each is called in every script that has it, as the C++ listener is called
 * in every plugin that registered one. Slots are 0-based; 64-bit SteamIDs
 * (xuid) are decimal text.
 */

#if defined _source2mod_listener_included
 #endinput
#endif
#define _source2mod_listener_included

#include <source2mod/types>

forward void Source2Mod_OnLevelInit(const char[] mapName, const char[] mapEntities, const char[] oldLevel, const char[] landmarkName, bool loadGame, bool background);

forward void Source2Mod_OnLevelShutdown();

/** Steam validated the player in this slot: from now on they are checked under steamId. Not called for bots. */
forward void Source2Mod_OnClientAuthorized(CPlayerSlot slot, const char[] steamId);

/** Steam refused the ticket of the player in this slot; steamId is the one they claimed. */
forward void Source2Mod_OnClientAuthorizeFailed(CPlayerSlot slot, const char[] steamId);

/** What this SteamID64 holds may have changed; "0" when a change can touch anybody. */
forward void Source2Mod_OnPermissionsChanged(const char[] steamId);

/** Another Source2Mod plugin (native or script) loaded / is unloading; `id` is its plugin id. */
forward void Source2Mod_OnPluginLoad(int id, const char[] name);

forward void Source2Mod_OnPluginUnload(int id, const char[] name);

/** A Metamod plugin loaded / unloaded (Metamod's numbering). */
forward void Source2Mod_OnMetamodPluginLoad(int id);

forward void Source2Mod_OnMetamodPluginUnload(int id);

/** Every Source2Mod plugin of the startup batch is loaded (SourceMod's OnAllPluginsLoaded runs right after). */
forward void Source2Mod_OnAllSource2ModPluginsLoaded();

forward void Source2Mod_OnAllMetamodPluginsLoaded();
