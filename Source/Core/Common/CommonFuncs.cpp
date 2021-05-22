// Copyright 2009 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include <cstddef>
#include <cstring>
#include <errno.h>
#include <type_traits>

#include "Common/CommonFuncs.h"

#ifdef _WIN32
#include <windows.h>
#define strerror_r(err, buf, len) strerror_s(buf, len, err)
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#include <emscripten/html5.h>
#include <fmt/format.h>
#include <unzip.h>

#include "Common/CommonPaths.h"
#include "Common/FileUtil.h"
#include "Common/IOFile.h"
#include "Common/ScopeGuard.h"
#include "Common/StringUtil.h"
#endif

constexpr size_t BUFFER_SIZE = 256;

// Wrapper function to get last strerror(errno) string.
// This function might change the error code.
std::string LastStrerrorString()
{
  char error_message[BUFFER_SIZE];

  // There are two variants of strerror_r. The XSI version stores the message to the passed-in
  // buffer and returns an int (0 on success). The GNU version returns a pointer to the message,
  // which might have been stored in the passed-in buffer or might be a static string.

  // We check defines in order to figure out which variant is in use, and we store the returned
  // value to a variable so that we'll get a compile-time check that our assumption was correct.

#if (defined(__GLIBC__) || __ANDROID_API__ >= 23) &&                                               \
    (_GNU_SOURCE || (_POSIX_C_SOURCE < 200112L && _XOPEN_SOURCE < 600))
  const char* str = strerror_r(errno, error_message, BUFFER_SIZE);
  return std::string(str);
#else
  int error_code = strerror_r(errno, error_message, BUFFER_SIZE);
  return error_code == 0 ? std::string(error_message) : "";
#endif
}

#ifdef _WIN32
// Wrapper function to get GetLastError() string.
// This function might change the error code.
std::string GetLastErrorString()
{
  char error_message[BUFFER_SIZE];

  FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), error_message, BUFFER_SIZE, nullptr);
  return std::string(error_message);
}

// Obtains a full path to the specified module.
std::optional<std::wstring> GetModuleName(void* hInstance)
{
  DWORD max_size = 50;  // Start with space for 50 characters and grow if needed
  std::wstring name(max_size, L'\0');

  DWORD size;
  while ((size = GetModuleFileNameW(static_cast<HMODULE>(hInstance), name.data(), max_size)) ==
             max_size &&
         GetLastError() == ERROR_INSUFFICIENT_BUFFER)
  {
    max_size *= 2;
    name.resize(max_size);
  }

  if (size == 0)
  {
    return std::nullopt;
  }
  name.resize(size);
  return name;
}
#endif

#ifdef __EMSCRIPTEN__
#define RETURN_FAILURE(msg, ...)                                                                   \
  do                                                                                               \
  {                                                                                                \
    emscripten_console_error(fmt::format(msg, ##__VA_ARGS__).c_str());                             \
    return -1;                                                                                     \
  } while (0)

/// Unpack an zip file on the filsystem into a specified directory.
/// Called from the web environment wrapping code to decompress the user
/// directory to the application's filesystem prior to boot.
///
/// If the operation failed to complete, files
/// that were already extracted remain on the
/// filesystem.
///
/// Returns 0 on success, -1 on failure.
extern "C" EMSCRIPTEN_KEEPALIVE int Dolphin_UnpackZip(const char* zip_path, const char* out_dir)
{
  unzFile zip_file = unzOpen(zip_path);
  if (!zip_file)
    RETURN_FAILURE("{}: not found", zip_path);

  Common::ScopeGuard zip_guard{[&] { unzClose(zip_file); }};

  std::string out_dir_str(out_dir);

  if (!StringEndsWith(out_dir_str, DIR_SEP))
    out_dir_str += DIR_SEP;

  if (!File::CreateFullPath(out_dir_str))
    RETURN_FAILURE("could not create extraction destination path {}", out_dir_str);

  unz_global_info global_info;
  if (unzGetGlobalInfo(zip_file, &global_info) != UNZ_OK)
    RETURN_FAILURE("could not read zip archive global info");

  // Buffer to hold a chunk of data read from the
  // file in the archive that is currently being
  // extracted
  constexpr u32 READ_SIZE = 4096;
  char read_buffer[READ_SIZE];

  // Loop to extract all files
  for (uLong i = 0; i < global_info.number_entry;)
  {
    unz_file_info file_info;
    char filename_buffer[PATH_MAX];
    if (unzGetCurrentFileInfo(zip_file, &file_info, filename_buffer, PATH_MAX, nullptr, 0, nullptr,
                              0) != UNZ_OK)
      RETURN_FAILURE("could not retrieve a compressed file's information from the zip archive");

    // Is it a directory? (in that case, the file is empty and last character of its name is a
    // slash, the directory separator)
    if (file_info.uncompressed_size == 0 &&
        filename_buffer[std::strlen(filename_buffer) - 1] == '/')
    {
      // Entry is a directory, so create it
      if (!File::CreateDir(out_dir_str + filename_buffer))
        RETURN_FAILURE("could not create subdirectory {}", filename_buffer);
    }
    else
    {
      // Entry is a file, so extract it
      if (unzOpenCurrentFile(zip_file) != UNZ_OK)
        RETURN_FAILURE("could not open zipped file (compressed file {})", filename_buffer);

      Common::ScopeGuard cur_file_guard{[&] { unzCloseCurrentFile(zip_file); }};

      // Open a file to write out the data
      File::IOFile out_file(out_dir_str + filename_buffer, "wb");
      if (!out_file)
        RETURN_FAILURE("could not open destination file (compressed file {})", filename_buffer);

      int bytes_read;
      do
      {
        bytes_read = unzReadCurrentFile(zip_file, read_buffer, READ_SIZE);
        if (bytes_read < 0)
          RETURN_FAILURE("error while reading compressed file {}: {}", filename_buffer, bytes_read);

        if (!out_file.WriteBytes(read_buffer, bytes_read))
          RETURN_FAILURE("error while writing to destination file (compressed file {})",
                         filename_buffer);

      } while (bytes_read > 0);
    }

    // Go the the next entry listed in the zip file
    if (++i < global_info.number_entry)
    {
      if (unzGoToNextFile(zip_file) != UNZ_OK)
        RETURN_FAILURE("could not switch to next file in zip archive");
    }
  }

  return 0;
}
#endif  // __EMSCRIPTEN__
