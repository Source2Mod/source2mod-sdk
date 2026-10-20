/**
* vim: set ts=4 sw=4 tw=99 noet:
 * =============================================================================
 * Source2Mod
 * Copyright (C) 2025-2026 Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl.
 * All rights reserved.
 * =============================================================================
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License, version 3.0, as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 * gives you permission to link the code of this program
 * (as well as its derivative works) to "Counter-Strike 2," "Source 2,"
 * "Steam," and any Game MODs or server software running on software by
 * Valve Corporation. You must obey the GNU General Public License in all
 * respects for all other code used.
 *
 * Additionally, this exception applies to all derivative works unless
 * otherwise stated in LICENSE.txt.
 *
 * Authors:
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: Source2Mod
 */

/**
 * @file ISource2ModHookContexts.h
 * @brief What the handlers of the typed hook points get (ISource2ModHooks.h).
 *
 * Every point hands its handler a context: the hooked object, the
 * arguments, and `result` where the function returns something. What the
 * handler returns is the usual Action:
 *
 * * Ignore    -- nothing changed.
 * * Override  -- pre: the original still runs but the function returns
 *                `ctx.result`; post: the function returns `ctx.result`
 *                instead of what the original produced.
 * * Supersede -- pre only: the original does not run, the function returns
 *                `ctx.result`. Stops the remaining pre handlers.
 *
 * Arguments passed by pointer (the damage info, the move data, the weapon)
 * can be changed in place and the original sees the change; arguments passed
 * by value (a speed, a flag) cannot -- HookPoint::Function (FunctionContext)
 * can. Handlers run in registration order, pre before the original and post
 * after it; in post, `result` holds what the original returned.
 *
 * The game-function points are one detour per function, placed by the core
 * from its gamedata the first time a plugin asks for it and kept until
 * shutdown (a detour taken out and put back would miss calls while KHook
 * inserts it again). When Valve changes one -- an argument added,
 * the order swapped between platforms, as GroundAccelerate's is -- the core's
 * gamedata and its one call site change, and the plugins keep running.
 */

#ifndef _INCLUDE_ISOURCE2MOD_HOOKCONTEXTS_H
#define _INCLUDE_ISOURCE2MOD_HOOKCONTEXTS_H

#pragma once

#include "ISource2ModPlugin.h"
#include "ISource2ModTypes.h"

#include <cstdint>
#include <functional>

// variant_t is a typedef of a template in s2sdk, so it cannot be forward
// declared like the rest.
#include "entity2/variant.h"

class CBaseEntity;
class CCommand;
class CBasePlayerController;
class CBasePlayerWeapon;
class CCSPlayer_ItemServices;
class CCSPlayer_MovementServices;
class CCSPlayer_WeaponServices;
class CCSPlayerController;
class CCSPlayerInventory;
class CCSPlayerLegacyJump;
class CCSPlayerModernJump;
class CCSPlayerPawn;
class CCSPlayerPawnBase;
class CEconItemView;
class CEntityKeyValues;
class CEntityIdentity;
class CEntityInstance;
class CGameTrace;
class CMoveData;
class CPlayer_MovementServices;
class CTakeDamageInfo;
class CTakeDamageResult;
class CUserCmd;
class CUtlSymbolLarge;
class Vector;

/// What CCSPlayer_ItemServices::CanAcquire answers.
enum AcquireResult : std::uint32_t
{
    Allowed = 0,
    InvalidItem,
    AlreadyOwned,
    AlreadyPurchased,
    ReachedGrenadeTypeLimit,
    ReachedGrenadeTotalLimit,
    NotAllowedByTeam,
    NotAllowedByMap,
    NotAllowedByMode,
    NotAllowedForPurchase,
    NotAllowedByProhibition,
};

/// How an item is being acquired in CCSPlayer_ItemServices::CanAcquire.
enum AcquireMethod : std::uint32_t
{
    PickUp = 0,
    Buy,
    BuyWithCtrl,
};

/* =========================
Contexts
========================= */

