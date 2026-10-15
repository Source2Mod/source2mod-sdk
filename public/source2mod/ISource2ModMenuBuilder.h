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

/**
 * @file ISource2ModMenuBuilder.h
 * @brief Menus built item by item: pages, submenus with a way back, items
 *        whose text or state is worked out per player when shown.
 *
 * Shown as the center HTML menu and picked with the slot keys (1-9), like
 * ISource2ModMenus' CenterHtmlMenu, which it builds on. A menu can be shown
 * to many players at once; each has their own page and submenu path.
 *
 * @code
 * ISource2ModMenu* weapons = g_pSource2ModMenuBuilder->CreateMenu(g_PluginID, "Weapons")
 *     ->AddItem("AK-47", [](CCSPlayerController* p, ISource2ModMenu*, int) { Give(p, "weapon_ak47"); })
 *     ->AddItem("M4A4", [](CCSPlayerController* p, ISource2ModMenu*, int) { Give(p, "weapon_m4a1"); });
 *
 * ISource2ModMenu* main = g_pSource2ModMenuBuilder->CreateMenu(g_PluginID, "Shop")
 *     ->AddSubmenu("Weapons", weapons)
 *     ->AddDynamicItem([](CCSPlayerController* p) { return "Credits: " + std::to_string(Credits(p)); },
 *                      {}, SOURCE2MOD_MENUITEM_DISABLED);
 * main->Display(player);
 * @endcode
 */

#ifndef _INCLUDE_ISOURCE2MOD_MENUBUILDER_H
#define _INCLUDE_ISOURCE2MOD_MENUBUILDER_H

#pragma once
#include "ISource2ModPlugin.h"

#include <string>

class CCSPlayerController;
class ISource2ModMenu;

/* -------- item flags -------- */

#define SOURCE2MOD_MENUITEM_DISABLED (1 << 0)   ///< shown greyed out, cannot be picked
#define SOURCE2MOD_MENUITEM_HIDDEN   (1 << 1)   ///< not shown
#define SOURCE2MOD_MENUITEM_CLOSE    (1 << 2)   ///< close the menu after it is picked
#define SOURCE2MOD_MENUITEM_STAY     (1 << 3)   ///< keep the page after it is picked (else the menu's PostSelect)

/// Why a menu went away for a player.
enum ESource2ModMenuEnd
{
    SOURCE2MOD_MENUEND_SELECTED = 0,   ///< an item closed it
    SOURCE2MOD_MENUEND_EXIT,           ///< the player pressed Exit
    SOURCE2MOD_MENUEND_REPLACED,       ///< another menu was opened over it
    SOURCE2MOD_MENUEND_CLOSED,         ///< Close / CloseActiveMenu / the menu was released
    SOURCE2MOD_MENUEND_DISCONNECTED,
    SOURCE2MOD_MENUEND_TIMEOUT,
};

/// An item was picked: the menu it is in and its index there.
using Source2ModMenuSelect = Source2ModCallback<void(CCSPlayerController* player, ISource2ModMenu* menu, int item)>;
/// Text worked out when the menu is shown to a player.
using Source2ModMenuText = Source2ModCallback<std::string(CCSPlayerController* player)>;
/// A state worked out when the menu is shown to a player (enabled, visible).
using Source2ModMenuCheck = Source2ModCallback<bool(CCSPlayerController* player)>;
/// The menu went away for a player.
using Source2ModMenuEnd = Source2ModCallback<void(CCSPlayerController* player, ISource2ModMenu* menu, ESource2ModMenuEnd reason)>;

/**
 * @brief A menu. The setters return the menu so calls chain.
 */
class ISource2ModMenu
{
public:
    virtual ISource2ModMenu* SetTitle(const char* title) = 0;
    /// A title worked out per player (replaces the fixed one).
    virtual ISource2ModMenu* SetDynamicTitle(Source2ModMenuText title) = 0;

    /// An item; an empty @p onSelect ({}) for one that only shows text.
    virtual ISource2ModMenu* AddItem(const char* text, Source2ModMenuSelect onSelect, int flags = 0) = 0;
    /**
     * @brief An item whose text, and optionally whether it can be picked and
     *        whether it shows, are worked out each time it is shown.
     */
    virtual ISource2ModMenu* AddDynamicItem(Source2ModMenuText text, Source2ModMenuSelect onSelect, int flags = 0,
                                            Source2ModMenuCheck enabled = {}, Source2ModMenuCheck visible = {}) = 0;
    /// An item that opens @p submenu; its Back button comes back here.
    virtual ISource2ModMenu* AddSubmenu(const char* text, ISource2ModMenu* submenu, int flags = 0) = 0;
    /// A line of text that cannot be picked and takes no number.
    virtual ISource2ModMenu* AddText(const char* text) = 0;

    virtual int GetItemCount() = 0;
    virtual void SetItemText(int item, const char* text) = 0;
    /// SOURCE2MOD_MENUITEM_* flags of an item.
    virtual void SetItemFlags(int item, int flags) = 0;
    virtual int GetItemFlags(int item) = 0;
    virtual void RemoveItem(int item) = 0;
    virtual void ClearItems() = 0;

    /// Items per page (1-7, default 5).
    virtual ISource2ModMenu* SetItemsPerPage(int count) = 0;
    virtual ISource2ModMenu* SetExitButton(bool exit) = 0;
    /// What happens after an item without CLOSE/STAY is picked (default Reset: page 1).
    virtual ISource2ModMenu* SetPostSelect(PostSelectAction action) = 0;
    /// Closes the menu after this many seconds (0 = never).
    virtual ISource2ModMenu* SetTimeout(int seconds) = 0;
    /// HTML colors: title, enabled items, disabled items, navigation.
    virtual ISource2ModMenu* SetColors(const char* title, const char* enabled, const char* disabled, const char* navigation) = 0;
    virtual ISource2ModMenu* OnEnd(Source2ModMenuEnd onEnd) = 0;

    /// Shows the menu to a player (page 1), replacing what they have open.
    virtual void Display(CCSPlayerController* player) = 0;
    /// Closes it for one player, or for everyone with a null @p player.
    virtual void Close(CCSPlayerController* player = nullptr) = 0;
    /// Whether @p player has this menu (or one of its submenus) open.
    virtual bool IsOpenFor(CCSPlayerController* player) = 0;

    /// Something the plugin wants to keep with the menu.
    virtual void SetData(void* data) = 0;
    virtual void* GetData() = 0;

    /// Closes it for everyone and frees it (its submenus are not freed).
    virtual void Release() = 0;

protected:
    ~ISource2ModMenu() = default;
};

/* =========================
Core Source2Mod Menu Builder
========================= */

#define SOURCE2MOD_MENUBUILDER_INTERFACE "ISource2ModMenuBuilder001"

class ISource2ModMenuBuilder
{
public:
    virtual ~ISource2ModMenuBuilder() = default;

    /// A new empty menu; released with its plugin if not before.
    virtual ISource2ModMenu* CreateMenu(PluginId owner, const char* title) = 0;

    /// The builder menu @p player has open (the submenu they are in), or nullptr.
    virtual ISource2ModMenu* GetOpenMenu(CCSPlayerController* player) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_MENUBUILDER_H
