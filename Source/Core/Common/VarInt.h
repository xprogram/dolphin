// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

#include <type_traits>

#include "Common/CommonTypes.h"

namespace Common
{
template <typename T, class C>
void EncodeVarInt(C* container, T value)
{
  static_assert(std::is_integral_v<T>, "VarInt (LEB128) encoding only works with integral types");

  const bool negative = value < 0;

  bool more;
  do
  {
    const u8 byte = value & 0x7f;
    value >>= 7;

    if constexpr (std::is_unsigned_v<T>)
      more = value != 0;
    else
      more = !(negative ? value == -1 && (byte & 0x40) != 0 : value == 0 && (byte & 0x40) == 0);

    container->push_back(byte | (more << 7));
  } while (more);
}
}  // Namespace Common