/**
 * Every context ends with CallOriginal(): the game's function, run now with
 * the arguments the game passed, past every hook on it -- SourceHook's
 * SH_CALL. What it returns is what the game would have answered.
 *
 * In a pre handler the function really runs (with all its side effects), so
 * a handler that calls it should answer Supersede with ctx.result set to the
 * value, or the game runs it a second time after the handlers:
 *
 *     const AcquireResult game = ctx.CallOriginal();
 *     ctx.result = game == AcquireResult::Allowed && Blocked(ctx) ? AcquireResult::NotAllowedByMode : game;
 *     return Action::Supersede;
 *
 * A changed pointed-to object (the damage info, the move data) counts; a
 * scalar field changed in the context does not, the call takes what the
 * game passed. In a post handler ctx.result already holds the game's answer.
 *
 * The two members after it belong to the core; the movement contexts shared
 * by void and bool functions answer false for the void ones.
 */
#define SOURCE2MOD_HOOK_CALL_ORIGINAL(RET)                              \
    using Return = RET;                                                   \
    RET CallOriginal() const { return callOriginal_(callOriginalData_); } \
    RET (*callOriginal_)(const void*) = nullptr;                          \
    const void* callOriginalData_ = nullptr;

/// CBaseEntity::TakeDamageOld(info, result) -> int64
struct TakeDamageContext
{
    CBaseEntity* entity;
    CTakeDamageInfo* info;
    CTakeDamageResult* damageResult;
    std::int64_t result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(std::int64_t)
};

/// CCSPlayer_ItemServices::CanAcquire(item, method, unk) -> AcquireResult
struct CanAcquireContext
{
    CCSPlayer_ItemServices* services;
    CEconItemView* item;
    AcquireMethod method;
    void* unk;
    AcquireResult result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(AcquireResult)
};

