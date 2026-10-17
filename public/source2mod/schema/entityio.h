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

#ifndef ENTITYIO_H
#define ENTITYIO_H
#ifdef _WIN32
#pragma once
#endif

/**

* @file entityio.h
* @brief Entity I/O system (inputs/outputs) for Source2Mod.
*
* Provides:
* * Access to entity outputs (OnTrigger, OnUse, etc.)
* * Hooking system for listening to outputs
* * Filtering by entity + classname + output name
*
* @note Equivalent to Hammer I/O system (FireOutput / AcceptInput).
*/




#ifndef NULL
#define NULL 0
#endif

#include "entityhandle.h"
#include "entityinstance.h"
#include "entitysystem.h"
#include "entity2/ientity2_entityio.h"
#include "entity2/entitycomponent.h"
#include "string_t.h"
#include "source2mod/ISource2ModPlugin.h"

/* =========================
Connections and outputs
========================= */

// EntityIOConnectionDesc_t, EntityIOConnection_t, CEntityIOOutput (whose m_pDesc
// is an EntOutput_t) and CEntityOutputTemplate<T> are s2sdk's:
// entity2/ientity2_entityio.h and entity2/entitycomponent.h.

/* =========================
Input system
========================= */

/**

* @brief Data passed to inputs.
  */
class InputData_t
{
public:
    CBaseEntity* pActivator; ///< Entity that triggered the input
    CBaseEntity* pCaller; ///< Entity owning the output
    variant_t value; ///< Passed value
    int nOutputID;
};

#endif // ENTITYIO_H
