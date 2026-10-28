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
* @file ISource2ModConfigs.h
* @brief JSON config files a plugin declares once and reads already merged.
*
* A plugin names its configs and gives each one its defaults as JSON text
* (comments allowed). Source2Mod then owns the files:
*
* @code
* addons/source2mod/configs/<PluginName>/
*   <name>.example.json   the defaults, verbatim; rewritten whenever they change
*   <name>.json           the admin's file; created from the defaults when missing
* @endcode
*
* What the plugin reads is the admin's file merged over the defaults: a key
* the admin's file lacks (a setting added in a newer plugin version) comes from
* the defaults, a value of the wrong type is reported and replaced by the
* default, and everything the admin wrote stays as written. So every key the
* defaults have is always there, and the plugin needs no fallback values of
* its own.
*
* The usual shape: the plugin class derives from ISource2ModConfigListener next
* to ISource2ModPlugin / ISource2ModListener, declares its configs in
* OnDeclareConfigs() and copies the values it needs in OnConfigChanged():
*
* @code
* bool MyPlugin::Load(PluginId id, ISource2ModAPI* api, char* error, size_t maxlen, bool late)
* {
*     SOURCE2MOD_SAVEVARS();
*     if (!g_pSource2ModConfigs || !g_pSource2ModConfigs->AddListener(this, this, error, maxlen))
*         return false;
*     ...
* }
*
* void MyPlugin::OnDeclareConfigs(ISource2ModConfigDeclarations* configs)
* {
*     configs->Add("config", R"json({ "Prefix": "[MY]", "Enabled": true })json");
* }
*
* void MyPlugin::OnConfigChanged(ISource2ModConfig* config, ConfigChange reason)
* {
*     m_bEnabled = config->Root()->GetBool("Enabled");
* }
* @endcode
*
* The defaults can also be built key by key (Source2ModConfigDefaults below)
* instead of written as one JSON text.
*
* Everything here runs on the game thread.
*/

#ifndef _INCLUDE_ISOURCE2MOD_CONFIGS_H
#define _INCLUDE_ISOURCE2MOD_CONFIGS_H

#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "ISource2ModJSON.h"

class ISource2ModPlugin;
using PluginId = int;

/**
* @brief How a config's file is handled.
*/
enum : uint32_t
{
    SOURCE2MOD_CONFIG_DEFAULT       = 0,
    /// The first read fails (AddListener / Register return false) when the
    /// admin's file does not parse, instead of running on the defaults.
    SOURCE2MOD_CONFIG_REQUIRED      = 1 << 0,
    /// No reload when the file changes on disk; "s2m config reload" and
    /// ISource2ModConfig::Reload() still work.
    SOURCE2MOD_CONFIG_NO_WATCH      = 1 << 1,
    /// No <name>.example.json; <name>.json is still created from the defaults.
    SOURCE2MOD_CONFIG_NO_EXAMPLE    = 1 << 2,
    /// Settings new in the defaults are written back into <name>.json (the old
    /// file kept as <name>.json.bak). Off by default: the rewrite loses the
    /// admin's comments and layout.
    SOURCE2MOD_CONFIG_WRITE_MISSING = 1 << 3,
    /// The defaults only seed the first <name>.json and the example; the admin's
    /// file is read as it is, without the merge. For files that are lists the
    /// admin owns (groups, admins, database entries), where merging would put
    /// the example entries back.
    SOURCE2MOD_CONFIG_NO_MERGE      = 1 << 4,
};

/**
* @brief Why OnConfigChanged() was called.
*/
enum class ConfigChange
{
    /// The first read, from AddListener() / Register().
    Loaded,
    /// The file changed on disk.
    FileChanged,
    /// "s2m config reload".
    Command,
    /// ISource2ModConfig::Reload().
    Reload,
};

class ISource2ModConfig;