/// CCSPlayerPawnBase::CanMove() -> bool; false while frozen, defusing, ...
struct CanMoveContext
{
    CCSPlayerPawnBase* pawn;
    bool result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayer_WeaponServices::CanUse(weapon) -> bool
struct CanUseContext
{
    CCSPlayer_WeaponServices* services;
    CBasePlayerWeapon* weapon;
    bool result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayerPawn::PostThink()
struct PostThinkContext
{
    CCSPlayerPawnBase* pawn;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayerController::ProcessUserCmd(cmds, count, paused, margin) -> void*.
/// The commands are CUserCmd (a protobuf-backed type), left untyped here.
struct ProcessUsercmdsContext
{
    CCSPlayerController* controller;
    void* cmds;
    int count;
    bool paused;
    float margin;
    void* result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void*)
};

/// CBasePlayerController::OnSimulateUserCommands()
struct SimulateUserCommandsContext
{
    CBasePlayerController* controller;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CPlayer_MovementServices::RunCommand(cmd), on CCSPlayer_MovementServices'
/// vtable; the command is a CUserCmd.
struct RunCommandContext
{
    CCSPlayer_MovementServices* services;
    void* cmd;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CEntityIdentity::AcceptInput(name, activator, caller, value, ...) -> bool
struct AcceptInputContext
{
    CEntityIdentity* identity;
    CUtlSymbolLarge* inputName;
    CEntityInstance* activator;
    CEntityInstance* caller;
    variant_t* value;
    bool result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(bool)
};

/// CBaseEntity::Touch(other), on CBaseEntity's own vtable -- an entity class
/// that overrides Touch is not seen here.
struct TouchContext
{
    CBaseEntity* entity;
    CBaseEntity* other;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_WeaponServices::DropWeapon(weapon, target, velocity)
struct DropWeaponContext
{
    CCSPlayer_WeaponServices* services;
    CBasePlayerWeapon* weapon;
    Vector* target;
    Vector* velocity;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// The CCSPlayer_MovementServices functions that take the move data alone:
/// AirMove, CheckFalling, CheckParameters, Duck, Friction, PlayerMove,
/// ProcessMovement, WalkMove, WaterMove (void), and CanUnduck, CheckWater,
/// LadderMove, MoveInit (bool, in `result`).
struct MovementContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    bool result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayer_MovementServices::AirAccelerate(move, wishDirection, wishSpeed, acceleration)
struct AirAccelerateContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    Vector* wishDirection;
    float wishSpeed;
    float acceleration;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::GroundAccelerate(move, wishDirection, frameTime, wishSpeed, acceleration).
/// The engine's argument order differs between Linux and Windows; here it is
/// always this one.
struct GroundAccelerateContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    Vector* wishDirection;
    float frameTime;
    float wishSpeed;
    float acceleration;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::CategorizePosition(move, stayOnGround)
struct CategorizePositionContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    bool stayOnGround;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::CheckVelocity(move, unk)
struct CheckVelocityContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    void* unk;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::FullWalkMove(move, onGround)
struct FullWalkMoveContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    bool onGround;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::SetupMove(cmd, move); the command is a CUserCmd.
struct SetupMoveContext
{
    CCSPlayer_MovementServices* services;
    CUserCmd* cmd;
    CMoveData* moveData;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::TryPlayerMove(move, firstDest, firstTrace, isSurfing)
struct TryPlayerMoveContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    Vector* firstDest;
    CGameTrace* firstTrace;
    bool* isSurfing;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayerLegacyJump::OnJump / CheckJumpButton(move)
struct LegacyJumpContext
{
    CCSPlayerLegacyJump* jump;
    CMoveData* moveData;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CCSPlayerModernJump::OnJump / CheckJumpButton(move)
struct ModernJumpContext
{
    CCSPlayerModernJump* jump;
    CMoveData* moveData;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// Host_Say(controller, args, teamOnly, ...): a say / say_team line the game
/// is about to print and send. Lines a silent chat trigger hid for a
/// Source2Mod command never get here; "!command" lines do (they stay in chat).
/// Pre: Supersede drops the line (no chat, no player_say event).
/// After the say command, Source2Mod fires the game's player_chat event for a
/// line that went out (the game never does), unless somebody else already
/// fired one for it (CounterStrikeSharp does, from its own Host_Say hook).
struct PlayerChatContext
{
    CCSPlayerController* controller;    // nullptr: the server console
    CCommand* args;                     // the say command; Arg(1) is the line
    const char* message;                // the line (Arg(1), or ArgS() for an unquoted one), never nullptr
    bool teamOnly;                      // say_team
    int unk1;
    const char* unk2;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/* =========================
Entity virtuals (SourceMod's SDKHooks)
========================= */

// Spawn, Use, StartTouch, EndTouch, Blocked, Think, Killed: virtuals that
// most entity classes override, so they are hooked on the vtable of the
// target's class -- HookTarget::Instance(entity) (one entity, SDKHook) or
// HookTarget::Class("CCSPlayerPawn") (every entity of that class or one
// derived from it). HookTarget::All() is refused: there is no one function
// to hook. The handler runs for the entity's own override.

/// CBaseEntity::Use's argument (activator, caller, USE_TYPE, value).
struct EntityUseInfo
{
    CBaseEntity* activator;
    CBaseEntity* caller;
    std::int32_t useType;           // 0 off, 1 on, 2 set, 3 toggle
    float value;
};

/// CBaseEntity::Spawn(keyValues) -- the entity's keyvalues are applied here.
struct SpawnContext
{
    CBaseEntity* entity;
    const CEntityKeyValues* keyValues;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CBaseEntity::Use(info) -- a player's +use, a map's Use input. Changing
/// *info in pre changes what the entity gets.
struct UseContext
{
    CBaseEntity* entity;
    EntityUseInfo* info;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CBaseEntity::StartTouch / EndTouch / Blocked(other)
struct EntityOtherContext
{
    CBaseEntity* entity;
    CBaseEntity* other;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CBaseEntity::Think() -- the entity's think function, every time it runs.
struct ThinkContext
{
    CBaseEntity* entity;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(void)
};

/// CBaseEntity::Event_Killed(damage) -> bool -- health reached 0 and
/// m_lifeState is already "dying". Supersede skips the class's death handling
/// (a player's ragdoll and death, a breakable's gibs).
struct KilledContext
{
    CBaseEntity* entity;
    const CTakeDamageResult* damage;
    bool result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayer_WeaponServices::PickupWeapon(weapon) -> 0 refused, 1 taken,
/// 2 taken as ammo -- touching or +using a weapon (SDKHooks' WeaponEquip /
/// WeaponCanUse). Every player: HookTarget::All() works.
struct WeaponPickupContext
{
    CCSPlayer_WeaponServices* services;
    CBasePlayerWeapon* weapon;
    std::int32_t result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(std::int32_t)
};

/// CCSPlayer_WeaponServices::SwitchToWeapon(weapon, viewModelIndex) -> bool
/// (SDKHooks' WeaponSwitch). Every player: HookTarget::All() works.
struct WeaponSwitchContext
{
    CCSPlayer_WeaponServices* services;
    CBasePlayerWeapon* weapon;
    std::int32_t viewModelIndex;
    bool result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayerInventory::GetItemInLoadout(team, slot) -> CEconItemView* -- the
/// item a player has in a loadout slot (team CS_TEAM_T / CS_TEAM_CT, slot a
/// loadout position), asked whenever the game needs it: giving weapons,
/// gloves, agents, music kits, the scoreboard. Answer another item with
/// Supersede (pre) or Override (post) and `result`. Every player:
/// HookTarget::All() works.
struct GetItemInLoadoutContext
{
    CCSPlayerInventory* inventory;
    /// Whose inventory it is; nullptr for one no player in the game has.
    CCSPlayerController* controller;
    std::int32_t team;
    std::int32_t slot;
    CEconItemView* result;

