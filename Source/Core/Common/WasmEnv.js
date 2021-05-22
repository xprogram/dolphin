// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

mergeInto(LibraryManager.library, {
  // Tests taken from: https://github.com/GoogleChromeLabs/wasm-feature-detect
  WasmEnv_SupportsFeature__sig: 'ii',
  WasmEnv_SupportsFeature: function(feat){
    switch(feat){
    case 0:
      // WASM_FEATURE_BULK_MEMORY
      return WebAssembly.validate(new Uint8Array([0,97,115,109,1,0,0,0,1,4,1,96,0,0,3,2,1,0,5,3,1,0,1,10,14,1,12,0,65,0,65,0,65,0,252,10,0,0,11]));
    case 1:
      // WASM_FEATURE_SIMD
      return WebAssembly.validate(new Uint8Array([0,97,115,109,1,0,0,0,1,4,1,96,0,0,3,2,1,0,10,9,1,7,0,65,0,253,15,26,11]));
    case 2:
      // WASM_FEATURE_TAIL_CALL
      return WebAssembly.validate(new Uint8Array([0,97,115,109,1,0,0,0,1,4,1,96,0,0,3,2,1,0,10,6,1,4,0,18,0,11]));
    }
    throw new Error('Invalid feature id ' + feat + ' provided!');
  }
});