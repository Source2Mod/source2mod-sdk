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
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl gives
 * you permission to link the code of this program (as well as its derivative
 * works) to "Counter-Strike 2," "Source 2," "Steam," and any Game MODs or
 * server software running on software by Valve Corporation. You must obey the
 * GNU General Public License in all respects for all other code used.
 *
 * Additionally, this exception applies to all derivative works unless
 * otherwise stated in LICENSE.txt.
 *
 * Authors:
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: Source2Mod
 */

#ifndef _INCLUDE_ISOURCE2MOD_FUNCTIONS_H
#define _INCLUDE_ISOURCE2MOD_FUNCTIONS_H

#pragma once

#include "ISource2ModTypes.h"

#include <cstddef>
#include <cstdint>
#include <type_traits>

class CEntityInstance;
class CBufferString;
class CCommand;
class Vector;
class QAngle;

/*
 * Game functions by name, described once in gamedata and called the same way
 * from C++ and SourcePawn (Source2ModFunction.Find) -- SourceMod's SDKCall, with
 * the signature in the gamedata instead of in every plugin.
 *
 * A gamedata entry that has a "function" object is one. Where the function
 * is comes from the entry as before: "signatures" (a pattern or an "@symbol"),
 * or "offsets" for a virtual called on `this`. The object says what it takes:
 *
 *   "CBaseEntity::CollisionRulesChanged": {
 *     "offsets": { "windows": 188, "linux": 187 },
 *     "function": { "this": "entity", "return": "void", "params": [] }
 *   },
 *   "GetWeaponCSDataFromKey": {
 *     "signatures": { "library": "server", "windows": "...", "linux": "..." },
 *     "function": { "return": "pointer", "params": [ "int32", "string" ] }
 *   }
 *
 *   "this":   "entity" | "object" | "pointer" -- a member function; `this` is
 *             the first argument of a call. Left out: a free function. A
 *             virtual ("offsets") needs one.
 *   "return": a type, "void" when left out.
 *   "params": the types after `this`; or { "windows": [...], "linux": [...] }
 *             when they differ between the platforms.
 *
 * Types (Source2ModType; SourcePawn's Source2ModType_* has the same values):
 * void, bool, int8, int16, int32 (also enums, uint32), int64, float, double,
 * entity (CEntityInstance* and derived), object (a schema object pointer),
 * string (const char*), vector (Vector* / const Vector&), vector_value (Vector
 * by value), pointer (anything else), buffer_string (CBufferString*), command
 * (const CCommand&), int_ref (int32*), bool_ref (bool*), float_ref (float*).
 *
 * A plugin's own gamedata file in addons/source2mod/gamedata is read with the
 * core's, so a plugin describes its own functions the same way.
 *
 * @code
 * ISource2ModFunction* fn = g_pSource2ModFunctions->Find("CBaseEntity::CollisionRulesChanged");
 * if (fn)
 *     fn->Call<void(CBaseEntity*)>(pEntity);
 *
 * auto* data = g_pSource2ModFunctions->Find("GetWeaponCSDataFromKey")
 *                  ->Call<CCSWeaponBaseVData*(int, const char*)>(-1, "weapon_ak47");
 * @endcode
 *
 * Call<Signature> checks the signature against the gamedata on every call (a
 * few compares) and calls nothing when they differ: it reports the mismatch
 * on the console and returns a value-initialised result.
 */

#define SOURCE2MOD_FUNCTIONS_INTERFACE "ISource2ModFunctions002"

/**
 * How a value crosses into a game function and back. The numbers are
 * SourcePawn's Source2ModType_*.
 */
enum class Source2ModType : int32_t
{
    Void = 0,
    Bool,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    Entity,
    Object,
    String,
    Vector,
    VectorValue,
    Pointer,
    BufferString,
    Command,
    IntRef,
    BoolRef,
    FloatRef,
};

