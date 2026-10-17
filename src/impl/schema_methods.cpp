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
 * As a special exception, Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
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
 *   - Michal "Slynx (˙·٠● S l y n x ●٠·˙)" Přikryl
 *
 * Project: Source2Mod
 */

// The core's ISource2ModSchemaMethods: the generated table
// (schema_methods.gen.cpp), searched by key. Core only (src/impl).

#include "source2mod/ISource2ModSchemaMethods.h"

#include <cstring>
#include <string_view>
#include <unordered_map>

extern const Source2ModSchemaMethodEntry g_Source2ModSchemaMethodTable[];
extern const size_t g_Source2ModSchemaMethodCount;

namespace
{
    class SchemaMethods final : public ISource2ModSchemaMethods
    {
    public:
        void* Find(const char* key) override
        {
            if (!key)
                return nullptr;
            if (m_byKey.empty())
            {
                for (size_t i = 0; i < g_Source2ModSchemaMethodCount; i++)
                    m_byKey.emplace(g_Source2ModSchemaMethodTable[i].key, g_Source2ModSchemaMethodTable[i].function);
            }
            auto it = m_byKey.find(key);
            return it == m_byKey.end() ? nullptr : it->second;
        }

        int GetCount() override { return static_cast<int>(g_Source2ModSchemaMethodCount); }

        const char* GetKey(int index) override
        {
            return index >= 0 && static_cast<size_t>(index) < g_Source2ModSchemaMethodCount ? g_Source2ModSchemaMethodTable[index].key : nullptr;
        }

    private:
        std::unordered_map<std::string_view, void*> m_byKey;
    };
}

ISource2ModSchemaMethods* Source2ModSchemaMethodsImpl()
{
    static SchemaMethods s_methods;
    return &s_methods;
}
