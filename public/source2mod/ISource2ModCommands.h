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

* @file ISource2ModCommands.h
* @brief Command and chat handling interface for Source2Mod.
*
* Provides functionality for:
* * Registering console commands
* * Listening to console command execution
* * Handling in-game chat messages
*
* @note This system allows plugins to hook into player input via chat or console.
  */

#ifndef _INCLUDE_ISOURCE2MOD_COMMANDS_H
#define _INCLUDE_ISOURCE2MOD_COMMANDS_H

#pragma once
#include "ISource2ModPlugin.h"
#include "ISource2ModTypes.h"

#include <convar.h>
#include <engine2/isource2server.h>
#include <engine2/ivengineserver2.h>

/* =========================
Forward declarations
========================= */

/**

* @brief Callback for chat messages.
*
* @param ctx Command execution context (player, etc.)
* @param cmd Parsed command arguments
* @param post false when called before the original, true after
  */
/**
* @brief Where a command came from, in Source2Mod's own layout.
*
* The engine's CCommandContext is what the core sees; what a handler gets is
* this, so the engine type's layout is not part of the plugin API. The two
* methods handlers use are the same as on the engine type.
  */
/// Where a command was typed: what ReplyToCommand() answers into.
enum class Source2ModCommandSource : uint8_t
{
    Console,        ///< A console -- the server's or the player's own
    Chat,           ///< Chat, with a public trigger ("!kick"); the message shows
    SilentChat,     ///< Chat, with a silent trigger ("/kick"); the message is hidden
};

class Source2ModCommandContext
{
public:
    Source2ModCommandContext(CPlayerSlot slot, int target, Source2ModCommandSource source = Source2ModCommandSource::Console)
        : m_slot(slot), m_target(target), m_source(source) {}

    /// The player who issued the command; invalid for the server console.
    CPlayerSlot GetPlayerSlot() const { return m_slot; }

    /// The engine's CommandTarget_t (CT_NO_TARGET, CT_FIRST_SPLITSCREEN_CLIENT, ...).
    int GetCommandTarget() const { return m_target; }

    /// Where it was typed.
    Source2ModCommandSource GetSource() const { return m_source; }

    /// Typed in chat, with either trigger.
    bool IsFromChat() const { return m_source != Source2ModCommandSource::Console; }

private:
    CPlayerSlot m_slot;
    int m_target;
    Source2ModCommandSource m_source;
};

/**
* @brief A command's arguments, in Source2Mod's own layout; same accessors
* as the engine's CCommand. The strings belong to the dispatch and are gone
* when the handler returns -- copy what has to outlive it.
  */
class Source2ModCommandArgs
{
public:
    static constexpr int kMaxArgs = 64;

    Source2ModCommandArgs() = default;

    Source2ModCommandArgs(int argc, const char* const* argv, const char* argS, const char* command)
        : m_argc(argc < kMaxArgs ? argc : kMaxArgs), m_argS(argS ? argS : ""), m_command(command ? command : "")
    {
        for (int i = 0; i < m_argc; i++)
            m_argv[i] = argv[i] ? argv[i] : "";
    }

    /// Number of arguments, the command name included.
    int ArgC() const { return m_argc; }

    /// Argument i, the command name at 0; "" past the end.
    const char* Arg(int i) const { return (i >= 0 && i < m_argc) ? m_argv[i] : ""; }

    const char* operator[](int i) const { return Arg(i); }

    /// Everything after the command name, as typed.
    const char* ArgS() const { return m_argS; }

    /// The whole command line, as typed.
    const char* GetCommandString() const { return m_command; }

private:
    int m_argc = 0;
    const char* m_argv[kMaxArgs] = {};
    const char* m_argS = "";
    const char* m_command = "";
};

/// A chat command or console command handler: a function, an object and a
/// method, or a lambda (Source2ModCallback).
using ChatHandler = Source2ModCallback<void(const Source2ModCommandContext&, const Source2ModCommandArgs&, bool post)>;

/**

* @brief Callback for console commands.
*
* @param ctx Command execution context
* @param cmd Parsed command arguments
* @param post false when called before the original, true after
* @return Action describing how to handle execution (Action::Ignore, Action::Override, Action::Supersede)
  */
using CommandHandler = Source2ModCallback<Action(const Source2ModCommandContext&, const Source2ModCommandArgs&, bool post)>;

/* =========================
Core Source2Mod Commands
========================= */

/**

* @brief Interface for registering and handling commands.
*
* Supports:
* * Chat-based commands (player messages)
* * Console commands (server or client)
* * Command listeners with pre/post execution hooks
*
* @note Every registration belongs to the plugin whose handler it is.
  */
#define SOURCE2MOD_COMMANDS_INTERFACE "ISource2ModCommands003"

class ISource2ModCommands
{
public:
    virtual ~ISource2ModCommands() = default;

