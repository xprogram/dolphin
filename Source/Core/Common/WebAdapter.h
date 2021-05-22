// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

extern "C" {

/**
 * Logs a message to the console with the specified text color.
 *
 * @param msg the message to log
 * @param color a valid CSS color to style the text with
 */
extern void WebAdapter_LogColored(const char* msg, const char* color);

/**
 * Encode a string as a URI component, for suitable use within URL parameters.
 *
 * @return a null-terminated buffer containing the encoded data that must be
 * freed via free().
 */
extern char* WebAdapter_EncodeURIComponent(const char* data);

/**
 * Launches a synchronous HTTP request using the given verb, URL, headers and (optional) body.
 * Unless a suitable proxy is used, the request will be subject to limitations arising from CORS
 * and may fail as a result. Should not be used on the main UI thread.
 *
 * @param verb what kind of request to do, should either be "GET" or "POST"
 * @param url the URL that will receive the HTTP request
 * @param headers an optional list of C-strings as key-value pairs; {header name, header value,
 * header name,
 * ..., 0}
 * @param timeout the timeout, in milliseconds, to wait before aborting the sent request
 * @param out_data the HTTP response. The data it points to must be freed using free()
 * @param out_size length of the data from the HTTP response
 * @param payload the HTTP request body (optional, can be null)
 * @param payload_size the HTTP request body length (ignored if there was no payload given)
 * @param progress_cb an optional progress function that can stop the transfer if it ever returns
 * true
 * @param user_data a pointer that is passed into the progress function
 *
 * @return the HTTP status code if the transfer completed, or a negative value if there was an
 * error while processing the request.
 */
extern int WebAdapter_FetchSync(const char* verb, const char* url, const char** headers,
                                unsigned int timeout, unsigned char** out_data,
                                unsigned int* out_size, const unsigned char* payload,
                                unsigned int payload_size,
                                int (*progress_cb)(void*, double, double), void* user_data);

/**
 * Return the user agent string of the web browser the program is currently
 * running in. This corresponds to the JavaScript 'navigator.userAgent' property.
 * If the program's runtime environment is not that of a browser (e.g. a Node.js
 * process), a generic placeholder string is returned. The returned value is cached, so
 * there is no need to free() it.
 */
extern const char* WebAdapter_GetUserAgent(void);

/**
 * Shows a blocking prompt (when possible) to the user with a specified message.
 * In a web browser environment, this function will use the 'alert()' & 'confirm()'
 * JavaScript functions for displaying a blocking popup with the message, and then
 * return 1 only if a confirmation dialog was shown and the user accepted it; otherwise, 0
 * is returned. Should those functions not exist (for example, in a Node.js process), then
 * the message is logged to the console and 0 is always returned.
 *
 * @param msg The message to display/log.
 * @param use_confirm whether or not to ask for user confirmation (via 'confirm()').
 */
extern int WebAdapter_DisplayAlert(const char* msg, int use_confirm);

typedef void (*WebAdapter_SignalHandler)(int);

/**
 * Set a signal handler for when the specified (pseudo-)signal is triggered from
 * external code. The handler will be reset as soon as its corresponding signal
 * is triggered. The handler runs in the main UI thread, which means that
 * blocking operations within it must be avoided.
 *
 * @param sig a constant that identifies the signal to attach to
 * @param handler the signal()-like handler that will be attached
 */
extern void WebAdapter_SetSignalHandler(int sig, WebAdapter_SignalHandler handler);
}