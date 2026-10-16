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
 * @file ISource2ModGameHooks.h
 * @brief Hooks on the game functions plugins reach for most often, held by
 *        the core.
 *
 * One detour per function, placed by the core from its gamedata the first
 * time a plugin asks for it and taken out again once the last listener is
 * gone -- a function nobody listens to is not hooked at all. A plugin
 * registers a handler and never touches KHook, a signature or a prototype.
 * When Valve changes one -- an argument added, the order swapped between
 * platforms, as GroundAccelerate's is -- the core's gamedata and its one call
 * site change, and the plugins keep running. Anything not in this set is
 * still a KHOOK_MEMBER / KHOOK_VIRTUAL of the plugin's own.
 *
 * Every hook hands its handler a context: the hooked object, the arguments,
 * and `result` where the function returns something. What the handler
 * returns is the usual Action:
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
 * by value (a speed, a flag) cannot -- a handler that needs that keeps a raw
 * KHOOK_MEMBER of its own. Handlers run in registration order, pre before the
 * original and post after it; in post, `result` holds what the original
 * returned.
 *
 * HookX() returns a Source2ModGameHookId for UnhookX(); a plugin may have several
 * handlers on one function. The handlers are the plugin's until it unhooks
 * them or unloads, whichever comes first; nothing to undo in Unload().
 */

#ifndef _INCLUDE_ISOURCE2MOD_GAMEHOOKS_H
#define _INCLUDE_ISOURCE2MOD_GAMEHOOKS_H

#pragma once

#include "ISource2ModPlugin.h"
#include "ISource2ModTypes.h"

#include <cstdint>
#include <functional>

// variant_t is a typedef of a template in s2sdk, so it cannot be forward
// declared like the rest.
#include "entity2/variant.h"

class CBaseEntity;
class CBasePlayerController;
class CBasePlayerWeapon;
class CCSPlayer_ItemServices;
class CCSPlayer_MovementServices;
class CCSPlayer_WeaponServices;
class CCSPlayerController;
class CCSPlayerLegacyJump;
class CCSPlayerModernJump;
class CCSPlayerPawn;
class CCSPlayerPawnBase;
class CEconItemView;
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
#define SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(RET)                              \
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

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(std::int64_t)
};

/// CCSPlayer_ItemServices::CanAcquire(item, method, unk) -> AcquireResult
struct CanAcquireContext
{
    CCSPlayer_ItemServices* services;
    CEconItemView* item;
    AcquireMethod method;
    void* unk;
    AcquireResult result;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(AcquireResult)
};

/// CCSPlayerPawnBase::CanMove() -> bool; false while frozen, defusing, ...
struct CanMoveContext
{
    CCSPlayerPawnBase* pawn;
    bool result;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayer_WeaponServices::CanUse(weapon) -> bool
struct CanUseContext
{
    CCSPlayer_WeaponServices* services;
    CBasePlayerWeapon* weapon;
    bool result;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayerPawn::PostThink()
struct PostThinkContext
{
    CCSPlayerPawnBase* pawn;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
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

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void*)
};

/// CBasePlayerController::OnSimulateUserCommands()
struct SimulateUserCommandsContext
{
    CBasePlayerController* controller;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CPlayer_MovementServices::RunCommand(cmd), on CCSPlayer_MovementServices'
/// vtable; the command is a CUserCmd.
struct RunCommandContext
{
    CCSPlayer_MovementServices* services;
    void* cmd;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
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

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(bool)
};

/// CBaseEntity::Touch(other), on CBaseEntity's own vtable -- an entity class
/// that overrides Touch is not seen here.
struct TouchContext
{
    CBaseEntity* entity;
    CBaseEntity* other;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_WeaponServices::DropWeapon(weapon, target, velocity)
struct DropWeaponContext
{
    CCSPlayer_WeaponServices* services;
    CBasePlayerWeapon* weapon;
    Vector* target;
    Vector* velocity;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
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

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(bool)
};

/// CCSPlayer_MovementServices::AirAccelerate(move, wishDirection, wishSpeed, acceleration)
struct AirAccelerateContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    Vector* wishDirection;
    float wishSpeed;
    float acceleration;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
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

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::CategorizePosition(move, stayOnGround)
struct CategorizePositionContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    bool stayOnGround;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::CheckVelocity(move, unk)
struct CheckVelocityContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    void* unk;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::FullWalkMove(move, onGround)
struct FullWalkMoveContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    bool onGround;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::SetupMove(cmd, move); the command is a CUserCmd.
struct SetupMoveContext
{
    CCSPlayer_MovementServices* services;
    CUserCmd* cmd;
    CMoveData* moveData;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayer_MovementServices::TryPlayerMove(move, firstDest, firstTrace, isSurfing)
struct TryPlayerMoveContext
{
    CCSPlayer_MovementServices* services;
    CMoveData* moveData;
    Vector* firstDest;
    CGameTrace* firstTrace;
    bool* isSurfing;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayerLegacyJump::OnJump / CheckJumpButton(move)
struct LegacyJumpContext
{
    CCSPlayerLegacyJump* jump;
    CMoveData* moveData;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/// CCSPlayerModernJump::OnJump / CheckJumpButton(move)
struct ModernJumpContext
{
    CCSPlayerModernJump* jump;
    CMoveData* moveData;

