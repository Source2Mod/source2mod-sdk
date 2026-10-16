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
 * @file ISource2ModMenus.h
 * @brief Interface and base implementation for menu system.
 *
 * Provides functionality for:
 *  * Defining menus and menu options
 *  * Handling per-player menu instances
 *  * Processing menu navigation and input
 *
 * A menu is an object of the plugin's: a CenterHtmlMenu filled with options,
 * or a class derived from it that overrides the handlers (DisplayTitle,
 * OnOptionSelected, OnMenuEnd). Options can be fixed or worked out per player
 * (text, disabled, visible), open a submenu (Back returns), or be plain text
 * lines. Keys: the numbered options of the page, 7 Back (previous page, else
 * the parent menu), 8 Next, 9 Exit -- so 1-6 options a page.
 *
 * @code
 * class ShopMenu : public CenterHtmlMenu
 * {
 * public:
 *     ShopMenu() : CenterHtmlMenu("Shop")
 *     {
 *         AddSubmenu("Weapons", &m_weapons);
 *         AddDynamicOption([](CCSPlayerController* p) { return "Credits: " + std::to_string(Credits(p)); }, {})
 *             .Disabled = true;
 *     }
 *     void OnMenuEnd(CCSPlayerController* player, MenuEndReason reason) override { ... }
 * private:
 *     CenterHtmlMenu m_weapons{"Weapons"};
 * };
 * @endcode
 */

#ifndef _INCLUDE_ISOURCE2MOD_MENUS_H
#define _INCLUDE_ISOURCE2MOD_MENUS_H

#pragma once
// Only the types: ISource2ModPlugin.h includes this header, and so do plugin
// headers whose menus build on IBaseMenu, so it must not include the plugin
// header back.
#include "ISource2ModTypes.h"

#include <engine2/isource2server.h>
#include <engine2/ivengineserver2.h>

// The same alias ISource2ModTypes.h declares, repeated so this header reads the
// same whichever header of the SDK pulled it in first.
using PluginId = int;

#include <string>
#include <vector>
#include <functional>
#include <stack>

/* =========================
Forward declarations
========================= */

class CCSPlayerController;
class IBaseMenu;

/* =========================
Post select behavior
========================= */

/**
 * @brief Action executed after selecting a menu option.
 */
enum class PostSelectAction
{
    /// Close menu after selection
    Close,

    /// Reset menu to first page
    Reset,

    /// Do nothing
    Nothing
};

/**
 * @brief Why a menu went away for a player (IBaseMenu::OnMenuEnd).
 */
enum class MenuEndReason
{
    /// An option closed it (CloseOnSelect, or PostSelectAction::Close)
    Selected,

    /// The player pressed Exit
    Exit,

    /// Another menu was opened over it
    Replaced,

    /// CloseActiveMenu / CloseMenu
    Closed,

    /// The player left
    Disconnected,

    /// The menu's timeout ran out
    Timeout
};

/* =========================
Menu option
========================= */

/**
 * @brief Represents a single menu option.
 *
 * Contains display text, state and callback logic.
 */
struct ChatMenuOption
{
    /// Display text
    std::string Text;

    /// Whether option is disabled
    bool Disabled{false};

    /// Callback executed on select
    std::function<void(CCSPlayerController*, ChatMenuOption&)> OnSelect;

    /// Optional dynamic disabled evaluator, asked for the player it is shown to
    std::function<bool(CCSPlayerController*)> DisabledEvaluator;

    /* =========================
    ISource2ModMenus006
    ========================= */

    /// Text worked out for the player it is shown to (replaces Text)
    std::function<std::string(CCSPlayerController*)> TextEvaluator;

    /// Whether it shows for the player; not shown (and takes no number) when false
    std::function<bool(CCSPlayerController*)> VisibleEvaluator;

    /// Opens this menu when picked; its Back button comes back
    IBaseMenu* Submenu{nullptr};

    /// A line of text: never picked, takes no number
    bool IsText{false};

    /// Not shown at all
    bool Hidden{false};

    /// Close the menu after it is picked (MenuEndReason::Selected)
    bool CloseOnSelect{false};

