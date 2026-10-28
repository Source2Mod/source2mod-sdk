/**
 * vim: set ts=4 sw=4 tw=99 noet:
 * =============================================================================
 * Source2Mod
 * Copyright (C) 2025-2026 Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl.
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
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl gives
 * you permission to link the code of this program (as well as its derivative
 * works) to "Counter-Strike 2," "Source 2," "Steam," and any Game MODs or
 * server software running on software by Valve Corporation. You must obey the
 * GNU General Public License in all respects for all other code used.
 *
 * Additionally, this exception applies to all derivative works unless
 * otherwise stated in LICENSE.txt.
 *
 * Authors:
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: Source2Mod
 */

#ifndef _INCLUDE_ISOURCE2MOD_HOOKS_H
#define _INCLUDE_ISOURCE2MOD_HOOKS_H

#pragma once

#include "ISource2ModFunctions.h"
#include "ISource2ModHookContexts.h"
#include "ISource2ModTypes.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

/**
 * @file ISource2ModHooks.h
 * @brief One way to hook anything, the same from C++ and SourcePawn:
 *
 *     Hook = Point (what) + Target (on which object) + Handler (with its Context)
 *
 * Points (HookPoint::*):
 *   - game functions with a typed context: TakeDamage, Touch, PostThink,
 *     AcceptInput, the movement functions, PlayerChat (Host_Say), ...
 *     (ISource2ModHookContexts.h);
 *   - entity virtuals with a typed context, hooked on the target's own class
 *     (SourceMod's SDKHooks): Spawn, Use, StartTouch, EndTouch, Blocked,
 *     Think, Killed, WeaponPickup, WeaponSwitch; and GetItemInLoadout (a
 *     player inventory's virtual, HookTarget::All() works);
 *   - engine callbacks: GameFrame, StartupServer, ClientConnect, ClientConnected,
 *     ClientPutInServer, ClientDisconnect, ClientVoice, ClientSettingsChanged,
 *     the Steam API, LoadEventsFromFile;
 *   - any function: HookPoint::Function(fn), fn an ISource2ModFunction --
 *     a gamedata entry (Find), a signature, a symbol, a vtable slot, an engine
 *     interface's virtual, a core address (ISource2ModFunctions::From*).
 *     SourceMod's DHooks.
 *
 * Targets (HookTarget):
 *   - All()                 every call: a detour, every instance;
 *   - Instance(object)      one object: an entity, a CServerSideClient, a
 *                           services object, the game rules, ...;
 *   - Class("CCSPlayerPawn") every object of that C++ class or one derived
 *                           from it (RTTI).
 *   The object is the function's `this` (for a free function its first
 *   argument). A virtual is hooked once, on
 *   the vtable, and the core compares the object before a handler runs.
 *   Engine callbacks take All() only.
 *
 * Filters (HookFilter, optional): a point whose object is an entity compares
 * its designer name ("logic_relay", "player"); AcceptInput also the input's
 * name.
 *
 * Entity events (created, spawned, deleted, parent changed) and entity
 * outputs are listeners of ISource2ModEntities, the way commands, convars and
 * game events have theirs: this is for functions.
 *
 * Every Hook() returns a Source2ModHookId; Unhook(id) drops it. What a plugin
 * still holds when it unloads, the core drops for it.
 *
 * KHook (ISource2ModKHook.h: KHOOK_VIRTUAL, KHOOK_MEMBER, KHOOK_FUNCTION) stays
 * available to C++ plugins, but prefer this: a plugin with KHook hooks of its
 * own is "raw tier" in `s2m list` and needs a rebuild whenever KHook or the
 * engine changes, a plugin on ISource2ModHooks only a core update.
 *
 * @code
 * // Half damage for one pawn.
 * g_pSource2ModHooks->Hook(HookPoint::TakeDamage, HookTarget::Instance(pawn),
 *     [](TakeDamageContext& ctx, bool post) -> HookReturn<std::int64_t>
 *     {
 *         ctx.info->m_flDamage /= 2;     // the original sees it: the info is passed by pointer
 *         return Action::Ignore;
 *     });
 *
 * // Every tick, after the game's own work.
 * g_pSource2ModHooks->Hook(HookPoint::GameFrame, HookTarget::All(), { this, &Plugin::OnGameFrame }, true);
 *
 * // One input of one kind of entity.
 * g_pSource2ModHooks->Hook(HookPoint::AcceptInput, HookTarget::All(), OnRelayInput, false, HookFilter{ "logic_relay", "Trigger" });
 *
 * // Any function, here one client's string commands.
 * ISource2ModFunction* fn = g_pSource2ModFunctions->Find("CServerSideClient::ExecuteStringCommand");
 * g_pSource2ModHooks->Hook(HookPoint::Function(fn), HookTarget::Instance(client), OnStringCommand);
 * @endcode
 */

