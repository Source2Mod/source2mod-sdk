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
* @file ISource2ModWebSockets.h
* @brief WebSocket clients (ws:// and wss://).
*
* Steam has no WebSocket client, so these are Source2Mod's own: one IO thread
* in the core runs every connection, and everything a socket reports reaches
* its listener on the main thread. A lost connection is opened again on its
* own, with a delay growing from 1 to 30 seconds.
*
* @code
* class Feed : public ISource2ModWebSocketListener
* {
*     void OnWebSocketOpen(ISource2ModWebSocket* socket, bool reconnected) override
*     {
*         socket->Send("{\"hello\":1}");
*     }
*     void OnWebSocketMessage(ISource2ModWebSocket* socket, const char* data, size_t length, bool binary) override
*     {
*         // data is valid for this call only.
*     }
* };
*
* Feed g_feed;
* ISource2ModWebSocket* g_pSocket = g_pSource2ModWebSockets->Connect("wss://example.com/feed", &g_feed);
* // ...
* g_pSocket->Destroy();
* @endcode
  */

#ifndef _INCLUDE_ISOURCE2MOD_WEBSOCKETS_H
#define _INCLUDE_ISOURCE2MOD_WEBSOCKETS_H

#pragma once
#include "ISource2ModHTTP.h"
#include "ISource2ModPlugin.h"

#include <cstddef>
#include <vector>

/// Where a socket is.
enum class ESource2ModWebSocketState : int
{
    /// Opening, or waiting to open again after the connection was lost.
    Connecting = 0,
    Open,
    /// Closed from this side, or lost with reconnecting off. For good.
    Closed,
};

/// How Connect() connects. The defaults suit most servers.
struct Source2ModWebSocketOptions
{
    /// Extra headers on the opening handshake (Authorization, cookies, ...).
    std::vector<Source2ModHTTPHeader> m_Headers;
    /// Open the connection again when it is lost.
    bool m_bAutoReconnect = true;
    /// Seconds without traffic before a ping goes out; the connection counts
    /// as lost after twice that. 0 for no pings.
    int m_nPingIntervalSec = 30;
    /// Check the server's certificate (wss://) against the system's CA store.
    /// Off only for test servers with a self-signed one.
    bool m_bVerifyPeer = true;
    /// A larger message drops the connection.
    size_t m_nMaxMessageSize = 16u << 20;
};

class ISource2ModWebSocket;

/**
* @brief What a socket reports, on the main thread. Every method is optional.
*
* The plugin the listener's class lies in owns the socket: it is destroyed,
* listener calls and all, when that plugin unloads.
*/
class ISource2ModWebSocketListener
{
public:
    virtual ~ISource2ModWebSocketListener() = default;

    /// The handshake went through. @p reconnected for every open after the first.
    virtual void OnWebSocketOpen(ISource2ModWebSocket* socket, bool reconnected) {}

    /// A whole message. @p data is valid only for this call; a text message is
    /// also null-terminated.
    virtual void OnWebSocketMessage(ISource2ModWebSocket* socket, const char* data, size_t length, bool binary) {}

    /**
    * @brief The connection is gone.
    *
    * @param code The close code either side sent, 1006 when the connection
    *        was lost without one.
    * @note Unless it was closed from here or m_bAutoReconnect is off, the
    *       socket is Connecting again after this, and OnWebSocketOpen follows
    *       once it is back.
    */
    virtual void OnWebSocketClose(ISource2ModWebSocket* socket, int code, const char* reason) {}

    /// Why connecting or the connection failed, for logging.
    virtual void OnWebSocketError(ISource2ModWebSocket* socket, const char* error) {}
};

/// One connection. Valid until Destroy(), or until its owner unloads.
class ISource2ModWebSocket
{
public:
    /// Sends a text message. Queued in order; false while the socket is not Open.
    virtual bool Send(const char* text) = 0;

    /// Sends a binary message; otherwise like Send().
    virtual bool SendBinary(const void* data, size_t length) = 0;

    /// Closes the connection with @p code; it is not opened again.
    /// OnWebSocketClose follows once the server answered.
    virtual void Close(int code = 1000, const char* reason = "") = 0;

    virtual ESource2ModWebSocketState GetState() const = 0;

    virtual const char* GetUrl() const = 0;

    /// Closes the connection if it is open and frees the socket. The listener
    /// hears nothing more, not even OnWebSocketClose. Safe from inside a
    /// listener call.
    virtual void Destroy() = 0;

protected:
    ~ISource2ModWebSocket() = default;
};

#define SOURCE2MOD_WEBSOCKETS_INTERFACE "ISource2ModWebSockets001"

class ISource2ModWebSockets
{
public:
    virtual ~ISource2ModWebSockets() = default;

    /**
    * @brief Opens a WebSocket to @p url.
    *
    * @param url ws:// or wss://, with a port and a path when needed.
    * @param listener Where the socket reports; must outlive it.
    * @return The socket, Connecting; nullptr for a URL that is not ws:// or wss://.
    */
    virtual ISource2ModWebSocket* Connect(const char* url, ISource2ModWebSocketListener* listener,
                                          const Source2ModWebSocketOptions& options = Source2ModWebSocketOptions()) = 0;
};

#endif //_INCLUDE_ISOURCE2MOD_WEBSOCKETS_H
