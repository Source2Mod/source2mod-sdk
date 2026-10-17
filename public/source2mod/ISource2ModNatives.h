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
 * @file ISource2ModNatives.h
 * @brief Natives for SourcePawn plugins, from a native (C++) plugin.
 *
 * What a SourceMod extension does for its scripts: register natives a .sp
 * declares and calls, a library its scripts can test for, and handles of a
 * type of its own. The natives are SourcePawn's -- SPVM_NATIVE_FUNC, called
 * with the script's IPluginContext and its parameters -- so the SourcePawn
 * headers come with this one (public/sourcepawn, the core's VM version).
 *
 * SOURCE2MOD_SAVEVARS() fills g_pSource2ModNatives like every other global
 * (null on a core without script support); including this header is only
 * needed to call it.
 *
 * @code
 * g_pSource2ModNatives->AddNatives(g_PluginID, g_MyNatives);
 * @endcode
 *
 * Everything registered here goes when the plugin unloads: its natives
 * become unbound (a script calling one gets an error), its libraries are
 * removed (OnLibraryRemoved) and its handle types are destroyed together
 * with every handle of them.
 *
 * The other way round (002): a script's API, called from C++. A script
 * exposes it the way it does for other scripts -- natives
 * (Source2ModNatives.CreateNative), a library (RegisterLibrary) and global
 * forwards (GlobalForward) -- and a C++ plugin reaches them by name:
 *
 * @code
 * ISource2ModScriptNative* give = g_pSource2ModNatives->FindScriptNative("Shop_GiveCredits");
 * int total = give->Call<int>(pController, 100, "kill bonus");   // 0 when no script serves it
 * if (!give->TryCall(&total, pController, 100, "kill bonus"))
 *     Msg("%s\n", give->GetLastError());
 *
 * g_pSource2ModNatives->HookScriptForward("Shop_OnBuy", [](const char*, Source2ModScriptArg* args, int count) -> cell_t
 * {
 *     return args[1].AsInt() > 1000 ? 3 : 0;   // Plugin_Handled
 * });
 * g_pSource2ModNatives->AddLibraryListener([](const char* name, bool added) { ... });
 * @endcode
 *
 * The objects Find* returns belong to the core and stay valid until it
 * unloads: they name the native or forward, not the script, so one found
 * before the script loads works once it does, and a call after the script
 * unloaded is refused (false / a value-initialised result) instead of
 * reaching freed code.
 */

#ifndef _INCLUDE_ISOURCE2MOD_NATIVES_H
#define _INCLUDE_ISOURCE2MOD_NATIVES_H

#pragma once
#include "ISource2ModTypes.h"

#include <sp_vm_api.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <type_traits>

#define SOURCE2MOD_NATIVES_INTERFACE "ISource2ModNatives002"

/// A handle type registered through CreateHandleType(); 0 is none.
using Source2ModHandleType = uint32_t;

/// Takes a handle's object when its last handle is closed.
using Source2ModHandleDestructor = void (*)(void* object);

class CEntityInstance;

/// What an argument of a call into a script is.
enum class Source2ModScriptArgType : int32_t
{
    Cell = 0,       ///< any, int, a Handle: cell
    Float,          ///< float: flt
    Bool,           ///< bool: cell (0 / 1)
    Entity,         ///< an entity methodmap value: entity (nullptr is NULL_ENTITY)
    String,         ///< const char[]: string
    StringBuffer,   ///< char[] the script may write (Source2ModNatives.SetString): buffer, size bytes
    Array,          ///< any[]: array, size cells; written back when copyback
    CellRef,        ///< int &: cellRef
    FloatRef,       ///< float &: floatRef
    NullString,     ///< NULL_STRING
    NullVector,     ///< NULL_VECTOR
};

/// A string a script writes into: Call(..., Source2ModScriptBuffer{ buf, sizeof(buf) }).
struct Source2ModScriptBuffer
{
    char* data;
    size_t size;
};

/// A cell array: Call(..., Source2ModScriptArray{ values, 3 }); copyback takes the script's changes.
struct Source2ModScriptArray
{
    cell_t* data;
    size_t count;
    bool copyback = false;
};

struct Source2ModScriptNullString {};
struct Source2ModScriptNullVector {};

/**
 * One argument of a call into a script, or of a script forward a C++ hook
 * receives. The pointers are the caller's: a script writing a reference, an
 * array or a string buffer writes through them.
 */
struct Source2ModScriptArg
{
    Source2ModScriptArgType type = Source2ModScriptArgType::Cell;
    union
    {
        cell_t cell;
        float flt;
        CEntityInstance* entity;
        const char* string;
        char* buffer;
        cell_t* array;
        cell_t* cellRef;
        float* floatRef;
    };
    uint32_t size = 0;      ///< StringBuffer: bytes, Array: cells
    bool copyback = false;  ///< Array

    Source2ModScriptArg() : cell(0) {}

    static Source2ModScriptArg Int(cell_t value) { Source2ModScriptArg a; a.cell = value; return a; }
    static Source2ModScriptArg Float(float value) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::Float; a.flt = value; return a; }
    static Source2ModScriptArg Bool(bool value) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::Bool; a.cell = value ? 1 : 0; return a; }
    static Source2ModScriptArg Entity(CEntityInstance* value) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::Entity; a.entity = value; return a; }
    static Source2ModScriptArg String(const char* value) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::String; a.string = value ? value : ""; return a; }
    static Source2ModScriptArg Buffer(char* data, size_t size) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::StringBuffer; a.buffer = data; a.size = static_cast<uint32_t>(size); return a; }
    static Source2ModScriptArg Array(cell_t* data, size_t count, bool copyback = false) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::Array; a.array = data; a.size = static_cast<uint32_t>(count); a.copyback = copyback; return a; }
    static Source2ModScriptArg CellRef(cell_t* value) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::CellRef; a.cellRef = value; return a; }
    static Source2ModScriptArg FloatRef(float* value) { Source2ModScriptArg a; a.type = Source2ModScriptArgType::FloatRef; a.floatRef = value; return a; }
    static Source2ModScriptArg Null(Source2ModScriptArgType nullType) { Source2ModScriptArg a; a.type = nullType; return a; }

    /// The value as a cell: a Cell / Bool / Entity forward argument (an entity
    /// as the script's cell, see EntityFromScriptCell), a reference's value.
    cell_t AsInt() const
    {
        switch (type)
        {
        case Source2ModScriptArgType::Float: return static_cast<cell_t>(flt);
        case Source2ModScriptArgType::CellRef: return cellRef ? *cellRef : 0;
        case Source2ModScriptArgType::FloatRef: return floatRef ? static_cast<cell_t>(*floatRef) : 0;
        case Source2ModScriptArgType::Cell:
        case Source2ModScriptArgType::Bool: return cell;
        default: return 0;
        }
    }

    float AsFloat() const
    {
        switch (type)
        {
        case Source2ModScriptArgType::Float: return flt;
        case Source2ModScriptArgType::FloatRef: return floatRef ? *floatRef : 0.0f;
        case Source2ModScriptArgType::CellRef: return cellRef ? static_cast<float>(*cellRef) : 0.0f;
        case Source2ModScriptArgType::Cell:
        case Source2ModScriptArgType::Bool: return static_cast<float>(cell);
        default: return 0.0f;
        }
    }

    bool AsBool() const { return AsInt() != 0; }

    /// A String or StringBuffer argument; "" for anything else.
    const char* AsString() const
    {
        if (type == Source2ModScriptArgType::String)
            return string;
        if (type == Source2ModScriptArgType::StringBuffer && buffer)
            return buffer;
        return "";
    }
};