#define SOURCE2MOD_HOOKS_INTERFACE "ISource2ModHooks001"

/* =========================
Targets and filters
========================= */

struct HookTarget
{
    enum class Kind : int32_t
    {
        All = 0,
        Instance,
        Class,
    };

    Kind kind = Kind::All;
    void* instance = nullptr;
    const char* className = nullptr;    // Kind::Class: the RTTI name ("CCSPlayerPawn")

    static HookTarget All() { return {}; }

    static HookTarget Instance(const void* object)
    {
        HookTarget t;
        t.kind = Kind::Instance;
        t.instance = const_cast<void*>(object);
        return t;
    }

    static HookTarget Class(const char* name)
    {
        HookTarget t;
        t.kind = Kind::Class;
        t.className = name;
        return t;
    }
};

/// Compared before a handler runs; nullptr or "" matches anything.
struct HookFilter
{
    const char* designerName = nullptr;   // the object is an entity: its designer name
    const char* name = nullptr;           // AcceptInput: the input
};

/* =========================
Points
========================= */

/// Every point. The numbers are SourcePawn's HookPoint_* too.
enum class HookPointId : int32_t
{
    // Game functions (ISource2ModHookContexts.h).
    TakeDamage = 0,
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
    PlayerChat,

    // Entity virtuals, on the target's class vtable (SDKHooks).
    Spawn = 50,
    Use,
    StartTouch,
    EndTouch,
    Blocked,
    Think,
    Killed,
    WeaponPickup,
    WeaponSwitch,
    GetItemInLoadout,   // CCSPlayerInventory, not an entity: All() covers every player

    // Engine callbacks.
    GameFrame = 100,
    StartupServer,
    ClientConnect,
    ClientConnected,
    ClientPutInServer,
    ClientDisconnect,
    ClientVoice,
    ClientSettingsChanged,
    GameServerSteamAPIActivated,
    GameServerSteamAPIDeactivated,
    LoadEventsFromFile,

    // HookPoint::Function.
    Function = 300,
};

/// A point and the context its handlers get, so Hook() checks the handler's
/// signature at compile time.
template <typename CONTEXT>
struct HookPointOf
{
    HookPointId id;
    ISource2ModFunction* function = nullptr;    // HookPointId::Function only
};

/* =========================
Function points
========================= */

/**
 * The context of HookPoint::Function: the arguments by index, as the
 * function describes them (`this` of a member function is index 0), and the
 * return value.
 *
 * SetArg in a pre handler changes what the original (and the later
 * handlers) get. SetReturn with Override / Supersede is what
 * the caller gets; in a post handler GetReturn is what the original returned.
 * A Vector / string argument is read as a pointer (GetArg<Vector*>).
 */
class FunctionContext
{
public:
    using Return = void;

    ISource2ModFunction* Function() const { return function_; }

    /// `this` of a member function, the first argument otherwise.
    void* This() const { return GetArg<void*>(0); }

    template <typename T>
    T GetArg(int index) const
    {
        T v{};
        ReadArg(index, &v, sizeof(T));
        return v;
    }

    template <typename T>
    void SetArg(int index, T value)
    {
        WriteArg(index, &value, sizeof(T));
    }

    template <typename T>
    T GetReturn() const
    {
        T v{};
        ReadReturn(&v, sizeof(T));
        return v;
    }

    template <typename T>
    void SetReturn(T value)
    {
        WriteReturn(&value, sizeof(T));
    }

    /// The function now, with the current arguments, past every hook on it.
    /// In a pre handler answer Supersede afterwards (with SetReturn), or the
    /// game calls it a second time.
    template <typename T = void>
    T CallOriginal()
    {
        if constexpr (std::is_void_v<T>)
            CallOriginalRaw(nullptr, 0);
        else
        {
            T v{};
            CallOriginalRaw(&v, sizeof(T));
            return v;
        }
    }

protected:
    virtual ~FunctionContext() = default;
    virtual void ReadArg(int index, void* out, size_t size) const = 0;
    virtual void WriteArg(int index, const void* in, size_t size) = 0;
    virtual void ReadReturn(void* out, size_t size) const = 0;
    virtual void WriteReturn(const void* in, size_t size) = 0;
    virtual void CallOriginalRaw(void* out, size_t size) = 0;

    ISource2ModFunction* function_ = nullptr;
};

/* =========================
Handlers
========================= */

