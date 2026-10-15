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
 * @file ISource2ModTranslations.h
 * @brief Translated text: phrases per plugin, each player's language.
 *
 * A phrase is a name ("Welcome") with a text per language and, optionally,
 * a format that says what its {1}, {2}, ... placeholders are ("{1:s},{2:d}").
 * Phrases come from wherever the plugin has them: a phrase file
 * (translations/<file>.txt, SourceMod's layout), JSON, or one call per text at
 * runtime -- a plugin that downloads its texts (HTTP, SQL) pushes them when
 * they arrive and can change or remove them later. Files are only one source.
 *
 * Lookups never fail: a phrase without the asked language falls back to the
 * server's language, then English, then the phrase name itself.
 *
 * @code
 * auto* tr = g_pSource2ModTranslations;
 * tr->AddPhrase(g_PluginID, "Welcome", "en", "Welcome, {1}!", "{1:s}");
 * tr->AddPhrase(g_PluginID, "Welcome", "cze", "Vítej, {1}!");
 *
 * const char* args[] = { player->GetPlayerName() };
 * char text[256];
 * tr->FormatPhrase(g_PluginID, "Welcome", player->GetPlayerSlot(), args, 1, text, sizeof(text));
 * @endcode
 */

#ifndef _INCLUDE_ISOURCE2MOD_TRANSLATIONS_H
#define _INCLUDE_ISOURCE2MOD_TRANSLATIONS_H

#pragma once
#include "ISource2ModPlugin.h"

#include <cstddef>

/// The server, as a translation target (FormatPhrase / Translate).
#define SOURCE2MOD_LANG_SERVER CPlayerSlot(-1)

/* =========================
Core Source2Mod Translations
========================= */

#define SOURCE2MOD_TRANSLATIONS_INTERFACE "ISource2ModTranslations001"

class ISource2ModTranslations
{
public:
    virtual ~ISource2ModTranslations() = default;

    /* -------- languages -------- */

    /// Languages known: configs/languages.cfg plus those added at runtime.
    virtual int GetLanguageCount() = 0;

    /**
     * @brief A language's code ("en", "cze") and name ("English").
     * @return false for an index out of range.
     */
    virtual bool GetLanguageInfo(int language, const char** code, const char** name) = 0;

    /// A language by its code or its name (either, case-insensitive); -1 when unknown.
    virtual int FindLanguage(const char* codeOrName) = 0;

    /**
     * @brief Adds a language at runtime (or finds it when the code is known).
     * @return Its index.
     */
    virtual int AddLanguage(const char* code, const char* name) = 0;

    /// The server's language (ServerLang in core.json).
    virtual int GetServerLanguage() = 0;

    /**
     * @brief A player's language: one set with SetPlayerLanguage, else the
     *        client's own (its cl_language), else the server's.
     */
    virtual int GetPlayerLanguage(CPlayerSlot slot) = 0;

    /// Overrides a player's language until they disconnect; -1 goes back to theirs.
    virtual void SetPlayerLanguage(CPlayerSlot slot, int language) = 0;

    /* -------- phrases (each plugin has its own) -------- */

    /**
     * @brief Loads translations/<file>.txt and translations/<code>/<file>.txt
     *        for every language (SourceMod's phrase files) into @p owner's phrases.
     * @return false (and why in @p error) when the base file is missing or broken.
     */
    virtual bool LoadPhraseFile(PluginId owner, const char* file, char* error, size_t maxlen) = 0;

    /**
     * @brief Loads phrases from JSON: {"<phrase>": {"#format": "{1:s}",
     *        "<code>": "<text>", ...}, ...}. Unknown language codes are added.
     * @return false (and why in @p error) for text that is not such an object.
     */
    virtual bool LoadPhrasesJson(PluginId owner, const char* json, char* error, size_t maxlen) = 0;

    /**
     * @brief Adds or replaces one text of a phrase.
     *
     * @param language A language code; one not known yet is added.
     * @param format   The phrase's placeholder format ("{1:s},{2:d}"); nullptr
     *                 keeps the one it has.
     */
    virtual void AddPhrase(PluginId owner, const char* phrase, const char* language, const char* text, const char* format = nullptr) = 0;

    /**
     * @brief Removes a phrase's text in one language, or the whole phrase for
     *        a null / empty @p language.
     * @return false when there was nothing to remove.
     */
    virtual bool RemovePhrase(PluginId owner, const char* phrase, const char* language = nullptr) = 0;

    /// Drops every phrase of @p owner (a plugin's go when it unloads anyway).
    virtual void ClearPhrases(PluginId owner) = 0;

    virtual bool PhraseExists(PluginId owner, const char* phrase) = 0;
    virtual bool IsTranslatedFor(PluginId owner, const char* phrase, int language) = 0;

    /* -------- text -------- */

    /**
     * @brief The raw text of a phrase for a target ({n} placeholders left in).
     *
     * @param target A player, or SOURCE2MOD_LANG_SERVER.
     * @return Bytes written; the fallbacks (server language, English, the
     *         phrase name) mean there always is a text.
     */
    virtual size_t Translate(PluginId owner, const char* phrase, CPlayerSlot target, char* buffer, size_t maxlen) = 0;

    /**
     * @brief The text for a target with its {1}..{n} replaced by @p args.
     *        The values are text already; the phrase's format is not applied.
     * @return Bytes written.
     */
    virtual size_t FormatPhrase(PluginId owner, const char* phrase, CPlayerSlot target,
                                const char* const* args, int numArgs, char* buffer, size_t maxlen) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_TRANSLATIONS_H
