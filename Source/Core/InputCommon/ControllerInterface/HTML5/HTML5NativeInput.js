// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

var LibraryHTML5NativeInput = {

  // Object that manages input state
  $BrowserNativeInput: {
    lastMouseX: 0,
    lastMouseY: 0,

    normMouseX: 0,
    normMouseY: 0,

    buttonBits: 0,

    mouseAxisX: 0,
    mouseAxisY: 0,
    mouseAxisZ: 0,

    keyStates: [],
    keyboardAndMouseTarget: null
  },
  
  ///////////////////
  // Exposed C API //
  ///////////////////

  HTML5NativeInput_SetupKeyboardMouseDevice__deps: ['$findEventTarget'],
  HTML5NativeInput_SetupKeyboardMouseDevice__proxy: 'sync',
  HTML5NativeInput_SetupKeyboardMouseDevice__sig: 'viiii',
  HTML5NativeInput_SetupKeyboardMouseDevice: function(targetPtr) {
    if (BrowserNativeInput.keyboardAndMouseTarget)
      throw new Error('Attempted to setup a second keyboard + mouse device!');

    var target = BrowserNativeInput.keyboardAndMouseTarget = findEventTarget(targetPtr);
    if (!target)
      throw new Error('No element with selector ' + target + ' exists for custom input device!');

    for (var i = 0, l = 256; i < l; ++i) {
      BrowserNativeInput.keyStates[i] = 0;
    }

    function getFullDim() {
      return {
        w: target.offsetWidth,
        h: target.offsetHeight
      };
    }

    function getBoundingClientRectOrZeros() {
      return target.getBoundingClientRect ? target.getBoundingClientRect() : {
        left: 0,
        top: 0
      };
    }

    function clampNorm(val) {
      return Math.min(Math.max(val, -1), 1);
    }

    BrowserNativeInput['keydownFunc'] = function(ev) {
      ev.preventDefault();
      BrowserNativeInput.keyStates[ev.keyCode] = 1;
    };

    BrowserNativeInput['keyupFunc'] = function(ev) {
      ev.preventDefault();
      BrowserNativeInput.keyStates[ev.keyCode] = 0;
    };

    function handleMouseEvent(ev) {
      var dim = getFullDim();
      var rect = getBoundingClientRectOrZeros();
      var mouseX, mouseY;

      if (document.pointerLockElement) {
        mouseX = BrowserNativeInput.lastMouseX + ev.movementX;
        mouseY = BrowserNativeInput.lastMouseY + ev.movementY;
      } else {
        mouseX = ev.clientX;
        mouseY = ev.clientY;
      }

      BrowserNativeInput.lastMouseX = mouseX;
      BrowserNativeInput.lastMouseY = mouseY;

      // TODO: Move this magic into C++ code eventually
      var curNormMouseX = clampNorm((mouseX - rect['left']) / dim['w'] * 2 - 1);
      var curNormMouseY = clampNorm((mouseY - rect['top']) / dim['h'] * 2 - 1);

      // Create axis inputs and smooth them out
      var deltaNormMouseX = curNormMouseX - BrowserNativeInput.normMouseX;
      var deltaNormMouseY = curNormMouseY - BrowserNativeInput.normMouseY;
      deltaNormMouseX += deltaNormMouseX / 2;
      deltaNormMouseY += deltaNormMouseY / 2;

      BrowserNativeInput.mouseAxisX = deltaNormMouseX;
      BrowserNativeInput.mouseAxisY = deltaNormMouseY;
      BrowserNativeInput.normMouseX = curNormMouseX;
      BrowserNativeInput.normMouseY = curNormMouseY;

      BrowserNativeInput.buttonBits = ev.buttons;
    };

    BrowserNativeInput['mousemoveFunc'] = handleMouseEvent;
    BrowserNativeInput['mouseupFunc'] = handleMouseEvent;
    BrowserNativeInput['mousedownFunc'] = handleMouseEvent;

    BrowserNativeInput['wheelFunc'] = function(ev) {
      ev.preventDefault();
      BrowserNativeInput.mouseAxisZ = Math.sign(ev.deltaY);

      // Since this inherits MouseEvent, we can process it using the other function as well
      handleMouseEvent(ev);
    };

    target.addEventListener('keydown', BrowserNativeInput['keydownFunc'], false);
    target.addEventListener('keyup', BrowserNativeInput['keyupFunc'], false);
    target.addEventListener('mousedown', BrowserNativeInput['mousedownFunc'], false);
    target.addEventListener('mouseup', BrowserNativeInput['mouseupFunc'], false);
    target.addEventListener('mousemove', BrowserNativeInput['mousemoveFunc'], false);
    target.addEventListener('wheel', BrowserNativeInput['wheelFunc'], false);
  },

  HTML5NativeInput_RemoveKeyboardMouseDevice__proxy: 'sync',
  HTML5NativeInput_RemoveKeyboardMouseDevice__sig: 'v',
  HTML5NativeInput_RemoveKeyboardMouseDevice: function() {
    if (!BrowserNativeInput.keyboardAndMouseTarget)
      throw new Error('Attempted to remove non-existent keyboard + mouse device!');

    var target = BrowserNativeInput.keyboardAndMouseTarget;
    target.removeEventListener('keydown', BrowserNativeInput['keydownFunc'], false);
    target.removeEventListener('keyup', BrowserNativeInput['keyupFunc'], false);
    target.removeEventListener('mousedown', BrowserNativeInput['mousedownFunc'], false);
    target.removeEventListener('mouseup', BrowserNativeInput['mouseupFunc'], false);
    target.removeEventListener('mousemove', BrowserNativeInput['mousemoveFunc'], false);
    target.removeEventListener('wheel', BrowserNativeInput['wheelFunc'], false);

    BrowserNativeInput.keyboardAndMouseTarget = null;
  },

  HTML5NativeInput_GetKeyboardMouseInputState__proxy: 'sync',
  HTML5NativeInput_GetKeyboardMouseInputState__sig: 'viiiii',
  HTML5NativeInput_GetKeyboardMouseInputState: function(cursorXPtr, cursorYPtr, buttonsPtr, axesArray, keyboardArray) {
    if (!BrowserNativeInput.keyboardAndMouseTarget)
      throw new Error('Attempted to query input from non-existent keyboard + mouse device!');

    HEAPF32[cursorXPtr >> 2] = BrowserNativeInput.normMouseX;
    HEAPF32[cursorYPtr >> 2] = BrowserNativeInput.normMouseY;

    HEAPU32[buttonsPtr >> 2] = BrowserNativeInput.buttonBits;

    HEAPF32[axesArray >> 2] = BrowserNativeInput.mouseAxisX;
    HEAPF32[(axesArray >> 2) + 1] = BrowserNativeInput.mouseAxisY;
    HEAPF32[(axesArray >> 2) + 2] = BrowserNativeInput.mouseAxisZ;

    BrowserNativeInput.mouseAxisX = 0;
    BrowserNativeInput.mouseAxisY = 0;
    BrowserNativeInput.mouseAxisZ = 0;

    var val, i;
    for (i = 0, l = 256; i < l; ++i) {
      val = BrowserNativeInput.keyStates[i];
      if (val < 0) {
        continue;
      }

      HEAP8[keyboardArray + i] = val;
      BrowserNativeInput.keyStates[i] = -1;
    }
  }
};

autoAddDeps(LibraryHTML5NativeInput, '$BrowserNativeInput');
mergeInto(LibraryManager.library, LibraryHTML5NativeInput);