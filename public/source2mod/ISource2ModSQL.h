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
 * @file ISource2ModSQL.h
 * @brief Interfaces and utilities for SQL database access: MySQL and SQLite
 *        behind one interface.
 *
 * This module provides functionality for:
 * * MySQL and SQLite connections (ESource2ModSQLDriver), or ones named in
 *   configs/databases.json
 * * Query execution
 * * Transactions
 * * Result set handling
 * * Data escaping
 *
 * @note Commonly used for persistent storage and asynchronous database operations.
 */

#ifndef _INCLUDE_ISOURCE2MOD_SQL_H
#define _INCLUDE_ISOURCE2MOD_SQL_H

#pragma once

#include <source2mod/ISource2ModTypes.h>
#include "ISource2ModPlugin.h"

#include <functional>
#include <string>
#include <vector>

/* =========================
Forward declarations
========================= */

class ISource2ModSQLQuery;

/* =========================
Callback typedefs
========================= */

typedef std::function<void(bool)> Source2ModSQLConnectCallbackFunc;
typedef std::function<void(ISource2ModSQLQuery*)> Source2ModSQLQueryCallbackFunc;
typedef std::function<void(std::vector<ISource2ModSQLQuery*>)> Source2ModSQLTransactionSuccessCallbackFunc;
typedef std::function<void(std::string, int)> Source2ModSQLTransactionFailureCallbackFunc;

/* =========================
Drivers
========================= */

enum ESource2ModSQLDriver
{
    /// A MySQL / MariaDB server: host, user, pass, database, port.
    SOURCE2MOD_SQL_DRIVER_MYSQL = 0,
    /// An SQLite file: database is its path (a relative one is under
    /// addons/source2mod/data, made when missing); host, user and pass unused.
    SOURCE2MOD_SQL_DRIVER_SQLITE = 1,
};

/* =========================
Field types: MySQL's numbering. An SQLite column reports INTEGER as LONGLONG,
REAL as DOUBLE, TEXT as STRING, BLOB as BLOB and NULL as NULL.
========================= */

typedef enum ESource2ModSQLType
{
    SOURCE2MOD_SQL_TYPE_DECIMAL,
    SOURCE2MOD_SQL_TYPE_TINY,
    SOURCE2MOD_SQL_TYPE_SHORT,
    SOURCE2MOD_SQL_TYPE_LONG,
    SOURCE2MOD_SQL_TYPE_FLOAT,
    SOURCE2MOD_SQL_TYPE_DOUBLE,
    SOURCE2MOD_SQL_TYPE_NULL,
    SOURCE2MOD_SQL_TYPE_TIMESTAMP,
    SOURCE2MOD_SQL_TYPE_LONGLONG,
    SOURCE2MOD_SQL_TYPE_INT24,
    SOURCE2MOD_SQL_TYPE_DATE,
    SOURCE2MOD_SQL_TYPE_TIME,
    SOURCE2MOD_SQL_TYPE_DATETIME,
    SOURCE2MOD_SQL_TYPE_YEAR,
    SOURCE2MOD_SQL_TYPE_NEWDATE,
    SOURCE2MOD_SQL_TYPE_VARCHAR,
    SOURCE2MOD_SQL_TYPE_BIT,
    SOURCE2MOD_SQL_TYPE_TIMESTAMP2,
    SOURCE2MOD_SQL_TYPE_DATETIME2,
    SOURCE2MOD_SQL_TYPE_TIME2,
    SOURCE2MOD_SQL_TYPE_UNKNOWN,

    SOURCE2MOD_SQL_TYPE_JSON = 245,
    SOURCE2MOD_SQL_TYPE_NEWDECIMAL = 246,
    SOURCE2MOD_SQL_TYPE_ENUM = 247,
    SOURCE2MOD_SQL_TYPE_SET = 248,
    SOURCE2MOD_SQL_TYPE_TINY_BLOB = 249,
    SOURCE2MOD_SQL_TYPE_MEDIUM_BLOB = 250,
    SOURCE2MOD_SQL_TYPE_LONG_BLOB = 251,
    SOURCE2MOD_SQL_TYPE_BLOB = 252,
    SOURCE2MOD_SQL_TYPE_VAR_STRING = 253,
    SOURCE2MOD_SQL_TYPE_STRING = 254,
    SOURCE2MOD_SQL_TYPE_GEOMETRY = 255
} ESource2ModSQLType;

