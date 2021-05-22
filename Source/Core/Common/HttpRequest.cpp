// Copyright 2017 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Common/HttpRequest.h"

#include <chrono>
#include <cstddef>
#include <mutex>
#include <optional>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <cstdlib>
#include "Common/WebAdapter.h"
#else
#include <curl/curl.h>
#endif

#include "Common/Logging/Log.h"
#include "Common/MsgHandler.h"
#include "Common/ScopeGuard.h"
#include "Common/StringUtil.h"

namespace Common
{
class HttpRequest::Impl final
{
public:
  enum class Method
  {
    GET,
    POST,
  };

  explicit Impl(std::chrono::milliseconds timeout_ms, ProgressCallback callback);

  bool IsValid() const;
  void SetCookies(const std::string& cookies);
  void UseIPv4();
  void FollowRedirects(long max);
  Response Fetch(const std::string& url, Method method, const Headers& headers, const u8* payload,
                 size_t size, AllowedReturnCodes codes = AllowedReturnCodes::Ok_Only);

#ifdef __EMSCRIPTEN__
  static int FetchProgressCallback(void* user_data, double dlnow, double dltotal);
#else
  static int CurlProgressCallback(Impl* impl, double dlnow, double dltotal, double ulnow,
                                  double ultotal);
#endif
  std::string EscapeComponent(const std::string& string);

private:
#ifdef __EMSCRIPTEN__
  u32 m_fetch_timeout_ms;
  double m_fetch_ultotal;
#else
  static inline std::once_flag s_curl_was_initialized;
  std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> m_curl{nullptr, curl_easy_cleanup};
  std::string m_error_string;
#endif
  ProgressCallback m_callback;
};

HttpRequest::HttpRequest(std::chrono::milliseconds timeout_ms, ProgressCallback callback)
    : m_impl(std::make_unique<Impl>(timeout_ms, std::move(callback)))
{
}

HttpRequest::~HttpRequest() = default;

bool HttpRequest::IsValid() const
{
  return m_impl->IsValid();
}

void HttpRequest::SetCookies(const std::string& cookies)
{
  m_impl->SetCookies(cookies);
}

void HttpRequest::UseIPv4()
{
  m_impl->UseIPv4();
}

void HttpRequest::FollowRedirects(long max)
{
  m_impl->FollowRedirects(max);
}

std::string HttpRequest::EscapeComponent(const std::string& string)
{
  return m_impl->EscapeComponent(string);
}

HttpRequest::Response HttpRequest::Get(const std::string& url, const Headers& headers,
                                       AllowedReturnCodes codes)
{
  return m_impl->Fetch(url, Impl::Method::GET, headers, nullptr, 0, codes);
}

HttpRequest::Response HttpRequest::Post(const std::string& url, const std::vector<u8>& payload,
                                        const Headers& headers, AllowedReturnCodes codes)
{
  return m_impl->Fetch(url, Impl::Method::POST, headers, payload.data(), payload.size(), codes);
}

HttpRequest::Response HttpRequest::Post(const std::string& url, const std::string& payload,
                                        const Headers& headers, AllowedReturnCodes codes)
{
  return m_impl->Fetch(url, Impl::Method::POST, headers,
                       reinterpret_cast<const u8*>(payload.data()), payload.size(), codes);
}

#ifdef __EMSCRIPTEN__
int HttpRequest::Impl::FetchProgressCallback(void* user_data, double dlnow, double dltotal)
{
  Impl* impl = static_cast<Impl*>(user_data);

  // Abort if callback isn't true
  return !impl->m_callback(dlnow, dltotal, impl->m_fetch_ultotal, impl->m_fetch_ultotal);
}
#else
int HttpRequest::Impl::CurlProgressCallback(Impl* impl, double dlnow, double dltotal, double ulnow,
                                            double ultotal)
{
  // Abort if callback isn't true
  return !impl->m_callback(dlnow, dltotal, ulnow, ultotal);
}
#endif  // __EMSCRIPTEN__

HttpRequest::Impl::Impl(std::chrono::milliseconds timeout_ms, ProgressCallback callback)
    : m_callback(std::move(callback))
{
#ifdef __EMSCRIPTEN__
  m_fetch_timeout_ms = timeout_ms.count();
#else
  std::call_once(s_curl_was_initialized, [] { curl_global_init(CURL_GLOBAL_DEFAULT); });

  m_curl.reset(curl_easy_init());
  if (!m_curl)
    return;

  curl_easy_setopt(m_curl.get(), CURLOPT_NOPROGRESS, m_callback == nullptr);

  if (m_callback)
  {
    curl_easy_setopt(m_curl.get(), CURLOPT_PROGRESSDATA, this);
    curl_easy_setopt(m_curl.get(), CURLOPT_PROGRESSFUNCTION, CurlProgressCallback);
  }

  // Set up error buffer
  m_error_string.resize(CURL_ERROR_SIZE);
  curl_easy_setopt(m_curl.get(), CURLOPT_ERRORBUFFER, m_error_string.data());

  // libcurl may not have been built with async DNS support, so we disable
  // signal handlers to avoid a possible and likely crash if a resolve times out.
  curl_easy_setopt(m_curl.get(), CURLOPT_NOSIGNAL, true);
  curl_easy_setopt(m_curl.get(), CURLOPT_CONNECTTIMEOUT_MS, static_cast<long>(timeout_ms.count()));
  // Sadly CURLOPT_LOW_SPEED_TIME doesn't have a millisecond variant so we have to use seconds
  curl_easy_setopt(
      m_curl.get(), CURLOPT_LOW_SPEED_TIME,
      static_cast<long>(std::chrono::duration_cast<std::chrono::seconds>(timeout_ms).count()));
  curl_easy_setopt(m_curl.get(), CURLOPT_LOW_SPEED_LIMIT, 1);
#ifdef _WIN32
  // ALPN support is enabled by default but requires Windows >= 8.1.
  curl_easy_setopt(m_curl.get(), CURLOPT_SSL_ENABLE_ALPN, false);
#endif
#endif  // __EMSCRIPTEN__
}

bool HttpRequest::Impl::IsValid() const
{
#ifdef __EMSCRIPTEN__
  return true;
#else
  return m_curl != nullptr;
#endif
}

void HttpRequest::Impl::SetCookies(const std::string& cookies)
{
  // Emscripten: no support, since the web is very strict about specifying custom cookies
  // TODO: maybe use a custom header paired with a special proxy?
#ifndef __EMSCRIPTEN__
  curl_easy_setopt(m_curl.get(), CURLOPT_COOKIE, cookies.c_str());
#endif
}

void HttpRequest::Impl::UseIPv4()
{
  // Emscripten: On the web, this is done automatically by the
  // browser, so the API cannot change this
#ifndef __EMSCRIPTEN__
  curl_easy_setopt(m_curl.get(), CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
#endif
}

void HttpRequest::Impl::FollowRedirects(long max)
{
  // Emscripten: could be done with a proper custom proxy
#ifndef __EMSCRIPTEN__
  curl_easy_setopt(m_curl.get(), CURLOPT_FOLLOWLOCATION, 1);
  curl_easy_setopt(m_curl.get(), CURLOPT_MAXREDIRS, max);
#endif
}

std::string HttpRequest::Impl::EscapeComponent(const std::string& string)
{
#ifdef __EMSCRIPTEN__
  char* escaped = WebAdapter_EncodeURIComponent(string.c_str());
#else
  char* escaped = curl_easy_escape(m_curl.get(), string.c_str(), static_cast<int>(string.size()));
#endif

  std::string escaped_str(escaped);

#ifdef __EMSCRIPTEN__
  std::free(escaped);
#else
  curl_free(escaped);
#endif

  return escaped_str;
}

#ifndef __EMSCRIPTEN__
static size_t CurlWriteCallback(char* data, size_t size, size_t nmemb, void* userdata)
{
  auto* buffer = static_cast<std::vector<u8>*>(userdata);
  const size_t actual_size = size * nmemb;
  buffer->insert(buffer->end(), data, data + actual_size);
  return actual_size;
}
#endif

HttpRequest::Response HttpRequest::Impl::Fetch(const std::string& url, Method method,
                                               const Headers& headers, const u8* payload,
                                               size_t size, AllowedReturnCodes codes)
{
  std::vector<u8> buffer;

#ifdef __EMSCRIPTEN__
  constexpr const char* cors_proxy = "https://cors-proxy.htmldriven.com/?url=";

  const char* type = method == Method::POST ? "POST" : "GET";

  // List of headers formatted as {key, value, key, value, ..., 0}
  std::vector<const char*> header_pack;
  header_pack.reserve(headers.size() * 2 + 1);

  for (const auto& [name, value] : headers)
  {
    header_pack.push_back(name.c_str());
    header_pack.push_back(value ? value->c_str() : "");
  }

  header_pack.push_back(0);

  m_fetch_ultotal = size;

  u8* fetch_data;
  u32 fetch_data_size;

  int response_code = WebAdapter_FetchSync(type, (cors_proxy + url).c_str(), header_pack.data(),
                                           m_fetch_timeout_ms, &fetch_data, &fetch_data_size,
                                           payload, size, FetchProgressCallback, this);

  if (response_code < 0)
  {
    ERROR_LOG_FMT(COMMON, "Failed to {} {}: a network error occurred", type, url);
    return std::nullopt;
  }

  buffer.insert(buffer.begin(), fetch_data, fetch_data + fetch_data_size);
  std::free(fetch_data);

  if (codes == AllowedReturnCodes::All)
    return buffer;

  if (response_code != 200)
  {
    if (buffer.empty())
    {
      ERROR_LOG_FMT(COMMON, "Failed to {} {}: fetch sent but got status {}", type, url,
                    response_code);
    }
    else
    {
      ERROR_LOG_FMT(COMMON, "Failed to {} {}: fetch sent but got status {} and body\n{:.{}}", type,
                    url, response_code, buffer.data(), static_cast<int>(buffer.size()));
    }
    return std::nullopt;
  }
#else
  curl_easy_setopt(m_curl.get(), CURLOPT_POST, method == Method::POST);
  curl_easy_setopt(m_curl.get(), CURLOPT_URL, url.c_str());
  if (method == Method::POST)
  {
    curl_easy_setopt(m_curl.get(), CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(m_curl.get(), CURLOPT_POSTFIELDSIZE, size);
  }

  curl_slist* list = nullptr;
  Common::ScopeGuard list_guard{[&list] { curl_slist_free_all(list); }};
  for (const auto& [name, value] : headers)
  {
    if (!value)
      list = curl_slist_append(list, (name + ':').c_str());
    else if (value->empty())
      list = curl_slist_append(list, (name + ';').c_str());
    else
      list = curl_slist_append(list, (name + ": " + *value).c_str());
  }
  curl_easy_setopt(m_curl.get(), CURLOPT_HTTPHEADER, list);

  curl_easy_setopt(m_curl.get(), CURLOPT_WRITEFUNCTION, CurlWriteCallback);
  curl_easy_setopt(m_curl.get(), CURLOPT_WRITEDATA, &buffer);

  const char* type = method == Method::POST ? "POST" : "GET";
  const CURLcode res = curl_easy_perform(m_curl.get());
  if (res != CURLE_OK)
  {
    ERROR_LOG_FMT(COMMON, "Failed to {} {}: {}", type, url, m_error_string);
    return std::nullopt;
  }

  if (codes == AllowedReturnCodes::All)
    return buffer;

  long response_code = 0;
  curl_easy_getinfo(m_curl.get(), CURLINFO_RESPONSE_CODE, &response_code);
  if (response_code != 200)
  {
    if (buffer.empty())
    {
      ERROR_LOG_FMT(COMMON, "Failed to {} {}: server replied with code {}", type, url,
                    response_code);
    }
    else
    {
      ERROR_LOG_FMT(COMMON, "Failed to {} {}: server replied with code {} and body\n\x1b[0m{:.{}}",
                    type, url, response_code, buffer.data(), static_cast<int>(buffer.size()));
    }
    return std::nullopt;
  }
#endif  // __EMSCRIPTEN__

  return buffer;
}
}  // namespace Common