/**
* @brief Handed to ISource2ModConfigListener::OnDeclareConfigs(); valid only
* during that call.
*/
class ISource2ModConfigDeclarations
{
public:
    /**
    * @brief Declares one config of this plugin.
    *
    * @param pszName     File stem under configs/<PluginName>/: letters, digits,
    *                    '_', '-', '.' and '/' for a subfolder; no "..", no
    *                    extension. "config" is configs/<PluginName>/config.json.
    * @param pszDefaults The defaults as JSON text, comments allowed; the root
    *                    must be an object. Copied.
    * @param nFlags      SOURCE2MOD_CONFIG_* flags.
    */
    virtual void Add(const char* pszName, const char* pszDefaults, uint32_t nFlags = SOURCE2MOD_CONFIG_DEFAULT) = 0;
};

/**
* @brief A plugin's view of its configs. Derive the plugin class from it; every
* method is optional.
*/
class ISource2ModConfigListener
{
public:
    virtual ~ISource2ModConfigListener() = default;

    /// Which configs the plugin has. Called once, from AddListener().
    virtual void OnDeclareConfigs(ISource2ModConfigDeclarations* configs) {}

    /**
    * @brief Checks a freshly read file before it goes live.
    *
    * @param root The new document, already merged with the defaults.
    * @return False to refuse it, with the reason in error: a reload keeps the
    *         previous document, the first read fails AddListener().
    */
    virtual bool OnConfigValidate(ISource2ModConfig* config, const ISource2ModJSONValue* root, char* error, size_t maxlen)
    {
        return true;
    }

    /**
    * @brief A new document is live; config->GetName() says which one.
    *
    * Copy what you need into your own variables here: the nodes of the
    * previous document are gone after the next frame.
    */
    virtual void OnConfigChanged(ISource2ModConfig* config, ConfigChange reason) {}
};

/**
* @brief One config file. Owned by Source2Mod; valid until Unregister() or the
* owning plugin's unload.
*
* Nodes reached through Root(), Defaults() and Find() belong to the current
* generation. A reload or a Set*() makes a new one; the old nodes stay readable
* until the next frame, then are freed -- never keep them past the callback
* that read them.
*/
class ISource2ModConfig
{
public:
    /// The name it was declared with ("config", "maps/de_dust2", ...).
    virtual const char* GetName() const = 0;

    /// Absolute path of <name>.json.
    virtual const char* GetPath() const = 0;

    /// The admin's file merged over the defaults (the file alone with
    /// SOURCE2MOD_CONFIG_NO_MERGE). Never null.
    virtual const ISource2ModJSONValue* Root() const = 0;

    /// The defaults as declared. Never null.
    virtual const ISource2ModJSONValue* Defaults() const = 0;

    /// The node at a JSON pointer into Root() ("/Api/Url"); null when absent.
    virtual const ISource2ModJSONValue* Find(const char* pszPointer) const = 0;

    /// Bumped by every reload and Set*().
    virtual uint32_t GetGeneration() const = 0;

    /// What the last read reported: the parse error, wrong-typed values, ...
    /// Empty when it was clean.
    virtual const char* GetLastError() const = 0;

    /**
    * @brief Changes a value in memory; Save() writes it out.
    *
    * Objects on the way are created. False when the pointer is malformed or
    * runs through something that is not an object. OnConfigChanged() is not
    * called: the plugin made the change itself.
    */
    virtual bool SetString(const char* pszPointer, const char* pszValue) = 0;
    virtual bool SetInt64(const char* pszPointer, int64_t nValue) = 0;
    virtual bool SetDouble(const char* pszPointer, double flValue) = 0;
    virtual bool SetBool(const char* pszPointer, bool bValue) = 0;
    /// Any JSON value given as text: an object, an array, null.
    virtual bool SetJSON(const char* pszPointer, const char* pszJson) = 0;
    virtual bool Remove(const char* pszPointer) = 0;

    /**
    * @brief Writes the current document to <name>.json.
    *
    * The previous file is kept as <name>.json.bak. The admin's comments do not
    * survive the rewrite. Does not trigger a reload.
    */
    virtual bool Save(char* error, size_t maxlen) = 0;

    /// Reads the file again now; OnConfigChanged(ConfigChange::Reload) when it
    /// went live. False with the reason when it did not (the old one stays).
    virtual bool Reload(char* error, size_t maxlen) = 0;
};

#define SOURCE2MOD_CONFIGS_INTERFACE "ISource2ModConfigs001"