/* =========================
Transaction
========================= */

struct Source2ModSQLTransaction
{
    std::vector<std::string> queries;
};

/* =========================
Connection info
========================= */

struct Source2ModSQLConnectionInfo
{
    const char* host;
    const char* user;
    const char* pass;
    const char* database;

    int port = 3306;
    int timeout = 60;

    /// Which server this is; MySQL unless said otherwise.
    ESource2ModSQLDriver driver = SOURCE2MOD_SQL_DRIVER_MYSQL;
};

/* =========================
Result row
========================= */

class ISource2ModSQLRow
{
public:
    virtual ~ISource2ModSQLRow() = default;
};

/* =========================
Result set
========================= */

class ISource2ModSQLResult
{
public:
    virtual ~ISource2ModSQLResult() = default;

    /**
     * @brief Returns the total number of rows in the result set.
     */
    virtual int GetRowCount() = 0;

    /**
     * @brief Returns the number of columns in the result set.
     */
    virtual int GetFieldCount() = 0;

    /**
     * @brief Converts a field name into a column index.
     *
     * @param name Field name to search for.
     * @param columnId Output column index.
     * @return True if the field exists, false otherwise.
     */
    virtual bool FieldNameToNum(const char* name, unsigned int* columnId) = 0;

    /**
     * @brief Returns the field name for a column index.
     *
     * @param colId Column index.
     * @return Field name string.
     */
    virtual const char* FieldNumToName(unsigned int colId) = 0;

    /**
     * @brief Checks whether additional rows are available.
     *
     * @return True if more rows can be fetched.
     */
    virtual bool MoreRows() = 0;

    /**
     * @brief Fetches the next row from the result set.
     *
     * @return Pointer to the fetched row.
     */
    virtual ISource2ModSQLRow* FetchRow() = 0;

    /**
     * @brief Returns the currently active row.
     *
     * @return Pointer to the current row.
     */
    virtual ISource2ModSQLRow* CurrentRow() = 0;

    /**
     * @brief Rewinds the internal row cursor to the beginning.
     *
     * @return True on success.
     */
    virtual bool Rewind() = 0;

    /**
     * @brief Returns the field type of a column (ESource2ModSQLType numbering).
     *
     * @param field Column index.
     * @return Field type identifier.
     */
    virtual int GetFieldType(unsigned int field) = 0;

    /**
     * @brief Retrieves a string value from a column.
     *
     * @param columnId Column index.
     * @param length Optional output string length.
     * @return Pointer to string data.
     */
    virtual const char* GetString(unsigned int columnId, size_t* length = nullptr) = 0;

    /**
     * @brief Returns the raw data size of a column value.
     *
     * @param columnId Column index.
     * @return Size in bytes.
     */
    virtual size_t GetDataSize(unsigned int columnId) = 0;

    /**
     * @brief Retrieves a floating-point value from a column.
     *
     * @param columnId Column index.
     * @return Float value.
     */
    virtual float GetFloat(unsigned int columnId) = 0;

    /**
     * @brief Retrieves an integer value from a column.
     *
     * @param columnId Column index.
     * @return Integer value.
     */
    virtual int GetInt(unsigned int columnId) = 0;

    /**
     * @brief Retrieves a 64-bit integer value from a column.
     *
     * @param columnId Column index.
     * @return 64-bit integer value.
     */
    virtual int64_t GetInt64(unsigned int columnId) = 0;

    /**
     * @brief Checks whether a column contains a NULL value.
     *
     * @param columnId Column index.
     * @return True if the value is NULL.
     */
    virtual bool IsNull(unsigned int columnId) = 0;
};

/* =========================
Query
========================= */

class ISource2ModSQLQuery
{
public:
    virtual ~ISource2ModSQLQuery() = default;

    /**
     * @brief Returns the current result set.
     *
     * @return Pointer to the result set.
     */
    virtual ISource2ModSQLResult* GetResultSet() = 0;

    /**
     * @brief Advances to the next available result set.
     *
     * @return True if another result set exists.
     */
    virtual bool FetchMoreResults() = 0;

    /**
     * @brief Returns the last inserted row ID.
     *
     * @return Insert ID value.
     */
    virtual unsigned int GetInsertId() = 0;

