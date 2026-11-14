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

#ifndef _INCLUDE_ISOURCE2MOD_FUNCTIONS_H
#define _INCLUDE_ISOURCE2MOD_FUNCTIONS_H

#pragma once

#include <source2mod/ISource2ModTypes.h>

#include "ISource2ModModule.h"
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

#define SOURCE2MOD_FUNCTIONS_INTERFACE "ISource2ModFunctions003"

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

struct GameTime_t;
class CEntityInstance;

/**
 * What a C++ type is as a Source2ModType, where the rules of TypeOf() below
 * get it wrong: a small class passed by value that holds a float, say.
 *
 * @code
 * template <> struct Source2ModTypeTraits<MyFloatWrapper> { static constexpr Source2ModType value = Source2ModType::Float; };
 * @endcode
 */
template <typename T>
struct Source2ModTypeTraits
{
};

template <>
struct Source2ModTypeTraits<GameTime_t>
{
    static constexpr Source2ModType value = Source2ModType::Float;
};

namespace source2mod_functions_detail
{
    template <typename T, typename = void>
    struct HasTypeTraits : std::false_type
    {
    };

    template <typename T>
    struct HasTypeTraits<T, std::void_t<decltype(Source2ModTypeTraits<T>::value)>> : std::true_type
    {
    };

    template <typename T, typename = void>
    struct IsComplete : std::false_type
    {
    };

    template <typename T>
    struct IsComplete<T, std::void_t<decltype(sizeof(T))>> : std::true_type
    {
    };

    // A pointer to an entity class (CEntityInstance among its bases, its
    // definition included): Entity, which a hook's first parameter needs for
    // HookFilter::designerName.
    template <typename P>
    constexpr bool IsEntityClass()
    {
        if constexpr (std::is_class_v<P> && IsComplete<P>::value)
            return std::is_base_of_v<::CEntityInstance, P>;
        else
            return false;
    }

    template <size_t N>
    constexpr Source2ModType IntOfSize()
    {
        return N == 8 ? Source2ModType::Int64 : N == 2 ? Source2ModType::Int16 : N == 1 ? Source2ModType::Int8 : Source2ModType::Int32;
    }