    SOURCE2MOD_HOOK_CALL_ORIGINAL(CEconItemView*)
};

/* =========================
Engine callbacks
========================= */

// No CallOriginal: the core produces these from its own hooks on the engine,
// and the handlers run around the engine's call. Return = void: a handler
// answers an Action alone (only ClientConnect has a value).
#define SOURCE2MOD_HOOK_NO_RETURN using Return = void;

class IGameEventManager2;
class ISource2WorldSession;
class CBufferString;

/// ISource2Server::GameFrame -- every tick.
struct GameFrameContext
{
    bool simulating;
    bool firstTick;
    bool lastTick;

    SOURCE2MOD_HOOK_NO_RETURN
};

/// INetworkServerService::StartupServer -- a new map's server is up.
struct StartupServerContext
{
    const GameSessionConfiguration_t* config;
    ISource2WorldSession* session;
    const char* mapName;

    SOURCE2MOD_HOOK_NO_RETURN
};

/**
 * ISource2GameClients::ClientConnect. Pre only means anything: answer
 * { Action::Supersede, false } (or set ctx.result = false and Supersede) to
 * turn the client away, with the reason in ctx.rejectReason. The first
 * handler that refuses ends it. Not called for bots.
 */
struct ClientConnectContext
{
    CPlayerSlot slot;
    const char* name;
    uint64 xuid;
    const char* networkId;
    bool unk1;
    CBufferString* rejectReason;
    bool result;

    using Return = bool;
};

/// ISource2GameClients::OnClientConnected -- bots too (fakePlayer).
struct ClientConnectedContext
{
    CPlayerSlot slot;
    const char* name;
    uint64 xuid;
    const char* networkId;
    const char* address;
    bool fakePlayer;

    SOURCE2MOD_HOOK_NO_RETURN
};

/// ISource2GameClients::ClientPutInServer.
struct ClientPutInServerContext
{
    CPlayerSlot slot;
    const char* name;
    int type;
    uint64 xuid;

    SOURCE2MOD_HOOK_NO_RETURN
};

/// ISource2GameClients::ClientDisconnect: pre while the client is still in
/// game, post once it is gone.
struct ClientDisconnectContext
{
    CPlayerSlot slot;
    ENetworkDisconnectionReason reason;
    const char* name;
    uint64 xuid;
    const char* networkId;

    SOURCE2MOD_HOOK_NO_RETURN
};

/// ClientVoice, ClientSettingsChanged: which client.
struct ClientSlotContext
{
    CPlayerSlot slot;

    SOURCE2MOD_HOOK_NO_RETURN
};

/// GameServerSteamAPIActivated / Deactivated: nothing to pass.
struct EmptyContext
{
    SOURCE2MOD_HOOK_NO_RETURN
};

/// IGameEventManager2::LoadEventsFromFile -- a resource file of game events.
struct LoadEventsFromFileContext
{
    IGameEventManager2* manager;
    const char* filename;
    bool searchAll;

    SOURCE2MOD_HOOK_NO_RETURN
};

#endif //_INCLUDE_ISOURCE2MOD_HOOKCONTEXTS_H
