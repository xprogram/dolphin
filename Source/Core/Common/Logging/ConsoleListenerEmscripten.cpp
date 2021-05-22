// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Common/Logging/ConsoleListener.h"
#include "Common/Logging/Log.h"
#include "Common/WebAdapter.h"

ConsoleListener::ConsoleListener() : m_use_color(true)
{
}

ConsoleListener::~ConsoleListener()
{
}

void ConsoleListener::Log(Common::Log::LOG_LEVELS level, const char* text)
{
  const char* color = "black";

  if(m_use_color)
  {
    switch (level)
    {
    case Common::Log::LOG_LEVELS::LNOTICE:
      color = "green";
      break;
    case Common::Log::LOG_LEVELS::LERROR:
      color = "red";
      break;
    case Common::Log::LOG_LEVELS::LWARNING:
      color = "orange";
      break;
    default:
      break;
    }
  }

  WebAdapter_LogColored(text, color);
}