class ISource2ModConfigs
{
public:
    virtual ~ISource2ModConfigs() = default;

    /**
    * @brief Hooks a plugin's config listener up and reads its configs.
    *
    * Calls OnDeclareConfigs(), then for every declared config writes the
    * example, creates <name>.json when it is missing, reads and merges it, asks
    * OnConfigValidate() and calls OnConfigChanged(ConfigChange::Loaded) -- all
    * before it returns, so Load() can use the values right after.
    *
    * @param plugin   The plugin (this), as for ISource2ModAPI::AddListener().
    * @param listener Usually the plugin object itself.
    * @return False with the reason when a declaration is broken (name,
    *         defaults, a duplicate), a REQUIRED file does not parse or
    *         OnConfigValidate() refused the first read.
    */
    virtual bool AddListener(ISource2ModPlugin* plugin, ISource2ModConfigListener* listener, char* error, size_t maxlen) = 0;

    /// Optional; the unload drops it anyway. The configs stay readable.
    virtual void RemoveListener(ISource2ModPlugin* plugin) = 0;

    /**
    * @brief A config outside the declaration step, e.g. one per map.
    *
    * Same file handling as a declared one. The listener may be null; when set,
    * it gets OnConfigValidate / OnConfigChanged for this config only
    * (OnConfigChanged(Loaded) before Register returns).
    *
    * @return The config, or null with the reason (as AddListener()).
    */
    virtual ISource2ModConfig* Register(ISource2ModPlugin* plugin, const char* pszName, const char* pszDefaults, uint32_t nFlags,
                                        ISource2ModConfigListener* listener, char* error, size_t maxlen) = 0;

    /// Forgets a config (the files stay). The handle is gone afterwards.
    virtual void Unregister(ISource2ModConfig* config) = 0;

    /// A config by owner and name, this plugin's or another one's (plugin ids
    /// as OnPluginLoad reports them); null when there is none.
    virtual ISource2ModConfig* Get(PluginId owner, const char* pszName) = 0;
};

/// This plugin's config by name.
#define PLUGIN_CONFIG(name) g_pSource2ModConfigs->Get(g_PluginID, name)

/**
* @brief Config defaults built key by key instead of written as JSON text.
*
* Header-only: it only produces the text Add() / Register() take. A key is a
* plain member name ("RelayControl") or a JSON pointer into nested objects
* ("/Feature/Enabled"), whose objects are created on the way; keys keep the
* order they were added in, and adding one again replaces its value. An
* optional comment goes into the generated text above the key, so it shows in
* <name>.example.json and the first <name>.json.
*
* @code
* Source2ModConfigDefaults defaults;
* defaults.Add("RelayControl", "127.0.0.1:27100")
*         .Add("RelayTimeoutMs", 300, "milliseconds")
*         .Add("/Feature/Enabled", true)
*         .AddJSON("Admins", "[]");
* configs->Add("config", defaults.Json());
* @endcode
*/
class Source2ModConfigDefaults
{
public:
    Source2ModConfigDefaults& Add(const char* key, const char* value, const char* comment = nullptr)
    {
        return Set(key, Quote(value ? value : ""), comment);
    }

    Source2ModConfigDefaults& Add(const char* key, const std::string& value, const char* comment = nullptr)
    {
        return Set(key, Quote(value), comment);
    }

    Source2ModConfigDefaults& Add(const char* key, bool value, const char* comment = nullptr)
    {
        return Set(key, value ? "true" : "false", comment);
    }

    Source2ModConfigDefaults& Add(const char* key, int value, const char* comment = nullptr)
    {
        return Set(key, std::to_string(value), comment);
    }

    Source2ModConfigDefaults& Add(const char* key, int64_t value, const char* comment = nullptr)
    {
        return Set(key, std::to_string(value), comment);
    }

    Source2ModConfigDefaults& Add(const char* key, double value, const char* comment = nullptr)
    {
        char text[64];
        snprintf(text, sizeof(text), "%.15g", value);
        std::string number(text);
        // Written as a fraction even when whole, so it reads as the float it is.
        if (number.find_first_of(".eEn") == std::string::npos)
            number += ".0";
        return Set(key, number, comment);
    }