/**
 * What a handler answers: an Action, and for a function with a return value
 * optionally the value -- KHook's { action, value }.
 *
 * @code
 * return { Action::Supersede, 0 };              // skip the original, it returns 0
 * return { Action::Override, AcquireResult::NotAllowedByMode };
 * return { Action::Supersede, ctx.CallOriginal() };
 * return Action::Ignore;                         // an action alone
 * @endcode
 *
 * An action alone takes its value from ctx.result, so a handler that sets
 * the field and returns Override / Supersede keeps working. Several handlers
 * on one point: Supersede stops the chain, otherwise the strongest action
 * wins; each value a handler returns lands in ctx.result right away.
 */
template <typename RETURN>
struct HookReturn
{
    Action action;
    RETURN value{};
    bool hasValue = false;

    HookReturn(Action a) : action(a) {}
    HookReturn(Action a, RETURN v) : action(a), value(v), hasValue(true) {}
};

template <>
struct HookReturn<void>
{
    Action action;

    HookReturn(Action a) : action(a) {}
};

/// A handler: the context, and whether this is the post pass. A function, an
/// object and a method ({ this, &Plugin::Method }), or a lambda.
template <typename CONTEXT>
using HookHandler = Source2ModCallback<HookReturn<typename CONTEXT::Return>(CONTEXT& ctx, bool post)>;

namespace HookPoint
{
    // Game functions.
    inline constexpr HookPointOf<TakeDamageContext> TakeDamage{ HookPointId::TakeDamage };
    inline constexpr HookPointOf<CanAcquireContext> CanAcquire{ HookPointId::CanAcquire };
    inline constexpr HookPointOf<CanMoveContext> CanMove{ HookPointId::CanMove };
    inline constexpr HookPointOf<CanUseContext> CanUse{ HookPointId::CanUse };
    inline constexpr HookPointOf<PostThinkContext> PostThink{ HookPointId::PostThink };
    inline constexpr HookPointOf<ProcessUsercmdsContext> ProcessUsercmds{ HookPointId::ProcessUsercmds };
    inline constexpr HookPointOf<SimulateUserCommandsContext> SimulateUserCommands{ HookPointId::SimulateUserCommands };
    inline constexpr HookPointOf<RunCommandContext> RunCommand{ HookPointId::RunCommand };
    inline constexpr HookPointOf<AcceptInputContext> AcceptInput{ HookPointId::AcceptInput };
    inline constexpr HookPointOf<TouchContext> Touch{ HookPointId::Touch };
    inline constexpr HookPointOf<DropWeaponContext> DropWeapon{ HookPointId::DropWeapon };
    inline constexpr HookPointOf<AirAccelerateContext> AirAccelerate{ HookPointId::AirAccelerate };
    inline constexpr HookPointOf<MovementContext> AirMove{ HookPointId::AirMove };
    inline constexpr HookPointOf<MovementContext> CanUnduck{ HookPointId::CanUnduck };
    inline constexpr HookPointOf<CategorizePositionContext> CategorizePosition{ HookPointId::CategorizePosition };
    inline constexpr HookPointOf<MovementContext> CheckFalling{ HookPointId::CheckFalling };
    inline constexpr HookPointOf<MovementContext> CheckParameters{ HookPointId::CheckParameters };
    inline constexpr HookPointOf<CheckVelocityContext> CheckVelocity{ HookPointId::CheckVelocity };
    inline constexpr HookPointOf<MovementContext> CheckWater{ HookPointId::CheckWater };
    inline constexpr HookPointOf<MovementContext> Duck{ HookPointId::Duck };
    inline constexpr HookPointOf<MovementContext> Friction{ HookPointId::Friction };
    inline constexpr HookPointOf<FullWalkMoveContext> FullWalkMove{ HookPointId::FullWalkMove };
    inline constexpr HookPointOf<GroundAccelerateContext> GroundAccelerate{ HookPointId::GroundAccelerate };
    inline constexpr HookPointOf<MovementContext> LadderMove{ HookPointId::LadderMove };
    inline constexpr HookPointOf<MovementContext> MoveInit{ HookPointId::MoveInit };
    inline constexpr HookPointOf<MovementContext> PlayerMove{ HookPointId::PlayerMove };
    inline constexpr HookPointOf<MovementContext> ProcessMovement{ HookPointId::ProcessMovement };
    inline constexpr HookPointOf<SetupMoveContext> SetupMove{ HookPointId::SetupMove };
    inline constexpr HookPointOf<TryPlayerMoveContext> TryPlayerMove{ HookPointId::TryPlayerMove };
    inline constexpr HookPointOf<MovementContext> WalkMove{ HookPointId::WalkMove };
    inline constexpr HookPointOf<MovementContext> WaterMove{ HookPointId::WaterMove };
    inline constexpr HookPointOf<LegacyJumpContext> OnJumpLegacy{ HookPointId::OnJumpLegacy };
    inline constexpr HookPointOf<ModernJumpContext> OnJumpModern{ HookPointId::OnJumpModern };
    inline constexpr HookPointOf<LegacyJumpContext> CheckJumpButtonLegacy{ HookPointId::CheckJumpButtonLegacy };
    inline constexpr HookPointOf<ModernJumpContext> CheckJumpButtonModern{ HookPointId::CheckJumpButtonModern };
    inline constexpr HookPointOf<PlayerChatContext> PlayerChat{ HookPointId::PlayerChat };

