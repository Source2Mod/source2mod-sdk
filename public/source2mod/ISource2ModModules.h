#pragma once

#include "ISource2ModModule.h"

/**
 * @file ISource2ModModules.h
 * @brief The game's modules, owned by the core: every lookup of one library
 * gives the same object, nobody allocates or frees one.
 *
 * @code
 * ISource2ModModule* server = g_pSource2ModModules->Get(Source2ModModuleId::Server);
 * ISource2ModModule* foo = g_pSource2ModModules->Get("foo");
 * ISource2ModModule* own = g_pSource2ModModules->GetFromAddress(g_pSource2Server);
 * @endcode
 */

/// The modules everybody needs. HookTarget::Class looks for a class in these, in this order.
enum class Source2ModModuleId : int
{
    Server = 0,
    Engine,
    Tier0,
    VScript,
    SchemaSystem,
    NetworkSystem,
    ResourceSystem,
    FileSystem,
    SteamApi,
    Count,
};

#define SOURCE2MOD_MODULES_INTERFACE "ISource2ModModules001"

class ISource2ModModules
{
public:
    virtual ~ISource2ModModules() = default;

    /// One of the common modules; nullptr while it is not loaded.
    virtual ISource2ModModule* Get(Source2ModModuleId id) = 0;

    /// A module by name, without the extension ("server", "engine2"); nullptr
    /// while it is not loaded.
    virtual ISource2ModModule* Get(const char* name) = 0;

    /// The module an address lies in; nullptr when it lies in none.
    virtual ISource2ModModule* GetFromAddress(const void* address) = 0;
};