    /**
     * @brief Returns the number of affected rows.
     *
     * @return Affected row count.
     */
    virtual unsigned int GetAffectedRows() = 0;
};

/* =========================
Connection
========================= */

class ISource2ModSQLConnection
{
public:
    virtual ~ISource2ModSQLConnection() = default;

    /**
     * @brief Establishes the connection (opens the file, for SQLite).
     *
     * @param callback Callback invoked after connection attempt.
     */
    virtual void Connect(Source2ModSQLConnectCallbackFunc callback) = 0;

    /**
     * @brief Executes a raw SQL query.
     *
     * @param query SQL query string.
     * @param callback Callback invoked with query result, on the main thread.
     *
     * @note The callback always runs, once. On success it gets the query,
     *       owned by Source2Mod and valid only for the duration of the
     *       callback -- copy anything you keep, do not delete it. On failure
     *       it gets nullptr: the connection is not up (Connect() has not
     *       reported true yet, or it failed) or the server rejected the
     *       query. The reason is written to the server log.
     */
    virtual void Query(char* query, Source2ModSQLQueryCallbackFunc callback) = 0;

    /**
     * @brief Executes a formatted SQL query.
     *
     * @param query SQL format string.
     * @param callback Callback invoked with query result, on the main thread.
     *
     * @note Same callback contract as the overload above: nullptr on failure,
     *       otherwise a source2mod-owned query valid only during the callback.
     */
    virtual void Query(const char* query, Source2ModSQLQueryCallbackFunc callback, ...) = 0;

    /**
     * @brief Executes multiple queries as a transaction.
     *
     * Exactly one of the two callbacks runs, on the main thread.
     *
     * @param txn Transaction query batch.
     * @param success Callback invoked on success, with one query per entry of
     *                @p txn, in order. They are owned by Source2Mod and freed
     *                once the callback returns -- copy anything you keep, do
     *                not delete them.
     * @param failure Callback invoked on failure, after the transaction has
     *                been rolled back, with the error and the index of the
     *                query that failed -- or -1 when it was not one of them
     *                (not connected, BEGIN or COMMIT failed).
     */
    virtual void ExecuteTransaction(Source2ModSQLTransaction txn, Source2ModSQLTransactionSuccessCallbackFunc success,
                                    Source2ModSQLTransactionFailureCallbackFunc failure) = 0;

    /**
     * @brief Destroys and releases the connection instance.
     */
    virtual void Destroy() = 0;

    /**
     * @brief The driver this connection talks to.
     */
    virtual ESource2ModSQLDriver GetDriver() = 0;

    /**
     * @brief Escapes a mutable string for safe SQL usage.
     *
     * @param string Input string.
     * @return Escaped string.
     */
    virtual std::string Escape(char* string) = 0;

    /**
     * @brief Escapes a constant string for safe SQL usage.
     *
     * @param string Input string.
     * @return Escaped string.
     */
    virtual std::string Escape(const char* string) = 0;
};

/* =========================
Serverless queries
========================= */

/**
 * @brief What one serverless query came back with.
 *
 * @note m_bSuccess is about the query, not about rows -- a SELECT that matched
 *       nothing is a success with an empty result set. A query that failed has
 *       m_bSuccess false, m_sError filled in and m_pQuery null.
 */
struct Source2ModSQLServerlessResult
{
    bool m_bSuccess = false;

    /// The driver's own error number (MySQL's, or SQLite's result code), or 0
    /// when the query never reached the database.
    unsigned int m_nErrorCode = 0;
    std::string m_sError;

    /// Rows, insert id and affected rows, read off the connection before it
    /// was closed again. Owned by Source2Mod and valid only for the duration
    /// of the callback -- copy anything you keep. Null when the query failed.
    ISource2ModSQLQuery* m_pQuery = nullptr;
};

/**
 * @brief Callback for a completed serverless query. Runs on the main thread.
 */
typedef std::function<void(const Source2ModSQLServerlessResult&)> Source2ModSQLServerlessCallbackFunc;