namespace source2mod_script_detail
{
    template <typename T>
    inline constexpr bool kAlwaysFalse = false;

    template <typename T>
    Source2ModScriptArg ToArg(T&& value)
    {
        using U = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (std::is_same_v<U, Source2ModScriptArg>)
            return value;
        else if constexpr (std::is_same_v<U, Source2ModScriptBuffer>)
            return Source2ModScriptArg::Buffer(value.data, value.size);
        else if constexpr (std::is_same_v<U, Source2ModScriptArray>)
            return Source2ModScriptArg::Array(value.data, value.count, value.copyback);
        else if constexpr (std::is_same_v<U, Source2ModScriptNullString>)
            return Source2ModScriptArg::Null(Source2ModScriptArgType::NullString);
        else if constexpr (std::is_same_v<U, Source2ModScriptNullVector>)
            return Source2ModScriptArg::Null(Source2ModScriptArgType::NullVector);
        else if constexpr (std::is_same_v<U, std::nullptr_t>)
            return Source2ModScriptArg::Entity(nullptr);
        else if constexpr (std::is_same_v<U, bool>)
            return Source2ModScriptArg::Bool(value);
        else if constexpr (std::is_integral_v<U> || std::is_enum_v<U>)
            return Source2ModScriptArg::Int(static_cast<cell_t>(value));
        else if constexpr (std::is_floating_point_v<U>)
            return Source2ModScriptArg::Float(static_cast<float>(value));
        else if constexpr (std::is_same_v<U, std::string>)
            return Source2ModScriptArg::String(value.c_str());
        // char buf[N]: the script may write it; a literal or const array is read only.
        else if constexpr (std::is_array_v<U> && std::is_same_v<std::remove_extent_t<U>, char> && !std::is_const_v<std::remove_reference_t<T>>)
            return Source2ModScriptArg::Buffer(value, std::extent_v<U>);
        else if constexpr (std::is_array_v<U> && std::is_same_v<std::remove_cv_t<std::remove_extent_t<U>>, char>)
            return Source2ModScriptArg::String(value);
        else if constexpr (std::is_pointer_v<std::decay_t<U>>)
        {
            using P = std::remove_pointer_t<std::decay_t<U>>;
            if constexpr (std::is_same_v<std::remove_cv_t<P>, char>)
                return Source2ModScriptArg::String(value);
            else if constexpr (std::is_same_v<P, cell_t>)
                return Source2ModScriptArg::CellRef(value);
            else if constexpr (std::is_same_v<P, float>)
                return Source2ModScriptArg::FloatRef(value);
            else if constexpr (std::is_class_v<P>)
            {
                static_assert(std::is_base_of_v<CEntityInstance, std::remove_cv_t<P>>, "a pointer argument must be an entity (CEntityInstance or derived)");
                return Source2ModScriptArg::Entity(const_cast<CEntityInstance*>(static_cast<const CEntityInstance*>(value)));
            }
            else
                static_assert(kAlwaysFalse<T>, "this pointer type has no script equivalent");
        }
        else
            static_assert(kAlwaysFalse<T>, "this argument type has no script equivalent");
    }
}