namespace source2mod_functions_detail
{
    // What a C++ parameter or return type is in Source2ModType terms.
    // `Pointer` stands for any pointer: it matches entity, object and pointer
    // in the gamedata, whichever the C++ side calls it.
    template <typename T>
    constexpr Source2ModType TypeOf()
    {
        using U = std::remove_cv_t<T>;
        if constexpr (std::is_void_v<U>)
            return Source2ModType::Void;
        else if constexpr (std::is_same_v<U, bool>)
            return Source2ModType::Bool;
        else if constexpr (std::is_enum_v<U>)
            return sizeof(U) == 8 ? Source2ModType::Int64 : sizeof(U) == 2 ? Source2ModType::Int16 : sizeof(U) == 1 ? Source2ModType::Int8 : Source2ModType::Int32;
        else if constexpr (std::is_integral_v<U>)
            return sizeof(U) == 8 ? Source2ModType::Int64 : sizeof(U) == 2 ? Source2ModType::Int16 : sizeof(U) == 1 ? Source2ModType::Int8 : Source2ModType::Int32;
        else if constexpr (std::is_same_v<U, float>)
            return Source2ModType::Float;
        else if constexpr (std::is_same_v<U, double>)
            return Source2ModType::Double;
        else if constexpr (std::is_same_v<U, ::Vector> || std::is_same_v<U, ::QAngle>)
            return Source2ModType::VectorValue;
        else if constexpr (std::is_reference_v<T>)
        {
            using R = std::remove_cv_t<std::remove_reference_t<T>>;
            if constexpr (std::is_same_v<R, ::Vector> || std::is_same_v<R, ::QAngle>)
                return Source2ModType::Vector;
            else if constexpr (std::is_same_v<R, ::CCommand>)
                return Source2ModType::Command;
            else
                return Source2ModType::Pointer;
        }
        else if constexpr (std::is_pointer_v<U>)
        {
            using P = std::remove_cv_t<std::remove_pointer_t<U>>;
            if constexpr (std::is_same_v<P, char>)
                return Source2ModType::String;
            else if constexpr (std::is_same_v<P, ::Vector> || std::is_same_v<P, ::QAngle>)
                return Source2ModType::Vector;
            else if constexpr (std::is_same_v<P, ::CBufferString>)
                return Source2ModType::BufferString;
            else if constexpr (std::is_same_v<P, int32_t>)
                return Source2ModType::IntRef;
            else if constexpr (std::is_same_v<P, bool>)
                return Source2ModType::BoolRef;
            else if constexpr (std::is_same_v<P, float>)
                return Source2ModType::FloatRef;
            else
                return Source2ModType::Pointer;
        }
        else
            return Source2ModType::Pointer;
    }

    template <typename Sig>
    struct Signature;

    template <typename R, typename... P>
    struct Signature<R(P...)>
    {
        using Return = R;
        using Pointer = R (*)(P...);
        static constexpr int kCount = sizeof...(P);
        static constexpr Source2ModType kReturn = TypeOf<R>();
        static constexpr Source2ModType kParams[sizeof...(P) + 1] = { TypeOf<P>()..., Source2ModType::Void };
    };

    template <typename A>
    void* AsThis(A&& arg)
    {
        if constexpr (std::is_pointer_v<std::decay_t<A>>)
            return const_cast<void*>(static_cast<const void*>(arg));
        else
            return nullptr;
    }
}

/**
 * A game function: where it is and what it takes. From Find() (owned by the
 * core, valid until the server shuts down) or Create() (the plugin's, freed
 * with Destroy() or when the plugin unloads).
 */
class ISource2ModFunction
{
public:
    virtual ~ISource2ModFunction() = default;

    /** The gamedata name, "" for one from Create(). */
    virtual const char* GetName() const = 0;

    virtual Source2ModType GetReturnType() const = 0;

    /** Parameters, `this` of a member function included (it is the first). */
    virtual int GetParamCount() const = 0;
    virtual Source2ModType GetParamType(int index) const = 0;

    /** A virtual: the address comes from the vtable of the object it is called on. */
    virtual bool IsVirtual() const = 0;

    /**
     * Where a call goes. A virtual reads it off @p pThis's vtable (or off the
     * fixed vtable it was created with). @p bypassHooks: past every hook on
     * the function (KHook::FindOriginal), as HookChain::Bypass.
     *
     * @return nullptr when there is no address (a virtual without an object).
     */
    virtual void* GetAddress(void* pThis = nullptr, bool bypassHooks = false) const = 0;

    /**
     * Whether a call of this shape matches the gamedata; a mismatch is
     * reported on the console (once per function and shape).
     */
    virtual bool CheckCall(Source2ModType returnType, const Source2ModType* params, int count) const = 0;

    /**
     * Calls it. @p Sig is the function's C++ type with `this` as the first
     * parameter of a member function: Call<void(CBaseEntity*, bool)>(ent, true).
     * Nothing is called when it does not match the gamedata; the result is
     * then value-initialised.
     */
    template <typename Sig, typename... A>
    typename source2mod_functions_detail::Signature<Sig>::Return Call(A&&... args) const
    {
        return Invoke<Sig>(false, static_cast<A&&>(args)...);
    }

    /** Call past every hook on the function (all plugins', raw and game hooks). */
    template <typename Sig, typename... A>
    typename source2mod_functions_detail::Signature<Sig>::Return CallOriginal(A&&... args) const
    {
        return Invoke<Sig>(true, static_cast<A&&>(args)...);
    }

private:
    template <typename Sig, typename... A>
    typename source2mod_functions_detail::Signature<Sig>::Return Invoke(bool bypass, A&&... args) const
    {
        using S = source2mod_functions_detail::Signature<Sig>;
        using R = typename S::Return;
        static_assert(sizeof...(A) == S::kCount, "Call<Sig>: the arguments do not match the signature");

        if (!CheckCall(S::kReturn, S::kParams, S::kCount))
        {
            if constexpr (std::is_void_v<R>)
                return;
            else
                return R{};
        }

        void* pThis = nullptr;
        if constexpr (sizeof...(A) > 0)
            pThis = FirstAsThis(args...);
        void* target = GetAddress(pThis, bypass);
        if (!target)
        {
            if constexpr (std::is_void_v<R>)
                return;
            else
                return R{};
        }

#if defined(_WIN32)
        // MSVC returns a class from a member function through a hidden
        // pointer after `this`, from a free function before the arguments;
        // a free-function pointer would put it first.
        if constexpr (std::is_class_v<R> && sizeof...(A) > 0)
        {
            if (GetParamCount() > 0 && IsMember())
                return CallMemberReturningClass<R>(target, static_cast<A&&>(args)...);
        }
#endif
        return reinterpret_cast<typename S::Pointer>(target)(static_cast<A&&>(args)...);
    }

