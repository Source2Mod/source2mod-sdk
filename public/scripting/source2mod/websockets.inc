/**
 * Source2Mod SourcePawn API: WebSocket clients (ISource2ModWebSockets).
 * Connections run off the game thread; every callback comes back on the
 * game thread. A lost connection is opened again on its own, with a delay
 * growing from 1 to 30 seconds. A socket goes with the plugin that made it.
 */

#if defined _source2mod_websockets_included
 #endinput
#endif
#define _source2mod_websockets_included

#include <source2mod/http>

enum Source2ModWebSocketState
{
	/** Opening, or waiting to open again after the connection was lost. */
	WebSocketState_Connecting = 0,
	WebSocketState_Open,
	/** Closed from here, or lost with reconnecting off. For good. */
	WebSocketState_Closed
};

/** The handshake went through; reconnected for every open after the first. */
typedef Source2ModWebSocketOpen = function void (Source2ModWebSocket socket, bool reconnected, any data);

/**
 * A whole message. A binary one may hold zeros: length is its size.
 */
typedef Source2ModWebSocketMessage = function void (Source2ModWebSocket socket, const char[] message, int length, bool binary, any data);

/**
 * The connection is gone. code is the close code either side sent, 1006 when
 * it was lost without one. Unless it was closed from here or reconnecting is
 * off, the socket is connecting again after this.
 */
typedef Source2ModWebSocketClose = function void (Source2ModWebSocket socket, int code, const char[] reason, any data);

/** Why connecting or the connection failed, for logging. */
typedef Source2ModWebSocketError = function void (Source2ModWebSocket socket, const char[] error, any data);

methodmap Source2ModWebSocket < Handle
{
	/**
	 * Opens a WebSocket. delete closes it; no callback runs after that.
	 *
	 * @param url           ws:// or wss://.
	 * @param headers       Extra headers on the opening handshake, or null; copied.
	 * @param autoReconnect Open the connection again when it is lost.
	 * @param verifyPeer    Check the server's certificate (wss://); off only for
	 *                      test servers with a self-signed one.
	 * @param pingInterval  Seconds without traffic before a ping; 0 for none.
	 * @return              The socket, connecting; null for a URL that is not
	 *                      ws:// or wss://.
	 */
	public native Source2ModWebSocket(const char[] url, Source2ModWebSocketMessage onMessage,
		Source2ModWebSocketOpen onOpen = INVALID_FUNCTION, Source2ModWebSocketClose onClose = INVALID_FUNCTION,
		Source2ModWebSocketError onError = INVALID_FUNCTION, any data = 0, Source2ModHTTPHeaders headers = null,
		bool autoReconnect = true, bool verifyPeer = true, int pingInterval = 30);

	/** Sends a text message. Queued in order; false while not open. */
	public native bool Send(const char[] text);

	/** Sends length bytes of data as a binary message; false while not open. */
	public native bool SendBinary(const char[] data, int length);

	/** Closes the connection with code; it is not opened again. onClose follows. */
	public native void Close(int code = 1000, const char[] reason = "");

	public native void GetUrl(char[] buffer, int maxlen);

	property Source2ModWebSocketState State
	{
		public native get();
	}
};
