// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once

extern "C" {

/// Bulk memory proposal: https://github.com/WebAssembly/bulk-memory-operations
#define WASM_FEATURE_BULK_MEMORY 0
/// Fixed-Width SIMD proposal: https://github.com/WebAssembly/simd
#define WASM_FEATURE_SIMD 1
/// Tail call proposal: https://github.com/WebAssembly/tail-call
#define WASM_FEATURE_TAIL_CALL 2

/**
 * @brief Test if a WebAssembly VM feature is supported.
 *
 * @param feature the feature to test for, one of WASM_FEATURE_XXX
 * @return 1 if the feature is supported, else 0
 */
extern int WasmEnv_SupportsFeature(int feature);
}