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

* @file ISource2ModHTTP.h
* @brief Interface for HTTP requests.
*
* Requests go out through Steam's own HTTP client, which the game server already
* has. Where Steam cannot take one, the core's own client (Boost.Beast) does:
* before the Steam API has activated (after level startup), when Steam refuses
* a request, and when Steam loses a GET, HEAD, PUT or DELETE without an answer
* (a POST or PATCH is never sent twice). A plugin does not have to care whether
* it is early. With core.json's HttpFallback off it is Steam only, and requests
* made before the API activates are queued until it does.
*
* Responses come back on the main thread.
  */

#ifndef _INCLUDE_ISOURCE2MOD_HTTP_H
#define _INCLUDE_ISOURCE2MOD_HTTP_H

#pragma once
#include <string>
#include <utility>
#include <vector>

// Before ISource2ModJSON.h and ISource2ModPlugin.h: ISource2ModWebSockets.h,
// which they pull in, uses it too.

/**

* @brief One request or response header.
  */
struct Source2ModHTTPHeader
{
    Source2ModHTTPHeader() = default;
    Source2ModHTTPHeader(std::string name, std::string value)
        : m_sName(std::move(name)), m_sValue(std::move(value))
    {
    }

    std::string m_sName;
    std::string m_sValue;
};

#include "ISource2ModJSON.h"
#include "ISource2ModPlugin.h"

/* =========================
Request types
========================= */

/**

* @brief HTTP method of a request.
  */
enum class ESource2ModHTTPMethod : int
{
    Get = 0,
    Post,
    Put,
    Patch,
    Delete,
    Head,
};


/**

* @brief What a completed request came back with.
*
* @note m_bSuccess is about the transport, not the status code -- a 404 that
*       arrived is a success with m_nStatusCode == 404. A request that never
*       completed has m_bSuccess false and m_nStatusCode 0.
  */
struct Source2ModHTTPResponse
{
    bool m_bSuccess = false;
    int m_nStatusCode = 0;
    std::string m_sBody;

    /// The body parsed as JSON, when it was JSON. Owned by Source2Mod and
    /// valid only for the duration of the callback -- copy anything you keep.
    /// Null when the body was empty or did not parse.
    const ISource2ModJSONValue* m_pJson = nullptr;
};

/**

* @brief Callback for a completed request.
  */
/// Called with the response on the main thread: a function, an object and a
/// method, or a lambda (Source2ModCallback) -- whose it is, the core reads off it.
using Source2ModHTTPCallback = Source2ModCallback<void(const Source2ModHTTPResponse& response)>;

/* =========================
Core Source2Mod HTTP
========================= */

/**

* @brief Issues HTTP requests through Steam's HTTP client, or the core's own
*        where Steam cannot (see the file comment).
*
* @code
* HTTP_GET("https://example.com/api", [](const Source2ModHTTPResponse& r)
* {
*     if (!r.m_bSuccess || r.m_nStatusCode != 200)
*         return;
*
*     // r.m_sBody -- parse with whatever your plugin already uses.
* });
* @endcode
  */
#define SOURCE2MOD_HTTP_INTERFACE "ISource2ModHTTP002"

class ISource2ModHTTP
{
public:
    virtual ~ISource2ModHTTP() = default;

    /**

    * @brief Whether a request goes out right away.
    *
    * Always true unless HttpFallback is off and Steam's HTTP client is not
    * there yet. Requests made while this is false are queued, not dropped, so
    * checking is only worth it to avoid building a body you cannot send.
      */
    virtual bool IsAvailable() const = 0;

    /**

    * @brief Number of requests waiting for Steam's HTTP client (only ever
    *        non-zero with HttpFallback off).
      */
    virtual int PendingCount() const = 0;

    /**

    * @brief Issues a request.
    *
    * The plugin the callback's code lies in owns the request.
    *
    * @param method HTTP method
    * @param pszUrl Absolute URL
    * @param pszBody Request body, or nullptr for methods that take none
    * @param callback Called once the request completes; may be null
    * @param pHeaders Extra request headers, or nullptr
    *
    * @note Cancelled, callback and all, if the owning plugin unloads first --
    *       the callback holds code inside that plugin's library.
      */
    virtual void Request(ESource2ModHTTPMethod method,
                         const char* pszUrl,
                         const char* pszBody,
                         Source2ModHTTPCallback callback,
                         const std::vector<Source2ModHTTPHeader>* pHeaders = nullptr) = 0;


    /* =========================
    Shorthands
    ========================= */

    virtual void Get(const char* pszUrl, Source2ModHTTPCallback callback,
                     const std::vector<Source2ModHTTPHeader>* pHeaders = nullptr) = 0;

    virtual void Post(const char* pszUrl, const char* pszBody, Source2ModHTTPCallback callback,
                      const std::vector<Source2ModHTTPHeader>* pHeaders = nullptr) = 0;

    virtual void Put(const char* pszUrl, const char* pszBody, Source2ModHTTPCallback callback,
                     const std::vector<Source2ModHTTPHeader>* pHeaders = nullptr) = 0;

    virtual void Patch(const char* pszUrl, const char* pszBody, Source2ModHTTPCallback callback,
                       const std::vector<Source2ModHTTPHeader>* pHeaders = nullptr) = 0;

    virtual void Delete(const char* pszUrl, const char* pszBody, Source2ModHTTPCallback callback,
                        const std::vector<Source2ModHTTPHeader>* pHeaders = nullptr) = 0;

};

#define HTTP_REQUEST(method, pszUrl, pszBody, cb, ...) \
    g_pSource2ModHTTP->Request(method, pszUrl, pszBody, cb, ##__VA_ARGS__)

#define HTTP_GET(pszUrl, cb, ...) \
    g_pSource2ModHTTP->Get(pszUrl, cb, ##__VA_ARGS__)
#define HTTP_POST(pszUrl, pszBody, cb, ...) \
    g_pSource2ModHTTP->Post(pszUrl, pszBody, cb, ##__VA_ARGS__)
#define HTTP_PUT(pszUrl, pszBody, cb, ...) \
    g_pSource2ModHTTP->Put(pszUrl, pszBody, cb, ##__VA_ARGS__)
#define HTTP_PATCH(pszUrl, pszBody, cb, ...) \
    g_pSource2ModHTTP->Patch(pszUrl, pszBody, cb, ##__VA_ARGS__)
#define HTTP_DELETE(pszUrl, pszBody, cb, ...) \
    g_pSource2ModHTTP->Delete(pszUrl, pszBody, cb, ##__VA_ARGS__)

#endif //_INCLUDE_ISOURCE2MOD_HTTP_H