    /// Keep the page after it is picked, whatever the PostSelectAction
    bool StayOnPage{false};
};

/* =========================
IMenu interface
========================= */

/**
 * @brief Base interface for menu definitions.
 */
class IMenu
{
public:
    virtual ~IMenu() = default;

    /**
     * @brief Gets menu title.
     *
     * @return Reference to menu title string
     */
    virtual const std::string& Title() const = 0;

    /**
     * @brief Sets the menu title.
     *
     * @param t New title
     */
    virtual void SetTitle(std::string t) = 0;

    /**
     * @brief Gets menu options.
     *
     * @return Reference to options vector
     */
    virtual std::vector<ChatMenuOption>& Options() = 0;

    /**
     * @brief Gets menu options (const).
     *
     * @return Const reference to options vector
     */
    virtual const std::vector<ChatMenuOption>& Options() const = 0;

    /**
     * @brief Gets post-select behavior.
     *
     * @return PostSelectAction value
     */
    virtual PostSelectAction GetPostSelectAction() const = 0;

    /**
     * @brief Sets post-select behavior.
     *
     * @param a Behavior to apply after selection
     */
    virtual void SetPostSelectAction(PostSelectAction a) = 0;

    /**
     * @brief Gets exit button state.
     *
     * @return True if exit button is enabled
     */
    virtual bool GetExitButton() const = 0;

    /**
     * @brief Sets exit button state.
     *
     * @param v True to enable exit button
     */
    virtual void SetExitButton(bool v) = 0;
};

/* =========================
BaseMenu
========================= */

/**
 * @brief Base implementation of IMenu.
 */
class IBaseMenu : public IMenu
{
public:
    /**
     * @brief Constructs a menu.
     *
     * @param title Menu title
     */
    explicit IBaseMenu(std::string title)
        : title_(std::move(title))
    {
    }

    const std::string& Title() const override { return title_; }

    void SetTitle(std::string t) override { title_ = std::move(t); }

    std::vector<ChatMenuOption>& Options() override { return options_; }
    const std::vector<ChatMenuOption>& Options() const override { return options_; }

    /**
     * @brief Drops every option, for a menu that is rebuilt in place.
     */
    void ClearOptions() { options_.clear(); }

    PostSelectAction GetPostSelectAction() const override { return postSelect_; }

    /**
     * @brief Sets post-select behavior.
     *
     * @param a Behavior to apply
     */
    void SetPostSelectAction(PostSelectAction a) override { postSelect_ = a; }

    bool GetExitButton() const override { return exitButton_; }

    /**
     * @brief Sets exit button state.
     *
     * @param v True to enable exit button
     */
    void SetExitButton(bool v) override { exitButton_ = v; }

    /* =========================
    ISource2ModMenus006
    ========================= */

    /// Options a page, 1-6 (keys 7, 8, 9 are Back, Next, Exit); default 5.
    int GetItemsPerPage() const { return itemsPerPage_; }
    void SetItemsPerPage(int count) { itemsPerPage_ = count < 1 ? 1 : (count > 6 ? 6 : count); }

    /// Seconds after which the menu closes (MenuEndReason::Timeout); 0 = never.
    int GetTimeout() const { return timeout_; }
    void SetTimeout(int seconds) { timeout_ = seconds > 0 ? seconds : 0; }

    /// A title worked out per player (what the default DisplayTitle shows).
    void SetDynamicTitle(std::function<std::string(CCSPlayerController*)> title) { dynamicTitle_ = std::move(title); }

    /// What the default OnMenuEnd calls.
    void SetEndHandler(std::function<void(CCSPlayerController*, MenuEndReason)> handler) { endHandler_ = std::move(handler); }

    /**
     * @brief The title shown to a player. Override for a title of your own;
     *        the default is the dynamic title if set, else Title().
     */
    virtual std::string DisplayTitle(CCSPlayerController* player)
    {
        return dynamicTitle_ ? dynamicTitle_(player) : title_;
    }