/**
 * Something in a script C++ can call: a native a script made, a global
 * forward. Owned by the core, valid until it unloads.
 */
class ISource2ModScriptCallable
{
public:
    virtual ~ISource2ModScriptCallable() = default;

    virtual const char* GetName() const = 0;

    /// Whether a call would reach a script right now.
    virtual bool IsAvailable() const = 0;

    /**
     * @brief Calls it with arguments as they are.
     *
     * @param result    The return value (a forward's, combined by its
     *                  ExecType); may be nullptr
     * @return          false when refused (not available, a bad argument) or
     *                  the script raised an error; GetLastError() says which
     */
    virtual bool Invoke(const Source2ModScriptArg* args, int count, cell_t* result) = 0;

    /// Why the last Invoke() failed; "" after one that went through.
    virtual const char* GetLastError() const = 0;

    /// The live entity a script's entity cell names, nullptr for NULL_ENTITY or a removed one.
    virtual CEntityInstance* EntityFromCell(cell_t value) const = 0;

    /**
     * @brief A typed call. Arguments: integers / enums / a Handle (cell),
     * bool, float / double, const char* / std::string / a string literal
     * (const char[]), char buf[N] or Source2ModScriptBuffer (a string the
     * script writes), Source2ModScriptArray, cell_t* / float* (int& /
     * float&), an entity pointer (CEntityInstance or derived; nullptr is
     * NULL_ENTITY), Source2ModScriptNullString / NullVector, or a ready
     * Source2ModScriptArg. R: void, bool, an integer or enum, float, or an
     * entity pointer.
     *
     * @return The result, value-initialised when the call failed
     */
    template <typename R = void, typename... A>
    R Call(A&&... args)
    {
        cell_t result = 0;
        const Source2ModScriptArg list[sizeof...(A) + 1] = { source2mod_script_detail::ToArg(static_cast<A&&>(args))..., Source2ModScriptArg() };
        if (!Invoke(list, static_cast<int>(sizeof...(A)), &result))
            result = 0;
        return FromCell<R>(result);
    }

