// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#include "Common/CPUDetect.h"

#include <cstring>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten/threading.h>
#endif

#include "Common/WasmEnv.h"

CPUInfo cpu_info;

CPUInfo::CPUInfo()
{
  Detect();
}

// Detects the various CPU features
void CPUInfo::Detect()
{
  vendor = CPUVendor::Other;
  num_cores = 1;

  std::strncpy(brand_string, "GenericWebAssemblyVM", sizeof(brand_string));

#ifdef __EMSCRIPTEN__
  logical_cpu_count = emscripten_num_logical_cores();
  std::strncpy(cpu_string, "WebAssemblyJavaScriptVM", sizeof(cpu_string));
#else
  logical_cpu_count = 1;
  std::strcpy(cpu_string, brand_string);
#endif

  bBulkMem = WasmEnv_SupportsFeature(WASM_FEATURE_BULK_MEMORY);
  bFWSIMD = WasmEnv_SupportsFeature(WASM_FEATURE_SIMD);
  bTailCall = WasmEnv_SupportsFeature(WASM_FEATURE_TAIL_CALL);
}

// Turn the CPU info into a string we can show
std::string CPUInfo::Summarize()
{
  std::string sum(cpu_string);
  sum += " (";
  sum += brand_string;
  sum += ")";

  if (bBulkMem)
    sum += ", Bulk Memory";
  if (bFWSIMD)
    sum += ", Fixed-Width SIMD";
  if (bTailCall)
    sum += ", Tail Call Optimization";

  return sum;
}