    /**
     * @brief An option was picked (not a submenu, not text). The default runs
     *        its OnSelect. @p index is its index in Options().
     */
    virtual void OnOptionSelected(CCSPlayerController* player, ChatMenuOption& option, int index)
    {
        if (option.OnSelect)
            option.OnSelect(player, option);
    }

    /**
     * @brief The menu (opened as the root) went away for a player. The default
     *        calls the end handler. Not called when the owning plugin unloads.
     */
    virtual void OnMenuEnd(CCSPlayerController* player, MenuEndReason reason)
    {
        if (endHandler_)
            endHandler_(player, reason);
    }

    /// An option that opens @p submenu.
    ChatMenuOption& AddSubmenu(std::string display, IBaseMenu* submenu, bool disabled = false)
    {
        ChatMenuOption& opt = AddMenuOption(std::move(display), nullptr, disabled);
        opt.Submenu = submenu;
        return opt;
    }

    /// A line of text that cannot be picked and takes no number.
    ChatMenuOption& AddText(std::string text)
    {
        ChatMenuOption& opt = AddMenuOption(std::move(text), nullptr);
        opt.IsText = true;
        return opt;
    }

    /// An option whose text, and optionally disabled / visible state, is worked out per player.
    ChatMenuOption& AddDynamicOption(
        std::function<std::string(CCSPlayerController*)> text,
        std::function<void(CCSPlayerController*, ChatMenuOption&)> onSelect,
        std::function<bool(CCSPlayerController*)> disabledEvaluator = nullptr,
        std::function<bool(CCSPlayerController*)> visibleEvaluator = nullptr)
    {
        ChatMenuOption& opt = AddMenuOption(std::string(), std::move(onSelect));
        opt.TextEvaluator = std::move(text);
        opt.DisabledEvaluator = std::move(disabledEvaluator);
        opt.VisibleEvaluator = std::move(visibleEvaluator);
        return opt;
    }

    /**
     * @brief Adds a menu option.
     *
     * @param display Display text
     * @param onSelect Callback executed on selection
     * @param disabled Whether option is disabled
     *
     * @return Reference to created option
     */
    ChatMenuOption& AddMenuOption(
        std::string display,
        std::function<void(CCSPlayerController*, ChatMenuOption&)> onSelect,
        bool disabled = false)
    {
        options_.push_back(ChatMenuOption{
            .Text = std::move(display),
            .Disabled = disabled,
            .OnSelect = std::move(onSelect)
        });
        return options_.back();
    }

    /**
     * @brief Adds a menu option with cooldown support.
     *
     * @param optionText Display text
     * @param action Callback executed on selection
     * @param disabled Whether option is disabled
     * @param close Whether menu closes after select
     * @param disabledEvaluator Optional dynamic disabled evaluator
     *
     * @return Reference to created option
     */
    ChatMenuOption& AddMenuOptionWithCooldown(
        std::string optionText,
        std::function<void(CCSPlayerController*, ChatMenuOption&)> action,
        bool disabled = false,
        bool close = true,
        std::function<bool(CCSPlayerController*)> disabledEvaluator = nullptr);

    /**
     * @brief Sets global cooldown handlers.
     *
     * @param canSelect Function that checks if player can select
     * @param onSelect Function called after selection
     */
    static void SetCooldownHandlers(
        std::function<bool(CCSPlayerController*)> canSelect,
        std::function<void(CCSPlayerController*)> onSelect)
    {
        if (canSelect) s_canSelect = std::move(canSelect);
        if (onSelect) s_onSelect = std::move(onSelect);
    }

public:
    // Global, as the name and the doc above say: set them once and every menu
    // built afterwards goes through them.
    static std::function<bool(CCSPlayerController*)> s_canSelect;
    static std::function<void(CCSPlayerController*)> s_onSelect;

private:
    std::string title_;
    std::vector<ChatMenuOption> options_;

    PostSelectAction postSelect_{PostSelectAction::Reset};
    bool exitButton_{true};

    int itemsPerPage_{5};
    int timeout_{0};
    std::function<std::string(CCSPlayerController*)> dynamicTitle_;
    std::function<void(CCSPlayerController*, MenuEndReason)> endHandler_;
};

/* =========================
Menu instances
========================= */