    /// Call() that says whether it went through; @p out may be nullptr.
    template <typename R, typename... A>
    bool TryCall(R* out, A&&... args)
    {
        cell_t result = 0;
        const Source2ModScriptArg list[sizeof...(A) + 1] = { source2mod_script_detail::ToArg(static_cast<A&&>(args))..., Source2ModScriptArg() };
        const bool ok = Invoke(list, static_cast<int>(sizeof...(A)), &result);
        if (out)
            *out = ok ? FromCell<R>(result) : R{};
        return ok;
    }

private:
    template <typename R>
    R FromCell(cell_t value) const
    {
        if constexpr (std::is_void_v<R>)
            return;
        else if constexpr (std::is_same_v<R, bool>)
            return value != 0;
        else if constexpr (std::is_integral_v<R> || std::is_enum_v<R>)
            return static_cast<R>(value);
        else if constexpr (std::is_floating_point_v<R>)
            return static_cast<R>(sp_ctof(value));
        else if constexpr (std::is_pointer_v<R> && std::is_class_v<std::remove_pointer_t<R>>)
        {
            static_assert(std::is_base_of_v<CEntityInstance, std::remove_cv_t<std::remove_pointer_t<R>>>, "a pointer result must be an entity (CEntityInstance or derived)");
            return static_cast<R>(EntityFromCell(value));
        }
        else
            static_assert(source2mod_script_detail::kAlwaysFalse<R>, "this result type has no script equivalent");
    }
};

/**
 * A native a script made with Source2ModNatives.CreateNative, by name. The
 * script reads the arguments with Source2ModNatives.GetCell / GetString /
 * GetCellRef / GetArray... as for a script caller; its `plugin` parameter is
 * INVALID_HANDLE (a C++ caller has none). Source2ModNatives.FormatString and
 * GetFunction have nothing to read from a C++ caller and fail.
 */
class ISource2ModScriptNative : public ISource2ModScriptCallable
{
public:
    /// The plugin that registered it now, 0 while none has.
    virtual PluginId GetOwner() const = 0;
};

/**
 * A global forward by name: every running script's public function of that
 * name (GlobalForward semantics) and every C++ HookScriptForward() on it, in
 * that order. The result is combined by the ExecType of the script's
 * GlobalForward of that name, ET_Event (the highest) when no script made one.
 */
class ISource2ModScriptForward : public ISource2ModScriptCallable
{
public:
    /// Script functions plus C++ hooks a call would reach.
    virtual int GetFunctionCount() const = 0;
};

/**
 * A C++ hook on a script global forward. @p args are the forward's
 * parameters; a reference, an array or a string buffer is written back to
 * the script that fired it. Returns what a script function of the forward
 * would (an Action for most).
 */
using Source2ModScriptForwardHandler = Source2ModCallback<cell_t(const char* forward, Source2ModScriptArg* args, int count)>;

/// A library came (added) or went, as OnLibraryAdded / OnLibraryRemoved.
using Source2ModLibraryHandler = Source2ModCallback<void(const char* name, bool added)>;

class ISource2ModNatives
{
public:
    virtual ~ISource2ModNatives() = default;

    /**
     * @brief Registers natives for scripts.
     *
     * Scripts already loaded get the ones they were missing; a script that
     * failed to load for want of one of them is loaded again. A name that a
     * live plugin already registered is refused (and logged), the rest of the
     * list still goes in.
     *
     * @param owner     The plugin's id (g_PluginID)
     * @param natives   Name and function pairs, ending in { nullptr, nullptr };
     *                  copied, the array need not outlive the call
     * @return          false when any of them was refused
     */
    virtual bool AddNatives(PluginId owner, const sp_nativeinfo_t* natives) = 0;

    /**
     * @brief Registers a library, what LibraryExists() and OnLibraryAdded()
     * in a script see. Scripts that are running get OnLibraryAdded now.
     *
     * Several plugins may register the same library; it is removed
     * (OnLibraryRemoved) when the last of them unloads.
     *
     * @return false on an empty name
     */
    virtual bool RegisterLibrary(PluginId owner, const char* name) = 0;