    // What a C++ parameter or return type is in Source2ModType terms.
    // `Pointer` stands for any pointer: it matches entity, object and pointer
    // in the gamedata, whichever the C++ side calls it.
    template <typename T>
    constexpr Source2ModType TypeOf()
    {
        using U = std::remove_cv_t<T>;
        if constexpr (HasTypeTraits<U>::value)
            return Source2ModTypeTraits<U>::value;
        else if constexpr (std::is_void_v<U>)
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

    // A parameter: as TypeOf(), except a small trivially copyable class by
    // value (CPlayerSlot, CEntityHandle, CSplitScreenSlot), which both ABIs
    // pass in an integer register like an integer of its size. A larger or
    // non-trivial one goes by reference: a pointer.
    template <typename T>
    constexpr Source2ModType ParamTypeOf()
    {
        using U = std::remove_cv_t<T>;
        if constexpr (HasTypeTraits<U>::value)
            return Source2ModTypeTraits<U>::value;
        else if constexpr (std::is_class_v<U> && !std::is_same_v<U, ::Vector> && !std::is_same_v<U, ::QAngle>)
        {
            if constexpr (std::is_trivially_copyable_v<U> && (sizeof(U) == 1 || sizeof(U) == 2 || sizeof(U) == 4 || sizeof(U) == 8))
                return IntOfSize<sizeof(U)>();
            else
                return Source2ModType::Pointer;
        }
        else
            return TypeOf<T>();
    }

    template <typename Sig>
    struct Signature;

    // A function's description (Source2ModFunctionTypes): a pointer to an
    // entity class is Entity there, which a hook's first parameter needs for
    // HookFilter::designerName. A call's check (Call<Sig>) keeps Pointer,
    // which every core takes for an entity.
    template <typename T>
    constexpr Source2ModType DescribedTypeOf(Source2ModType t)
    {
        if constexpr (std::is_pointer_v<T>)
        {
            if (t == Source2ModType::Pointer && IsEntityClass<std::remove_cv_t<std::remove_pointer_t<T>>>())
                return Source2ModType::Entity;
        }
        return t;
    }

    template <typename R, typename... P>
    struct Signature<R(P...)>
    {
        using Return = R;
        using Pointer = R (*)(P...);
        static constexpr int kCount = sizeof...(P);
        static constexpr Source2ModType kReturn = TypeOf<R>();
        static constexpr Source2ModType kParams[sizeof...(P) + 1] = { ParamTypeOf<P>()..., Source2ModType::Void };
        static constexpr Source2ModType kDescribedReturn = DescribedTypeOf<R>(TypeOf<R>());
        static constexpr Source2ModType kDescribedParams[sizeof...(P) + 1] = { DescribedTypeOf<P>(ParamTypeOf<P>())..., Source2ModType::Void };
    };

    // A function's type in any spelling -- a function type, a pointer to one,
    // a pointer to member (const / noexcept too), or a typedef / using of any
    // of them -- as the Signature it is called with (`this` first for a
    // member) and whether it is a member.
    template <typename F>
    struct FunctionOf;

    template <typename R, typename... P>
    struct FunctionOf<R(P...)>
    {
        using Sig = R(P...);
        static constexpr bool kMember = false;
    };

    template <typename R, typename... P>
    struct FunctionOf<R(P...) noexcept> : FunctionOf<R(P...)>
    {
    };

    template <typename F>
    struct FunctionOf<F*> : FunctionOf<F>
    {
    };

    template <typename F>
    struct FunctionOf<F&> : FunctionOf<F>
    {
    };

#define SOURCE2MOD_FUNCTION_OF_MEMBER(QUALIFIERS)               \
    template <typename R, typename C, typename... P>            \
    struct FunctionOf<R (C::*)(P...) QUALIFIERS>                \
    {                                                           \
        using Sig = R(C*, P...);                                \
        static constexpr bool kMember = true;                   \
    };
    SOURCE2MOD_FUNCTION_OF_MEMBER()
    SOURCE2MOD_FUNCTION_OF_MEMBER(const)
    SOURCE2MOD_FUNCTION_OF_MEMBER(noexcept)
    SOURCE2MOD_FUNCTION_OF_MEMBER(const noexcept)
#undef SOURCE2MOD_FUNCTION_OF_MEMBER

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

/**
 * The FunctionTypes of a C++ function type, read off the type instead of
 * listed by hand: a pointer to member (a virtual of an engine interface), a
 * function pointer, a function type, or a typedef / using of one.
 *
 * @code
 * Source2ModFunctionTypes<decltype(&IVEngineServer2::SetClientListening)>()   // bool (IVEngineServer2* this, CPlayerSlot, CPlayerSlot, bool)
 * using SwitchTeam_t = void (CCSPlayerController::*)(int);
 * Source2ModFunctionTypes<SwitchTeam_t>()                                    // member: this, int
 * Source2ModFunctionTypes<void(CBaseEntity*, bool)>()                        // free function of these parameters
 * Source2ModFunctionTypes(&IVEngineServer2::SetClientListening)             // the same from a value
 * @endcode
 *
 * A small trivially copyable class by value is an integer of its size;
 * Source2ModTypeTraits says otherwise for one that is not (GameTime_t: Float).
 */
template <typename F>
constexpr FunctionTypes Source2ModFunctionTypes()
{
    using Of = source2mod_functions_detail::FunctionOf<std::remove_cv_t<std::remove_reference_t<F>>>;
    using S = source2mod_functions_detail::Signature<typename Of::Sig>;
    return FunctionTypes{ S::kDescribedReturn, S::kDescribedParams, S::kCount, Of::kMember };
}

template <typename F>
constexpr FunctionTypes Source2ModFunctionTypes(F)
{
    return Source2ModFunctionTypes<F>();
}

class ISource2ModFunctions
{
public:
    virtual ~ISource2ModFunctions() = default;

    /**
     * The function a gamedata entry with a "function" object describes: its
     * signature, or a virtual's offset ("this" required). A virtual hooks per
     * instance or class; "vtable": { "module", "class", "base" } in the object
     * ties it to one class's vtable, for HookTarget::All().
     *
     * @return nullptr when there is no such entry, it has no "function", or
     *         its signature or vtable does not match the running game (the
     *         reason is on the console).
     */
    virtual ISource2ModFunction* Find(const char* name) = 0;

    /**
     * A function described here instead of in gamedata.
     *
     * @param owner       The plugin it belongs to; the overload without it passes this plugin; freed when it unloads.
     * @param address     The function; nullptr for a virtual.
     * @param vtable      For a virtual called through one class's vtable whatever the
     *                    object's class; nullptr to use the object's own.
     * @param vtableIndex The slot of a virtual; -1 for a function at @p address.
     * @param isMember    Whether the first parameter is `this` (a virtual always is).
     * @param params      The parameter types, `this` included.
     */
    virtual ISource2ModFunction* Create(void* address, void** vtable, int vtableIndex, bool isMember,
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
    virtual ISource2ModFunction* FromGameConfig(const char* name, const FunctionTypes& types) = 0;

    /// A signature of the plugin's own in a module ("server"), optionally only
    /// in one section (".text").
    virtual ISource2ModFunction* FromSignature(ISource2ModModule* module, const char* signature,
                                               const FunctionTypes& types, const char* section = "") = 0;

    /// The function a matched call / jmp / lea refers to: the rel32 at
    /// @p offset from the match, relative to the next instruction at
    /// @p nextInstructionOffset (a call or jmp at the start: 1, 5).
    virtual ISource2ModFunction* FromSignatureRelative(ISource2ModModule* module, const char* signature, int offset,
                                                       int nextInstructionOffset, const FunctionTypes& types,
                                                       const char* section = "") = 0;

    /// An exported or debug symbol in a module.
    virtual ISource2ModFunction* FromSymbol(ISource2ModModule* module, const char* symbol, const FunctionTypes& types) = 0;

    /// Slot @p index of one class's vtable (an RTTI name in a module; @p baseName
    /// picks a secondary base's vtable): called on any object, hooked on that
    /// class.
    virtual ISource2ModFunction* FromVirtualTable(ISource2ModModule* module, const char* className, int index,
                                                  const FunctionTypes& types, const char* baseName = "") = 0;

    /// Slot @p index of whatever object it is called / hooked on (an engine
    /// interface: FromVirtual(KHook::GetVtableIndex(&ISource2Server::GameFrame), ...)
    /// with HookTarget::Instance(g_pSource2Server)).
    virtual ISource2ModFunction* FromVirtual(int index, const FunctionTypes& types) = 0;

    /// A think, touch or use function by its name in an entity class's datamap,
    /// which the game keeps, so no signature is needed; the class's parents are
    /// searched too. FromDatamap("CBaseButton", "CBaseButtonButtonUse", types).
    /// Null before a map is up: the entity system holds the classes.
    virtual ISource2ModFunction* FromDatamap(const char* className, const char* functionName, const FunctionTypes& types) = 0;

    /*
     * The same with the types read off a C++ function type instead of a
     * FunctionTypes (Source2ModFunctionTypes): a pointer to member for a
     * member function, so `this` and isMember come with it.
     *
     *   FromVirtual(&IVEngineServer2::SetClientListening)       // slot and types from the method
     *   FromGameConfig<void (CCSPlayerController::*)(int)>("CCSPlayerController::SwitchTeam")
     *   using Think_t = void (CBaseEntity::*)();
     *   FromDatamap<Think_t>("CBaseButton", "CBaseButtonButtonUse")
     */

    /// A virtual of an engine interface or a class: its slot (KHook::GetVtableIndex) and types from the method.
    template <typename M, std::enable_if_t<std::is_member_function_pointer_v<M>, int> = 0>
    ISource2ModFunction* FromVirtual(M method)
    {
        const int index = KHook::GetVtableIndex(method);
        return index < 0 ? nullptr : FromVirtual(index, Source2ModFunctionTypes<M>());
    }

    /// A free function of the plugin's process: its address and types from the pointer.
    template <typename R, typename... P>
    ISource2ModFunction* Create(R (*function)(P...))
    {
        constexpr FunctionTypes types = Source2ModFunctionTypes<R (*)(P...)>();
        return Create(reinterpret_cast<void*>(function), nullptr, -1, false, types.returnType, types.params, types.count);
    }

    template <typename F>
    ISource2ModFunction* FromVirtual(int index)
    {
        return FromVirtual(index, Source2ModFunctionTypes<F>());
    }

    template <typename F>
    ISource2ModFunction* FromGameConfig(const char* name)
    {
        return FromGameConfig(name, Source2ModFunctionTypes<F>());
    }

    template <typename F>
    ISource2ModFunction* FromSignature(ISource2ModModule* module, const char* signature, const char* section = "")
    {
        return FromSignature(module, signature, Source2ModFunctionTypes<F>(), section);
    }

    template <typename F>
    ISource2ModFunction* FromSignatureRelative(ISource2ModModule* module, const char* signature, int offset, int nextInstructionOffset,
                                               const char* section = "")
    {
        return FromSignatureRelative(module, signature, offset, nextInstructionOffset, Source2ModFunctionTypes<F>(), section);
    }

    template <typename F>
    ISource2ModFunction* FromSymbol(ISource2ModModule* module, const char* symbol)
    {
        return FromSymbol(module, symbol, Source2ModFunctionTypes<F>());
    }

    template <typename F>
    ISource2ModFunction* FromVirtualTable(ISource2ModModule* module, const char* className, int index, const char* baseName = "")
    {
        return FromVirtualTable(module, className, index, Source2ModFunctionTypes<F>(), baseName);
    }

    /// A virtual of one class's vtable: its slot and types from the method.
    template <typename M, std::enable_if_t<std::is_member_function_pointer_v<M>, int> = 0>
    ISource2ModFunction* FromVirtualTable(ISource2ModModule* module, const char* className, M method, const char* baseName = "")
    {
        const int index = KHook::GetVtableIndex(method);
        return index < 0 ? nullptr : FromVirtualTable(module, className, index, Source2ModFunctionTypes<M>(), baseName);
    }

    template <typename F>
    ISource2ModFunction* FromDatamap(const char* className, const char* functionName)
    {
        return FromDatamap(className, functionName, Source2ModFunctionTypes<F>());
    }
};

/**
 * A gamedata function by name (its "function" object), called through the
 * hooks (HookChain::Run) or straight to the original (HookChain::Bypass) --
 * how the SDK's schema helpers (Teleport, SwitchTeam, GiveNamedItem, ...) make
 * their calls. Found once per call site. When the entry is missing or does not
 * match @p Sig nothing is called and the result is value-initialised.
 */
template <typename Sig, typename... A>
inline typename source2mod_functions_detail::Signature<Sig>::Return
Source2ModCallFunction(ISource2ModFunctions* functions, ISource2ModFunction*& cache, const char* name, HookChain chain, A&&... args)
{
    using R = typename source2mod_functions_detail::Signature<Sig>::Return;
    if (!cache && functions)
        cache = functions->Find(name);
    if (!cache)
    {
        if constexpr (std::is_void_v<R>)
            return;
        else
            return R{};
    }
    if (chain == HookChain::Bypass)
        return cache->template CallOriginal<Sig>(static_cast<A&&>(args)...);
    return cache->template Call<Sig>(static_cast<A&&>(args)...);
}

/// SOURCE2MOD_CALL_FUNCTION(void(CCSPlayerController*, int), "CCSPlayerController::SwitchTeam", eChain, this, nTeam)
#define SOURCE2MOD_CALL_FUNCTION(SIG, NAME, CHAIN, ...)                                                   \
    ([&]() -> typename source2mod_functions_detail::Signature<SIG>::Return {                              \
        static ISource2ModFunction* s_function = nullptr;                                                 \
        return Source2ModCallFunction<SIG>(g_pSource2ModFunctions, s_function, NAME, CHAIN, __VA_ARGS__); \
    }())

#endif //_INCLUDE_ISOURCE2MOD_FUNCTIONS_H