    SOURCE2MOD_GAMEHOOK_CALL_ORIGINAL(void)
};

/**
 * @brief What a game hook handler answers -- KHook's { action, value }.
 *
 * @code
 * return { Action::Supersede, 0 };              // skip the original, it returns 0
 * return { Action::Override, AcquireResult::NotAllowedByMode };
 * return { Action::Supersede, ctx.CallOriginal() };
 * return Action::Ignore;                         // an action alone
 * @endcode
 *
 * An action alone takes its value from ctx.result, so a handler that sets
 * the field and returns Override / Supersede keeps working. For a function
 * without a return value there is only the action.
 *
 * Several handlers on one function: Supersede stops the chain, otherwise the
 * strongest action wins; each value a handler returns lands in ctx.result
 * right away, so the last one to return a value decides it and the next
 * handler sees it there.
 */
template <typename RETURN>
struct GameHookReturn
{
    Action action;
    RETURN value{};
    bool hasValue = false;

    GameHookReturn(Action a) : action(a) {}
    GameHookReturn(Action a, RETURN v) : action(a), value(v), hasValue(true) {}
};

template <>
struct GameHookReturn<void>
{
    Action action;

    GameHookReturn(Action a) : action(a) {}
};

/// A handler: the context, and whether this is the post pass. A function,
/// an object and a method, or a lambda (Source2ModCallback).
template <typename CONTEXT>
using GameHookHandler = Source2ModCallback<GameHookReturn<typename CONTEXT::Return>(CONTEXT& ctx, bool post)>;

/// What HookX() returns and UnhookX() takes: one registered handler. A plugin
/// may register several handlers on the same function and drop any one of
/// them; whatever it still holds at unload the core drops for it.
using Source2ModGameHookId = Source2ModHookId;

/// The hooks, for IsAvailable().
enum class GameHook : int
{
    TakeDamage,
    CanAcquire,
    CanMove,
    CanUse,
    PostThink,
    ProcessUsercmds,
    SimulateUserCommands,
    RunCommand,
    AcceptInput,
    Touch,
    DropWeapon,
    AirAccelerate,
    AirMove,
    CanUnduck,
    CategorizePosition,
    CheckFalling,
    CheckParameters,
    CheckVelocity,
    CheckWater,
    Duck,
    Friction,
    FullWalkMove,
    GroundAccelerate,
    LadderMove,
    MoveInit,
    PlayerMove,
    ProcessMovement,
    SetupMove,
    TryPlayerMove,
    WalkMove,
    WaterMove,
    OnJumpLegacy,
    OnJumpModern,
    CheckJumpButtonLegacy,
    CheckJumpButtonModern,