    /**
     * @brief Registers a chat command.
     *
     * Only fires for a chat message that starts with one of the configured
     * chat triggers (public or silent, "!" and "/" by default), e.g. "!kick"
     * or "/kick"; plain "kick" typed into chat does not fire it. The trigger
     * is stripped before matching, so args.Arg(0) is the bare name. A silent
     * trigger hides the message; a public one lets it show.
     *
     * Whose it is, the core reads off the handler (see Source2ModCallback); what
     * a plugin still holds at unload the core drops for it.
     *
     * @param pchName Command name without a trigger (e.g. "kick")
     * @param handler A function, an object and a method, or a lambda
     * @param pchPermission What a player needs to run it (see
     *        ISource2ModPermissions); nullptr or "" for everybody. A player
     *        without it gets the access-denied answer instead.
     * @return The id UnregisterChatListener(id) takes
     */
    virtual Source2ModHookId RegisterChatListener(const char* pchName, ChatHandler handler, const char* pchPermission = nullptr) = 0;

    /**
     * @brief Drops the chat command with this handler -- a function or an
     * object and a method; a lambda goes by its id.
     *
     * @return true when one was found
     */
    virtual bool UnregisterChatListener(const char* pchName, const ChatHandler& handler) = 0;

    /**
     * @brief Drops the chat command RegisterChatListener() returned the id for.
     *
     * @return true when it was still there
     */
    virtual bool UnregisterChatListener(Source2ModHookId id) = 0;

    /**
     * @brief Registers a console command, reachable from chat as "!name" and
     * "/name" too. Several handlers may share a name; each runs.
     *
     * Whose it is, the core reads off the handler (see Source2ModCallback); what
     * a plugin still holds at unload the core drops for it.
     *
     * @param pchName Command name (e.g. "sv_test")
     * @param handler A function, an object and a method, or a lambda
     * @param pchPermission What a player needs to run it (see
     *        ISource2ModPermissions); nullptr or "" for everybody. The server
     *        console always may. The owner can re-assign it under
     *        "Overrides" in permissions.json.
     * @return The id UnregisterConCommand(id) takes
     */
    virtual Source2ModHookId RegisterConCommand(const char* pchName, ChatHandler handler, const char* pchPermission = nullptr) = 0;

    /**
     * @brief Drops the console command handler -- a function or an object and
     * a method; a lambda goes by its id. The command name stays claimed until
     * the plugin unloads.
     *
     * @return true when one was found
     */
    virtual bool UnregisterConCommand(const char* pchName, const ChatHandler& handler) = 0;

    /**
     * @brief Drops the console command handler RegisterConCommand() returned the id for.
     *
     * @return true when it was still there
     */
    virtual bool UnregisterConCommand(Source2ModHookId id) = 0;

    /**
     * @brief Registers a listener on an existing console command.
     *
     * The handler's Action:
     * * Ignore: do nothing
     * * Override: the original still runs; commands return nothing, so this
     *   has no further effect. Later pre listeners and all post listeners
     *   still fire.
     * * Supersede: block the original (pre only); the remaining pre listeners
     *   and all post listeners are skipped
     *
     * Whose it is, the core reads off the handler (see Source2ModCallback); what
     * a plugin still holds at unload the core drops for it.
     *
     * @param pchName Existing command name to listen for
     * @param handler A function, an object and a method, or a lambda
     * @param post false to run before the original, true to run after
     * @return The id UnregisterConListener(id) takes
     */
    virtual Source2ModHookId RegisterConListener(const char* pchName, CommandHandler handler, bool post) = 0;

    /**
     * @brief Drops the console listener with this handler -- a function or an
     * object and a method; a lambda goes by its id.
     *
     * @return true when one was found
     */
    virtual bool UnregisterConListener(const char* pchName, const CommandHandler& handler, bool post) = 0;

    /**
     * @brief Drops the console listener RegisterConListener() returned the id for.
     *
     * @return true when it was still there
     */
    virtual bool UnregisterConListener(Source2ModHookId id) = 0;

    /**
     * @brief Answers a command where it was typed: to the player's chat when
     * it came from chat, to the player's console when it came from their
     * console, to the server console when it came from there. One line; the
     * newline is added.
     */
    virtual void ReplyToCommand(const Source2ModCommandContext& ctx, const char* pszMessage) = 0;
};

/// RegisterConCommand(name, handler) or RegisterConCommand(name, handler, permission).
#define REGISTER_CHAT_LISTENER(pchName, ...) g_pSource2ModCommands->RegisterChatListener(pchName, __VA_ARGS__)
#define REGISTER_CON_COMMAND(pchName, ...)   g_pSource2ModCommands->RegisterConCommand(pchName, __VA_ARGS__)
#define REGISTER_CON_LISTENER(pchName, handler, post) g_pSource2ModCommands->RegisterConListener(pchName, handler, post)

/// ReplyToCommand() with printf formatting.
#define REPLY_TO_COMMAND(ctx, fmt, ...) \
    do { \
        char _source2modReply[512]; \
        snprintf(_source2modReply, sizeof(_source2modReply), fmt, ##__VA_ARGS__); \
        g_pSource2ModCommands->ReplyToCommand(ctx, _source2modReply); \
    } while (0)

#endif //_INCLUDE_ISOURCE2MOD_COMMANDS_H