    Source2ModConfigDefaults& Add(const char* key, float value, const char* comment = nullptr)
    {
        return Add(key, static_cast<double>(value), comment);
    }

    /// Any JSON value as text: an array, an object, null. Invalid text makes
    /// the whole defaults invalid, which Add() / Register() report.
    Source2ModConfigDefaults& AddJSON(const char* key, const char* json, const char* comment = nullptr)
    {
        return Set(key, json && *json ? json : "null", comment);
    }

    /// The defaults as JSON text, comments included.
    const char* Json()
    {
        m_text.clear();
        Write(m_root, 0);
        m_text += "\n";
        return m_text.c_str();
    }

private:
    struct Node
    {
        std::string comment;
        bool object = true;
        std::string value;    // JSON text of a leaf
        std::vector<std::pair<std::string, Node>> members;
    };

    static std::string Quote(const std::string& in)
    {
        std::string out = "\"";
        for (unsigned char c : in)
        {
            switch (c)
            {
                case '"':  out += "\\\""; break;
                case '\\': out += "\\\\"; break;
                case '\n': out += "\\n"; break;
                case '\r': out += "\\r"; break;
                case '\t': out += "\\t"; break;
                default:
                    if (c < 0x20)
                    {
                        char esc[8];
                        snprintf(esc, sizeof(esc), "\\u%04x", c);
                        out += esc;
                    }
                    else
                    {
                        out += static_cast<char>(c);
                    }
            }
        }
        return out + "\"";
    }

    static Node& Member(Node& parent, const std::string& key)
    {
        for (auto& member : parent.members)
        {
            if (member.first == key)
                return member.second;
        }
        parent.members.emplace_back(key, Node());
        return parent.members.back().second;
    }

    Source2ModConfigDefaults& Set(const char* key, const std::string& value, const char* comment)
    {
        if (!key || !*key)
            return *this;

        // "/a/b" walks objects; "a" is a member of the root. ~1 and ~0 are
        // '/' and '~' as in a JSON pointer.
        std::vector<std::string> path;
        if (*key == '/')
        {
            std::string token;
            for (const char* p = key + 1;; p++)
            {
                if (*p == '/' || !*p)
                {
                    path.push_back(token);
                    token.clear();
                    if (!*p)
                        break;
                }
                else if (p[0] == '~' && p[1] == '1') { token += '/'; p++; }
                else if (p[0] == '~' && p[1] == '0') { token += '~'; p++; }
                else
                    token += *p;
            }
        }
        else
        {
            path.emplace_back(key);
        }

        Node* node = &m_root;
        for (size_t i = 0; i < path.size(); i++)
        {
            node = &Member(*node, path[i]);
            if (i + 1 < path.size() && !node->object)
            {
                // A leaf on the way becomes an object.
                node->object = true;
                node->value.clear();
            }
        }

        node->object = false;
        node->members.clear();
        node->value = value;
        if (comment && *comment)
            node->comment = comment;
        return *this;
    }

    void Write(const Node& node, int depth)
    {
        if (!node.object)
        {
            m_text += node.value;
            return;
        }

        if (node.members.empty())
        {
            m_text += "{}";
            return;
        }

        const std::string indent(static_cast<size_t>(depth + 1) * 4, ' ');
        m_text += "{\n";
        for (size_t i = 0; i < node.members.size(); i++)
        {
            const auto& member = node.members[i];
            if (!member.second.comment.empty())
            {
                // One "//" line per line of the comment.
                size_t start = 0;
                for (;;)
                {
                    const size_t end = member.second.comment.find('\n', start);
                    m_text += indent + "// " + member.second.comment.substr(start, end == std::string::npos ? std::string::npos : end - start) + "\n";
                    if (end == std::string::npos)
                        break;
                    start = end + 1;
                }
            }
            m_text += indent + Quote(member.first) + ": ";
            Write(member.second, depth + 1);
            m_text += (i + 1 < node.members.size()) ? ",\n" : "\n";
        }
        m_text += std::string(static_cast<size_t>(depth) * 4, ' ') + "}";
    }

    Node m_root;
    std::string m_text;
};

#endif //_INCLUDE_ISOURCE2MOD_CONFIGS_H