/**
 * @brief A set of connection details that queries are run against one at a time.
 *
 * Every query opens its own connection, runs, reads its rows out and closes
 * again -- there is nothing to keep alive, nothing to reconnect, and nothing
 * that can go stale over a map change. None of it happens on the main thread:
 * queries go through Source2Mod's shared SQL worker, one thread for all of
 * them however many plugins are using it, and land back in a callback on the
 * main thread.
 *
 * Meant for the occasional query -- loading a player on connect, writing their
 * stats on disconnect. A connection costs a handshake and a login every single
 * time, so for anything that runs per tick or per shot, keep an
 * ISource2ModSQLConnection open instead.
 *
 * @code
 * // once, when your plugin loads
 * m_pDb = api->SQL()->CreateServerless(g_PLID, { "127.0.0.1", "user", "pass", "db" });
 *
 * // wherever you need it
 * m_pDb->Query("SELECT name, kills FROM players WHERE steamid = ?", { steamid },
 *     [](const Source2ModSQLServerlessResult& r)
 * {
 *     if (!r.m_bSuccess)
 *         return; // r.m_sError says what went wrong
 *
 *     ISource2ModSQLResult* pSet = r.m_pQuery->GetResultSet();
 *     while (pSet && pSet->MoreRows() && pSet->FetchRow())
 *     {
 *         // pSet->GetString(0), pSet->GetInt(1) -- copy anything you keep.
 *     }
 * });
 * @endcode
 */
class ISource2ModSQLServerless
{
public:
    virtual ~ISource2ModSQLServerless() = default;

    /**
     * @brief Runs one query on a connection of its own.
     *
     * @param query SQL to run.
     * @param callback Callback invoked with the result, on the main thread.
     */
    virtual void Query(const char* query, Source2ModSQLServerlessCallbackFunc callback) = 0;

    /**
     * @brief Runs one parameterised query on a connection of its own.
     *
     * Placeholders are filled in on the worker against the live connection:
     * `?` becomes the escaped value in quotes, `??` becomes a quoted
     * identifier -- a table or column name, which cannot be a `?`. A `?`
     * inside a string literal is left alone, and a placeholder count that does
     * not match @p params fails the query instead of running it.
     *
     * @param query SQL to run, with `?` / `??` placeholders.
     * @param params Values for the placeholders, in order.
     * @param callback Callback invoked with the result, on the main thread.
     */
    virtual void Query(const char* query, std::vector<std::string> params,
                       Source2ModSQLServerlessCallbackFunc callback) = 0;

    /**
     * @brief Drops these details and cancels what they still have queued. A
     *        query already on the wire is left to finish and its result thrown
     *        away -- its callback does not run.
     */
    virtual void Destroy() = 0;
};

/* =========================
SQL client
========================= */

#define SOURCE2MOD_SQL_INTERFACE "ISource2ModSQL002"

class ISource2ModSQL
{
public:
    virtual ~ISource2ModSQL() = default;

    /**
     * @brief Creates a new connection instance (MySQL or SQLite, by info.driver).
     *
     * @param owner Plugin the connection belongs to.
     * @param info Connection configuration data.
     * @return Newly created connection.
     *
     * @note Destroyed for you if the owning plugin unloads without calling
     *       Destroy(): the worker thread runs code, and the query callbacks
     *       are std::functions holding code, inside that plugin's library.
     */
    virtual ISource2ModSQLConnection* CreateConnection(Source2ModSQLConnectionInfo info) = 0;

    /**
     * @brief Creates a handle that runs each of its queries on a connection of
     *        its own, opened and closed around that one query.
     *
     * @param owner Plugin the handle belongs to.
     * @param info Connection configuration data. Copied, strings included --
     *             it does not have to outlive this call.
     * @return Newly created serverless handle.
     *
     * @note Destroyed for you if the owning plugin unloads without calling
     *       Destroy(), together with every query of its still in the air: a
     *       pending callback is a std::function holding code inside that
     *       plugin's library.
     */
    virtual ISource2ModSQLServerless* CreateServerless(Source2ModSQLConnectionInfo info) = 0;
    /**
     * @brief The connection details named @p name in configs/databases.json
     *        ({"name": {"driver": "mysql"|"sqlite", "host", "user", "pass",
     *        "database", "port", "timeout"}}), or "default" for an empty name.
     *
     * @param info Filled in when found; its strings stay valid until the
     *             config is read again (map change).
     * @return false when there is no such entry.
     */
    virtual bool GetNamedConnection(const char* name, Source2ModSQLConnectionInfo& info) = 0;
};

#define SQL_CREATE_CONNECTION(info)    g_pSource2ModSQL->CreateConnection(info)

#endif //_INCLUDE_ISOURCE2MOD_SQL_H
