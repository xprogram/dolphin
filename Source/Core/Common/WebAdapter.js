// Copyright 2021 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

mergeInto(LibraryManager.library, {
  WebAdapter_LogColored: function(msgPtr, colorPtr){
    console.log('%c' + UTF8ToString(msgPtr), 'color:' + UTF8ToString(colorPtr));
  },

  WebAdapter_EncodeURIComponent__sig: 'ii',
  WebAdapter_EncodeURIComponent: function(strPtr){
    return allocateUTF8(encodeURIComponent(UTF8ToString(strPtr)));
  },
  
  WebAdapter_GetUserAgent__proxy: 'sync',
  WebAdapter_GetUserAgent__sig: 'i',
  WebAdapter_GetUserAgent: function() {
    if (!_WebAdapter_GetUserAgent.uaPtr) {
      var ua =
#if ENVIRONMENT_MAY_BE_WEB
        ENVIRONMENT_IS_WEB && navigator.userAgent ||
#endif
        'Undefined User Agent';

      // Small memory leak, as there is no routine to free this pointer
      _WebAdapter_GetUserAgent.uaPtr = allocateUTF8(ua);
    }

    return _WebAdapter_GetUserAgent.uaPtr;
  },
  
  WebAdapter_DisplayAlert__proxy: 'sync',
  WebAdapter_DisplayAlert__sig: 'iii',
  WebAdapter_DisplayAlert: function(msgPtr, useConfirm) {
    var doMsg =
#if ENVIRONMENT_MAY_BE_WEB
      (ENVIRONMENT_IS_WEB) ? (useConfirm ? window.confirm : window.alert) :
#endif
      err;
    return doMsg(UTF8ToString(msgPtr));
  },
  
  WebAdapter_FetchSync__deps: ['$getFuncWrapper'],
  WebAdapter_FetchSync__sig: 'iiiiiiiiiii',
  WebAdapter_FetchSync: function(verbStr, urlStr, headerList, timeoutMs, fetchDataOut, fetchDataSizePtr, payloadPtr, payloadSize, progressCbPtr, userData) {
    var val, i = 0, headerName,
      xhr = new XMLHttpRequest(), progressCb = getFuncWrapper(progressCbPtr, 'iidd');

    xhr.responseType = 'arraybuffer';
    xhr['timeout'] = timeoutMs;

    try {
      xhr.open(UTF8ToString(verbStr), UTF8ToString(urlStr), false);
    } catch(e){
      return -1;
    }

    // Note: because we are running synchronously, this will only fire once, at the very end
    if(progressCb){
      xhr.onprogress = function(ev) {
        if (progressCb(userData, ev.loaded, ev.total))
          xhr.abort();
      };
    }

    if(headerList){
      while ((val = HEAPU32[(headerList >> 2) + i])) {
        if (i % 2)
          xhr.setRequestHeader(headerName, UTF8ToString(val));
        else
          headerName = UTF8ToString(val);

        ++i;
      }
    }

    try {
      xhr.send(payloadPtr ? HEAPU8.slice(payloadPtr, payloadPtr + payloadSize) : undefined);
    } catch (e) {
      return -1;
    }

    if (xhr.readyState != 4) {
      return -1;
    }

    var res = xhr.response;
    if (!res) {
      return -1;
    }
    
    res = new Uint8Array(res);

    var resLen = res.byteLength;
    var fetchDataBuf = _malloc(resLen); // must be freed manually!
    HEAPU32[fetchDataSizePtr >> 2] = resLen;

    // Copy the returned response into program memory efficiently
    HEAPU8.set(res, fetchDataBuf);

    HEAPU32[fetchDataOut >> 2] = fetchDataBuf;

    return xhr.status;
  },
  
  $SignalHandlers: {
    _call: function(sig){
      if(SignalHandlers[sig])
        SignalHandlers[sig](sig);
      
      SignalHandlers[sig] = 0;
    },
    
    _init: function(){
      Module['raise_SIGINT'] = function(){
        SignalHandlers._call(2 /* SIGINT */);
      };
      
      Module['raise_SIGTERM'] = function(){
        SignalHandlers._call(15 /* SIGTERM */);
      };
    }
  },
  
  $SignalHandlers__postset: 'SignalHandlers._init();',
  
  WebAdapter_SetSignalHandler__deps: ['$SignalHandlers', '$getFuncWrapper'],
  WebAdapter_SetSignalHandler__proxy: 'sync',
  WebAdapter_SetSignalHandler__sig: 'vii',
  WebAdapter_SetSignalHandler: function(sig, handlerFuncPtr){
    SignalHandlers[sig] = getFuncWrapper(handlerFuncPtr, 'vi');
  }
});
