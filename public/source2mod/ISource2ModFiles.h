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
 * @file ISource2ModFiles.h
 * @brief Files, directories, KeyValues (Valve's KV1 text) and a streaming
 *        KeyValues reader.
 *
 * Paths that are not absolute are under the game folder (csgo), as in
 * SourceMod; ResolvePath says where one lands. A plugin that only needs to
 * read and write its own files can as well use the C++ library -- these exist
 * so plugins and SourcePawn scripts share one notion of where things are and
 * one KeyValues.
 *
 * KeyValues3 (the format CS2's own files use) is not here: the engine's KV3
 * (tier1/keyvalues3.h in the SDK) reads and writes it.
 */

#ifndef _INCLUDE_ISOURCE2MOD_FILES_H
#define _INCLUDE_ISOURCE2MOD_FILES_H

#pragma once
#include "ISource2ModPlugin.h"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

/* =========================
KeyValues
========================= */

/// What a key holds (SourceMod's KvDataTypes numbering).
enum ESource2ModKvType
{
    SOURCE2MOD_KV_NONE = 0,     ///< a section: subkeys, no value
    SOURCE2MOD_KV_STRING = 1,
    SOURCE2MOD_KV_INT = 2,
    SOURCE2MOD_KV_FLOAT = 3,
    SOURCE2MOD_KV_COLOR = 6,
    SOURCE2MOD_KV_UINT64 = 7,
};

/**
 * @brief One key of a KeyValues tree: a section with subkeys, or a value.
 *
 * Key arguments may be paths ("a/b/c"); names compare case-insensitively. A
 * null or empty key means this key itself. Values convert on read (a string
 * "5" reads as 5, an int as "5"). Pointers to keys stay valid until that key
 * or one above it is removed, or the root is released.
 */
class ISource2ModKeyValues
{
public:
    virtual const char* GetName() = 0;
    virtual void SetName(const char* name) = 0;

    /// A key under this one; nullptr when it is not there and @p create is false.
    virtual ISource2ModKeyValues* FindKey(const char* key, bool create = false) = 0;

    /// The first / next key of any kind.
    virtual ISource2ModKeyValues* GetFirstSubKey() = 0;
    virtual ISource2ModKeyValues* GetNextKey() = 0;
    /// The first / next key that is a section.
    virtual ISource2ModKeyValues* GetFirstTrueSubKey() = 0;
    virtual ISource2ModKeyValues* GetNextTrueSubKey() = 0;
    /// The first / next key that is a value.
    virtual ISource2ModKeyValues* GetFirstValue() = 0;
    virtual ISource2ModKeyValues* GetNextValue() = 0;

    virtual ESource2ModKvType GetDataType(const char* key = nullptr) = 0;

    /// The value as text; valid until the key changes. @p def when there is none.
    virtual const char* GetString(const char* key = nullptr, const char* def = "") = 0;
    virtual int GetInt(const char* key = nullptr, int def = 0) = 0;
    virtual float GetFloat(const char* key = nullptr, float def = 0.0f) = 0;
    virtual uint64_t GetUint64(const char* key = nullptr, uint64_t def = 0) = 0;
    /// r | g << 8 | b << 16 | a << 24 (a "r g b a" string reads too).
    virtual uint32_t GetColor(const char* key = nullptr, uint32_t def = 0) = 0;

    virtual void SetString(const char* key, const char* value) = 0;
    virtual void SetInt(const char* key, int value) = 0;
    virtual void SetFloat(const char* key, float value) = 0;
    virtual void SetUint64(const char* key, uint64_t value) = 0;
    virtual void SetColor(const char* key, uint32_t rgba) = 0;

    /// Removes a key under this one. @return false when it was not there.
    virtual bool RemoveKey(const char* key) = 0;

    /// Copies @p from's subkeys into this one; Merge replaces keys of the same name.
    virtual void CopySubkeys(ISource2ModKeyValues* from) = 0;
    virtual void Merge(ISource2ModKeyValues* from) = 0;

    /// A detached copy of this key and everything under it; Release() it.
    virtual ISource2ModKeyValues* Clone() = 0;

    /**
     * @brief Replaces this key with the first section of a file / text
     *        (its name and subkeys). @return false on a missing file or bad text.
     */
    virtual bool LoadFromFile(const char* path, bool escapes = false) = 0;
    virtual bool LoadFromString(const char* text, bool escapes = false) = 0;

    /// Writes this key and everything under it in Valve's layout.
    virtual bool SaveToFile(const char* path, bool escapes = false) = 0;
    /// @return The text's length; at most @p maxlen - 1 bytes are written.
    virtual size_t SaveToString(char* buffer, size_t maxlen, bool escapes = false) = 0;

    /// Frees a tree from CreateKeyValues or Clone (only a root).
    virtual void Release() = 0;

protected:
    ~ISource2ModKeyValues() = default;
};

/* =========================
Streaming reader (SourceMod's SMC)
========================= */

enum ESource2ModKvParseResult
{
    SOURCE2MOD_KVPARSE_CONTINUE = 0,
    SOURCE2MOD_KVPARSE_HALT,        ///< stop, not an error
    SOURCE2MOD_KVPARSE_HALT_FAIL,   ///< stop with SOURCE2MOD_KVERROR_CUSTOM
};

/// SourceMod's SMCError numbering.
enum ESource2ModKvError
{
    SOURCE2MOD_KVERROR_OK = 0,
    SOURCE2MOD_KVERROR_STREAM_OPEN,
    SOURCE2MOD_KVERROR_STREAM_ERROR,
    SOURCE2MOD_KVERROR_CUSTOM,
    SOURCE2MOD_KVERROR_INVALID_SECTION1,
    SOURCE2MOD_KVERROR_INVALID_SECTION2,   ///< { without a name
    SOURCE2MOD_KVERROR_INVALID_SECTION3,   ///< a lone token before }
    SOURCE2MOD_KVERROR_INVALID_SECTION4,   ///< } without a section
    SOURCE2MOD_KVERROR_INVALID_SECTION5,   ///< a section not closed
    SOURCE2MOD_KVERROR_INVALID_TOKENS,     ///< an unterminated string, a lone token
    SOURCE2MOD_KVERROR_TOKEN_OVERFLOW,
    SOURCE2MOD_KVERROR_INVALID_PROPERTY1,  ///< a key/value outside any section
};

/**
 * @brief Gets the reader's events in order; return HALT to stop early.
 */
class ISource2ModKvListener
{
public:
    virtual ~ISource2ModKvListener() = default;
    virtual void OnStart() {}
    virtual ESource2ModKvParseResult OnEnterSection(const char* name, bool quoted) { return SOURCE2MOD_KVPARSE_CONTINUE; }
    virtual ESource2ModKvParseResult OnKeyValue(const char* key, const char* value, bool keyQuoted, bool valueQuoted) { return SOURCE2MOD_KVPARSE_CONTINUE; }
    virtual ESource2ModKvParseResult OnLeaveSection() { return SOURCE2MOD_KVPARSE_CONTINUE; }
    virtual ESource2ModKvParseResult OnRawLine(const char* line, int lineNumber) { return SOURCE2MOD_KVPARSE_CONTINUE; }
    virtual void OnEnd(bool halted, bool failed) {}
};

/* =========================
Core Source2Mod Files
========================= */

/// Called per directory entry; return false to stop.
using Source2ModDirCallback = std::function<bool(const char* name, bool isDirectory)>;

#define SOURCE2MOD_FILES_INTERFACE "ISource2ModFiles001"

class ISource2ModFiles
{
public:
    virtual ~ISource2ModFiles() = default;

    /// A path as the other calls use it: absolute as given, else under the game folder.
    virtual std::string ResolvePath(const char* path) = 0;

    virtual bool FileExists(const char* path) = 0;
    virtual bool DirExists(const char* path) = 0;
    /// -1 when there is no such file.
    virtual int64_t FileSize(const char* path) = 0;
    /// Seconds since 1970 of the last change; -1 when there is no such file.
    virtual int64_t FileTime(const char* path) = 0;

    /// The whole file. @return false when it cannot be read.
    virtual bool ReadFile(const char* path, std::string& contents) = 0;
    /// Creates or replaces (or appends to) a file; missing folders are made.
    virtual bool WriteFile(const char* path, const void* data, size_t size, bool append = false) = 0;

    virtual bool DeleteFile(const char* path) = 0;
    virtual bool RenameFile(const char* from, const char* to) = 0;
    /// Makes a folder and those above it.
    virtual bool CreateDirectory(const char* path) = 0;
    /// Removes an empty folder.
    virtual bool RemoveDirectory(const char* path) = 0;

    /// The entries of a folder, without "." and "..". @return false when it cannot be read.
    virtual bool ListDirectory(const char* path, Source2ModDirCallback callback) = 0;

    /// A new KeyValues tree with an empty root named @p name; Release() it.
    virtual ISource2ModKeyValues* CreateKeyValues(const char* name) = 0;

    /**
     * @brief Reads KeyValues text, calling @p listener per event.
     * @param line, col Where it stopped (optional).
     */
    virtual ESource2ModKvError ParseKeyValues(const char* text, ISource2ModKvListener* listener,
                                              int* line = nullptr, int* col = nullptr, bool escapes = true) = 0;
    virtual ESource2ModKvError ParseKeyValuesFile(const char* path, ISource2ModKvListener* listener,
                                                  int* line = nullptr, int* col = nullptr, bool escapes = true) = 0;
    /// What an error means.
    virtual const char* KvErrorString(ESource2ModKvError error) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_FILES_H