    Count
};

/* =========================
Interface
========================= */

/**
 * Whose a handler is, the core reads off the handler (see Source2ModCallback):
 * no call takes a plugin id. What a plugin still holds at unload the core
 * drops for it.
 */
class ISource2ModGameHooks
{
public:
    virtual ~ISource2ModGameHooks() = default;


    virtual Source2ModGameHookId HookTakeDamage(GameHookHandler<TakeDamageContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCanAcquire(GameHookHandler<CanAcquireContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCanMove(GameHookHandler<CanMoveContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCanUse(GameHookHandler<CanUseContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookPostThink(GameHookHandler<PostThinkContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookProcessUsercmds(GameHookHandler<ProcessUsercmdsContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookSimulateUserCommands(GameHookHandler<SimulateUserCommandsContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookRunCommand(GameHookHandler<RunCommandContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookAcceptInput(GameHookHandler<AcceptInputContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookTouch(GameHookHandler<TouchContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookDropWeapon(GameHookHandler<DropWeaponContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookAirAccelerate(GameHookHandler<AirAccelerateContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookAirMove(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCanUnduck(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCategorizePosition(GameHookHandler<CategorizePositionContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCheckFalling(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCheckParameters(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCheckVelocity(GameHookHandler<CheckVelocityContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCheckWater(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookDuck(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookFriction(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookFullWalkMove(GameHookHandler<FullWalkMoveContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookGroundAccelerate(GameHookHandler<GroundAccelerateContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookLadderMove(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookMoveInit(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookPlayerMove(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookProcessMovement(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookSetupMove(GameHookHandler<SetupMoveContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookTryPlayerMove(GameHookHandler<TryPlayerMoveContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookWalkMove(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookWaterMove(GameHookHandler<MovementContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookOnJumpLegacy(GameHookHandler<LegacyJumpContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookOnJumpModern(GameHookHandler<ModernJumpContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCheckJumpButtonLegacy(GameHookHandler<LegacyJumpContext> handler, bool post) = 0;
    virtual Source2ModGameHookId HookCheckJumpButtonModern(GameHookHandler<ModernJumpContext> handler, bool post) = 0;

    /// Drops a handler: the one HookX() returned the id for, or the one that is
    /// this function or object and method (a lambda goes by its id). True
    /// when it was found.
    virtual bool UnhookTakeDamage(Source2ModGameHookId id) = 0;
    virtual bool UnhookTakeDamage(const GameHookHandler<TakeDamageContext>& handler, bool post) = 0;
    virtual bool UnhookCanAcquire(Source2ModGameHookId id) = 0;
    virtual bool UnhookCanAcquire(const GameHookHandler<CanAcquireContext>& handler, bool post) = 0;
    virtual bool UnhookCanMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookCanMove(const GameHookHandler<CanMoveContext>& handler, bool post) = 0;
    virtual bool UnhookCanUse(Source2ModGameHookId id) = 0;
    virtual bool UnhookCanUse(const GameHookHandler<CanUseContext>& handler, bool post) = 0;
    virtual bool UnhookPostThink(Source2ModGameHookId id) = 0;
    virtual bool UnhookPostThink(const GameHookHandler<PostThinkContext>& handler, bool post) = 0;
    virtual bool UnhookProcessUsercmds(Source2ModGameHookId id) = 0;
    virtual bool UnhookProcessUsercmds(const GameHookHandler<ProcessUsercmdsContext>& handler, bool post) = 0;
    virtual bool UnhookSimulateUserCommands(Source2ModGameHookId id) = 0;
    virtual bool UnhookSimulateUserCommands(const GameHookHandler<SimulateUserCommandsContext>& handler, bool post) = 0;
    virtual bool UnhookRunCommand(Source2ModGameHookId id) = 0;
    virtual bool UnhookRunCommand(const GameHookHandler<RunCommandContext>& handler, bool post) = 0;
    virtual bool UnhookAcceptInput(Source2ModGameHookId id) = 0;
    virtual bool UnhookAcceptInput(const GameHookHandler<AcceptInputContext>& handler, bool post) = 0;
    virtual bool UnhookTouch(Source2ModGameHookId id) = 0;
    virtual bool UnhookTouch(const GameHookHandler<TouchContext>& handler, bool post) = 0;
    virtual bool UnhookDropWeapon(Source2ModGameHookId id) = 0;
    virtual bool UnhookDropWeapon(const GameHookHandler<DropWeaponContext>& handler, bool post) = 0;
    virtual bool UnhookAirAccelerate(Source2ModGameHookId id) = 0;
    virtual bool UnhookAirAccelerate(const GameHookHandler<AirAccelerateContext>& handler, bool post) = 0;
    virtual bool UnhookAirMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookAirMove(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookCanUnduck(Source2ModGameHookId id) = 0;
    virtual bool UnhookCanUnduck(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookCategorizePosition(Source2ModGameHookId id) = 0;
    virtual bool UnhookCategorizePosition(const GameHookHandler<CategorizePositionContext>& handler, bool post) = 0;
    virtual bool UnhookCheckFalling(Source2ModGameHookId id) = 0;
    virtual bool UnhookCheckFalling(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookCheckParameters(Source2ModGameHookId id) = 0;
    virtual bool UnhookCheckParameters(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookCheckVelocity(Source2ModGameHookId id) = 0;
    virtual bool UnhookCheckVelocity(const GameHookHandler<CheckVelocityContext>& handler, bool post) = 0;
    virtual bool UnhookCheckWater(Source2ModGameHookId id) = 0;
    virtual bool UnhookCheckWater(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookDuck(Source2ModGameHookId id) = 0;
    virtual bool UnhookDuck(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookFriction(Source2ModGameHookId id) = 0;
    virtual bool UnhookFriction(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookFullWalkMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookFullWalkMove(const GameHookHandler<FullWalkMoveContext>& handler, bool post) = 0;
    virtual bool UnhookGroundAccelerate(Source2ModGameHookId id) = 0;
    virtual bool UnhookGroundAccelerate(const GameHookHandler<GroundAccelerateContext>& handler, bool post) = 0;
    virtual bool UnhookLadderMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookLadderMove(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookMoveInit(Source2ModGameHookId id) = 0;
    virtual bool UnhookMoveInit(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookPlayerMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookPlayerMove(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookProcessMovement(Source2ModGameHookId id) = 0;
    virtual bool UnhookProcessMovement(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookSetupMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookSetupMove(const GameHookHandler<SetupMoveContext>& handler, bool post) = 0;
    virtual bool UnhookTryPlayerMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookTryPlayerMove(const GameHookHandler<TryPlayerMoveContext>& handler, bool post) = 0;
    virtual bool UnhookWalkMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookWalkMove(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookWaterMove(Source2ModGameHookId id) = 0;
    virtual bool UnhookWaterMove(const GameHookHandler<MovementContext>& handler, bool post) = 0;
    virtual bool UnhookOnJumpLegacy(Source2ModGameHookId id) = 0;
    virtual bool UnhookOnJumpLegacy(const GameHookHandler<LegacyJumpContext>& handler, bool post) = 0;
    virtual bool UnhookOnJumpModern(Source2ModGameHookId id) = 0;
    virtual bool UnhookOnJumpModern(const GameHookHandler<ModernJumpContext>& handler, bool post) = 0;
    virtual bool UnhookCheckJumpButtonLegacy(Source2ModGameHookId id) = 0;
    virtual bool UnhookCheckJumpButtonLegacy(const GameHookHandler<LegacyJumpContext>& handler, bool post) = 0;
    virtual bool UnhookCheckJumpButtonModern(Source2ModGameHookId id) = 0;
    virtual bool UnhookCheckJumpButtonModern(const GameHookHandler<ModernJumpContext>& handler, bool post) = 0;

    /// False when the core's gamedata has no entry for the function on this
    /// platform -- handlers can still be registered, they just never run.
    virtual bool IsAvailable(GameHook hook) = 0;

};

#endif //_INCLUDE_ISOURCE2MOD_GAMEHOOKS_H