/**
 * @brief Runtime instance of a menu bound to a player.
 */
class IMenuInstance
{
public:
    virtual ~IMenuInstance() = default;

    /**
     * @brief Displays the menu.
     */
    virtual void Display() = 0;

    /**
     * @brief Handles key press input.
     *
     * @param player Player who triggered input
     * @param key Pressed key (1-9)
     */
    virtual void OnKeyPress(CCSPlayerController* player, int key) = 0;

    /**
     * @brief Resets menu state.
     */
    virtual void Reset()
    {
        while (!prevPageOffsets_.empty()) prevPageOffsets_.pop();
        page_ = 0;
        currentOffset_ = 0;
    }

    /**
     * @brief Closes the menu.
     */
    virtual void Close() = 0;

    /* =========================
    ISource2ModMenus005
    ========================= */

    /**
     * @brief Once a frame while this is the player's open menu.
     *
     * The default redraws, which is what a center HTML menu needs (it
     * fades). A menu drawn on a HUD layout overrides it to refresh only
     * when something changed.
     */
    virtual void OnFrame() { Display(); }

    /**
     * @brief Whether the number keys (chat triggers, binds) pick options.
     *
     * A menu the player clicks instead returns false and the core drops
     * the key presses while it is open.
     */
    virtual bool AcceptsKeys() const { return true; }

    /**
     * @brief Destroys the instance.
     *
     * The core calls this, never delete, when the menu is closed, replaced
     * or the player leaves -- so an instance a plugin built (OpenMenu) is
     * freed by the library that allocated it.
     */
    virtual void Destroy() { delete this; }

    /**
     * @brief The menu is going away for this player, just before Destroy();
     *        not called when the owning plugin unloads. (ISource2ModMenus006)
     */
    virtual void OnEnd(MenuEndReason reason) {}

    /**
     * @brief The number keys the menu does something with right now, as a
     *        bit mask (bit 1 = key 1 ... bit 9 = key 9). A key outside it is
     *        left to the game: with no binds, Source2Mod reads the keys from
     *        the weapon slots, and only the keys in the mask are taken.
     *        The default takes 1-9. (ISource2ModMenus007)
     */
    virtual uint32_t KeyMask() const { return 0x3FEu; }

    /**
     * @brief Gets underlying menu.
     *
     * @return Pointer to menu
     */
    IMenu* Menu() const { return menu_; }

    /**
     * @brief Gets owning player.
     *
     * @return Pointer to player
     */
    CCSPlayerController* Player() { return player_; }

protected:
    /**
     * @brief Constructs menu instance.
     *
     * @param player Owning player
     * @param menu Menu definition
     */
    IMenuInstance(CCSPlayerController* player, IMenu* menu)
        : player_(player), menu_(menu)
    {
    }

    /**
     * @brief Number of items per page.
     *
     * @return Items per page
     */
    virtual int NumPerPage() const { return 6; }

    /**
     * @brief Menu items per page.
     *
     * @return Items per page (including navigation)
     */
    virtual int MenuItemsPerPage() const { return NumPerPage(); }

    /**
     * @brief Checks if previous page exists.
     *
     * @return True if previous page exists
     */
    bool HasPrevButton() const { return page_ > 0; }

    /**
     * @brief Checks if next page exists.
     *
     * @return True if next page exists
     */
    bool HasNextButton() const
    {
        const auto& opts = menu_->Options();
        return (int)opts.size() > NumPerPage() && (currentOffset_ + NumPerPage()) < (int)opts.size();
    }

    /**
     * @brief Checks if exit button is enabled.
     *
     * @return True if enabled
     */
    bool HasExitButton() const { return menu_->GetExitButton(); }

    /**
     * @brief Advances to next page.
     */
    void NextPage()
    {
        prevPageOffsets_.push(currentOffset_);
        currentOffset_ += MenuItemsPerPage();
        ++page_;
        Display();
    }

    /**
     * @brief Goes back to previous page.
     */
    void PrevPage()
    {
        if (page_ <= 0 || prevPageOffsets_.empty()) return;
        --page_;
        currentOffset_ = prevPageOffsets_.top();
        prevPageOffsets_.pop();
        Display();
    }

protected:
    IMenu* menu_;
    CCSPlayerController* player_;