    /// @brief Whether a plugin (script or native) registered this library.
    virtual bool LibraryExists(const char* name) = 0;

    /**
     * @brief The script plugin a native was called from.
     *
     * @return Its plugin id, 0 when the context is no script's
     */
    virtual PluginId GetScriptPlugin(SourcePawn::IPluginContext* ctx) = 0;

    /**
     * @brief SourceMod's format, as Format() and PrintToServer() do it: the
     * format at params[fmtParam], its arguments after it.
     *
     * @return false after reporting the error to the script
     */
    virtual bool FormatString(SourcePawn::IPluginContext* ctx, const cell_t* params, int fmtParam,
                              char* buffer, size_t maxlen, size_t* written = nullptr) = 0;

    /**
     * @brief Registers a handle type for the objects this plugin hands out.
     *
     * @param destroy   Called with the object when its last handle is closed
     *                  (CloseHandle / delete, the owning script unloading);
     *                  may be nullptr
     */
    virtual Source2ModHandleType CreateHandleType(PluginId owner, const char* name, Source2ModHandleDestructor destroy) = 0;

    /**
     * @brief A new handle to `object`, owned by the script that `ctx` is --
     * closed when that script unloads.
     *
     * @return The handle, 0 on an unknown type or a full handle table
     */
    virtual cell_t CreateHandle(Source2ModHandleType type, void* object, SourcePawn::IPluginContext* ctx) = 0;

    /**
     * @brief The object behind a handle of this type.
     *
     * @return The object, or nullptr after reporting SourceMod's
     *         "Invalid Handle" error to the script
     */
    virtual void* ReadHandle(SourcePawn::IPluginContext* ctx, cell_t handle, Source2ModHandleType type) = 0;

    /// @brief Closes a handle whoever owns it.
    virtual bool CloseHandle(cell_t handle) = 0;

    // ---- 002: scripts' natives, forwards and libraries from C++ ----------

    /**
     * @brief A native a script made (CreateNative), by name.
     *
     * Never nullptr for a non-empty name: the object stands for the name, so
     * it can be found before the script registers the native and keeps
     * working across the script's reloads. IsAvailable() says whether a
     * script serves it now; a native a C++ plugin registered is not
     * available here (call that plugin's code directly).
     */
    virtual ISource2ModScriptNative* FindScriptNative(const char* name) = 0;

    /// @brief A global forward by name (see ISource2ModScriptForward); never nullptr for a non-empty name.
    virtual ISource2ModScriptForward* FindScriptForward(const char* name) = 0;

    /**
     * @brief Hooks a script global forward: the handler runs whenever a
     * script fires it (after the scripts' own functions), and when C++ calls
     * it through FindScriptForward(). Goes when the plugin that made the
     * handler unloads.
     *
     * @return The hook's id, 0 on an empty name or handler
     */
    virtual Source2ModHookId HookScriptForward(const char* name, Source2ModScriptForwardHandler handler) = 0;
    virtual bool UnhookScriptForward(Source2ModHookId id) = 0;

    /**
     * @brief Tells about libraries coming and going -- a script's
     * RegisterLibrary or a C++ RegisterLibrary(), what scripts get as
     * OnLibraryAdded / OnLibraryRemoved. A library goes when the last plugin
     * that registered it unloads; by then that plugin's natives no longer
     * answer (a call from the handler is refused).
     *
     * @param replay    Call the handler now for every library already there
     * @return          The listener's id, 0 on an empty handler
     */
    virtual Source2ModHookId AddLibraryListener(Source2ModLibraryHandler handler, bool replay = true) = 0;
    virtual bool RemoveLibraryListener(Source2ModHookId id) = 0;

    /// @brief The live entity a script's entity cell names (natives, forward arguments); nullptr for NULL_ENTITY or a removed one.
    virtual CEntityInstance* EntityFromScriptCell(cell_t value) = 0;

    /// @brief An entity as a script's entity methodmap value; -1 (NULL_ENTITY) for nullptr.
    virtual cell_t ScriptCellFromEntity(CEntityInstance* entity) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_NATIVES_H