    template <typename First, typename... Rest>
    static void* FirstAsThis(First&& first, Rest&&...)
    {
        return source2mod_functions_detail::AsThis(static_cast<First&&>(first));
    }

#if defined(_WIN32)
    template <typename R, typename First, typename... Rest>
    static R CallMemberReturningClass(void* target, First&& self, Rest&&... rest)
    {
        R result;
        using Fn = R* (*)(std::decay_t<First>, R*, Rest...);
        reinterpret_cast<Fn>(target)(static_cast<First&&>(self), &result, static_cast<Rest&&>(rest)...);
        return result;
    }
#endif

public:
    /** A member function ("this" in the gamedata, or a virtual). */
    virtual bool IsMember() const = 0;
};

/**
 * What a function takes and returns, for one found without a gamedata
 * "function" object (ISource2ModFunctions::From*).
 *
 * @code
 * const Source2ModType params[] = { Source2ModType::Entity, Source2ModType::Bool };
 * FunctionTypes types{ Source2ModType::Void, params, 2, true };   // void (CBaseEntity* this, bool)
 * @endcode
 */
struct FunctionTypes
{
    Source2ModType returnType = Source2ModType::Void;
    const Source2ModType* params = nullptr;     // `this` included for a member function
    int count = 0;
    bool isMember = false;                      // the first parameter is `this`
};

class ISource2ModFunctions
{
public:
    virtual ~ISource2ModFunctions() = default;

    /**
     * The function a gamedata entry with a "function" object describes.
     *
     * @return nullptr when there is no such entry, it has no "function", or
     *         its signature does not match the running game (the reason is on
     *         the console).
     */
    virtual ISource2ModFunction* Find(const char* name) = 0;

    /**
     * A function described here instead of in gamedata.
     *
     * @param owner       The plugin it belongs to (g_PluginID); freed when it unloads.
     * @param address     The function; nullptr for a virtual.
     * @param vtable      For a virtual called through one class's vtable whatever the
     *                    object's class; nullptr to use the object's own.
     * @param vtableIndex The slot of a virtual; -1 for a function at @p address.
     * @param isMember    Whether the first parameter is `this` (a virtual always is).
     * @param params      The parameter types, `this` included.
     */
    virtual ISource2ModFunction* Create(PluginId owner, void* address, void** vtable, int vtableIndex, bool isMember,
                                        Source2ModType returnType, const Source2ModType* params, int count) = 0;

    /** Frees one from Create() / From*(). One from Find() is left alone. */
    virtual void Destroy(ISource2ModFunction* function) = 0;

    /*
     * Found here, described by @p types: the plugin's (@p owner), freed with
     * Destroy() or when it unloads. nullptr when the function is not found.
     * Each can be called (Call<Sig>) and hooked (HookPoint::Function).
     */

    /// A gamedata entry (its signature, symbol or -- a virtual -- offset),
    /// with the types given here instead of in a "function" object.
    virtual ISource2ModFunction* FromGameConfig(PluginId owner, const char* name, const FunctionTypes& types) = 0;

    /// A signature of the plugin's own in a module ("server"), optionally only
    /// in one section (".text").
    virtual ISource2ModFunction* FromSignature(PluginId owner, const char* module, const char* signature,
                                               const FunctionTypes& types, const char* section = "") = 0;

    /// The function a matched call / jmp / lea refers to: the rel32 at
    /// @p offset from the match, relative to the next instruction at
    /// @p nextInstructionOffset (a call or jmp at the start: 1, 5).
    virtual ISource2ModFunction* FromSignatureRelative(PluginId owner, const char* module, const char* signature, int offset,
                                                       int nextInstructionOffset, const FunctionTypes& types,
                                                       const char* section = "") = 0;

    /// An exported or debug symbol in a module.
    virtual ISource2ModFunction* FromSymbol(PluginId owner, const char* module, const char* symbol, const FunctionTypes& types) = 0;

    /// Slot @p index of one class's vtable (an RTTI name in a module; @p baseName
    /// picks a secondary base's vtable): called on any object, hooked on that
    /// class.
    virtual ISource2ModFunction* FromVirtualTable(PluginId owner, const char* module, const char* className, int index,
                                                  const FunctionTypes& types, const char* baseName = "") = 0;

    /// Slot @p index of whatever object it is called / hooked on (an engine
    /// interface: FromVirtual(g_PluginID, KHook::GetVtableIndex(&ISource2Server::GameFrame), ...)
    /// with HookTarget::Instance(g_pSource2Server)).
    virtual ISource2ModFunction* FromVirtual(PluginId owner, int index, const FunctionTypes& types) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_FUNCTIONS_H