    int page_{0};
    int currentOffset_{0};

    std::stack<int> prevPageOffsets_;
};

/* =========================
Center HTML menu
========================= */

/**
 * @brief Styled center HTML menu.
 */
class CenterHtmlMenu : public IBaseMenu
{
public:
    /**
     * @brief Constructs center HTML menu.
     *
     * @param title Menu title
     */
    explicit CenterHtmlMenu(std::string title)
        : IBaseMenu(title)
    {
        SetExitButton(true);
    }

    std::string TitleColor = "yellow";
    std::string EnabledColor = "green";
    std::string DisabledColor = "grey";
    std::string PrevPageColor = "yellow";
    std::string NextPageColor = "yellow";
    std::string CloseColor = "red";
};

/* =========================
Core Source2Mod Menus
========================= */

/**
 * @brief Interface for menu system management.
 */
#define SOURCE2MOD_MENUS_INTERFACE "ISource2ModMenus007"

class ISource2ModMenus
{
public:
    virtual ~ISource2ModMenus() = default;

    /**
     * @brief Opens a CenterHtmlMenu for a player.
     *
     * @param owner Plugin the menu belongs to
     * @param player Target player
     * @param menu Menu to open
     *
     * @note Closed for you if the owning plugin unloads while it is open. The
     *       menu object and its option handlers live inside that plugin's
     *       library, and the next key press would go looking for them there.
     */
    virtual void OpenCenterHtmlMenu(PluginId owner, CCSPlayerController* player, CenterHtmlMenu* menu) = 0;

    /**
     * @brief Gets active menu instance for player.
     *
     * @param player Target player
     *
     * @return Pointer to active menu instance or nullptr
     */
    virtual IMenuInstance* GetActiveMenu(CCSPlayerController* player) = 0;

    /**
     * @brief Closes active menu for player.
     *
     * @param player Target player
     */
    virtual void CloseActiveMenu(CCSPlayerController* player) = 0;

    /**
     * @brief Handles key press input.
     *
     * @param player Player who pressed key
     * @param key Pressed key (1-9)
     */
    virtual void OnKeyPress(CCSPlayerController* player, int key) = 0;

    /**
     * @brief Opens a menu instance another library built.
     *
     * The instance is drawn, refreshed (OnFrame) and fed the keys by the
     * core like its own, and Destroy()ed when closed, replaced, the player
     * leaves or `owner` unloads. A plugin that draws menus somewhere of its
     * own (a HUD menu, say) serves them this way.
     *
     * @param owner    Plugin the menu belongs to (whose unload closes it)
     * @param player   Target player
     * @param instance Built with new by the caller; the core owns it from here
     */
    virtual void OpenMenu(PluginId owner, CCSPlayerController* player, IMenuInstance* instance) = 0;

    /* =========================
    ISource2ModMenus006
    ========================= */

    /**
     * @brief Closes a menu for everyone who has it open, also where it is a
     *        submenu they are in (MenuEndReason::Closed). Call it before
     *        destroying a menu that may be open.
     */
    virtual void CloseMenu(IMenu* menu) = 0;

    /// Whether @p player has @p menu open, as the root or as the submenu they are in or came through.
    virtual bool IsMenuOpen(CCSPlayerController* player, IMenu* menu) = 0;

    /// The menu @p player is looking at (the submenu they are in), or nullptr.
    virtual IMenu* GetOpenMenu(CCSPlayerController* player) = 0;
};

#define OPEN_CENTER_HTML_MENU(player, menu)  g_pSource2ModMenus->OpenCenterHtmlMenu(g_PluginID, player, menu)
#define GET_ACTIVE_MENU(player)              g_pSource2ModMenus->GetActiveMenu(player)
#define CLOSE_ACTIVE_MENU(player)            g_pSource2ModMenus->CloseActiveMenu(player)
#define MENU_ON_KEY_PRESS(player, key)       g_pSource2ModMenus->OnKeyPress(player, key)

#endif //_INCLUDE_ISOURCE2MOD_MENUS_H