    // Entity virtuals (SDKHooks): HookTarget::Instance(entity) or Class(name).
    inline constexpr HookPointOf<SpawnContext> Spawn{ HookPointId::Spawn };
    inline constexpr HookPointOf<UseContext> Use{ HookPointId::Use };
    inline constexpr HookPointOf<EntityOtherContext> StartTouch{ HookPointId::StartTouch };
    inline constexpr HookPointOf<EntityOtherContext> EndTouch{ HookPointId::EndTouch };
    inline constexpr HookPointOf<EntityOtherContext> Blocked{ HookPointId::Blocked };
    inline constexpr HookPointOf<ThinkContext> Think{ HookPointId::Think };
    inline constexpr HookPointOf<KilledContext> Killed{ HookPointId::Killed };
    inline constexpr HookPointOf<WeaponPickupContext> WeaponPickup{ HookPointId::WeaponPickup };
    inline constexpr HookPointOf<WeaponSwitchContext> WeaponSwitch{ HookPointId::WeaponSwitch };
    inline constexpr HookPointOf<GetItemInLoadoutContext> GetItemInLoadout{ HookPointId::GetItemInLoadout };

    // Engine callbacks.
    inline constexpr HookPointOf<GameFrameContext> GameFrame{ HookPointId::GameFrame };
    inline constexpr HookPointOf<StartupServerContext> StartupServer{ HookPointId::StartupServer };
    inline constexpr HookPointOf<ClientConnectContext> ClientConnect{ HookPointId::ClientConnect };
    inline constexpr HookPointOf<ClientConnectedContext> ClientConnected{ HookPointId::ClientConnected };
    inline constexpr HookPointOf<ClientPutInServerContext> ClientPutInServer{ HookPointId::ClientPutInServer };
    inline constexpr HookPointOf<ClientDisconnectContext> ClientDisconnect{ HookPointId::ClientDisconnect };
    inline constexpr HookPointOf<ClientSlotContext> ClientVoice{ HookPointId::ClientVoice };
    inline constexpr HookPointOf<ClientSlotContext> ClientSettingsChanged{ HookPointId::ClientSettingsChanged };
    inline constexpr HookPointOf<EmptyContext> GameServerSteamAPIActivated{ HookPointId::GameServerSteamAPIActivated };
    inline constexpr HookPointOf<EmptyContext> GameServerSteamAPIDeactivated{ HookPointId::GameServerSteamAPIDeactivated };
    inline constexpr HookPointOf<LoadEventsFromFileContext> LoadEventsFromFile{ HookPointId::LoadEventsFromFile };


    /// Any function from ISource2ModFunctions.
    inline HookPointOf<FunctionContext> Function(ISource2ModFunction* fn) { return { HookPointId::Function, fn }; }
}

/* =========================
The interface
========================= */

// Keeps the handler out of template deduction, so a plain function or a lambda
// converts to the point's HookHandler (CONTEXT comes from the point alone).
template <typename T>
struct HookNonDeduced
{
    using type = T;
};

class ISource2ModHooks
{
public:
    virtual ~ISource2ModHooks() = default;

    /**
     * Hooks @p point on @p target.
     *
     * @param post    false: before the original (may change arguments,
     *                Override / Supersede); true: after it (sees the result).
     * @param filter  Designer / output / input name, for the points that take one.
     * @return The hook's id for Unhook(); 0 when the point does not exist on
     *         this game, the target does not fit the point, or the function
     *         of a Function point is null.
     */
    template <typename CONTEXT>
    Source2ModHookId Hook(HookPointOf<CONTEXT> point, const HookTarget& target, typename HookNonDeduced<HookHandler<CONTEXT>>::type handler,
                          bool post = false, const HookFilter& filter = {})
    {
        return AddHook(point.id, point.function, target, filter, &handler, post);
    }

    /// Drops one hook. @return false when there is no such hook.
    virtual bool Unhook(Source2ModHookId id) = 0;

    /// Whether the point resolves on the running game (its gamedata matched).
    virtual bool IsAvailable(HookPointId point) = 0;

    /**
     * Type-erased Hook(): @p handler is a HookHandler<CONTEXT>* for
     * @p point's context, copied by the core. Hook() is the way in.
     */
    virtual Source2ModHookId AddHook(HookPointId point, ISource2ModFunction* function, const HookTarget& target,
                                     const HookFilter& filter, const void* handler, bool post) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_HOOKS_H
